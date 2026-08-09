/* ============================================================
 * fx-builtin-shim.c
 *
 * 설치본 cgc-bin(param/map 정상)은 sqlite_*·server_req_body 를
 * 빌트인으로 모른다 → 코드젠이 `fl_fn_call(sqlite_open, ...)` 형태로
 * 생성한다. 그 첫 인자는 FLValue(클로저)여야 하므로, 동명의 전역
 * 클로저를 만들어 실제 구현 fxb_*(sqlite.c/http.c)로 디스패치한다.
 *
 * constructor 로 초기화: arena 미초기화(g_arena=NULL) 시 fl_fn_new 가
 * malloc 폴백으로 영속 객체를 만들므로 전역 보관에 안전.
 * ============================================================ */
#include "runtime.h"

/* 전역 클로저 실체 (runtime.h 의 extern 선언과 1:1) */
FLValue sqlite_open, sqlite_query, sqlite_exec, sqlite_one;
FLValue sqlite_query_p, sqlite_exec_p, sqlite_one_p, sqlite_close;
FLValue server_req_body;
FLValue math_clamp;
FLValue str_lines;
FLValue math_round_n;
FLValue math_sign;
FLValue str_indent;
FLValue str_truncate;
FLValue math_lerp;
FLValue str_count;
FLValue path_dirname;
FLValue time_now_ms;
FLValue time_elapsed;
FLValue path_basename;
FLValue str_rpad;
/* throw shim: old cgc-bin generates fl_fn_call(throw,...) for (throw expr) */
/* In C (not C++), 'throw' is a valid identifier. */
FLValue throw;

/* math-clamp builtin is implemented in user-fns.c as ufl_math_clamp. */
FLValue ufl_math_clamp(FLValue value, FLValue min, FLValue max);
FLValue ufl_str_lines(FLValue s);
FLValue ufl_str_count(FLValue s, FLValue sub);
FLValue ufl_math_lerp(FLValue a, FLValue b, FLValue t);
FLValue ufl_path_dirname(FLValue p);
FLValue ufl_math_round_n(FLValue v, FLValue n);
FLValue ufl_math_sign(FLValue v);
FLValue ufl_str_indent(FLValue s, FLValue n);
FLValue ufl_str_truncate(FLValue s, FLValue n, FLValue suffix);
FLValue ufl_time_now_ms(void);
FLValue ufl_time_elapsed(FLValue start);
FLValue ufl_path_basename(FLValue p);
FLValue ufl_str_rpad(FLValue s, FLValue n, FLValue ch);

/* arity 별 디스패치 래퍼 */
static FLValue w_sqlite_open   (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_open(a[0]);}
static FLValue w_sqlite_query  (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_query(a[0],a[1]);}
static FLValue w_sqlite_exec   (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_exec(a[0],a[1]);}
static FLValue w_sqlite_one    (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_one(a[0],a[1]);}
static FLValue w_sqlite_query_p(FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_query_p(a[0],a[1],a[2]);}
static FLValue w_sqlite_exec_p (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_exec_p(a[0],a[1],a[2]);}
static FLValue w_sqlite_one_p  (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_one_p(a[0],a[1],a[2]);}
static FLValue w_sqlite_close  (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_sqlite_close(a[0]);}
static FLValue w_server_req_body(FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return fxb_server_req_body(a[0]);}
static FLValue w_math_clamp     (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_math_clamp(a[0], a[1], a[2]);}
static FLValue w_str_lines     (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_str_lines(a[0]);}
static FLValue w_str_count     (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_str_count(a[0], a[1]);}
static FLValue w_math_lerp      (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_math_lerp(a[0], a[1], a[2]);}
static FLValue w_path_dirname (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_path_dirname(a[0]);}
static FLValue w_math_round_n  (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_math_round_n(a[0], a[1]);}
static FLValue w_math_sign    (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_math_sign(a[0]);}
static FLValue w_str_indent   (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_str_indent(a[0], a[1]);}
static FLValue w_str_truncate (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_str_truncate(a[0], a[1], a[2]);}
static FLValue w_time_now_ms     (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_time_now_ms();}
static FLValue w_time_elapsed    (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_time_elapsed(a[0]);}
static FLValue w_path_basename   (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_path_basename(a[0]);}
static FLValue w_str_rpad       (FLClosure* s,int ac,FLValue* a){(void)s;(void)ac;return ufl_str_rpad(a[0], a[1], a[2]);}
static FLValue w_throw         (FLClosure* s,int ac,FLValue* a){(void)s; fl_throw(ac>0?a[0]:fl_nil()); return fl_nil();}

__attribute__((constructor))
static void fx_builtin_shim_init(void) {
    sqlite_open     = fl_fn_new(w_sqlite_open,    0, NULL);
    sqlite_query    = fl_fn_new(w_sqlite_query,   0, NULL);
    sqlite_exec     = fl_fn_new(w_sqlite_exec,    0, NULL);
    sqlite_one      = fl_fn_new(w_sqlite_one,     0, NULL);
    sqlite_query_p  = fl_fn_new(w_sqlite_query_p, 0, NULL);
    sqlite_exec_p   = fl_fn_new(w_sqlite_exec_p,  0, NULL);
    sqlite_one_p    = fl_fn_new(w_sqlite_one_p,   0, NULL);
    sqlite_close    = fl_fn_new(w_sqlite_close,   0, NULL);
    server_req_body = fl_fn_new(w_server_req_body,0, NULL);
    math_clamp      = fl_fn_new(w_math_clamp,      0, NULL);
    math_lerp       = fl_fn_new(w_math_lerp,       0, NULL);
    str_count       = fl_fn_new(w_str_count,       0, NULL);
    path_dirname    = fl_fn_new(w_path_dirname,    0, NULL);
    math_round_n   = fl_fn_new(w_math_round_n,   0, NULL);
    math_sign      = fl_fn_new(w_math_sign,      0, NULL);
    str_indent     = fl_fn_new(w_str_indent,     0, NULL);
    str_truncate   = fl_fn_new(w_str_truncate,   0, NULL);
    str_lines       = fl_fn_new(w_str_lines,      0, NULL);
    time_now_ms    = fl_fn_new(w_time_now_ms,    0, NULL);
    time_elapsed   = fl_fn_new(w_time_elapsed,   0, NULL);
    path_basename  = fl_fn_new(w_path_basename,  0, NULL);
    str_rpad       = fl_fn_new(w_str_rpad,       0, NULL);
    throw           = fl_fn_new(w_throw,           0, NULL);
}
