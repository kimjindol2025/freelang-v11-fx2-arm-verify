#!/bin/bash
# fl-build — FreeLang 네이티브 빌드 스크립트
# 사용법: fl-build.sh <input.fl> [output-binary] [--no-net] [--plan]
# 결과:   Node.js 없는 단일 ELF 바이너리

set -e

# 심링크 지원: 실제 스크립트 위치 해석
SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
FL_STR_SPLIT_SRC="$SCRIPT_DIR/fl-str-split.c"
FL_STR_SPLIT_BIN="$SCRIPT_DIR/.fl-str-split"
FL_MODULE_PARSE_SRC="$SCRIPT_DIR/fl-module-parse.c"
FL_MODULE_PARSE_BIN="$SCRIPT_DIR/.fl-module-parse"
FL_BUILD_PLAN_SRC="$SCRIPT_DIR/fl-build-plan.c"
FL_BUILD_PLAN_BIN="$SCRIPT_DIR/.fl-build-plan"
FL_RESOLVE_DEPS_SRC="$SCRIPT_DIR/fl-resolve-deps-profiles.c"
FL_RESOLVE_DEPS_BIN="$SCRIPT_DIR/.fl-resolve-deps-profiles"
FL_USER_FNS_GEN_SRC="$SCRIPT_DIR/fl-generate-user-fns.c"
FL_USER_FNS_GEN_BIN="$SCRIPT_DIR/.fl-generate-user-fns"
FL_BUILD_HELPER_SRC="$SCRIPT_DIR/fl-build-helper.c"
FL_BUILD_HELPER_BIN="$SCRIPT_DIR/.fl-build-helper"
if [ -f "$SCRIPT_DIR/tools/fl-rules-native-runner.sh" ]; then
  source "$SCRIPT_DIR/tools/fl-rules-native-runner.sh"
fi

build_fl_str_split() {
  if [ -x "$FL_STR_SPLIT_BIN" ] && [ "$FL_STR_SPLIT_BIN" -nt "$FL_STR_SPLIT_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_STR_SPLIT_SRC" -o "$FL_STR_SPLIT_BIN" 2>/tmp/fl-str-split-build.log; then
    echo "[fl-build] fl-str-split.c build failed" >&2
    cat /tmp/fl-str-split-build.log >&2
    return 1
  fi
}

build_fl_module_parse() {
  if [ -x "$FL_MODULE_PARSE_BIN" ] && [ "$FL_MODULE_PARSE_BIN" -nt "$FL_MODULE_PARSE_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_MODULE_PARSE_SRC" -o "$FL_MODULE_PARSE_BIN" 2>/tmp/fl-module-parse-build.log; then
    echo "[fl-build] fl-module-parse.c build failed" >&2
    cat /tmp/fl-module-parse-build.log >&2
    return 1
  fi
}

build_fl_build_plan() {
  if [ -x "$FL_BUILD_PLAN_BIN" ] && [ "$FL_BUILD_PLAN_BIN" -nt "$FL_BUILD_PLAN_SRC" ]; then
    build_fl_module_parse
    return $?
  fi

  if ! build_fl_module_parse; then
    return 1
  fi

  if ! cc -O2 -std=c99 "$FL_BUILD_PLAN_SRC" -o "$FL_BUILD_PLAN_BIN" 2>/tmp/fl-build-plan-build.log; then
    echo "[fl-build] fl-build-plan.c build failed" >&2
    cat /tmp/fl-build-plan-build.log >&2
    return 1
  fi
}

run_fl_str_split() {
  local input_file="$1"
  if build_fl_str_split && "$FL_STR_SPLIT_BIN" "$input_file"; then
    return 0
  fi

  echo "[fl-build] fl-str-split skipped (non-fatal)" >&2
  return 1
}

build_fl_module_parse() {
  if [ -x "$FL_MODULE_PARSE_BIN" ] && [ "$FL_MODULE_PARSE_BIN" -nt "$FL_MODULE_PARSE_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_MODULE_PARSE_SRC" -o "$FL_MODULE_PARSE_BIN" 2>/tmp/fl-module-parse-build.log; then
    echo "[fl-build] fl-module-parse.c build failed" >&2
    cat /tmp/fl-module-parse-build.log >&2
    return 1
  fi
}

run_fl_module_parse() {
  local input_file="$1"
  if build_fl_module_parse && "$FL_MODULE_PARSE_BIN" "$input_file"; then
    return 0
  fi

  echo "[fl-build] fl-module-parse skipped (non-fatal)" >&2
  return 1
}

build_fl_resolve_deps_profiles() {
  if [ -x "$FL_RESOLVE_DEPS_BIN" ] && [ "$FL_RESOLVE_DEPS_BIN" -nt "$FL_RESOLVE_DEPS_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_RESOLVE_DEPS_SRC" -o "$FL_RESOLVE_DEPS_BIN" 2>/tmp/fl-resolve-deps-build.log; then
    echo "[fl-build] fl-resolve-deps-profiles.c build failed" >&2
    cat /tmp/fl-resolve-deps-build.log >&2
    return 1
  fi
}

run_fl_resolve_deps_profiles() {
  local -a profile_names=("$@")
  if build_fl_resolve_deps_profiles && "$FL_RESOLVE_DEPS_BIN" "$SCRIPT_DIR" "${profile_names[@]}"; then
    return 0
  fi

  echo "[fl-build] fl-resolve-deps 단계 실패" >&2
  return 1
}

run_fl_resolve_deps_metadata() {
  local output_file="$1"
  shift
  local -a package_names=("$@")
  if build_fl_resolve_deps_profiles && "$FL_RESOLVE_DEPS_BIN" --metadata "$SCRIPT_DIR" "${package_names[@]}" > "$output_file"; then
    return 0
  fi

  echo "[fl-build] fl-resolve-deps metadata 단계 실패" >&2
  return 1
}

build_fl_user_fns_gen() {
  if [ -x "$FL_USER_FNS_GEN_BIN" ] && [ "$FL_USER_FNS_GEN_BIN" -nt "$FL_USER_FNS_GEN_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_USER_FNS_GEN_SRC" -o "$FL_USER_FNS_GEN_BIN" 2>/tmp/fl-generate-user-fns-build.log; then
    echo "[fl-build] fl-generate-user-fns.c build failed" >&2
    cat /tmp/fl-generate-user-fns-build.log >&2
    return 1
  fi
}

run_fl_generate_user_fns() {
  local metadata_file="$1"
  if build_fl_user_fns_gen && "$FL_USER_FNS_GEN_BIN" "$SCRIPT_DIR" "$metadata_file"; then
    return 0
  fi

  echo "[fl-build] user-fns 생성 단계 실패" >&2
  return 1
}

build_fl_build_helper() {
  if [ -x "$FL_BUILD_HELPER_BIN" ] && [ "$FL_BUILD_HELPER_BIN" -nt "$FL_BUILD_HELPER_SRC" ]; then
    return 0
  fi

  if ! cc -O2 -std=c99 "$FL_BUILD_HELPER_SRC" -o "$FL_BUILD_HELPER_BIN" 2>/tmp/fl-build-helper-build.log; then
    echo "[fl-build] fl-build-helper.c build failed" >&2
    cat /tmp/fl-build-helper-build.log >&2
    return 1
  fi
}

run_fl_build_helper() {
  if ! build_fl_build_helper; then
    return 1
  fi
  "$FL_BUILD_HELPER_BIN" "$@"
}

pick_cgc_bin() {

  if [ -n "$CGC_BIN" ] && [ -x "$CGC_BIN" ]; then
    echo "$CGC_BIN"
    return 0
  fi

  case "$(uname -m)" in
    aarch64|arm64)
      for candidate in         /root/freelang-v11/bin/cgc-bin         /root/freelang-v11/bin/cgc-bin.bak         /root/freelang-v11/bin/cgc-bin-x86_64-backup         /home/kimjin/freelang-v11/bin/cgc-bin
      do
        if [ -x "$candidate" ]; then
          echo "$candidate"
          return 0
        fi
      done
      ;;
    *)
      for candidate in         /root/freelang-v11/bin/cgc-bin         /root/freelang-v11/bin/cgc-bin.bak         /home/kimjin/freelang-v11/bin/cgc-bin         /root/freelang-v11/bin/cgc-bin-x86_64-backup
      do
        if [ -x "$candidate" ]; then
          echo "$candidate"
          return 0
        fi
      done
      ;;
  esac

  return 1
}

CGC_BIN="$(pick_cgc_bin || true)"
if [ -z "$CGC_BIN" ]; then
  echo "❌ cgc-bin 실행 파일을 찾지 못했습니다."
  echo "   /root/freelang-v11/bin/cgc-bin.bak 같은 arm64 빌드나"
  echo "   환경변수 CGC_BIN을 지정해서 다시 시도하세요."
  exit 1
fi

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
  build_fl_build_plan || exit 1
  "$FL_BUILD_PLAN_BIN" "$FL_INPUT" "$SCRIPT_DIR" "$OUTPUT" "$MODE_ARG"
  exit $?
fi

echo "🔨 FreeLang 네이티브 빌드"
echo "   입력: $FL_INPUT"
echo "   출력: $OUTPUT"
echo ""

# ─── 0. module declaration 처리 ──────────────────────────────────
MODULE_JSON=$(run_fl_module_parse "$FL_INPUT" 2>/dev/null || true)
HAS_USER_FNS=0
if [ -n "$MODULE_JSON" ]; then
  MODULE_NAME=$(printf '%s' "$MODULE_JSON" | sed 's/.*"name":"\([^\"]*\)".*/\1/')
  MODULE_VER=$(printf '%s' "$MODULE_JSON" | sed 's/.*"version":"\([^\"]*\)".*/\1/')
  USE_LIST=$(printf '%s' "$MODULE_JSON" | sed -n 's/.*"use":\[\([^]]*\)\].*/\1/p')
  USE_PROFILES=$(printf '%s' "$USE_LIST" | tr -d '"' | tr ',' ' ')

  echo "📦 module: $MODULE_NAME v$MODULE_VER"
  if [ -n "$USE_PROFILES" ]; then
    echo "   :use → $USE_PROFILES"
    echo "   의존성 해결 중..."
    pkg_file="$(mktemp)"
    if ! run_fl_resolve_deps_profiles $USE_PROFILES > "$pkg_file"; then
      rm -f "$pkg_file"
      exit 1
    fi
    mapfile -t RESOLVE_PACKAGES < "$pkg_file"
    metadata_file="$(mktemp)"
    if ! run_fl_resolve_deps_metadata "$metadata_file" "${RESOLVE_PACKAGES[@]}"; then
      rm -f "$metadata_file" "$pkg_file"
      exit 1
    fi
    run_fl_generate_user_fns "$metadata_file"
    HAS_USER_FNS=1
    rm -f "$pkg_file" "$metadata_file"
    echo ""
  fi
fi

# ─── 1. load 인라인 전처리 ───────────────────────────────────────
# (load "path.fl") 를 파일 내용으로 인라인 치환
PREPROCESSED="/tmp/fl_preprocessed_$$.fl"

run_fl_build_helper --preprocess "$FL_INPUT" "$PREPROCESSED" || { rm -f "$PREPROCESSED"; exit 1; }

# 긴 문자열 자동 분할 (>900B)
run_fl_str_split "$PREPROCESSED" 2>&1 || true

# ─── 2. 사전 검사 ────────────────────────────────────────────────
CHECK_PARENS_SRC="$SCRIPT_DIR/fl-check-parens.c"
CHECK_PARENS_BIN="$SCRIPT_DIR/.fl-check-parens"
if [ -f "$CHECK_PARENS_SRC" ]; then
  if [ ! -x "$CHECK_PARENS_BIN" ] || [ "$CHECK_PARENS_BIN" -ot "$CHECK_PARENS_SRC" ]; then
    if ! cc -O2 -std=c99 "$CHECK_PARENS_SRC" -o "$CHECK_PARENS_BIN" 2>/tmp/fl-check-parens-build.log; then
      echo "[fl-build] fl-check-parens.c build failed" >&2
      cat /tmp/fl-check-parens-build.log >&2
      rm -f "$PREPROCESSED"
      exit 1
    fi
  fi
  echo "🔍 괄호/브래킷 검사..."
  # CSS/JS/Python 문자열이 있는 파일은 false positive 위험 → cgc-bin이 최종 판단
  # 줄 수 300 이하 단순 FL만 검사
  LINE_COUNT=$(wc -l < "$PREPROCESSED" 2>/dev/null || echo 999)
  if [ "$LINE_COUNT" -le 300 ]; then
    PARENS_OUT=$(timeout 3 "$CHECK_PARENS_BIN" "$PREPROCESSED" 2>&1)
    PARENS_EXIT=$?
    if [ $PARENS_EXIT -eq 124 ]; then
      echo "   ⚠️  괄호 검사 타임아웃 → cgc-bin 파서가 최종 판단"
    elif [ $PARENS_EXIT -ne 0 ]; then
      echo "$PARENS_OUT"
      echo "❌ 사전 검사 실패 — 컴파일 중단"
      rm -f "$PREPROCESSED"
      exit 1
    else
      echo "$PARENS_OUT"
    fi
  else
    echo "   ✅ ${LINE_COUNT}줄 — cgc-bin 파서로 직접 검사 (문자열 내 코드 포함 파일)"
  fi
fi

# ─── 2.5. 시맨틱 분석 (Semi-static Linter) ─────────────────────────
echo "🧠 시맨틱 분석..."
if ! run_fl_build_helper --semantic "$PREPROCESSED"; then
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
  if RULES_LOG=$(run_rules_native "$SCRIPT_DIR" "$FL_RULES" "$PREPROCESSED" "$SCRIPT_DIR/fl-build.sh" 2>&1); then
    RULES_STATUS=0
  else
    RULES_STATUS=$?
  fi
  RULES_OUT=$(printf '%s\n' "$RULES_LOG" | grep -E "^(error|warn):" || true)

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

  if [ "$RULES_STATUS" -ne 0 ] && [ "$RULES_ERRORS" -eq 0 ]; then
    echo "$RULES_LOG"
    echo "❌ 프로젝트 규칙 검사 실패"
    rm -f "$PREPROCESSED"
    exit 1
  fi

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
  run_fl_build_helper --syntax-map "$SYNTAX_LOG" "$PREPROCESSED"

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
  $RUNTIME_DIR/net.c \
  $RUNTIME_DIR/builtins-shim.c"

if [ -f "$RUNTIME_DIR/mariadb.c" ]; then
  CORE_SRCS="$CORE_SRCS $RUNTIME_DIR/mariadb.c"
  echo "   + MariaDB dlopen 바인딩 포함"
fi

# ── libfx.a 캐시 (런타임 변경 시에만 재빌드) ─────────────────────
LIBFX_A="$RUNTIME_DIR/libfx.a"
LIBFX_HASH_FILE="$RUNTIME_DIR/.libfx_hash"
CURRENT_HASH=$(md5sum $CORE_SRCS "$RUNTIME_DIR/runtime.h" "$SCRIPT_REAL" 2>/dev/null | md5sum | cut -d' ' -f1)

if [ ! -f "$LIBFX_A" ] || [ "$(cat "$LIBFX_HASH_FILE" 2>/dev/null)" != "${CURRENT_HASH}_${NO_NET}" ]; then
  echo "   📦 libfx.a 빌드 중... (런타임 변경 감지)"
  OBJ_DIR="/tmp/fl_runtime_objs_$$"
  mkdir -p "$OBJ_DIR"
  for src in $CORE_SRCS; do
    obj="$OBJ_DIR/$(basename "$src" .c).o"
    gcc -O2 -c "$src" -I "$RUNTIME_DIR" $EXTRA_CFLAGS -w -o "$obj" &
  done
  wait
  rm -f "$LIBFX_A"
  ar rcs "$LIBFX_A" "$OBJ_DIR"/*.o
  rm -rf "$OBJ_DIR"
  echo "${CURRENT_HASH}_${NO_NET}" > "$LIBFX_HASH_FILE"
  echo "   ✅ libfx.a 완성 (이후 빌드는 재사용)"
else
  echo "   ✅ libfx.a 캐시 사용"
fi

# user-fns.c — 패키지/내장 함수가 들어 있으므로 항상 링크
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
    -I "$RUNTIME_DIR" $EXTRA_CFLAGS -o "$APP_O" 2>/tmp/fl_app_gcc_$$.log; then
    echo "${APP_HASH}_${NO_NET}" > "$APP_HASH_FILE"
    echo "   ✅ app.o 완성 (이후 빌드는 재사용)"
  else
    rm -f "$APP_O" "$APP_HASH_FILE"
    echo "❌ app.o 컴파일 실패:"
    cat /tmp/fl_app_gcc_$$.log
    rm -f "$C_FILE" "$PREPROCESSED" /tmp/fl_app_gcc_$$.log
    exit 1
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
  if ! run_fl_build_helper --syntax-map "$GCC_LOG" "$PREPROCESSED"; then
    cat "$GCC_LOG"
  fi
  rm -f "$C_FILE" "$PREPROCESSED" "$GCC_LOG"
  exit 1
fi

# ─── 5. 정리 ─────────────────────────────────────────────────────
rm -f "$C_FILE" "$PREPROCESSED"

echo ""
echo "✅ 빌드 완료: ./$OUTPUT"
echo "   크기: $(du -sh "$OUTPUT" | cut -f1)"
echo "   실행: ./$OUTPUT"
