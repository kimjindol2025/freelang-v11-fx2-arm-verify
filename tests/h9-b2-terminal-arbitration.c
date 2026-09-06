#include "runtime.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

static _Atomic int callback_total;
static _Atomic int callback_success;
static _Atomic int callback_error;
static _Atomic int callback_timeout;
static _Atomic int callback_cancel;

static FLValue terminal_callback(FLClosure *self, int argc, FLValue *argv) {
    (void)self;
    if (argc != 1 || !argv || argv[0].tag != FL_MAP) return fl_nil();
    atomic_fetch_add(&callback_total, 1);
    FLValue status = fl_map_get(argv[0], fl_str_val("status"));
    FLValue error = fl_map_get(argv[0], fl_str_val("errorType"));
    if (status.tag == FL_STRING && strcmp(((FLString *)status.obj)->data, "ok") == 0)
        atomic_fetch_add(&callback_success, 1);
    else if (error.tag == FL_STRING && strcmp(((FLString *)error.obj)->data, "AsyncTimeoutError") == 0)
        atomic_fetch_add(&callback_timeout, 1);
    else if (error.tag == FL_STRING && strcmp(((FLString *)error.obj)->data, "AsyncCancelledError") == 0)
        atomic_fetch_add(&callback_cancel, 1);
    else if (error.tag == FL_STRING) atomic_fetch_add(&callback_error, 1);
    return fl_nil();
}

static FLValue callback_fn(void) { return fl_make_native_fn(terminal_callback, "h9-b2-terminal"); }
static void reset_counts(void) {
    atomic_store(&callback_total, 0); atomic_store(&callback_success, 0);
    atomic_store(&callback_error, 0); atomic_store(&callback_timeout, 0); atomic_store(&callback_cancel, 0);
}
static void assert_one_callback(void) { assert(atomic_load(&callback_total) == 1); }

static void case_success(void) {
    reset_counts(); FLValue task = fl_async_sleep(fl_int(2));
    assert(fl_truthy(fl_async_set_callback(task, callback_fn())));
    FLValue result = fl_async_await(task, fl_int(1000));
    assert(fl_map_get(result, fl_str_val("status")).tag == FL_STRING);
    assert(atomic_load(&callback_success) == 1); assert_one_callback(); assert(fl_truthy(fl_async_release(task)));
    puts("B2_SUCCESS=PASS");
}
static void case_error(void) {
    reset_counts(); FLValue task = fl_async_file_read(fl_str_val("/tmp/h9-b2-no-such-file"));
    assert(fl_truthy(fl_async_set_callback(task, callback_fn())));
    FLValue result = fl_async_await(task, fl_int(1000));
    assert(strcmp(((FLString *)fl_map_get(result, fl_str_val("errorType")).obj)->data, "AsyncIOError") == 0);
    assert(atomic_load(&callback_error) == 1); assert_one_callback(); assert(fl_truthy(fl_async_release(task)));
    puts("B2_ERROR=PASS");
}
static void case_timeout(void) {
    reset_counts(); FLValue task = fl_async_sleep(fl_int(40));
    assert(fl_truthy(fl_async_set_callback(task, callback_fn())));
    FLValue result = fl_async_await(task, fl_int(1));
    assert(strcmp(((FLString *)fl_map_get(result, fl_str_val("errorType")).obj)->data, "AsyncTimeoutError") == 0);
    assert(atomic_load(&callback_timeout) == 1); assert_one_callback(); assert(fl_truthy(fl_async_release(task)));
    puts("B2_TIMEOUT=PASS");
}
static void case_cancel(void) {
    reset_counts(); FLValue task = fl_async_sleep(fl_int(40));
    assert(fl_truthy(fl_async_set_callback(task, callback_fn())));
    assert(fl_truthy(fl_async_cancel(task)));
    FLValue result = fl_async_await(task, fl_int(1000));
    assert(strcmp(((FLString *)fl_map_get(result, fl_str_val("errorType")).obj)->data, "AsyncCancelledError") == 0);
    assert(atomic_load(&callback_cancel) == 1); assert_one_callback(); assert(fl_truthy(fl_async_release(task)));
    puts("B2_CANCEL=PASS");
}

typedef struct { FLValue task; } RaceArg;
static void *race_cancel(void *arg) { RaceArg *a = arg; (void)fl_async_cancel(a->task); return NULL; }
static void *race_timeout(void *arg) { RaceArg *a = arg; (void)fl_async_await(a->task, fl_int(1)); return NULL; }

static void case_race(void) {
    for (int i = 0; i < 100; i++) {
        reset_counts(); FLValue task = fl_async_sleep(fl_int(20));
        assert(fl_truthy(fl_async_set_callback(task, callback_fn())));
        RaceArg arg = {task}; pthread_t a, b;
        assert(pthread_create(&a, NULL, race_cancel, &arg) == 0);
        assert(pthread_create(&b, NULL, race_timeout, &arg) == 0);
        pthread_join(a, NULL); pthread_join(b, NULL);
        assert_one_callback(); assert(fl_truthy(fl_async_release(task)));
    }
    puts("B2_CONCURRENT_TERMINAL_RACE=PASS");
    puts("RACE_RUNS_PER_CASE=100");
    puts("DUPLICATE_CALLBACK=0");
    puts("MISSING_TERMINAL_CALLBACK=0");
    puts("MULTIPLE_TERMINAL_WINNER=0");
}

int main(void) {
    case_success(); case_error(); case_timeout(); case_cancel(); case_race();
    puts("B2_EXACTLY_ONCE=PASS");
    return 0;
}
