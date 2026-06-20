/*
 * websocket.c — FreeLang C 런타임 WebSocket 서버
 *
 * API:
 *   server_ws(path, "handler")  — WS 라우트 등록
 *   ws_recv(ws_conn)            — 메시지 수신 (text/binary)
 *   ws_send(ws_conn, msg)       — 메시지 전송
 *   ws_close(ws_conn)           — 연결 종료
 *   ws_conn_id(ws_conn)         — 연결 ID (string)
 *
 * 핸들러 시그니처:
 *   (defn handle-ws [$ws]
 *     (loop []
 *       (let [[$msg (ws_recv $ws)]]
 *         (if (nil? $msg)
 *           nil   ;; 연결 종료
 *           (do (ws_send $ws (str "Echo: " $msg))
 *               (recur))))))
 *
 * 프로토콜: RFC 6455 WebSocket (text frame only, no compression)
 * 의존: OpenSSL SHA-1 (libssl), base64 직접 구현
 */

#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <dlfcn.h>
#include <openssl/sha.h>

/* ── WS 연결 구조체 ── */
typedef struct {
    int      fd;
    int      closed;
    uint64_t id;
    char     query[512];
} WSConn;

/* ── WS 라우트 테이블 ── */
#define MAX_WS_ROUTES 32
typedef FLValue (*WSHandlerFn)(FLValue ws);
typedef struct {
    char        path[256];
    WSHandlerFn fn;
} WSRoute;

static WSRoute g_ws_routes[MAX_WS_ROUTES];
static int     g_n_ws_routes = 0;
static uint64_t g_ws_id_counter = 0;

/* ── 내부 헬퍼 ── */
static const char* ws_strval(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

/* Base64 인코딩 */
static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(const unsigned char* in, size_t in_len,
                          char* out, size_t out_size) {
    size_t i = 0, j = 0;
    while (i < in_len) {
        unsigned char b0 = in[i++];
        int has_b1 = (i < in_len);
        unsigned char b1 = has_b1 ? in[i++] : 0;
        int has_b2 = (i < in_len);
        unsigned char b2 = has_b2 ? in[i++] : 0;
        if (j + 4 >= out_size) break;
        out[j++] = B64_TABLE[b0 >> 2];
        out[j++] = B64_TABLE[((b0 & 3) << 4) | (b1 >> 4)];
        out[j++] = has_b1 ? B64_TABLE[((b1 & 0xf) << 2) | (b2 >> 6)] : '=';
        out[j++] = has_b2 ? B64_TABLE[b2 & 0x3f] : '=';
    }
    out[j] = '\0';
}

/* WebSocket 핸드셰이크 Accept 키 계산 */
static void compute_ws_accept(const char* client_key, char* out, size_t out_size) {
    static const char MAGIC[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", client_key, MAGIC);

    unsigned char sha1[20];
    SHA1((const unsigned char*)combined, strlen(combined), sha1);
    base64_encode(sha1, 20, out, out_size);
}

/* ── WebSocket 핸드셰이크 수행 ── */
/* raw: 원본 HTTP 요청 데이터, raw_len: 길이 */
/* key_out: Sec-WebSocket-Key 값 저장 */
static int ws_parse_upgrade(const char* raw, int raw_len,
                             char* key_out, size_t key_size,
                             char* path_out, size_t path_size) {
    /* Path 추출 */
    const char* p = raw;
    while (*p && *p != ' ') p++;  /* skip method */
    while (*p == ' ') p++;
    const char* path_start = p;
    while (*p && *p != ' ' && *p != '?') p++;
    size_t plen = (size_t)(p - path_start);
    if (plen >= path_size) plen = path_size - 1;
    memcpy(path_out, path_start, plen);
    path_out[plen] = '\0';

    /* Sec-WebSocket-Key 헤더 추출 */
    const char* key_hdr = strcasestr(raw, "Sec-WebSocket-Key:");
    if (!key_hdr) return -1;
    key_hdr += 18;
    while (*key_hdr == ' ') key_hdr++;
    const char* key_end = key_hdr;
    while (*key_end && *key_end != '\r' && *key_end != '\n') key_end++;
    size_t klen = (size_t)(key_end - key_hdr);
    if (klen >= key_size) klen = key_size - 1;
    memcpy(key_out, key_hdr, klen);
    key_out[klen] = '\0';

    /* Upgrade: websocket 확인 */
    if (!strcasestr(raw, "Upgrade: websocket") &&
        !strcasestr(raw, "upgrade: websocket")) return -1;

    (void)raw_len;
    return 0;
}

static int ws_do_handshake(int fd, const char* key, const char* path) {
    char accept_key[64];
    compute_ws_accept(key, accept_key, sizeof(accept_key));

    char resp[512];
    int n = snprintf(resp, sizeof(resp),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "\r\n",
        accept_key);
    return send(fd, resp, n, 0) == n ? 0 : -1;
}

/* ── WebSocket 프레임 읽기 ── */
/* 반환: 동적 할당된 payload (호출자가 free), *out_len에 길이 저장 */
/* 연결 종료/오류: NULL 반환 */
static char* ws_read_frame(int fd, size_t* out_len, int* is_close) {
    *is_close = 0;

    /* 최소 2바이트 헤더 읽기 */
    unsigned char hdr[2];
    ssize_t n = recv(fd, hdr, 2, MSG_WAITALL);
    if (n != 2) return NULL;

    /* int fin = (hdr[0] >> 7) & 1; */
    int opcode = hdr[0] & 0x0f;
    int masked  = (hdr[1] >> 7) & 1;
    uint64_t payload_len = hdr[1] & 0x7f;

    if (payload_len == 126) {
        unsigned char ext[2];
        if (recv(fd, ext, 2, MSG_WAITALL) != 2) return NULL;
        payload_len = ((uint64_t)ext[0] << 8) | ext[1];
    } else if (payload_len == 127) {
        unsigned char ext[8];
        if (recv(fd, ext, 8, MSG_WAITALL) != 8) return NULL;
        payload_len = 0;
        for (int i = 0; i < 8; i++)
            payload_len = (payload_len << 8) | ext[i];
    }

    /* 연결 종료 프레임 */
    if (opcode == 0x8) { *is_close = 1; return NULL; }
    /* ping → pong (빈 응답) */
    if (opcode == 0x9) {
        unsigned char pong[2] = {0x8a, 0x00};
        send(fd, pong, 2, 0);
        return ws_read_frame(fd, out_len, is_close);
    }

    /* 마스킹 키 */
    unsigned char mask[4] = {0};
    if (masked) {
        if (recv(fd, mask, 4, MSG_WAITALL) != 4) return NULL;
    }

    /* payload 크기 한도 (16MB) */
    if (payload_len > 16 * 1024 * 1024) return NULL;

    char* buf = malloc(payload_len + 1);
    if (!buf) return NULL;

    if (payload_len > 0) {
        ssize_t got = recv(fd, buf, (size_t)payload_len, MSG_WAITALL);
        if (got != (ssize_t)payload_len) { free(buf); return NULL; }
        if (masked) {
            for (uint64_t i = 0; i < payload_len; i++)
                buf[i] ^= mask[i % 4];
        }
    }
    buf[payload_len] = '\0';
    *out_len = (size_t)payload_len;
    return buf;
}

/* ── WebSocket 프레임 쓰기 (text frame, unmasked) ── */
static int ws_write_frame(int fd, const char* data, size_t len) {
    unsigned char hdr[10];
    int hdr_len = 0;

    hdr[hdr_len++] = 0x81;  /* FIN + opcode text */

    if (len < 126) {
        hdr[hdr_len++] = (unsigned char)len;
    } else if (len < 65536) {
        hdr[hdr_len++] = 126;
        hdr[hdr_len++] = (unsigned char)(len >> 8);
        hdr[hdr_len++] = (unsigned char)(len & 0xff);
    } else {
        hdr[hdr_len++] = 127;
        for (int i = 7; i >= 0; i--)
            hdr[hdr_len++] = (unsigned char)((len >> (8 * i)) & 0xff);
    }

    /* header + data 한 번에 전송 */
    struct iovec iov[2];
    iov[0].iov_base = hdr;
    iov[0].iov_len  = hdr_len;
    iov[1].iov_base = (void*)data;
    iov[1].iov_len  = len;

    struct msghdr msg = {0};
    msg.msg_iov    = iov;
    msg.msg_iovlen = 2;
    return sendmsg(fd, &msg, 0) >= 0 ? 0 : -1;
}

/* ── FLValue WSConn 래퍼 ── */
/* ws_conn은 FL_MAP {"__ws__": ptr, "id": id_str} */

/* URL 디코딩 (websocket 전용, static) */
static void ws_url_decode(const char* src, char* dst, size_t max) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 1 < max; i++) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            char hex[3] = {src[i+1], src[i+2], 0};
            dst[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (src[i] == '+') {
            dst[j++] = ' ';
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

/* "key=val&key2=val2" → FL_MAP */
static FLValue parse_query_string(const char* qs) {
    FLValue m = fl_map_new();
    if (!qs || !*qs) return m;
    char buf[512];
    strncpy(buf, qs, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char* p = buf;
    while (p && *p) {
        char* amp = strchr(p, '&');
        if (amp) *amp = '\0';
        char* eq = strchr(p, '=');
        if (eq) {
            *eq = '\0';
            char dk[128] = {0}, dv[256] = {0};
            ws_url_decode(p, dk, sizeof(dk));
            ws_url_decode(eq + 1, dv, sizeof(dv));
            m = fl_map_set(m, fl_str_val(dk), fl_str_val(dv));
        }
        p = amp ? amp + 1 : NULL;
    }
    return m;
}

static FLValue make_ws_conn_q(int fd, const char* query_str) {
    WSConn* conn = malloc(sizeof(WSConn));
    conn->fd     = fd;
    conn->closed = 0;
    conn->id     = __atomic_fetch_add(&g_ws_id_counter, 1, __ATOMIC_RELAXED);
    strncpy(conn->query, query_str ? query_str : "", sizeof(conn->query) - 1);

    FLValue m = fl_map_new();
    m = fl_map_set(m, fl_str_val("__ws__"), fl_int((int64_t)(uintptr_t)conn));
    char id_buf[32];
    snprintf(id_buf, sizeof(id_buf), "ws-%lu", (unsigned long)conn->id);
    m = fl_map_set(m, fl_str_val("id"), fl_str_val(id_buf));
    m = fl_map_set(m, fl_str_val("query"), parse_query_string(query_str));
    return m;
}

/* 하위 호환성 */
static FLValue make_ws_conn(int fd) {
    return make_ws_conn_q(fd, "");
}

static WSConn* get_ws_conn(FLValue v) {
    if (v.tag != FL_MAP) return NULL;
    FLValue ptr_v = fl_map_get(v, fl_str_val("__ws__"));
    if (ptr_v.tag != FL_INT) return NULL;
    return (WSConn*)(uintptr_t)ptr_v.i;
}

/* ── FL API ── */

/* ws_recv(ws) — 다음 메시지 수신 (nil = 연결 종료) */
FLValue ws_recv(FLValue ws) {
    WSConn* conn = get_ws_conn(ws);
    if (!conn || conn->closed) return fl_nil();

    size_t len;
    int is_close = 0;
    char* data = ws_read_frame(conn->fd, &len, &is_close);

    if (!data || is_close) {
        if (conn) conn->closed = 1;
        free(data);
        return fl_nil();
    }

    FLValue result = fl_str_val(data);
    free(data);
    return result;
}

/* ws_send(ws, msg) — 메시지 전송 */
FLValue ws_send(FLValue ws, FLValue msg) {
    WSConn* conn = get_ws_conn(ws);
    if (!conn || conn->closed) return fl_bool(0);

    const char* s = ws_strval(msg);
    int r = ws_write_frame(conn->fd, s, strlen(s));
    return fl_bool(r == 0);
}

/* ws_close(ws) — 연결 종료 프레임 전송 후 닫기 */
FLValue ws_close(FLValue ws) {
    WSConn* conn = get_ws_conn(ws);
    if (!conn || conn->closed) return fl_nil();
    conn->closed = 1;
    unsigned char close_frame[2] = {0x88, 0x00};
    send(conn->fd, close_frame, 2, 0);
    close(conn->fd);
    free(conn);
    return fl_nil();
}

/* ws_conn_id(ws) — 연결 ID 문자열 */
FLValue ws_conn_id(FLValue ws) {
    FLValue id_v = fl_map_get(ws, fl_str_val("id"));
    return (id_v.tag == FL_STRING) ? id_v : fl_str_val("ws-unknown");
}

/* ── 서버 통합: http.c에서 호출 ── */

/* WS 라우트 등록 */
FLValue server_ws(FLValue path, FLValue handler_name) {
    if (g_n_ws_routes >= MAX_WS_ROUTES) {
        fprintf(stderr, "[ws] 라우트 한도 초과\n");
        return fl_nil();
    }

    const char* h = ws_strval(handler_name);
    char cname[256];
    /* hyphen → underscore */
    for (size_t i = 0; i < sizeof(cname) - 1 && h[i]; i++)
        cname[i] = (h[i] == '-') ? '_' : h[i];
    cname[strlen(h)] = '\0';

    WSHandlerFn fn = (WSHandlerFn)dlsym(RTLD_DEFAULT, cname);
    if (!fn) {
        fprintf(stderr, "[ws] 핸들러 '%s' (%s) 없음\n", h, cname);
        return fl_nil();
    }

    WSRoute* r = &g_ws_routes[g_n_ws_routes++];
    strncpy(r->path, ws_strval(path), sizeof(r->path) - 1);
    r->fn = fn;
    fprintf(stderr, "[ws] 라우트 등록: WS %s → %s\n", ws_strval(path), cname);
    return fl_nil();
}

/* http.c에서 호출: Upgrade 요청인지 확인 */
int ws_is_upgrade_request(const char* raw) {
    return strcasestr(raw, "Upgrade: websocket") != NULL ||
           strcasestr(raw, "upgrade: websocket") != NULL;
}

/* http.c에서 호출: WS 업그레이드 처리 + 핸들러 실행 */
/* 반환 1: WS 처리됨, 0: 일반 HTTP로 계속 */
int ws_handle_upgrade(int fd, const char* raw, int raw_len) {
    char ws_key[128] = {0};
    char ws_path[512] = {0};

    /* 쿼리 스트링도 분리 파싱 */
    char ws_query[512] = {0};
    if (ws_parse_upgrade(raw, raw_len, ws_key, sizeof(ws_key),
                         ws_path, sizeof(ws_path)) < 0)
        return 0;
    /* 원본 요청에서 ? 이후 추출 */
    const char* q_start = raw;
    while (*q_start && *q_start != ' ') q_start++;
    while (*q_start == ' ') q_start++;
    const char* q_mark = q_start;
    while (*q_mark && *q_mark != '?' && *q_mark != ' ') q_mark++;
    if (*q_mark == '?') {
        q_mark++;
        const char* q_end = q_mark;
        while (*q_end && *q_end != ' ' && *q_end != '#') q_end++;
        size_t qlen = (size_t)(q_end - q_mark);
        if (qlen >= sizeof(ws_query)) qlen = sizeof(ws_query) - 1;
        memcpy(ws_query, q_mark, qlen);
        ws_query[qlen] = '\0';
    }

    /* 라우트 매칭 */
    WSRoute* matched = NULL;
    for (int i = 0; i < g_n_ws_routes; i++) {
        if (strcmp(g_ws_routes[i].path, ws_path) == 0) {
            matched = &g_ws_routes[i];
            break;
        }
    }
    if (!matched) return 0;

    /* 핸드셰이크 */
    if (ws_do_handshake(fd, ws_key, ws_path) < 0) return 1;

    /* WS는 장기 연결 — recv_one_request가 설정한 단기 타임아웃 제거 */
    struct timeval ws_tv = {0, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &ws_tv, sizeof(ws_tv));

    fprintf(stderr, "[ws] 연결: %s?%s\n", ws_path, ws_query);

    /* 핸들러 실행 (현재 스레드에서 블로킹) */
    FLValue ws_conn = make_ws_conn_q(fd, ws_query);
    if (fl_try_top < FL_TRY_MAX) {
        FLTryFrame* frame = &fl_try_stack[fl_try_top++];
        if (setjmp(frame->buf) == 0) {
            matched->fn(ws_conn);
            fl_try_top--;
        } else {
            fl_try_top--;
            fprintf(stderr, "[ws] 핸들러 오류: %s\n",
                    frame->err.tag == FL_STRING ? ws_strval(frame->err) : "unknown");
        }
    } else {
        matched->fn(ws_conn);
    }

    /* 정리 */
    WSConn* conn = get_ws_conn(ws_conn);
    if (conn && !conn->closed) {
        unsigned char close_frame[2] = {0x88, 0x00};
        send(fd, close_frame, 2, 0);
        free(conn);
    }

    fprintf(stderr, "[ws] 연결 종료: %s\n", ws_path);
    return 1;
}
