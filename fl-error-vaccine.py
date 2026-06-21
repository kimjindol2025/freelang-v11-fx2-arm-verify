#!/usr/bin/env python3
"""
fl-error-vaccine.py — gcc 에러를 FL 개발자 언어로 번역하는 백신
사용법: python3 fl-error-vaccine.py <gcc_log> <fl_source> [<c_source>]

패턴 DB: fl-error-patterns.json (없으면 내장 기본 패턴 사용)
새 패턴은 patterns.json에 추가 → 백신이 누적 강화됨
"""

import sys, re, json, os

# ── 패턴 DB 로드 ─────────────────────────────────────────────
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PATTERNS_FILE = os.path.join(SCRIPT_DIR, "fl-error-patterns.json")

BUILTIN_PATTERNS = [
    {
        "id": "name-conflict",
        "gcc": r"called object '(.+?)' is not a function",
        "cause": "함수와 변수 이름 충돌",
        "fix": "'{0}' 이름으로 변수와 함수가 동시에 존재합니다\n"
               "        → (defn {1} ...) 함수명을 다르게 바꾸거나\n"
               "        → (define {1} ...) 변수명을 다르게 바꾸세요\n"
               "        예: pct → calc-pct, count → item-count",
        "fl_convert": True
    },
    {
        "id": "undef-ref-fxb",
        "gcc": r"undefined reference to `(fxb_\w+)'",
        "cause": "SQLite/외부 바인딩 함수 누락",
        "fix": "'{0}' 함수가 없습니다\n"
               "        → ({1}) 패턴으로 호출하세요\n"
               "        → fxb_sqlite_query → (fxb-sqlite-query DB sql)\n"
               "        → fxb_sqlite_exec  → (fxb-sqlite-exec DB sql)",
        "fl_convert": True
    },
    {
        "id": "undef-ref-fl",
        "gcc": r"undefined reference to `(fl_\w+)'",
        "cause": "런타임 함수 누락 또는 이름 오류",
        "fix": "'{0}' 런타임 함수를 찾을 수 없습니다\n"
               "        → runtime.h 에 선언이 있는지 확인\n"
               "        → 함수명 오타: fl_str_val / fl_int / fl_println 등",
        "fl_convert": False
    },
    {
        "id": "too-few-args",
        "gcc": r"too few arguments to function '(.+?)'",
        "cause": "인자 부족",
        "fix": "({1}) 호출 시 인자가 부족합니다\n"
               "        → 함수 정의를 확인하고 필요한 인자를 추가하세요",
        "fl_convert": True
    },
    {
        "id": "too-many-args",
        "gcc": r"too many arguments to function '(.+?)'",
        "cause": "인자 초과",
        "fix": "({1}) 호출 시 인자가 너무 많습니다\n"
               "        → 함수 정의를 확인하고 인자를 줄이세요",
        "fl_convert": True
    },
    {
        "id": "implicit-decl",
        "gcc": r"implicit declaration of function '(.+?)'",
        "cause": "선언 없이 함수 호출",
        "fix": "'{0}' 함수가 선언되지 않았습니다\n"
               "        → runtime.h 에 FLValue {0}(FLValue); 추가 필요\n"
               "        → 또는 ({1}) 철자 확인",
        "fl_convert": True
    },
    {
        "id": "incompatible-type",
        "gcc": r"incompatible types when (\w+) type '(.+?)' but (.+?) has type '(.+?)'",
        "cause": "타입 불일치",
        "fix": "타입 불일치: '{1}' 대신 '{3}'\n"
               "        → FLValue를 직접 int/char* 로 사용하지 마세요\n"
               "        → fl_int() / fl_str_val() 등 생성 함수를 통해 전달",
        "fl_convert": False
    },
    {
        "id": "undeclared-var",
        "gcc": r"'(.+?)' undeclared",
        "cause": "변수/함수 미선언",
        "fix": "'{0}' 이 선언되지 않았습니다\n"
               "        → FL에서 (define ${1} ...) 또는 (let [${1} ...]) 확인\n"
               "        → 또는 runtime.h 함수명 오타",
        "fl_convert": True
    },
    {
        "id": "number-to-string",
        "gcc": r"undefined reference to `number_to_string'",
        "cause": "FL 표준 함수 이름 오류",
        "fix": "number_to_string → (number->string $n) 또는 (str $n) 사용",
        "fl_convert": False
    },
]

def load_patterns():
    try:
        with open(PATTERNS_FILE) as f:
            extra = json.load(f)
        return BUILTIN_PATTERNS + extra
    except FileNotFoundError:
        return BUILTIN_PATTERNS
    except Exception as e:
        print(f"  ⚠️  패턴 DB 로드 실패: {e}", file=sys.stderr)
        return BUILTIN_PATTERNS

# ── C 함수명 → FL 표현 변환 ──────────────────────────────────
def c_to_fl(name):
    n = name.replace('_', '-')
    for prefix in ['fl-', 'fxb-']:
        if n.startswith(prefix):
            return n
    return n

# ── 패턴 매칭 + 힌트 출력 ────────────────────────────────────
def apply_pattern(p, msg, seen):
    m = re.search(p["gcc"], msg)
    if not m:
        return False
    key = (p["id"], m.group(1) if m.lastindex else "")
    if key in seen:
        return True
    seen.add(key)

    groups = [m.group(i+1) for i in range(m.lastindex or 0)]
    fl_groups = [c_to_fl(g) if p.get("fl_convert") else g for g in groups]

    fix = p["fix"]
    try:
        fix = fix.format(*groups, *fl_groups[:len(groups)])
    except (IndexError, KeyError):
        pass

    print(f"     원인: {p['cause']}")
    print(f"     💡 {fix}")
    return True

# ── FL 줄 역추적 ─────────────────────────────────────────────
def find_fl_line(gcc_log, fl_lines):
    results = []
    for m in re.finditer(r'<fl>:(\d+):\d+: (?:error|warning): (.+)', gcc_log):
        lineno = int(m.group(1)) - 1
        msg = m.group(2)
        fl_line = fl_lines[lineno].strip() if lineno < len(fl_lines) else ''
        results.append((lineno + 1, msg, fl_line))
    return results

# ── C 소스에서 FL 이름 역추적 ────────────────────────────────
def extract_c_context(c_source, c_line_no):
    """C 소스에서 특정 줄 주변 컨텍스트 추출"""
    if not c_source:
        return ""
    lines = c_source.splitlines()
    start = max(0, c_line_no - 3)
    end = min(len(lines), c_line_no + 2)
    return "\n".join(f"     {i+start+1}: {lines[i+start]}" for i in range(end - start))

# ── 메인 ─────────────────────────────────────────────────────
def main():
    if len(sys.argv) < 3:
        print("사용법: fl-error-vaccine.py <gcc_log> <fl_source> [<c_source>]")
        sys.exit(1)

    gcc_log    = open(sys.argv[1]).read()
    fl_lines   = open(sys.argv[2]).read().splitlines() if os.path.exists(sys.argv[2]) else []
    c_source   = open(sys.argv[3]).read() if len(sys.argv) > 3 and os.path.exists(sys.argv[3]) else ""

    patterns = load_patterns()
    seen = set()
    any_output = False

    # 1) FL 줄 번호가 있는 에러 (가장 정밀)
    fl_errors = find_fl_line(gcc_log, fl_lines)
    if fl_errors:
        for lineno, msg, fl_line in fl_errors:
            print(f"\n  📍 FL 줄 {lineno}: {msg}")
            if fl_line:
                print(f"     코드: {fl_line[:120]}")
            matched = False
            for p in patterns:
                if apply_pattern(p, msg, seen):
                    matched = True
                    break
            if not matched:
                print(f"     💡 알 수 없는 오류 → GCC 원본: {msg[:100]}")
            any_output = True

    # 2) 링커 에러 (undefined reference) — 줄 번호 없음
    undef_refs = list(dict.fromkeys(re.findall(r"undefined reference to `([^']+)'", gcc_log)))
    if undef_refs:
        print(f"\n  🔗 링커 에러:")
        for sym in undef_refs:
            key = ("undef", sym)
            if key in seen:
                continue
            seen.add(key)
            print(f"     ❌ {sym}")
            matched = False
            for p in patterns:
                fake_msg = f"undefined reference to '{sym}'"
                if apply_pattern(p, fake_msg, set()):
                    matched = True
                    break
            if not matched:
                fl_name = c_to_fl(sym)
                print(f"     💡 ({fl_name}) 함수 확인 또는 외부 라이브러리 누락")
            any_output = True

    # 3) 패턴 전체 스캔 (줄 번호 없는 에러)
    for line in gcc_log.splitlines():
        if 'error:' not in line and 'warning:' not in line:
            continue
        if '<fl>' in line:
            continue
        msg = re.sub(r'^[^:]+:\d+:\d+: (?:error|warning): ', '', line)
        for p in patterns:
            if apply_pattern(p, msg, seen):
                print(f"\n  ❌ {msg[:80]}")
                any_output = True
                break

    if not any_output:
        print("\n  (알 수 없는 에러 — 원본 GCC 로그:)")
        print(gcc_log[:1500])

if __name__ == "__main__":
    main()
