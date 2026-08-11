#include "runtime.h"
#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE
int fx2_wasm_smoke(void) {
    FLValue left = fl_str_val("fx2 ");
    FLValue right = fl_str_val("wasm");
    FLValue joined = fl_add(left, right);
    FLValue values[2] = { fl_int(7), fl_int(11) };
    FLValue vector = fl_vec_from(values, 2);
    return joined.tag == FL_STRING &&
           fl_vec_len(vector).i == 2 &&
           fl_vec_get(vector, fl_int(1)).i == 11;
}
