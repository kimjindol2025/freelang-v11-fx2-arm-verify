/*
 * debug.c — FreeLang 런타임 디버그 레이어
 *
 * 환경변수:
 *   FL_DEBUG=1        기본 디버그 (요청/응답 로그)
 *   FL_DEBUG=2        상세 (헤더 + body + SQL)
 *   FL_DEBUG=3        최상세 (내부 값 덤프)
 *   FL_LOG_FILE=path  파일로 출력 (기본: stderr)
 *   FL_NO_COLOR=1     ANSI 색상 비활성
 */

#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

/* ── 글로벌 설정 ─────────────────────────────────────── */
static int    g_debug_level = 0;
static FILE*  g_log_file    = NULL;
static int    g_use_color   = 1;
static pthread_once_t g_debug_init_once = PTHREAD_ONCE_INIT;

static void debug_init_impl(void) {
    const char* lv = getenv("FL_DEBUG");
    if (lv) g_debug_level = atoi(lv);

    const char* lf = getenv("FL_LOG_FILE");
    if (lf) {
        g_log_file = fopen(lf, "a");
        if (!g_log_file) g_log_file = stderr;
        g_use_color = 0;
    } else {
        g_log_file = stderr;
        g_use_color = isatty(fileno(stderr)) && !getenv("FL_NO_COLOR");
    }
}

static void debug_init(void) {
    pthread_once(&g_debug_init_once, debug_init_impl);
}

int fl_debug_level(void) {
    debug_init();
    return g_debug_level;
}

/* ── 색상 코드 ───────────────────────────────────────── */
#define C_RESET  (g_use_color ? "\033[0m"  : "")
#define C_GRAY   (g_use_color ? "\033[90m" : "")
#define C_GREEN  (g_use_color ? "\033[32m" : "")
#define C_YELLOW (g_use_color ? "\033[33m" : "")
#define C_RED    (g_use_color ? "\033[31m" : "")
#define C_CYAN   (g_use_color ? "\033[36m" : "")
#define C_BLUE   (g_use_color ? "\033[34m" : "")
#define C_BOLD   (g_use_color ? "\033[1m"  : "")

/* ── 타임스탬프 ─────────────────────────────────────── */
static void write_ts(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm t;
    localtime_r(&ts.tv_sec, &t);
    fprintf(g_log_file, "%s%02d:%02d:%02d.%03ld%s ",
        C_GRAY,
        t.tm_hour, t.tm_min, t.tm_sec,
        ts.tv_nsec / 1000000,
        C_RESET);
}

/* ── 핵심 로그 함수 ──────────────────────────────────── */
void fl_log(int level, const char* tag, const char* fmt, ...) {
    debug_init();
    if (g_debug_level < level) return;

    write_ts();

    const char* color = C_GRAY;
    if (level == 1) color = C_GREEN;
    else if (level == 2) color = C_CYAN;
    else if (level == 3) color = C_YELLOW;

    fprintf(g_log_file, "%s[%s]%s ", color, tag, C_RESET);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log_file, fmt, ap);
    va_end(ap);
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

void fl_log_error(const char* tag, const char* fmt, ...) {
    debug_init();
    write_ts();
    fprintf(g_log_file, "%s%s[ERROR %s]%s ", C_BOLD, C_RED, tag, C_RESET);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log_file, fmt, ap);
    va_end(ap);
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

/* ── HTTP 요청 로그 (level=1) ────────────────────────── */
void fl_log_request(const char* method, const char* path,
                    const char* body, size_t body_len,
                    const char* content_type) {
    debug_init();
    if (g_debug_level < 1) return;

    write_ts();

    /* method 색상 */
    const char* mc = C_GREEN;
    if (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0)   mc = C_YELLOW;
    if (strcmp(method, "DELETE") == 0) mc = C_RED;

    fprintf(g_log_file, "%s→%s %s%s%s %s%s%s\n",
        C_GRAY, C_RESET,
        mc, method, C_RESET,
        C_BOLD, path, C_RESET);

    if (g_debug_level >= 2 && body_len > 0 && body) {
        size_t show = body_len > 512 ? 512 : body_len;
        fprintf(g_log_file, "  %sbody%s [%zu] %.*s%s\n",
            C_GRAY, C_RESET,
            body_len, (int)show, body,
            body_len > 512 ? "..." : "");
    }
    fflush(g_log_file);
}

/* ── HTTP 응답 로그 (level=1) ────────────────────────── */
void fl_log_response(int status, const char* path, long elapsed_ms) {
    debug_init();
    if (g_debug_level < 1) return;

    write_ts();

    const char* sc = C_GREEN;
    if (status >= 300 && status < 400) sc = C_CYAN;
    if (status >= 400 && status < 500) sc = C_YELLOW;
    if (status >= 500) sc = C_RED;

    fprintf(g_log_file, "%s←%s %s%d%s %s %s%ldms%s\n",
        C_GRAY, C_RESET,
        sc, status, C_RESET,
        path,
        C_GRAY, elapsed_ms, C_RESET);
    fflush(g_log_file);
}

/* ── SQL 로그 (level=2) ──────────────────────────────── */
void fl_log_sql(const char* driver, const char* sql) {
    debug_init();
    if (g_debug_level < 2) return;

    write_ts();
    /* SQL 첫 80자만 표시 */
    size_t len = strlen(sql);
    size_t show = len > 80 ? 80 : len;
    fprintf(g_log_file, "%s[sql/%s]%s %.*s%s\n",
        C_BLUE, driver, C_RESET,
        (int)show, sql,
        len > 80 ? "..." : "");
    fflush(g_log_file);
}

/* ── FLValue 덤프 (level=3) ─────────────────────────── */
static void dump_val_inner(FLValue v, int depth) {
    if (depth > 3) { fprintf(g_log_file, "..."); return; }
    switch (v.tag) {
        case FL_NIL:    fprintf(g_log_file, "nil"); break;
        case FL_BOOL:   fprintf(g_log_file, "%s", v.b ? "true" : "false"); break;
        case FL_INT:    fprintf(g_log_file, "%lld", (long long)v.i); break;
        case FL_FLOAT:  fprintf(g_log_file, "%g", v.f); break;
        case FL_STRING:
            fprintf(g_log_file, "\"%s\"", ((FLString*)v.obj)->data);
            break;
        case FL_VECTOR: {
            FLVector* vec = (FLVector*)v.obj;
            fprintf(g_log_file, "[");
            for (uint32_t i = 0; i < vec->len && i < 5; i++) {
                if (i) fprintf(g_log_file, ", ");
                dump_val_inner(vec->data[i], depth + 1);
            }
            if (vec->len > 5) fprintf(g_log_file, ", ...(%u)", vec->len);
            fprintf(g_log_file, "]");
            break;
        }
        case FL_MAP: {
            FLMap* map = (FLMap*)v.obj;
            fprintf(g_log_file, "{");
            for (uint32_t i = 0; i < map->len && i < 4; i++) {
                if (i) fprintf(g_log_file, ", ");
                dump_val_inner(map->entries[i].key, depth + 1);
                fprintf(g_log_file, ": ");
                dump_val_inner(map->entries[i].val, depth + 1);
            }
            if (map->len > 4) fprintf(g_log_file, ", ...(%u)", map->len);
            fprintf(g_log_file, "}");
            break;
        }
        case FL_FN: fprintf(g_log_file, "<fn>"); break;
        default:    fprintf(g_log_file, "<?>");  break;
    }
}

FLValue fl_debug_dump(FLValue v) {
    debug_init();
    if (g_debug_level < 3) return v;
    write_ts();
    fprintf(g_log_file, "%s[dump]%s ", C_YELLOW, C_RESET);
    dump_val_inner(v, 0);
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
    return v;
}

/* ── 스타트업 배너 ───────────────────────────────────── */
void fl_debug_banner(const char* app_name, int port) {
    debug_init();
    if (g_debug_level < 1) return;

    write_ts();
    fprintf(g_log_file,
        "%s%s━━━ %s%s%s 시작 ━━━%s  포트:%s%d%s  debug:%s%d%s\n",
        C_BOLD, C_GREEN,
        C_RESET, C_BOLD, app_name,
        C_RESET,
        C_CYAN, port, C_RESET,
        C_YELLOW, g_debug_level, C_RESET);
    fflush(g_log_file);
}
