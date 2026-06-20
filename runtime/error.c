#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* 스레드마다 독립된 try 스택 */
__thread FLTryFrame fl_try_stack[FL_TRY_MAX];
__thread int fl_try_top = 0;

/* FL 소스 라인 추적 (cgc-main이 throw 직전에 설정) */
int __fl_throw_line = 0;

/* ── 콜스택 추적 ── */
#define FL_CALLSTACK_MAX 64
__thread const char* __fl_callstack[FL_CALLSTACK_MAX];
__thread int         __fl_callstack_line[FL_CALLSTACK_MAX];
__thread int __fl_callstack_depth = 0;

void fl_push_frame(const char* fn) {
    if (__fl_callstack_depth < FL_CALLSTACK_MAX) {
        __fl_callstack[__fl_callstack_depth]      = fn;
        __fl_callstack_line[__fl_callstack_depth] = 0;
        __fl_callstack_depth++;
    }
}
void fl_push_frame_ln(const char* fn, int line) {
    if (__fl_callstack_depth < FL_CALLSTACK_MAX) {
        __fl_callstack[__fl_callstack_depth]      = fn;
        __fl_callstack_line[__fl_callstack_depth] = line;
        __fl_callstack_depth++;
        if (line > 0) __fl_throw_line = line;
    }
}
void fl_pop_frame(void) {
    if (__fl_callstack_depth > 0) __fl_callstack_depth--;
}

static void print_uncaught(FLValue err) {
    int lineno = __fl_throw_line;
    char locbuf[32] = "";
    if (lineno > 0) snprintf(locbuf, sizeof(locbuf), " (line %d)", lineno);
    if (err.tag == FL_STRING && err.obj)
        fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)err.obj)->data);
    else if (err.tag == FL_MAP) {
        FLValue msg = fl_map_get(err, fl_str_val("message"));
        if (msg.tag == FL_STRING && msg.obj)
            fprintf(stderr, "Uncaught error%s: %s\n", locbuf, ((FLString*)msg.obj)->data);
        else fprintf(stderr, "Uncaught error%s\n", locbuf);
    } else fprintf(stderr, "Uncaught error%s\n", locbuf);
    /* 콜스택 출력 */
    if (__fl_callstack_depth > 0) {
        fprintf(stderr, "Stack trace:\n");
        for (int i = __fl_callstack_depth - 1; i >= 0; i--) {
            int ln = __fl_callstack_line[i];
            if (ln > 0)
                fprintf(stderr, "  at %s (line %d)\n", __fl_callstack[i], ln);
            else
                fprintf(stderr, "  at %s\n", __fl_callstack[i]);
        }
    }
}

void fl_throw(FLValue err) {
    if (fl_try_top <= 0) {
        print_uncaught(err);
        exit(1);
    }
    fl_try_stack[fl_try_top - 1].err = err;
    longjmp(fl_try_stack[fl_try_top - 1].buf, 1);
}

FLValue fl_make_error(const char* type, const char* msg) {
    FLValue m = fl_map_new();
    m = fl_map_set(m, fl_str_val("type"),    fl_str_val(type));
    m = fl_map_set(m, fl_str_val("message"), fl_str_val(msg));
    return m;
}
