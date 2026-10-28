#!/usr/bin/env bash
set -eu
ROOT_DIR="$(cd "$(dirname "$(readlink -f "$0")")/.." && pwd)"
BIN="$(mktemp /tmp/fx-async-native.XXXXXX)"
trap 'rm -f "$BIN"' EXIT
gcc -D_GNU_SOURCE -std=c11 -O2 -I "$ROOT_DIR/runtime" \
  "$ROOT_DIR/tests/async_native_test.c" "$ROOT_DIR/runtime/async.c" \
  "$ROOT_DIR/runtime/core.c" "$ROOT_DIR/runtime/collection.c" \
  "$ROOT_DIR/runtime/gc.c" "$ROOT_DIR/runtime/io.c" "$ROOT_DIR/runtime/error.c" \
  "$ROOT_DIR/runtime/debug.c" \
  -lpthread -lcurl -lm -ldl -o "$BIN"
timeout 20s "$BIN"
