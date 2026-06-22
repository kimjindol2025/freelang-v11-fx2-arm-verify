#!/usr/bin/env bash
# tools/fl-verify.sh — canonical verification entrypoint

set -e

ROOT_DIR="$(cd "$(dirname "$(readlink -f "$0")")/.." && pwd)"
BOOTSTRAP="${BOOTSTRAP:-/root/freelang-v11/bootstrap.js}"

mode="${1:-official}"
shift || true

run_check() {
  local input="$1"
  local output="${2:-$(basename "${input%.fl}")}"
  python3 "$ROOT_DIR/fl-build-plan.py" "$input" "$ROOT_DIR" "$output" check
}

run_graph() {
  local input="$1"
  local output="${2:-$(basename "${input%.fl}")}"
  python3 "$ROOT_DIR/fl-build-plan.py" "$input" "$ROOT_DIR" "$output" graph
}

run_rules() {
  local rules="$1"
  local input="$2"
  node "$BOOTSTRAP" run \
    "$ROOT_DIR/fl-rules-runner.fl" \
    "$rules" "$input"
}

case "$mode" in
  official)
    input="${1:-}"
    if [ -z "$input" ]; then
      echo "사용법: tools/fl-verify.sh official <input.fl>" >&2
      exit 1
    fi
    if [ "$(basename "$input")" = "conformance.fl" ]; then
      exec "$0" conformance "$input"
    fi
    exec "$0" local "$input"
    ;;

  conformance)
    input="${1:-$ROOT_DIR/spec/conformance.fl}"
    output="${2:-$(basename "${input%.fl}")}"
    echo "🔎 verify-only (conformance)"
    echo "   입력: $input"
    echo "   출력: $output"
    echo ""
    run_check "$input" "$output"
    echo ""
    echo "📋 규칙 검사: $ROOT_DIR/spec/fx.rules.conformance"
    echo ""
    run_rules "$ROOT_DIR/spec/fx.rules.conformance" "$input"
    echo ""
    run_graph "$input" "$output"
    ;;

  local)
    input="${1:-}"
    if [ -z "$input" ]; then
      echo "사용법: tools/fl-verify.sh local <input.fl>" >&2
      exit 1
    fi
    output="${2:-$(basename "${input%.fl}")}"
    rules="$ROOT_DIR/spec/fx.rules.narrow2"
    echo "🔎 verify-only (local)"
    echo "   입력: $input"
    echo "   출력: $output"
    echo ""
    run_check "$input" "$output"
    echo ""
    if [ -f "$rules" ]; then
      echo "📋 규칙 검사: $rules"
      echo ""
      run_rules "$rules" "$input"
      echo ""
    fi
    run_graph "$input" "$output"
    ;;

  *)
    echo "사용법:" >&2
    echo "  tools/fl-verify.sh official <input.fl>" >&2
    echo "  tools/fl-verify.sh conformance [input.fl]" >&2
    echo "  tools/fl-verify.sh local <input.fl>" >&2
    exit 1
    ;;
esac

