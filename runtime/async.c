#define _GNU_SOURCE
#include "runtime.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum { FL_ASYNC_READ, FL_ASYNC_WRITE, FL_ASYNC_SLEEP, FL_ASYNC_HTTP_GET, FL_ASYNC_HTTP_POST } FLAsyncOp;
typedef enum { FL_ASYNC_PENDING, FL_ASYNC_SUCCESS, FL_ASYNC_ERROR, FL_ASYNC_TIMEOUT, FL_ASYNC_CANCELLED } FLAsyncTerminal;
typedef struct FLAsyncTask {
    FLAsyncOp op;
    FLAsyncTerminal terminal;
    FLValue path, content, headers, result, callback;
    pthread_t thread;
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int callback_registered, callback_dispatched, callback_completed, callback_failed, joined;
    char callback_error_type[64];
    char callback_error_message[256];
    uint64_t id;
    struct FLAsyncTask *next;
} FLAsyncTask;

static pthread_mutex_t async_handle_mu = PTHREAD_MUTEX_INITIALIZER;
static FLAsyncTask *async_registry = NULL;
static uint64_t async_next_id = 1;

static void async_destroy_task(FLAsyncTask *t) {
    pthread_cond_destroy(&t->cv);
    pthread_mutex_destroy(&t->mu);
    fl_heap_release(t->path);
    if (t->content.tag != FL_NIL) fl_heap_release(t->content);
    if (t->headers.tag != FL_NIL) fl_heap_release(t->headers);
    if (t->callback.tag != FL_NIL) fl_heap_release(t->callback);
    free(t);
}

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
    case FL_ASYNC_ERROR: { const char *type = "AsyncIOError", *code = "E_ASYNC_IO"; if (t->result.tag == FL_MAP) { FLValue et = fl_map_get(t->result, fl_str_val("errorType")); FLValue ec = fl_map_get(t->result, fl_str_val("code")); if (et.tag == FL_STRING && et.obj) type = ((FLString *)et.obj)->data; if (ec.tag == FL_STRING && ec.obj) code = ((FLString *)ec.obj)->data; } return async_packet("error", fl_nil(), type, code); }
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
    if (callback.tag == FL_FN) { FLValue callback_result = fl_fn_call(callback,1,&packet); int failed = 0; if (callback_result.tag == FL_MAP) { FLValue type = fl_map_get(callback_result, fl_str_val("type")); FLValue error = fl_map_get(callback_result, fl_str_val("error")); failed = (type.tag == FL_STRING || error.tag != FL_NIL); } if (failed) { pthread_mutex_lock(&t->mu); if (!t->callback_failed) { FLValue type = fl_map_get(callback_result, fl_str_val("type")); FLValue message = fl_map_get(callback_result, fl_str_val("message")); snprintf(t->callback_error_type, sizeof(t->callback_error_type), "%s", type.tag == FL_STRING && type.obj ? ((FLString *)type.obj)->data : "AsyncCallbackError"); snprintf(t->callback_error_message, sizeof(t->callback_error_message), "%s", message.tag == FL_STRING && message.obj ? ((FLString *)message.obj)->data : "callback reported error"); t->callback_failed = 1; } pthread_mutex_unlock(&t->mu); fl_heap_release(callback_result); } pthread_mutex_lock(&t->mu); t->callback_completed = 1; pthread_cond_broadcast(&t->cv); pthread_mutex_unlock(&t->mu); fl_heap_release(callback); }
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
    pthread_mutex_lock(&async_handle_mu); t->id=async_next_id++; t->next=async_registry; async_registry=t; pthread_mutex_unlock(&async_handle_mu);
    if(pthread_create(&t->thread,NULL,async_worker,t)!=0){
        pthread_mutex_lock(&async_handle_mu); if(async_registry==t) async_registry=t->next; else { FLAsyncTask *p=async_registry; while(p&&p->next!=t)p=p->next; if(p)p->next=t->next; } pthread_mutex_unlock(&async_handle_mu);
        async_destroy_task(t); return fl_nil();
    }
    FLValue h=fl_map_new(); return fl_map_set(h,fl_str_val("__async_task__"),fl_int((int64_t)t->id));
}
static FLAsyncTask *async_task(FLValue h){ if(h.tag!=FL_MAP)return NULL; FLValue p=fl_map_get(h,fl_str_val("__async_task__")); if(p.tag!=FL_INT)return NULL; for(FLAsyncTask*t=async_registry;t;t=t->next) if(t->id==(uint64_t)p.i)return t; return NULL; }
FLValue fl_async_file_read(FLValue p){return async_start(FL_ASYNC_READ,p,fl_nil(),fl_nil());}
FLValue fl_async_file_write(FLValue p,FLValue c){return async_start(FL_ASYNC_WRITE,p,c,fl_nil());}
FLValue fl_async_sleep(FLValue ms){return async_start(FL_ASYNC_SLEEP,fl_str_val(""),ms,fl_nil());}
FLValue fl_async_http_get(FLValue url,FLValue headers){return async_start(FL_ASYNC_HTTP_GET,url,fl_nil(),headers);}
FLValue fl_async_http_post(FLValue url,FLValue body,FLValue headers){return async_start(FL_ASYNC_HTTP_POST,url,body,headers);}
FLValue fl_async_await(FLValue h,FLValue mv){
    pthread_mutex_lock(&async_handle_mu);
    FLAsyncTask*t=async_task(h); if(!t){pthread_mutex_unlock(&async_handle_mu);return async_packet("error",fl_nil(),"AsyncIOError","E_ASYNC_IO");}
    int64_t ms=mv.tag==FL_INT?mv.i:30000; struct timespec d; clock_gettime(CLOCK_REALTIME,&d); d.tv_sec+=ms/1000; d.tv_nsec+=(ms%1000)*1000000L;
    if(d.tv_nsec>=1000000000L){d.tv_sec++;d.tv_nsec-=1000000000L;}
    pthread_mutex_lock(&t->mu);
    while(t->terminal==FL_ASYNC_PENDING) if(pthread_cond_timedwait(&t->cv,&t->mu,&d)==ETIMEDOUT){if(t->terminal==FL_ASYNC_PENDING)t->terminal=FL_ASYNC_TIMEOUT; pthread_cond_broadcast(&t->cv); break;}
    FLValue packet=async_terminal_packet_locked(t); pthread_mutex_unlock(&t->mu); async_dispatch_callback(t); pthread_mutex_unlock(&async_handle_mu); return packet;
}
FLValue fl_async_cancel(FLValue h){pthread_mutex_lock(&async_handle_mu);FLAsyncTask*t=async_task(h);if(!t){pthread_mutex_unlock(&async_handle_mu);return fl_bool(false);}int result=async_claim_terminal(t,FL_ASYNC_CANCELLED,fl_nil());pthread_mutex_unlock(&async_handle_mu);return fl_bool(result);}
FLValue fl_async_set_callback(FLValue h,FLValue callback){
    pthread_mutex_lock(&async_handle_mu); FLAsyncTask*t=async_task(h); if(!t||callback.tag!=FL_FN){pthread_mutex_unlock(&async_handle_mu);return fl_bool(false);}
    pthread_mutex_lock(&t->mu); if(t->callback_registered){pthread_mutex_unlock(&t->mu);pthread_mutex_unlock(&async_handle_mu);return fl_bool(false);}
    t->callback=fl_heap_copy(callback); t->callback_registered=1; pthread_mutex_unlock(&t->mu); async_dispatch_callback(t); pthread_mutex_unlock(&async_handle_mu); return fl_bool(true);
}
FLValue fl_async_callback_error(FLValue h){ pthread_mutex_lock(&async_handle_mu); FLAsyncTask*t=async_task(h); if(!t){pthread_mutex_unlock(&async_handle_mu);return fl_nil();} pthread_mutex_lock(&t->mu); while (t->terminal == FL_ASYNC_PENDING || (t->callback_registered && !t->callback_completed)) pthread_cond_wait(&t->cv, &t->mu); FLValue result=t->callback_failed?fl_make_error(t->callback_error_type, t->callback_error_message):fl_nil(); pthread_mutex_unlock(&t->mu); pthread_mutex_unlock(&async_handle_mu); return result;}
FLValue fl_async_release(FLValue h){
    pthread_mutex_lock(&async_handle_mu); FLAsyncTask*t=async_task(h); if(!t||t->joined){pthread_mutex_unlock(&async_handle_mu);return fl_bool(false);}
    t->joined=1; pthread_join(t->thread,NULL);
    FLAsyncTask **pp=&async_registry; while(*pp&&*pp!=t)pp=&(*pp)->next; if(*pp==t)*pp=t->next;
    async_destroy_task(t); pthread_mutex_unlock(&async_handle_mu); return fl_bool(true);}

void fl_async_runtime_shutdown(void) {
    pthread_mutex_lock(&async_handle_mu);
    FLAsyncTask *tasks = async_registry;
    async_registry = NULL;
    pthread_mutex_unlock(&async_handle_mu);
    while (tasks) {
        FLAsyncTask *next = tasks->next;
        tasks->joined = 1;
        pthread_join(tasks->thread, NULL);
        async_destroy_task(tasks);
        tasks = next;
    }
}
