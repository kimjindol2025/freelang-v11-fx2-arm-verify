/* builtins-shim.c — 253 서버 cgc-bin: 직접 C 호출로 생성되는 빌트인 래퍼
 *
 * 253 서버 x86_64 cgc-bin은 str-repeat/str-reverse를 직접 C 함수 호출로 생성한다.
 *   str_repeat(s, n)   — (str-repeat s n)
 *   str_reverse(s)     — (str-reverse s)
 *   apply → fl_apply   — cgc-bin이 직접 fl_apply로 생성 (충돌 없음)
 *   max   → fl_max2    — cgc-bin이 직접 fl_max2로 생성 (충돌 없음)
 *   str   → fl_str_n   — cgc-bin이 직접 fl_str_n으로 생성 (충돌 없음)
 */
#include "runtime.h"

/* ── C 함수 래퍼 (직접 호출 경로) ─────────────────────── */
FLValue str_repeat(FLValue s, FLValue n) {
    return _impl_str_repeat(s, n);
}

FLValue str_reverse(FLValue s) {
    return _impl_str_reverse(s);
}
