#define _GNU_SOURCE
#include "runtime.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

typedef enum { FL_ASYNC_READ, FL_ASYNC_WRITE, FL_ASYNC_SLEEP, FL_ASYNC_HTTP_GET, FL_ASYNC_HTTP_POST } FLAsyncOp;
typedef enum { FL_ASYNC_PENDING, FL_ASYNC_SUCCESS, FL_ASYNC_ERROR, FL_ASYNC_TIMEOUT, FL_ASYNC_CANCELLED } FLAsyncTerminal;
typedef struct {
    FLAsyncOp op;
    FLAsyncTerminal terminal;
    FLValue path, content, headers, result, callback;
    pthread_t thread;
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int callback_registered, callback_dispatched, joined;
} FLAsyncTask;

static FLValue async_packet(const char *status, FLValue value, const char *type, const char *code) {
    FLValue m=fl_map_new();
    m=fl_map_set(m,fl_str_val("status"),fl_str_val(status));
    m=fl_map_set(m,fl_str_val("value"),value);
    m=fl_map_set(m,fl_str_val("errorType"),type?fl_str_val(type):fl_nil());
    return fl_map_set(m,fl_str_val("code"),code?fl_str_val(code):fl_nil());
}
static int async_result_ok(FLAsyncTask *t, FLValue r) {
    if (t->op == FL_ASYNC_WRITE) return 1;
    if (r.tag == FL_NIL) return 0;
    if ((t->op == FL_ASYNC_HTTP_GET || t->op == FL_ASYNC_HTTP_POST) && r.tag == FL_MAP && fl_map_get(r,fl_str_val("error")).tag != FL_NIL) return 0;
    return 1;
}
static FLValue async_terminal_packet_locked(FLAsyncTask *t) {
    switch (t->terminal) {
    case FL_ASYNC_SUCCESS: return async_packet("ok", t->op == FL_ASYNC_WRITE ? fl_bool(true) : t->result, NULL, NULL);
    case FL_ASYNC_TIMEOUT: return async_packet("error", fl_nil(), "AsyncTimeoutError", "E_ASYNC_TIMEOUT");
    case FL_ASYNC_CANCELLED: return async_packet("error", fl_nil(), "AsyncCancelledError", "E_ASYNC_CANCELLED");
    case FL_ASYNC_ERROR: return async_packet("error", fl_nil(), "AsyncIOError", "E_ASYNC_IO");
    case FL_ASYNC_PENDING: break;
    }
    return async_packet("error", fl_nil(), "AsyncIOError", "E_ASYNC_IO");
}
static void async_dispatch_callback(FLAsyncTask *t) {
    FLValue callback=fl_nil(), packet=fl_nil();
    pthread_mutex_lock(&t->mu);
    if (t->terminal != FL_ASYNC_PENDING && t->callback_registered && !t->callback_dispatched) {
        t->callback_dispatched=1; callback=t->callback; t->callback=fl_nil();
        packet=async_terminal_packet_locked(t);
    }
    pthread_mutex_unlock(&t->mu);
    if (callback.tag == FL_FN) { (void)fl_fn_call(callback,1,&packet); fl_heap_release(callback); }
}
static int async_claim_terminal(FLAsyncTask *t, FLAsyncTerminal terminal, FLValue result) {
    int claimed=0;
    pthread_mutex_lock(&t->mu);
    if (t->terminal == FL_ASYNC_PENDING) {
        t->terminal=terminal;
        if (terminal == FL_ASYNC_SUCCESS || terminal == FL_ASYNC_ERROR) t->result=result;
        pthread_cond_broadcast(&t->cv); claimed=1;
    }
    pthread_mutex_unlock(&t->mu);
    if (claimed) async_dispatch_callback(t);
    return claimed;
}
static void *async_worker(void *arg) {
    FLAsyncTask *t=arg; FLValue r;
    if(t->op==FL_ASYNC_READ) r=fl_file_read(t->path);
    else if(t->op==FL_ASYNC_WRITE) r=fl_file_write(t->path,t->content);
    else if(t->op==FL_ASYNC_HTTP_GET) r=http_get_h(t->path,t->headers);
    else if(t->op==FL_ASYNC_HTTP_POST) r=http_post_h(t->path,t->content,t->headers);
    else { struct timespec ts={.tv_sec=t->content.i/1000,.tv_nsec=(t->content.i%1000)*1000000L}; nanosleep(&ts,NULL); r=fl_bool(true); }
    (void)async_claim_terminal(t,async_result_ok(t,r)?FL_ASYNC_SUCCESS:FL_ASYNC_ERROR,r);
    return NULL;
}
static FLValue async_start(FLAsyncOp op, FLValue path, FLValue content, FLValue headers) {
    if(path.tag!=FL_STRING||fl_string_has_embedded_nul(path))return fl_nil();
    FLAsyncTask*t=calloc(1,sizeof(*t)); if(!t)return fl_nil(); t->op=op; t->terminal=FL_ASYNC_PENDING;
    t->path=fl_heap_copy(path); t->content=content.tag==FL_NIL?fl_nil():fl_heap_copy(content); t->headers=headers.tag==FL_NIL?fl_nil():fl_heap_copy(headers);
    pthread_mutex_init(&t->mu,NULL); pthread_cond_init(&t->cv,NULL);
    if(pthread_create(&t->thread,NULL,async_worker,t)!=0){
        pthread_cond_destroy(&t->cv); pthread_mutex_destroy(&t->mu); fl_heap_release(t->path);
        if(t->content.tag!=FL_NIL)fl_heap_release(t->content); if(t->headers.tag!=FL_NIL)fl_heap_release(t->headers); free(t); return fl_nil();
    }
    FLValue h=fl_map_new(); return fl_map_set(h,fl_str_val("__async_task__"),fl_int((int64_t)(uintptr_t)t));
}
static FLAsyncTask *async_task(FLValue h){if(h.tag!=FL_MAP)return NULL;FLValue p=fl_map_get(h,fl_str_val("__async_task__"));return p.tag==FL_INT?(FLAsyncTask*)(uintptr_t)p.i:NULL;}
FLValue fl_async_file_read(FLValue p){return async_start(FL_ASYNC_READ,p,fl_nil(),fl_nil());}
FLValue fl_async_file_write(FLValue p,FLValue c){return async_start(FL_ASYNC_WRITE,p,c,fl_nil());}
FLValue fl_async_sleep(FLValue ms){return async_start(FL_ASYNC_SLEEP,fl_str_val(""),ms,fl_nil());}
FLValue fl_async_http_get(FLValue url,FLValue headers){return async_start(FL_ASYNC_HTTP_GET,url,fl_nil(),headers);}
FLValue fl_async_http_post(FLValue url,FLValue body,FLValue headers){return async_start(FL_ASYNC_HTTP_POST,url,body,headers);}
FLValue fl_async_await(FLValue h,FLValue mv){
    FLAsyncTask*t=async_task(h); if(!t)return async_packet("error",fl_nil(),"AsyncIOError","E_ASYNC_IO");
    int64_t ms=mv.tag==FL_INT?mv.i:30000; struct timespec d; clock_gettime(CLOCK_REALTIME,&d); d.tv_sec+=ms/1000; d.tv_nsec+=(ms%1000)*1000000L;
    if(d.tv_nsec>=1000000000L){d.tv_sec++;d.tv_nsec-=1000000000L;}
    pthread_mutex_lock(&t->mu);
    while(t->terminal==FL_ASYNC_PENDING) if(pthread_cond_timedwait(&t->cv,&t->mu,&d)==ETIMEDOUT){if(t->terminal==FL_ASYNC_PENDING)t->terminal=FL_ASYNC_TIMEOUT; pthread_cond_broadcast(&t->cv); break;}
    FLValue packet=async_terminal_packet_locked(t); pthread_mutex_unlock(&t->mu); async_dispatch_callback(t); return packet;
}
FLValue fl_async_cancel(FLValue h){FLAsyncTask*t=async_task(h);if(!t)return fl_bool(false);return fl_bool(async_claim_terminal(t,FL_ASYNC_CANCELLED,fl_nil()));}
FLValue fl_async_set_callback(FLValue h,FLValue callback){
    FLAsyncTask*t=async_task(h); if(!t||callback.tag!=FL_FN)return fl_bool(false);
    pthread_mutex_lock(&t->mu); if(t->callback_registered){pthread_mutex_unlock(&t->mu);return fl_bool(false);}
    t->callback=fl_heap_copy(callback); t->callback_registered=1; pthread_mutex_unlock(&t->mu); async_dispatch_callback(t); return fl_bool(true);
}
FLValue fl_async_release(FLValue h){FLAsyncTask*t=async_task(h);if(!t||t->joined)return fl_bool(false);pthread_join(t->thread,NULL);t->joined=1;pthread_cond_destroy(&t->cv);pthread_mutex_destroy(&t->mu);fl_heap_release(t->path);if(t->content.tag!=FL_NIL)fl_heap_release(t->content);if(t->headers.tag!=FL_NIL)fl_heap_release(t->headers);if(t->callback.tag!=FL_NIL)fl_heap_release(t->callback);free(t);return fl_bool(true);}
