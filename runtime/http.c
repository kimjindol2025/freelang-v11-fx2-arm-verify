/*
 * http.c — FreeLang C 런타임 HTTP 서버
 *
 * 구현 함수:
 *   server_get / server_post / server_put / server_patch / server_delete
 *   server_html / server_text / server_status / server_json
 *   server_start
 *   server_req_param / server_req_query / fxb_server_req_body / server_req_header
 *   server_redirect
 *
 * 의존: POSIX sockets + dlsym (외부 라이브러리 없음)
 * 빌드: gcc ... -rdynamic -lpthread
 */

#define _GNU_SOURCE
#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

/* ───────────────────────────────────────────
   내부 타입
─────────────────────────────────────────── */

#define MAX_ROUTES      256
#define MAX_HEADERS     64
#define RECV_BUF        65536
#define SEND_BUF        65536

/* ── 보안 한도 ── */
#define MAX_URI_LEN         8192        /* URI 최대 길이 → 414 */
#define MAX_HEADER_TOTAL    32768       /* 헤더 총합 최대 → 431 */
#define MAX_BODY_SIZE       (10*1024*1024) /* Body 최대 10MB → 413 */
#define HEADER_RECV_TIMEOUT 10          /* 헤더 완성 타임아웃 (초) */
#define MAX_CONNECTIONS     512         /* 동시 연결 한도 → 503 */

typedef FLValue (*HandlerFn)(FLValue req);

typedef struct {
    char   method[8];      /* GET POST PUT PATCH DELETE */
    char   path[512];      /* /api/users/:id */
    HandlerFn fn;
    FLValue  closure;      /* FL_FN 클로저 핸들러 (fn이 NULL일 때 사용) */
} Route;

/* ── 응답 FLMap 키 상수 ── */
static const char* K_STATUS  = "status";
static const char* K_BODY    = "body";
static const char* K_HEADERS = "headers";
static const char* K_TYPE    = "type";

/* ── 라우트 테이블 ── */
static Route   g_routes[MAX_ROUTES];
static int     g_nroutes = 0;

/* ── 현재 요청 context (per-thread 확장 가능하나 단순 구현은 전역) ── */
static __thread FLValue g_current_req;

/* ───────────────────────────────────────────
   내부 헬퍼
─────────────────────────────────────────── */

static const char* strval(FLValue v) {
    if (v.tag == FL_STRING && v.obj) return ((FLString*)v.obj)->data;
    return "";
}

/* "handle-index" → "handle_index" (hyphen to underscore) */
static void fn_name_to_c(const char* fl_name, char* out, size_t max) {
    size_t i;
    for (i = 0; i < max - 1 && fl_name[i]; i++) {
        out[i] = (fl_name[i] == '-') ? '_' : fl_name[i];
    }
    out[i] = '\0';
}

/* FLMap 빠른 생성 헬퍼 */
static FLValue make_response(int status, const char* content_type, const char* body) {
    FLValue headers = fl_map_new();
    headers = fl_map_set(headers, fl_str_val("Content-Type"), fl_str_val(content_type));

    FLValue resp = fl_map_new();
    resp = fl_map_set(resp, fl_str_val(K_STATUS),  fl_int(status));
    resp = fl_map_set(resp, fl_str_val(K_BODY),    fl_str_val(body));
    resp = fl_map_set(resp, fl_str_val(K_HEADERS), headers);
    return resp;
}

/* ───────────────────────────────────────────
   라우트 등록 API
─────────────────────────────────────────── */

static void register_route(const char* method, FLValue path, FLValue handler) {
    if (g_nroutes >= MAX_ROUTES) {
        fprintf(stderr, "[http] 라우트 한도 초과\n");
        return;
    }
    const char* p = strval(path);
    Route* r = &g_routes[g_nroutes++];
    strncpy(r->method, method, sizeof(r->method) - 1);
    strncpy(r->path,   p,      sizeof(r->path) - 1);
    r->closure = fl_nil();
    r->fn = NULL;

    if (handler.tag == FL_FN) {
        /* FL_FN 클로저 핸들러 */
        r->closure = handler;
        fprintf(stderr, "[http] 라우트 등록: %s %s → <closure>\n", method, p);
    } else {
        /* 문자열 이름 → dlsym 룩업 */
        const char* h = strval(handler);
        char cname[256];
        fn_name_to_c(h, cname, sizeof(cname));
        HandlerFn fn = (HandlerFn)dlsym(RTLD_DEFAULT, cname);
        if (!fn) {
            fprintf(stderr, "[http] 핸들러 '%s' (%s) 를 찾을 수 없음\n", h, cname);
            g_nroutes--;
            return;
        }
        r->fn = fn;
        fprintf(stderr, "[http] 라우트 등록: %s %s → %s\n", method, p, cname);
    }
}

FLValue server_get(FLValue path, FLValue handler) {
    register_route("GET",    path, handler); return fl_nil();
}
FLValue server_post(FLValue path, FLValue handler) {
    register_route("POST",   path, handler); return fl_nil();
}
FLValue server_put(FLValue path, FLValue handler) {
    register_route("PUT",    path, handler); return fl_nil();
}
FLValue server_patch(FLValue path, FLValue handler) {
    register_route("PATCH",  path, handler); return fl_nil();
}
FLValue server_delete(FLValue path, FLValue handler) {
    register_route("DELETE", path, handler); return fl_nil();
}

/* ───────────────────────────────────────────
   응답 생성 API
─────────────────────────────────────────── */

FLValue server_html(FLValue html) {
    return make_response(200, "text/html; charset=utf-8", strval(html));
}

FLValue server_text(FLValue text) {
    return make_response(200, "text/plain; charset=utf-8", strval(text));
}

FLValue server_json(FLValue json_str) {
    return make_response(200, "application/json; charset=utf-8", strval(json_str));
}

FLValue server_status(FLValue code, FLValue body) {
    int status = (code.tag == FL_INT) ? (int)code.i : 200;
    return make_response(status, "text/plain; charset=utf-8", strval(body));
}

FLValue server_redirect(FLValue url) {
    FLValue headers = fl_map_new();
    headers = fl_map_set(headers, fl_str_val("Location"), url);
    FLValue resp = fl_map_new();
    resp = fl_map_set(resp, fl_str_val(K_STATUS),  fl_int(302));
    resp = fl_map_set(resp, fl_str_val(K_BODY),    fl_str_val(""));
    resp = fl_map_set(resp, fl_str_val(K_HEADERS), headers);
    return resp;
}

/* ───────────────────────────────────────────
   요청 접근자 API
─────────────────────────────────────────── */

FLValue server_req_param(FLValue req, FLValue name) {
    FLValue params = fl_map_get(req, fl_str_val("params"));
    if (params.tag == FL_MAP) return fl_map_get(params, name);
    return fl_nil();
}

FLValue server_req_query(FLValue req, FLValue name) {
    FLValue query = fl_map_get(req, fl_str_val("query"));
    if (query.tag == FL_MAP) return fl_map_get(query, name);
    return fl_nil();
}

FLValue fxb_server_req_body(FLValue req) {
    return fl_map_get(req, fl_str_val("body"));
}

FLValue server_req_header(FLValue req, FLValue name) {
    FLValue headers = fl_map_get(req, fl_str_val("headers"));
    if (headers.tag == FL_MAP) return fl_map_get(headers, name);
    return fl_nil();
}

FLValue server_req_method(FLValue req) {
    return fl_map_get(req, fl_str_val("method"));
}

FLValue server_req_path(FLValue req) {
    return fl_map_get(req, fl_str_val("path"));
}

/* ───────────────────────────────────────────
   HTTP 파서
─────────────────────────────────────────── */

typedef struct {
    char method[8];
    char path[1024];
    char query_str[1024];
    char headers[MAX_HEADERS][2][512];  /* [i][0]=name [i][1]=value */
    int  nheaders;
    char body[RECV_BUF];
    int  body_len;
    int  content_length;
    char content_type[256];
} HttpRequest;

/* URL 디코드 (%XX → char) */
static void http_url_decode_internal(const char* src, char* dst, size_t max) {
    size_t i = 0;
    while (*src && i < max - 1) {
        if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], 0 };
            dst[i++] = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            dst[i++] = ' '; src++;
        } else {
            dst[i++] = *src++;
        }
    }
    dst[i] = '\0';
}

/* query string 파싱 → FLMap */
static FLValue parse_query(const char* qs) {
    FLValue map = fl_map_new();
    if (!qs || !*qs) return map;
    char buf[1024];
    strncpy(buf, qs, sizeof(buf) - 1);
    char* pair = strtok(buf, "&");
    while (pair) {
        char* eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            char k[512], v[512];
            http_url_decode_internal(pair, k, sizeof(k));
            http_url_decode_internal(eq + 1, v, sizeof(v));
            map = fl_map_set(map, fl_str_val(k), fl_str_val(v));
        }
        pair = strtok(NULL, "&");
    }
    return map;
}

/* HTTP 요청 파싱 */
static int parse_http_request(const char* raw, int raw_len, HttpRequest* req) {
    (void)raw_len;
    memset(req, 0, sizeof(*req));

    /* 요청 라인 파싱 */
    char line[2048];
    const char* p = raw;
    const char* eol = strstr(p, "\r\n");
    if (!eol) return -1;
    int llen = (int)(eol - p);
    if (llen >= (int)sizeof(line)) return -1;
    strncpy(line, p, llen); line[llen] = '\0';

    char path_and_query[1024];
    if (sscanf(line, "%7s %1023s", req->method, path_and_query) != 2) return -1;

    /* path / query 분리 */
    char* q = strchr(path_and_query, '?');
    if (q) {
        *q = '\0';
        strncpy(req->query_str, q + 1, sizeof(req->query_str) - 1);
    }
    http_url_decode_internal(path_and_query, req->path, sizeof(req->path));

    /* 헤더 파싱 */
    p = eol + 2;
    while (*p && !(p[0] == '\r' && p[1] == '\n')) {
        eol = strstr(p, "\r\n");
        if (!eol) break;
        llen = (int)(eol - p);
        if (llen >= (int)sizeof(line)) { p = eol + 2; continue; }
        strncpy(line, p, llen); line[llen] = '\0';
        char* colon = strchr(line, ':');
        if (colon && req->nheaders < MAX_HEADERS) {
            *colon = '\0';
            const char* val = colon + 1;
            while (*val == ' ') val++;
            strncpy(req->headers[req->nheaders][0], line, 511);
            strncpy(req->headers[req->nheaders][1], val,  511);
            if (strcasecmp(line, "Content-Length") == 0)
                req->content_length = atoi(val);
            if (strcasecmp(line, "Content-Type") == 0)
                strncpy(req->content_type, val, sizeof(req->content_type) - 1);
            req->nheaders++;
        }
        p = eol + 2;
    }
    p += 2; /* 빈 줄 건너뜀 */

    /* 바디 */
    if (req->content_length > 0) {
        int bl = req->content_length;
        if (bl >= RECV_BUF) bl = RECV_BUF - 1;
        memcpy(req->body, p, bl);
        req->body[bl] = '\0';
        req->body_len = bl;
    }
    return 0;
}

/* ───────────────────────────────────────────
   라우트 매칭 + 파라미터 추출
─────────────────────────────────────────── */

/* /users/:id  vs  /users/42  → params["id"] = "42" */
static int match_route(const Route* r, const char* method, const char* path, FLValue* params) {
    if (strcmp(r->method, method) != 0) return 0;

    const char* rp = r->path;
    const char* pp = path;
    *params = fl_map_new();

    while (*rp && *pp) {
        if (*rp == ':') {
            /* 파라미터 세그먼트 */
            rp++;
            char pname[128]; int pi = 0;
            while (*rp && *rp != '/') pname[pi++] = *rp++;
            pname[pi] = '\0';
            char pval[512]; int vi = 0;
            while (*pp && *pp != '/') pval[vi++] = *pp++;
            pval[vi] = '\0';
            *params = fl_map_set(*params, fl_str_val(pname), fl_str_val(pval));
        } else if (*rp == *pp) {
            rp++; pp++;
        } else {
            return 0;
        }
    }
    /* 양쪽 끝에 도달하거나 라우트가 / 로 끝나는 경우 */
    return (*rp == '\0' && (*pp == '\0' || *pp == '?'));
}

/* ───────────────────────────────────────────
   HTTP 응답 전송
─────────────────────────────────────────── */

static const char* status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 422: return "Unprocessable Entity";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

/* keep_alive=1이면 Connection: keep-alive 헤더 포함 */
static void send_response(int client_fd, FLValue resp, int keep_alive) {
    int    status  = 200;
    const char* body    = "";
    const char* ctype   = "text/plain";
    char   extra_headers[2048] = "";

    if (resp.tag == FL_MAP) {
        FLValue sv = fl_map_get(resp, fl_str_val(K_STATUS));
        if (sv.tag == FL_INT) status = (int)sv.i;

        FLValue bv = fl_map_get(resp, fl_str_val(K_BODY));
        if (bv.tag == FL_STRING) body = strval(bv);

        FLValue hv = fl_map_get(resp, fl_str_val(K_HEADERS));
        if (hv.tag == FL_MAP) {
            FLMap* hm = (FLMap*)hv.obj;
            for (uint32_t i = 0; i < hm->len; i++) {
                const char* k = strval(hm->entries[i].key);
                const char* v = strval(hm->entries[i].val);
                if (strcasecmp(k, "Content-Type") == 0) {
                    ctype = v;
                } else {
                    char hbuf[512];
                    snprintf(hbuf, sizeof(hbuf), "%s: %s\r\n", k, v);
                    strncat(extra_headers, hbuf, sizeof(extra_headers) - strlen(extra_headers) - 1);
                }
            }
        }
    } else if (resp.tag == FL_STRING) {
        body  = strval(resp);
        ctype = "text/html; charset=utf-8";
    }

    size_t blen = strlen(body);
    char header_buf[4096];
    snprintf(header_buf, sizeof(header_buf),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: %s\r\n"
        "%s"
        "\r\n",
        status, status_text(status),
        ctype,
        blen,
        keep_alive ? "keep-alive" : "close",
        extra_headers
    );

    send(client_fd, header_buf, strlen(header_buf), 0);
    if (blen > 0) send(client_fd, body, blen, 0);
}

/* ───────────────────────────────────────────
   요청 → FLMap 변환
─────────────────────────────────────────── */

static FLValue make_req_map(HttpRequest* hr, FLValue params) {
    /* 헤더 맵 */
    FLValue headers = fl_map_new();
    for (int i = 0; i < hr->nheaders; i++) {
        headers = fl_map_set(headers,
            fl_str_val(hr->headers[i][0]),
            fl_str_val(hr->headers[i][1]));
    }

    /* body — Content-Type에 따라 자동 파싱
     * application/json → json_parse (맵/벡터)
     * 기타 → 문자열 그대로 */
    FLValue body;
    {
        const char* ct = "";
        for (int i = 0; i < hr->nheaders; i++) {
            if (strcasecmp(hr->headers[i][0], "content-type") == 0) {
                ct = hr->headers[i][1]; break;
            }
        }
        const char* raw = hr->body_len > 0 ? hr->body : "";
        if (strstr(ct, "application/json") && hr->body_len > 0) {
            FLValue parsed = json_parse(fl_str_val(raw));
            body = (parsed.tag == FL_MAP || parsed.tag == FL_VECTOR)
                   ? parsed : fl_str_val(raw);
        } else {
            body = fl_str_val(raw);
        }
    }

    FLValue req = fl_map_new();
    req = fl_map_set(req, fl_str_val("method"),  fl_str_val(hr->method));
    req = fl_map_set(req, fl_str_val("path"),    fl_str_val(hr->path));
    req = fl_map_set(req, fl_str_val("query"),   parse_query(hr->query_str));
    req = fl_map_set(req, fl_str_val("params"),  params);
    req = fl_map_set(req, fl_str_val("headers"), headers);
    req = fl_map_set(req, fl_str_val("body"),    body);
    return req;
}

/* ───────────────────────────────────────────
   연결 처리 (단일 요청)
─────────────────────────────────────────── */

typedef struct { int fd; } ConnArg;

/*
 * keep-alive 설정:
 * - HTTP/1.1 기본 keep-alive, HTTP/1.0 기본 close
 * - 클라이언트 Connection 헤더 우선
 * - 최대 100 요청 또는 60초 유휴시간 후 자동 close
 */
#define KEEPALIVE_MAX_REQUESTS 100
#define KEEPALIVE_IDLE_SEC     60

/* 단일 요청 수신
 *
 * 반환값:
 *   >0   : 수신 바이트 수 (정상)
 *   -408 : 헤더 미완성 timeout (Slowloris) → 408
 *   -413 : Content-Length 초과 → 413
 *   -414 : URI 너무 김 → 414
 *   -431 : 헤더 총합 초과 → 431
 *   <=0  : 연결 종료 or 기타 오류 → break
 */
static int recv_one_request(int fd, char* raw, int buf_size) {
    /* 헤더 수신 타임아웃 (짧은 값) — Slowloris 방어 */
    struct timeval hdr_tv = { .tv_sec = HEADER_RECV_TIMEOUT, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &hdr_tv, sizeof(hdr_tv));

    int total = 0;
    int n;
    int headers_done = 0;

    while ((n = recv(fd, raw + total, buf_size - total - 1, 0)) > 0) {
        total += n;
        raw[total] = '\0';

        if (!headers_done) {
            /* 헤더 총합 한도 초과 */
            if (total > MAX_HEADER_TOTAL) {
                raw[0] = '\0';
                return -431;
            }

            char* hdr_end = strstr(raw, "\r\n\r\n");
            if (hdr_end) {
                headers_done = 1;

                /* URI 길이 검사 */
                char* uri_start = strchr(raw, ' ');
                if (uri_start) {
                    uri_start++;
                    char* uri_end = strchr(uri_start, ' ');
                    if (uri_end && (uri_end - uri_start) > MAX_URI_LEN) {
                        raw[0] = '\0';
                        return -414;
                    }
                }

                /* Content-Length 검사 — 초과 시 즉시 413 */
                char* cl_str = strcasestr(raw, "Content-Length:");
                if (cl_str) {
                    long cl = atol(cl_str + 15);
                    if (cl > MAX_BODY_SIZE) {
                        raw[0] = '\0';
                        return -413;
                    }
                    /* 바디 수신 시 더 긴 타임아웃 적용 */
                    struct timeval body_tv = { .tv_sec = KEEPALIVE_IDLE_SEC, .tv_usec = 0 };
                    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &body_tv, sizeof(body_tv));

                    int body_recv = total - (int)(hdr_end + 4 - raw);
                    if (body_recv >= (int)cl) break;
                } else {
                    break;   /* 바디 없음 */
                }
            }
        } else {
            /* 바디 수신 중: Content-Length 충족 여부 확인 */
            char* cl_str = strcasestr(raw, "Content-Length:");
            char* body_start = strstr(raw, "\r\n\r\n");
            if (cl_str && body_start) {
                int cl = atoi(cl_str + 15);
                int body_recv = total - (int)(body_start + 4 - raw);
                if (body_recv >= cl) break;
            }
        }

        if (total >= buf_size - 1) break;
    }

    /* recv 실패 — timeout 또는 연결 종료 */
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        if (!headers_done) {
            /* 헤더 미완성 timeout = Slowloris 공격 */
            return -408;
        }
        /* 바디 수신 중 timeout */
        return -408;
    }

    return total;
}

/* Connection 헤더에서 keep-alive 여부 파악
 * HTTP/1.1: 기본 keep-alive (Connection: close 있으면 close)
 * HTTP/1.0: 기본 close (Connection: keep-alive 있으면 keep-alive)
 */
static int check_keep_alive(HttpRequest* hr, const char* raw) {
    /* HTTP 버전 확인 */
    int is_11 = (strstr(raw, "HTTP/1.1") != NULL);
    int want_keepalive = is_11 ? 1 : 0;   /* 1.1: default on, 1.0: default off */

    for (int i = 0; i < hr->nheaders; i++) {
        if (strcasecmp(hr->headers[i][0], "Connection") == 0) {
            if (strcasecmp(hr->headers[i][1], "keep-alive") == 0)
                want_keepalive = 1;
            else if (strcasecmp(hr->headers[i][1], "close") == 0)
                want_keepalive = 0;
            break;
        }
    }
    return want_keepalive;
}

/* websocket.c에서 선언 */
extern int ws_is_upgrade_request(const char* raw);
extern int ws_handle_upgrade(int fd, const char* raw, int raw_len);

/* sse.c에서 선언 */
extern int sse_is_sse_request(const char* raw);
extern int sse_handle_request(int fd, const char* raw, int raw_len);

static void* handle_connection(void* arg) {
    ConnArg* ca = (ConnArg*)arg;
    int client_fd = ca->fd;
    free(ca);

    /* TCP_NODELAY: Nagle 비활성화
     * 헤더+바디를 2번 send() 할 때 Nagle+DelayedACK로 40ms 지연 방지
     * keep-alive 연결에서 특히 중요 */
    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    /* keep-alive 유휴 타임아웃 설정 */
    struct timeval tv = { .tv_sec = KEEPALIVE_IDLE_SEC, .tv_usec = 0 };
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char* raw = malloc(RECV_BUF);
    if (!raw) { close(client_fd); return NULL; }

    int req_count = 0;

    while (req_count < KEEPALIVE_MAX_REQUESTS) {

        /* ── 1. 요청 수신 ── */
        int total = recv_one_request(client_fd, raw, RECV_BUF);
        if (total == -408) {
            const char* r = "HTTP/1.1 408 Request Timeout\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            send(client_fd, r, strlen(r), 0);
            break;
        }
        if (total == -413) {
            const char* r = "HTTP/1.1 413 Payload Too Large\r\nContent-Type: application/json\r\nContent-Length: 34\r\nConnection: close\r\n\r\n{\"error\":\"payload exceeds 10MB\"}";
            send(client_fd, r, strlen(r), 0);
            break;
        }
        if (total == -414) {
            const char* r = "HTTP/1.1 414 URI Too Long\r\nContent-Type: application/json\r\nContent-Length: 28\r\nConnection: close\r\n\r\n{\"error\":\"URI exceeds 8192B\"}";
            send(client_fd, r, strlen(r), 0);
            break;
        }
        if (total == -431) {
            const char* r = "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Type: application/json\r\nContent-Length: 32\r\nConnection: close\r\n\r\n{\"error\":\"headers exceed 32KB\"}";
            send(client_fd, r, strlen(r), 0);
            break;
        }
        if (total <= 0) break;   /* 연결 종료 or 타임아웃 */

        /* ── 2a. SSE 감지 (WS보다 먼저 체크) ── */
        if (sse_is_sse_request(raw)) {
            if (!sse_handle_request(client_fd, raw, total))
                goto normal_http;   /* 등록된 라우트 없으면 일반 HTTP */
            break;  /* SSE는 keep-alive 루프 탈출 */
        }

        /* ── 2b. WebSocket 업그레이드 감지 ── */
        if (ws_is_upgrade_request(raw)) {
            ws_handle_upgrade(client_fd, raw, total);
            break;  /* WS는 keep-alive 루프 탈출 (핸들러가 fd 관리) */
        }

        normal_http:;

        /* ── 3. 아레나 시작 (요청 단위 메모리 관리) ── */
        fl_arena_begin();

        /* ── 4. 파싱 ── */
        HttpRequest hr;
        if (parse_http_request(raw, total, &hr) < 0) {
            const char* err = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            send(client_fd, err, strlen(err), 0);
            fl_arena_end();
            break;
        }

        /* ── 5. keep-alive 결정 ── */
        int keep_alive = check_keep_alive(&hr, raw);

        /* ── 6. 디버그 로그 ── */
        struct timespec t_start;
        clock_gettime(CLOCK_MONOTONIC, &t_start);
        fl_log_request(hr.method, hr.path, hr.body, hr.body_len,
                       hr.content_type[0] ? hr.content_type : NULL);

        /* ── 6. 라우트 매칭 ── */
        FLValue params = fl_nil();
        Route* matched = NULL;
        for (int i = 0; i < g_nroutes; i++) {
            FLValue p;
            if (match_route(&g_routes[i], hr.method, hr.path, &p)) {
                matched = &g_routes[i];
                params  = p;
                break;
            }
        }

        FLValue resp;
        if (!matched) {
            char notfound_body[256];
            snprintf(notfound_body, sizeof(notfound_body),
                "{\"error\":\"Not Found\",\"path\":\"%s\"}", hr.path);
            resp = make_response(404, "application/json", notfound_body);
        } else {
            /* ── 7. 핸들러 호출 (try/catch 보호) ── */
            FLValue req = make_req_map(&hr, params);
            resp = fl_nil();
            if (fl_try_top < FL_TRY_MAX) {
                FLTryFrame* frame = &fl_try_stack[fl_try_top++];
                if (setjmp(frame->buf) == 0) {
                    resp = (matched->fn) ? matched->fn(req) : fl_fn_call(matched->closure, 1, &req);
                    fl_try_top--;
                } else {
                    fl_try_top--;
                    char errbuf[1024];
                    const char* emsg = (frame->err.tag == FL_STRING)
                        ? strval(frame->err) : "Internal error";
                    /* 핸들러 에러 — stderr에 상세 출력 */
                    fprintf(stderr, "\n[FreeLang 500] %s %s\n  -> %s\n",
                            hr.method, hr.path, emsg);
                    snprintf(errbuf, sizeof(errbuf),
                        "{\"error\":\"%s\",\"path\":\"%s\",\"method\":\"%s\"}",
                        emsg, hr.path, hr.method);
                    resp = make_response(500, "application/json", errbuf);
                    keep_alive = 0;
                }
            } else {
                resp = (matched->fn) ? matched->fn(req) : fl_fn_call(matched->closure, 1, &req);
            }
        }

        /* ── 8. 응답 전송 ── */
        struct timespec t_end;
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        long elapsed_ms = (t_end.tv_sec - t_start.tv_sec) * 1000
                        + (t_end.tv_nsec - t_start.tv_nsec) / 1000000;
        int status_code = 200;
        if (resp.tag == FL_MAP) {
            FLValue sc = fl_map_get(resp, fl_str_val("__status"));
            if (sc.tag == FL_INT) status_code = (int)sc.i;
        }
        fl_log_response(status_code, hr.path, elapsed_ms);
        fl_arena_stats();

        send_response(client_fd, resp, keep_alive);

        /* ── 9. 아레나 종료 ── */
        fl_arena_end();

        req_count++;
        if (!keep_alive) break;

    }   /* while keep_alive */

    free(raw);
    close(client_fd);
    return NULL;
}

/* ═══════════════════════════════════════════════════════
   스레드 풀 + Graceful Shutdown
   ═══════════════════════════════════════════════════════

   환경변수:
     FL_WORKERS=N    워커 스레드 수 (기본: CPU코어 수, 최대 256)
     FL_QUEUE=N      작업 큐 크기 (기본: 1024)
     FL_CONN_TIMEOUT=N  연결 타임아웃 초 (기본: 30)
     FL_SHUTDOWN_TIMEOUT=N  shutdown 대기 초 (기본: 30)
*/

#define POOL_MAX_WORKERS 256
#define POOL_DEFAULT_WORKERS 16
#define POOL_DEFAULT_QUEUE   1024

/* ── 작업 큐 항목 ── */
typedef struct QueueItem {
    int fd;
    struct QueueItem* next;
} QueueItem;

/* ── 스레드 풀 상태 ── */
static struct {
    pthread_t*      threads;
    int             n_workers;
    int             conn_timeout;

    QueueItem*      head;
    QueueItem*      tail;
    int             queue_len;
    int             queue_cap;

    pthread_mutex_t lock;
    pthread_cond_t  cond_work;   /* 새 작업 */
    pthread_cond_t  cond_drain;  /* 큐 비어짐 */

    volatile int    shutdown;    /* 1 = graceful shutdown 시작 */
    volatile int    active;      /* 현재 처리 중인 연결 수 */
} g_pool;

static volatile int g_server_fd = -1;  /* SIGTERM에서 close용 */

/* ── 503 즉시 반환 ── */
static void send_503(int fd) {
    const char* r = "HTTP/1.1 503 Service Unavailable\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: 35\r\n"
                    "Retry-After: 1\r\n\r\n"
                    "{\"error\":\"server busy, try again\"}";
    send(fd, r, strlen(r), 0);
    close(fd);
}

/* ── 워커 스레드 루프 ── */
static void* worker_loop(void* arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&g_pool.lock);
        while (g_pool.head == NULL && !g_pool.shutdown)
            pthread_cond_wait(&g_pool.cond_work, &g_pool.lock);

        if (g_pool.head == NULL && g_pool.shutdown) {
            pthread_mutex_unlock(&g_pool.lock);
            break;
        }

        QueueItem* item = g_pool.head;
        g_pool.head = item->next;
        if (g_pool.head == NULL) g_pool.tail = NULL;
        g_pool.queue_len--;
        g_pool.active++;
        pthread_mutex_unlock(&g_pool.lock);

        /* 연결 처리 */
        ConnArg* ca = malloc(sizeof(ConnArg));
        ca->fd = item->fd;
        free(item);
        handle_connection(ca);   /* 내부에서 free(ca), close(fd) */

        pthread_mutex_lock(&g_pool.lock);
        g_pool.active--;
        if (g_pool.active == 0 && g_pool.queue_len == 0)
            pthread_cond_broadcast(&g_pool.cond_drain);
        pthread_mutex_unlock(&g_pool.lock);
    }
    return NULL;
}

/* ── SIGTERM/SIGINT 핸들러 ── */
static void handle_shutdown_signal(int sig) {
    (void)sig;
    fl_log(1, "http", "shutdown 신호 수신 — graceful 종료 시작");
    g_pool.shutdown = 1;
    /* accept 루프 깨우기 */
    if (g_server_fd >= 0) {
        shutdown(g_server_fd, SHUT_RDWR);
        close(g_server_fd);
        g_server_fd = -1;
    }
    /* 워커 깨우기 */
    pthread_cond_broadcast(&g_pool.cond_work);
}

/* ── 스레드 풀 초기화 ── */
static int pool_init(void) {
    /* 워커 수 결정 */
    int n = POOL_DEFAULT_WORKERS;
    const char* env_w = getenv("FL_WORKERS");
    if (env_w) n = atoi(env_w);
    if (n <= 0) n = 1;
    if (n > POOL_MAX_WORKERS) n = POOL_MAX_WORKERS;

    const char* env_q = getenv("FL_QUEUE");
    g_pool.queue_cap = env_q ? atoi(env_q) : POOL_DEFAULT_QUEUE;
    if (g_pool.queue_cap < 8) g_pool.queue_cap = 8;

    const char* env_t = getenv("FL_CONN_TIMEOUT");
    g_pool.conn_timeout = env_t ? atoi(env_t) : 30;

    g_pool.n_workers = n;
    g_pool.threads   = malloc(n * sizeof(pthread_t));
    g_pool.head = g_pool.tail = NULL;
    g_pool.queue_len = 0;
    g_pool.shutdown  = 0;
    g_pool.active    = 0;

    pthread_mutex_init(&g_pool.lock, NULL);
    pthread_cond_init(&g_pool.cond_work, NULL);
    pthread_cond_init(&g_pool.cond_drain, NULL);

    for (int i = 0; i < n; i++) {
        if (pthread_create(&g_pool.threads[i], NULL, worker_loop, NULL) != 0) {
            fprintf(stderr, "[http] 워커 스레드 %d 생성 실패\n", i);
            g_pool.n_workers = i;
            break;
        }
    }

    fprintf(stderr, "[http] 스레드 풀: %d 워커, 큐 %d, 타임아웃 %ds\n",
            g_pool.n_workers, g_pool.queue_cap, g_pool.conn_timeout);
    return 0;
}

/* ── 스레드 풀 종료 (graceful) ── */
static void pool_shutdown(int timeout_sec) {
    fprintf(stderr, "[http] graceful shutdown (최대 %ds 대기)...\n", timeout_sec);

    pthread_mutex_lock(&g_pool.lock);
    g_pool.shutdown = 1;
    pthread_cond_broadcast(&g_pool.cond_work);

    /* 진행 중 요청 완료 대기 */
    if (g_pool.active > 0 || g_pool.queue_len > 0) {
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += timeout_sec;
        pthread_cond_timedwait(&g_pool.cond_drain, &g_pool.lock, &deadline);
    }
    pthread_mutex_unlock(&g_pool.lock);

    /* 워커 스레드 합류 */
    pthread_cond_broadcast(&g_pool.cond_work);
    for (int i = 0; i < g_pool.n_workers; i++)
        pthread_join(g_pool.threads[i], NULL);

    fprintf(stderr, "[http] 종료 완료 (처리 중 연결: %d)\n", g_pool.active);
}

/* ───────────────────────────────────────────
   server_start — 메인 루프 (스레드 풀)
─────────────────────────────────────────── */

FLValue server_start(FLValue port_val) {
    /* 수리(2026-06-14): 이전엔 FL_INT가 아니면 조용히 8080으로 떨어져
     * (server_start (str-to-num env))·(server_start (floor x)) 가 무력화됐다(트랩 #9,
     * str-to-num/floor 은 FL_FLOAT 반환). 이제 FLOAT·STRING 도 명시 수용. */
    int port = 8080;
    if      (port_val.tag == FL_INT)    port = (int)port_val.i;
    else if (port_val.tag == FL_FLOAT)  port = (int)port_val.f;
    else if (port_val.tag == FL_STRING) { int p = atoi(strval(port_val)); if (p > 0) port = p; }

    /* 시그널 설정 */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, handle_shutdown_signal);
    signal(SIGINT,  handle_shutdown_signal);

    /* 스레드 풀 초기화 */
    pool_init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "[http] socket() 실패: %s\n", strerror(errno));
        return fl_nil();
    }
    g_server_fd = server_fd;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[http] bind(%d) 실패: %s\n", port, strerror(errno));
        close(server_fd);
        return fl_nil();
    }

    listen(server_fd, 512);   /* backlog 증가 */
    fprintf(stderr, "[http] 서버 시작: http://0.0.0.0:%d\n", port);
    fl_debug_banner("fl-app", port);

    /* ── accept 루프 ── */
    while (!g_pool.shutdown) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            break;   /* shutdown이 server_fd를 닫으면 여기서 탈출 */
        }

        /* 송신 타임아웃 설정 (SO_RCVTIMEO는 recv_one_request에서 설정) */
        struct timeval tv = { g_pool.conn_timeout, 0 };
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        /* 큐 엔큐 + 연결 수 한도 통합 체크 */
        pthread_mutex_lock(&g_pool.lock);
        int total_conns = g_pool.queue_len + g_pool.active;
        if (total_conns >= MAX_CONNECTIONS || g_pool.queue_len >= g_pool.queue_cap) {
            pthread_mutex_unlock(&g_pool.lock);
            fl_log_error("http", "연결 한도(%d) 초과(큐=%d,활성=%d) → 503",
                         MAX_CONNECTIONS, g_pool.queue_len, g_pool.active);
            send_503(client_fd);
            continue;
        }
        QueueItem* item = malloc(sizeof(QueueItem));
        item->fd   = client_fd;
        item->next = NULL;
        if (g_pool.tail) g_pool.tail->next = item;
        else             g_pool.head = item;
        g_pool.tail = item;
        g_pool.queue_len++;
        pthread_cond_signal(&g_pool.cond_work);
        pthread_mutex_unlock(&g_pool.lock);
    }

    /* ── Graceful shutdown ── */
    if (g_server_fd >= 0) { close(g_server_fd); g_server_fd = -1; }

    const char* env_st = getenv("FL_SHUTDOWN_TIMEOUT");
    int shutdown_timeout = env_st ? atoi(env_st) : 30;
    pool_shutdown(shutdown_timeout);

    return fl_nil();
}

/* ── fl_http_route / fl_http_start — kebab-case 앱 호환 alias ── */
/* (server-get "/path" "handler")  → fl_http_route("GET", "/path", "handler") */
/* (server-post ...)               → fl_http_route("POST", ...) */
/* (server-start PORT)             → fl_http_start(PORT) */

FLValue fl_http_route(FLValue method_v, FLValue path_v, FLValue handler_v) {
    if (method_v.tag != FL_STRING) return fl_nil();
    const char* method = ((FLString*)method_v.obj)->data;
    if      (strcmp(method, "GET")    == 0) return server_get(path_v, handler_v);
    else if (strcmp(method, "POST")   == 0) return server_post(path_v, handler_v);
    else if (strcmp(method, "PUT")    == 0) return server_put(path_v, handler_v);
    else if (strcmp(method, "PATCH")  == 0) return server_patch(path_v, handler_v);
    else if (strcmp(method, "DELETE") == 0) return server_delete(path_v, handler_v);
    return fl_nil();
}

FLValue fl_http_start(FLValue port) { return server_start(port); }

/* server-json (kebab) → server_json (이미 있음, 하지만 alias 명시) */
/* server-text, server-html, server-status, server-redirect도 동일 */
/* cgc-bin이 이미 _ 로 변환하므로 추가 alias 불필요 */

/* ── fl_resp_* alias — kebab-case response 함수 ── */
FLValue fl_resp_html(FLValue html)              { return server_html(html); }
FLValue fl_resp_json(FLValue json)              { return server_json(json); }
FLValue fl_resp_text(FLValue text)              { return server_text(text); }
FLValue fl_resp_status(FLValue code, FLValue b) { return server_status(code, b); }
FLValue fl_resp_redirect(FLValue url)           { return server_redirect(url); }

/* ───────────────────────────────────────────
   HTTP 클라이언트 (outbound)
   fl_http_get(url) → 응답 본문 FL String 반환 (실패 시 nil)
   HTTP only (TLS 미지원). 단순 GET, Connection: close.

   실패 시 nil을 반환하되 *왜* 실패했는지 stderr로 항상 진단한다
   (FL_DEBUG 무관). 침묵하는 nil은 디버깅 지옥이므로.
   끄려면: 환경변수 FL_HTTP_QUIET=1
─────────────────────────────────────────── */
static void httpget_diag(const char* fmt, ...) {
    if (getenv("FL_HTTP_QUIET")) return;
    va_list ap; va_start(ap, fmt);
    fprintf(stderr, "[http-get] ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

FLValue fl_http_get(FLValue url_v) {
    if (url_v.tag != FL_STRING || !url_v.obj) {
        httpget_diag("인자가 문자열이 아님 (URL 필요)");
        return fl_nil();
    }
    const char* url = ((FLString*)url_v.obj)->data;

    /* URL 파싱: http(s)://host[:port][/path] */
    int use_ssl = 0;
    const char* p = url;
    if (strncmp(p, "https://", 8) == 0)      { p += 8; use_ssl = 1; }
    else if (strncmp(p, "http://", 7) == 0)  { p += 7; }
    else { /* scheme 없으면 HTTP */ }

    char host[256]; int port = use_ssl ? 443 : 80; char path[2048] = "/";
    int hi = 0;
    while (*p && *p != ':' && *p != '/' && hi < 255) host[hi++] = *p++;
    host[hi] = 0;
    if (host[0] == 0) {
        httpget_diag("URL에서 host를 파싱하지 못함: %s", url);
        return fl_nil();
    }
    if (*p == ':') {
        p++; port = 0;
        while (*p >= '0' && *p <= '9') port = port * 10 + (*p++ - '0');
    }
    if (*p == '/') { strncpy(path, p, sizeof(path) - 1); path[sizeof(path) - 1] = 0; }

    /* resolve + connect */
    char port_str[16]; snprintf(port_str, sizeof(port_str), "%d", port);
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int gai = getaddrinfo(host, port_str, &hints, &res);
    if (gai != 0 || !res) {
        httpget_diag("DNS 해석 실패 %s:%s — %s", host, port_str, gai_strerror(gai));
        return fl_nil();
    }

    int fd = -1, last_errno = 0, tried = 0;
    for (struct addrinfo* ai = res; ai; ai = ai->ai_next) {
        tried++;
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) { last_errno = errno; continue; }
        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) break;
        last_errno = errno;
        close(fd); fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) {
        httpget_diag("연결 실패 %s:%d (후보 %d개 시도) — %s",
               host, port, tried, strerror(last_errno));
        return fl_nil();
    }

    /* ── SSL 핸드셰이크 (HTTPS) ───────────────────────────────────── */
    SSL_CTX* ctx = NULL;
    SSL*     ssl = NULL;
    if (use_ssl) {
        SSL_library_init();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) { close(fd); return fl_nil(); }
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);  /* 인증서 검증 생략 */
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, fd);
        SSL_set_tlsext_host_name(ssl, host);
        if (SSL_connect(ssl) <= 0) {
            httpget_diag("TLS 핸드셰이크 실패: %s", host);
            SSL_free(ssl); SSL_CTX_free(ctx); close(fd);
            return fl_nil();
        }
    }

    /* send request */
    char req[4096];
    int rn = snprintf(req, sizeof(req),
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n"
        "User-Agent: fx-http/1.0\r\nAccept: */*\r\n\r\n", path, host);
    ssize_t sent = 0;
    while (sent < rn) {
        ssize_t s = use_ssl
            ? SSL_write(ssl, req + sent, rn - sent)
            : send(fd, req + sent, rn - sent, MSG_NOSIGNAL);
        if (s <= 0) {
            httpget_diag("요청 전송 실패 %s", host);
            if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
            close(fd); return fl_nil();
        }
        sent += s;
    }

    /* recv 전체 응답 */
    size_t cap = 8192, len = 0;
    char* buf = malloc(cap);
    if (!buf) {
        if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
        close(fd); return fl_nil();
    }
    ssize_t n;
    while ((n = use_ssl
               ? SSL_read(ssl, buf + len, (int)(cap - len - 1))
               : recv(fd, buf + len, cap - len - 1, 0)) > 0) {
        len += (size_t)n;
        if (len + 1 >= cap) {
            cap *= 2;
            char* nb = realloc(buf, cap);
            if (!nb) { free(buf);
                if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
                close(fd); return fl_nil(); }
            buf = nb;
        }
    }
    buf[len] = 0;
    if (use_ssl) { SSL_free(ssl); SSL_CTX_free(ctx); }
    close(fd);

    /* 본문만 추출 */
    char* sep = strstr(buf, "\r\n\r\n");
    const char* body = sep ? sep + 4 : buf;

    /* chunked transfer encoding 디코딩 */
    int is_chunked = 0;
    char* hdrs_end = sep ? sep : buf;
    char* hdr_search = buf;
    while (hdr_search < hdrs_end) {
        if (strncasecmp(hdr_search, "Transfer-Encoding: chunked", 26) == 0) {
            is_chunked = 1; break;
        }
        char* nl = memchr(hdr_search, '\n', (size_t)(hdrs_end - hdr_search));
        if (!nl) break;
        hdr_search = nl + 1;
    }
    if (is_chunked) {
        size_t dcap = strlen(body) + 1;
        char* decoded = malloc(dcap);
        size_t dlen = 0;
        const char* p = body;
        while (*p) {
            /* 청크 크기 (hex) */
            char* end;
            long chunk_size = strtol(p, &end, 16);
            if (end == p || chunk_size < 0) break;
            if (chunk_size == 0) break;
            p = end;
            if (*p == '\r') p++;
            if (*p == '\n') p++;
            /* 데이터 복사 */
            if (dlen + (size_t)chunk_size + 1 >= dcap) {
                dcap = dlen + (size_t)chunk_size + 64;
                decoded = realloc(decoded, dcap);
            }
            memcpy(decoded + dlen, p, (size_t)chunk_size);
            dlen += (size_t)chunk_size;
            p += chunk_size;
            if (*p == '\r') p++;
            if (*p == '\n') p++;
        }
        decoded[dlen] = '\0';
        FLValue out = fl_str_val(decoded);
        free(decoded);
        free(buf);
        return out;
    }

    FLValue out = fl_str_val(body);
    free(buf);
    return out;
}

/* ───────────────────────────────────────────
   HTTPS 서버 (server_start_tls)
   server_start_tls(port, "/path/to/cert.pem", "/path/to/key.pem")
   → HTTP와 동일한 라우트 + SSL 래핑
─────────────────────────────────────────── */

/* SSL 연결 래퍼 — recv/send 대신 SSL_read/SSL_write */
typedef struct {
    int   fd;
    SSL*  ssl;
} TLSConn;

static __thread TLSConn* g_tls_conn = NULL;

/* handle_connection에서 사용하는 recv/send를 TLS로 오버라이드하기 위한
 * 별도 핸들러 — SSL_read/SSL_write 직접 사용 */
static void* handle_tls_connection(void* arg) {
    TLSConn* tc = (TLSConn*)arg;
    int client_fd  = tc->fd;
    SSL* ssl       = tc->ssl;
    free(tc);

    /* SSL 핸드셰이크 */
    if (SSL_accept(ssl) <= 0) {
        SSL_free(ssl);
        close(client_fd);
        return NULL;
    }

    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    char* raw = malloc(RECV_BUF);
    if (!raw) { SSL_free(ssl); close(client_fd); return NULL; }

    /* 단순 1-요청 처리 (TLS에서 keep-alive는 복잡도 증가) */
    ssize_t total = SSL_read(ssl, raw, RECV_BUF - 1);
    if (total <= 0) { free(raw); SSL_free(ssl); close(client_fd); return NULL; }
    raw[total] = '\0';

    fl_arena_begin();

    HttpRequest hr;
    if (parse_http_request(raw, (int)total, &hr) < 0) {
        const char* err = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        SSL_write(ssl, err, (int)strlen(err));
        fl_arena_end();
        free(raw); SSL_free(ssl); close(client_fd);
        return NULL;
    }

    FLValue params = fl_nil();
    Route* matched = NULL;
    for (int i = 0; i < g_nroutes; i++) {
        FLValue p;
        if (match_route(&g_routes[i], hr.method, hr.path, &p)) {
            matched = &g_routes[i]; params = p; break;
        }
    }

    FLValue resp;
    if (!matched) {
        char nb[256];
        snprintf(nb, sizeof(nb), "{\"error\":\"Not Found\",\"path\":\"%s\"}", hr.path);
        resp = make_response(404, "application/json", nb);
    } else {
        FLValue req = make_req_map(&hr, params);
        resp = fl_nil();
        if (fl_try_top < FL_TRY_MAX) {
            FLTryFrame* frame = &fl_try_stack[fl_try_top++];
            if (setjmp(frame->buf) == 0) {
                resp = (matched->fn) ? matched->fn(req) : fl_fn_call(matched->closure, 1, &req); fl_try_top--;
            } else {
                fl_try_top--;
                const char* emsg = frame->err.tag == FL_STRING ? strval(frame->err) : "error";
                char eb[512]; snprintf(eb, sizeof(eb), "{\"error\":\"%s\"}", emsg);
                resp = make_response(500, "application/json", eb);
            }
        } else {
            resp = (matched->fn) ? matched->fn(req) : fl_fn_call(matched->closure, 1, &req);
        }
    }

    /* 응답 직렬화 + SSL_write */
    FLValue status_v  = fl_map_get(resp, fl_str_val(K_STATUS));
    FLValue body_v    = fl_map_get(resp, fl_str_val(K_BODY));
    FLValue headers_v = fl_map_get(resp, fl_str_val(K_HEADERS));
    int status_code   = (status_v.tag == FL_INT) ? (int)status_v.i : 200;
    const char* body  = (body_v.tag == FL_STRING) ? strval(body_v) : "";
    size_t body_len   = strlen(body);

    const char* ctype = "text/plain";
    if (headers_v.tag == FL_MAP) {
        FLValue ct = fl_map_get(headers_v, fl_str_val("Content-Type"));
        if (ct.tag == FL_STRING) ctype = strval(ct);
    }

    char hdr_buf[512];
    int hdr_len = snprintf(hdr_buf, sizeof(hdr_buf),
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, ctype, body_len);
    SSL_write(ssl, hdr_buf, hdr_len);
    if (body_len > 0) SSL_write(ssl, body, (int)body_len);

    fl_arena_end();
    free(raw);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    return NULL;
}

/* server_start_tls(port, cert_path, key_path) */
FLValue server_start_tls(FLValue port_val, FLValue cert_val, FLValue key_val) {
    int port = 8443;
    if      (port_val.tag == FL_INT)   port = (int)port_val.i;
    else if (port_val.tag == FL_FLOAT) port = (int)port_val.f;

    const char* cert_path = strval(cert_val);
    const char* key_path  = strval(key_val);

    /* SSL 초기화 */
    SSL_library_init();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        fprintf(stderr, "[tls] SSL_CTX_new 실패\n");
        return fl_nil();
    }
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[tls] 인증서 로드 실패: %s\n", cert_path);
        SSL_CTX_free(ctx); return fl_nil();
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[tls] 개인키 로드 실패: %s\n", key_path);
        SSL_CTX_free(ctx); return fl_nil();
    }

    signal(SIGPIPE, SIG_IGN);
    pool_init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[tls] bind(%d) 실패: %s\n", port, strerror(errno));
        close(server_fd); SSL_CTX_free(ctx); return fl_nil();
    }
    listen(server_fd, 128);
    fprintf(stderr, "[tls] HTTPS 서버 시작: https://0.0.0.0:%d\n", port);

    /* accept 루프 */
    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(server_fd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) { if (errno == EINTR) continue; break; }

        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cfd);

        TLSConn* tc = malloc(sizeof(TLSConn));
        tc->fd = cfd; tc->ssl = ssl;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_tls_connection, tc);
        pthread_detach(tid);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return fl_nil();
}
