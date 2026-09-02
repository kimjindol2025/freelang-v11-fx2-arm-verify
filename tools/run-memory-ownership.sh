#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC_BIN="${CC:-cc}"
BUILD_DIR="$(mktemp -d "${TMPDIR:-/tmp}/freelang-memory-ownership.XXXXXX")"
BIN="$BUILD_DIR/runtime-sanitize"

cleanup() {
  rm -rf "$BUILD_DIR"
}
trap cleanup EXIT

cd "$ROOT_DIR"

test "$(uname -m)" = "aarch64" -o "$(uname -m)" = "x86_64" || {
  echo "MEMORY_OWNERSHIP=FAIL_UNSUPPORTED_ARCH: $(uname -m)" >&2
  exit 2
}

command -v "$CC_BIN" >/dev/null || {
  echo "MEMORY_OWNERSHIP=FAIL_MISSING_COMPILER: $CC_BIN" >&2
  exit 2
}

sources=(
  runtime/test_runtime.c
  runtime/aliases.c runtime/builtins-shim.c runtime/collection.c
  runtime/core.c runtime/debug.c runtime/error.c runtime/fx-builtin-shim.c
  runtime/gc.c runtime/http.c runtime/http_client.c runtime/io.c
  runtime/jit.c runtime/json.c runtime/mariadb.c runtime/math.c
  runtime/process.c runtime/regex.c runtime/smtp.c runtime/sqlite.c
  runtime/sse.c runtime/user-fns.c runtime/websocket.c
)

for source in "${sources[@]}"; do
  test -f "$source" || {
    echo "MEMORY_OWNERSHIP=FAIL_MISSING_SOURCE: $source" >&2
    exit 2
  }
done

"$CC_BIN" -std=c11 -D_POSIX_C_SOURCE=200809L \
  -fsanitize=address,undefined -fno-omit-frame-pointer -g -O1 \
  -Iruntime "${sources[@]}" -o "$BIN" \
  -lpthread -lm -lsqlite3 -lcrypto -lssl -lcurl -ldl

ARCH="$(uname -m)"
ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1:halt_on_error=1}" \
UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
  "$BIN"

echo "ARCH=$ARCH"
echo "ASAN=PASS"
echo "UBSAN=PASS"
echo "LEAK=PASS"
echo "PROCESS_EXIT=0"
