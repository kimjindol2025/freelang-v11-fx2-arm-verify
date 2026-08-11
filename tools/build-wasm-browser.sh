#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT_DIR/build/wasm-browser}"
mkdir -p "$OUT_DIR"

command -v emcc >/dev/null || { echo "emcc is required" >&2; exit 1; }

emcc -O2 -DFL_NO_CRYPTO -I "$ROOT_DIR/runtime" \
  "$ROOT_DIR/runtime/wasm_browser_smoke.c" \
  "$ROOT_DIR/runtime/core.c" \
  "$ROOT_DIR/runtime/collection.c" \
  "$ROOT_DIR/runtime/error.c" \
  "$ROOT_DIR/runtime/gc.c" \
  "$ROOT_DIR/runtime/math.c" \
  "$ROOT_DIR/runtime/crypto-stub.c" \
  -o "$OUT_DIR/fx2-browser.js" \
  -sENVIRONMENT=web \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXPORTED_FUNCTIONS='["_fx2_wasm_smoke"]' \
  -sEXPORTED_RUNTIME_METHODS='["ccall"]'

cp "$ROOT_DIR/docs/wasm-browser-adapter-smoke.html" "$OUT_DIR/index.html"
cp "$ROOT_DIR/docs/fx2-wasm-adapter.js" "$OUT_DIR/fx2-wasm-adapter.js"
echo "Browser WASM smoke built: $OUT_DIR"
