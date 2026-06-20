// websocket-stub.c — WebSocket 없는 빌드용 stub
#include "runtime.h"

FLValue ws_recv(FLValue ws)               { return fl_nil(); }
FLValue ws_send(FLValue ws, FLValue msg)  { return fl_nil(); }
FLValue ws_close(FLValue ws)              { return fl_nil(); }
FLValue ws_conn_id(FLValue ws)            { return fl_nil(); }
FLValue server_ws(FLValue path, FLValue h){ return fl_nil(); }
