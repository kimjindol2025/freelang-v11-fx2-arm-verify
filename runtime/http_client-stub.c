// http_client-stub.c — HTTP 클라이언트 없는 빌드용 stub
#include "runtime.h"

FLValue http_get(FLValue url)                           { return fl_nil(); }
FLValue http_get_h(FLValue url, FLValue headers)        { return fl_nil(); }
FLValue http_post(FLValue url, FLValue body)            { return fl_nil(); }
FLValue http_post_h(FLValue url, FLValue body, FLValue h){ return fl_nil(); }
FLValue http_put(FLValue url, FLValue body)             { return fl_nil(); }
FLValue http_del(FLValue url)                           { return fl_nil(); }
FLValue http_patch(FLValue url, FLValue body)           { return fl_nil(); }
FLValue http_req(FLValue method, FLValue url, FLValue body, FLValue headers) { return fl_nil(); }
FLValue http_body(FLValue res)                          { return fl_nil(); }
FLValue http_status(FLValue res)                        { return fl_nil(); }
FLValue http_ok_p(FLValue res)                          { return fl_nil(); }
FLValue fl_http_get(FLValue url)                        { return fl_nil(); }
