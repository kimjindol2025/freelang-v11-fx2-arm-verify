#!/bin/bash
set -e

DEMO_DIR="$(cd "$(dirname "$0")" && pwd)"
FX2_DIR="$(dirname "$DEMO_DIR")"

echo "────────────────────────────────────"
echo " fx2 Demo — Self-Extensible Runtime"
echo "────────────────────────────────────"
echo ""

# 1단계: 패키지 설치 (v11 bootstrap)
echo "[1/2] 패키지 설치..."
node /home/kimjin/freelang-v11/bootstrap.js run "$DEMO_DIR/setup.fl"

echo ""
echo "[2/2] 앱 빌드 & 실행..."
cd "$DEMO_DIR"
bash "$FX2_DIR/fl-build.sh" app.fl demo-bin 2>&1 | grep -E "✅|❌|빌드|완료" || true
echo ""
./demo-bin

echo ""
echo "────────────────────────────────────"
echo " Done."
echo "────────────────────────────────────"
