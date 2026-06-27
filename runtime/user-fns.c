/**
 * freelang-v11-fx2 — 사용자 패키지 함수
 * fl-pkg-gen-c.py가 installed.json에서 자동 생성합니다.
 * 직접 수정하지 마세요.
 */
#include "runtime.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


/* FL:USER_SECTION:BEGIN */
/* FL:FN:str-indent */
FLValue ufl_str_indent(FLValue s, FLValue n) {
    if (s.tag != FL_STRING || n.tag != FL_INT) return s;
    int64_t cnt = n.i < 0 ? 0 : n.i;
    const char* src = ((FLString*)s.obj)->data;
    size_t slen = strlen(src);
    char* buf = (char*)fl_arena_alloc(cnt + slen + 1);
    if (!buf) return s;
    memset(buf, 32, cnt);
    memcpy(buf + cnt, src, slen);
    buf[cnt + slen] = 0;
    return fl_str_val(buf);
}
/* FL:FN_END */

/* FL:FN:str-truncate */
FLValue ufl_str_truncate(FLValue s, FLValue n, FLValue suffix) {
    if (s.tag != FL_STRING || n.tag != FL_INT) return s;
    const char* src = ((FLString*)s.obj)->data;
    int64_t maxlen = n.i < 0 ? 0 : n.i;
    size_t slen = strlen(src);
    if ((int64_t)slen <= maxlen) return s;
    const char* suf = (suffix.tag == FL_STRING) ? ((FLString*)suffix.obj)->data : "...";
    size_t suflen = strlen(suf);
    int64_t cutlen = maxlen - (int64_t)suflen;
    if (cutlen < 0) cutlen = 0;
    char* buf = (char*)fl_arena_alloc(cutlen + suflen + 1);
    if (!buf) return s;
    memcpy(buf, src, cutlen);
    memcpy(buf + cutlen, suf, suflen);
    buf[cutlen + suflen] = 0;
    return fl_str_val(buf);
}
/* FL:FN_END */

/* FL:FN:math-clamp */
FLValue ufl_math_clamp(FLValue v, FLValue lo, FLValue hi) {
    double val = (v.tag == FL_INT) ? (double)v.i : v.f;
    double low = (lo.tag == FL_INT) ? (double)lo.i : lo.f;
    double high = (hi.tag == FL_INT) ? (double)hi.i : hi.f;
    if (val < low) val = low;
    if (val > high) val = high;
    return fl_float(val);
}
/* FL:FN_END */

/* FL:USER_SECTION:END */

/* FL:SHIM_SECTION:BEGIN */
/* FL:SHIM_FN:str-indent */
FLValue str_indent(FLValue a0, FLValue a1) { return ufl_str_indent(a0, a1); }
/* FL:SHIM_FN_END */
/* FL:SHIM_FN:str-truncate */
FLValue str_truncate(FLValue a0, FLValue a1, FLValue a2) { return ufl_str_truncate(a0, a1, a2); }
/* FL:SHIM_FN_END */
/* FL:SHIM_FN:math-clamp */
FLValue math_clamp(FLValue a0, FLValue a1, FLValue a2) { return ufl_math_clamp(a0, a1, a2); }
/* FL:SHIM_FN_END */
/* FL:SHIM_SECTION:END */
