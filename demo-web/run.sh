#!/bin/bash
set -e

DEMO_DIR="$(cd "$(dirname "$0")" && pwd)"
FX2_DIR="$(dirname "$DEMO_DIR")"

echo "────────────────────────────────────"
echo " fx2 Mini Web Server"
echo "────────────────────────────────────"
echo ""

echo "[1/3] 패키지 설치..."
node /home/kimjin/freelang-v11/bootstrap.js run "$DEMO_DIR/setup.fl"

echo ""
echo "[2/3] 바이너리 빌드..."
cd "$DEMO_DIR"
bash "$FX2_DIR/fl-build.sh" app.fl web-server 2>&1 | grep -E "✅|❌|빌드|완료" || true

echo ""
echo "[3/3] 서버 시작..."
./web-server
