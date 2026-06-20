// http-stub.c — HTTP 서버 없는 빌드용 stub
// openssl 의존 없이 링크 가능. 모든 함수는 nil 반환.
#include "runtime.h"

FLValue server_get(FLValue p, FLValue h)     { return fl_nil(); }
FLValue server_post(FLValue p, FLValue h)    { return fl_nil(); }
FLValue server_put(FLValue p, FLValue h)     { return fl_nil(); }
FLValue server_patch(FLValue p, FLValue h)   { return fl_nil(); }
FLValue server_delete(FLValue p, FLValue h)  { return fl_nil(); }
FLValue server_html(FLValue html)            { return fl_nil(); }
FLValue server_text(FLValue text)            { return fl_nil(); }
FLValue server_json(FLValue json)            { return fl_nil(); }
FLValue server_status(FLValue code, FLValue body) { return fl_nil(); }
FLValue server_redirect(FLValue url)         { return fl_nil(); }
FLValue server_req_param(FLValue req, FLValue k)  { return fl_nil(); }
FLValue server_req_query(FLValue req, FLValue k)  { return fl_nil(); }
FLValue fxb_server_req_body(FLValue req)     { return fl_nil(); }
FLValue server_req_header(FLValue req, FLValue k) { return fl_nil(); }
FLValue server_req_method(FLValue req)       { return fl_nil(); }
FLValue server_req_path(FLValue req)         { return fl_nil(); }
FLValue server_start(FLValue port)           { return fl_nil(); }
FLValue server_start_tls(FLValue p, FLValue c, FLValue k) { return fl_nil(); }
FLValue fl_http_route(FLValue m, FLValue p, FLValue h)    { return fl_nil(); }
FLValue fl_http_start(FLValue port)          { return fl_nil(); }
FLValue fl_resp_html(FLValue html)           { return fl_nil(); }
FLValue fl_resp_json(FLValue json)           { return fl_nil(); }
FLValue fl_resp_text(FLValue text)           { return fl_nil(); }
FLValue fl_resp_status(FLValue code, FLValue body) { return fl_nil(); }
FLValue fl_resp_redirect(FLValue url)        { return fl_nil(); }
