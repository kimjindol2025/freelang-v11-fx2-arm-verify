/*
 * sqlite.c — FreeLang 네이티브 SQLite 바인딩
 * libsqlite3가 설치되어 있으면 헤더 포함, 아니면 dlopen 방식
 *
 * 지원 함수 (FL에서 호출):
 *   (fxb_sqlite_open  path)            → "db:N" 핸들
 *   (fxb_sqlite_query db sql)          → 벡터 of 맵
 *   (fxb_sqlite_exec  db sql)          → {"affected": N, "last_id": M}
 *   (fxb_sqlite_one   db sql)          → 단일 맵 or nil
 *   (fxb_sqlite_close db)              → nil
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* SQLite3 헤더 직접 포함 (설치되어 있음) */
#include <sqlite3.h>

/* ── 연결 풀 (path-based 재사용, Connection-Per-Query 방지) ── */
#define MAX_SQLITE_CONN 32

typedef struct {
    char      id[32];
    char      path[512];  /* DB 경로 키 */
    sqlite3*  db;
    pthread_mutex_t lock;
    int       used;
} FLSQLiteConn;

static FLSQLiteConn sq_pool[MAX_SQLITE_CONN];
static int sq_count = 0;
static pthread_mutex_t sq_pool_lock = PTHREAD_MUTEX_INITIALIZER;

static FLSQLiteConn* sq_find(const char* id) {
    for (int i = 0; i < sq_count; i++)
        if (strcmp(sq_pool[i].id, id) == 0) return &sq_pool[i];
    return NULL;
}

/* 동일 경로로 이미 열린 DB 존재 → 재사용 */
static FLSQLiteConn* sq_find_by_path(const char* path) {
    for (int i = 0; i < sq_count; i++)
        if (sq_pool[i].used && strcmp(sq_pool[i].path, path) == 0) return &sq_pool[i];
    return NULL;
}

/* ── FL 인터페이스 ── */

/* (fxb_sqlite_open "/path/to/db.sqlite") → "db:N"
 * 동일 경로로 재호출 시 기존 연결 ID 반환 (연결 재사용) */
FLValue fxb_sqlite_open(FLValue path_v) {
    const char* path = (path_v.tag == FL_STRING)
        ? ((FLString*)path_v.obj)->data
        : ":memory:";

    pthread_mutex_lock(&sq_pool_lock);
    FLSQLiteConn* existing = sq_find_by_path(path);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&sq_pool_lock);
        return fl_str_val(id_copy);
    }
    pthread_mutex_unlock(&sq_pool_lock);

    sqlite3* db = NULL;
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 열기 실패: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return fl_str_val(buf);
    }

    /* WAL 모드 활성화 (동시성 향상) */
    sqlite3_exec(db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON",  NULL, NULL, NULL);

    pthread_mutex_lock(&sq_pool_lock);
    /* double-check */
    existing = sq_find_by_path(path);
    if (existing) {
        char id_copy[32];
        strncpy(id_copy, existing->id, 32);
        pthread_mutex_unlock(&sq_pool_lock);
        sqlite3_close(db);
        return fl_str_val(id_copy);
    }
    if (sq_count >= MAX_SQLITE_CONN) {
        pthread_mutex_unlock(&sq_pool_lock);
        sqlite3_close(db);
        return fl_str_val("[sqlite] 연결 풀 가득");
    }
    int idx = sq_count++;
    snprintf(sq_pool[idx].id, sizeof(sq_pool[idx].id), "db:%d", idx);
    strncpy(sq_pool[idx].path, path, sizeof(sq_pool[idx].path));
    sq_pool[idx].db   = db;
    sq_pool[idx].used = 1;
    pthread_mutex_init(&sq_pool[idx].lock, NULL);
    char id_copy[32];
    strncpy(id_copy, sq_pool[idx].id, 32);
    pthread_mutex_unlock(&sq_pool_lock);

    return fl_str_val(id_copy);
}

/* 내부: rows → FLValue */
static FLValue sq_fetch_rows(sqlite3_stmt* stmt) {
    int ncols = sqlite3_column_count(stmt);

    /* O(n) 수집: C 동적 배열 → fl_vec_from 단일 호출 */
    size_t cap = 16, len = 0;
    FLValue* arr = (FLValue*)malloc(sizeof(FLValue) * cap);
    if (!arr) return fl_vec_new();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FLValue row = fl_map_new();
        for (int i = 0; i < ncols; i++) {
            const char* col_name = sqlite3_column_name(stmt, i);
            FLValue key = fl_str_val(col_name);
            FLValue val;
            int col_type = sqlite3_column_type(stmt, i);
            if (col_type == SQLITE_NULL) {
                val = fl_nil();
            } else if (col_type == SQLITE_INTEGER) {
                val = fl_int(sqlite3_column_int64(stmt, i));
            } else if (col_type == SQLITE_FLOAT) {
                val = fl_float(sqlite3_column_double(stmt, i));
            } else {
                const char* text = (const char*)sqlite3_column_text(stmt, i);
                val = fl_str_val(text ? text : "");
            }
            row = fl_map_set(row, key, val);
        }
        if (len >= cap) {
            cap *= 2;
            FLValue* tmp = (FLValue*)realloc(arr, sizeof(FLValue) * cap);
            if (!tmp) { free(arr); return fl_vec_new(); }
            arr = tmp;
        }
        arr[len++] = row;
    }

    FLValue result = fl_vec_from(arr, (uint32_t)len);
    free(arr);
    return result;
}

/* (fxb_sqlite_query db sql) → 벡터 of 맵 */
FLValue fxb_sqlite_query(FLValue conn_v, FLValue sql_v) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    FLValue rows = sq_fetch_rows(stmt);
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);
    return rows;
}

/* (fxb_sqlite_exec db sql) → {"affected": N, "last_id": M} */
FLValue fxb_sqlite_exec(FLValue conn_v, FLValue sql_v) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    char* errmsg = NULL;
    int rc = sqlite3_exec(c->db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 실행 오류: %s", errmsg ? errmsg : "?");
        if (errmsg) sqlite3_free(errmsg);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }
    int64_t affected = (int64_t)sqlite3_changes(c->db);
    int64_t last_id  = (int64_t)sqlite3_last_insert_rowid(c->db);
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int(affected));
    map = fl_map_set(map, fl_str_val("last_id"),  fl_int(last_id));
    return map;
}

/* (fxb_sqlite_one db sql) → 단일 맵 or nil */
FLValue fxb_sqlite_one(FLValue conn_v, FLValue sql_v) {
    FLValue rows = fxb_sqlite_query(conn_v, sql_v);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    return (vec->len > 0) ? vec->data[0] : fl_nil();
}

/* ── 파라미터 바인딩 헬퍼 ──────────────────────────────────
   FLVector → sqlite3_stmt 바인딩
   지원 타입: nil, bool, int, float, string
   SQL injection 방어: 값은 절대 SQL에 직접 삽입 안 함
*/
static int sq_bind_params(sqlite3_stmt* stmt, FLValue params) {
    if (params.tag != FL_VECTOR) return SQLITE_OK;
    FLVector* vec = (FLVector*)params.obj;
    for (uint32_t i = 0; i < vec->len; i++) {
        int idx = (int)i + 1;   /* SQLite 바인딩은 1-based */
        FLValue v = vec->data[i];
        int rc;
        switch (v.tag) {
            case FL_NIL:
                rc = sqlite3_bind_null(stmt, idx);
                break;
            case FL_BOOL:
                rc = sqlite3_bind_int(stmt, idx, v.b ? 1 : 0);
                break;
            case FL_INT:
                rc = sqlite3_bind_int64(stmt, idx, v.i);
                break;
            case FL_FLOAT:
                rc = sqlite3_bind_double(stmt, idx, v.f);
                break;
            case FL_STRING:
                rc = sqlite3_bind_text(stmt, idx,
                    ((FLString*)v.obj)->data, -1, SQLITE_TRANSIENT);
                break;
            default:
                rc = sqlite3_bind_null(stmt, idx);
        }
        if (rc != SQLITE_OK) return rc;
    }
    return SQLITE_OK;
}

/* (fxb_sqlite_query_p db sql params) → 벡터 of 맵
   예: (fxb_sqlite_query_p db "SELECT * FROM t WHERE id=? AND name=?"
                          (list 42 "kim"))
*/
FLValue fxb_sqlite_query_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_vec_new();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    fl_log_sql("sqlite", sql);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sq_bind_params(stmt, params);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 바인딩 실패: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    FLValue rows = sq_fetch_rows(stmt);
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);
    return rows;
}

/* (fxb_sqlite_exec_p db sql params) → {"affected": N, "last_id": M}
   예: (fxb_sqlite_exec_p db "INSERT INTO links (code, url) VALUES (?, ?)"
                         (list $code $url))
*/
FLValue fxb_sqlite_exec_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    if (conn_v.tag != FL_STRING || sql_v.tag != FL_STRING)
        return fl_nil();

    const char* id  = ((FLString*)conn_v.obj)->data;
    const char* sql = ((FLString*)sql_v.obj)->data;

    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_str_val("[sqlite] 연결 없음");

    pthread_mutex_lock(&c->lock);
    fl_log_sql("sqlite", sql);

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(c->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 준비 실패: %s", sqlite3_errmsg(c->db));
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sq_bind_params(stmt, params);
    if (rc != SQLITE_OK) {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 바인딩 실패: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    rc = sqlite3_step(stmt);
    int64_t affected = 0, last_id = 0;
    if (rc == SQLITE_DONE) {
        affected = (int64_t)sqlite3_changes(c->db);
        last_id  = (int64_t)sqlite3_last_insert_rowid(c->db);
    } else {
        char buf[512];
        snprintf(buf, sizeof(buf), "[sqlite] 실행 오류: %s", sqlite3_errmsg(c->db));
        sqlite3_finalize(stmt);
        pthread_mutex_unlock(&c->lock);
        return fl_str_val(buf);
    }

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&c->lock);

    FLValue map = fl_map_new();
    map = fl_map_set(map, fl_str_val("affected"), fl_int(affected));
    map = fl_map_set(map, fl_str_val("last_id"),  fl_int(last_id));
    return map;
}

/* (fxb_sqlite_one_p db sql params) → 단일 맵 or nil */
FLValue fxb_sqlite_one_p(FLValue conn_v, FLValue sql_v, FLValue params) {
    FLValue rows = fxb_sqlite_query_p(conn_v, sql_v, params);
    if (rows.tag != FL_VECTOR) return fl_nil();
    FLVector* vec = (FLVector*)rows.obj;
    return (vec->len > 0) ? vec->data[0] : fl_nil();
}

/* (fxb_sqlite_close db) → nil */
FLValue fxb_sqlite_close(FLValue conn_v) {
    if (conn_v.tag != FL_STRING) return fl_nil();
    const char* id = ((FLString*)conn_v.obj)->data;
    FLSQLiteConn* c = sq_find(id);
    if (!c) return fl_nil();
    pthread_mutex_lock(&c->lock);
    if (c->db) {
        sqlite3_close(c->db);
        c->db   = NULL;
        c->used = 0;
    }
    pthread_mutex_unlock(&c->lock);
    return fl_nil();
}
