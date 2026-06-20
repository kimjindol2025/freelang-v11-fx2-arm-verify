/*
 * mariadb.c — FreeLang 네이티브 MariaDB 바인딩
 * 헤더 없이 dlopen/dlsym으로 런타임 로딩 (libmariadb-dev 불필요)
 *
 * 지원 함수:
 *   (mariadb_connect host port user pw db)  → conn 핸들(문자열)
 *   (mariadb_query conn sql)               → 결과 벡터 (맵의 벡터)
 *   (mariadb_exec conn sql)                → {"affected": N}
 *   (mariadb_one conn sql)                 → 단일 행 맵 or nil
 *   (mariadb_close conn)                   → nil
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>

/* ── 타입 정의 (opaque pointer로 처리) ── */
typedef void MYSQL;
typedef void MYSQL_RES;
typedef char** MYSQL_ROW;
typedef void MYSQL_FIELD;  /* opaque — name은 첫 번째 char* 멤버 */

/* ── 함수 포인터 테이블 ── */
static void* mariadb_lib = NULL;
static pthread_once_t lib_init_once = PTHREAD_ONCE_INIT;

typedef MYSQL*       (*fn_mysql_init)(MYSQL*);
typedef MYSQL*       (*fn_mysql_real_connect)(MYSQL*, const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned long);
typedef int          (*fn_mysql_real_query)(MYSQL*, const char*, unsigned long);
typedef MYSQL_RES*   (*fn_mysql_store_result)(MYSQL*);
typedef MYSQL_ROW    (*fn_mysql_fetch_row)(MYSQL_RES*);
typedef unsigned long* (*fn_mysql_fetch_lengths)(MYSQL_RES*);
typedef unsigned int (*fn_mysql_num_fields)(MYSQL_RES*);
typedef MYSQL_FIELD* (*fn_mysql_fetch_field_direct)(MYSQL_RES*, unsigned int);
typedef void         (*fn_mysql_free_result)(MYSQL_RES*);
typedef void         (*fn_mysql_close)(MYSQL*);
typedef const char*  (*fn_mysql_error)(MYSQL*);
typedef uint64_t     (*fn_mysql_affected_rows)(MYSQL*);
typedef uint64_t     (*fn_mysql_num_rows)(MYSQL_RES*);
typedef unsigned long (*fn_mysql_real_escape_string)(MYSQL*, char*, const char*, unsigned long);

static fn_mysql_init           p_mysql_init           = NULL;
static fn_mysql_real_connect   p_mysql_real_connect   = NULL;
static fn_mysql_real_query     p_mysql_real_query     = NULL;
static fn_mysql_store_result   p_mysql_store_result   = NULL;
static fn_mysql_fetch_row      p_mysql_fetch_row      = NULL;
static fn_mysql_fetch_lengths  p_mysql_fetch_lengths  = NULL;
static fn_mysql_num_fields     p_mysql_num_fields     = NULL;
static fn_mysql_fetch_field_direct p_mysql_fetch_field_direct = NULL;
static fn_mysql_free_result    p_mysql_free_result    = NULL;
static fn_mysql_close          p_mysql_close          = NULL;
static fn_mysql_error          p_mysql_error          = NULL;
static fn_mysql_affected_rows  p_mysql_affected_rows  = NULL;
static fn_mysql_num_rows       p_mysql_num_rows       = NULL;
static fn_mysql_real_escape_string p_mysql_real_escape_string = NULL;

static void load_mariadb_lib(void) {
    /* libmariadb.so.3 → libmysqlclient.so.21 순서로 시도 */
    const char* libs[] = {
        "libmariadb.so.3",
        "libmariadb.so",
        "libmysqlclient.so.21",
        "libmysqlclient.so",
        NULL
    };
    for (int i = 0; libs[i]; i++) {
        mariadb_lib = dlopen(libs[i], RTLD_LAZY | RTLD_GLOBAL);
        if (mariadb_lib) break;
    }
    if (!mariadb_lib) {
        fprintf(stderr, "[mariadb] 라이브러리 로딩 실패: %s\n", dlerror());
        return;
    }

#define LOAD(fn) p_##fn = (fn_##fn)dlsym(mariadb_lib, #fn); \
    if (!p_##fn) fprintf(stderr, "[mariadb] 함수 없음: " #fn "\n");

    LOAD(mysql_init)
    LOAD(mysql_real_connect)
    LOAD(mysql_real_query)
    LOAD(mysql_store_result)
    LOAD(mysql_fetch_row)
    LOAD(mysql_fetch_lengths)
    LOAD(mysql_num_fields)
    LOAD(mysql_fetch_field_direct)
    LOAD(mysql_free_result)
    LOAD(mysql_close)
    LOAD(mysql_error)
    LOAD(mysql_affected_rows)
    LOAD(mysql_num_rows)
    LOAD(mysql_real_escape_string)
#undef LOAD
}

static int ensure_lib(void) {
    pthread_once(&lib_init_once, load_mariadb_lib);
    return mariadb_lib != NULL;
}

/* ── 연결 풀 (idempotent key 기반, Connection-Per-Query 방지) ── */
#define MAX_CONNECTIONS 32

typedef struct {
    char     id[32];
    char     key[256];   /* "host:port/db/user" — 연결 식별 키 */
    MYSQL*   conn;
    pthread_mutex_t lock;
    int      used;
} FLMariaConn;

static FLMariaConn conn_pool[MAX_CONNECTIONS];
static int conn_count = 0;
static pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;

static FLMariaConn* find_conn(const char* id) {
    for (int i = 0; i < conn_count; i++) {
        if (strcmp(conn_pool[i].id, id) == 0)
            return &conn_pool[i];
    }
    return NULL;
}

/* 동일 key(host+port+db+user)로 이미 연결 존재 → 재사용 (Connection-Per-Query 방지) */
static FLMariaConn* find_conn_by_key(const char* key) {
    for (int i = 0; i < conn_count; i++) {
        if (conn_pool[i].used && strcmp(conn_pool[i].key, key) == 0)
            return &conn_pool[i];
    }
    return NULL;
}

/* ── FL 인터페이스 ── */

/* (mariadb_connect "localhost" 3306 "user" "pass" "dbname") → "conn:N"
 * 동일 파라미터로 재호출 시 기존 연결 ID 반환 (연결 재사용) */
FLValue mariadb_connect(FLValue host_v, FLValue port_v, FLValue user_v,
                        FLValue pw_v, FLValue db_v) {
    if (!ensure_lib())
        return fl_str_val("[mariadb] 라이브러리 없음");

    const char* host = (host_v.tag == FL_STRING) ? ((FLString*)host_v.obj)->data : "localhost";
    int         port = (port_v.tag == FL_INT)    ? (int)port_v.i : 3306;
    const char* user = (user_v.tag == FL_STRING) ? ((FLString*)user_v.obj)->data : "root";
    const char* pw   = (pw_v.tag  == FL_STRING)  ? ((FLString*)pw_v.obj)->data  : "";
    const char* db   = (db_v.tag  == FL_STRING)  ? ((FLString*)db_v.obj)->data  : NULL;

    /* 연결 키: "host:port/db/user" */
    char key[256];
    snprintf(key, sizeof(key), "%s:%d/%s/%s", host, port, db ? db : "", user);

    pthread_mutex_lock(&pool_lock);
    /* 이미 동일 파라미터 연결 존재 → 기존 ID 반환 */
    FLMariaConn* existing = find_conn_by_key(key);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&pool_lock);
        return fl_str_val(id_copy);
    }
    pthread_mutex_unlock(&pool_lock);

    MYSQL* conn = p_mysql_init(NULL);
    if (!conn) return fl_str_val("[mariadb] mysql_init 실패");

    MYSQL* ok = p_mysql_real_connect(conn, host, user, pw, db, (unsigned int)port, NULL, 0);
    if (!ok) {
        const char* err = p_mysql_error(conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 연결 실패: %s", err);
        p_mysql_close(conn);
        return fl_str_val(buf);
    }

    pthread_mutex_lock(&pool_lock);
    /* double-check: 다른 스레드가 먼저 연결했을 수 있음 */
    existing = find_conn_by_key(key);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&pool_lock);
        p_mysql_close(conn);
        return fl_str_val(id_copy);
    }
    if (conn_count >= MAX_CONNECTIONS) {
        pthread_mutex_unlock(&pool_lock);
        p_mysql_close(conn);
        return fl_str_val("[mariadb] 연결 풀 가득");
    }
    int idx = conn_count++;
    snprintf(conn_pool[idx].id, sizeof(conn_pool[idx].id), "conn:%d", idx);
    strncpy(conn_pool[idx].key, key, sizeof(conn_pool[idx].key));
    conn_pool[idx].conn = conn;
    conn_pool[idx].used = 1;
    pthread_mutex_init(&conn_pool[idx].lock, NULL);
    char id_copy[32];
    strncpy(id_copy, conn_pool[idx].id, 32);
    pthread_mutex_unlock(&pool_lock);

    return fl_str_val(id_copy);
}

/* rows → FLValue vector of maps  (O(n), 단일 fl_vec_from 할당) */
static FLValue fetch_rows(MYSQL_RES* res) {
    if (!res) return fl_vec_new();

    unsigned int nfields = p_mysql_num_fields(res);
    uint64_t nrows = p_mysql_num_rows ? p_mysql_num_rows(res) : 0;

    /* pre-allocate C 배열 (mysql_store_result 후 행수 확정) */
    size_t cap = nrows > 0 ? (size_t)nrows : 16;
    size_t len = 0;
    FLValue* arr = (FLValue*)malloc(sizeof(FLValue) * cap);
    if (!arr) { p_mysql_free_result(res); return fl_vec_new(); }

    MYSQL_ROW row;
    while ((row = p_mysql_fetch_row(res))) {
        unsigned long* lens = p_mysql_fetch_lengths(res);
        FLValue map = fl_map_new();
        for (unsigned int i = 0; i < nfields; i++) {
            MYSQL_FIELD* field = p_mysql_fetch_field_direct(res, i);
            const char* col_name = *(const char**)field;
            FLValue key = fl_str_val(col_name ? col_name : "");
            FLValue val;
            if (row[i] == NULL) {
                val = fl_nil();
            } else {
                char* buf = (char*)malloc(lens[i] + 1);
                memcpy(buf, row[i], lens[i]);
                buf[lens[i]] = '\0';
                val = fl_str_val(buf);
                free(buf);
            }
            map = fl_map_set(map, key, val);
        }
        if (len >= cap) {
            cap *= 2;
            FLValue* tmp = (FLValue*)realloc(arr, sizeof(FLValue) * cap);
            if (!tmp) { free(arr); p_mysql_free_result(res); return fl_vec_new(); }
            arr = tmp;
        }
        arr[len++] = map;
    }
    p_mysql_free_result(res);

    FLValue result = fl_vec_from(arr, (uint32_t)len);
    free(arr);
    return result;
}

/* (mariadb_query conn_id sql) → vector of maps */
FLValue mariadb_query(FLValue conn_v, FLValue sql_v) {
    if (!ensure_lib()) return fl_vec_new();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    int rc = p_mysql_real_query(c->conn, sql, (unsigned long)strlen(sql));
    if (rc != 0) {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 쿼리 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    MYSQL_RES* res = p_mysql_store_result(c->conn);
    pthread_mutex_unlock(&c->lock);

    return fetch_rows(res);
}

/* (mariadb_exec conn_id sql) → {"affected": N} */
FLValue mariadb_exec(FLValue conn_v, FLValue sql_v) {
    if (!ensure_lib()) return fl_nil();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    int rc = p_mysql_real_query(c->conn, sql, (unsigned long)strlen(sql));
    uint64_t affected = 0;
    if (rc == 0) {
        affected = (uint64_t)p_mysql_affected_rows(c->conn);
    } else {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 실행 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int((int64_t)affected));
    return map;
}

/* (mariadb_one conn_id sql) → 단일 맵 or nil */
FLValue mariadb_one(FLValue conn_v, FLValue sql_v) {
    FLValue rows = mariadb_query(conn_v, sql_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    if (vec->len == 0) return fl_nil();
    return vec->data[0];
}

/* (mariadb_close conn_id) → nil */
FLValue mariadb_close(FLValue conn_v) {
    if (!ensure_lib()) return fl_nil();
    if (conn_v.tag != FL_STRING) return fl_nil();

    const char* id = ((FLString*)conn_v.obj)->data;
    FLMariaConn* c = find_conn(id);
    if (!c) return fl_nil();

    pthread_mutex_lock(&c->lock);
    if (c->conn) {
        p_mysql_close(c->conn);
        c->conn = NULL;
        c->used = 0;
    }
    pthread_mutex_unlock(&c->lock);
    return fl_nil();
}

/* ── ? 파라미터 바인딩 헬퍼 ──
 * SQL 문자열의 ? 를 params 벡터의 값으로 치환 (mysql_real_escape_string 사용)
 */
static char* build_sql_with_params(MYSQL* conn, const char* sql, FLValue params_v) {
    /* params가 벡터가 아니면 SQL 그대로 복사 */
    FLVector* params = NULL;
    uint32_t nparams = 0;
    if (params_v.tag == FL_VECTOR) {
        params = (FLVector*)params_v.obj;
        nparams = params->len;
    }

    size_t sql_len = strlen(sql);
    /* worst case: 각 ? 를 최대 2*val_len+2 로 치환 */
    size_t buf_cap = sql_len * 4 + nparams * 512 + 64;
    char*  buf     = (char*)malloc(buf_cap);
    if (!buf) return NULL;

    size_t out = 0;
    uint32_t pi = 0;

    for (size_t i = 0; i < sql_len; i++) {
        if (sql[i] == '?' && pi < nparams) {
            FLValue v = params->data[pi++];
            char tmp[2048];
            size_t tmp_len = 0;

            if (v.tag == FL_NIL) {
                memcpy(tmp, "NULL", 4); tmp_len = 4;
            } else if (v.tag == FL_BOOL) {
                int b = (v.i != 0);
                memcpy(tmp, b ? "1" : "0", 1); tmp_len = 1;
            } else if (v.tag == FL_INT) {
                tmp_len = (size_t)snprintf(tmp, sizeof(tmp), "%lld", (long long)v.i);
            } else if (v.tag == FL_FLOAT) {
                tmp_len = (size_t)snprintf(tmp, sizeof(tmp), "%.17g", v.f);
            } else {
                /* 문자열 — real_escape */
                const char* s = ((FLString*)v.obj)->data;
                size_t slen   = strlen(s);
                char* esc     = (char*)malloc(slen * 2 + 1);
                if (esc && p_mysql_real_escape_string) {
                    unsigned long elen = p_mysql_real_escape_string(conn, esc, s, (unsigned long)slen);
                    /* 따옴표 포함 */
                    if (out + elen + 2 + 1 > buf_cap) {
                        buf_cap = (out + elen + 2 + 1) * 2;
                        buf = (char*)realloc(buf, buf_cap);
                    }
                    buf[out++] = '\'';
                    memcpy(buf + out, esc, elen); out += elen;
                    buf[out++] = '\'';
                    free(esc);
                    continue;
                }
                if (esc) free(esc);
                /* fallback: 수동 이스케이프 */
                tmp[0] = '\'';
                size_t k = 1;
                for (size_t j = 0; j < slen && k < sizeof(tmp) - 4; j++) {
                    if (s[j] == '\'' || s[j] == '\\') tmp[k++] = '\\';
                    tmp[k++] = s[j];
                }
                tmp[k++] = '\'';
                tmp_len = k;
            }

            if (out + tmp_len + 1 > buf_cap) {
                buf_cap = (out + tmp_len + 1) * 2;
                buf = (char*)realloc(buf, buf_cap);
            }
            memcpy(buf + out, tmp, tmp_len);
            out += tmp_len;
        } else {
            if (out + 1 >= buf_cap) {
                buf_cap *= 2;
                buf = (char*)realloc(buf, buf_cap);
            }
            buf[out++] = sql[i];
        }
    }
    buf[out] = '\0';
    return buf;
}

/* (mariadb_query_p conn sql params) → vector of maps */
FLValue mariadb_query_p(FLValue conn_v, FLValue sql_v, FLValue params_v) {
    if (!ensure_lib()) return fl_vec_new();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    char* built = build_sql_with_params(c->conn, sql, params_v);
    if (!built) { pthread_mutex_unlock(&c->lock); return fl_vec_new(); }

    int rc = p_mysql_real_query(c->conn, built, (unsigned long)strlen(built));
    free(built);
    if (rc != 0) {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 쿼리 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    MYSQL_RES* res = p_mysql_store_result(c->conn);
    pthread_mutex_unlock(&c->lock);

    return fetch_rows(res);
}

/* (mariadb_exec_p conn sql params) → {"affected": N} */
FLValue mariadb_exec_p(FLValue conn_v, FLValue sql_v, FLValue params_v) {
    if (!ensure_lib()) return fl_nil();
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLMariaConn* c = find_conn(id);
    if (!c) return fl_str_val("[mariadb] 연결 없음");

    pthread_mutex_lock(&c->lock);
    char* built = build_sql_with_params(c->conn, sql, params_v);
    if (!built) { pthread_mutex_unlock(&c->lock); return fl_nil(); }

    int rc = p_mysql_real_query(c->conn, built, (unsigned long)strlen(built));
    free(built);
    uint64_t affected = 0;
    if (rc == 0) {
        affected = (uint64_t)p_mysql_affected_rows(c->conn);
    } else {
        const char* err = p_mysql_error(c->conn);
        char buf[512];
        snprintf(buf, sizeof(buf), "[mariadb] 실행 오류: %s", err);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int((int64_t)affected));
    return map;
}

/* (mariadb_one_p conn sql params) → 단일 맵 or nil */
FLValue mariadb_one_p(FLValue conn_v, FLValue sql_v, FLValue params_v) {
    FLValue rows = mariadb_query_p(conn_v, sql_v, params_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    if (vec->len == 0) return fl_nil();
    return vec->data[0];
}
