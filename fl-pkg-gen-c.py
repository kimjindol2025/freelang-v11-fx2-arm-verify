#!/usr/bin/env python3
"""
fl-pkg-gen-c.py — installed.json → user-fns.c 재생성 (P2.1 구조)

사용: python3 fl-pkg-gen-c.py <fx2_dir>
  - runtime/installed.json 에서 설치 목록 읽기
  - packages/<name>/metadata.json 에서 C body 읽기
  - runtime/user-fns.c 재생성

출력 형식: JSON {"ok": bool, "count": int, "packages": [...], "error": str|null}
"""
import sys, json, os

HEADER = """\
/**
 * freelang-v11-fx2 — 사용자 패키지 함수
 * fl-pkg-gen-c.py가 installed.json에서 자동 생성합니다.
 * 직접 수정하지 마세요.
 */
#include "runtime.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

"""

def main():
    if len(sys.argv) < 2:
        print(json.dumps({"ok": False, "error": "사용: fl-pkg-gen-c.py <fx2_dir>"}))
        sys.exit(1)

    fx2_dir      = sys.argv[1]
    packages_dir = os.path.join(fx2_dir, "packages")
    runtime_dir  = os.path.join(fx2_dir, "runtime")
    installed_path = os.path.join(runtime_dir, "installed.json")
    user_fns_path  = os.path.join(runtime_dir, "user-fns.c")

    # 설치 목록 읽기
    if not os.path.exists(installed_path):
        installed = []
    else:
        try:
            installed = json.load(open(installed_path))
        except Exception as e:
            print(json.dumps({"ok": False, "error": f"installed.json 읽기 실패: {e}"}))
            sys.exit(1)

    # 패키지 C body 수집
    fn_sections = []   # (name, arity, body)
    missing     = []
    for pkg_name in installed:
        meta_path = os.path.join(packages_dir, pkg_name, "metadata.json")
        if not os.path.exists(meta_path):
            # fallback: 구형 flat JSON
            flat_path = os.path.join(packages_dir, f"{pkg_name}.json")
            if os.path.exists(flat_path):
                meta_path = flat_path
            else:
                missing.append(pkg_name)
                continue
        try:
            pkg = json.load(open(meta_path))
            fn_sections.append((pkg_name, pkg.get("args", 1), pkg.get("body", "")))
        except Exception as e:
            missing.append(pkg_name)

    # user-fns.c 생성
    out = [HEADER, "/* FL:USER_SECTION:BEGIN */"]
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
        c_name    = "ufl_" + name.replace("-", "_")
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

    try:
        with open(user_fns_path, "w") as f:
            f.write("\n".join(out) + "\n")
    except Exception as e:
        print(json.dumps({"ok": False, "error": f"user-fns.c 쓰기 실패: {e}"}))
        sys.exit(1)

    result = {
        "ok": True,
        "count": len(fn_sections),
        "packages": [s[0] for s in fn_sections],
        "missing": missing,
        "error": None
    }
    print(json.dumps(result))


if __name__ == "__main__":
    main()
