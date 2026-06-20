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
/* throw shim: old cgc-bin generates fl_fn_call(throw,...) for (throw expr) */
/* In C (not C++), 'throw' is a valid identifier. */
FLValue throw;

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
    throw           = fl_fn_new(w_throw,           0, NULL);
}
