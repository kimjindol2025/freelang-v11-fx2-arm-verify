#!/bin/bash
# fl-build — FreeLang 네이티브 빌드 스크립트
# 사용법: fl-build.sh <input.fl> [output-binary] [--no-net] [--plan]
# 결과:   Node.js 없는 단일 ELF 바이너리

set -e

# 심링크 지원: 실제 스크립트 위치 해석
SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
CGC_BIN="${CGC_BIN:-/root/freelang-v11/bin/cgc-bin}"

# 플래그 파싱
NO_NET=0
PLAN_MODE=0
ARGS=()
for arg in "$@"; do
  if [ "$arg" = "--no-net" ]; then NO_NET=1;
  elif [ "$arg" = "--plan" ]; then PLAN_MODE=1;
  else ARGS+=("$arg"); fi
done

FL_INPUT="${ARGS[0]}"
if [ -z "$FL_INPUT" ]; then
  echo "사용법: $0 <input.fl> [output-name] [--no-net] [--plan]"
  exit 1
fi

FL_BASE="$(basename "$FL_INPUT" .fl)"
OUTPUT="${ARGS[1]:-$FL_BASE}"
C_FILE="/tmp/fl_build_$$.c"

# ─── --plan: 선언 정보 출력 후 종료 (빌드 없이) ──────────────────
if [ "$PLAN_MODE" = "1" ]; then
  echo "📋 fl build --plan"
  echo ""
  python3 "$SCRIPT_DIR/fl-build-plan.py" "$FL_INPUT" "$SCRIPT_DIR" "$OUTPUT"
  exit 0
fi

echo "🔨 FreeLang 네이티브 빌드"
echo "   입력: $FL_INPUT"
echo "   출력: $OUTPUT"
echo ""

# ─── 0. module declaration 처리 ──────────────────────────────────
MODULE_JSON=$(python3 "$SCRIPT_DIR/fl-module-parse.py" "$FL_INPUT" 2>/dev/null || true)
if [ -n "$MODULE_JSON" ]; then
  MODULE_NAME=$(echo "$MODULE_JSON" | python3 -c "import json,sys; print(json.load(sys.stdin).get('name',''))")
  USE_PROFILES=$(echo "$MODULE_JSON" | python3 -c "import json,sys; d=json.load(sys.stdin); print(' '.join(d.get('use', [])))")
  MODULE_VER=$(echo "$MODULE_JSON" | python3 -c "import json,sys; print(json.load(sys.stdin).get('version',''))")

  echo "📦 module: $MODULE_NAME v$MODULE_VER"
  if [ -n "$USE_PROFILES" ]; then
    echo "   :use → $USE_PROFILES"
    echo "   의존성 해결 중..."
    python3 "$SCRIPT_DIR/fl-resolve-deps.py" $USE_PROFILES "$SCRIPT_DIR"
    echo ""
  fi
fi

# ─── 1. load 인라인 전처리 ───────────────────────────────────────
# (load "path.fl") 를 파일 내용으로 인라인 치환
PREPROCESSED="/tmp/fl_preprocessed_$$.fl"

python3 << PYEOF
import re, os, sys

def strip_module_form(src):
    """(module ...) 블록 제거 — cgc-bin은 module form 미지원"""
    start = src.find('(module')
    if start == -1:
        return src
    depth = 0
    end = start
    for i, ch in enumerate(src[start:], start):
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth -= 1
            if depth == 0:
                end = i
                break
    return src[:start] + "; [module form stripped by fl-build]\n" + src[end+1:]

def inline_loads(path, visited=None):
    is_root = visited is None
    if visited is None:
        visited = set()
    abs_path = os.path.abspath(path)
    if abs_path in visited:
        return ""
    visited.add(abs_path)
    base_dir = os.path.dirname(abs_path)
    try:
        content = open(abs_path).read()
    except:
        print(f"; [fl-build] 경고: {path} 읽기 실패", file=sys.stderr)
        return ""
    if is_root:
        content = strip_module_form(content)
    result = []
    for line in content.splitlines():
        m = re.match(r'\s*\(load\s+"([^"]+)"\)', line) or \
            re.match(r"\s*\(load\s+'([^']+)'\)", line)
        if m:
            load_path = m.group(1)
            if not os.path.isabs(load_path):
                load_path = os.path.join(base_dir, load_path)
            print(f"; [fl-build] 인라인: {load_path}", file=sys.stderr)
            result.append(f"; --- inlined: {load_path} ---")
            result.append(inline_loads(load_path, visited))
            result.append(f"; --- end inlined: {load_path} ---")
        else:
            result.append(line)
    return "\n".join(result)

output = inline_loads("$FL_INPUT")

lines = output.splitlines()
cleaned = [l for l in lines if not re.match(r'\s*\(println\s+"?\[DEBUG\]', l)]
output = "\n".join(cleaned)

open("$PREPROCESSED", "w").write(output)
print(f"[fl-build] 전처리 완료", file=sys.stderr)
PYEOF

# ─── 2. 사전 검사 ────────────────────────────────────────────────
CHECK_PARENS="/home/kimjin/freelang-v11/scripts/check-parens.py"
if [ -f "$CHECK_PARENS" ]; then
  echo "🔍 괄호/브래킷 검사..."
  if ! python3 "$CHECK_PARENS" "$PREPROCESSED" 2>&1; then
    echo "❌ 사전 검사 실패 — 컴파일 중단"
    rm -f "$PREPROCESSED"
    exit 1
  fi
fi

# ─── 3. cgc-bin: FL → C ──────────────────────────────────────────
echo "⚙️  FL → C 컴파일..."
COMPILE_OUT=$("$CGC_BIN" "$PREPROCESSED" "$C_FILE" 2>&1)
echo "$COMPILE_OUT" | grep -v "^$" || true

WARN_COUNT=$(echo "$COMPILE_OUT" | grep -c "\[W[123]\]" || true)
if [ "$WARN_COUNT" -gt 0 ]; then
  echo ""
  echo "⚠️  컴파일 경고 ${WARN_COUNT}개:"
  echo "$COMPILE_OUT" | grep "\[W[123]\]"
  echo ""
fi

# ─── 4. gcc: C → ELF ─────────────────────────────────────────────
echo "⚙️  C → 바이너리 컴파일..."
if [ "$NO_NET" = "1" ]; then
  echo "   + --no-net: HTTP/WS/crypto stub 사용 (openssl/curl 불필요)"
  NET_SRCS="$RUNTIME_DIR/http-stub.c $RUNTIME_DIR/websocket-stub.c $RUNTIME_DIR/http_client-stub.c $RUNTIME_DIR/crypto-stub.c $RUNTIME_DIR/sse.c"
  NET_LIBS=""
  EXTRA_CFLAGS="-DFL_NO_CRYPTO"
else
  NET_SRCS="$RUNTIME_DIR/http.c $RUNTIME_DIR/websocket.c $RUNTIME_DIR/http_client.c $RUNTIME_DIR/sse.c"
  NET_LIBS="-lssl -lcrypto -lcurl"
  EXTRA_CFLAGS=""
fi

RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/math.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/error.c $NET_SRCS $RUNTIME_DIR/aliases.c \
  $RUNTIME_DIR/sqlite.c $RUNTIME_DIR/debug.c $RUNTIME_DIR/gc.c \
  $RUNTIME_DIR/jit.c $RUNTIME_DIR/fx-builtin-shim.c \
  $RUNTIME_DIR/regex.c $RUNTIME_DIR/smtp.c $RUNTIME_DIR/pdf_ttf.c $RUNTIME_DIR/pdf_img.c \
  $RUNTIME_DIR/builtins-shim.c"

if [ -f "$RUNTIME_DIR/mariadb.c" ]; then
  RUNTIME_SRCS="$RUNTIME_SRCS $RUNTIME_DIR/mariadb.c"
  echo "   + MariaDB dlopen 바인딩 포함"
fi

if [ -f "$RUNTIME_DIR/user-fns.c" ]; then
  RUNTIME_SRCS="$RUNTIME_SRCS $RUNTIME_DIR/user-fns.c"
fi

GCC_LOG="/tmp/fl_gcc_$$.log"
if gcc -O2 -Werror=implicit-function-declaration -o "$OUTPUT" $C_FILE $RUNTIME_SRCS \
  -I "$RUNTIME_DIR" $EXTRA_CFLAGS \
  -rdynamic -lpthread -lm -ldl -lsqlite3 $NET_LIBS \
  -w 2>"$GCC_LOG"; then
  rm -f "$GCC_LOG"
else
  echo "❌ gcc 컴파일 실패:"
  cat "$GCC_LOG"
  rm -f "$C_FILE" "$PREPROCESSED" "$GCC_LOG"
  exit 1
fi

# ─── 5. 정리 ─────────────────────────────────────────────────────
rm -f "$C_FILE" "$PREPROCESSED"

echo ""
echo "✅ 빌드 완료: ./$OUTPUT"
echo "   크기: $(du -sh "$OUTPUT" | cut -f1)"
echo "   실행: ./$OUTPUT"
