#!/usr/bin/env bash
# apply-on-x86.sh — fx2 컴파일러 패치를 x86 노드에서 게이트·롤백 방식으로 적용.
#
# ⛔ aarch64 노드에서는 cgc-bin 실행 불가 → 반드시 x86 노드(kimjin)에서 실행.
#
# 사용:
#   bash apply-on-x86.sh --dry-run            # 앵커 매칭만 확인 (수정 X)
#   bash apply-on-x86.sh --bug 2              # #2만 적용·검증
#   bash apply-on-x86.sh all                  # 2→3→1→5 순서로 전부
#   bash apply-on-x86.sh all --commit         # 각 단계 통과 시 git 커밋
#
# 환경변수(노드별 경로 오버라이드):
#   CODEGEN     codegen-c.fl 경로            (기본 /root/freelang-v11/self/codegen-c.fl)
#   BUILD_CMD   cgc-bin 재빌드 명령
#   FIXPOINT    고정점 검증 스크립트 경로
#   FLBUILD     fl-build.sh 경로 (재현 테스트 컴파일용)
#   CONF        conformance.fl 경로
# 플래그:
#   --skip-fixpoint  고정점 스크립트 부재 시 명시적으로 생략(권장 안 함)
set -u

HERE="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
CODEGEN="${CODEGEN:-/root/freelang-v11/self/codegen-c.fl}"
BUILD_CMD="${BUILD_CMD:-cd /root/freelang-v11 && node scripts/build.js}"
FIXPOINT="${FIXPOINT:-/root/freelang-v11-fx2/verify-fixpoint.sh}"
FLBUILD="${FLBUILD:-/root/freelang-v11-fx2/fl-build.sh}"
CONF="${CONF:-/root/freelang-v11-fx2/spec/conformance.fl}"
EDITS_JS="$HERE/_edits.js"

DRY=0; COMMIT=0; SKIP_FP=0; ORDER=""
while [ $# -gt 0 ]; do
  case "$1" in
    --dry-run) DRY=1 ;;
    --commit) COMMIT=1 ;;
    --skip-fixpoint) SKIP_FP=1 ;;
    --bug) shift; ORDER="$1" ;;
    all) ORDER="2 3 1 5" ;;
    2|3|1|5) ORDER="$1" ;;
    *) echo "알 수 없는 인자: $1" >&2; exit 2 ;;
  esac; shift
done
[ -z "$ORDER" ] && ORDER="2 3 1 5"

say(){ printf '\n\033[1m== %s ==\033[0m\n' "$*"; }
die(){ printf '\033[31m✖ %s\033[0m\n' "$*" >&2; exit 1; }

# ── 사전 점검 ──
say "사전 점검"
[ -f "$CODEGEN" ] || die "codegen 없음: $CODEGEN"
ARCH="$(uname -m)"
echo "  arch=$ARCH  codegen=$CODEGEN"
[ "$ARCH" = "x86_64" ] || echo "  ⚠ x86_64가 아님 — cgc-bin 실행/빌드 불가할 수 있음"
if [ $DRY -eq 0 ]; then
  command -v git >/dev/null && git -C "$(dirname "$CODEGEN")" diff --quiet 2>/dev/null \
    || echo "  ⚠ codegen 디렉토리에 미커밋 변경 있음(롤백은 .bak 기준으로 동작)"
fi

# ── dry-run: 앵커만 확인 ──
if [ $DRY -eq 1 ]; then
  say "DRY-RUN — 앵커 매칭 확인 (수정 안 함)"
  rc=0
  for b in $ORDER; do node "$EDITS_JS" check "$CODEGEN" "$b" || rc=1; done
  [ $rc -eq 0 ] && echo "전부 매칭 OK — 실제 적용 가능" || echo "일부 불일치 — 해당 패치 수동 적용 필요"
  exit $rc
fi

# ── 단계별 적용 ──
gate_fixpoint(){
  if [ -x "$FIXPOINT" ] || [ -f "$FIXPOINT" ]; then
    bash "$FIXPOINT" || return 1
  elif [ $SKIP_FP -eq 1 ]; then
    echo "  ⚠ 고정점 검증 생략(--skip-fixpoint)"
  else
    echo "  ✖ 고정점 스크립트 없음: $FIXPOINT"
    echo "    검증 없는 진행 금지. 경로를 FIXPOINT=로 지정하거나 --skip-fixpoint 명시."
    return 1
  fi
}

gate_repro(){ # $1=bug
  local b="$1" t="/tmp/fxrepro_$1.fl" bin="/tmp/fxrepro_$1" out exp
  node "$EDITS_JS" repro "$b" > "$t"
  exp="$(node "$EDITS_JS" expect "$b")"
  bash "$FLBUILD" "$t" "$bin" >/dev/null 2>&1 || { echo "  ✖ 재현 빌드 실패"; return 1; }
  out="$("$bin" 2>/dev/null | tr -d '[:space:]')"
  if [ "$out" = "$exp" ]; then echo "  ✓ 재현 PASS (out=$out)"; else
    echo "  ✖ 재현 FAIL (기대=$exp 실제=$out)"; return 1; fi
}

for b in $ORDER; do
  say "버그 #$b 적용"
  node "$EDITS_JS" check "$CODEGEN" "$b" || die "#$b 앵커 불일치 — patches/bug$b-*.patch.md 수동 적용"
  cp "$CODEGEN" "$CODEGEN.bak.$b" || die "백업 실패"
  node "$EDITS_JS" apply "$CODEGEN" "$b" || { mv "$CODEGEN.bak.$b" "$CODEGEN"; die "#$b 적용 실패(원복함)"; }

  echo "  [빌드] $BUILD_CMD"
  if ! bash -c "$BUILD_CMD"; then mv "$CODEGEN.bak.$b" "$CODEGEN"; die "#$b 빌드 실패(원복함)"; fi
  echo "  [고정점]"; if ! gate_fixpoint; then mv "$CODEGEN.bak.$b" "$CODEGEN"; die "#$b 고정점 실패(원복함 — cgc-bin 재빌드 필요)"; fi
  echo "  [재현]";   if ! gate_repro "$b"; then mv "$CODEGEN.bak.$b" "$CODEGEN"; die "#$b 재현 실패(원복함 — cgc-bin 재빌드 필요)"; fi
  echo "  [conformance]"; if ! bash "$FLBUILD" "$CONF" "/tmp/fxconf" >/dev/null 2>&1 || ! /tmp/fxconf 2>&1 | grep -q "CONFORMANCE PASS"; then
    mv "$CODEGEN.bak.$b" "$CODEGEN"; die "#$b conformance 실패(원복함 — cgc-bin 재빌드 필요)"; fi
  echo "  ✓ #$b 전 게이트 통과"
  rm -f "$CODEGEN.bak.$b"

  if [ $COMMIT -eq 1 ] && command -v git >/dev/null; then
    git -C "$(dirname "$CODEGEN")" add "$CODEGEN" \
      && git -C "$(dirname "$CODEGEN")" commit -q -m "fix(cgc): 버그 #$b 적용 (patches/bug$b)" \
      && echo "  ✓ 커밋됨"
  fi
done

say "완료 — 적용 순서: $ORDER"
echo "주의: 고정점/conformance 실패 시 .bak로 원복되지만 cgc-bin은 직접 재빌드 필요."
