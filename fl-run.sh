#!/bin/bash
# fl-run.sh — FreeLang fx 자동 빌드 + 실행 (PM2 호환)
#
# 사용:
#   bash fl-run.sh server.fl              # 빌드 후 직접 실행
#   bash fl-run.sh server.fl my-app       # 바이너리 이름 지정
#
# PM2:
#   pm2 start /home/kimjin/freelang-v11-fx/fl-run.sh \
#     --name my-app -- server.fl my-app
#
# 환경변수:
#   FL_WATCH=1     파일 변경 감시 (inotifywait 필요)
#   FL_NO_REBUILD  빌드 스킵 (이미 빌드된 바이너리 그대로 실행)

set -e

SRC="${1:-server.fl}"
NAME="${2:-$(basename "$(pwd)")}"
FX_DIR="/home/kimjin/freelang-v11-fx"

if [ ! -f "$SRC" ]; then
    echo "[fl-run] ❌ 소스 파일 없음: $SRC" >&2
    exit 1
fi

if [ -z "$FL_NO_REBUILD" ]; then
    echo "[fl-run] 🔨 빌드: $SRC → ./$NAME"
    bash "$FX_DIR/fl-build.sh" "$SRC" "$NAME"
    echo "[fl-run] ✅ 빌드 완료"
fi

if [ ! -f "./$NAME" ]; then
    echo "[fl-run] ❌ 바이너리 없음: ./$NAME" >&2
    exit 1
fi

echo "[fl-run] 🚀 실행: ./$NAME"
exec "./$NAME"
