/* ─────────────────────────────────────────────────────────────────
   fx 정규식 — POSIX ERE (regex.h)
   ─────────────────────────────────────────────────────────────────
   함수 목록:
     regex_match   pattern str         → true/false
     regex_find    pattern str         → 매치 문자열 or nil
     regex_find_all pattern str        → [매치1, 매치2, ...]
     regex_replace  pattern str repl   → 결과 문자열 (첫 번째만)
     regex_replace_all pattern str repl→ 결과 문자열 (전체)
     regex_split   pattern str         → ["a","b","c",...]
     regex_groups  pattern str         → ["전체", "그룹1", ...]
 ───────────────────────────────────────────────────────────────── */
#include "runtime.h"
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_GROUPS 16

static const char* fl_cstr(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return NULL;
}

/* ── regex_match pattern str → bool ── */
FLValue regex_match(FLValue pat_v, FLValue str_v) {
    const char* pat = fl_cstr(pat_v);
    const char* str = fl_cstr(str_v);
    if (!pat || !str) return fl_bool(0);

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED | REG_NOSUB) != 0) return fl_bool(0);
    int matched = (regexec(&re, str, 0, NULL, 0) == 0);
    regfree(&re);
    return fl_bool(matched);
}

/* ── regex_find pattern str → 첫 매치 문자열 or nil ── */
FLValue regex_find(FLValue pat_v, FLValue str_v) {
    const char* pat = fl_cstr(pat_v);
    const char* str = fl_cstr(str_v);
    if (!pat || !str) return fl_nil();

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return fl_nil();
    regmatch_t m[1];
    FLValue result = fl_nil();
    if (regexec(&re, str, 1, m, 0) == 0 && m[0].rm_so >= 0) {
        size_t len = (size_t)(m[0].rm_eo - m[0].rm_so);
        char* buf = malloc(len + 1);
        memcpy(buf, str + m[0].rm_so, len);
        buf[len] = '\0';
        result = fl_str_val(buf);
        free(buf);
    }
    regfree(&re);
    return result;
}

/* ── regex_find_all pattern str → 벡터 ── */
FLValue regex_find_all(FLValue pat_v, FLValue str_v) {
    const char* pat = fl_cstr(pat_v);
    const char* str = fl_cstr(str_v);
    FLValue vec = fl_vec_new();
    if (!pat || !str) return vec;

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return vec;

    const char* cursor = str;
    regmatch_t m[1];
    while (regexec(&re, cursor, 1, m, 0) == 0 && m[0].rm_so >= 0) {
        size_t len = (size_t)(m[0].rm_eo - m[0].rm_so);
        char* buf = malloc(len + 1);
        memcpy(buf, cursor + m[0].rm_so, len);
        buf[len] = '\0';
        vec = fl_vec_push(vec, fl_str_val(buf));
        free(buf);
        if (len == 0) cursor++;  /* 무한루프 방지 */
        else cursor += m[0].rm_eo;
        if (*cursor == '\0') break;
    }
    regfree(&re);
    return vec;
}

/* ── regex_groups pattern str → ["전체","그룹1","그룹2",...] ── */
FLValue regex_groups(FLValue pat_v, FLValue str_v) {
    const char* pat = fl_cstr(pat_v);
    const char* str = fl_cstr(str_v);
    FLValue vec = fl_vec_new();
    if (!pat || !str) return vec;

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return vec;

    regmatch_t m[MAX_GROUPS];
    if (regexec(&re, str, MAX_GROUPS, m, 0) == 0) {
        for (int i = 0; i < MAX_GROUPS && m[i].rm_so >= 0; i++) {
            size_t len = (size_t)(m[i].rm_eo - m[i].rm_so);
            char* buf = malloc(len + 1);
            memcpy(buf, str + m[i].rm_so, len);
            buf[len] = '\0';
            vec = fl_vec_push(vec, fl_str_val(buf));
            free(buf);
        }
    }
    regfree(&re);
    return vec;
}

/* ── regex_replace pattern str repl → 첫 매치 교체 ── */
FLValue regex_replace(FLValue pat_v, FLValue str_v, FLValue repl_v) {
    const char* pat  = fl_cstr(pat_v);
    const char* str  = fl_cstr(str_v);
    const char* repl = fl_cstr(repl_v);
    if (!pat || !str || !repl) return str_v;

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return str_v;

    regmatch_t m[1];
    FLValue result = str_v;
    if (regexec(&re, str, 1, m, 0) == 0 && m[0].rm_so >= 0) {
        size_t pre  = (size_t)m[0].rm_so;
        size_t post = strlen(str) - (size_t)m[0].rm_eo;
        size_t rlen = strlen(repl);
        char* buf = malloc(pre + rlen + post + 1);
        memcpy(buf, str, pre);
        memcpy(buf + pre, repl, rlen);
        memcpy(buf + pre + rlen, str + m[0].rm_eo, post);
        buf[pre + rlen + post] = '\0';
        result = fl_str_val(buf);
        free(buf);
    }
    regfree(&re);
    return result;
}

/* ── regex_replace_all pattern str repl → 전체 교체 ── */
FLValue regex_replace_all(FLValue pat_v, FLValue str_v, FLValue repl_v) {
    const char* pat  = fl_cstr(pat_v);
    const char* str  = fl_cstr(str_v);
    const char* repl = fl_cstr(repl_v);
    if (!pat || !str || !repl) return str_v;

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return str_v;

    size_t cap = strlen(str) * 2 + strlen(repl) + 64;
    char*  out = malloc(cap);
    size_t outlen = 0;
    const char* cursor = str;
    regmatch_t m[1];

    while (*cursor && regexec(&re, cursor, 1, m, 0) == 0 && m[0].rm_so >= 0) {
        size_t pre  = (size_t)m[0].rm_so;
        size_t rlen = strlen(repl);
        /* 버퍼 확장 */
        while (outlen + pre + rlen + 1 >= cap) {
            cap *= 2;
            out = realloc(out, cap);
        }
        memcpy(out + outlen, cursor, pre);
        outlen += pre;
        memcpy(out + outlen, repl, rlen);
        outlen += rlen;
        size_t skip = (m[0].rm_eo > 0) ? (size_t)m[0].rm_eo : 1;
        cursor += skip;
    }
    /* 나머지 */
    size_t tail = strlen(cursor);
    while (outlen + tail + 1 >= cap) { cap *= 2; out = realloc(out, cap); }
    memcpy(out + outlen, cursor, tail);
    outlen += tail;
    out[outlen] = '\0';

    regfree(&re);
    FLValue result = fl_str_val(out);
    free(out);
    return result;
}

/* ── regex_split pattern str → ["a","b",...] ── */
FLValue regex_split(FLValue pat_v, FLValue str_v) {
    const char* pat = fl_cstr(pat_v);
    const char* str = fl_cstr(str_v);
    FLValue vec = fl_vec_new();
    if (!pat || !str) return vec;

    regex_t re;
    if (regcomp(&re, pat, REG_EXTENDED) != 0) return vec;

    const char* cursor = str;
    regmatch_t m[1];
    while (regexec(&re, cursor, 1, m, 0) == 0 && m[0].rm_so >= 0) {
        size_t len = (size_t)m[0].rm_so;
        char* tok = malloc(len + 1);
        memcpy(tok, cursor, len);
        tok[len] = '\0';
        vec = fl_vec_push(vec, fl_str_val(tok));
        free(tok);
        size_t skip = (m[0].rm_eo > 0) ? (size_t)m[0].rm_eo : 1;
        cursor += skip;
    }
    /* 마지막 토큰 */
    vec = fl_vec_push(vec, fl_str_val(cursor));
    regfree(&re);
    return vec;
}
