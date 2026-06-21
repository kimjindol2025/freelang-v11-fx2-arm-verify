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

# ─── 2.5. 시맨틱 분석 (Semi-static Linter) ─────────────────────────
echo "🧠 시맨틱 분석..."
SEMANTIC_ERRORS=0
python3 - "$PREPROCESSED" << 'PYEOF'
import sys, re

src = open(sys.argv[1]).read()
lines = src.splitlines()
errors = []
warnings = []

# ── 헬퍼: 라인 번호 찾기 ─────────────────────────────────────────
def find_line(pattern, start=0):
    for i, l in enumerate(lines[start:], start+1):
        if re.search(pattern, l):
            return i
    return 0

# ── 1. SQL ? 개수 vs 인자 개수 불일치 ───────────────────────────
# 패턴: (fxb-sqlite-*-p DB "sql..." [...]) 또는 (fxb-sqlite-exec-p DB "sql..." [...])
sql_pat = re.compile(
    r'\(fxb-sqlite-(?:query|exec)-p\s+\S+\s+"([^"]+)"\s+(\[.*?\])',
    re.DOTALL
)
for m in sql_pat.finditer(src):
    sql_str = m.group(1)
    args_str = m.group(2)
    q_count = sql_str.count('?')
    # 배열 안 원소 수: $var 또는 "str" 또는 숫자
    arg_items = re.findall(r'\$\w+|"[^"]*"|\d+', args_str)
    arg_count = len(arg_items)
    if q_count != arg_count:
        line_no = src[:m.start()].count('\n') + 1
        errors.append(
            f"  📍 줄 {line_no}: SQL ? 개수({q_count}) ≠ 인자 개수({arg_count})\n"
            f"     SQL: {sql_str[:60]}{'...' if len(sql_str)>60 else ''}\n"
            f"     💡 배열 원소를 {q_count}개로 맞추세요"
        )

# ── 2. fxb-sqlite-query (비-p 버전)에 ? 포함 → SQL 인젝션 위험 ──
unsafe_pat = re.compile(r'\(fxb-sqlite-query\s+\S+\s+"([^"]*\?[^"]*)"')
for m in unsafe_pat.finditer(src):
    line_no = src[:m.start()].count('\n') + 1
    warnings.append(
        f"  ⚠️  줄 {line_no}: fxb-sqlite-query 에 ? 포함 → SQL 인젝션 위험\n"
        f"     💡 (fxb-sqlite-query-p DB sql [param ...]) 로 교체하세요"
    )

# ── 3. $var 정의 전 사용 (단순 스코프 분석) ──────────────────────
# defn 파라미터와 let 바인딩 수집, 사용 시점과 비교
defined = set()
# 함수 파라미터 ($x $y ...)
for m in re.finditer(r'\(defn\s+\S+\s+\[([^\]]*)\]', src):
    for p in re.findall(r'\$(\w+)', m.group(1)):
        defined.add(p)
# let 바인딩 [$var expr]
for m in re.finditer(r'\(let\s+\[([^\]]+)\]', src, re.DOTALL):
    for p in re.findall(r'\$(\w+)', m.group(1).split(']')[0]):
        defined.add(p)
# fn 파라미터
for m in re.finditer(r'\(fn\s+\[([^\]]*)\]', src):
    for p in re.findall(r'\$(\w+)', m.group(1)):
        defined.add(p)

# ── 4. define 없는 전역 사용 감지 (보수적으로) ──────────────────
defines = set(re.findall(r'\(define\s+(\$\w+)', src))
# 전역에서 $로 시작하는 사용 중 define 없는 것 (false positive 많아 경고만)
# → 이건 스킵 (CGC가 더 잘 잡음)

# ── 5. (or nil "fallback") 패턴 — nil 전파 경고 ─────────────────
# nil이 첫 인자로 직접 오면 의미 없음
nil_or_pat = re.compile(r'\(or\s+nil\s+')
for m in nil_or_pat.finditer(src):
    line_no = src[:m.start()].count('\n') + 1
    warnings.append(
        f"  ⚠️  줄 {line_no}: (or nil ...) — 첫 인자가 nil 리터럴\n"
        f"     💡 (or $var fallback) 패턴을 확인하세요"
    )

# ── 출력 ─────────────────────────────────────────────────────────
has_error = len(errors) > 0

if errors:
    print(f"❌ 시맨틱 오류 {len(errors)}개:")
    for e in errors:
        print(e)
if warnings:
    print(f"⚠️  시맨틱 경고 {len(warnings)}개:")
    for w in warnings:
        print(w)
if not errors and not warnings:
    print("   ✅ 시맨틱 OK")

sys.exit(1 if has_error else 0)
PYEOF

SEMANTIC_EXIT=$?
if [ "$SEMANTIC_EXIT" -ne 0 ]; then
  echo "❌ 시맨틱 분석 실패 — 컴파일 중단"
  rm -f "$PREPROCESSED"
  exit 1
fi

# ─── 2.6. 프로젝트 커스텀 규칙 (fx.rules) ──────────────────────────
FL_RULES="$(dirname "$FL_INPUT")/fx.rules"
if [ ! -f "$FL_RULES" ]; then
  FL_RULES="$(pwd)/fx.rules"
fi

if [ -f "$FL_RULES" ]; then
  echo "📋 프로젝트 규칙 검사 ($FL_RULES)..."
  RULES_OUT=$(node /home/kimjin/freelang-v11/bootstrap.js run \
    "$SCRIPT_DIR/fl-rules-runner.fl" \
    "$FL_RULES" "$PREPROCESSED" 2>&1 | grep -E "^(error|warn):")

  RULES_ERRORS=0
  RULES_WARNS=0
  while IFS= read -r line; do
    [ -z "$line" ] && continue
    LEVEL="${line%%:*}"
    REST="${line#*:}"
    DESC="${REST%%:*}"
    MSG="${REST#*:}"
    if [ "$LEVEL" = "error" ]; then
      echo "  ❌ [규칙] $DESC"
      echo "     $MSG"
      RULES_ERRORS=$((RULES_ERRORS + 1))
    elif [ "$LEVEL" = "warn" ]; then
      echo "  ⚠️  [규칙] $DESC"
      echo "     $MSG"
      RULES_WARNS=$((RULES_WARNS + 1))
    fi
  done <<< "$RULES_OUT"

  if [ "$RULES_ERRORS" -gt 0 ]; then
    echo "❌ 프로젝트 규칙 위반 ${RULES_ERRORS}개 — 컴파일 중단"
    rm -f "$PREPROCESSED"
    exit 1
  fi
  [ "$RULES_WARNS" -eq 0 ] && [ -z "$RULES_OUT" ] && echo "   ✅ 규칙 OK"
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

# ── app.o 캐시 (FL 소스 변경 시에만 재컴파일) ─────────────────────
APP_OBJ_DIR="/tmp/fl_app_cache"
mkdir -p "$APP_OBJ_DIR"
OUTPUT_BASE="$(basename "$OUTPUT")"
APP_O="$APP_OBJ_DIR/${OUTPUT_BASE}.o"
APP_HASH_FILE="$APP_OBJ_DIR/${OUTPUT_BASE}.hash"
APP_HASH=$(md5sum "$PREPROCESSED" 2>/dev/null | cut -d' ' -f1)

if [ ! -f "$APP_O" ] || [ "$(cat "$APP_HASH_FILE" 2>/dev/null)" != "${APP_HASH}_${NO_NET}" ]; then
  echo "   🔧 app.o 컴파일 중..."
  if gcc -O2 -c -Werror=implicit-function-declaration "$C_FILE" \
    -I "$RUNTIME_DIR" $EXTRA_CFLAGS -w -o "$APP_O" 2>/tmp/fl_app_gcc_$$.log; then
    echo "${APP_HASH}_${NO_NET}" > "$APP_HASH_FILE"
    echo "   ✅ app.o 완성 (이후 빌드는 재사용)"
  else
    # 컴파일 실패 → 캐시된 .o 삭제 후 에러 출력 (Stage 4 파서로 위임)
    rm -f "$APP_O" "$APP_HASH_FILE"
    cat /tmp/fl_app_gcc_$$.log >> /tmp/fl_gcc_placeholder_$$.log 2>/dev/null || true
    mv /tmp/fl_app_gcc_$$.log /tmp/fl_gcc_placeholder_$$.log
  fi
  rm -f /tmp/fl_app_gcc_$$.log 2>/dev/null || true
else
  echo "   ✅ app.o 캐시 사용"
fi

GCC_LOG="/tmp/fl_gcc_$$.log"
if gcc -o "$OUTPUT" "$APP_O" $USER_SRCS "$LIBFX_A" \
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
