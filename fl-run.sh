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

# ── fx.toml 파싱 (Python) ─────────────────────────────────────
eval "$(python3 << PYEOF
import re, json

with open('$TOML_FILE') as f:
    content = f.read()

# [project] 섹션
name  = re.search(r'name\s*=\s*"([^"]+)"',  content)
entry = re.search(r'entry\s*=\s*"([^"]+)"', content)
port  = re.search(r'port\s*=\s*(\d+)',       content)
out   = re.search(r'output\s*=\s*"([^"]+)"',content)

print(f'FL_NAME={name.group(1) if name else "app"}')
print(f'FL_ENTRY={entry.group(1) if entry else "main.fl"}')
print(f'FL_PORT={port.group(1) if port else "0"}')
print(f'FL_OUTPUT={out.group(1) if out else "app-bin"}')

# packages 배열 파싱
m = re.search(r'packages\s*=\s*\[(.*?)\]', content, re.DOTALL)
pkgs = []
if m:
    raw = m.group(1)
    pkgs = [p.strip().strip('"') for p in re.findall(r'"([^"]+)"', raw)]
print(f"FL_PACKAGES='{json.dumps(pkgs)}'")
PYEOF
)"

echo "────────────────────────────────────────"
echo " fl run — $FL_NAME"
echo "────────────────────────────────────────"
echo ""

# ── 패키지 설치 ───────────────────────────────────────────────
if [ "$CMD" = "run" ] || [ "$CMD" = "install" ] || [ "$CMD" = "build" ]; then
  echo "[install] 런타임 패키지 설치..."
  python3 << PYEOF
import json, urllib.request, sys

pkgs = json.loads('$FL_PACKAGES')
if not pkgs:
    print("  (패키지 없음)")
    sys.exit(0)

ok_count = 0
for name in pkgs:
    payload = json.dumps({"name": name, "build": True}).encode()
    req = urllib.request.Request(
        "http://localhost:40330/fx/fns/import",
        data=payload, headers={"Content-Type":"application/json"}, method="POST"
    )
    try:
        with urllib.request.urlopen(req, timeout=60) as resp:
            d = json.loads(resp.read())
            if d.get("ok"):
                ms = (d.get("build") or {}).get("durationMs", "?")
                print(f"  ✅ {name} ({ms}ms)")
                ok_count += 1
            else:
                print(f"  ❌ {name} — {d.get('error','')}")
    except Exception as e:
        print(f"  ❌ {name} — {e}")

print(f"\n  {ok_count}/{len(pkgs)} 패키지 설치 완료")
PYEOF
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
