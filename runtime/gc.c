/*
 * gc.c — FreeLang Hybrid GC
 *
 * 두 가지 GC 전략을 혼합:
 *
 * 1. Arena GC (요청 스코프 — 기존 유지)
 *    - HTTP 요청마다 arena 생성 → 완료 시 bulk free
 *    - STW 없음, 극도로 빠름
 *    - 단점: atom/global이 보유한 값은 요청 완료 후 dangling!
 *
 * 2. RC-Heap GC (atom 보유값 — 신규)
 *    - atom.data[0]이 가리키는 값은 반드시 heap에 있어야 함
 *    - fl_heap_copy(v): FLValue를 재귀적으로 malloc으로 복사
 *    - fl_heap_release(v): RC--, 0이면 재귀 free
 *    - swap!: 새값 heap_copy → 이전값 heap_release
 *
 * 왜 이 설계인가:
 *   - atom이 보유하는 값(예: fl-kv의 store 맵)은 요청 사이에 살아있어야 함
 *   - 기존 코드: atom.data[0] = arena 객체 → arena 리셋 후 dangling (UB!)
 *   - RC-Heap: atom 전용 heap 영역 → 명시적 생명주기 관리
 *   - 전체 GC (mark-sweep, tri-color)는 오버킬 — RC로 충분
 */

#define _GNU_SOURCE
#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/* Live RC-object registry: validate opaque FLValue handles before touching
 * object memory, so stale and double releases are harmless. */
typedef struct FLHeapLive { FLObject *obj; struct FLHeapLive *next; } FLHeapLive;
static FLHeapLive *g_heap_live = NULL;
static pthread_mutex_t g_heap_live_lock = PTHREAD_MUTEX_INITIALIZER;
static void heap_live_add(FLObject *obj) {
    FLHeapLive *n = (FLHeapLive *)malloc(sizeof(*n)); if (!n) return; n->obj = obj;
    pthread_mutex_lock(&g_heap_live_lock); n->next = g_heap_live; g_heap_live = n; pthread_mutex_unlock(&g_heap_live_lock);
}
static bool heap_live_has(FLObject *obj) {
    bool found = false; pthread_mutex_lock(&g_heap_live_lock);
    for (FLHeapLive *n = g_heap_live; n; n = n->next) if (n->obj == obj) { found = true; break; }
    pthread_mutex_unlock(&g_heap_live_lock); return found;
}
static bool heap_live_remove(FLObject *obj) {
    bool found = false; pthread_mutex_lock(&g_heap_live_lock); FLHeapLive **p = &g_heap_live;
    while (*p) { if ((*p)->obj == obj) { FLHeapLive *d = *p; *p = d->next; free(d); found = true; break; } p = &(*p)->next; }
    pthread_mutex_unlock(&g_heap_live_lock); return found;
}

/* ── Arena 구조 ────────────────────────────────────────
   단일 연결 리스트로 청크를 이어붙임
   청크 크기: 64KB (L2 캐시에 맞게)
*/
#define ARENA_CHUNK_SIZE (64 * 1024)

typedef struct ArenaChunk {
    struct ArenaChunk* next;
    size_t used;
    size_t cap;
    char   data[];    /* 유연 배열 */
} ArenaChunk;

typedef struct {
    ArenaChunk* head;       /* 현재 청크 */
    ArenaChunk* first;      /* 첫 청크 (재사용용) */
    size_t      total_alloc; /* 디버그: 총 할당량 */
    size_t      total_free;  /* 디버그: 총 해제량 */
} Arena;

/* ── per-thread 아레나 ── */
static __thread Arena* g_arena = NULL;

/* Runtime-owned memory must be released before LeakSanitizer runs.  The
 * registration is deliberately lazy: native embedders that never allocate a
 * managed value do not acquire any runtime teardown work. */
static pthread_once_t g_runtime_cleanup_once = PTHREAD_ONCE_INIT;

static void fl_register_runtime_cleanup(void) {
    (void)atexit(fl_runtime_shutdown);
}

static ArenaChunk* arena_new_chunk(size_t min_size) {
    size_t cap = ARENA_CHUNK_SIZE;
    if (min_size + sizeof(ArenaChunk) > cap)
        cap = min_size + sizeof(ArenaChunk);
    ArenaChunk* c = malloc(sizeof(ArenaChunk) + cap);
    if (!c) return NULL;
    c->next = NULL;
    c->used = 0;
    c->cap  = cap;
    return c;
}

void fl_arena_begin(void) {
    pthread_once(&g_runtime_cleanup_once, fl_register_runtime_cleanup);
    if (!g_arena) {
        g_arena = malloc(sizeof(Arena));
        g_arena->head  = arena_new_chunk(0);
        g_arena->first = g_arena->head;
        g_arena->total_alloc = 0;
        g_arena->total_free  = 0;
    } else {
        /* 아레나 재사용: 첫 청크로 리셋 */
        g_arena->head = g_arena->first;
        ArenaChunk* c = g_arena->first;
        while (c) { c->used = 0; c = c->next; }
    }
}

void* fl_arena_alloc(size_t size) {
    /* Values created outside a request are process-lifetime values, not
     * unowned malloc allocations.  fl_runtime_shutdown releases them. */
    if (!g_arena || !g_arena->head) return fl_perm_alloc(size);

    /* 8바이트 정렬 */
    size = (size + 7) & ~(size_t)7;

    ArenaChunk* c = g_arena->head;
    if (c->used + size <= c->cap) {
        void* ptr = c->data + c->used;
        c->used += size;
        g_arena->total_alloc += size;
        return ptr;
    }

    /* 새 청크 필요 */
    ArenaChunk* nc = c->next ? c->next : arena_new_chunk(size);
    if (!nc) return fl_perm_alloc(size);  /* OOM 폴백 */

    if (!c->next) c->next = nc;
    nc->used = 0;
    g_arena->head = nc;

    void* ptr = nc->data;
    nc->used = size;
    g_arena->total_alloc += size;
    return ptr;
}

void fl_arena_end(void) {
    /* 아레나 리셋만 (메모리 유지 — 다음 요청에 재사용) */
    if (!g_arena) return;
    ArenaChunk* c = g_arena->first;
    while (c) { c->used = 0; c = c->next; }
    g_arena->head = g_arena->first;
}

static void fl_arena_cleanup(void) {
    ArenaChunk* c;
    if (!g_arena) return;
    c = g_arena->first;
    while (c) {
        ArenaChunk* next = c->next;
        free(c);
        c = next;
    }
    free(g_arena);
    g_arena = NULL;
}

/* 누적 통계 (debug level 2+) */
void fl_arena_stats(void) {
    if (!g_arena) return;
    size_t n_chunks = 0;
    ArenaChunk* c = g_arena->first;
    while (c) { n_chunks++; c = c->next; }
    fl_log(2, "gc", "아레나 청크=%zu, 총할당=%zuKB",
           n_chunks, g_arena->total_alloc / 1024);
}

/* ── 영속 풀 (DB 연결, 설정 등) ──────────────────────
   요청 생명주기 밖의 객체는 여기서 관리
   동적 확장 (기존 256 고정 제한 제거)
*/
typedef struct PermNode {
    struct PermNode* next;
    void*            ptr;
} PermNode;

static PermNode*       g_perm_head  = NULL;
static size_t          g_perm_count = 0;
static pthread_mutex_t g_perm_lock  = PTHREAD_MUTEX_INITIALIZER;

void* fl_perm_alloc(size_t size) {
    pthread_once(&g_runtime_cleanup_once, fl_register_runtime_cleanup);
    void* ptr = malloc(size);
    if (!ptr) return NULL;
    PermNode* node = (PermNode*)malloc(sizeof(PermNode));
    if (!node) { free(ptr); return NULL; }
    node->ptr = ptr;
    pthread_mutex_lock(&g_perm_lock);
    node->next = g_perm_head;
    g_perm_head = node;
    g_perm_count++;
    pthread_mutex_unlock(&g_perm_lock);
    return ptr;
}

/* 프로세스 종료 시 전체 해제 (ASAN 호환) */
void fl_perm_cleanup(void) {
    pthread_mutex_lock(&g_perm_lock);
    PermNode* n = g_perm_head;
    while (n) {
        PermNode* nx = n->next;
        free(n->ptr);
        free(n);
        n = nx;
    }
    g_perm_head  = NULL;
    g_perm_count = 0;
    pthread_mutex_unlock(&g_perm_lock);
}

/* Call only after application work and worker shutdown are complete.  It is
 * idempotent and is also registered automatically for normal process exit. */
void fl_runtime_shutdown(void) {
    fl_perm_cleanup();
    fl_arena_cleanup();
}

/* ── RC-Heap GC — atom 보유값 전용 ──────────────────
 *
 * atom.data[0]이 가리키는 FLValue는 반드시 RC-Heap에 있어야 함.
 * Arena 리셋 후에도 살아있어야 하기 때문.
 *
 * FLObject.rc 필드를 활용:
 *   rc == 0    → heap에 없음 (arena 객체 또는 임시값)
 *   rc == 1..N → heap 객체, N개 참조
 *   rc == 0xFF → 고정 (절대 해제 안 함, DB 연결 등)
 */

/* FLValue를 malloc 힙으로 deep-copy (RC = 1로 설정) */
FLValue fl_heap_copy(FLValue v) {
    switch (v.tag) {
    case FL_INT: case FL_FLOAT: case FL_BOOL: case FL_NIL:
        return v;  /* 스칼라: 값 복사로 충분 */

    case FL_STRING: {
        FLString* src = (FLString*)v.obj;
        FLString* dst = (FLString*)malloc(sizeof(FLString) + src->len + 1);
        if (!dst) return fl_nil();
        dst->base.type = FL_STRING;
        dst->base.rc   = 1;
        heap_live_add(&dst->base);
        dst->len = src->len;
        memcpy(dst->data, src->data, src->len + 1);
        FLValue r; r.tag = FL_STRING; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_VECTOR: {
        FLVector* src = (FLVector*)v.obj;
        FLVector* dst = (FLVector*)malloc(sizeof(FLVector));
        if (!dst) return fl_nil();
        dst->base.type = FL_VECTOR;
        dst->base.rc   = 1;
        heap_live_add(&dst->base);
        dst->len = src->len;
        dst->cap = src->len;  /* 딱 맞게 */
        dst->data = src->len > 0
            ? (FLValue*)malloc(src->len * sizeof(FLValue))
            : NULL;
        for (uint32_t i = 0; i < src->len; i++)
            dst->data[i] = fl_heap_copy(src->data[i]);
        FLValue r; r.tag = FL_VECTOR; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_MAP: {
        FLMap* src = (FLMap*)v.obj;
        FLMap* dst = (FLMap*)malloc(sizeof(FLMap));
        if (!dst) return fl_nil();
        dst->base.type = FL_MAP;
        dst->base.rc   = 1;
        heap_live_add(&dst->base);
        dst->len = src->len;
        dst->cap = src->len;
        dst->entries = src->len > 0
            ? (FLMapEntry*)malloc(src->len * sizeof(FLMapEntry))
            : NULL;
        for (uint32_t i = 0; i < src->len; i++) {
            dst->entries[i].key = fl_heap_copy(src->entries[i].key);
            dst->entries[i].val = fl_heap_copy(src->entries[i].val);
        }
        FLValue r; r.tag = FL_MAP; r.obj = (FLObject*)dst;
        return r;
    }

    case FL_FN:
        /* 함수는 shared — RC++ */
        if (v.obj && v.obj->rc < 0xFE) v.obj->rc++;
        return v;
    }
    return fl_nil();
}

/* RC++ (heap 객체에 대한 추가 참조) */
void fl_heap_retain(FLValue v) {
    if (v.tag < FL_STRING || !v.obj) return;
    if (!heap_live_has(v.obj)) return;
    if (v.obj->rc == 0 || v.obj->rc == 0xFF) return;
    v.obj->rc++;
}

/* RC-- → 0이면 재귀 해제 */
void fl_heap_release(FLValue v) {
    if (v.tag < FL_STRING || !v.obj) return;
    if (!heap_live_has(v.obj)) return;
    if (v.obj->rc == 0 || v.obj->rc == 0xFF) return;
    if (--v.obj->rc > 0) return;

    /* RC = 0 → 해제 */
    if (!heap_live_remove(v.obj)) return;
    switch (v.tag) {
    case FL_STRING:
        free(v.obj);
        break;
    case FL_VECTOR: {
        FLVector* vp = (FLVector*)v.obj;
        for (uint32_t i = 0; i < vp->len; i++)
            fl_heap_release(vp->data[i]);
        free(vp->data);
        free(vp);
        break;
    }
    case FL_MAP: {
        FLMap* mp = (FLMap*)v.obj;
        for (uint32_t i = 0; i < mp->len; i++) {
            fl_heap_release(mp->entries[i].key);
            fl_heap_release(mp->entries[i].val);
        }
        free(mp->entries);
        free(mp);
        break;
    }
    case FL_FN:
        free(v.obj);
        break;
    default: break;
    }
}

/* ── 메모리 사용량 통계 ── */
FLValue fl_memory_stats(void) {
    FLValue m = fl_map_new();

    /* /proc/self/status에서 VmRSS 읽기 */
    FILE* f = fopen("/proc/self/status", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            long kb = 0;
            if (sscanf(line, "VmRSS: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("rss_kb"), fl_int(kb));
            if (sscanf(line, "VmPeak: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("peak_kb"), fl_int(kb));
            if (sscanf(line, "VmSize: %ld kB", &kb) == 1)
                m = fl_map_set(m, fl_str_val("virt_kb"), fl_int(kb));
        }
        fclose(f);
    }

    if (g_arena) {
        size_t n = 0; ArenaChunk* c = g_arena->first;
        while (c) { n += c->used; c = c->next; }
        m = fl_map_set(m, fl_str_val("arena_kb"), fl_int((int64_t)(n / 1024)));
    }
    return m;
}
