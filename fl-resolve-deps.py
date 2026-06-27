#!/usr/bin/env python3
"""
fl-resolve-deps.py — module :use profiles → user-fns.c 재생성

사용법: fl-resolve-deps.py <profile1> [profile2 ...] <script_dir>
마지막 인자가 script_dir (디렉터리).
"""
import sys, json, os

HEADER = """\
/**
 * freelang-v11-fx2 — 사용자 패키지 함수
 * fl-resolve-deps.py가 module :use 선언에서 자동 생성합니다.
 * 직접 수정하지 마세요.
 */
#include "runtime.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

"""

def main():
    if len(sys.argv) < 3:
        print("사용법: fl-resolve-deps.py <profiles...> <script_dir>", file=sys.stderr)
        sys.exit(1)

    script_dir   = sys.argv[-1]
    profile_names = sys.argv[1:-1]
    profiles_dir = os.path.join(script_dir, "profiles")
    packages_dir = os.path.join(script_dir, "packages")
    runtime_dir  = os.path.join(script_dir, "runtime")

    # profiles → packages 해결
    all_packages = []
    seen = set()
    for pname in profile_names:
        pfile = os.path.join(profiles_dir, f"{pname}.json")
        if not os.path.exists(pfile):
            print(f"  ⚠️  프로파일 없음: {pname}", file=sys.stderr)
            continue
        prof  = json.load(open(pfile))
        pkgs  = prof.get("packages", [])
        added = [p for p in pkgs if p not in seen]
        seen.update(added)
        all_packages.extend(added)
        print(f"  ✓ :{pname} → {', '.join(pkgs)}")

    if not all_packages:
        print("  (해결된 패키지 없음 — user-fns.c 변경 없음)")
        return

    # packages → C 코드 수집 (P2.1 디렉토리 구조 우선, 구형 flat JSON 폴백)
    fn_sections  = []  # (name, arity, body)
    missing      = []
    for pkgname in all_packages:
        # P2.1: packages/name/metadata.json
        new_path = os.path.join(packages_dir, pkgname, "metadata.json")
        # 구형: packages/name.json
        old_path = os.path.join(packages_dir, f"{pkgname}.json")
        pkgfile = new_path if os.path.exists(new_path) else (old_path if os.path.exists(old_path) else None)
        if not pkgfile:
            print(f"  ⚠️  패키지 없음: {pkgname}", file=sys.stderr)
            missing.append(pkgname)
            continue
        pkg = json.load(open(pkgfile))
        fn_sections.append((pkgname, pkg.get("args", 1), pkg.get("body", "")))

    # user-fns.c 생성
    user_fns_path = os.path.join(runtime_dir, "user-fns.c")
    out = [HEADER]

    # FL:USER_SECTION
    out.append("/* FL:USER_SECTION:BEGIN */")
    for name, arity, body in fn_sections:
        out.append(f"/* FL:FN:{name} */")
        out.append(body.strip())
        out.append("/* FL:FN_END */")
        out.append("")
    out.append("/* FL:USER_SECTION:END */")
    out.append("")

    # SHIM_SECTION — cgc-bin이 kebab 이름을 snake_case로 호출
    out.append("/* FL:SHIM_SECTION:BEGIN */")
    for name, arity, _ in fn_sections:
        c_name   = "ufl_" + name.replace("-", "_")
        shim_name = name.replace("-", "_")
        args_def  = ", ".join(f"FLValue a{i}" for i in range(arity))
        args_call = ", ".join(f"a{i}" for i in range(arity))
        out.append(f"/* FL:SHIM_FN:{name} */")
        if arity == 0:
            out.append(f"FLValue {shim_name}(void) {{ return {c_name}(); }}")
        else:
            out.append(f"FLValue {shim_name}({args_def}) {{ return {c_name}({args_call}); }}")
        out.append("/* FL:SHIM_FN_END */")
    out.append("/* FL:SHIM_SECTION:END */")

    open(user_fns_path, "w").write("\n".join(out) + "\n")
    print(f"  → user-fns.c 생성 ({len(fn_sections)}개 함수)")
    if missing:
        print(f"  ⚠️  누락 패키지: {', '.join(missing)}", file=sys.stderr)


if __name__ == "__main__":
    main()
