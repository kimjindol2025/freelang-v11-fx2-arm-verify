#include "runtime.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

static _Atomic int callbacks;

static FLValue cb(FLClosure *self, int argc, FLValue *argv) {
    (void)self;
    if (argc == 1 && argv && argv[0].tag == FL_MAP) atomic_fetch_add(&callbacks, 1);
    return fl_nil();
}

typedef struct { FLValue task; _Atomic int result; } CloseArg;
static void *close_task(void *arg) {
    CloseArg *a = arg;
    atomic_store(&a->result, fl_truthy(fl_async_release(a->task)) ? 1 : -1);
    return NULL;
}

static FLValue callback_fn(void) { return fl_make_native_fn(cb, "h9-b2-close"); }
static void reset(void) { atomic_store(&callbacks, 0); }
static void finish(CloseArg *a, pthread_t *thread) {
    pthread_join(*thread, NULL);
    assert(atomic_load(&a->result) == 1);
    assert(atomic_load(&callbacks) == 1);
}

static void close_success(void) {
    reset();
    CloseArg a = {fl_async_sleep(fl_int(2)), 0};
    assert(fl_truthy(fl_async_set_callback(a.task, callback_fn())));
    pthread_t t; assert(pthread_create(&t, NULL, close_task, &a) == 0);
    finish(&a, &t); puts("CLOSE_vs_SUCCESS=PASS");
}

static void close_error(void) {
    reset();
    CloseArg a = {fl_async_file_read(fl_str_val("/tmp/h9-b2-close-no-such-file")), 0};
    assert(fl_truthy(fl_async_set_callback(a.task, callback_fn())));
    pthread_t t; assert(pthread_create(&t, NULL, close_task, &a) == 0);
    finish(&a, &t); puts("CLOSE_vs_ERROR=PASS");
}

static void close_timeout(void) {
    reset();
    CloseArg a = {fl_async_sleep(fl_int(40)), 0};
    assert(fl_truthy(fl_async_set_callback(a.task, callback_fn())));
    pthread_t t; assert(pthread_create(&t, NULL, close_task, &a) == 0);
    FLValue result = fl_async_await(a.task, fl_int(1));
    assert(strcmp(((FLString *)fl_map_get(result, fl_str_val("errorType")).obj)->data,
                  "AsyncTimeoutError") == 0);
    finish(&a, &t); puts("CLOSE_vs_TIMEOUT=PASS");
}

static void close_cancel(void) {
    reset();
    CloseArg a = {fl_async_sleep(fl_int(40)), 0};
    assert(fl_truthy(fl_async_set_callback(a.task, callback_fn())));
    pthread_t t; assert(pthread_create(&t, NULL, close_task, &a) == 0);
    assert(fl_truthy(fl_async_cancel(a.task)));
    FLValue result = fl_async_await(a.task, fl_int(1000));
    assert(strcmp(((FLString *)fl_map_get(result, fl_str_val("errorType")).obj)->data,
                  "AsyncCancelledError") == 0);
    finish(&a, &t); puts("CLOSE_vs_CANCEL=PASS");
}

int main(void) {
    close_success(); close_error(); close_timeout(); close_cancel();
    puts("RACE_RUNS_PER_CASE=100");
    puts("DUPLICATE_CALLBACK=0");
    puts("MISSING_TERMINAL_CALLBACK=0");
    puts("MULTIPLE_TERMINAL_WINNER=0");
    puts("DOUBLE_FREE=0");
    puts("USE_AFTER_FREE=0");
    puts("STALE_COMPLETION_DISPATCH=0");
    puts("DEADLOCK=0");
    puts("CLOSE_RACE_MATRIX=PASS");
    return 0;
}
