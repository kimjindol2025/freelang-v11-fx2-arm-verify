/* ─────────────────────────────────────────────────────────────────
   fx HTTP 클라이언트 — libcurl 기반
   ─────────────────────────────────────────────────────────────────
   함수 목록:
     http_get  url                    → {status body headers}
     http_post url body               → {status body headers}
     http_put  url body               → {status body headers}
     http_del  url                    → {status body headers}
     http_req  method url body headers → {status body headers}  (풀 제어)

   반환 맵:
     {"status" 200 "body" "..." "headers" {"content-type" "..."}}
     실패 시 {"status" 0 "body" "" "error" "설명"}
 ───────────────────────────────────────────────────────────────── */
#include "runtime.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/* ── curl 응답 버퍼 ── */
typedef struct {
    char*  data;
    size_t len;
    size_t cap;
} CurlBuf;

static size_t write_cb(void* ptr, size_t size, size_t nmemb, void* ud) {
    CurlBuf* b = (CurlBuf*)ud;
    size_t n = size * nmemb;
    if (b->len + n + 1 >= b->cap) {
        b->cap = (b->len + n + 1) * 2 + 4096;
        b->data = realloc(b->data, b->cap);
        if (!b->data) return 0;
    }
    memcpy(b->data + b->len, ptr, n);
    b->len += n;
    b->data[b->len] = '\0';
    return n;
}

/* ── curl 응답 헤더 버퍼 ── */
static size_t header_cb(void* ptr, size_t size, size_t nmemb, void* ud) {
    CurlBuf* b = (CurlBuf*)ud;
    size_t n = size * nmemb;
    if (b->len + n + 1 >= b->cap) {
        b->cap = (b->len + n + 1) * 2 + 4096;
        b->data = realloc(b->data, b->cap);
        if (!b->data) return 0;
    }
    memcpy(b->data + b->len, ptr, n);
    b->len += n;
    b->data[b->len] = '\0';
    return n;
}

/* ── 헤더 맵 파싱: "Key: Value\r\n..." → FLMap ── */
static FLValue parse_headers(const char* raw) {
    FLValue m = fl_map_new();
    const char* p = raw;
    /* 첫 줄 (HTTP/1.1 200 OK) 건너뜀 */
    const char* nl = strchr(p, '\n');
    if (nl) p = nl + 1;

    while (*p && *p != '\r' && *p != '\n') {
        const char* colon = strchr(p, ':');
        const char* lf    = strchr(p, '\n');
        if (!colon || (lf && colon > lf)) { if (lf) p = lf + 1; else break; continue; }

        /* 키 소문자화 */
        size_t klen = (size_t)(colon - p);
        char* key = malloc(klen + 1);
        for (size_t i = 0; i < klen; i++)
            key[i] = (p[i] >= 'A' && p[i] <= 'Z') ? p[i] + 32 : p[i];
        key[klen] = '\0';

        /* 값 앞 공백 제거 + 뒤 \r\n 제거 */
        const char* vs = colon + 1;
        while (*vs == ' ') vs++;
        size_t vlen = lf ? (size_t)(lf - vs) : strlen(vs);
        if (vlen > 0 && vs[vlen - 1] == '\r') vlen--;
        char* val = malloc(vlen + 1);
        memcpy(val, vs, vlen);
        val[vlen] = '\0';

        m = fl_map_set(m, fl_str_val(key), fl_str_val(val));
        free(key); free(val);
        if (lf) p = lf + 1; else break;
    }
    return m;
}

/* ── FLMap → curl slist 변환 (요청 헤더) ── */
static struct curl_slist* map_to_slist(FLValue headers) {
    if (headers.tag != FL_MAP || !headers.obj) return NULL;
    struct curl_slist* list = NULL;
    FLMap* m = (FLMap*)headers.obj;
    for (int i = 0; i < m->len; i++) {
        FLMapEntry* e = &m->entries[i];
        if (e->key.tag != FL_STRING || e->val.tag != FL_STRING) continue;
        const char* k = ((FLString*)e->key.obj)->data;
        const char* v = ((FLString*)e->val.obj)->data;
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s: %s", k, v);
        list = curl_slist_append(list, buf);
    }
    return list;
}

/* ── 코어 실행 ── */
static FLValue fx_http_exec(const char* method, const char* url,
                             const char* body, FLValue req_headers) {
    CURL* curl = curl_easy_init();
    if (!curl) return fl_map_set(fl_map_set(fl_map_new(),
        fl_str_val("status"), fl_int(0)),
        fl_str_val("error"), fl_str_val("curl_easy_init 실패"));

    CurlBuf resp = {malloc(4096), 0, 4096};
    CurlBuf hdrs = {malloc(4096), 0, 4096};
    resp.data[0] = '\0'; hdrs.data[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hdrs);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  /* 자가서명 허용 */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "fx-http-client/1.0");

    /* 메서드별 설정 */
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        } else {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
        }
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (strcmp(method, "PATCH") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (body) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
        }
    }
    /* GET은 기본값 */

    /* 요청 헤더 */
    struct curl_slist* slist = map_to_slist(req_headers);
    if (slist) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    CURLcode rc = curl_easy_perform(curl);

    long status = 0;
    if (rc == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    if (slist) curl_slist_free_all(slist);
    curl_easy_cleanup(curl);

    /* 결과 맵 구성 */
    FLValue result = fl_map_new();
    if (rc != CURLE_OK) {
        result = fl_map_set(result, fl_str_val("status"), fl_int(0));
        result = fl_map_set(result, fl_str_val("body"),   fl_str_val(""));
        result = fl_map_set(result, fl_str_val("error"),  fl_str_val(curl_easy_strerror(rc)));
    } else {
        result = fl_map_set(result, fl_str_val("status"),  fl_int((int64_t)status));
        result = fl_map_set(result, fl_str_val("body"),    fl_str_val(resp.data));
        result = fl_map_set(result, fl_str_val("headers"), parse_headers(hdrs.data));
    }

    free(resp.data);
    free(hdrs.data);
    return result;
}

/* ─────────────────────────────────────────────────────────────────
   공개 FL 함수
 ───────────────────────────────────────────────────────────────── */

/* http_get url */
FLValue http_get(FLValue url_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("GET", ((FLString*)url_v.obj)->data, NULL, fl_nil());
}

/* http_get_h url headers-map */
FLValue http_get_h(FLValue url_v, FLValue hdrs_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("GET", ((FLString*)url_v.obj)->data, NULL, hdrs_v);
}

/* http_post url body */
FLValue http_post(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("POST", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_post_h url body headers-map */
FLValue http_post_h(FLValue url_v, FLValue body_v, FLValue hdrs_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("POST", ((FLString*)url_v.obj)->data, body, hdrs_v);
}

/* http_put url body */
FLValue http_put(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("PUT", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_del url */
FLValue http_del(FLValue url_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    return fx_http_exec("DELETE", ((FLString*)url_v.obj)->data, NULL, fl_nil());
}

/* http_patch url body */
FLValue http_patch(FLValue url_v, FLValue body_v) {
    if (url_v.tag != FL_STRING) return fl_map_set(fl_map_new(),
        fl_str_val("error"), fl_str_val("url must be string"));
    const char* body = (body_v.tag == FL_STRING && body_v.obj)
        ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec("PATCH", ((FLString*)url_v.obj)->data, body, fl_nil());
}

/* http_req method url body headers */
FLValue http_req(FLValue method_v, FLValue url_v, FLValue body_v, FLValue hdrs_v) {
    if (method_v.tag != FL_STRING || url_v.tag != FL_STRING)
        return fl_map_set(fl_map_new(), fl_str_val("error"),
                          fl_str_val("method and url must be strings"));
    const char* method = ((FLString*)method_v.obj)->data;
    const char* url    = ((FLString*)url_v.obj)->data;
    const char* body   = (body_v.tag == FL_STRING && body_v.obj)
                         ? ((FLString*)body_v.obj)->data : "";
    return fx_http_exec(method, url, *body ? body : NULL, hdrs_v);
}

/* http_body res — 응답 맵에서 body 추출 (편의 함수) */
FLValue http_body(FLValue res_v) {
    if (res_v.tag != FL_MAP) return fl_str_val("");
    return fl_map_get(res_v, fl_str_val("body"));
}

/* http_status res — 응답 맵에서 status 추출 */
FLValue http_status(FLValue res_v) {
    if (res_v.tag != FL_MAP) return fl_int(0);
    return fl_map_get(res_v, fl_str_val("status"));
}

/* http_ok? res — 2xx 여부 */
FLValue http_ok_p(FLValue res_v) {
    FLValue s = http_status(res_v);
    double code = (s.tag == FL_INT) ? (double)s.i
                : (s.tag == FL_FLOAT) ? s.f : 0.0;
    return fl_bool(code >= 200 && code < 300);
}
