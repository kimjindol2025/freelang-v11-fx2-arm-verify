/*
 * aliases.c — cgc-bin이 생성하는 함수명 ↔ 런타임 구현 매핑
 *
 * cgc-bin은 FreeLang stdlib 함수를 특정 C 이름으로 emit.
 * 런타임 구현은 fl_ 접두사를 쓰므로 여기서 bridge.
 */

#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>

/* ── 내부 헬퍼: fn에 인자 1개 전달 ── */
static FLValue fl_apply_fn(FLValue fn, FLValue arg) {
    return fl_fn_call(fn, 1, &arg);
}

/* ── 환경 ── */
FLValue env_get(FLValue key)        { return _fl_env_get(key); }
FLValue env_set(FLValue k, FLValue v) { return _fl_env_set(k, v); }

/* ── 수학 ── */
FLValue math_floor(FLValue x)  { return fl_floor(x); }
FLValue math_ceil(FLValue x)   { return fl_ceil(x); }
FLValue math_abs(FLValue x)    { return fl_abs(x); }
FLValue math_sqrt(FLValue x)   { return fl_math_sqrt(x); }

FLValue math_round(FLValue x) {
    if (x.tag == FL_INT)   return x;
    if (x.tag == FL_FLOAT) return fl_int((int64_t)round(x.f));
    return fl_nil();
}
FLValue math_max(FLValue a, FLValue b) {
    double va = (a.tag == FL_INT) ? (double)a.i : a.f;
    double vb = (b.tag == FL_INT) ? (double)b.i : b.f;
    return (va >= vb) ? a : b;
}
FLValue math_min(FLValue a, FLValue b) {
    double va = (a.tag == FL_INT) ? (double)a.i : a.f;
    double vb = (b.tag == FL_INT) ? (double)b.i : b.f;
    return (va <= vb) ? a : b;
}
FLValue math_pow(FLValue base, FLValue exp) {
    double b = (base.tag == FL_INT) ? (double)base.i : base.f;
    double e = (exp.tag  == FL_INT) ? (double)exp.i  : exp.f;
    return fl_float(pow(b, e));
}
FLValue math_random(void) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = 1; }
    return fl_float((double)rand() / ((double)RAND_MAX + 1.0));
}

/* ── 문자열 ── */

/* 공통: FL_STRING인지 확인 */
static inline const char* fl_cstr(FLValue v) {
    if (v.tag != FL_STRING) return "";
    return ((FLString*)v.obj)->data;
}

FLValue str_trim(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    while (*p && isspace((unsigned char)*p)) p++;
    size_t len = strlen(p);
    while (len > 0 && isspace((unsigned char)p[len-1])) len--;
    char* buf = (char*)malloc(len + 1);
    memcpy(buf, p, len); buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_upper(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    size_t len = strlen(p);
    char* buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)toupper((unsigned char)p[i]);
    buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_lower(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* p = fl_cstr(s);
    size_t len = strlen(p);
    char* buf = (char*)malloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)tolower((unsigned char)p[i]);
    buf[len] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_starts_with(FLValue s, FLValue prefix) {
    const char* sp = fl_cstr(s);
    const char* pp = fl_cstr(prefix);
    return fl_bool(strncmp(sp, pp, strlen(pp)) == 0);
}

FLValue str_ends_with(FLValue s, FLValue suffix) {
    const char* sp = fl_cstr(s);
    const char* su = fl_cstr(suffix);
    size_t slen = strlen(sp), sulen = strlen(su);
    if (sulen > slen) return fl_bool(false);
    return fl_bool(strcmp(sp + slen - sulen, su) == 0);
}

FLValue str_split(FLValue s, FLValue sep) {
    /* alias — split already exists in runtime.h (cgc-bridge.c) */
    return split(s, sep);
}

FLValue str_pad_left(FLValue s, FLValue width_v, FLValue ch_v) {
    const char* sp  = fl_cstr(s);
    int width = (width_v.tag == FL_INT) ? (int)width_v.i : 0;
    const char* ch  = fl_cstr(ch_v);
    char pad = (ch && ch[0]) ? ch[0] : ' ';
    int slen = (int)strlen(sp);
    if (slen >= width) return s;
    int pads = width - slen;
    char* buf = (char*)malloc((size_t)(width + 1));
    for (int i = 0; i < pads; i++) buf[i] = pad;
    memcpy(buf + pads, sp, (size_t)slen + 1);
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_pad_right(FLValue s, FLValue width_v, FLValue ch_v) {
    const char* sp  = fl_cstr(s);
    int width = (width_v.tag == FL_INT) ? (int)width_v.i : 0;
    const char* ch  = fl_cstr(ch_v);
    char pad = (ch && ch[0]) ? ch[0] : ' ';
    int slen = (int)strlen(sp);
    if (slen >= width) return s;
    int pads = width - slen;
    char* buf = (char*)malloc((size_t)(width + 1));
    memcpy(buf, sp, (size_t)slen);
    for (int i = 0; i < pads; i++) buf[slen + i] = pad;
    buf[width] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue _impl_str_repeat(FLValue s, FLValue n_v) {
    const char* sp = fl_cstr(s);
    int n = (n_v.tag == FL_INT) ? (int)n_v.i : 0;
    size_t slen = strlen(sp);
    if (n <= 0 || slen == 0) return fl_str_val("");
    size_t total = slen * (size_t)n;
    char* buf = (char*)malloc(total + 1);
    for (int i = 0; i < n; i++) memcpy(buf + i * slen, sp, slen);
    buf[total] = '\0';
    FLValue r = fl_str_val(buf); free(buf);
    return r;
}

FLValue str_contains(FLValue s, FLValue sub) { return fl_str_includes(s, sub); }
FLValue str_includes(FLValue s, FLValue sub) { return fl_str_includes(s, sub); }

/* parse number */
FLValue parse_int(FLValue s) {
    const char* p = fl_cstr(s);
    return fl_int((int64_t)strtoll(p, NULL, 10));
}
FLValue parse_float(FLValue s) {
    const char* p = fl_cstr(s);
    return fl_float(strtod(p, NULL));
}
FLValue to_string(FLValue v) {
    char buf[64];
    if (v.tag == FL_INT)    { snprintf(buf, sizeof(buf), "%lld", (long long)v.i); return fl_str_val(buf); }
    if (v.tag == FL_FLOAT)  { snprintf(buf, sizeof(buf), "%g", v.f); return fl_str_val(buf); }
    if (v.tag == FL_BOOL)   return fl_str_val(v.b ? "true" : "false");
    if (v.tag == FL_NIL)    return fl_str_val("nil");
    if (v.tag == FL_STRING) return v;
    return fl_json_stringify(v);
}
FLValue to_int(FLValue v) {
    if (v.tag == FL_INT)   return v;
    if (v.tag == FL_FLOAT) return fl_int((int64_t)v.f);
    if (v.tag == FL_STRING) return parse_int(v);
    return fl_nil();
}
FLValue to_float(FLValue v) {
    if (v.tag == FL_FLOAT) return v;
    if (v.tag == FL_INT)   return fl_float((double)v.i);
    if (v.tag == FL_STRING) return parse_float(v);
    return fl_nil();
}
/* str_to_num alias (FreeLang v11 stdlib) */
FLValue str_to_num(FLValue v) { return parse_float(v); }

/* ── 컬렉션 ── */

/* first / last / rest / nth */
FLValue first(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    return v->data[0];
}
FLValue last(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    return v->data[v->len - 1];
}
FLValue rest(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len <= 1) return fl_vec_new();
    return fl_vec_slice(vec, fl_int(1), fl_int((int64_t)vp->len));
}
FLValue nth(FLValue vec, FLValue idx) { return fl_vec_get(vec, idx); }
FLValue count(FLValue v)              { return length(v); }

/* take / drop */
FLValue take(FLValue n_v, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    if (n < 0) n = 0;
    if (n > (int64_t)vp->len) n = (int64_t)vp->len;
    return fl_vec_slice(vec, fl_int(0), fl_int(n));
}
FLValue drop(FLValue n_v, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    if (n < 0) n = 0;
    if (n > (int64_t)vp->len) n = (int64_t)vp->len;
    return fl_vec_slice(vec, fl_int(n), fl_int((int64_t)vp->len));
}

/* flatten (1단계) */
FLValue flatten(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue item = vp->data[i];
        if (item.tag == FL_VECTOR) {
            FLVector* inner = (FLVector*)item.obj;
            for (uint32_t j = 0; j < inner->len; j++)
                result = fl_vec_push(result, inner->data[j]);
        } else {
            result = fl_vec_push(result, item);
        }
    }
    return result;
}

/* sort (숫자/문자열 오름차순) */
static int fl_compare(const void* a, const void* b) {
    const FLValue* va = (const FLValue*)a;
    const FLValue* vb = (const FLValue*)b;
    if (va->tag == FL_INT && vb->tag == FL_INT)
        return (va->i < vb->i) ? -1 : (va->i > vb->i) ? 1 : 0;
    if ((va->tag == FL_FLOAT || va->tag == FL_INT) &&
        (vb->tag == FL_FLOAT || vb->tag == FL_INT)) {
        double da = va->tag == FL_INT ? (double)va->i : va->f;
        double db = vb->tag == FL_INT ? (double)vb->i : vb->f;
        return (da < db) ? -1 : (da > db) ? 1 : 0;
    }
    if (va->tag == FL_STRING && vb->tag == FL_STRING)
        return strcmp(((FLString*)va->obj)->data, ((FLString*)vb->obj)->data);
    return 0;
}
FLValue sort(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    memcpy(copy, vp->data, vp->len * sizeof(FLValue));
    qsort(copy, vp->len, sizeof(FLValue), fl_compare);
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* reverse */
FLValue reverse(FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    for (uint32_t i = 0; i < vp->len; i++)
        copy[i] = vp->data[vp->len - 1 - i];
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* zip / uniq */
FLValue zip(FLValue a, FLValue b) {
    if (a.tag != FL_VECTOR || b.tag != FL_VECTOR) return fl_vec_new();
    FLVector* va = (FLVector*)a.obj;
    FLVector* vb = (FLVector*)b.obj;
    uint32_t n = va->len < vb->len ? va->len : vb->len;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < n; i++) {
        FLValue pair = fl_vec_new();
        pair = fl_vec_push(pair, va->data[i]);
        pair = fl_vec_push(pair, vb->data[i]);
        result = fl_vec_push(result, pair);
    }
    return result;
}

/* map 관련 */
FLValue merge(FLValue a, FLValue b) { return fl_map_merge(a, b); }
FLValue dissoc(FLValue m, FLValue k) { return fl_map_del(m, k); }
FLValue vals(FLValue m)  { return fl_map_vals(m); }
FLValue keys(FLValue m)  { return fl_map_keys(m); }

/* assoc (set alias) */
FLValue assoc(FLValue m, FLValue k, FLValue v) { return fl_map_set(m, k, v); }

/* some / every / any */
FLValue some(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_bool(false);
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (fl_truthy(r)) return fl_bool(true);
    }
    return fl_bool(false);
}
FLValue every(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_bool(true);
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (!fl_truthy(r)) return fl_bool(false);
    }
    return fl_bool(true);
}

/* max / min on vector */
FLValue vec_max(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len == 0) return fl_nil();
    FLValue m = vp->data[0];
    for (uint32_t i = 1; i < vp->len; i++) {
        if (fl_truthy(fl_gt(vp->data[i], m))) m = vp->data[i];
    }
    return m;
}
FLValue vec_min(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    if (vp->len == 0) return fl_nil();
    FLValue m = vp->data[0];
    for (uint32_t i = 1; i < vp->len; i++) {
        if (fl_truthy(fl_lt(vp->data[i], m))) m = vp->data[i];
    }
    return m;
}

/* atom aliases */
FLValue atom(FLValue v)             { return fl_atom_new(v); }
FLValue deref(FLValue a)            { return fl_deref(a); }  /* atom + future 모두 처리 */
FLValue swap_bang(FLValue a, FLValue fn) {
    FLValue cur = fl_atom_deref(a);
    FLValue next = fl_fn_call(fn, 1, &cur);
    return fl_atom_reset(a, next);
}
FLValue reset_bang(FLValue a, FLValue v) { return fl_atom_reset(a, v); }

/* 타입 변환 */
FLValue int_p(FLValue v)    { return fl_bool(v.tag == FL_INT); }
FLValue float_p(FLValue v)  { return fl_bool(v.tag == FL_FLOAT); }
FLValue bool_p(FLValue v)   { return fl_bool(v.tag == FL_BOOL); }
FLValue nil_p(FLValue v)    { return fl_bool(v.tag == FL_NIL); }
FLValue number_p(FLValue v) { return fl_bool(v.tag == FL_INT || v.tag == FL_FLOAT); }

/* 기타 */
FLValue identity(FLValue v) { return v; }
FLValue not_p(FLValue v)    { return fl_not(v); }
FLValue even_p(FLValue v)   { return fl_bool(v.tag == FL_INT && v.i % 2 == 0); }
FLValue odd_p(FLValue v)    { return fl_bool(v.tag == FL_INT && v.i % 2 != 0); }
FLValue inc(FLValue v)      { return fl_add(v, fl_int(1)); }
FLValue dec(FLValue v)      { return fl_sub(v, fl_int(1)); }

/* list = vec 생성 (cgc-bin이 list를 fl_vec_from으로 emit하므로 alias 불필요할 수 있음) */
/* 하지만 혹시 필요할 경우를 위해 */
FLValue list_new0(void)                    { return fl_vec_new(); }

/* fl_vec_* aliases (cgc-bin이 emit하는 내부 이름) */
FLValue fl_vec_first(FLValue vec) { return first(vec); }
/* fl_vec_last: collection.c에 이미 구현됨 */
FLValue fl_vec_rest(FLValue vec)  { return rest(vec);  }

/* fl_concat — 벡터/문자열 연결 */
FLValue fl_concat(FLValue a, FLValue b) {
    if (a.tag == FL_VECTOR && b.tag == FL_VECTOR) {
        FLVector* va = (FLVector*)a.obj;
        FLVector* vb = (FLVector*)b.obj;
        FLValue result = fl_vec_new();
        for (uint32_t i = 0; i < va->len; i++) result = fl_vec_push(result, va->data[i]);
        for (uint32_t i = 0; i < vb->len; i++) result = fl_vec_push(result, vb->data[i]);
        return result;
    }
    /* 문자열 연결 */
    if (a.tag == FL_STRING && b.tag == FL_STRING) {
        const char* sa = fl_cstr(a);
        const char* sb = fl_cstr(b);
        size_t la = strlen(sa), lb = strlen(sb);
        char* buf = (char*)malloc(la + lb + 1);
        memcpy(buf, sa, la); memcpy(buf + la, sb, lb); buf[la + lb] = '\0';
        FLValue r = fl_str_val(buf); free(buf);
        return r;
    }
    return fl_nil();
}

/* concat alias (짧은 이름) */
FLValue concat(FLValue a, FLValue b) { return fl_concat(a, b); }

/* ── 환경 변수 / .env ── */
/* env-load ".env" → env_load(".env") */
FLValue env_load(FLValue path_v) {
    if (path_v.tag != FL_STRING) return fl_nil();
    const char* path = ((FLString*)path_v.obj)->data;
    FILE* f = fopen(path, "r");
    if (!f) return fl_nil();  /* .env 없으면 조용히 무시 */
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        /* 앞뒤 공백 제거 */
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        /* 주석/빈 줄 무시 */
        if (*p == '#' || *p == '\n' || *p == '\0') continue;
        /* KEY=VALUE 파싱 */
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = p;
        char* val = eq + 1;
        /* 값 끝의 개행 제거 */
        size_t vlen = strlen(val);
        while (vlen > 0 && (val[vlen-1] == '\n' || val[vlen-1] == '\r' || val[vlen-1] == '"' || val[vlen-1] == '\''))
            val[--vlen] = '\0';
        /* 값 앞의 따옴표 제거 */
        if (*val == '"' || *val == '\'') val++;
        setenv(key, val, 1);
    }
    fclose(f);
    return fl_nil();
}

/* num "42" → 42 or 42.0 */
FLValue num(FLValue v) {
    if (v.tag == FL_INT || v.tag == FL_FLOAT) return v;
    if (v.tag == FL_STRING) {
        const char* p = ((FLString*)v.obj)->data;
        /* 소수점 있으면 float */
        if (strchr(p, '.')) return fl_float(strtod(p, NULL));
        return fl_int((int64_t)strtoll(p, NULL, 10));
    }
    return fl_nil();
}

/* fl_or: core.c에 이미 구현됨 */

/* server-rate-limit — 현재 no-op (향후 구현 가능) */
FLValue server_rate_limit(FLValue max_reqs, FLValue window_ms) {
    (void)max_reqs; (void)window_ms;
    return fl_nil();
}

/* str_slice — 문자열 슬라이스 (str-slice s start end) */
FLValue str_slice(FLValue s, FLValue start_v, FLValue end_v) {
    /* FL_FLOAT이면 정수로 변환 */
    int64_t st = (start_v.tag == FL_FLOAT) ? (int64_t)start_v.f :
                 (start_v.tag == FL_INT)   ? start_v.i : 0;
    int64_t en = (end_v.tag == FL_FLOAT)   ? (int64_t)end_v.f :
                 (end_v.tag == FL_INT)      ? end_v.i : 0;
    return substring(s, fl_int(st), fl_int(en));
}

/* mariadb_connect 4인자 버전 (host, user, pass, db → port=3306) */
FLValue mariadb_connect4(FLValue host, FLValue user, FLValue pw, FLValue db) {
    return mariadb_connect(host, fl_int(3306), user, pw, db);
}

/* ── URL 디코딩 + 폼 파싱 ── */

/* %XX 디코딩 + '+' → 공백 변환 */
static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

/* url_decode "hello%20world" → "hello world" */
FLValue url_decode(FLValue s) {
    if (s.tag != FL_STRING) return s;
    const char* src = ((FLString*)s.obj)->data;
    size_t slen = strlen(src);
    char* out = (char*)malloc(slen + 1);
    size_t j = 0;
    for (size_t i = 0; i < slen; i++) {
        if (src[i] == '+') {
            out[j++] = ' ';
        } else if (src[i] == '%' && i + 2 < slen &&
                   isxdigit((unsigned char)src[i+1]) &&
                   isxdigit((unsigned char)src[i+2])) {
            out[j++] = (char)((hex_val(src[i+1]) << 4) | hex_val(src[i+2]));
            i += 2;
        } else {
            out[j++] = src[i];
        }
    }
    out[j] = '\0';
    FLValue r = fl_str_val(out);
    free(out);
    return r;
}

/* ── P0: 이름 불일치 alias ── */

/* fl_str_to_num, str_to_upper/lower (runtime.h에 없는 것만) */
FLValue fl_str_to_num(FLValue s)  { return str_to_num(s); }
FLValue str_to_upper(FLValue s)   { return str_upper(s); }
FLValue str_to_lower(FLValue s)   { return str_lower(s); }

/* any?/every? fl_ alias */
FLValue fl_any_p(FLValue fn, FLValue vec)   { return some(fn, vec); }
FLValue fl_every_p(FLValue fn, FLValue vec) { return every(fn, vec); }

/* contains? — 맵 키 존재 / 벡터 원소 포함 / 문자열 부분문자열 */
FLValue fl_contains_p(FLValue coll, FLValue key) {
    if (coll.tag == FL_MAP) {
        FLValue v = fl_map_get(coll, key);
        return fl_bool(v.tag != FL_NIL);
    }
    if (coll.tag == FL_VECTOR) {
        FLVector* vp = (FLVector*)coll.obj;
        for (size_t i = 0; i < vp->len; i++) {
            if (fl_truthy(fl_eq(vp->data[i], key))) return fl_bool(true);
        }
        return fl_bool(false);
    }
    if (coll.tag == FL_STRING) return fl_str_includes(coll, key);
    return fl_bool(false);
}

/* apply — (apply fn vec) : 벡터를 인자 목록으로 펼쳐 fn 호출 */
FLValue fl_apply(FLValue fn, FLValue args) {
    if (args.tag != FL_VECTOR) {
        return fl_fn_call(fn, 1, &args);
    }
    FLVector* vp = (FLVector*)args.obj;
    return fl_fn_call(fn, (int)vp->len, vp->data);
}

/* ── P0: 미구현 기본 함수 ── */

/* sleep */
FLValue fl_sleep_ms(FLValue ms_v) {
    int64_t ms = (ms_v.tag == FL_INT) ? ms_v.i
               : (ms_v.tag == FL_FLOAT) ? (int64_t)ms_v.f : 0;
    if (ms > 0) {
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
        nanosleep(&ts, NULL);
    }
    return fl_nil();
}

/* fl_empty_p, fl_nil_or_empty_p, fl_not_empty_p → runtime.h에 static inline으로 이미 정의됨 */

/* none? — 하나도 true 없어야 */
FLValue fl_none_p(FLValue fn, FLValue vec) {
    return fl_not(some(fn, vec));
}

/* count-if */
FLValue fl_count_if(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(0);
    FLVector* vp = (FLVector*)vec.obj;
    int64_t cnt = 0;
    for (uint32_t i = 0; i < vp->len; i++) {
        if (fl_truthy(fl_fn_call(fn, 1, &vp->data[i]))) cnt++;
    }
    return fl_int(cnt);
}

/* find-first */
FLValue fl_find_first(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* vp = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < vp->len; i++) {
        if (fl_truthy(fl_fn_call(fn, 1, &vp->data[i]))) return vp->data[i];
    }
    return fl_nil();
}

/* ── P1: 컬렉션 함수 ── */

/* entries: 맵 → [[k v] ...] */
FLValue entries(FLValue m) {
    if (m.tag != FL_MAP) return fl_vec_new();
    FLValue ks = fl_map_keys(m);
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue k = kv->data[i];
        FLValue v = fl_map_get(m, k);
        FLValue pair = fl_vec_new();
        pair = fl_vec_push(pair, k);
        pair = fl_vec_push(pair, v);
        result = fl_vec_push(result, pair);
    }
    return result;
}

/* select-keys: 맵에서 특정 키만 추출 */
FLValue select_keys(FLValue m, FLValue ks) {
    if (m.tag != FL_MAP || ks.tag != FL_VECTOR) return fl_map_new();
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_map_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue v = fl_map_get(m, kv->data[i]);
        if (v.tag != FL_NIL) result = fl_map_set(result, kv->data[i], v);
    }
    return result;
}

/* get-in: 중첩 맵 접근 (get-in m ["a" "b"]) */
FLValue fl_get_in(FLValue m, FLValue path) {
    if (path.tag != FL_VECTOR) return fl_nil();
    FLVector* pv = (FLVector*)path.obj;
    FLValue cur = m;
    for (uint32_t i = 0; i < pv->len; i++) {
        if (cur.tag != FL_MAP) return fl_nil();
        cur = fl_map_get(cur, pv->data[i]);
    }
    return cur;
}

/* obj-omit: 맵에서 키 제거 */
FLValue fl_obj_omit(FLValue m, FLValue ks) {
    if (m.tag != FL_MAP) return m;
    if (ks.tag != FL_VECTOR) return m;
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = m;
    for (uint32_t i = 0; i < kv->len; i++)
        result = fl_map_del(result, kv->data[i]);
    return result;
}

/* repeat: n번 값 반복 → 벡터 */
FLValue fl_repeat(FLValue n_v, FLValue val) {
    int64_t n = (n_v.tag == FL_INT) ? n_v.i : 0;
    FLValue result = fl_vec_new();
    for (int64_t i = 0; i < n; i++) result = fl_vec_push(result, val);
    return result;
}

/* sort-by: keyfn으로 정렬 */
static FLValue _sort_by_fn;
static int _sort_by_cmp(const void* a, const void* b) {
    FLValue ka = fl_fn_call(_sort_by_fn, 1, (FLValue*)a);
    FLValue kb = fl_fn_call(_sort_by_fn, 1, (FLValue*)b);
    if (ka.tag == FL_INT && kb.tag == FL_INT) return (ka.i < kb.i) ? -1 : (ka.i > kb.i) ? 1 : 0;
    if (ka.tag == FL_STRING && kb.tag == FL_STRING)
        return strcmp(((FLString*)ka.obj)->data, ((FLString*)kb.obj)->data);
    double da = (ka.tag == FL_FLOAT) ? ka.f : (double)ka.i;
    double db = (kb.tag == FL_FLOAT) ? kb.f : (double)kb.i;
    return (da < db) ? -1 : (da > db) ? 1 : 0;
}
FLValue fl_sort_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return vec;
    FLVector* vp = (FLVector*)vec.obj;
    FLValue* copy = (FLValue*)malloc(vp->len * sizeof(FLValue));
    memcpy(copy, vp->data, vp->len * sizeof(FLValue));
    _sort_by_fn = fn;
    qsort(copy, vp->len, sizeof(FLValue), _sort_by_cmp);
    FLValue r = fl_vec_from(copy, vp->len);
    free(copy);
    return r;
}

/* keep: fn 결과가 nil이 아닌 것만 모음 */
FLValue fl_keep(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue r = fl_fn_call(fn, 1, &vp->data[i]);
        if (r.tag != FL_NIL) result = fl_vec_push(result, r);
    }
    return result;
}

/* map-indexed: (fn index elem) */
FLValue fl_map_indexed(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue args[2] = { fl_int((int64_t)i), vp->data[i] };
        result = fl_vec_push(result, fl_fn_call(fn, 2, args));
    }
    return result;
}

/* mapcat: map + flatten 1단계 */
FLValue fl_mapcat(FLValue fn, FLValue vec) {
    return flatten(fl_map_fn(fn, vec));
}

/* into: coll에 항목 추가 */
FLValue fl_into(FLValue target, FLValue src) {
    if (src.tag != FL_VECTOR) return target;
    FLVector* sv = (FLVector*)src.obj;
    FLValue result = target;
    if (result.tag != FL_VECTOR) result = fl_vec_new();
    for (uint32_t i = 0; i < sv->len; i++) result = fl_vec_push(result, sv->data[i]);
    return result;
}

/* conj: 컬렉션에 항목 추가 */
FLValue fl_conj(FLValue coll, FLValue item) {
    if (coll.tag == FL_VECTOR) return fl_vec_push(coll, item);
    return fl_vec_push(fl_vec_new(), item);
}

/* comp: 함수 합성 (comp f g) → (fn [x] (f (g x))) */
/* comp은 고차함수라 FL 클로저로 래핑이 필요하나 C에서 단순 구현 */
/* 현재는 2-인자 직접 호출만 지원 */
FLValue fl_comp(FLValue f, FLValue g) {
    /* fl_comp(f, g) 자체는 잘 안 쓰임 — 대신 cgc가 직접 emit */
    /* 여기서는 fl_fn_call로 compose 함수 반환은 불가, 대신 apply */
    (void)f; (void)g;
    return fl_nil();  /* intentional limitation: comp is frozen until closure support is implemented */
}

/* map-vals: 맵의 모든 값에 fn 적용 */
FLValue fl_map_vals_fn(FLValue fn, FLValue m) {
    if (m.tag != FL_MAP) return m;
    FLValue ks = fl_map_keys(m);
    FLVector* kv = (FLVector*)ks.obj;
    FLValue result = fl_map_new();
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue k = kv->data[i];
        FLValue v = fl_map_get(m, k);
        FLValue nv = fl_fn_call(fn, 1, &v);
        result = fl_map_set(result, k, nv);
    }
    return result;
}

/* frequencies: 빈도 집계 */
FLValue frequencies(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_map_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue m = fl_map_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue k = vp->data[i];
        FLValue cur = fl_map_get(m, k);
        int64_t cnt = (cur.tag == FL_INT) ? cur.i : 0;
        m = fl_map_set(m, k, fl_int(cnt + 1));
    }
    return m;
}

/* group-by: 키 함수로 그룹핑 */
FLValue group_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_map_new();
    FLVector* vp = (FLVector*)vec.obj;
    FLValue m = fl_map_new();
    for (uint32_t i = 0; i < vp->len; i++) {
        FLValue k = fl_fn_call(fn, 1, &vp->data[i]);
        FLValue cur = fl_map_get(m, k);
        if (cur.tag != FL_VECTOR) cur = fl_vec_new();
        cur = fl_vec_push(cur, vp->data[i]);
        m = fl_map_set(m, k, cur);
    }
    return m;
}

/* ── P2: 응답 쿠키 ── */
FLValue fl_resp_set_cookie(FLValue name, FLValue val, FLValue opts) {
    /* opts: {"path" "/" "max-age" 3600 "httponly" true} */
    const char* n = (name.tag == FL_STRING) ? ((FLString*)name.obj)->data : "";
    const char* v = (val.tag  == FL_STRING) ? ((FLString*)val.obj)->data  : "";
    char buf[512];
    int pos = snprintf(buf, sizeof(buf), "%s=%s", n, v);
    /* path */
    FLValue path = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("path")) : fl_nil();
    if (path.tag == FL_STRING)
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Path=%s", ((FLString*)path.obj)->data);
    else
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Path=/");
    /* max-age */
    FLValue max_age = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("max-age")) : fl_nil();
    if (max_age.tag == FL_INT)
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; Max-Age=%lld", (long long)max_age.i);
    /* HttpOnly */
    FLValue httponly = (opts.tag == FL_MAP) ? fl_map_get(opts, fl_str_val("httponly")) : fl_nil();
    if (fl_truthy(httponly))
        pos += snprintf(buf+pos, sizeof(buf)-pos, "; HttpOnly");
    (void)pos;
    return fl_str_val(buf);
}
FLValue fl_resp_html_cookie(FLValue html, FLValue cookie_hdr) {
    /* cookie_hdr는 fl_resp_set_cookie가 반환한 헤더 문자열 */
    /* 현재 server.c는 직접 Set-Cookie 헤더를 넣는 방법이 없음 */
    /* 임시: html 그대로 반환 (향후 server.c 개선 시 수정) */
    (void)cookie_hdr;
    return server_html(html);
}

/* form_parse "title=hello&content=world" → {"title":"hello","content":"world"} */
/* ── 중첩 맵 업데이트 ── */

/* assoc-in: (assoc-in m [k1 k2 ...] v) */
FLValue assoc_in(FLValue m, FLValue path, FLValue v) {
    int n = (int)fl_vec_len(path).i;
    if (n == 0) return m;
    if (n == 1) {
        FLValue k = fl_vec_get(path, fl_int(0));
        return fl_map_set(m, k, v);
    }
    FLValue k    = fl_vec_get(path, fl_int(0));
    FLValue tail = fl_vec_slice(path, fl_int(1), fl_int(n));
    FLValue sub  = fl_map_get(m, k);
    if (sub.tag == FL_NIL) sub = fl_map_new();
    return fl_map_set(m, k, assoc_in(sub, tail, v));
}

/* update-in: (update-in m [k1 k2 ...] fn) */
FLValue update_in(FLValue m, FLValue path, FLValue fn) {
    int n = (int)fl_vec_len(path).i;
    if (n == 0) return m;
    FLValue k = fl_vec_get(path, fl_int(0));
    if (n == 1) {
        FLValue old = fl_map_get(m, k);
        FLValue new_val = fl_fn_call(fn, 1, &old);
        return fl_map_set(m, k, new_val);
    }
    FLValue tail = fl_vec_slice(path, fl_int(1), fl_int(n));
    FLValue sub  = fl_map_get(m, k);
    if (sub.tag == FL_NIL) sub = fl_map_new();
    return fl_map_set(m, k, update_in(sub, tail, fn));
}

/* get-in alias (FL 이름과 일치) */
FLValue get_in(FLValue m, FLValue path) { return fl_get_in(m, path); }

/* ── 문자열 포맷 ── */
/* str-format: (str-format "%s is %d" name age) — args는 벡터 */
FLValue str_format(FLValue fmt_v, FLValue args) {
    if (fmt_v.tag != FL_STRING) return fl_str_val("");
    const char* fmt = ((FLString*)fmt_v.obj)->data;
    /* 결과 버퍼: fmt 길이의 8배 정도 */
    size_t cap = strlen(fmt) * 8 + 256;
    char* buf = (char*)malloc(cap);
    if (!buf) return fl_str_val("");

    int arg_n = (int)fl_vec_len(args).i;
    int ai = 0;
    size_t wi = 0;

    for (const char* p = fmt; *p && wi < cap - 64; p++) {
        if (*p != '%') { buf[wi++] = *p; continue; }
        p++;
        if (*p == '%') { buf[wi++] = '%'; continue; }
        FLValue arg = (ai < arg_n) ? fl_vec_get(args, fl_int(ai++)) : fl_nil();
        char spec = *p;
        if (spec == 's') {
            FLValue sv = to_string(arg);
            const char* s = ((FLString*)sv.obj)->data;
            size_t sl = strlen(s);
            if (wi + sl < cap) { memcpy(buf + wi, s, sl); wi += sl; }
        } else if (spec == 'd' || spec == 'i') {
            long long iv = (arg.tag == FL_INT) ? (long long)arg.i : 0;
            wi += snprintf(buf + wi, cap - wi, "%lld", iv);
        } else if (spec == 'f') {
            double dv = (arg.tag == FL_FLOAT) ? arg.f :
                        (arg.tag == FL_INT)   ? (double)arg.i : 0.0;
            wi += snprintf(buf + wi, cap - wi, "%.6f", dv);
        } else {
            buf[wi++] = '%'; buf[wi++] = spec;
        }
    }
    buf[wi] = '\0';
    FLValue result = fl_str_val(buf);
    free(buf);
    return result;
}

FLValue form_parse(FLValue body) {
    if (body.tag != FL_STRING) return fl_map_new();
    const char* src = ((FLString*)body.obj)->data;
    FLValue map = fl_map_new();
    if (!src || src[0] == '\0') return map;

    /* 복사본으로 작업 */
    char* buf = strdup(src);
    char* pair = buf;
    while (pair && *pair) {
        char* next = strchr(pair, '&');
        if (next) *next = '\0';

        char* eq = strchr(pair, '=');
        FLValue key, val;
        if (eq) {
            *eq = '\0';
            key = url_decode(fl_str_val(pair));
            val = url_decode(fl_str_val(eq + 1));
        } else {
            key = url_decode(fl_str_val(pair));
            val = fl_str_val("");
        }
        map = fl_map_set(map, key, val);

        pair = next ? next + 1 : NULL;
    }
    free(buf);
    return map;
}

/* ═══════════════════════════════════════════════════════════════════
 * 정규식 (POSIX ERE)
 * ═══════════════════════════════════════════════════════════════════ */
#include <regex.h>

/* str_match(str, pattern) → 첫 번째 매칭 문자열 또는 nil */
FLValue str_match(FLValue str_v, FLValue pat_v) {
    if (str_v.tag != FL_STRING || pat_v.tag != FL_STRING) return fl_nil();
    const char* s   = ((FLString*)str_v.obj)->data;
    const char* pat = ((FLString*)pat_v.obj)->data;
    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return fl_nil();
    regmatch_t m[1];
    FLValue result = fl_nil();
    if (regexec(&re, s, 1, m, 0) == 0) {
        int start = m[0].rm_so, end = m[0].rm_eo;
        char* buf = (char*)malloc(end - start + 1);
        memcpy(buf, s + start, end - start);
        buf[end - start] = '\0';
        result = fl_str_val(buf);
        free(buf);
    }
    regfree(&re);
    return result;
}

/* str_match_all(str, pattern) → 모든 매칭 벡터 */
FLValue str_match_all(FLValue str_v, FLValue pat_v) {
    if (str_v.tag != FL_STRING || pat_v.tag != FL_STRING) return fl_vec_new();
    const char* s   = ((FLString*)str_v.obj)->data;
    const char* pat = ((FLString*)pat_v.obj)->data;
    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return fl_vec_new();
    FLValue vec = fl_vec_builder_new();
    const char* p = s;
    regmatch_t m[1];
    while (regexec(&re, p, 1, m, 0) == 0) {
        int start = m[0].rm_so, end = m[0].rm_eo;
        char* buf = (char*)malloc(end - start + 1);
        memcpy(buf, p + start, end - start);
        buf[end - start] = '\0';
        fl_vec_builder_push(vec, fl_str_val(buf));
        free(buf);
        p += end;
        if (end == 0) p++;  /* 빈 매칭 무한루프 방지 */
    }
    regfree(&re);
    return fl_vec_builder_freeze(vec);
}

/* str_replace_re(str, pattern, replacement) → 첫 번째 치환 */
FLValue str_replace_re(FLValue str_v, FLValue pat_v, FLValue rep_v) {
    if (str_v.tag != FL_STRING || pat_v.tag != FL_STRING || rep_v.tag != FL_STRING)
        return str_v;
    const char* s   = ((FLString*)str_v.obj)->data;
    const char* pat = ((FLString*)pat_v.obj)->data;
    const char* rep = ((FLString*)rep_v.obj)->data;
    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return str_v;
    regmatch_t m[1];
    FLValue result = str_v;
    if (regexec(&re, s, 1, m, 0) == 0) {
        int start = m[0].rm_so, end = m[0].rm_eo;
        int slen = (int)strlen(s), rlen = (int)strlen(rep);
        char* buf = (char*)malloc(slen - (end - start) + rlen + 1);
        int tail = slen - end;
        memcpy(buf, s, start);
        memcpy(buf + start, rep, rlen);
        memcpy(buf + start + rlen, s + end, tail + 1);  /* tail + null */
        result = fl_str_val(buf);
        free(buf);
    }
    regfree(&re);
    return result;
}

/* str_replace_all_re(str, pattern, replacement) → 전체 치환 */
FLValue str_replace_all_re(FLValue str_v, FLValue pat_v, FLValue rep_v) {
    if (str_v.tag != FL_STRING || pat_v.tag != FL_STRING || rep_v.tag != FL_STRING)
        return str_v;
    const char* orig = ((FLString*)str_v.obj)->data;
    const char* pat  = ((FLString*)pat_v.obj)->data;
    const char* rep  = ((FLString*)rep_v.obj)->data;
    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return str_v;
    int rlen = (int)strlen(rep);
    int cap  = (int)strlen(orig) * 2 + 256;
    char* buf = (char*)malloc(cap);
    int blen = 0;
    const char* p = orig;
    regmatch_t m[1];
    while (regexec(&re, p, 1, m, 0) == 0) {
        int start = m[0].rm_so, end = m[0].rm_eo;
        /* 매칭 전 텍스트 */
        while (blen + start + rlen + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
        memcpy(buf + blen, p, start);
        blen += start;
        memcpy(buf + blen, rep, rlen);
        blen += rlen;
        p += end;
        if (end == 0) { if (*p) buf[blen++] = *p++; else break; }
    }
    /* 나머지 */
    int tail = (int)strlen(p);
    while (blen + tail + 1 >= cap) { cap *= 2; buf = (char*)realloc(buf, cap); }
    memcpy(buf + blen, p, tail);
    blen += tail;
    buf[blen] = '\0';
    regfree(&re);
    FLValue r = fl_str_val(buf);
    free(buf);
    return r;
}

/* str_test(str, pattern) → boolean (매칭 여부) */
FLValue str_test(FLValue str_v, FLValue pat_v) {
    if (str_v.tag != FL_STRING || pat_v.tag != FL_STRING) return fl_bool(false);
    const char* s   = ((FLString*)str_v.obj)->data;
    const char* pat = ((FLString*)pat_v.obj)->data;
    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return fl_bool(false);
    regmatch_t m[1];
    bool matched = (regexec(&re, s, 1, m, 0) == 0);
    regfree(&re);
    return fl_bool(matched);
}

/* ═══════════════════════════════════════════════════════════════════
 * future (pthread 기반 비동기)
 * ═══════════════════════════════════════════════════════════════════ */
#include <pthread.h>

typedef struct {
    FLValue fn;
    FLValue result;
    int     done;
    int     freed;   /* deref 후 cleanup 완료 표시 */
    pthread_mutex_t mu;
    pthread_cond_t  cv;
} FLFuture;

static void* future_runner(void* arg) {
    FLFuture* f = (FLFuture*)arg;
    FLValue result = fl_fn_call(f->fn, 0, NULL);
    pthread_mutex_lock(&f->mu);
    f->result = fl_heap_copy(result);
    f->done   = 1;
    pthread_cond_broadcast(&f->cv);
    pthread_mutex_unlock(&f->mu);
    return NULL;
}

/* fl_future(fn) → opaque FLValue (FL_MAP으로 래핑) */
FLValue fl_future(FLValue fn) {
    if (fn.tag != FL_FN) return fl_nil();
    FLFuture* f = (FLFuture*)malloc(sizeof(FLFuture));
    f->fn     = fl_heap_copy(fn);
    f->done   = 0;
    f->freed  = 0;
    f->result = fl_nil();
    pthread_mutex_init(&f->mu, NULL);
    pthread_cond_init(&f->cv, NULL);

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, future_runner, f);
    pthread_attr_destroy(&attr);

    /* 포인터를 int64로 저장 (opaque) */
    FLValue handle = fl_map_new();
    handle = fl_map_set(handle, fl_str_val("__future__"), fl_int((int64_t)(uintptr_t)f));
    return handle;
}

/* fl_deref — atom(FL_VECTOR)과 future(FL_MAP) 모두 처리 */
FLValue fl_deref(FLValue handle) {
    /* atom */
    if (handle.tag == FL_VECTOR) return fl_atom_deref(handle);
    if (handle.tag != FL_MAP) return fl_nil();
    FLValue ptr_v = fl_map_get(handle, fl_str_val("__future__"));
    if (ptr_v.tag != FL_INT) return fl_nil();
    FLFuture* f = (FLFuture*)(uintptr_t)ptr_v.i;
    pthread_mutex_lock(&f->mu);
    while (!f->done) pthread_cond_wait(&f->cv, &f->mu);
    FLValue result = f->result;
    if (!f->freed) {
        f->freed = 1;
        pthread_mutex_unlock(&f->mu);
        pthread_mutex_destroy(&f->mu);
        pthread_cond_destroy(&f->cv);
        free(f);
    } else {
        pthread_mutex_unlock(&f->mu);
    }
    return result;
}

/* fl_deref_timeout(future, ms) — ms 초과 시 nil 반환 */
FLValue fl_deref_timeout(FLValue handle, FLValue ms_v) {
    if (handle.tag == FL_VECTOR) return fl_atom_deref(handle);
    if (handle.tag != FL_MAP) return fl_nil();
    FLValue ptr_v = fl_map_get(handle, fl_str_val("__future__"));
    if (ptr_v.tag != FL_INT) return fl_nil();
    FLFuture* f = (FLFuture*)(uintptr_t)ptr_v.i;
    int64_t ms = (ms_v.tag == FL_INT) ? ms_v.i
               : (ms_v.tag == FL_FLOAT) ? (int64_t)ms_v.f : 5000;
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec  += ms / 1000;
    deadline.tv_nsec += (ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) { deadline.tv_sec++; deadline.tv_nsec -= 1000000000L; }
    pthread_mutex_lock(&f->mu);
    while (!f->done) {
        if (pthread_cond_timedwait(&f->cv, &f->mu, &deadline) == ETIMEDOUT) break;
    }
    FLValue result = f->done ? f->result : fl_nil();
    if (f->done && !f->freed) {
        f->freed = 1;
        pthread_mutex_unlock(&f->mu);
        pthread_mutex_destroy(&f->mu);
        pthread_cond_destroy(&f->cv);
        free(f);
    } else {
        pthread_mutex_unlock(&f->mu);
    }
    return result;
}

/* fl_future_done(future) → boolean */
FLValue fl_future_done(FLValue handle) {
    if (handle.tag != FL_MAP) return fl_bool(false);
    FLValue ptr_v = fl_map_get(handle, fl_str_val("__future__"));
    if (ptr_v.tag != FL_INT) return fl_bool(false);
    FLFuture* f = (FLFuture*)(uintptr_t)ptr_v.i;
    pthread_mutex_lock(&f->mu);
    int done = f->done;
    pthread_mutex_unlock(&f->mu);
    return fl_bool(done);
}

/* ═══════════════════════════════════════════════════════════════
   stdlib 확장 — v11 동등 함수 (2026-06-15)
   ═══════════════════════════════════════════════════════════════ */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* ── 날짜/시간 ── */

/* (date_format timestamp fmt)
   timestamp = Unix 초 (fl_now 반환값)
   fmt = strftime 포맷 문자열 e.g. "%Y-%m-%d %H:%M:%S"
   반환: 포맷된 문자열 */
FLValue date_format(FLValue ts, FLValue fmt) {
    time_t t = (time_t)((ts.tag == FL_INT) ? ts.i : (int64_t)ts.f);
    const char* f = (fmt.tag == FL_STRING) ? ((FLString*)fmt.obj)->data : "%Y-%m-%d %H:%M:%S";
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    char buf[256];
    strftime(buf, sizeof(buf), f, &tm_info);
    return fl_str_val(buf);
}

/* (date_now_str) → "2026-06-15 12:34:56" */
FLValue date_now_str(void) {
    time_t t = time(NULL);
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    return fl_str_val(buf);
}

/* (date_parse str fmt) → Unix 타임스탬프 (정수) */
FLValue date_parse(FLValue str_v, FLValue fmt_v) {
    const char* s = (str_v.tag == FL_STRING) ? ((FLString*)str_v.obj)->data : "";
    const char* f = (fmt_v.tag == FL_STRING) ? ((FLString*)fmt_v.obj)->data : "%Y-%m-%d %H:%M:%S";
    struct tm tm_info = {0};
    if (!strptime(s, f, &tm_info)) return fl_nil();
    return fl_int((int64_t)mktime(&tm_info));
}

/* (date_add timestamp seconds) → 새 타임스탬프 */
FLValue date_add(FLValue ts, FLValue secs) {
    int64_t t = (ts.tag == FL_INT) ? ts.i : (int64_t)ts.f;
    int64_t s = (secs.tag == FL_INT) ? secs.i : (int64_t)secs.f;
    return fl_int(t + s);
}

/* (date_diff ts1 ts2) → 초 차이 (ts1 - ts2) */
FLValue date_diff(FLValue ts1, FLValue ts2) {
    int64_t t1 = (ts1.tag == FL_INT) ? ts1.i : (int64_t)ts1.f;
    int64_t t2 = (ts2.tag == FL_INT) ? ts2.i : (int64_t)ts2.f;
    return fl_int(t1 - t2);
}

/* ── UUID v4 ── */

/* (uuid4) → "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx" */
FLValue uuid4(void) {
    unsigned char rnd[16];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return fl_str_val("00000000-0000-4000-8000-000000000000");
    ssize_t got = read(fd, rnd, 16);
    close(fd);
    if (got != 16) return fl_nil();

    /* RFC 4122 버전4 비트 설정 */
    rnd[6] = (rnd[6] & 0x0f) | 0x40;   /* version 4 */
    rnd[8] = (rnd[8] & 0x3f) | 0x80;   /* variant 10xx */

    char buf[37];
    snprintf(buf, sizeof(buf),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        rnd[0],  rnd[1],  rnd[2],  rnd[3],
        rnd[4],  rnd[5],  rnd[6],  rnd[7],
        rnd[8],  rnd[9],  rnd[10], rnd[11],
        rnd[12], rnd[13], rnd[14], rnd[15]);
    return fl_str_val(buf);
}

/* ── 컬렉션 유틸 ── */

/* (sum vec) → 합계 */
FLValue fl_sum(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(0);
    FLVector* v = (FLVector*)vec.obj;
    double total = 0;
    int has_float = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue x = v->data[i];
        if      (x.tag == FL_INT)   total += (double)x.i;
        else if (x.tag == FL_FLOAT) { total += x.f; has_float = 1; }
    }
    return has_float ? fl_float(total) : fl_int((int64_t)total);
}

/* (average vec) → 평균 (float) */
FLValue fl_average(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_float(0.0);
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_float(0.0);
    FLValue s = fl_sum(vec);
    double total = (s.tag == FL_INT) ? (double)s.i : s.f;
    return fl_float(total / v->len);
}

/* (distinct vec) → 중복 제거 벡터 */
FLValue fl_distinct(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue item = v->data[i];
        int found = 0;
        FLVector* r = (FLVector*)result.obj;
        for (uint32_t j = 0; j < r->len; j++) {
            if (fl_truthy(fl_eq(r->data[j], item))) { found = 1; break; }
        }
        if (!found) result = fl_vec_push(result, item);
    }
    return result;
}

/* (max-by fn vec) → fn 결과가 최대인 원소 */
FLValue fl_max_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    FLValue best = v->data[0];
    FLValue best_score = fl_apply_fn(fn,best);
    for (uint32_t i = 1; i < v->len; i++) {
        FLValue score = fl_apply_fn(fn,v->data[i]);
        if (fl_truthy(fl_gt(score, best_score))) {
            best = v->data[i]; best_score = score;
        }
    }
    return best;
}

/* (min-by fn vec) → fn 결과가 최소인 원소 */
FLValue fl_min_by(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_nil();
    FLValue best = v->data[0];
    FLValue best_score = fl_apply_fn(fn,best);
    for (uint32_t i = 1; i < v->len; i++) {
        FLValue score = fl_apply_fn(fn,v->data[i]);
        if (fl_truthy(fl_lt(score, best_score))) {
            best = v->data[i]; best_score = score;
        }
    }
    return best;
}

/* (partition n vec) → n개씩 묶은 벡터의 벡터 */
FLValue fl_partition(FLValue n_v, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    int n = (n_v.tag == FL_INT) ? (int)n_v.i : 1;
    if (n <= 0) n = 1;
    FLVector* v = (FLVector*)vec.obj;
    FLValue result = fl_vec_new();
    FLValue chunk = fl_vec_new();
    for (uint32_t i = 0; i < v->len; i++) {
        chunk = fl_vec_push(chunk, v->data[i]);
        if ((int)((FLVector*)chunk.obj)->len == n) {
            result = fl_vec_push(result, chunk);
            chunk  = fl_vec_new();
        }
    }
    if (((FLVector*)chunk.obj)->len > 0)
        result = fl_vec_push(result, chunk);
    return result;
}

/* (index-where fn vec) → 조건에 맞는 첫 인덱스 (없으면 -1) */
FLValue fl_index_where(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(-1);
    FLVector* v = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < v->len; i++) {
        if (fl_truthy(fl_apply_fn(fn,v->data[i]))) return fl_int((int64_t)i);
    }
    return fl_int(-1);
}

/* (zip-map keys vals) → 맵 생성 */
FLValue fl_zip_map(FLValue keys, FLValue vals) {
    if (keys.tag != FL_VECTOR || vals.tag != FL_VECTOR) return fl_map_new();
    FLVector* ks = (FLVector*)keys.obj;
    FLVector* vs = (FLVector*)vals.obj;
    uint32_t len = ks->len < vs->len ? ks->len : vs->len;
    FLValue m = fl_map_new();
    for (uint32_t i = 0; i < len; i++)
        m = fl_map_set(m, ks->data[i], vs->data[i]);
    return m;
}

/* ── 문자열 유틸 ── */

/* (str-char-at s idx) → 1글자 문자열 */
FLValue str_char_at(FLValue s, FLValue idx_v) {
    if (s.tag != FL_STRING) return fl_str_val("");
    const char* data = ((FLString*)s.obj)->data;
    int idx = (int)((idx_v.tag == FL_INT) ? idx_v.i : (int64_t)idx_v.f);
    int len = (int)((FLString*)s.obj)->len;
    if (idx < 0 || idx >= len) return fl_str_val("");
    char buf[2] = {data[idx], '\0'};
    return fl_str_val(buf);
}

/* (str-length s) → 바이트 길이 (정수) */
FLValue str_length(FLValue s) {
    if (s.tag != FL_STRING) return fl_int(0);
    return fl_int((int64_t)((FLString*)s.obj)->len);
}

/* (str-reverse s) → 역순 문자열 */
FLValue _impl_str_reverse(FLValue s) {
    if (s.tag != FL_STRING) return fl_str_val("");
    const char* data = ((FLString*)s.obj)->data;
    int len = (int)((FLString*)s.obj)->len;
    char* buf = malloc(len + 1);
    for (int i = 0; i < len; i++) buf[i] = data[len - 1 - i];
    buf[len] = '\0';
    FLValue r = fl_str_val(buf);
    free(buf);
    return r;
}

/* (num-to-str n) → 숫자 → 문자열 (alias) */
FLValue num_to_str(FLValue n) { return to_string(n); }

/* (pad-zero n width) → "007", "042" */
FLValue pad_zero(FLValue n, FLValue width_v) {
    long long val = (n.tag == FL_INT) ? n.i : (long long)n.f;
    int width = (width_v.tag == FL_INT) ? (int)width_v.i : 1;
    char buf[64];
    snprintf(buf, sizeof(buf), "%0*lld", width, val);
    return fl_str_val(buf);
}

/* ── 수학 유틸 ── */

/* (clamp val min max) → min ≤ val ≤ max */
FLValue fl_clamp(FLValue val, FLValue lo, FLValue hi) {
    double v = (val.tag == FL_INT) ? (double)val.i : val.f;
    double l = (lo.tag  == FL_INT) ? (double)lo.i  : lo.f;
    double h = (hi.tag  == FL_INT) ? (double)hi.i  : hi.f;
    if (v < l) v = l;
    if (v > h) v = h;
    return (val.tag == FL_INT && lo.tag == FL_INT && hi.tag == FL_INT)
           ? fl_int((int64_t)v) : fl_float(v);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
 * fx 독립 언어 — 투명 alias
 * fxb_ 접두사 없이 직접 사용 가능:
 *   (sqlite-open path)  (sqlite-exec db sql)  (sqlite-query db sql)
 *   (http-get url)  (http-post url body)
 *   (server-json data)  — 맵이면 자동 직렬화
 * ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ */

/* SQLite 투명 alias */
FLValue sqlite_open_v2(FLValue path)                         { return fxb_sqlite_open(path); }
FLValue sqlite_query_v2(FLValue db, FLValue sql)             { return fxb_sqlite_query(db, sql); }
FLValue sqlite_exec_v2(FLValue db, FLValue sql)              { return fxb_sqlite_exec(db, sql); }
FLValue sqlite_one_v2(FLValue db, FLValue sql)               { return fxb_sqlite_one(db, sql); }
FLValue sqlite_query_p_v2(FLValue db, FLValue sql, FLValue p){ return fxb_sqlite_query_p(db, sql, p); }
FLValue sqlite_exec_p_v2(FLValue db, FLValue sql, FLValue p) { return fxb_sqlite_exec_p(db, sql, p); }
FLValue sqlite_close_v2(FLValue db)                          { return fxb_sqlite_close(db); }

/* server_json — 맵/리스트면 자동 json_stringify */
FLValue fx_server_json(FLValue data) {
    if (data.tag == FL_MAP || data.tag == FL_VECTOR) {
        FLValue json = json_stringify(data);
        return server_json(json);
    }
    return server_json(data);
}

/* server_respond — status + body 한번에 */
FLValue fx_respond(FLValue status, FLValue data) {
    FLValue json;
    if (data.tag == FL_MAP || data.tag == FL_VECTOR)
        json = json_stringify(data);
    else
        json = data;
    return server_status(status, json);
}


/* ── HTTP 클라이언트 kebab-case alias ── */
/* fx에서: (http-get url) → http_get(url) */
FLValue http_get_alias(FLValue url)                               { return http_get(url); }
FLValue http_post_alias(FLValue url, FLValue body)                { return http_post(url, body); }
FLValue http_put_alias(FLValue url, FLValue body)                 { return http_put(url, body); }
FLValue http_del_alias(FLValue url)                               { return http_del(url); }
FLValue http_patch_alias(FLValue url, FLValue body)               { return http_patch(url, body); }
FLValue http_body_alias(FLValue res)                              { return http_body(res); }
FLValue http_status_alias(FLValue res)                            { return http_status(res); }
FLValue http_ok_p_alias(FLValue res)                              { return http_ok_p(res); }

/* ── 정규식 alias ── */
FLValue regex_match_alias(FLValue p, FLValue s)                   { return regex_match(p, s); }
FLValue regex_find_alias(FLValue p, FLValue s)                    { return regex_find(p, s); }
FLValue regex_find_all_alias(FLValue p, FLValue s)                { return regex_find_all(p, s); }
FLValue regex_groups_alias(FLValue p, FLValue s)                  { return regex_groups(p, s); }
FLValue regex_replace_alias(FLValue p, FLValue s, FLValue r)      { return regex_replace(p, s, r); }
FLValue regex_replace_all_alias(FLValue p, FLValue s, FLValue r)  { return regex_replace_all(p, s, r); }
FLValue regex_split_alias(FLValue p, FLValue s)                   { return regex_split(p, s); }

/* ── stdlib 갭 — kebab-case alias + 미구현 함수 ─────────────────── */
#ifndef FL_NO_CRYPTO
#include <openssl/sha.h>
#include <openssl/md5.h>
#endif

/* uuid, random, max-by, min-by, clamp, file-exists? */
FLValue uuid(void)                                { return uuid4(); }
FLValue fl_random(void)                          { return math_random(); }
FLValue max_by(FLValue fn, FLValue vec)           { return fl_max_by(fn, vec); }
FLValue min_by(FLValue fn, FLValue vec)           { return fl_min_by(fn, vec); }
FLValue clamp(FLValue val, FLValue lo, FLValue hi){ return fl_clamp(val, lo, hi); }
FLValue file_exists_p(FLValue path)               { return file_exists(path); }

#ifndef FL_NO_CRYPTO
/* sha256 str → hex string */
FLValue sha256(FLValue s) {
    if (s.tag != FL_STRING || !s.obj) return fl_str_val("");
    const char* data = ((FLString*)s.obj)->data;
    size_t len = ((FLString*)s.obj)->len;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)data, len, digest);
    char hex[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        snprintf(hex + i * 2, 3, "%02x", digest[i]);
    return fl_str_val(hex);
}

/* md5 str → hex string */
FLValue md5(FLValue s) {
    if (s.tag != FL_STRING || !s.obj) return fl_str_val("");
    const char* data = ((FLString*)s.obj)->data;
    size_t len = ((FLString*)s.obj)->len;
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)data, len, digest);
    char hex[MD5_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        snprintf(hex + i * 2, 3, "%02x", digest[i]);
    return fl_str_val(hex);
}
#endif

/* json-pretty val → 들여쓰기 JSON 문자열 */
FLValue json_pretty(FLValue v) {
    FLValue raw = json_stringify(v);
    if (raw.tag != FL_STRING || !raw.obj) return raw;
    const char* in = ((FLString*)raw.obj)->data;
    size_t ilen = strlen(in);
    size_t cap = ilen * 4 + 64;
    char* out = malloc(cap);
    size_t oi = 0;
    int indent = 0;
    int in_str = 0;
    for (size_t i = 0; i < ilen; i++) {
        char c = in[i];
        if (in_str) {
            out[oi++] = c;
            if (c == '\\' && i + 1 < ilen) { out[oi++] = in[++i]; }
            else if (c == '"') in_str = 0;
        } else if (c == '"') {
            out[oi++] = c; in_str = 1;
        } else if (c == '{' || c == '[') {
            out[oi++] = c;
            if (i + 1 < ilen && in[i+1] != '}' && in[i+1] != ']') {
                out[oi++] = '\n'; indent++;
                for (int k = 0; k < indent * 2; k++) out[oi++] = ' ';
            }
        } else if (c == '}' || c == ']') {
            if (oi > 0 && out[oi-1] != '\n') {
                out[oi++] = '\n'; indent--;
                for (int k = 0; k < indent * 2; k++) out[oi++] = ' ';
            } else { indent--; }
            out[oi++] = c;
        } else if (c == ',') {
            out[oi++] = c; out[oi++] = '\n';
            for (int k = 0; k < indent * 2; k++) out[oi++] = ' ';
        } else if (c == ':') {
            out[oi++] = c; out[oi++] = ' ';
        } else if (c != ' ' && c != '\n' && c != '\t') {
            out[oi++] = c;
        }
        if (oi + 256 >= cap) { cap *= 2; out = realloc(out, cap); }
    }
    out[oi] = '\0';
    FLValue result = fl_str_val(out);
    free(out);
    return result;
}

/* ── SMTP ── (smtp.c 구현 참조) */
extern FLValue fl_smtp_send(FLValue from, FLValue to, FLValue subject, FLValue body);
extern FLValue fl_smtp_send_html(FLValue from, FLValue to, FLValue subject, FLValue html);
extern FLValue fl_smtp_test(FLValue to);

FLValue smtp_send(FLValue from, FLValue to, FLValue subject, FLValue body) {
    return fl_smtp_send(from, to, subject, body);
}
FLValue smtp_send_html(FLValue from, FLValue to, FLValue subject, FLValue html) {
    return fl_smtp_send_html(from, to, subject, html);
}
FLValue smtp_test(FLValue to) { return fl_smtp_test(to); }

/* ── 프로세스/쉘 실행 ── (process.c 구현 참조) */
extern FLValue _fl_process_run(FLValue cmd);
FLValue shell_run(FLValue cmd) { return _fl_process_run(cmd); }

/* ── assoc-in / update-in ── */
static FLValue fl_assoc_in_impl(FLValue m, FLValue* keys, int klen, int ki, FLValue val) {
    if (ki >= klen) return val;
    FLValue key = keys[ki];
    if (ki == klen - 1) return fl_map_set(m, key, val);
    FLValue sub = (m.tag == FL_MAP) ? fl_map_get(m, key) : fl_nil();
    if (sub.tag != FL_MAP) sub = fl_map_new();
    return fl_map_set(m, key, fl_assoc_in_impl(sub, keys, klen, ki + 1, val));
}

FLValue fl_assoc_in(FLValue m, FLValue keys, FLValue val) {
    if (keys.tag != FL_VECTOR) return fl_map_set(m, keys, val);
    FLVector* kv = (FLVector*)keys.obj;
    if (kv->len == 0) return val;
    return fl_assoc_in_impl(m, kv->data, (int)kv->len, 0, val);
}

FLValue fl_update_in(FLValue m, FLValue keys, FLValue fn) {
    if (keys.tag != FL_VECTOR) {
        FLValue cur = (m.tag == FL_MAP) ? fl_map_get(m, keys) : fl_nil();
        FLValue nv = fl_fn_call(fn, 1, &cur);
        return fl_map_set(m, keys, nv);
    }
    FLVector* kv = (FLVector*)keys.obj;
    if (kv->len == 0) return m;
    /* get current value at path */
    FLValue cur = m;
    for (uint32_t i = 0; i < kv->len; i++) {
        if (cur.tag != FL_MAP) { cur = fl_nil(); break; }
        cur = fl_map_get(cur, kv->data[i]);
    }
    FLValue nv = fl_fn_call(fn, 1, &cur);
    return fl_assoc_in_impl(m, kv->data, (int)kv->len, 0, nv);
}

/* ── fl_inspect / pp ── */
static int fl_inspect_write(FLValue v, char* buf, int pos, int sz, int depth) {
    if (pos >= sz - 4) { memcpy(buf + pos, "...", 3); return pos + 3; }
    switch (v.tag) {
        case FL_NIL:    return pos + snprintf(buf + pos, sz - pos, "nil");
        case FL_BOOL:   return pos + snprintf(buf + pos, sz - pos, "%s", v.b ? "true" : "false");
        case FL_INT:    return pos + snprintf(buf + pos, sz - pos, "%lld", (long long)v.i);
        case FL_FLOAT:  return pos + snprintf(buf + pos, sz - pos, "%g", v.f);
        case FL_STRING: {
            const char* s = ((FLString*)v.obj)->data;
            return pos + snprintf(buf + pos, sz - pos, "\"%s\"", s);
        }
        case FL_VECTOR: {
            FLVector* vec = (FLVector*)v.obj;
            buf[pos++] = '[';
            for (uint32_t i = 0; i < vec->len && pos < sz - 4; i++) {
                if (i) { buf[pos++] = ' '; }
                pos = fl_inspect_write(vec->data[i], buf, pos, sz, depth + 1);
            }
            if (pos < sz - 1) buf[pos++] = ']';
            buf[pos] = '\0';
            return pos;
        }
        case FL_MAP: {
            FLMap* mp = (FLMap*)v.obj;
            buf[pos++] = '{';
            for (uint32_t i = 0; i < mp->len && pos < sz - 4; i++) {
                if (i) { buf[pos++] = ' '; }
                pos = fl_inspect_write(mp->entries[i].key, buf, pos, sz, depth + 1);
                buf[pos++] = ' ';
                pos = fl_inspect_write(mp->entries[i].val, buf, pos, sz, depth + 1);
            }
            if (pos < sz - 1) buf[pos++] = '}';
            buf[pos] = '\0';
            return pos;
        }
        case FL_FN: return pos + snprintf(buf + pos, sz - pos, "#<fn>");
        default:    return pos + snprintf(buf + pos, sz - pos, "#<?>");
    }
}

FLValue fl_inspect(FLValue v) {
    char buf[8192];
    int len = fl_inspect_write(v, buf, 0, (int)sizeof(buf), 0);
    buf[len] = '\0';
    return fl_str_val(buf);
}

FLValue fl_pp(FLValue v) {
    FLValue s = fl_inspect(v);
    fprintf(stderr, "[pp] %s\n", ((FLString*)s.obj)->data);
    return v;
}

/* ── format ── */
FLValue fl_format(FLValue fmt_v, FLValue args) {
    if (fmt_v.tag != FL_STRING) return fl_str_val("");
    const char* fmt = ((FLString*)fmt_v.obj)->data;
    FLValue argv[64]; int argc = 0;
    if (args.tag == FL_VECTOR) {
        FLVector* vp = (FLVector*)args.obj;
        for (uint32_t i = 0; i < vp->len && i < 64; i++) argv[argc++] = vp->data[i];
    } else { argv[argc++] = args; }
    char out[8192]; int op = 0, ai = 0;
    for (int i = 0; fmt[i] && op < (int)sizeof(out) - 128; i++) {
        if (fmt[i] != '%') { out[op++] = fmt[i]; continue; }
        i++;
        char spec[32] = {'%'}; int sp = 1;
        while (fmt[i] && strchr("-+0 #", fmt[i])) spec[sp++] = fmt[i++];
        while (fmt[i] && isdigit((unsigned char)fmt[i])) spec[sp++] = fmt[i++];
        if (fmt[i] == '.') { spec[sp++] = fmt[i++]; while (fmt[i] && isdigit((unsigned char)fmt[i])) spec[sp++] = fmt[i++]; }
        char conv = fmt[i]; spec[sp++] = conv; spec[sp] = '\0';
        FLValue av = (ai < argc) ? argv[ai++] : fl_nil();
        char tmp[512];
        switch (conv) {
            case 'd': case 'i': {
                /* spec = "%[flags][width][.prec]d" → "%[flags][width][.prec]lld" */
                char s2[64]; memcpy(s2, spec, sp - 1); s2[sp-1] = 'l'; s2[sp] = 'l'; s2[sp+1] = conv; s2[sp+2] = '\0';
                snprintf(tmp, sizeof(tmp), s2, (long long)(av.tag == FL_INT ? av.i : (int64_t)av.f));
                break;
            }
            case 'f': case 'e': case 'g': case 'E': case 'G':
                snprintf(tmp, sizeof(tmp), spec, av.tag == FL_FLOAT ? av.f : (double)av.i);
                break;
            case 's': {
                char vbuf[512];
                const char* s = av.tag == FL_STRING ? ((FLString*)av.obj)->data : fl_to_str(av, vbuf, sizeof(vbuf));
                snprintf(tmp, sizeof(tmp), spec, s);
                break;
            }
            case '%': tmp[0] = '%'; tmp[1] = '\0'; ai--; break;
            default:  snprintf(tmp, sizeof(tmp), "%s", spec); ai--; break;
        }
        int tl = (int)strlen(tmp);
        if (op + tl < (int)sizeof(out) - 1) { memcpy(out + op, tmp, tl); op += tl; }
    }
    out[op] = '\0';
    return fl_str_val(out);
}

/* ── merge ── */
FLValue fl_merge(FLValue m1, FLValue m2) {
    if (m1.tag != FL_MAP) m1 = fl_map_new();
    if (m2.tag != FL_MAP) return m1;
    FLMap* mp = (FLMap*)m2.obj;
    for (uint32_t i = 0; i < mp->len; i++)
        m1 = fl_map_set(m1, mp->entries[i].key, mp->entries[i].val);
    return m1;
}

/* ── base64 ── */
static const char B64_ENC[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int b64_dec(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62; if (c == '/') return 63; return -1;
}
FLValue fl_base64_encode(FLValue v) {
    if (v.tag != FL_STRING) return fl_str_val("");
    const unsigned char* s = (const unsigned char*)((FLString*)v.obj)->data;
    size_t n = strlen((char*)s);
    size_t olen = ((n + 2) / 3) * 4 + 1;
    char* out = malloc(olen); size_t j = 0;
    for (size_t i = 0; i < n; i += 3) {
        unsigned char a = s[i];
        unsigned char b = (i+1 < n) ? s[i+1] : 0;
        unsigned char c = (i+2 < n) ? s[i+2] : 0;
        out[j++] = B64_ENC[a >> 2];
        out[j++] = B64_ENC[((a & 3) << 4) | (b >> 4)];
        out[j++] = (i+1 < n) ? B64_ENC[((b & 0xf) << 2) | (c >> 6)] : '=';
        out[j++] = (i+2 < n) ? B64_ENC[c & 0x3f] : '=';
    }
    out[j] = '\0';
    FLValue r = fl_str_val(out); free(out); return r;
}
FLValue fl_base64_decode(FLValue v) {
    if (v.tag != FL_STRING) return fl_str_val("");
    const char* s = ((FLString*)v.obj)->data;
    size_t n = strlen(s);
    char* out = malloc(n / 4 * 3 + 4); size_t j = 0;
    for (size_t i = 0; i + 3 < n; i += 4) {
        int a = b64_dec(s[i]), b = b64_dec(s[i+1]), c = b64_dec(s[i+2]), d = b64_dec(s[i+3]);
        if (a < 0 || b < 0) break;
        out[j++] = (a << 2) | (b >> 4);
        if (s[i+2] != '=' && c >= 0) out[j++] = ((b & 0xf) << 4) | (c >> 2);
        if (s[i+3] != '=' && d >= 0) out[j++] = ((c & 3) << 6) | d;
    }
    out[j] = '\0';
    FLValue r = fl_str_val(out); free(out); return r;
}

/* ── dissoc / keys / vals ── */

FLValue fl_dissoc(FLValue m, FLValue key) {
    if (m.tag != FL_MAP) return m;
    FLMap* src = (FLMap*)m.obj;
    FLValue result = fl_map_new();
    for (uint32_t i = 0; i < src->len; i++) {
        FLValue k = src->entries[i].key;
        if (!fl_truthy(fl_eq(k, key)))
            result = fl_map_set(result, k, src->entries[i].val);
    }
    return result;
}

FLValue fl_keys(FLValue m) {
    if (m.tag != FL_MAP) return fl_vec_new();
    FLMap* src = (FLMap*)m.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < src->len; i++)
        result = fl_vec_push(result, src->entries[i].key);
    return result;
}

FLValue fl_vals(FLValue m) {
    if (m.tag != FL_MAP) return fl_vec_new();
    FLMap* src = (FLMap*)m.obj;
    FLValue result = fl_vec_new();
    for (uint32_t i = 0; i < src->len; i++)
        result = fl_vec_push(result, src->entries[i].val);
    return result;
}

/* ── select-keys / update ── */

FLValue fl_select_keys(FLValue m, FLValue ks) {
    if (m.tag != FL_MAP) return fl_map_new();
    FLValue result = fl_map_new();
    /* ks는 벡터 또는 리스트 */
    FLVector* kv = (FLVector*)ks.obj;
    for (uint32_t i = 0; i < kv->len; i++) {
        FLValue k = kv->data[i];
        FLValue v = fl_map_get(m, k);
        if (v.tag != FL_NIL)
            result = fl_map_set(result, k, v);
    }
    return result;
}

FLValue fl_update(FLValue m, FLValue key, FLValue fn) {
    FLValue cur = fl_map_get(m, key);
    FLValue args[1] = { cur };
    FLValue updated = fl_fn_call(fn, 1, args);
    return fl_map_set(m, key, updated);
}

/* ── 73서버 cgc-bin 호환: fl_db_* → fxb_sqlite_* 매핑 ── */
FLValue fl_db_open(FLValue path)                              { return fxb_sqlite_open(path); }
FLValue fl_db_query(FLValue db, FLValue sql, FLValue params)  { return fxb_sqlite_query_p(db, sql, params); }
FLValue fl_db_exec(FLValue db, FLValue sql, FLValue params)   { return fxb_sqlite_exec_p(db, sql, params); }

/* user package functions: implemented in user-fns.c, exposed as FLValue globals via builtins shim */

/* ── 매번 고쳐쓰던 것들 자동화 ───────────────────────────────────── */


/* req-param: (get (get $req "query") key) 패턴 */
FLValue req_param(FLValue req, FLValue key) {
    FLValue query = fl_map_get(req, fl_str_val("query"));
    return fl_map_get(query, key);
}

/* req-header: (get (get $req "headers") key) 패턴 */
FLValue req_header(FLValue req, FLValue key) {
    FLValue headers = fl_map_get(req, fl_str_val("headers"));
    return fl_map_get(headers, key);
}

/* req-body: request body 접근 */
FLValue req_body(FLValue req) {
    return fl_map_get(req, fl_str_val("body"));
}

/* ok?: nil/false 판별 */
FLValue ok_p(FLValue v) {
    if (v.tag == FL_NIL)  return fl_bool(0);
    if (v.tag == FL_BOOL) return fl_bool(v.b);
    return fl_bool(1);
}

/* set: (set m key val) → assoc 동일. cgc-bin이 3-arg set으로 emit */
FLValue set(FLValue m, FLValue key, FLValue val) {
    return fl_map_set(m, key, val);
}
