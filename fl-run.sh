#!/bin/bash
# fl-run.sh — fx.toml 읽고 패키지 설치 + 빌드 + 실행
#
# 사용법:
#   bash fl-run.sh            # 현재 디렉토리 fx.toml 사용
#   bash fl-run.sh build      # 설치+빌드만 (실행 안 함)
#   bash fl-run.sh install    # 설치만

set -e

SCRIPT_REAL="$(readlink -f "$0")"
FX2_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
TOML_FILE="${TOML:-$(pwd)/fx.toml}"
CMD="${1:-run}"

if [ ! -f "$TOML_FILE" ]; then
  echo "❌ fx.toml 없음: $TOML_FILE"
  echo "   fx.toml 예시:"
  echo '   [project]'
  echo '   name = "my-app"'
  echo '   entry = "main.fl"'
  echo '   [runtime]'
  echo '   packages = ["str-indent", "math-clamp"]'
  exit 1
fi

# ── fx.toml 파싱 (native helper) ──────────────────────────────
FL_TOML_PARSE_SRC="$FX2_DIR/fl-toml-parse.c"
FL_TOML_PARSE_BIN="$FX2_DIR/.fl-toml-parse"

build_fl_toml_parse() {
  if [ -x "$FL_TOML_PARSE_BIN" ] && [ "$FL_TOML_PARSE_BIN" -nt "$FL_TOML_PARSE_SRC" ]; then
    return 0
  fi
  if ! cc -O2 -std=c99 "$FL_TOML_PARSE_SRC" -o "$FL_TOML_PARSE_BIN" 2>/tmp/fl-toml-parse-build.log; then
    echo "[fl-run] fl-toml-parse.c build failed" >&2
    cat /tmp/fl-toml-parse-build.log >&2
    return 1
  fi
}

build_fl_toml_parse || exit 1
FL_NAME=$($FL_TOML_PARSE_BIN --get project.name "$TOML_FILE")
FL_ENTRY=$($FL_TOML_PARSE_BIN --get project.entry "$TOML_FILE")
FL_PORT=$($FL_TOML_PARSE_BIN --get project.port "$TOML_FILE")
FL_OUTPUT=$($FL_TOML_PARSE_BIN --get project.output "$TOML_FILE")
mapfile -t FL_PACKAGES < <($FL_TOML_PARSE_BIN --get-array-lines runtime.packages "$TOML_FILE")
[ -n "$FL_NAME" ] || FL_NAME="app"
[ -n "$FL_ENTRY" ] || FL_ENTRY="main.fl"
[ -n "$FL_PORT" ] || FL_PORT="0"
[ -n "$FL_OUTPUT" ] || FL_OUTPUT="app-bin"

echo "────────────────────────────────────────"
echo " fl run — $FL_NAME"
echo "────────────────────────────────────────"
echo ""

# ── 패키지 설치 ───────────────────────────────────────────────
if [ "$CMD" = "run" ] || [ "$CMD" = "install" ] || [ "$CMD" = "build" ]; then
  echo "[install] 런타임 패키지 설치..."
  if [ ${#FL_PACKAGES[@]} -eq 0 ]; then
    echo "  (패키지 없음)"
  else
    ok_count=0
    for name in "${FL_PACKAGES[@]}"; do
      payload=$($FL_TOML_PARSE_BIN --run-pkg-import-payload "$name")
      result=$(curl -s -X POST "http://localhost:40330/fx/fns/import"         -H "Content-Type: application/json" -d "$payload" 2>/dev/null)
      result_file=$(mktemp)
      printf '%s' "$result" > "$result_file"
      "$FL_TOML_PARSE_BIN" --run-pkg-import-output "$name" "$result_file"
      ok_flag=$($FL_TOML_PARSE_BIN --new-init-ok "$result_file" 2>/dev/null)
      [ "$ok_flag" = "True" ] && ok_count=$((ok_count + 1))
      rm -f "$result_file"
    done
    echo ""
    echo "  ${ok_count}/${#FL_PACKAGES[@]} 패키지 설치 완료"
  fi
  echo ""
fi

# ── 빌드 ─────────────────────────────────────────────────────
if [ "$CMD" = "run" ] || [ "$CMD" = "build" ]; then
  echo "[build] $FL_ENTRY → $FL_OUTPUT"
  bash "$FX2_DIR/fl-build.sh" "$FL_ENTRY" "$FL_OUTPUT" 2>&1 | grep -E "✅|❌|오류|error" || true
  echo ""
fi

# ── 실행 ─────────────────────────────────────────────────────
if [ "$CMD" = "run" ]; then
  echo "[run] ./$FL_OUTPUT"
  echo "────────────────────────────────────────"
  echo ""
  "./$FL_OUTPUT"
fi
