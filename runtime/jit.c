/*
 * jit.c — FreeLang JIT 컴파일러 기초 구조
 *
 * 설계:
 *   - FLClosure.call 포인터를 JIT 생성 native code로 교체
 *   - System V AMD64 ABI: FLValue(16B) = RAX(하위8B) + RDX(상위8B)
 *   - FLValue.tag @ offset 0 (4B), FLValue.union @ offset 8 (8B)
 *
 * JIT 적용 조건 (핫 함수):
 *   - 호출 횟수 >= FL_JIT_THRESHOLD (기본: 100)
 *   - 정수 연산만 대상 (FL_FLOAT, FL_STRING은 인터프리터 폴백)
 *
 * 현재 지원 패턴:
 *   P1: 상수 반환 (fn [] 42)
 *   P2: 단항 연산 (fn [x] (+ x N), (* x N))
 *   P3: 이항 연산 (fn [x y] (+ x y))
 *
 * ABI 요약:
 *   RDI = FLClosure* self
 *   RSI = int argc
 *   RDX = FLValue* argv
 *   반환: RAX = FLValue[0..7], RDX = FLValue[8..15]
 *         → RAX = tag(4)+pad(4), RDX = int64 값
 */

#define _GNU_SOURCE
#include "runtime.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

/* ── JIT 설정 ── */
#define FL_JIT_THRESHOLD     100   /* 호출 N회 후 JIT 트리거 */
#define FL_JIT_PAGE_SIZE     4096  /* 1 페이지 = 4KB */
#define FL_JIT_POOL_PAGES    16    /* JIT 코드 풀 = 64KB */

/* ── JIT 코드 버퍼 ── */
typedef struct JITBuf {
    uint8_t* mem;    /* mmap 실행 가능 메모리 */
    size_t   used;   /* 사용된 바이트 */
    size_t   cap;    /* 총 용량 */
} JITBuf;

static JITBuf        g_jit_pool = {0};
static pthread_mutex_t g_jit_lock = PTHREAD_MUTEX_INITIALIZER;

/* JIT 코드 풀 초기화 */
static int jit_pool_init(void) {
    if (g_jit_pool.mem) return 1;
    size_t cap = FL_JIT_PAGE_SIZE * FL_JIT_POOL_PAGES;
    void* mem = mmap(NULL, cap,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) return 0;
    g_jit_pool.mem  = (uint8_t*)mem;
    g_jit_pool.used = 0;
    g_jit_pool.cap  = cap;
    return 1;
}

/* JIT 코드 풀에서 N바이트 할당 */
static uint8_t* jit_alloc(size_t size) {
    pthread_mutex_lock(&g_jit_lock);
    if (!jit_pool_init()) { pthread_mutex_unlock(&g_jit_lock); return NULL; }
    /* 8바이트 정렬 */
    size = (size + 7) & ~(size_t)7;
    if (g_jit_pool.used + size > g_jit_pool.cap) {
        pthread_mutex_unlock(&g_jit_lock);
        return NULL;  /* 풀 꽉 참 */
    }
    uint8_t* ptr = g_jit_pool.mem + g_jit_pool.used;
    g_jit_pool.used += size;
    pthread_mutex_unlock(&g_jit_lock);
    return ptr;
}

/* ── x86_64 코드 생성 헬퍼 ── */
typedef struct Emitter {
    uint8_t* buf;
    size_t   pos;
    size_t   cap;
} Emitter;

static inline void emit_u8(Emitter* e, uint8_t b) {
    if (e->pos < e->cap) e->buf[e->pos++] = b;
}

static inline void emit_u32(Emitter* e, uint32_t v) {
    emit_u8(e, v & 0xFF); emit_u8(e, (v>>8)&0xFF);
    emit_u8(e, (v>>16)&0xFF); emit_u8(e, (v>>24)&0xFF);
}

static inline void emit_u64(Emitter* e, uint64_t v) {
    for (int i = 0; i < 8; i++) { emit_u8(e, v & 0xFF); v >>= 8; }
}

/* MOV RAX, imm64 → 48 B8 <8바이트> */
static inline void emit_mov_rax_imm64(Emitter* e, uint64_t v) {
    emit_u8(e, 0x48); emit_u8(e, 0xB8); emit_u64(e, v);
}

/* MOV RDX, imm64 → 48 BA <8바이트> */
static inline void emit_mov_rdx_imm64(Emitter* e, uint64_t v) {
    emit_u8(e, 0x48); emit_u8(e, 0xBA); emit_u64(e, v);
}

/* XOR EAX, EAX → 31 C0 (RAX = 0, tag = FL_INT) */
static inline void emit_xor_eax_eax(Emitter* e) {
    emit_u8(e, 0x31); emit_u8(e, 0xC0);
}

/* MOV EAX, [RDX] → 8B 02 (argv[0].tag) */
static inline void emit_mov_eax_deref_rdx(Emitter* e) {
    emit_u8(e, 0x8B); emit_u8(e, 0x02);
}

/* MOV RDX, [RDX+8] → 48 8B 52 08 (argv[0].i) */
static inline void emit_mov_rdx_rdx8(Emitter* e) {
    emit_u8(e, 0x48); emit_u8(e, 0x8B); emit_u8(e, 0x52); emit_u8(e, 0x08);
}

/* CMP EAX, imm8 → 83 F8 <imm> */
static inline void emit_cmp_eax_imm8(Emitter* e, uint8_t imm) {
    emit_u8(e, 0x83); emit_u8(e, 0xF8); emit_u8(e, imm);
}

/* JNE rel8 → 75 <rel> */
static inline size_t emit_jne_placeholder(Emitter* e) {
    emit_u8(e, 0x75); emit_u8(e, 0x00); /* 나중에 패치 */
    return e->pos - 1;  /* 패치할 위치 */
}

/* rel8 패치 */
static inline void patch_rel8(Emitter* e, size_t patch_pos) {
    int8_t rel = (int8_t)(e->pos - (patch_pos + 1));
    e->buf[patch_pos] = (uint8_t)rel;
}

/* ADD RDX, imm8 → 48 83 C2 <imm> */
static inline void emit_add_rdx_imm8(Emitter* e, uint8_t imm) {
    emit_u8(e, 0x48); emit_u8(e, 0x83); emit_u8(e, 0xC2); emit_u8(e, imm);
}

/* IMUL RDX, imm32 → 48 69 D2 <4바이트> */
static inline void emit_imul_rdx_imm32(Emitter* e, int32_t imm) {
    emit_u8(e, 0x48); emit_u8(e, 0x69); emit_u8(e, 0xD2);
    emit_u32(e, (uint32_t)imm);
}

/* RET → C3 */
static inline void emit_ret(Emitter* e) { emit_u8(e, 0xC3); }

/* JMP RAX → FF E0 (폴백 함수로 점프) */
static inline void emit_jmp_rax(Emitter* e) {
    emit_u8(e, 0xFF); emit_u8(e, 0xE0);
}

/* SUB RDX, imm8 → 48 83 EA <imm> */
static inline void emit_sub_rdx_imm8(Emitter* e, uint8_t imm) {
    emit_u8(e, 0x48); emit_u8(e, 0x83); emit_u8(e, 0xEA); emit_u8(e, imm);
}

/* NEG RDX → 48 F7 DA (RDX = -RDX) */
static inline void emit_neg_rdx(Emitter* e) {
    emit_u8(e, 0x48); emit_u8(e, 0xF7); emit_u8(e, 0xDA);
}

/* ── JIT 함수 타입 ── */
typedef FLValue (*JITFn)(FLClosure* self, int argc, FLValue* argv);

/* ── JIT 패턴 빌더 ── */

/*
 * P1: 상수 정수 반환 — (fn [] N)
 * 생성 코드:
 *   XOR EAX, EAX         ; RAX = FL_INT(0) + pad(0)
 *   MOV RDX, <N>         ; RDX = 정수값
 *   RET
 */
JITFn jit_build_const_int(int64_t n) {
    uint8_t tmp[32];
    Emitter e = { tmp, 0, sizeof(tmp) };
    emit_xor_eax_eax(&e);
    emit_mov_rdx_imm64(&e, (uint64_t)n);
    emit_ret(&e);

    uint8_t* code = jit_alloc(e.pos);
    if (!code) return NULL;
    memcpy(code, tmp, e.pos);
    return (JITFn)(void*)code;
}

/*
 * P2: (fn [x] (+ x N)) — 정수 타입 체크 + 가산
 *
 * 생성 코드:
 *   MOV EAX, [RDX]       ; argv[0].tag
 *   CMP EAX, 0           ; FL_INT?
 *   JNE fallback
 *   MOV RDX, [RDX+8]     ; argv[0].i
 *   ADD RDX, N           ; + N
 *   XOR EAX, EAX         ; tag = FL_INT
 *   RET
 * fallback:
 *   MOV RAX, <c_fn>
 *   JMP RAX
 */
JITFn jit_build_add_int_const(int64_t n, JITFn fallback) {
    uint8_t tmp[64];
    Emitter e = { tmp, 0, sizeof(tmp) };

    /* 타입 체크 */
    emit_mov_eax_deref_rdx(&e);           /* MOV EAX, [RDX] */
    emit_cmp_eax_imm8(&e, FL_INT);        /* CMP EAX, 0 */
    size_t jne_pos = emit_jne_placeholder(&e);  /* JNE fallback */

    /* fast path */
    emit_mov_rdx_rdx8(&e);               /* MOV RDX, [RDX+8] */
    if (n >= -128 && n <= 127) {
        emit_add_rdx_imm8(&e, (uint8_t)(int8_t)n);
    } else {
        /* ADD RDX, imm64은 복잡 → MOV R11, imm64; ADD RDX, R11 */
        emit_u8(&e, 0x49); emit_u8(&e, 0xBB);  /* MOV R11, imm64 */
        emit_u64(&e, (uint64_t)n);
        emit_u8(&e, 0x4C); emit_u8(&e, 0x01); emit_u8(&e, 0xDA);  /* ADD RDX, R11 */
    }
    emit_xor_eax_eax(&e);               /* XOR EAX, EAX */
    emit_ret(&e);

    /* fallback */
    patch_rel8(&e, jne_pos);
    emit_mov_rax_imm64(&e, (uint64_t)(uintptr_t)fallback);
    emit_jmp_rax(&e);

    uint8_t* code = jit_alloc(e.pos);
    if (!code) return NULL;
    memcpy(code, tmp, e.pos);
    return (JITFn)(void*)code;
}

/*
 * P2: (fn [x] (* x N)) — 정수 곱셈
 */
JITFn jit_build_mul_int_const(int32_t n, JITFn fallback) {
    uint8_t tmp[64];
    Emitter e = { tmp, 0, sizeof(tmp) };

    emit_mov_eax_deref_rdx(&e);
    emit_cmp_eax_imm8(&e, FL_INT);
    size_t jne_pos = emit_jne_placeholder(&e);

    emit_mov_rdx_rdx8(&e);
    emit_imul_rdx_imm32(&e, n);          /* IMUL RDX, RDX, N */
    emit_xor_eax_eax(&e);
    emit_ret(&e);

    patch_rel8(&e, jne_pos);
    emit_mov_rax_imm64(&e, (uint64_t)(uintptr_t)fallback);
    emit_jmp_rax(&e);

    uint8_t* code = jit_alloc(e.pos);
    if (!code) return NULL;
    memcpy(code, tmp, e.pos);
    return (JITFn)(void*)code;
}

/*
 * P3: (fn [x y] (+ x y)) — 두 정수 합산
 *
 * argv[0].i + argv[1].i
 * argv[0] = RDX[0..15], argv[1] = RDX[16..31]
 *
 * 생성 코드:
 *   MOV EAX, [RDX]       ; argv[0].tag
 *   CMP EAX, FL_INT
 *   JNE fallback
 *   MOV EAX, [RDX+16]    ; argv[1].tag
 *   CMP EAX, FL_INT
 *   JNE fallback
 *   MOV RAX, [RDX+8]     ; argv[0].i (R임시로 RAX 사용)
 *   ADD RAX, [RDX+24]    ; + argv[1].i
 *   MOV RDX, RAX
 *   XOR EAX, EAX
 *   RET
 */
JITFn jit_build_add_two_ints(JITFn fallback) {
    uint8_t tmp[80];
    Emitter e = { tmp, 0, sizeof(tmp) };

    /* arg0 타입 체크 */
    emit_mov_eax_deref_rdx(&e);                   /* MOV EAX, [RDX] */
    emit_cmp_eax_imm8(&e, FL_INT);
    size_t jne0 = emit_jne_placeholder(&e);

    /* arg1 타입 체크: [RDX+16].tag */
    emit_u8(&e, 0x8B); emit_u8(&e, 0x42); emit_u8(&e, 0x10); /* MOV EAX, [RDX+16] */
    emit_cmp_eax_imm8(&e, FL_INT);
    size_t jne1 = emit_jne_placeholder(&e);

    /* fast path: RAX = argv[0].i + argv[1].i */
    emit_u8(&e, 0x48); emit_u8(&e, 0x8B); emit_u8(&e, 0x42); emit_u8(&e, 0x08); /* MOV RAX, [RDX+8] */
    emit_u8(&e, 0x48); emit_u8(&e, 0x03); emit_u8(&e, 0x42); emit_u8(&e, 0x18); /* ADD RAX, [RDX+24] */
    emit_u8(&e, 0x48); emit_u8(&e, 0x89); emit_u8(&e, 0xC2); /* MOV RDX, RAX */
    emit_xor_eax_eax(&e);
    emit_ret(&e);

    /* fallback */
    patch_rel8(&e, jne0);
    patch_rel8(&e, jne1);
    emit_mov_rax_imm64(&e, (uint64_t)(uintptr_t)fallback);
    emit_jmp_rax(&e);

    uint8_t* code = jit_alloc(e.pos);
    if (!code) return NULL;
    memcpy(code, tmp, e.pos);
    return (JITFn)(void*)code;
}

/* ── 핫 카운터 + JIT 트리거 ── */
typedef struct JITMeta {
    uint32_t call_count;   /* 호출 횟수 */
    uint8_t  jit_compiled; /* JIT 완료 여부 */
    uint8_t  jit_type;     /* JIT 패턴 ID */
    int64_t  jit_const;    /* 상수 파라미터 (add/mul 패턴) */
} JITMeta;

/* 상수 반환 함수 (JIT 패턴 P1) */
FLValue fl_jit_probe_const(FLClosure* self, int argc, FLValue* argv) {
    (void)argc; (void)argv;
    /* self->env[0] = JITMeta 포인터 (FLValue로 래핑) */
    JITMeta* meta = (JITMeta*)(uintptr_t)((int64_t)self->env[0].i);
    if (!meta) return fl_nil();
    meta->call_count++;
    if (meta->call_count >= FL_JIT_THRESHOLD && !meta->jit_compiled) {
        JITFn fn = jit_build_const_int(meta->jit_const);
        if (fn) {
            self->call = fn;
            meta->jit_compiled = 1;
            fl_log(1, "jit", "P1 const(%lld) JIT 완료 (call #%u)",
                   (long long)meta->jit_const, meta->call_count);
        }
    }
    return fl_int(meta->jit_const);
}

/* ── JIT 통계 ── */
FLValue fl_jit_stats(void) {
    jit_pool_init();  /* lazy init — 첫 호출 시 풀 할당 */
    FLValue m = fl_map_new();
    m = fl_map_set(m, fl_str_val("pool_used_kb"),
                   fl_int((int64_t)(g_jit_pool.used / 1024)));
    m = fl_map_set(m, fl_str_val("pool_cap_kb"),
                   fl_int((int64_t)(g_jit_pool.cap / 1024)));
    m = fl_map_set(m, fl_str_val("pool_avail_kb"),
                   fl_int((int64_t)((g_jit_pool.cap - g_jit_pool.used) / 1024)));
    m = fl_map_set(m, fl_str_val("threshold"), fl_int(FL_JIT_THRESHOLD));
    m = fl_map_set(m, fl_str_val("ready"),
                   fl_bool(g_jit_pool.mem != NULL));
    return m;
}

/* JIT 풀 정리 */
void fl_jit_cleanup(void) {
    pthread_mutex_lock(&g_jit_lock);
    if (g_jit_pool.mem) {
        munmap(g_jit_pool.mem, g_jit_pool.cap);
        g_jit_pool.mem  = NULL;
        g_jit_pool.used = 0;
        g_jit_pool.cap  = 0;
    }
    pthread_mutex_unlock(&g_jit_lock);
}

/* ── FreeLang 인터페이스 ── */

/* FreeLang에서 호출: (jit-stats) → 맵 */
FLValue jit_stats(void) { return fl_jit_stats(); }

/* 직접 JIT 코드 실행 테스트 (개발용) */
FLValue jit_test_add1(FLValue x) {
    /* 테스트: (+ x 1) JIT 생성 및 실행 */
    static JITFn fn = NULL;
    if (!fn) {
        /* 폴백: 인터프리터 */
        fn = jit_build_add_int_const(1, NULL);
        if (fn) fl_log(1, "jit", "jit_test_add1 JIT 생성 완료");
    }
    if (fn) {
        FLValue argv[1] = { x };
        return fn(NULL, 1, argv);
    }
    /* 폴백 */
    return fl_add(x, fl_int(1));
}

FLValue jit_test_mul2(FLValue x) {
    static JITFn fn = NULL;
    if (!fn) {
        fn = jit_build_mul_int_const(2, NULL);
        if (fn) fl_log(1, "jit", "jit_test_mul2 JIT 생성 완료");
    }
    if (fn) {
        FLValue argv[1] = { x };
        return fn(NULL, 1, argv);
    }
    return fl_mul(x, fl_int(2));
}

FLValue jit_test_add_xy(FLValue x, FLValue y) {
    static JITFn fn = NULL;
    if (!fn) {
        fn = jit_build_add_two_ints(NULL);
        if (fn) fl_log(1, "jit", "jit_test_add_xy JIT 생성 완료");
    }
    if (fn) {
        FLValue argv[2] = { x, y };
        return fn(NULL, 2, argv);
    }
    return fl_add(x, y);
}
