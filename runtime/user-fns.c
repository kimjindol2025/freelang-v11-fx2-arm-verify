/**
 * freelang-v11-fx2 — 사용자 패키지 함수
 * 이 파일은 fl-source-manager가 자동 관리합니다.
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

/* FL:FN:math-lerp */
FLValue ufl_math_lerp(FLValue a, FLValue b, FLValue t) {
    double da = (a.tag == FL_INT) ? (double)a.i : a.f;
    double db = (b.tag == FL_INT) ? (double)b.i : b.f;
    double dt = (t.tag == FL_INT) ? (double)t.i : t.f;
    return fl_float(da + (db - da) * dt);
}
/* FL:FN_END */

/* FL:FN:math-round-n */
#include <math.h>
FLValue ufl_math_round_n(FLValue v, FLValue n) {
    double dv = (v.tag == FL_INT) ? (double)v.i : v.f;
    int64_t places = (n.tag == FL_INT) ? n.i : 0;
    if (places < 0) places = 0;
    double factor = pow(10.0, (double)places);
    return fl_float(round(dv * factor) / factor);
}
/* FL:FN_END */

/* FL:FN:math-sign */
FLValue ufl_math_sign(FLValue v) {
    double d = (v.tag == FL_INT) ? (double)v.i : v.f;
    if (d > 0) return fl_int(1);
    if (d < 0) return fl_int(-1);
    return fl_int(0);
}
/* FL:FN_END */

/* FL:FN:path-dirname */
FLValue ufl_path_dirname(FLValue p) {
    if (p.tag != FL_STRING) return p;
    const char* src = ((FLString*)p.obj)->data;
    const char* last = strrchr(src, '/');
    if (!last) return fl_str_val(".");
    size_t len = last - src;
    if (len == 0) return fl_str_val("/");
    char* buf = (char*)fl_arena_alloc(len + 1);
    memcpy(buf, src, len);
    buf[len] = 0;
    return fl_str_val(buf);
}
/* FL:FN_END */

/* FL:FN:str-lines */
FLValue ufl_str_lines(FLValue s) {
    if (s.tag != FL_STRING) return fl_vec_new();
    const char* src = ((FLString*)s.obj)->data;
    FLValue builder = fl_vec_builder_new();
    const char* start = src;
    for (const char* p = src; ; p++) {
        if (*p == '\n' || *p == '\0') {
            size_t len = p - start;
            if (len > 0) {
                char* buf = (char*)fl_arena_alloc(len + 1);
                memcpy(buf, start, len);
                buf[len] = 0;
                fl_vec_builder_push(builder, fl_str_val(buf));
            }
            if (*p == '\0') break;
            start = p + 1;
        }
    }
    return fl_vec_builder_freeze(builder);
}
/* FL:FN_END */

/* FL:FN:str-count */
FLValue ufl_str_count(FLValue s, FLValue sub) {
    if (s.tag != FL_STRING || sub.tag != FL_STRING) return fl_int(0);
    const char* haystack = ((FLString*)s.obj)->data;
    const char* needle   = ((FLString*)sub.obj)->data;
    size_t nlen = strlen(needle);
    if (nlen == 0) return fl_int(0);
    int64_t count = 0;
    const char* p = haystack;
    while ((p = strstr(p, needle)) != NULL) { count++; p += nlen; }
    return fl_int(count);
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

/* FL:FN:str-rpad */
FLValue ufl_str_rpad(FLValue s, FLValue n, FLValue ch) {
    if (s.tag != FL_STRING || n.tag != FL_INT) return s;
    const char* src = ((FLString*)s.obj)->data;
    size_t slen = strlen(src);
    int64_t total = n.i < 0 ? 0 : n.i;
    if ((int64_t)slen >= total) return s;
    char pad = (ch.tag == FL_STRING && ((FLString*)ch.obj)->data[0]) ? ((FLString*)ch.obj)->data[0] : 32;
    char* buf = (char*)fl_arena_alloc(total + 1);
    if (!buf) return s;
    memcpy(buf, src, slen);
    memset(buf + slen, pad, total - slen);
    buf[total] = 0;
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

/* FL:FN:time-now-ms */
#include <time.h>
FLValue ufl_time_now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return fl_int((int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
/* FL:FN_END */

/* FL:FN:time-elapsed */
#include <time.h>
FLValue ufl_time_elapsed(FLValue start) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int64_t now = (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    int64_t s   = (start.tag == FL_INT) ? start.i : (int64_t)start.f;
    return fl_int(now - s);
}
/* FL:FN_END */

/* FL:FN:path-basename */
FLValue ufl_path_basename(FLValue p) {
    if (p.tag != FL_STRING) return p;
    const char* src = ((FLString*)p.obj)->data;
    const char* last = strrchr(src, '/');
    const char* result = last ? last + 1 : src;
    return fl_str_val(result);
}
/* FL:FN_END */
/* FL:USER_SECTION:END */
