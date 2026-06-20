// crypto-stub.c — openssl 없는 빌드용 stub (sha256, md5)
// --no-net 빌드 시 aliases.c 대신 이 파일의 함수 사용
#include "runtime.h"

FLValue sha256(FLValue s) { return fl_str_val("(sha256-unavailable)"); }
FLValue md5(FLValue s)    { return fl_str_val("(md5-unavailable)"); }
