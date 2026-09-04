#ifndef FREELANG_RUNTIME_H
#define FREELANG_RUNTIME_H

/*
 * FreeLang C 런타임 ABI 버전
 *
 * Major: FLValue 구조 변경 (기존 앱 전체 재빌드 필요)
 * Minor: 새 함수 추가 (하위호환 유지)
 * Patch: 버그 수정 (재빌드 불필요)
 *
 * 빌드된 앱은 자신이 컴파일된 ABI Major를 포함.
 * runtime.so 로드 시 Major 불일치 → 시작 거부.
 */
#define FL_ABI_MAJOR  2
#define FL_ABI_MINOR  0
#define FL_ABI_PATCH  0
#define FL_ABI_VERSION ((FL_ABI_MAJOR << 16) | (FL_ABI_MINOR << 8) | FL_ABI_PATCH)
#define FL_ABI_STRING "2.0.0"

/* cgc-bin이 생성하는 코드에서 ABI 검사용 */
static inline void fl_check_abi(int expected_major) {
    if (expected_major != FL_ABI_MAJOR) {
        /* 런타임과 컴파일된 앱의 ABI 불일치 */
        __builtin_trap();   /* 즉시 SIGILL — 조용한 오동작 방지 */
    }
}

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <setjmp.h>

/* ── ABI 헌법 (self/ABI.md) ── */

typedef enum {
    FL_INT,
    FL_FLOAT,
    FL_BOOL,
    FL_NIL,
    FL_STRING,
    FL_VECTOR,
    FL_MAP,
    FL_FN
} FLTag;

typedef struct FLObject FLObject;

typedef struct {
    FLTag tag;
    union {
        int64_t  i;
        double   f;
        bool     b;
        FLObject* obj;
    };
} FLValue;

struct FLObject {
    FLTag    type;
    uint32_t rc;
};

typedef struct {
    FLObject base;
    uint32_t len;
    char     data[];
} FLString;

/* ── S5: Heap Object System ── */

typedef struct {
    FLValue key;
    FLValue val;
} FLMapEntry;

typedef struct {
    FLObject  base;   /* type = FL_VECTOR */
    uint32_t  len;
    uint32_t  cap;
    FLValue*  data;
} FLVector;

typedef struct {
    FLObject    base;  /* type = FL_MAP */
    uint32_t    len;
    uint32_t    cap;
    FLMapEntry* entries;
} FLMap;

/* ── 값 생성 ── */
FLValue fl_int(int64_t v);
FLValue fl_float(double v);
FLValue fl_str_val(const char* s);
FLValue fl_str_val_n(const char* data, uint32_t len);
FLValue fl_bool(bool v);
FLValue fl_nil(void);

/* ── 조건 판별 ── */
bool fl_truthy(FLValue v);

/* ── 산술 ── */
FLValue fl_add(FLValue a, FLValue b);
FLValue fl_sub(FLValue a, FLValue b);
FLValue fl_mul(FLValue a, FLValue b);
FLValue fl_div(FLValue a, FLValue b);
FLValue fl_mod(FLValue a, FLValue b);

/* ── 비교 ── */
FLValue fl_eq(FLValue a, FLValue b);
FLValue fl_lt(FLValue a, FLValue b);
FLValue fl_gt(FLValue a, FLValue b);
FLValue fl_lte(FLValue a, FLValue b);
FLValue fl_gte(FLValue a, FLValue b);
FLValue fl_neq(FLValue a, FLValue b);

/* ── 논리 ── */
FLValue fl_not(FLValue a);
FLValue fl_and(FLValue a, FLValue b);
FLValue fl_or(FLValue a, FLValue b);

/* ── 문자열 ── */
FLValue fl_str_n(int count, ...);

/* ── I/O ── */
FLValue fl_println(FLValue v);
FLValue fl_print(FLValue v);

/* ── 파일 I/O ── */
FLValue fl_file_read(FLValue path);
FLValue fl_file_write(FLValue path, FLValue content);

/* ── Vector ── */
FLValue fl_vec_new(void);
FLValue fl_vec_from(FLValue* items, uint32_t n);
FLValue fl_vec_get(FLValue vec, FLValue idx);
FLValue fl_vec_set(FLValue vec, FLValue idx, FLValue val);
FLValue fl_vec_push(FLValue vec, FLValue val);
FLValue fl_append(FLValue left, FLValue right);
FLValue fl_vec_len(FLValue vec);

/* ── 가변 빌더 (rc=0xFE, heap-allocated) ── */
FLValue fl_vec_builder_new(void);
void    fl_vec_builder_push(FLValue b, FLValue val);
FLValue fl_vec_builder_freeze(FLValue b);

/* ── Map ── */
FLValue fl_map_new(void);
FLValue fl_map_from_pairs(FLValue* kv, uint32_t n); /* n = 쌍의 수 */
FLValue fl_map_get(FLValue map, FLValue key);
FLValue fl_map_set(FLValue map, FLValue key, FLValue val);
FLValue fl_map_len(FLValue map);

/* ── S7: Closure ── */
typedef struct FLClosure {
    FLObject base;   /* type = FL_FN */
    FLValue (*call)(struct FLClosure* self, int argc, FLValue* argv);
    uint32_t nenv;
    uint32_t hot_count;  /* JIT 핫 카운터 (jit.c) */
    FLValue  env[];      /* flexible array — captured values */
} FLClosure;

FLValue fl_fn_new(FLValue (*call)(FLClosure*, int, FLValue*),
                  uint32_t nenv, FLValue* env);
FLValue fl_make_native_fn(FLValue (*call)(FLClosure*, int, FLValue*), const char* name);
FLValue fl_fn_call(FLValue fn, int argc, FLValue* argv);

/* ── S8: 고차함수 ── */
FLValue fl_map_fn(FLValue fn, FLValue vec);
FLValue fl_filter_fn(FLValue fn, FLValue vec);
FLValue fl_reduce_fn(FLValue fn, FLValue init, FLValue vec);
void    fl_for_each(FLValue fn, FLValue vec);
FLValue distinct(FLValue vec);
FLValue js_escape(FLValue s);
FLValue fl_html_escape(FLValue s);

/* ── RC-Heap GC (gc.c) ── */
FLValue fl_heap_copy(FLValue v);
void    fl_heap_retain(FLValue v);
void    fl_heap_release(FLValue v);

FLValue range2(FLValue start, FLValue end);  /* 직접 C 호출용 */

/* ── JIT 컴파일러 (jit.c) ── */
FLValue jit_stats(void);
FLValue jit_test_add1(FLValue x);
FLValue jit_test_mul2(FLValue x);
FLValue jit_test_add_xy(FLValue x, FLValue y);
void    fl_jit_cleanup(void);

/* ── 추가 alias/구현 (aliases.c) ── */
FLValue fl_str_to_num(FLValue s);
FLValue str_to_upper(FLValue s);
FLValue str_to_lower(FLValue s);
FLValue fl_any_p(FLValue fn, FLValue vec);
FLValue fl_every_p(FLValue fn, FLValue vec);
FLValue fl_contains_p(FLValue coll, FLValue key);
FLValue fl_apply(FLValue fn, FLValue args);
FLValue fl_deref_timeout(FLValue handle, FLValue ms_v);
FLValue fl_sleep_ms(FLValue ms_v);
FLValue fl_none_p(FLValue fn, FLValue vec);
FLValue fl_count_if(FLValue fn, FLValue vec);
FLValue fl_find_first(FLValue fn, FLValue vec);
FLValue entries(FLValue m);
FLValue select_keys(FLValue m, FLValue ks);
FLValue fl_get_in(FLValue m, FLValue path);
FLValue get_in(FLValue m, FLValue path);
FLValue assoc_in(FLValue m, FLValue path, FLValue v);
FLValue update_in(FLValue m, FLValue path, FLValue fn);
FLValue str_format(FLValue fmt_v, FLValue args);
FLValue concat(FLValue a, FLValue b) ;
FLValue list_new0(void)                    ;
FLValue fl_obj_omit(FLValue m, FLValue ks);
FLValue fl_repeat(FLValue n_v, FLValue val);
FLValue fl_sort_by(FLValue fn, FLValue vec);
FLValue fl_keep(FLValue fn, FLValue vec);
FLValue fl_map_indexed(FLValue fn, FLValue vec);
FLValue fl_mapcat(FLValue fn, FLValue vec);
FLValue fl_into(FLValue target, FLValue src);
FLValue fl_conj(FLValue coll, FLValue item);
FLValue fl_comp(FLValue f, FLValue g);
FLValue fl_map_vals_fn(FLValue fn, FLValue m);
FLValue frequencies(FLValue vec);
FLValue group_by(FLValue fn, FLValue vec);
FLValue fl_resp_set_cookie(FLValue name, FLValue val, FLValue opts);
FLValue fl_resp_html_cookie(FLValue html, FLValue cookie_hdr);

/* ── S9: 맵 accessor ── */
FLValue fl_map_keys(FLValue map);
FLValue fl_map_vals(FLValue map);
FLValue fl_map_entries(FLValue map);

/* ── S22: argv ── */
void    fl_init_argv(int argc, char** argv);
FLValue fl_get_argv(void);

/* ── S12: bridge builtins (FL generic) ── */
FLValue null_p(FLValue v);
FLValue get(FLValue obj, FLValue key);
FLValue length(FLValue obj);
FLValue char_at(FLValue str, FLValue idx);

/* ── S15: stdlib bridge ── */
FLValue fl_floor(FLValue x);
FLValue fl_ceil(FLValue x);
FLValue fl_abs(FLValue x);
FLValue fl_math_sqrt(FLValue x);
FLValue fl_now(void);
FLValue fl_now_ms(void);
FLValue string_p(FLValue v);
FLValue array_p(FLValue v);
FLValue list_p(FLValue v);
FLValue map_p(FLValue v);
FLValue fn_p(FLValue v);
FLValue type_of(FLValue v);
/* cgc-generated predicate aliases */
static inline FLValue fl_map_p(FLValue v)      { return fl_bool(v.tag == FL_MAP); }
static inline FLValue fl_array_p(FLValue v)    { return fl_bool(v.tag == FL_VECTOR); }
static inline FLValue fl_fn_p(FLValue v)       { return fl_bool(v.tag == FL_FN); }
static inline FLValue fl_number_p(FLValue v)   { return fl_bool(v.tag == FL_INT || v.tag == FL_FLOAT); }
static inline FLValue fl_boolean_p(FLValue v)  { return fl_bool(v.tag == FL_BOOL); }
static inline FLValue fl_integer_p(FLValue v)  { return fl_bool(v.tag == FL_INT); }
static inline FLValue fl_float_p(FLValue v)    { return fl_bool(v.tag == FL_FLOAT); }
static inline FLValue fl_empty_p(FLValue v)    {
    if (v.tag == FL_VECTOR) return fl_bool(((FLVector*)v.obj)->len == 0);
    if (v.tag == FL_MAP)    return fl_bool(((FLMap*)v.obj)->len == 0);
    if (v.tag == FL_STRING) return fl_bool(((FLString*)v.obj)->len == 0);
    return fl_bool(v.tag == FL_NIL);
}
static inline FLValue fl_not_empty_p(FLValue v)    { return fl_bool(!fl_truthy(fl_empty_p(v))); }
static inline FLValue fl_nil_or_empty_p(FLValue v) { return fl_bool(v.tag == FL_NIL || fl_truthy(fl_empty_p(v))); }
FLValue str_replace(FLValue s, FLValue from, FLValue to);
FLValue split(FLValue s, FLValue sep);
FLValue join(FLValue vec, FLValue sep);
FLValue range(FLValue n);  /* cgc: (range N) → 0..N-1 */
FLValue char_code_at(FLValue s, FLValue idx);
FLValue substring(FLValue s, FLValue start, FLValue end);
FLValue trim(FLValue s);
FLValue index_of(FLValue vec, FLValue val);
FLValue str_index_of(FLValue s, FLValue sub);

/* cgc-generated string/predicate aliases */
#define fl_str_starts_with str_starts_with
#define fl_str_ends_with   str_ends_with

/* ── S26: atom (mutable cell) ── */
FLValue fl_atom_new(FLValue init);
FLValue fl_atom_deref(FLValue atom);
FLValue fl_atom_reset(FLValue atom, FLValue val);

/* ── S26: stdlib bridge ── */
FLValue fl_includes_item(FLValue vec, FLValue item);
FLValue fl_str_includes(FLValue s, FLValue sub);
FLValue fl_string_p(FLValue v);
FLValue fl_vec_slice(FLValue vec, FLValue start, FLValue end);
FLValue fl_vec_last(FLValue vec);
FLValue fl_map_del(FLValue map, FLValue key);
FLValue fl_map_merge(FLValue a, FLValue b);
FLValue fl_concat(FLValue a, FLValue b);

/* ── S27: FL 소스 → AST (cgc-bridge.c + parser.c에서 제공) ── */
FLValue fl_parse(FLValue src);

/* ── JSON ── */
FLValue fl_json_parse(FLValue src);
FLValue fl_json_try_parse(FLValue src);
FLValue fl_json_stringify(FLValue val);
/* cgc-bin이 생성하는 짧은 이름 별칭 */
FLValue json_parse(FLValue src);
FLValue json_stringify(FLValue val);
FLValue json_try_parse(FLValue src);

/* ── 비트 연산 ── */
FLValue fl_bit_xor(FLValue a, FLValue b);
FLValue fl_bit_and(FLValue a, FLValue b);
FLValue fl_bit_or(FLValue a, FLValue b);
FLValue fl_bit_shl(FLValue a, FLValue b);
FLValue fl_bit_shr(FLValue a, FLValue b);

/* ── _fl_process_* ── */
FLValue _fl_process_getcwd(void);
FLValue _fl_process_chdir(FLValue path);
FLValue _fl_process_pid(void);
FLValue _fl_process_ppid(void);
FLValue _fl_process_kill(FLValue pid);
FLValue _fl_process_exists(FLValue pid);
FLValue _fl_process_exit(FLValue code);
FLValue _fl_process_wait(FLValue pid);
FLValue _fl_process_run(FLValue cmd);
FLValue _fl_process_run_args(FLValue cmd, FLValue args);
FLValue _fl_run_inherit(FLValue cmd);
FLValue _fl_process_exec(FLValue cmd);
FLValue _fl_process_exec_args(FLValue cmd, FLValue args);
FLValue _fl_process_spawn(FLValue cmd, FLValue args);
FLValue process_exit(FLValue code);

/* ── _fl_file_* / _fl_env_* / str_join ── */
FLValue _fl_file_append(FLValue path, FLValue content);
FLValue file_exists(FLValue path);
FLValue _fl_file_delete(FLValue path);
FLValue _fl_file_copy(FLValue src, FLValue dst);
FLValue _fl_file_rename(FLValue old, FLValue nw);
FLValue _fl_file_size(FLValue path);
FLValue _fl_file_modified(FLValue path);
FLValue _fl_file_mkdir(FLValue path);
FLValue _fl_file_rmdir(FLValue path);
FLValue _fl_file_list(FLValue path);
FLValue _fl_file_is_file(FLValue path);
FLValue _fl_file_is_dir(FLValue path);
FLValue _fl_env_get(FLValue key);
FLValue _fl_env_set(FLValue key, FLValue val);
FLValue _fl_env_all(void);
FLValue str_join(FLValue sep, FLValue vec);

/* ── HTTP 서버 (http.c) ── */
FLValue server_get(FLValue path, FLValue handler);
FLValue server_post(FLValue path, FLValue handler);
FLValue server_put(FLValue path, FLValue handler);
FLValue server_patch(FLValue path, FLValue handler);
FLValue server_delete(FLValue path, FLValue handler);
FLValue server_html(FLValue html);
FLValue server_file(FLValue path);
FLValue server_text(FLValue text);
FLValue server_json(FLValue json_str);
FLValue server_status(FLValue code, FLValue body);
FLValue server_redirect(FLValue url);
FLValue server_req_param(FLValue req, FLValue name);
FLValue server_req_query(FLValue req, FLValue name);
FLValue fxb_server_req_body(FLValue req);
FLValue server_req_header(FLValue req, FLValue name);
FLValue server_req_method(FLValue req);
FLValue server_req_path(FLValue req);
FLValue server_start(FLValue port);
/* kebab-case 앱 호환 alias */
FLValue fl_http_route(FLValue method, FLValue path, FLValue handler);
FLValue fl_http_start(FLValue port);
FLValue fl_http_get(FLValue url);   /* outbound HTTP 클라이언트 (경로 A PoC) */
/* kebab-case response alias */
FLValue fl_resp_html(FLValue html);
FLValue fl_resp_json(FLValue json);
FLValue fl_resp_text(FLValue text);
FLValue fl_resp_status(FLValue code, FLValue body);
FLValue fl_resp_redirect(FLValue url);

/* ── HTTP 서버 TLS (http.c) ── */
FLValue server_start_tls(FLValue port, FLValue cert, FLValue key);

/* ── WebSocket (websocket.c) ── */
FLValue server_ws(FLValue path, FLValue handler);
FLValue ws_recv(FLValue ws);
FLValue ws_send(FLValue ws, FLValue msg);
FLValue ws_close(FLValue ws);
FLValue ws_conn_id(FLValue ws);

/* ── sse.c — Server-Sent Events ── */
FLValue server_sse(FLValue path, FLValue handler);
FLValue sse_send(FLValue conn, FLValue data);
FLValue sse_close(FLValue conn);
FLValue sse_alive(FLValue conn);
int sse_is_sse_request(const char* raw);
int sse_handle_request(int fd, const char* raw, int raw_len);

/* ── aliases.c — cgc-bin 생성 함수 bridge ── */
/* 환경 */
FLValue env_get(FLValue key);
FLValue env_set(FLValue k, FLValue v);
/* 수학 */
FLValue math_floor(FLValue x);
FLValue math_ceil(FLValue x);
FLValue math_abs(FLValue x);
FLValue math_sqrt(FLValue x);
FLValue math_round(FLValue x);
FLValue math_max(FLValue a, FLValue b);
FLValue math_min(FLValue a, FLValue b);
FLValue math_pow(FLValue base, FLValue exp);
FLValue math_random(void);
/* 문자열 */
FLValue str_trim(FLValue s);
FLValue str_upper(FLValue s);
FLValue str_lower(FLValue s);
FLValue str_starts_with(FLValue s, FLValue prefix);
FLValue str_ends_with(FLValue s, FLValue suffix);
FLValue str_split(FLValue s, FLValue sep);
FLValue str_pad_left(FLValue s, FLValue width, FLValue ch);
FLValue str_pad_right(FLValue s, FLValue width, FLValue ch);
FLValue _impl_str_repeat(FLValue s, FLValue n);
FLValue str_repeat(FLValue s, FLValue n);
FLValue str_contains(FLValue s, FLValue sub);
FLValue str_includes(FLValue s, FLValue sub);
FLValue parse_int(FLValue s);
FLValue parse_float(FLValue s);
FLValue to_string(FLValue v);
FLValue to_int(FLValue v);
FLValue to_float(FLValue v);
FLValue str_to_num(FLValue v);
/* 컬렉션 */
FLValue first(FLValue vec);
FLValue last(FLValue vec);
FLValue rest(FLValue vec);
FLValue nth(FLValue vec, FLValue idx);
FLValue count(FLValue v);
FLValue take(FLValue n, FLValue vec);
FLValue drop(FLValue n, FLValue vec);
FLValue flatten(FLValue vec);
FLValue sort(FLValue vec);
FLValue reverse(FLValue vec);
FLValue zip(FLValue a, FLValue b);
FLValue some(FLValue fn, FLValue vec);
FLValue every(FLValue fn, FLValue vec);
FLValue vec_max(FLValue vec);
FLValue vec_min(FLValue vec);
/* 맵 */
FLValue merge(FLValue a, FLValue b);
FLValue dissoc(FLValue m, FLValue k);
FLValue vals(FLValue m);
FLValue keys(FLValue m);
FLValue assoc(FLValue m, FLValue k, FLValue v);
/* atom */
FLValue atom(FLValue v);
FLValue deref(FLValue a);
FLValue swap_bang(FLValue a, FLValue fn);
FLValue reset_bang(FLValue a, FLValue v);
/* 타입 */
/* 환경/.env */
FLValue env_load(FLValue path);
FLValue num(FLValue v);
FLValue url_decode(FLValue s);
FLValue form_parse(FLValue body);
FLValue str_slice(FLValue s, FLValue start, FLValue end);
/* fl_or: 93줄에 이미 선언됨 */
FLValue server_rate_limit(FLValue max_reqs, FLValue window_ms);
FLValue mariadb_connect4(FLValue host, FLValue user, FLValue pw, FLValue db);
FLValue int_p(FLValue v);
FLValue float_p(FLValue v);
FLValue bool_p(FLValue v);
FLValue nil_p(FLValue v);
FLValue ok_p(FLValue v);
FLValue set(FLValue m, FLValue key, FLValue val);
FLValue req_param(FLValue req, FLValue key);
FLValue req_header(FLValue req, FLValue key);
FLValue req_body(FLValue req);
FLValue number_p(FLValue v);
FLValue identity(FLValue v);
FLValue not_p(FLValue v);
FLValue even_p(FLValue v);
FLValue odd_p(FLValue v);
FLValue inc(FLValue v);
FLValue dec(FLValue v);
/* fl_vec_* aliases (cgc-bin 내부 이름) */
FLValue fl_vec_first(FLValue vec);
FLValue fl_vec_last(FLValue vec);
FLValue fl_vec_rest(FLValue vec);

/* ── SQLite (sqlite.c — libsqlite3 직접 링크) ── */
FLValue fxb_sqlite_open(FLValue path);
FLValue fxb_sqlite_query(FLValue conn, FLValue sql);
FLValue fxb_sqlite_exec(FLValue conn, FLValue sql);
FLValue fxb_sqlite_one(FLValue conn, FLValue sql);
/* 파라미터 바인딩 (SQL injection 방어) */
FLValue fxb_sqlite_query_p(FLValue conn, FLValue sql, FLValue params);
FLValue fxb_sqlite_exec_p(FLValue conn, FLValue sql, FLValue params);
FLValue fxb_sqlite_one_p(FLValue conn, FLValue sql, FLValue params);
FLValue fxb_sqlite_close(FLValue conn);
/* 73서버 cgc-bin 호환 alias (aliases.c에 구현) */
FLValue fl_db_open(FLValue path);
FLValue fl_db_query(FLValue db, FLValue sql, FLValue params);
FLValue fl_db_exec(FLValue db, FLValue sql, FLValue params);

/* ── MariaDB (mariadb.c — dlopen 방식, 헤더 불필요) ── */
FLValue mariadb_connect(FLValue host, FLValue port, FLValue user, FLValue pw, FLValue db);
FLValue mariadb_query(FLValue conn, FLValue sql);
FLValue mariadb_exec(FLValue conn, FLValue sql);
FLValue mariadb_one(FLValue conn, FLValue sql);
FLValue mariadb_close(FLValue conn);
FLValue mariadb_query_p(FLValue conn, FLValue sql, FLValue params);
FLValue mariadb_exec_p(FLValue conn, FLValue sql, FLValue params);
FLValue mariadb_one_p(FLValue conn, FLValue sql, FLValue params);

/* ── 메모리/GC (gc.c) ── */
void    fl_arena_begin(void);
void*   fl_arena_alloc(size_t size);
void    fl_arena_end(void);
void    fl_arena_stats(void);
void*   fl_perm_alloc(size_t size);
void    fl_perm_cleanup(void);
void    fl_runtime_shutdown(void);
FLValue fl_memory_stats(void);

/* ── 디버그 레이어 (debug.c) ── */
int     fl_debug_level(void);
void    fl_log(int level, const char* tag, const char* fmt, ...);
void    fl_log_error(const char* tag, const char* fmt, ...);
void    fl_log_request(const char* method, const char* path,
                       const char* body, size_t body_len,
                       const char* content_type);
void    fl_log_response(int status, const char* path, long elapsed_ms);
void    fl_log_sql(const char* driver, const char* sql);
FLValue fl_debug_dump(FLValue v);
void    fl_debug_banner(const char* app_name, int port);

/* ── try/catch 인프라 ── */
#define FL_TRY_MAX 64
typedef struct { jmp_buf buf; FLValue err; } FLTryFrame;
/* __thread: 각 워커 스레드가 독립된 try 스택을 가짐 */
extern __thread FLTryFrame fl_try_stack[FL_TRY_MAX];
extern __thread int fl_try_top;
void fl_throw(FLValue err);
FLValue fl_make_error(const char* type, const char* msg);
/* throw 시 FL 소스 라인 추적 (cgc-main이 설정, uncaught handler가 출력) */
extern int __fl_throw_line;
/* 콜스택 추적 */
void fl_push_frame(const char* fn);
void fl_push_frame_ln(const char* fn, int line);
void fl_pop_frame(void);
FLValue fl_assoc_in(FLValue m, FLValue keys, FLValue val);
FLValue fl_update_in(FLValue m, FLValue keys, FLValue fn);
FLValue fl_inspect(FLValue v);
FLValue fl_pp(FLValue v);
FLValue fl_format(FLValue fmt, FLValue args);
FLValue fl_merge(FLValue m1, FLValue m2);
FLValue fl_base64_encode(FLValue v);
FLValue fl_base64_decode(FLValue v);
FLValue fl_dissoc(FLValue m, FLValue key);
FLValue fl_keys(FLValue m);
FLValue fl_vals(FLValue m);
FLValue fl_select_keys(FLValue m, FLValue ks);
FLValue fl_update(FLValue m, FLValue key, FLValue fn);

/* ── 연산자 first-class 래퍼 (HOF 인자로 사용 가능, e.g. (map + list)) ── */
static inline FLValue __fl_op_add_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_add(a[0], a[1]); }
static inline FLValue __fl_op_sub_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_sub(a[0], a[1]); }
static inline FLValue __fl_op_mul_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_mul(a[0], a[1]); }
static inline FLValue __fl_op_div_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_div(a[0], a[1]); }
static inline FLValue __fl_op_mod_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_mod(a[0], a[1]); }
static inline FLValue __fl_op_eq_w (FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_eq (a[0], a[1]); }
static inline FLValue __fl_op_neq_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_neq(a[0], a[1]); }
static inline FLValue __fl_op_lt_w (FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_lt (a[0], a[1]); }
static inline FLValue __fl_op_gt_w (FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_gt (a[0], a[1]); }
static inline FLValue __fl_op_lte_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_lte(a[0], a[1]); }
static inline FLValue __fl_op_gte_w(FLClosure* _s, int _ac, FLValue* a) { (void)_s;(void)_ac; return fl_gte(a[0], a[1]); }

/* ── Raw TCP 소켓 (net.c) ── */
FLValue fxb_net_listen(FLValue port);
FLValue fxb_net_accept(FLValue server_fd);
FLValue fxb_net_readline(FLValue fd);
FLValue fxb_net_read_bytes(FLValue fd, FLValue n);
FLValue fxb_net_write(FLValue fd, FLValue str);
FLValue fxb_net_writeline(FLValue fd, FLValue str);
FLValue fxb_net_close(FLValue fd);
FLValue fxb_net_pasv_open(void);
FLValue fxb_net_pasv_accept(FLValue pasv_fd);
FLValue fxb_net_connect(FLValue host, FLValue port);
FLValue fxb_net_send_file(FLValue fd, FLValue path);
FLValue fxb_net_recv_file(FLValue fd, FLValue path);
FLValue fxb_net_local_ip(void);
FLValue fxb_net_listen_address(FLValue address, FLValue port);
FLValue fxb_net_set_nonblocking(FLValue fd, FLValue enabled);
FLValue fxb_net_wait(FLValue fd, FLValue events, FLValue timeout_ms);
FLValue fxb_net_accept_timeout(FLValue server_fd, FLValue timeout_ms);
FLValue fxb_net_connect_timeout(FLValue host, FLValue port, FLValue timeout_ms);
FLValue fxb_net_read_timeout(FLValue fd, FLValue max_bytes, FLValue timeout_ms);
FLValue fxb_net_write_timeout(FLValue fd, FLValue data, FLValue timeout_ms);
FLValue fxb_net_write_bytes(FLValue fd, FLValue data, FLValue length);
FLValue fxb_net_write_bytes_timeout(FLValue fd, FLValue data, FLValue length, FLValue timeout_ms);
FLValue fxb_net_set_keepalive(FLValue fd, FLValue enabled, FLValue idle, FLValue interval, FLValue count);
FLValue fxb_net_track_connection(FLValue fd);
FLValue fxb_net_untrack_connection(FLValue fd);
FLValue fxb_net_listener_shutdown(FLValue fd, FLValue timeout_ms);


#endif /* FREELANG_RUNTIME_H */

/* ── 정규식 (aliases.c) ── */
FLValue str_match(FLValue str_v, FLValue pat_v);
FLValue str_match_all(FLValue str_v, FLValue pat_v);
FLValue str_replace_re(FLValue str_v, FLValue pat_v, FLValue rep_v);
FLValue str_replace_all_re(FLValue str_v, FLValue pat_v, FLValue rep_v);
FLValue str_test(FLValue str_v, FLValue pat_v);

/* ── future (aliases.c) ── */
FLValue fl_future(FLValue fn);
FLValue fl_deref(FLValue handle);
FLValue fl_future_done(FLValue handle);
/* 날짜/시간 */
FLValue date_format(FLValue ts, FLValue fmt);
FLValue date_now_str(void);
FLValue date_parse(FLValue str_v, FLValue fmt_v);
FLValue date_add(FLValue ts, FLValue secs);
FLValue date_diff(FLValue ts1, FLValue ts2);
/* UUID */
FLValue uuid4(void);
/* 컬렉션 유틸 */
FLValue fl_sum(FLValue vec);
FLValue fl_average(FLValue vec);
FLValue fl_distinct(FLValue vec);
FLValue fl_max_by(FLValue fn, FLValue vec);
FLValue fl_min_by(FLValue fn, FLValue vec);
FLValue fl_partition(FLValue n, FLValue vec);
FLValue fl_index_where(FLValue fn, FLValue vec);
FLValue fl_zip_map(FLValue keys, FLValue vals);
/* 문자열 유틸 */
FLValue str_char_at(FLValue s, FLValue idx);
FLValue str_length(FLValue s);
FLValue _impl_str_reverse(FLValue s);
FLValue str_reverse(FLValue s);
FLValue num_to_str(FLValue n);
FLValue pad_zero(FLValue n, FLValue width);
/* 수학 유틸 */
FLValue fl_clamp(FLValue val, FLValue lo, FLValue hi);

/* ── fx builtin shim ── */
extern FLValue sqlite_open, sqlite_query, sqlite_exec, sqlite_one;
extern FLValue sqlite_query_p, sqlite_exec_p, sqlite_one_p, sqlite_close;
extern FLValue server_req_body;
/* throw: old cgc-bin generates fl_fn_call(throw,...) — shim maps to fl_throw() */
/* In C (not C++), 'throw' is a valid identifier */
extern FLValue throw;
/* apply, max, str: cgc-bin generates fl_fn_call(...) for first-class usage */
extern FLValue apply;
extern FLValue max;
extern FLValue str;

/* fx 독립 언어 — 투명 alias 선언 */
FLValue sqlite_open_v2(FLValue path);
FLValue sqlite_query_v2(FLValue db, FLValue sql);
FLValue sqlite_exec_v2(FLValue db, FLValue sql);
FLValue sqlite_one_v2(FLValue db, FLValue sql);
FLValue sqlite_query_p_v2(FLValue db, FLValue sql, FLValue p);
FLValue sqlite_exec_p_v2(FLValue db, FLValue sql, FLValue p);
FLValue sqlite_close_v2(FLValue db);
FLValue fx_server_json(FLValue data);
FLValue fx_respond(FLValue status, FLValue data);

/* ── HTTP 클라이언트 (http_client.c) ── */
FLValue http_get(FLValue url);
FLValue http_get_h(FLValue url, FLValue headers);
FLValue http_post(FLValue url, FLValue body);
FLValue http_post_h(FLValue url, FLValue body, FLValue headers);
FLValue http_put(FLValue url, FLValue body);
FLValue http_del(FLValue url);
FLValue http_patch(FLValue url, FLValue body);
FLValue http_req(FLValue method, FLValue url, FLValue body, FLValue headers);
FLValue http_body(FLValue res);
FLValue http_status(FLValue res);
FLValue http_ok_p(FLValue res);

/* ── stdlib 갭 함수 (aliases.c) ── */
FLValue uuid(void);
FLValue fl_random(void);
FLValue max_by(FLValue fn, FLValue vec);
FLValue min_by(FLValue fn, FLValue vec);
FLValue clamp(FLValue val, FLValue lo, FLValue hi);
FLValue file_exists_p(FLValue path);
FLValue sha256(FLValue s);
FLValue md5(FLValue s);
FLValue json_pretty(FLValue v);

/* ── 정규식 (regex.c) ── */
FLValue regex_match(FLValue pattern, FLValue str);
FLValue regex_find(FLValue pattern, FLValue str);
FLValue regex_find_all(FLValue pattern, FLValue str);
FLValue regex_groups(FLValue pattern, FLValue str);
FLValue regex_replace(FLValue pattern, FLValue str, FLValue replacement);
FLValue regex_replace_all(FLValue pattern, FLValue str, FLValue replacement);
FLValue regex_split(FLValue pattern, FLValue str);

/* SMTP */
FLValue smtp_send(FLValue from, FLValue to, FLValue subject, FLValue body);
FLValue smtp_send_html(FLValue from, FLValue to, FLValue subject, FLValue html);
FLValue smtp_test(FLValue to);

/* 쉘 실행 */
FLValue shell_run(FLValue cmd);
FLValue shell_exec(FLValue cmd);

/* PDF TTF 폰트 임베딩 */
FLValue pdf_ttf_load(FLValue path);
FLValue pdf_ttf_glyphs(FLValue text);
FLValue pdf_ttf_embed_objs(FLValue base_id);
FLValue pdf_ttf_size(void);

/* PDF 이미지 임베딩 */
FLValue fl_map_new(void);
FLValue fl_map_get(FLValue map, FLValue key);
FLValue fl_map_set(FLValue map, FLValue key, FLValue val);
FLValue pdf_img_load(FLValue path);
FLValue pdf_img_obj(FLValue img, FLValue obj_id);

/* FL:EXTERN_SECTION:BEGIN */
extern FLValue str_lines;
extern FLValue str_count;
extern FLValue math_lerp;
extern FLValue math_round_n;
extern FLValue math_sign;
extern FLValue path_dirname;
extern FLValue str_indent;
extern FLValue str_truncate;
extern FLValue str_rpad;
extern FLValue math_clamp;
extern FLValue time_now_ms;
extern FLValue time_elapsed;
extern FLValue path_basename;
/* FL:EXTERN_SECTION:END */
