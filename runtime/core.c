#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ── try 스택은 error.c에서 __thread로 정의 ── */


/* ── 값 생성 ── */

FLValue fl_int(int64_t v) {
    FLValue r; r.tag = FL_INT; r.i = v; return r;
}

FLValue fl_float(double v) {
    FLValue r; r.tag = FL_FLOAT; r.f = v; return r;
}

/*
 * fl_str_val: 요청 처리 중에는 Arena에서 할당, 그 외엔 process-lifetime registry
 *
 * Arena 할당은 요청 완료 시 fl_arena_end()로 일괄 해제.
 * 요청 밖 할당은 runtime process-lifetime registry가 소유하고
 * fl_runtime_shutdown()에서 해제한다.
 *
 * 둘을 섞어도 안전한 이유: FLString을 직접 free하는 코드가 없음.
 * Arena 객체는 arena_end 후 재사용되므로 dangling 참조 주의.
 * → 요청 핸들러 외부(전역 define)는 process-lifetime registry가 안전하다.
 */
FLValue fl_str_val(const char* s) {
    size_t len = strlen(s);
    size_t total = sizeof(FLString) + len + 1;
    FLString* obj = (FLString*)fl_arena_alloc(total);
    if (!obj) obj = malloc(total);   /* arena 없을 때 폴백 */
    obj->base.type = FL_STRING;
    obj->base.rc = 1;
    obj->len = (uint32_t)len;
    memcpy(obj->data, s, len + 1);
    FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)obj; return r;
}

/* 바이너리 안전 문자열 생성 (len 바이트 그대로, null 바이트 포함 가능) */
FLValue fl_str_val_n(const char* data, uint32_t len) {
    size_t total = sizeof(FLString) + len + 1;
    FLString* obj = (FLString*)fl_arena_alloc(total);
    if (!obj) obj = malloc(total);
    obj->base.type = FL_STRING;
    obj->base.rc = 1;
    obj->len = len;
    memcpy(obj->data, data, len);
    obj->data[len] = '\0';
    FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)obj; return r;
}

FLValue fl_bool(bool v) {
    FLValue r; r.tag = FL_BOOL; r.b = v; return r;
}

FLValue fl_nil(void) {
    FLValue r; r.tag = FL_NIL; r.i = 0; return r;
}

/* ── 조건 판별 ── */

bool fl_truthy(FLValue v) {
    switch (v.tag) {
        case FL_BOOL:   return v.b;
        case FL_NIL:    return false;
        case FL_INT:    return v.i != 0;
        case FL_FLOAT:  return v.f != 0.0;
        default:        return true;
    }
}

/* ── 타입 이름 (에러 메시지용) ── */

const char* fl_type_name(FLValue v) {
    switch (v.tag) {
        case FL_INT:    return "int";
        case FL_FLOAT:  return "float";
        case FL_BOOL:   return "bool";
        case FL_NIL:    return "nil";
        case FL_STRING: return "string";
        case FL_VECTOR: return "array";
        case FL_MAP:    return "map";
        case FL_FN:     return "function";
        default:        return "unknown";
    }
}

static void fl_type_error(const char* op, const char* expected, FLValue got) {
    char buf[256];
    snprintf(buf, sizeof(buf), "TypeError: %s — %s 필요, %s 제공",
             op, expected, fl_type_name(got));
    fl_throw(fl_str_val(buf));
    exit(1); /* unreachable — fl_throw exits or longjmps */
}

static int fl_is_num(FLValue v) {
    return v.tag == FL_INT || v.tag == FL_FLOAT;
}

/* ── 문자열 변환 (출력용) ── */

/* forward declaration */
const char* fl_to_str(FLValue v, char* buf, size_t sz);

/* Convert a FreeLang string to a printable C-string representation.
 * Embedded NUL is escaped instead of being treated as the end of the value. */
static int write_string_repr(FLString* string, char* buf, int pos, int sz) {
    if (pos >= sz - 3) return pos;
    buf[pos++] = '"';
    for (uint32_t i = 0; i < string->len; i++) {
        if (string->data[i] == '\0') {
            if (pos + 3 >= sz) { memcpy(buf + pos, "...", 3); return pos + 3; }
            buf[pos++] = '\\';
            buf[pos++] = '0';
        } else {
            if (pos + 2 >= sz) { memcpy(buf + pos, "...", 3); return pos + 3; }
            buf[pos++] = string->data[i];
        }
    }
    if (pos < sz - 1) buf[pos++] = '"';
    return pos;
}

/* repr 모드로 값을 buf[pos]에 쓰고 새 pos 반환 (문자열은 따옴표 포함) */
static int write_repr(FLValue v, char* buf, int pos, int sz) {
    if (pos >= sz - 4) return pos;
    if (v.tag == FL_STRING) {
        return write_string_repr((FLString*)v.obj, buf, pos, sz);
    }
    char tmp[512];
    const char* s = fl_to_str(v, tmp, sizeof(tmp));
    int slen = (int)strlen(s);
    if (pos + slen >= sz - 2) { memcpy(buf+pos,"...",3); return pos+3; }
    memcpy(buf+pos, s, slen);
    return pos + slen;
}

const char* fl_to_str(FLValue v, char* buf, size_t sz) {
    switch (v.tag) {
        case FL_INT:    snprintf(buf, sz, "%lld", (long long)v.i); return buf;
        case FL_FLOAT:  snprintf(buf, sz, "%g", v.f); return buf;
        case FL_BOOL:   return v.b ? "true" : "false";
        case FL_NIL:    return "nil";
        case FL_STRING: {
            int pos = write_string_repr((FLString*)v.obj, buf, 0, (int)sz);
            buf[pos < (int)sz ? pos : (int)sz - 1] = '\0';
            return buf;
        }
        case FL_VECTOR: {
            FLVector* vec = (FLVector*)v.obj;
            int pos = 0;
            buf[pos++] = '[';
            for (uint32_t i = 0; i < vec->len && pos < (int)sz - 4; i++) {
                if (i) { buf[pos++] = ','; buf[pos++] = ' '; }
                pos = write_repr(vec->data[i], buf, pos, (int)sz);
            }
            if (pos < (int)sz - 1) buf[pos++] = ']';
            buf[pos] = '\0';
            return buf;
        }
        case FL_MAP: {
            FLMap* mp = (FLMap*)v.obj;
            int pos = 0;
            buf[pos++] = '{';
            for (uint32_t i = 0; i < mp->len && pos < (int)sz - 4; i++) {
                if (i) { buf[pos++] = ','; buf[pos++] = ' '; }
                pos = write_repr(mp->entries[i].key, buf, pos, (int)sz);
                if (pos < (int)sz - 3) { buf[pos++] = ':'; buf[pos++] = ' '; }
                pos = write_repr(mp->entries[i].val, buf, pos, (int)sz);
            }
            if (pos < (int)sz - 1) buf[pos++] = '}';
            buf[pos] = '\0';
            return buf;
        }
        case FL_FN: return "#<fn>";
        default: return "";
    }
}

/* ── 산술 ── */

/* String values retain their canonical byte length. Non-string display values are C text. */
static size_t fl_display_bytes(FLValue v, char* buf, size_t sz, const char** out) {
    if (v.tag == FL_STRING) {
        FLString* s = (FLString*)v.obj;
        *out = s->data;
        return s->len;
    }
    *out = fl_to_str(v, buf, sz);
    return strlen(*out);
}

FLValue fl_add(FLValue a, FLValue b) {
    if (a.tag == FL_STRING || b.tag == FL_STRING) {
        char ba[256], bb[256];
        const char* sa;
        const char* sb;
        size_t la = fl_display_bytes(a, ba, sizeof(ba), &sa);
        size_t lb = fl_display_bytes(b, bb, sizeof(bb), &sb);
        FLString* obj = fl_arena_alloc(sizeof(FLString) + la + lb + 1);
        obj->base.type = FL_STRING; obj->base.rc = 1;
        obj->len = (uint32_t)(la + lb);
        memcpy(obj->data, sa, la);
        memcpy(obj->data + la, sb, lb);
        obj->data[la + lb] = '\0';
        FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)obj; return r;
    }
    if (!fl_is_num(a)) fl_type_error("+ (add)", "number", a);
    if (!fl_is_num(b)) fl_type_error("+ (add)", "number", b);
    if (a.tag == FL_FLOAT || b.tag == FL_FLOAT) {
        double av = (a.tag == FL_FLOAT) ? a.f : (double)a.i;
        double bv = (b.tag == FL_FLOAT) ? b.f : (double)b.i;
        return fl_float(av + bv);
    }
    return fl_int(a.i + b.i);
}

FLValue fl_sub(FLValue a, FLValue b) {
    if (!fl_is_num(a)) fl_type_error("- (sub)", "number", a);
    if (!fl_is_num(b)) fl_type_error("- (sub)", "number", b);
    if (a.tag == FL_FLOAT || b.tag == FL_FLOAT) {
        double av = (a.tag == FL_FLOAT) ? a.f : (double)a.i;
        double bv = (b.tag == FL_FLOAT) ? b.f : (double)b.i;
        return fl_float(av - bv);
    }
    return fl_int(a.i - b.i);
}

FLValue fl_mul(FLValue a, FLValue b) {
    if (!fl_is_num(a)) fl_type_error("* (mul)", "number", a);
    if (!fl_is_num(b)) fl_type_error("* (mul)", "number", b);
    if (a.tag == FL_FLOAT || b.tag == FL_FLOAT) {
        double av = (a.tag == FL_FLOAT) ? a.f : (double)a.i;
        double bv = (b.tag == FL_FLOAT) ? b.f : (double)b.i;
        return fl_float(av * bv);
    }
    return fl_int(a.i * b.i);
}

FLValue fl_div(FLValue a, FLValue b) {
    if (!fl_is_num(a)) fl_type_error("/ (div)", "number", a);
    if (!fl_is_num(b)) fl_type_error("/ (div)", "number", b);
    if (a.tag == FL_FLOAT || b.tag == FL_FLOAT) {
        double av = (a.tag == FL_FLOAT) ? a.f : (double)a.i;
        double bv = (b.tag == FL_FLOAT) ? b.f : (double)b.i;
        if (bv == 0.0) fl_throw(fl_str_val("ArithmeticError: 0으로 나눌 수 없습니다"));
        return fl_float(av / bv);
    }
    if (b.i == 0) fl_throw(fl_str_val("ArithmeticError: 0으로 나눌 수 없습니다"));
    return fl_int(a.i / b.i);
}

FLValue fl_mod(FLValue a, FLValue b) {
    long long ai = (a.tag == FL_FLOAT) ? (long long)a.f : a.i;
    long long bi = (b.tag == FL_FLOAT) ? (long long)b.f : b.i;
    if (bi == 0) { fputs("error: mod by zero\n", stderr); exit(1); }
    return fl_int(ai % bi);
}

/* ── 비교 ── */

FLValue fl_eq(FLValue a, FLValue b) {
    if (a.tag == FL_NIL && b.tag == FL_NIL) return fl_bool(true);
    if (a.tag != b.tag) {
        if ((a.tag == FL_INT || a.tag == FL_FLOAT) && (b.tag == FL_INT || b.tag == FL_FLOAT)) {
            double av = (a.tag == FL_FLOAT) ? a.f : (double)a.i;
            double bv = (b.tag == FL_FLOAT) ? b.f : (double)b.i;
            return fl_bool(av == bv);
        }
        return fl_bool(false);
    }
    switch (a.tag) {
        case FL_INT:    return fl_bool(a.i == b.i);
        case FL_FLOAT:  return fl_bool(a.f == b.f);
        case FL_BOOL:   return fl_bool(a.b == b.b);
        case FL_STRING: {
            FLString* sa = (FLString*)a.obj;
            FLString* sb = (FLString*)b.obj;
            return fl_bool(sa->len == sb->len &&
                           memcmp(sa->data, sb->data, sa->len) == 0);
        }
        default:        return fl_bool(false);
    }
}

static double fl_num(FLValue v) {
    return (v.tag == FL_FLOAT) ? v.f : (double)v.i;
}

static int fl_str_cmp(FLValue a, FLValue b) {
    FLString* sa = (a.tag == FL_STRING && a.obj) ? (FLString*)a.obj : NULL;
    FLString* sb = (b.tag == FL_STRING && b.obj) ? (FLString*)b.obj : NULL;
    size_t alen = sa ? sa->len : 0;
    size_t blen = sb ? sb->len : 0;
    size_t common = alen < blen ? alen : blen;
    int result = common ? memcmp(sa->data, sb->data, common) : 0;
    if (result) return result;
    return (alen > blen) - (alen < blen);
}
FLValue fl_lt(FLValue a, FLValue b)  {
    if (a.tag==FL_STRING && b.tag==FL_STRING) return fl_bool(fl_str_cmp(a,b) <  0);
    return fl_bool(fl_num(a) <  fl_num(b));
}
FLValue fl_gt(FLValue a, FLValue b)  {
    if (a.tag==FL_STRING && b.tag==FL_STRING) return fl_bool(fl_str_cmp(a,b) >  0);
    return fl_bool(fl_num(a) >  fl_num(b));
}
FLValue fl_lte(FLValue a, FLValue b) {
    if (a.tag==FL_STRING && b.tag==FL_STRING) return fl_bool(fl_str_cmp(a,b) <= 0);
    return fl_bool(fl_num(a) <= fl_num(b));
}
FLValue fl_gte(FLValue a, FLValue b) {
    if (a.tag==FL_STRING && b.tag==FL_STRING) return fl_bool(fl_str_cmp(a,b) >= 0);
    return fl_bool(fl_num(a) >= fl_num(b));
}
FLValue fl_neq(FLValue a, FLValue b) { return fl_bool(!fl_truthy(fl_eq(a, b))); }

/* ── 논리 ── */

FLValue fl_not(FLValue a)             { return fl_bool(!fl_truthy(a)); }
FLValue fl_and(FLValue a, FLValue b)  { return fl_truthy(a) ? b : a; }
FLValue fl_or(FLValue a, FLValue b)   { return fl_truthy(a) ? a : b; }

/* ── 문자열 (str n-인자) ── */

FLValue fl_str_n(int count, ...) {
    if (count == 0) return fl_str_val("");
    va_list ap;
    va_start(ap, count);
    char bufs[count][64];
    const char* parts[count];
    size_t lens[count];
    size_t total = 0;
    for (int i = 0; i < count; i++) {
        FLValue v = va_arg(ap, FLValue);
        lens[i] = fl_display_bytes(v, bufs[i], sizeof(bufs[i]), &parts[i]);
        total += lens[i];
    }
    va_end(ap);
    FLString* obj = fl_arena_alloc(sizeof(FLString) + total + 1);
    obj->base.type = FL_STRING; obj->base.rc = 1;
    obj->len = (uint32_t)total;
    size_t off = 0;
    for (int i = 0; i < count; i++) {
        memcpy(obj->data + off, parts[i], lens[i]);
        off += lens[i];
    }
    obj->data[total] = '\0';
    FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)obj; return r;
}

/* ── I/O ── */

FLValue fl_println(FLValue v) {
    if (v.tag == FL_STRING) {
        FLString* s = (FLString*)v.obj;
        fwrite(s->data, 1, s->len, stdout);
        fputc('\n', stdout);
    } else {
        char buf[4096];
        puts(fl_to_str(v, buf, sizeof(buf)));
    }
    return fl_nil();
}

FLValue fl_print(FLValue v) {
    if (v.tag == FL_STRING) {
        FLString* s = (FLString*)v.obj;
        fwrite(s->data, 1, s->len, stdout);
    } else {
        char buf[4096];
        fputs(fl_to_str(v, buf, sizeof(buf)), stdout);
    }
    return fl_nil();
}

/* ── 파일 I/O ── */

/* 결과 문자열은 request arena 또는 process-lifetime registry가 소유한다. */

FLValue fl_file_read(FLValue path) {
    if (path.tag != FL_STRING || fl_string_has_embedded_nul(path)) return fl_nil();
    const char* p = ((FLString*)path.obj)->data;
    FILE* f = fopen(p, "rb");
    if (!f) return fl_nil();
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    FLString* obj = fl_arena_alloc(sizeof(FLString) + (size_t)sz + 1);
    obj->base.type = FL_STRING; obj->base.rc = 1;
    obj->len = (uint32_t)sz;
    fread(obj->data, 1, (size_t)sz, f);
    obj->data[sz] = '\0';
    fclose(f);
    FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)obj; return r;
}

FLValue fl_file_write(FLValue path, FLValue content) {
    if (path.tag != FL_STRING || fl_string_has_embedded_nul(path)) return fl_nil();
    const char* p = ((FLString*)path.obj)->data;
    char buf[64];
    const char* s;
    size_t len = fl_display_bytes(content, buf, sizeof(buf), &s);
    FILE* f = fopen(p, "wb");
    if (!f) return fl_nil();
    fwrite(s, 1, len, f);
    fclose(f);
    return fl_nil();
}

/* ── js_escape / fl_html_escape ── */
FLValue js_escape(FLValue s) {
    if (s.tag != FL_STRING) return s;
    FLString* input = (FLString*)s.obj;
    const char* src = input->data;
    size_t n = input->len;
    /* 최대 6배 확장 */
    char* buf = (char*)malloc(n * 6 + 1);
    if (!buf) return s;
    char* dst = buf;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)src[i];
        switch (c) {
            case '"':  *dst++ = '\\'; *dst++ = '"';  break;
            case '\\': *dst++ = '\\'; *dst++ = '\\'; break;
            case '\n': *dst++ = '\\'; *dst++ = 'n';  break;
            case '\r': *dst++ = '\\'; *dst++ = 'r';  break;
            case '\t': *dst++ = '\\'; *dst++ = 't';  break;
            default:   *dst++ = (char)c;             break;
        }
    }
    *dst = '\0';
    FLValue r = fl_str_val_n(buf, (uint32_t)(dst - buf));
    free(buf);
    return r;
}

FLValue fl_html_escape(FLValue s) {
    if (s.tag != FL_STRING) return s;
    FLString* input = (FLString*)s.obj;
    const char* src = input->data;
    size_t n = input->len;
    char* buf = (char*)malloc(n * 6 + 1);
    if (!buf) return s;
    char* dst = buf;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)src[i];
        switch (c) {
            case '&':  memcpy(dst, "&amp;",  5); dst += 5; break;
            case '<':  memcpy(dst, "&lt;",   4); dst += 4; break;
            case '>':  memcpy(dst, "&gt;",   4); dst += 4; break;
            case '"':  memcpy(dst, "&quot;", 6); dst += 6; break;
            case '\'': memcpy(dst, "&#39;",  5); dst += 5; break;
            default:   *dst++ = (char)c;               break;
        }
    }
    *dst = '\0';
    FLValue r = fl_str_val_n(buf, (uint32_t)(dst - buf));
    free(buf);
    return r;
}
