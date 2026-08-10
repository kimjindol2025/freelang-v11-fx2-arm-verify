#!/usr/bin/env bash
# tools/fl-verify-callable-builtin-smoke.sh — native callable builtin smoke (no-net)

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$(readlink -f "$0")")/.." && pwd)"
SMOKE_SRC="$(mktemp /tmp/fx2-callable-builtin-smoke.XXXXXX.fl)"
SMOKE_BIN="$(mktemp /tmp/fx2-callable-builtin-smoke.XXXXXX.bin)"
SMOKE_LOG="$(mktemp /tmp/fx2-callable-builtin-smoke.XXXXXX.log)"
PKG_JSON_FILE="$(mktemp /tmp/fx2-callable-builtin-smoke.XXXXXX.json)"
BUILD_TREE="$(mktemp -d /tmp/fx2-callable-builtin-smoke-tree.XXXXXX)"

cleanup() {
  rm -f "$SMOKE_SRC" "$SMOKE_BIN" "$SMOKE_LOG" "$PKG_JSON_FILE"
  rm -rf "$BUILD_TREE"
}
trap cleanup EXIT

mkdir -p "$BUILD_TREE/runtime" "$BUILD_TREE/packages"
cp -R "$ROOT_DIR/runtime/." "$BUILD_TREE/runtime/"
cp -R "$ROOT_DIR/packages/." "$BUILD_TREE/packages/"
cp "$ROOT_DIR/fl-build.sh" "$ROOT_DIR/fl-build-helper.c" "$ROOT_DIR/fl-str-split.c" \
  "$ROOT_DIR/fl-module-parse.c" "$ROOT_DIR/fl-resolve-deps-profiles.c" "$ROOT_DIR/fl-generate-user-fns.c" \
  "$ROOT_DIR/fl-build-plan.py" "$ROOT_DIR/fl-module-parse.py" "$ROOT_DIR/fl-resolve-deps.py" "$ROOT_DIR/fl-str-split.py" "$ROOT_DIR/fl-pkg-gen-c.py" \
  "$ROOT_DIR/fl-error-vaccine.py" "$BUILD_TREE/"

cp "$BUILD_TREE/runtime/user-fns.c" "$BUILD_TREE/runtime/user-fns.c.orig"

echo "[1/4] registry + user-fns regeneration check via fl-pkg-gen-c.py"
python3 "$BUILD_TREE/fl-pkg-gen-c.py" "$BUILD_TREE" >"$PKG_JSON_FILE"
cat "$PKG_JSON_FILE"

PKG_MISSING_COUNT="$(python3 - "$PKG_JSON_FILE" <<'PY'
import json, sys
payload = json.load(open(sys.argv[1]))
print(len(payload.get("missing", [])))
PY
)"
PKG_COUNT="$(python3 - "$PKG_JSON_FILE" <<'PY'
import json, sys
payload = json.load(open(sys.argv[1]))
print(payload.get("count", 0))
PY
)"
if [ "$PKG_MISSING_COUNT" != "0" ]; then
  echo "❌ missing package metadata detected"
  exit 1
fi
echo "   package count: $PKG_COUNT, missing: []"

echo "[2/4] prepare copied user-fns without SHIM_SECTION"
awk '
  /\/\* FL:SHIM_SECTION:BEGIN \*\// {skip=1; next}
  /\/\* FL:SHIM_SECTION:END \*\// {skip=0; next}
  !skip
' "$BUILD_TREE/runtime/user-fns.c" > "$BUILD_TREE/runtime/user-fns.no-shim.c"
mv "$BUILD_TREE/runtime/user-fns.no-shim.c" "$BUILD_TREE/runtime/user-fns.c"

cat >"$SMOKE_SRC" <<'FL'
(println "[smoke] str-lines")
(println (length (str-lines "hello\nworld\nfx2")))
(println "[smoke] str-count")
(println (str-count "hello world hello" "hello"))
(println "[smoke] math-lerp")
(println (math-lerp 0 100 0.5))
(println "[smoke] math-round-n")
(println (math-round-n 3.14159 2))
(println "[smoke] math-sign")
(println (math-sign -3))
(println "[smoke] path-dirname")
(println (path-dirname "/home/user/docs/file.txt"))
(println "[smoke] str-indent")
(println (str-indent "hello" 4))
(println "[smoke] str-truncate")
(println (str-truncate "Hello, world!" 8 "..."))
(println "[smoke] str-rpad")
(println (str-rpad "hi" 6 "."))
(println "[smoke] math-clamp")
(println (math-clamp 15 0 10))
(println "[smoke] time-now-ms")
(println (> (time-now-ms) 0))
(println "[smoke] time-elapsed")
(let [$start (time-now-ms)] (println (>= (time-elapsed $start) 0)))
(println "[smoke] path-basename")
(println (path-basename "/home/user/docs/file.txt"))
FL

echo "[3/4] build native smoke binary (fl-build.sh --no-net)"
bash "$BUILD_TREE/fl-build.sh" "$SMOKE_SRC" "$SMOKE_BIN" --no-net
if [ ! -x "$SMOKE_BIN" ]; then
  echo "❌ smoke binary was not produced"
  exit 1
fi

echo "[4/4] execute smoke binary"
if ! "$SMOKE_BIN" >"$SMOKE_LOG" 2>&1; then
  echo "❌ smoke binary execution failed"
  cat "$SMOKE_LOG"
  exit 1
fi

cat "$SMOKE_LOG"
echo "✅ native callable builtin smoke PASS"
