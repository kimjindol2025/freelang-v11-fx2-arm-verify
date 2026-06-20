/*
 * sse.c — FreeLang fx SSE (Server-Sent Events)
 *
 * API:
 *   server_sse(path, "handler")  — SSE 라우트 등록
 *   sse_send(conn, data)         — "data: ...\n\n" 전송
 *   sse_close(conn)              — 연결 종료
 *   sse_alive(conn)              — 연결 살아있는지 확인
 *
 * 핸들러 시그니처:
 *   (defn handle-sse [$sse]
 *     (loop []
 *       (if (sse_alive $sse)
 *         (do (sse_send $sse "{\"type\":\"tick\"}")
 *             (sleep 1000)
 *             (recur))
 *         nil)))
 *
 * WebSocket 패턴 동일 — 핸드셰이크 없음 (HTTP 헤더만)
 *
 * 주의:
 *   - SO_RCVTIMEO 반드시 제거 (장기 연결)
 *   - sse_send 실패 시 closed = 1 (클라이언트 끊김 감지)
 *   - multiline: sse_send 호출 여러 번 or data 내 \n → "data: " prefix 반복
 */

#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <dlfcn.h>

/* ── SSE 연결 구조체 ── */
typedef struct {
    int fd;
    int closed;
} SSEConn;

/* ── SSE 라우트 테이블 ── */
#define MAX_SSE_ROUTES 16
typedef FLValue (*SSEHandlerFn)(FLValue conn);
typedef struct {
    char        path[256];
    SSEHandlerFn fn;
} SSERoute;

static SSERoute g_sse_routes[MAX_SSE_ROUTES];
static int      g_n_sse_routes = 0;

/* ── 내부 헬퍼 ── */
static const char* sse_strval(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

static SSEConn* get_sse_conn(FLValue v) {
    if (v.tag != FL_MAP) return NULL;
    FLValue ptr = fl_map_get(v, fl_str_val("__sse__"));
    if (ptr.tag != FL_INT) return NULL;
    return (SSEConn*)(uintptr_t)ptr.i;
}

static FLValue make_sse_conn(int fd) {
    SSEConn* c = malloc(sizeof(SSEConn));
    if (!c) return fl_nil();
    c->fd = fd;
    c->closed = 0;
    FLValue v = fl_map_new();
    v = fl_map_set(v, fl_str_val("__sse__"), fl_int((int64_t)(uintptr_t)c));
    v = fl_map_set(v, fl_str_val("type"), fl_str_val("sse"));
    return v;
}

/* ── SSE 헤더 전송 ── */
static int sse_do_open(int fd) {
    const char* hdr =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/event-stream\r\n"
        "Cache-Control: no-cache\r\n"
        "Connection: keep-alive\r\n"
        "X-Accel-Buffering: no\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n";
    return send(fd, hdr, strlen(hdr), 0) > 0 ? 0 : -1;
}

/* ── 경로 파싱 (첫 번째 줄 "GET /path HTTP/1.1" 추출) ── */
static void sse_parse_path(const char* raw, char* out, size_t out_sz) {
    const char* p = raw;
    while (*p && *p != ' ') p++;  /* "GET" 건너뜀 */
    while (*p == ' ') p++;
    const char* start = p;
    while (*p && *p != ' ' && *p != '?' && *p != '\r' && *p != '\n') p++;
    size_t len = (size_t)(p - start);
    if (len >= out_sz) len = out_sz - 1;
    memcpy(out, start, len);
    out[len] = '\0';
}

/* ── 공개 FL 함수 ── */

/* sse_send(conn, data) — "data: ...\n\n" 전송 */
FLValue sse_send(FLValue conn_v, FLValue data_v) {
    SSEConn* c = get_sse_conn(conn_v);
    if (!c || c->closed) return fl_bool(0);
    const char* data = sse_strval(data_v);
    /* data 내 \n 처리: 각 줄마다 "data: " prefix */
    char buf[16384];
    int n = 0;
    const char* line_start = data;
    while (*line_start) {
        const char* line_end = line_start;
        while (*line_end && *line_end != '\n') line_end++;
        int line_len = (int)(line_end - line_start);
        n += snprintf(buf + n, sizeof(buf) - n, "data: %.*s\n", line_len, line_start);
        if (*line_end == '\n') line_start = line_end + 1;
        else break;
    }
    /* 빈 줄로 이벤트 구분 */
    n += snprintf(buf + n, sizeof(buf) - n, "\n");
    int sent = (int)send(c->fd, buf, n, MSG_NOSIGNAL);
    if (sent <= 0) c->closed = 1;
    return fl_bool(sent > 0);
}

/* sse_close(conn) — 연결 종료 */
FLValue sse_close(FLValue conn_v) {
    SSEConn* c = get_sse_conn(conn_v);
    if (c && !c->closed) {
        c->closed = 1;
    }
    return fl_nil();
}

/* sse_alive(conn) — 연결 살아있는지 확인 */
FLValue sse_alive(FLValue conn_v) {
    SSEConn* c = get_sse_conn(conn_v);
    return fl_bool(c && !c->closed);
}

/* ── 서버 통합 ── */

/* SSE 라우트 등록 */
FLValue server_sse(FLValue path, FLValue handler_name) {
    if (g_n_sse_routes >= MAX_SSE_ROUTES) {
        fprintf(stderr, "[sse] 라우트 한도 초과\n");
        return fl_nil();
    }
    const char* h = sse_strval(handler_name);
    char cname[256];
    /* hyphen → underscore */
    size_t hlen = strlen(h);
    for (size_t i = 0; i < sizeof(cname) - 1 && i < hlen; i++)
        cname[i] = (h[i] == '-') ? '_' : h[i];
    cname[hlen < sizeof(cname) ? hlen : sizeof(cname) - 1] = '\0';

    SSEHandlerFn fn = (SSEHandlerFn)dlsym(RTLD_DEFAULT, cname);
    if (!fn) {
        fprintf(stderr, "[sse] 핸들러 '%s' (%s) 없음\n", h, cname);
        return fl_nil();
    }
    SSERoute* r = &g_sse_routes[g_n_sse_routes++];
    strncpy(r->path, sse_strval(path), sizeof(r->path) - 1);
    r->fn = fn;
    fprintf(stderr, "[sse] 라우트 등록: SSE %s → %s\n", sse_strval(path), cname);
    return fl_nil();
}

/* http.c에서 호출: SSE 요청 감지 (Accept: text/event-stream) */
int sse_is_sse_request(const char* raw) {
    return strcasestr(raw, "text/event-stream") != NULL;
}

/* http.c에서 호출: SSE 처리 + 핸들러 실행 */
int sse_handle_request(int fd, const char* raw, int raw_len) {
    (void)raw_len;
    char path[512] = {0};
    sse_parse_path(raw, path, sizeof(path));

    /* 라우트 매칭 */
    SSERoute* matched = NULL;
    for (int i = 0; i < g_n_sse_routes; i++) {
        if (strcmp(g_sse_routes[i].path, path) == 0) {
            matched = &g_sse_routes[i];
            break;
        }
    }
    if (!matched) return 0;  /* 일반 HTTP로 계속 */

    /* SSE 헤더 전송 */
    if (sse_do_open(fd) < 0) return 1;

    /* 장기 연결 — SO_RCVTIMEO 제거 (WS 패턴 동일) */
    struct timeval tv = {0, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    fprintf(stderr, "[sse] 연결: %s\n", path);

    /* 핸들러 실행 (현재 스레드 블로킹) */
    FLValue conn = make_sse_conn(fd);
    if (fl_try_top < FL_TRY_MAX) {
        FLTryFrame* frame = &fl_try_stack[fl_try_top++];
        if (setjmp(frame->buf) == 0) {
            matched->fn(conn);
            fl_try_top--;
        } else {
            fl_try_top--;
            fprintf(stderr, "[sse] 핸들러 오류: %s\n",
                    frame->err.tag == FL_STRING ? sse_strval(frame->err) : "unknown");
        }
    } else {
        matched->fn(conn);
    }

    /* 정리 */
    SSEConn* c = get_sse_conn(conn);
    if (c) {
        close(fd);
        free(c);
    }

    fprintf(stderr, "[sse] 연결 종료: %s\n", path);
    return 1;
}
