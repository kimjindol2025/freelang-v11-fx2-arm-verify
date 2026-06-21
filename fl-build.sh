#!/bin/bash
# fl-build — FreeLang 네이티브 빌드 스크립트
# 사용법: fl-build.sh <input.fl> [output-binary] [--no-net] [--plan]
# 결과:   Node.js 없는 단일 ELF 바이너리

set -e

# 심링크 지원: 실제 스크립트 위치 해석
SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
CGC_BIN="${CGC_BIN:-/home/kimjin/freelang-v11/bin/cgc-bin}"

# 플래그 파싱
NO_NET=0
PLAN_MODE=0
ARGS=()
for arg in "$@"; do
  if [ "$arg" = "--no-net" ]; then NO_NET=1;
  elif [ "$arg" = "--plan" ]; then PLAN_MODE=1;
  elif [ "$arg" = "--check" ]; then PLAN_MODE=check;
  elif [ "$arg" = "--graph" ]; then PLAN_MODE=graph;
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
if [ "$PLAN_MODE" = "1" ] || [ "$PLAN_MODE" = "check" ] || [ "$PLAN_MODE" = "graph" ]; then
  if [ "$PLAN_MODE" = "1" ]; then echo "📋 fl build --plan"; fi
  if [ "$PLAN_MODE" = "check" ]; then echo ""; fi
  if [ "$PLAN_MODE" = "graph" ]; then echo "📊 fl build --graph"; echo ""; fi
  MODE_ARG="plan"
  [ "$PLAN_MODE" = "check" ] && MODE_ARG="check"
  [ "$PLAN_MODE" = "graph" ] && MODE_ARG="graph"
  python3 "$SCRIPT_DIR/fl-build-plan.py" "$FL_INPUT" "$SCRIPT_DIR" "$OUTPUT" "$MODE_ARG"
  exit $?
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

def inline_loads(path, visited=None, cycle_stack=None):
    is_root = visited is None
    if visited is None:
        visited = set()
    if cycle_stack is None:
        cycle_stack = []
    abs_path = os.path.abspath(path)
    if abs_path in cycle_stack:
        print(f"[fl-build] \u26a0\ufe0f  순환 의존 감지: {path}", file=sys.stderr)
        return "; [fl-build] CYCLE DETECTED: " + path
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
            result.append(inline_loads(load_path, visited, cycle_stack + [abs_path]))
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

# 긴 문자열 자동 분할 (>900B)
python3 "$SCRIPT_DIR/fl-str-split.py" "$PREPROCESSED" 2>&1 || true

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

# ─── 3.5. gcc -fsyntax-only: C 타입 사전 검사 (0.3초, 바이너리 없음) ───
SYNTAX_LOG="/tmp/fl_syntax_$$.log"
echo "🔍 C 타입 사전 검사..."
if gcc -fsyntax-only -I "$RUNTIME_DIR" "$C_FILE" -w 2>"$SYNTAX_LOG"; then
  rm -f "$SYNTAX_LOG"
  echo "   ✅ 타입 OK"
else
  echo "❌ C 타입 오류 (풀 빌드 전 조기 감지):"
  # #line N "<fl>" 디렉티브 → FL 원본 라인 역추적
  python3 - "$SYNTAX_LOG" "$PREPROCESSED" << 'PYEOF'
import sys, re

syntax_log = open(sys.argv[1]).read()
fl_lines   = open(sys.argv[2]).read().splitlines()
errors     = re.findall(r'<fl>:(\d+):\d+: error: (.+)', syntax_log)

def c_to_fl(name):
    n = name.replace('_', '-')
    if n.startswith('fl-'): n = n[3:]
    return n

seen = set()
for lineno_s, msg in errors:
    lineno = int(lineno_s) - 1
    fl_line = fl_lines[lineno].strip() if lineno < len(fl_lines) else ''
    key = (lineno, msg[:60])
    if key in seen: continue
    seen.add(key)

    print(f"  📍 FL 줄 {lineno+1}: {msg}")
    if fl_line: print(f"     코드: {fl_line[:120]}")

    # "called object X is not a function"
    m = re.search(r"called object .([^''']+).", msg)
    if m:
        fn = m.group(1)
        print(f"     💡 {fn} 는 함수가 아닙니다 → (fxb-{c_to_fl(fn)}) 로 교체하세요")

    # "too few/many arguments"
    m = re.search(r"too (few|many) arguments to function .([^''']+).", msg)
    if m:
        direction, fn = m.group(1), m.group(2)
        print(f"     💡 ({c_to_fl(fn)}) 인자 수 {'부족' if direction=='few' else '초과'} — 함수 정의 확인")

    # "undeclared"
    m = re.search(r".([^''']+). undeclared", msg)
    if m:
        sym = m.group(1)
        print(f"     💡 {sym} 미선언 → runtime.h 확인 또는 (fxb-{c_to_fl(sym)}) 패턴 사용")

    # "incompatible type"
    if 'incompatible type' in msg:
        print(f"     💡 타입 불일치 — FLValue 필요 위치에 int/char* 전달 여부 확인")
        print(f"        (없는 함수 호출 시 GCC가 int 반환으로 추론 → 이 에러 발생)")

    print()
PYEOF
  rm -f "$SYNTAX_LOG" "$C_FILE" "$PREPROCESSED"
  exit 1
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

CORE_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/math.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/error.c $NET_SRCS $RUNTIME_DIR/aliases.c \
  $RUNTIME_DIR/sqlite.c $RUNTIME_DIR/debug.c $RUNTIME_DIR/gc.c \
  $RUNTIME_DIR/jit.c $RUNTIME_DIR/fx-builtin-shim.c \
  $RUNTIME_DIR/regex.c $RUNTIME_DIR/smtp.c $RUNTIME_DIR/pdf_ttf.c $RUNTIME_DIR/pdf_img.c \
  $RUNTIME_DIR/builtins-shim.c"

if [ -f "$RUNTIME_DIR/mariadb.c" ]; then
  CORE_SRCS="$CORE_SRCS $RUNTIME_DIR/mariadb.c"
  echo "   + MariaDB dlopen 바인딩 포함"
fi

# ── libfx.a 캐시 (런타임 변경 시에만 재빌드) ─────────────────────
LIBFX_A="$RUNTIME_DIR/libfx.a"
LIBFX_HASH_FILE="$RUNTIME_DIR/.libfx_hash"
CURRENT_HASH=$(md5sum $CORE_SRCS "$RUNTIME_DIR/runtime.h" 2>/dev/null | md5sum | cut -d' ' -f1)

if [ ! -f "$LIBFX_A" ] || [ "$(cat "$LIBFX_HASH_FILE" 2>/dev/null)" != "${CURRENT_HASH}_${NO_NET}" ]; then
  echo "   📦 libfx.a 빌드 중... (런타임 변경 감지)"
  OBJ_DIR="/tmp/fl_runtime_objs_$$"
  mkdir -p "$OBJ_DIR"
  for src in $CORE_SRCS; do
    obj="$OBJ_DIR/$(basename "$src" .c).o"
    gcc -O2 -c "$src" -I "$RUNTIME_DIR" $EXTRA_CFLAGS -w -o "$obj" &
  done
  wait
  ar rcs "$LIBFX_A" "$OBJ_DIR"/*.o
  rm -rf "$OBJ_DIR"
  echo "${CURRENT_HASH}_${NO_NET}" > "$LIBFX_HASH_FILE"
  echo "   ✅ libfx.a 완성 (이후 빌드는 재사용)"
else
  echo "   ✅ libfx.a 캐시 사용"
fi

# user-fns.c — 패키지 추가 시 변경되므로 항상 재컴파일
USER_SRCS=""
if [ -f "$RUNTIME_DIR/user-fns.c" ]; then
  USER_SRCS="$RUNTIME_DIR/user-fns.c"
fi

GCC_LOG="/tmp/fl_gcc_$$.log"
if gcc -O2 -Werror=implicit-function-declaration -o "$OUTPUT" $C_FILE $USER_SRCS "$LIBFX_A" \
  -I "$RUNTIME_DIR" $EXTRA_CFLAGS \
  -rdynamic -lpthread -lm -ldl -lsqlite3 $NET_LIBS \
  -w 2>"$GCC_LOG"; then
  rm -f "$GCC_LOG"
else
  echo "❌ gcc 컴파일 실패:"
  python3 - "$GCC_LOG" "$PREPROCESSED" << 'PYEOF'
import sys, re

gcc_log   = open(sys.argv[1]).read()
fl_lines  = open(sys.argv[2]).read().splitlines() if len(sys.argv) > 2 else []

# ── FL 줄 역추적 패턴 ────────────────────────────────────────
fl_errors   = re.findall(r'<fl>:(\d+):\d+: (?:error|warning): (.+)', gcc_log)
# ── 링커 에러 ────────────────────────────────────────────────
undef_refs  = re.findall(r"undefined reference to `([^']+)'", gcc_log)
# ── implicit declaration ──────────────────────────────────────
implicit    = re.findall(r"implicit declaration of function .([^''']+).", gcc_log)

def c_to_fl(name):
    """C 함수명 → FL 힌트 (fl_some_fn → some-fn, fxb_sqlite_query → fxb-sqlite-query)"""
    n = name.replace('_', '-')
    if n.startswith('fl-'): n = n[3:]
    return n

seen = set()

# 1) FL 줄 추적 가능한 에러
for lineno_s, msg in fl_errors:
    lineno = int(lineno_s) - 1
    fl_line = fl_lines[lineno].strip() if lineno < len(fl_lines) else ''
    key = (lineno, msg[:60])
    if key in seen: continue
    seen.add(key)

    print(f"\n  📍 FL 줄 {lineno+1}: {msg}")
    if fl_line:
        print(f"     코드: {fl_line[:120]}")

    # 패턴별 힌트
    # "called object X is not a function"
    m = re.search(r"called object .([^''']+).", msg)
    if m:
        fn = m.group(1)
        print(f"     💡 {fn} 는 함수가 아닙니다 → (fxb-{c_to_fl(fn)}) 로 교체하세요")

    # "too few/many arguments"
    m = re.search(r"too (few|many) arguments to function .([^''']+).", msg)
    if m:
        direction, fn = m.group(1), m.group(2)
        print(f"     💡 ({c_to_fl(fn)}) 인자 수 {'부족' if direction=='few' else '초과'} — 함수 정의 확인")

    # "incompatible type"
    if 'incompatible type' in msg:
        print(f"     💡 타입 불일치 — FLValue 가 필요한 곳에 int/char* 전달 여부 확인")

    # "undeclared"
    m = re.search(r".([^''']+). undeclared", msg)
    if m:
        sym = m.group(1)
        fl_name = c_to_fl(sym)
        print(f"     💡 {sym} 미선언 → runtime.h 확인 또는 (fxb-{fl_name}) 패턴 사용")

# 2) 링커 에러 (undefined reference)
if undef_refs:
    print(f"\n  🔗 링커 에러 (undefined reference):")
    seen_undef = set()
    for sym in undef_refs:
        if sym in seen_undef: continue
        seen_undef.add(sym)
        fl_name = c_to_fl(sym)
        if sym.startswith('fxb_sqlite'):
            print(f"     ❌ {sym}")
            print(f"        → SQLite 함수: (fxb-{fl_name.lstrip('fxb-')}) 패턴 사용")
        elif sym.startswith('fl_') or sym.startswith('server_') or sym.startswith('fl'):
            print(f"     ❌ {sym}")
            print(f"        → runtime.h 에 선언 누락 또는 소스 파일 누락")
        else:
            print(f"     ❌ {sym}")
            print(f"        → 외부 라이브러리 누락 (-l 링크 옵션 확인)")

# 3) implicit declaration
if implicit:
    print(f"\n  ⚠️  암묵적 선언 (runtime.h 누락):")
    for fn in set(implicit):
        print(f"     {fn} → runtime.h 에 FLValue {fn}(FLValue); 추가 필요")

if not fl_errors and not undef_refs and not implicit:
    print("\n  (원본 GCC 로그:)")
    print(gcc_log[:2000])
PYEOF
  rm -f "$C_FILE" "$PREPROCESSED" "$GCC_LOG"
  exit 1
fi

# ─── 5. 정리 ─────────────────────────────────────────────────────
rm -f "$C_FILE" "$PREPROCESSED"

echo ""
echo "✅ 빌드 완료: ./$OUTPUT"
echo "   크기: $(du -sh "$OUTPUT" | cut -f1)"
echo "   실행: ./$OUTPUT"
