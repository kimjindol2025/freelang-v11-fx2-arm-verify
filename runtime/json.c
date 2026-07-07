#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char* json_skip_ws(const char* p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static FLValue json_parse_value(const char** pp);

static FLValue json_parse_string(const char** pp) {
    const char* p = *pp + 1; /* skip opening " */
    size_t cap = 64, len = 0;
    char* buf = malloc(cap);
    while (*p && *p != '"') {
        if (*p == '\\') {
            p++;
            char c = *p++;
            if (c=='n') buf[len++]='\n';
            else if (c=='t') buf[len++]='\t';
            else if (c=='r') buf[len++]='\r';
            else if (c=='b') buf[len++]='\b';
            else if (c=='f') buf[len++]='\f';
            else if (c=='u') {
                /* \uXXXX → 코드포인트 → UTF-8 (서로게이트 페어 처리) */
                int cp = 0;
                for (int i=0;i<4 && *p;i++) {
                    char h=*p++; cp<<=4;
                    if (h>='0'&&h<='9') cp|=h-'0';
                    else if (h>='a'&&h<='f') cp|=h-'a'+10;
                    else if (h>='A'&&h<='F') cp|=h-'A'+10;
                }
                if (cp>=0xD800 && cp<=0xDBFF && p[0]=='\\' && p[1]=='u') {
                    p+=2; int lo=0;
                    for (int i=0;i<4 && *p;i++) {
                        char h=*p++; lo<<=4;
                        if (h>='0'&&h<='9') lo|=h-'0';
                        else if (h>='a'&&h<='f') lo|=h-'a'+10;
                        else if (h>='A'&&h<='F') lo|=h-'A'+10;
                    }
                    cp = 0x10000 + ((cp-0xD800)<<10) + (lo-0xDC00);
                }
                if (cp<0x80) buf[len++]=cp;
                else if (cp<0x800) { buf[len++]=0xC0|(cp>>6); buf[len++]=0x80|(cp&0x3F); }
                else if (cp<0x10000) { buf[len++]=0xE0|(cp>>12); buf[len++]=0x80|((cp>>6)&0x3F); buf[len++]=0x80|(cp&0x3F); }
                else { buf[len++]=0xF0|(cp>>18); buf[len++]=0x80|((cp>>12)&0x3F); buf[len++]=0x80|((cp>>6)&0x3F); buf[len++]=0x80|(cp&0x3F); }
            }
            else buf[len++]=c;
        } else {
            buf[len++] = *p++;
        }
        if (len + 4 >= cap) { cap *= 2; buf = realloc(buf, cap); }
    }
    if (*p == '"') p++;
    buf[len] = 0;
    FLValue r = fl_str_val(buf); free(buf);
    *pp = p; return r;
}

static FLValue json_parse_array(const char** pp) {
    const char* p = *pp + 1; /* skip [ */
    FLValue vec = fl_vec_new();
    p = json_skip_ws(p);
    if (*p == ']') { *pp = p+1; return vec; }
    while (*p) {
        p = json_skip_ws(p);
        FLValue v = json_parse_value(&p);
        vec = fl_vec_push(vec, v);
        p = json_skip_ws(p);
        if (*p == ',') p++;
        else break;
    }
    if (*p == ']') p++;
    *pp = p; return vec;
}

static FLValue json_parse_object(const char** pp) {
    const char* p = *pp + 1; /* skip { */
    FLValue map = fl_map_new();
    p = json_skip_ws(p);
    if (*p == '}') { *pp = p+1; return map; }
    while (*p) {
        p = json_skip_ws(p);
        if (*p != '"') break;
        FLValue key = json_parse_string(&p);
        p = json_skip_ws(p);
        if (*p == ':') p++;
        p = json_skip_ws(p);
        FLValue val = json_parse_value(&p);
        map = fl_map_set(map, key, val);
        p = json_skip_ws(p);
        if (*p == ',') p++;
        else break;
    }
    if (*p == '}') p++;
    *pp = p; return map;
}

static FLValue json_parse_value(const char** pp) {
    const char* p = json_skip_ws(*pp);
    if (*p == '"') { FLValue r = json_parse_string(&p); *pp = p; return r; }
    if (*p == '[') { FLValue r = json_parse_array(&p);  *pp = p; return r; }
    if (*p == '{') { FLValue r = json_parse_object(&p); *pp = p; return r; }
    if (strncmp(p,"null",4)==0)  { *pp=p+4; return fl_nil(); }
    if (strncmp(p,"true",4)==0)  { *pp=p+4; return fl_bool(true); }
    if (strncmp(p,"false",5)==0) { *pp=p+5; return fl_bool(false); }
    /* number */
    char* end; double d = strtod(p, &end);
    if (end > p) {
        *pp = end;
        if (d == (int64_t)d) return fl_int((int64_t)d);
        return fl_float(d);
    }
    *pp = p+1; return fl_nil();
}

FLValue fl_json_parse(FLValue src) {
    if (src.tag != FL_STRING) return fl_nil();
    FLString* s = (FLString*)src.obj;
    const char* p = s->data;
    FLValue result = json_parse_value(&p);
    /* 파싱 후 남은 비공백 문자 있으면 → 잘못된 JSON → throw */
    p = json_skip_ws(p);
    if (*p != '\0') {
        char buf[128];
        snprintf(buf, sizeof(buf), "[json] 파싱 실패: 잘못된 JSON (위치: '%.20s')", p);
        fl_throw(fl_str_val(buf));
    }
    return result;
}

/* fl_json_try_parse: try-catch를 C 레벨에서 처리 → FL try-catch 불필요
 * 반환: [값 nil] = 성공, [nil "에러"] = 실패 (Result 타입)
 * FL에서: (defn json-parse-safe [$s] (json_try_parse $s))  */
FLValue fl_json_try_parse(FLValue src) {
    extern __thread FLTryFrame fl_try_stack[];
    extern __thread int fl_try_top;
    if (fl_try_top >= FL_TRY_MAX) {
        /* try 스택 가득 → 직접 파싱 (에러 가능성 무시) */
        FLValue __fl_arr[2] = {fl_json_parse(src), fl_nil()};
        return fl_vec_from(__fl_arr, 2);
    }
    FLTryFrame* frame = &fl_try_stack[fl_try_top++];
    if (setjmp(frame->buf) == 0) {
        FLValue result = fl_json_parse(src);
        fl_try_top--;
        FLValue __fl_arr[2] = {result, fl_nil()};
        return fl_vec_from(__fl_arr, 2);
    } else {
        fl_try_top--;
        FLValue err = frame->err;
        FLValue __fl_arr[2] = {fl_nil(), err};
        return fl_vec_from(__fl_arr, 2);
    }
}
FLValue json_try_parse(FLValue src) { return fl_json_try_parse(src); }

/* ── 동적 버퍼 (realloc 기반, 64KB 시작 → 필요시 2배 확장) ── */
typedef struct { char* data; size_t cap; size_t pos; } JBuf;

static void jbuf_ensure(JBuf* b, size_t need) {
    if (b->pos + need < b->cap) return;
    size_t ncap = b->cap;
    while (ncap <= b->pos + need) ncap *= 2;
    b->data = realloc(b->data, ncap);
    b->cap  = ncap;
}

#define JCAT(fmt, ...) do { \
    jbuf_ensure(b, 64); \
    int _n = snprintf(b->data + b->pos, b->cap - b->pos, fmt, ##__VA_ARGS__); \
    if (_n > 0) { \
        if ((size_t)_n >= b->cap - b->pos) { \
            jbuf_ensure(b, (size_t)_n + 1); \
            _n = snprintf(b->data + b->pos, b->cap - b->pos, fmt, ##__VA_ARGS__); \
        } \
        b->pos += (size_t)_n; \
    } \
} while(0)

static void json_stringify_buf(FLValue v, JBuf* b);

static void json_stringify_buf(FLValue v, JBuf* b) {
    if (v.tag == FL_NIL)    { JCAT("null"); return; }
    if (v.tag == FL_BOOL)   { JCAT("%s", v.b ? "true" : "false"); return; }
    if (v.tag == FL_INT)    { JCAT("%lld", (long long)v.i); return; }
    if (v.tag == FL_FLOAT)  {
        /* 정수값 float은 소수점 없이 출력 (9227465.0 → 9227465) */
        double f = v.f;
        if (f == (double)(long long)f &&
            f >= -9007199254740992.0 && f <= 9007199254740992.0) {
            JCAT("%lld", (long long)f);
        } else {
            JCAT("%.17g", f);
        }
        return;
    }
    if (v.tag == FL_STRING) {
        FLString* s = (FLString*)v.obj;
        jbuf_ensure(b, s->len * 6 + 4);
        b->data[b->pos++] = '"';
        for (uint32_t i = 0; i < s->len; i++) {
            unsigned char c = (unsigned char)s->data[i];
            if      (c == '"')  { b->data[b->pos++]='\\'; b->data[b->pos++]='"';  }
            else if (c == '\\') { b->data[b->pos++]='\\'; b->data[b->pos++]='\\'; }
            else if (c == '\n') { b->data[b->pos++]='\\'; b->data[b->pos++]='n';  }
            else if (c == '\t') { b->data[b->pos++]='\\'; b->data[b->pos++]='t';  }
            else if (c == '\r') { b->data[b->pos++]='\\'; b->data[b->pos++]='r';  }
            else if (c < 0x20) {
                /* JSON 스펙: 0x00-0x1F는 전부 이스케이프 필수 (ANSI ESC 등).
                 * 스택 버퍼에 먼저 만들어서 정확히 6바이트만 복사 —
                 * snprintf의 null 종단자가 b->data 예산(char당 6바이트)을
                 * 넘겨쓰지 않도록. */
                char esc[7];
                snprintf(esc, sizeof(esc), "\\u%04x", c);
                memcpy(b->data + b->pos, esc, 6);
                b->pos += 6;
            }
            else                { b->data[b->pos++] = (char)c; }
        }
        b->data[b->pos++] = '"';
        return;
    }
    if (v.tag == FL_VECTOR) {
        FLVector* vec = (FLVector*)v.obj;
        JCAT("[");
        for (uint32_t i = 0; i < vec->len; i++) {
            if (i) JCAT(",");
            json_stringify_buf(vec->data[i], b);
        }
        JCAT("]"); return;
    }
    if (v.tag == FL_MAP) {
        FLMap* m = (FLMap*)v.obj;
        JCAT("{");
        for (uint32_t i = 0; i < m->len; i++) {
            if (i) JCAT(",");
            json_stringify_buf(m->entries[i].key, b);
            JCAT(":");
            json_stringify_buf(m->entries[i].val, b);
        }
        JCAT("}"); return;
    }
    JCAT("null");
}
#undef JCAT

FLValue fl_json_stringify(FLValue val) {
    JBuf b = { malloc(65536), 65536, 0 };
    json_stringify_buf(val, &b);
    b.data[b.pos] = '\0';
    FLValue r = fl_str_val(b.data);
    free(b.data);
    return r;
}

/* ── 비트 연산 ── */

/* cgc-bin alias: 짧은 이름 → fl_ 래퍼 */
FLValue json_parse(FLValue src)      { return fl_json_parse(src); }
FLValue json_stringify(FLValue val)  { return fl_json_stringify(val); }
