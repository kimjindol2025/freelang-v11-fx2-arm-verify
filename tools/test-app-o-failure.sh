#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$(mktemp /tmp/fx2-app-o-failure.XXXXXX.fl)"
OUT="$(mktemp /tmp/fx2-app-o-failure.XXXXXX.bin)"
LOG="$(mktemp /tmp/fx2-app-o-failure.XXXXXX.log)"

cleanup() {
  rm -f "$SRC" "$OUT" "$LOG"
}
trap cleanup EXIT

cat >"$SRC" <<'FL'
(definitely-missing-native 1)
FL

if CGC_BIN="${CGC_BIN:-/home/kimjin/freelang-v11/bin/cgc-bin}" \
  bash "$ROOT/fl-build.sh" "$SRC" "$OUT" >"$LOG" 2>&1; then
  echo "expected fl-build.sh to fail"
  cat "$LOG"
  exit 1
fi

if ! grep -q "app.o 컴파일 실패" "$LOG"; then
  echo "expected app.o failure diagnostic"
  cat "$LOG"
  exit 1
fi

if grep -q "cannot find .*\\.o" "$LOG"; then
  echo "linker .o-not-found error leaked through"
  cat "$LOG"
  exit 1
fi

echo "PASS app.o failure is reported before link"
