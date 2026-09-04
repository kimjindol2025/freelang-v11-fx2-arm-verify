/*
 * collection.c — FreeLang Vector / Map / Closure 구현
 *
 * 메모리 전략:
 *   - 요청 처리 중 (fl_arena_begin ~ fl_arena_end): Arena에 할당
 *   - 요청 외부 (전역 define, startup): process-lifetime registry
 *   - 클로저(fl_fn_new): 장기 생존 가능 → 항상 malloc
 *   - 빌더(fl_vec_builder_new): heap malloc, rc=0xFE 표시, freeze로 arena 변환
 *
 * copy semantics(불변 스타일)이므로 개별 free 불필요.
 * 요청 단위로 fl_arena_end()가 전부 일괄 해제.
 * 예외: rc==0xFE 빌더는 fl_vec_builder_freeze 시 heap free.
 */

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Arena 헬퍼 매크로 ──────────────────────────────────────────────
   fl_arena_alloc: arena 활성 시 bump 할당, 비활성 시 process-lifetime registry
*/
#define A(size) fl_arena_alloc(size)

/* ── Vector ── */

FLValue fl_vec_new(void) {
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = 0; v->cap = 0; v->data = NULL;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_vec_from(FLValue* items, uint32_t n) {
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = n; v->cap = n;
    v->data = n ? A(sizeof(FLValue) * n) : NULL;
    if (n) memcpy(v->data, items, sizeof(FLValue) * n);
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_vec_get(FLValue vec, FLValue idx) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    int64_t i = (idx.tag == FL_FLOAT) ? (int64_t)idx.f : idx.i;
    if (i < 0 || (uint32_t)i >= v->len) return fl_nil();
    return v->data[i];
}

FLValue fl_vec_len(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_int(0);
    return fl_int((int64_t)((FLVector*)vec.obj)->len);
}

/* ── 가변 빌더 (heap-allocated, rc=0xFE) ─────────────────────────────
 * fl_vec_builder_new  → heap FLVector, cap=16, rc=0xFE
 * fl_vec_builder_push → O(1) amortized (realloc+doubling)
 * fl_vec_builder_freeze → arena FLVector (불변) 반환 후 heap free
 *
 * 사용 패턴:
 *   FLValue b = fl_vec_builder_new();
 *   for (...) fl_vec_builder_push(b, elem);
 *   FLValue result = fl_vec_builder_freeze(b);
 */
FLValue fl_vec_builder_new(void) {
    FLVector* v = (FLVector*)malloc(sizeof(FLVector));
    v->base.type = FL_VECTOR;
    v->base.rc   = 0xFE;   /* mutable builder 표시 */
    v->len = 0; v->cap = 16;
    v->data = (FLValue*)malloc(sizeof(FLValue) * 16);
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

void fl_vec_builder_push(FLValue b, FLValue val) {
    if (b.tag != FL_VECTOR) return;
    FLVector* v = (FLVector*)b.obj;
    if (v->base.rc != 0xFE) return;
    if (v->len >= v->cap) {
        v->cap = v->cap ? v->cap * 2 : 16;
        v->data = (FLValue*)realloc(v->data, sizeof(FLValue) * v->cap);
    }
    v->data[v->len++] = val;
}

FLValue fl_vec_builder_freeze(FLValue b) {
    if (b.tag != FL_VECTOR) return fl_vec_new();
    FLVector* src = (FLVector*)b.obj;
    if (src->base.rc != 0xFE) return b;   /* 이미 불변 */
    FLValue result = fl_vec_from(src->data, src->len);
    free(src->data);
    free(src);
    return result;
}

/* copy semantics: 새 vector 반환. 구 vector는 Arena에 남아 일괄 해제됨.
 * 빌더(rc==0xFE) 입력이면 O(1) amortized 경로로 mutate. */
FLValue fl_vec_push(FLValue vec, FLValue val) {
    FLVector* src = (vec.tag == FL_VECTOR) ? (FLVector*)vec.obj : NULL;

    /* 빌더 fast-path: O(1) amortized */
    if (src && src->base.rc == 0xFE) {
        fl_vec_builder_push(vec, val);
        return vec;
    }

    /* 불변 copy-semantics */
    uint32_t n = src ? src->len : 0;
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = n + 1; v->cap = n + 1;
    v->data = A(sizeof(FLValue) * (n + 1));
    if (n && src->data) memcpy(v->data, src->data, sizeof(FLValue) * n);
    v->data[n] = val;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

FLValue fl_append(FLValue left, FLValue right) {
    if (left.tag == FL_VECTOR && right.tag == FL_VECTOR) {
        FLVector* a = (FLVector*)left.obj;
        FLVector* b = (FLVector*)right.obj;
        uint32_t n = a->len + b->len;
        /* This is a transient assembly buffer, unlike the returned vector.
         * It is deliberately plain heap memory because this function frees it
         * before returning. */
        FLValue* items = n ? malloc(sizeof(FLValue) * n) : NULL;
        if (a->len) memcpy(items, a->data, sizeof(FLValue) * a->len);
        if (b->len) memcpy(items + a->len, b->data, sizeof(FLValue) * b->len);
        FLValue out = fl_vec_from(items, n);
        if (items) free(items);
        return out;
    }
    return fl_vec_push(left, right);
}

FLValue fl_vec_set(FLValue vec, FLValue idx, FLValue val) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* src = (FLVector*)vec.obj;
    int64_t i = (idx.tag == FL_FLOAT) ? (int64_t)idx.f : idx.i;
    if (i < 0 || (uint32_t)i >= src->len) return vec;
    FLVector* v = A(sizeof(FLVector));
    v->base.type = FL_VECTOR; v->base.rc = 1;
    v->len = src->len; v->cap = src->len;
    v->data = A(sizeof(FLValue) * src->len);
    memcpy(v->data, src->data, sizeof(FLValue) * src->len);
    v->data[i] = val;
    FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)v; return r;
}

/* ── Map ── */

FLValue fl_map_new(void) {
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = 0; m->cap = 0; m->entries = NULL;
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

/* kv: [k0,v0, k1,v1, ...], n = 쌍의 수 */
FLValue fl_map_from_pairs(FLValue* kv, uint32_t n) {
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = n; m->cap = n;
    m->entries = n ? A(sizeof(FLMapEntry) * n) : NULL;
    for (uint32_t i = 0; i < n; i++) {
        m->entries[i].key = kv[i * 2];
        m->entries[i].val = kv[i * 2 + 1];
    }
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

FLValue fl_map_get(FLValue map, FLValue key) {
    if (map.tag != FL_MAP) return fl_nil();
    FLMap* m = (FLMap*)map.obj;
    for (uint32_t i = 0; i < m->len; i++) {
        if (fl_truthy(fl_eq(m->entries[i].key, key)))
            return m->entries[i].val;
    }
    return fl_nil();
}

FLValue fl_map_len(FLValue map) {
    if (map.tag != FL_MAP) return fl_int(0);
    return fl_int((int64_t)((FLMap*)map.obj)->len);
}

/* copy semantics: upsert 후 새 map 반환. 구 map은 Arena에서 일괄 해제됨 */
FLValue fl_map_set(FLValue map, FLValue key, FLValue val) {
    FLMap* src = (map.tag == FL_MAP) ? (FLMap*)map.obj : NULL;
    uint32_t n = src ? src->len : 0;
    /* 기존 키 탐색 */
    for (uint32_t i = 0; i < n; i++) {
        if (fl_truthy(fl_eq(src->entries[i].key, key))) {
            FLMap* m = A(sizeof(FLMap));
            m->base.type = FL_MAP; m->base.rc = 1;
            m->len = n; m->cap = n;
            m->entries = A(sizeof(FLMapEntry) * n);
            memcpy(m->entries, src->entries, sizeof(FLMapEntry) * n);
            m->entries[i].val = val;
            FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
        }
    }
    /* 새 키 추가 */
    FLMap* m = A(sizeof(FLMap));
    m->base.type = FL_MAP; m->base.rc = 1;
    m->len = n + 1; m->cap = n + 1;
    m->entries = A(sizeof(FLMapEntry) * (n + 1));
    if (n && src->entries) memcpy(m->entries, src->entries, sizeof(FLMapEntry) * n);
    m->entries[n].key = key; m->entries[n].val = val;
    FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)m; return r;
}

/* ── S7: Closure ──
   클로저는 전역 핸들러로 등록될 수 있어 요청 수명보다 길게 생존.
   반드시 malloc 사용 (Arena 비사용).
*/
FLValue fl_fn_new(FLValue (*call)(FLClosure*, int, FLValue*),
                  uint32_t nenv, FLValue* env) {
    FLClosure* cl = malloc(sizeof(FLClosure) + sizeof(FLValue) * nenv);
    cl->base.type = FL_FN; cl->base.rc = 1;
    cl->call = call; cl->nenv = nenv; cl->hot_count = 0;
    for (uint32_t i = 0; i < nenv; i++) cl->env[i] = env[i];
    FLValue r; r.tag = FL_FN; r.obj = (FLObject*)cl; return r;
}

FLValue fl_make_native_fn(FLValue (*call)(FLClosure*, int, FLValue*), const char* name) {
    (void)name;   /* SIS 메타데이터 확장 예약 — 현재는 무시 */
    return fl_fn_new(call, 0, NULL);
}

FLValue fl_fn_call(FLValue fn, int argc, FLValue* argv) {
    if (fn.tag != FL_FN) { fputs("error: not a fn\n", stderr); exit(1); }
    FLClosure* cl = (FLClosure*)fn.obj;
    return cl->call(cl, argc, argv);
}

/* ── S8: 고차함수 ── */

FLValue fl_map_fn(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    /* O(n): 결과 크기 확정(입력 동일) → 단일 arena 배열 */
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        tmp[i] = fl_fn_call(fn, 1, &elem);
    }
    FLValue r = fl_vec_from(tmp, v->len);
    free(tmp);
    return r;
}

FLValue fl_filter_fn(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    /* O(n): 최대 n개 → C 배열 수집 후 fl_vec_from */
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    uint32_t len = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        if (fl_truthy(fl_fn_call(fn, 1, &elem))) tmp[len++] = elem;
    }
    FLValue r = fl_vec_from(tmp, len);
    free(tmp);
    return r;
}

/* for-each: 부작용 전용 반복, nil 반환 */
void fl_for_each(FLValue fn, FLValue vec) {
    if (vec.tag != FL_VECTOR) return;
    FLVector* v = (FLVector*)vec.obj;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue elem = v->data[i];
        fl_fn_call(fn, 1, &elem);
    }
}

/* distinct: 중복 제거 (순서 유지, O(n²)) */
FLValue distinct(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    if (v->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * v->len);
    if (!tmp) return fl_vec_new();
    uint32_t len = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        int found = 0;
        for (uint32_t j = 0; j < len; j++) {
            if (fl_truthy(fl_eq(v->data[i], tmp[j]))) { found = 1; break; }
        }
        if (!found) tmp[len++] = v->data[i];
    }
    FLValue r = fl_vec_from(tmp, len);
    free(tmp);
    return r;
}

FLValue fl_reduce_fn(FLValue fn, FLValue init, FLValue vec) {
    if (vec.tag != FL_VECTOR) return init;
    FLVector* v = (FLVector*)vec.obj;
    FLValue acc = init;
    for (uint32_t i = 0; i < v->len; i++) {
        FLValue args[2] = { acc, v->data[i] };
        acc = fl_fn_call(fn, 2, args);
    }
    return acc;
}

/* ── S9: 맵 accessor ── */

FLValue fl_map_keys(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) tmp[i] = m->entries[i].key;
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

FLValue fl_map_vals(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) tmp[i] = m->entries[i].val;
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

FLValue fl_map_entries(FLValue map) {
    if (map.tag != FL_MAP) return fl_vec_new();
    FLMap* m = (FLMap*)map.obj;
    if (m->len == 0) return fl_vec_new();
    FLValue* tmp = (FLValue*)malloc(sizeof(FLValue) * m->len);
    if (!tmp) return fl_vec_new();
    for (uint32_t i = 0; i < m->len; i++) {
        FLValue pair[2] = { m->entries[i].key, m->entries[i].val };
        tmp[i] = fl_vec_from(pair, 2);
    }
    FLValue r = fl_vec_from(tmp, m->len); free(tmp); return r;
}

/* ── S12: bridge builtins ── */

FLValue null_p(FLValue v) {
    return fl_bool(v.tag == FL_NIL);
}

FLValue get(FLValue obj, FLValue key) {
    if (obj.tag == FL_NIL) {
        const char* ks = (key.tag == FL_STRING) ? ((FLString*)key.obj)->data : "?";
        char msg[256];
        snprintf(msg, sizeof(msg), "(get nil \"%s\") — nil에 key 접근 불가. nil 체크 먼저.", ks);
        fl_throw(fl_str_val(msg));
        return fl_nil();
    }
    if (obj.tag == FL_MAP) return fl_map_get(obj, key);
    if (obj.tag == FL_VECTOR) {
        if (key.tag != FL_INT) return fl_nil();
        return fl_vec_get(obj, key);
    }
    if (obj.tag == FL_STRING) {
        if (key.tag != FL_INT) return fl_nil();
        int64_t idx = key.i;
        FLString* s = (FLString*)obj.obj;
        if (idx < 0 || idx >= (int64_t)s->len) return fl_nil();
        return fl_str_val_n(s->data + idx, 1);
    }
    return fl_nil();
}

FLValue length(FLValue obj) {
    if (obj.tag == FL_VECTOR) return fl_vec_len(obj);
    if (obj.tag == FL_MAP)    return fl_map_len(obj);
    if (obj.tag == FL_STRING) return fl_int((int64_t)((FLString*)obj.obj)->len);
    return fl_int(0);
}

FLValue char_at(FLValue str, FLValue idx) {
    return get(str, idx);
}

/* ── S15: stdlib bridge ── */

FLValue fl_vec_slice(FLValue vec, FLValue start, FLValue end) {
    if (vec.tag != FL_VECTOR) return fl_vec_new();
    FLVector* v = (FLVector*)vec.obj;
    int64_t s = start.tag == FL_INT ? start.i : 0;
    int64_t e = end.tag == FL_INT ? (end.i < 0 ? (int64_t)v->len + end.i + 1 : end.i) : (int64_t)v->len;
    if (s < 0) s = 0;
    if (e > (int64_t)v->len) e = (int64_t)v->len;
    if (s >= e) return fl_vec_new();
    /* O(n): fl_vec_from으로 단일 alloc */
    return fl_vec_from(v->data + s, (uint32_t)(e - s));
}

FLValue fl_vec_last(FLValue vec) {
    if (vec.tag != FL_VECTOR) return fl_nil();
    FLVector* v = (FLVector*)vec.obj;
    return v->len > 0 ? v->data[v->len-1] : fl_nil();
}

FLValue fl_map_del(FLValue map, FLValue key) {
    if (map.tag != FL_MAP) return map;
    FLMap* m = (FLMap*)map.obj;
    FLValue r = fl_map_new();
    for (uint32_t i = 0; i < m->len; i++)
        if (!fl_truthy(fl_eq(m->entries[i].key, key)))
            r = fl_map_set(r, m->entries[i].key, m->entries[i].val);
    return r;
}

FLValue fl_map_merge(FLValue a, FLValue b) {
    if (a.tag != FL_MAP) return b;
    if (b.tag != FL_MAP) return a;
    FLValue r = a;
    FLMap* mb = (FLMap*)b.obj;
    for (uint32_t i = 0; i < mb->len; i++)
        r = fl_map_set(r, mb->entries[i].key, mb->entries[i].val);
    return r;
}
