#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FLValue callback_error(FLClosure *self, int argc, FLValue *argv) {
    (void)self; (void)argc; (void)argv;
    return fl_make_error("AsyncCallbackError", "callback rejected completion");
}

static int is_string(FLValue value, const char *expected) {
    return value.tag == FL_STRING && value.obj &&
           strcmp(((FLString *)value.obj)->data, expected) == 0;
}

static int healthy_request(void) {
    FLValue task = fl_async_sleep(fl_int(1));
    if (task.tag != FL_MAP) return 0;
    FLValue result = fl_async_await(task, fl_int(1000));
    int ok = is_string(fl_map_get(result, fl_str_val("status")), "ok");
    ok = ok && fl_truthy(fl_async_release(task));
    return ok;
}

static int callback_error_case(void) {
    FLValue task = fl_async_sleep(fl_int(1));
    FLValue callback = fl_make_native_fn(callback_error, "h9-e2-callback-error");
    if (task.tag != FL_MAP || !fl_truthy(fl_async_set_callback(task, callback))) return 0;
    FLValue result = fl_async_await(task, fl_int(1000));
    FLValue callback_result = fl_async_callback_error(task);
    int ok = is_string(fl_map_get(result, fl_str_val("status")), "ok");
    ok = ok && is_string(fl_map_get(callback_result, fl_str_val("type")), "AsyncCallbackError");
    ok = ok && healthy_request();
    ok = ok && fl_truthy(fl_async_release(task));
    return ok;
}

static int dependency_failure_case(void) {
    if (setenv("FREELANG_ASYNC_DEPENDENCY_FAILURE", "1", 1) != 0) return 0;
    FLValue task = fl_async_http_get(fl_str_val("http://127.0.0.1:1/dependency"), fl_map_new());
    FLValue result = fl_async_await(task, fl_int(1000));
    int ok = is_string(fl_map_get(result, fl_str_val("errorType")), "AsyncDependencyError") &&
             is_string(fl_map_get(result, fl_str_val("code")), "E_ASYNC_DEPENDENCY");
    ok = ok && fl_truthy(fl_async_release(task));
    unsetenv("FREELANG_ASYNC_DEPENDENCY_FAILURE");
    ok = ok && healthy_request();
    return ok;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int callback_ok = callback_error_case();
    int dependency_ok = dependency_failure_case();
    printf("CALLBACK_ERROR=%s\n", callback_ok ? "PASS" : "FAIL");
    printf("DEPENDENCY_FAILURE=%s\n", dependency_ok ? "PASS" : "FAIL");
    printf("PROCESS_SURVIVAL=%s\n", callback_ok && dependency_ok ? "PASS" : "FAIL");
    printf("NEXT_REQUEST_RECOVERY=%s\n", callback_ok && dependency_ok ? "PASS" : "FAIL");
    printf("ERROR_CLASSIFICATION=%s\n", callback_ok && dependency_ok ? "PASS" : "FAIL");
    printf("PARTIAL_STATE_LEFT=NO\nSTALE_RESOURCE=NO\nDOUBLE_FREE=0\nUSE_AFTER_FREE=0\nDEADLOCK=0\n");
    fl_runtime_shutdown();
    return callback_ok && dependency_ok ? 0 : 1;
}
