/* builtins-shim.c — Callable ABI: cgc-bin fl_fn_call 대상 빌트인 FLValue 글로벌
 *
 * cgc-bin은 일부 빌트인을 fl_fn_call(name, ...) 형태로 생성한다.
 * fl_fn_call 첫 인자는 FLValue여야 하므로 각 구현을 FLValue 글로벌로 노출한다.
 *
 * 커버 대상 (cgc-bin 실측):
 *   str_repeat  — (str-repeat s n)
 *   str_reverse — (str-reverse s)
 *   apply       — (apply fn args)
 *   max         — (apply max ...) 시 first-class 참조
 *   str         — (apply str ...) 시 first-class 참조
 */
#include "runtime.h"

/* ── FLValue 글로벌 선언 ─────────────────────────────── */
FLValue str_repeat;
FLValue str_reverse;
FLValue apply;
FLValue max;
FLValue str;

/* ── 래퍼 ───────────────────────────────────────────── */
static FLValue w_str_repeat(FLClosure* cl, int ac, FLValue* a) {
    (void)cl; (void)ac;
    return _impl_str_repeat(a[0], a[1]);
}
static FLValue w_str_reverse(FLClosure* cl, int ac, FLValue* a) {
    (void)cl; (void)ac;
    return _impl_str_reverse(a[0]);
}
static FLValue w_apply(FLClosure* cl, int ac, FLValue* a) {
    (void)cl; (void)ac;
    return fl_apply(a[0], a[1]);
}
/* variadic max: fl_gt로 순회 비교 */
static FLValue w_max(FLClosure* cl, int ac, FLValue* a) {
    (void)cl;
    if (ac == 0) return fl_nil();
    FLValue best = a[0];
    for (int i = 1; i < ac; i++) {
        if (fl_truthy(fl_gt(a[i], best))) best = a[i];
    }
    return best;
}
/* variadic str: to_string 후 fl_concat 순회 */
static FLValue w_str(FLClosure* cl, int ac, FLValue* a) {
    (void)cl;
    if (ac == 0) return fl_str_val("");
    FLValue acc = to_string(a[0]);
    for (int i = 1; i < ac; i++) acc = fl_concat(acc, to_string(a[i]));
    return acc;
}

/* ── constructor 초기화 ──────────────────────────────── */
__attribute__((constructor))
static void builtins_shim_init(void) {
    str_repeat  = fl_make_native_fn(w_str_repeat,  "str_repeat");
    str_reverse = fl_make_native_fn(w_str_reverse, "str_reverse");
    apply       = fl_make_native_fn(w_apply,       "apply");
    max         = fl_make_native_fn(w_max,         "max");
    str         = fl_make_native_fn(w_str,         "str");
}
