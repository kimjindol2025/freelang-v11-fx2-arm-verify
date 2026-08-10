#!/usr/bin/env python3
"""
fl-resolve-deps.py — module :use profiles → user-fns.c 재생성

실행 모드:
- fl-resolve-deps.py <profile1> [profile2 ...] <script_dir>
- fl-resolve-deps.py --packages <script_dir> [pkg1 ...]
- fl-resolve-deps.py --packages --metadata <metadata-json> <script_dir> [pkg1 ...]
"""
import json
import os
import sys

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

def resolve_profiles_to_packages(profile_names, profiles_dir):
    all_packages = []
    seen = set()
    for pname in profile_names:
        pfile = os.path.join(profiles_dir, f"{pname}.json")
        if not os.path.exists(pfile):
            print(f"  ⚠️  프로파일 없음: {pname}", file=sys.stderr)
            continue

        prof = json.load(open(pfile))
        pkgs = prof.get("packages", [])
        added = [p for p in pkgs if p not in seen]
        seen.update(added)
        all_packages.extend(added)
        print(f"  ✓ :{pname} → {', '.join(pkgs)}")

    return all_packages


def load_metadata_records(metadata_file):
    with open(metadata_file) as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise TypeError("metadata payload must be a list")
    return data


def _metadata_record_by_package(records):
    records_by_package = {}
    for rec in records:
        if isinstance(rec, dict) and "package" in rec:
            records_by_package[str(rec.get("package"))] = rec
    return records_by_package


def collect_function_sections_from_records(records, package_order):
    index = _metadata_record_by_package(records)
    fn_sections = []
    for pkg in package_order:
        rec = index.get(pkg)
        if not isinstance(rec, dict):
            continue
        name = rec.get("name", pkg)
        args = rec.get("args", 1)
        body = rec.get("body", "")
        fn_sections.append((name, args, body, bool(rec.get("skipRuntimeShim", False))))
    return fn_sections


def main():
    if len(sys.argv) < 2:
        print("사용법: fl-resolve-deps.py <profiles...> <script_dir>", file=sys.stderr)
        print("또는: fl-resolve-deps.py --packages [--metadata <metadata-file>] <script_dir> [package...]", file=sys.stderr)
        sys.exit(1)

    metadata_file = None
    script_dir = None
    all_packages = []

    if sys.argv[1] == "--packages":
        if len(sys.argv) < 3:
            print("사용법: fl-resolve-deps.py --packages [--metadata <metadata-file>] <script_dir> [package...]", file=sys.stderr)
            sys.exit(1)

        argi = 2
        while argi < len(sys.argv):
            arg = sys.argv[argi]
            if arg == "--metadata":
                if argi + 1 >= len(sys.argv):
                    print("사용법: fl-resolve-deps.py --packages --metadata <metadata-file> <script_dir> [package...]", file=sys.stderr)
                    sys.exit(1)
                metadata_file = sys.argv[argi + 1]
                argi += 2
                continue

            if script_dir is None:
                script_dir = arg
            else:
                all_packages.append(arg)
            argi += 1

        if script_dir is None:
            print("사용법: fl-resolve-deps.py --packages [--metadata <metadata-file>] <script_dir> [package...]", file=sys.stderr)
            sys.exit(1)

        profiles_dir = os.path.join(script_dir, "profiles")
        packages_dir = os.path.join(script_dir, "packages")
        runtime_dir = os.path.join(script_dir, "runtime")

        if not all_packages:
            print("  (해결된 패키지 없음 — user-fns.c 변경 없음)")
            return

        if metadata_file:
            metadata_records = load_metadata_records(metadata_file)
            fn_sections = collect_function_sections_from_records(metadata_records, all_packages)
            out = [HEADER]

            out.append("/* FL:USER_SECTION:BEGIN */")
            for name, args, body, skip_runtime_shim in fn_sections:
                out.append(f"/* FL:FN:{name} */")
                out.append(body.strip())
                out.append("/* FL:FN_END */")
                out.append("")
            out.append("/* FL:USER_SECTION:END */")
            out.append("")

            out.append("/* FL:SHIM_SECTION:BEGIN */")
            for name, args, _, skip_runtime_shim in fn_sections:
                if skip_runtime_shim:
                    continue
                c_name = "ufl_" + name.replace("-", "_")
                shim_name = name.replace("-", "_")
                args_def = ", ".join(f"FLValue a{i}" for i in range(args))
                args_call = ", ".join(f"a{i}" for i in range(args))
                out.append(f"/* FL:SHIM_FN:{name} */")
                if args == 0:
                    out.append(f"FLValue {shim_name}(void) {{ return {c_name}(); }}")
                else:
                    out.append(f"FLValue {shim_name}({args_def}) {{ return {c_name}({args_call}); }}")
                out.append("/* FL:SHIM_FN_END */")
            out.append("/* FL:SHIM_SECTION:END */")
            out.append("")

            user_fns_path = os.path.join(runtime_dir, "user-fns.c")
            open(user_fns_path, "w").write("\n".join(out) + "\n")
            print(f"  → user-fns.c 생성 ({len(fn_sections)}개 함수)")
            return


        # fallback: legacy local metadata loading
        fn_sections = []
        missing = []
        for pkgname in all_packages:
            new_path = os.path.join(packages_dir, pkgname, "metadata.json")
            old_path = os.path.join(packages_dir, f"{pkgname}.json")
            pkgfile = new_path if os.path.exists(new_path) else (old_path if os.path.exists(old_path) else None)
            if not pkgfile:
                print(f"  ⚠️  패키지 없음: {pkgname}", file=sys.stderr)
                missing.append(pkgname)
                continue
            pkg = json.load(open(pkgfile))
            fn_sections.append((pkgname, pkg.get("args", 1), pkg.get("body", "")))

        user_fns_path = os.path.join(runtime_dir, "user-fns.c")
        out = [HEADER]

        out.append("/* FL:USER_SECTION:BEGIN */")
        for name, args, body in fn_sections:
            out.append(f"/* FL:FN:{name} */")
            out.append(body.strip())
            out.append("/* FL:FN_END */")
            out.append("")
        out.append("/* FL:USER_SECTION:END */")
        out.append("")

        out.append("/* FL:SHIM_SECTION:BEGIN */")
        for name, args, _ in fn_sections:
            c_name = "ufl_" + name.replace("-", "_")
            shim_name = name.replace("-", "_")
            args_def = ", ".join(f"FLValue a{i}" for i in range(args))
            args_call = ", ".join(f"a{i}" for i in range(args))
            out.append(f"/* FL:SHIM_FN:{name} */")
            if args == 0:
                out.append(f"FLValue {shim_name}(void) {{ return {c_name}(); }}")
            else:
                out.append(f"FLValue {shim_name}({args_def}) {{ return {c_name}({args_call}); }}")
            out.append("/* FL:SHIM_FN_END */")
        out.append("/* FL:SHIM_SECTION:END */")

        open(user_fns_path, "w").write("\n".join(out) + "\n")
        print(f"  → user-fns.c 생성 ({len(fn_sections)}개 함수)")
        if missing:
            print(f"  ⚠️  누락 패키지: {', '.join(missing)}", file=sys.stderr)
        return

    if len(sys.argv) < 3:
        print("사용법: fl-resolve-deps.py <profiles...> <script_dir>", file=sys.stderr)
        print("또는: fl-resolve-deps.py --packages [--metadata <metadata-file>] <script_dir> [package...]", file=sys.stderr)
        sys.exit(1)

    script_dir = sys.argv[-1]
    all_packages = resolve_profiles_to_packages(sys.argv[1:-1], os.path.join(script_dir, "profiles"))

    profiles_dir = os.path.join(script_dir, "profiles")
    _ = profiles_dir
    packages_dir = os.path.join(script_dir, "packages")
    runtime_dir = os.path.join(script_dir, "runtime")

    if not all_packages:
        print("  (해결된 패키지 없음 — user-fns.c 변경 없음)")
        return

    # packages → C 코드 수집 (P2.1 디렉토리 구조 우선, 구형 flat JSON 폴백)
    fn_sections = []  # (name, arity, body)
    missing = []
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
    for name, args, body in fn_sections:
        out.append(f"/* FL:FN:{name} */")
        out.append(body.strip())
        out.append("/* FL:FN_END */")
        out.append("")
    out.append("/* FL:USER_SECTION:END */")
    out.append("")

    # SHIM_SECTION — cgc-bin이 kebab 이름을 snake_case로 호출
    out.append("/* FL:SHIM_SECTION:BEGIN */")
    for name, args, _ in fn_sections:
        c_name = "ufl_" + name.replace("-", "_")
        shim_name = name.replace("-", "_")
        args_def = ", ".join(f"FLValue a{i}" for i in range(args))
        args_call = ", ".join(f"a{i}" for i in range(args))
        out.append(f"/* FL:SHIM_FN:{name} */")
        if args == 0:
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
