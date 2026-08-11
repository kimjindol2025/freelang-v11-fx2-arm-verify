#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT_DIR/build/wasm}"
mkdir -p "$OUT_DIR"

command -v emcc >/dev/null || { echo "emcc is required" >&2; exit 1; }
command -v wasm2wat >/dev/null || { echo "wasm2wat is required" >&2; exit 1; }

emcc -O2 -DFL_NO_CRYPTO -I "$ROOT_DIR/runtime" \
  "$ROOT_DIR/runtime/test_runtime.c" \
  "$ROOT_DIR/runtime/core.c" \
  "$ROOT_DIR/runtime/collection.c" \
  "$ROOT_DIR/runtime/error.c" \
  "$ROOT_DIR/runtime/io.c" \
  "$ROOT_DIR/runtime/gc.c" \
  "$ROOT_DIR/runtime/math.c" \
  "$ROOT_DIR/runtime/aliases.c" \
  "$ROOT_DIR/runtime/crypto-stub.c" \
  "$ROOT_DIR/runtime/json.c" \
  -o "$OUT_DIR/fx2-runtime-smoke.js" \
  -sENVIRONMENT=node \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXIT_RUNTIME=1

wasm2wat "$OUT_DIR/fx2-runtime-smoke.wasm" -o "$OUT_DIR/fx2-runtime-smoke.wat"
node "$OUT_DIR/fx2-runtime-smoke.js"
echo "WASM runtime smoke passed: $OUT_DIR/fx2-runtime-smoke.wasm"
