#!/usr/bin/env python3
"""fl build --plan / --check / --graph"""
import sys, os, re, json, subprocess, urllib.request

def collect_loads(path, base, visited=None, cycle_stack=None):
    if visited is None: visited = set()
    if cycle_stack is None: cycle_stack = []
    abs_path = os.path.abspath(path)
    if abs_path in cycle_stack:
        print(f"[fl-build] ⚠️  순환 의존 감지: {os.path.relpath(abs_path, base)}", file=sys.stderr)
        return []
    if abs_path in visited: return []
    visited.add(abs_path)
    base_dir = os.path.dirname(abs_path)
    try: content = open(abs_path).read()
    except: return []
    result = []
    for line in content.splitlines():
        m = re.match(r'\s*\(load\s+"([^"]+)"\)', line) or \
            re.match(r"\s*\(load\s+'([^']+)'\)", line)
        if m:
            lp = m.group(1)
            if not os.path.isabs(lp):
                lp = os.path.join(base_dir, lp)
            rel = os.path.relpath(lp, base)
            result.append(rel)
            result.extend(collect_loads(lp, base, visited, cycle_stack + [abs_path]))
    return result

def get_module(fl_input, script_dir):
    r = subprocess.run(
        ["python3", os.path.join(script_dir, "fl-module-parse.py"), fl_input],
        capture_output=True, text=True
    )
    return json.loads(r.stdout) if r.returncode == 0 else {}

def resolve_profiles(profiles, script_dir):
    profiles_dir = os.path.join(script_dir, "profiles")
    all_packages = []
    seen = set()
    detail = {}
    for pname in profiles:
        pfile = os.path.join(profiles_dir, f"{pname}.json")
        if os.path.exists(pfile):
            pkgs = json.load(open(pfile)).get("packages", [])
            added = [p for p in pkgs if p not in seen]
            seen.update(added); all_packages.extend(added)
            detail[pname] = pkgs
        else:
            detail[pname] = ["(프로파일 없음)"]
    return all_packages, detail

fl_input  = sys.argv[1]
script_dir = sys.argv[2]
output    = sys.argv[3] if len(sys.argv) > 3 else os.path.basename(fl_input).replace('.fl','')
mode      = sys.argv[4] if len(sys.argv) > 4 else "plan"

base_dir = os.path.dirname(os.path.abspath(fl_input))
mod = get_module(fl_input, script_dir)
profiles = mod.get("use", [])
all_packages, profile_detail = resolve_profiles(profiles, script_dir)
loads = collect_loads(fl_input, base_dir)

# ── --plan ──────────────────────────────────────────────
if mode == "plan":
    if mod:
        print(f"module:  {mod.get('name','')}  v{mod.get('version','')}")
        print()
    if profiles:
        print("profiles:")
        for p in profiles:
            pkgs = profile_detail.get(p, [])
            print(f"  :{p:<8} → [{', '.join(pkgs)}]")
        print()
    if loads:
        print("loads:")
        for l in loads:
            print(f"  → {l}")
        print()
    print(f"packages: {len(all_packages)}개")
    print(f"output:   {output}  (ELF)")

# ── --check ─────────────────────────────────────────────
elif mode == "check":
    FSM_URL = "http://localhost:40330"
    print(f"🔍 declared vs installed 검사: {os.path.basename(fl_input)}")
    print()
    try:
        url = f"{FSM_URL}/fx/module/check?file={urllib.parse.quote(os.path.abspath(fl_input))}"
        import urllib.parse
        url = f"{FSM_URL}/fx/module/check?file={urllib.parse.quote(os.path.abspath(fl_input))}"
        with urllib.request.urlopen(url, timeout=3) as r:
            data = json.load(r)
        ok = data.get("ok", False)
        declared = data.get("declared", [])
        missing = data.get("missing", [])
        installed_count = data.get("installed_count", 0)
        profiles_result = data.get("profiles", [])

        print(f"module:  {data.get('module','?')}")
        print(f"declared profiles: {declared}")
        print(f"installed packages: {installed_count}개")
        print()
        for pr in profiles_result:
            status = "✅" if pr.get("ok") else "❌"
            miss = pr.get("missing", [])
            print(f"  {status} :{pr['profile']}", end="")
            if miss:
                print(f"  — missing: {miss}", end="")
            print()
        print()
        if ok:
            print("✅ 모든 선언된 패키지 설치됨")
        else:
            print(f"❌ 누락 패키지: {missing}")
            sys.exit(1)
    except Exception as e:
        # fl-source-manager 없으면 로컬에서 직접 확인
        packages_dir = os.path.join(script_dir, "packages")
        print(f"(fl-source-manager 없음, 로컬 확인)")
        all_ok = True
        for p in all_packages:
            exists = os.path.exists(os.path.join(packages_dir, f"{p}.json"))
            status = "✅" if exists else "❌"
            print(f"  {status} {p}")
            if not exists: all_ok = False
        print()
        if all_ok:
            print("✅ 모든 패키지 로컬 확인됨")
        else:
            print("❌ 누락 패키지 있음")
            sys.exit(1)

# ── --graph ─────────────────────────────────────────────
elif mode == "graph":
    name = mod.get("name", os.path.basename(fl_input))
    print(f"{os.path.basename(fl_input)}")
    if profiles:
        for i, p in enumerate(profiles):
            pkgs = profile_detail.get(p, [])
            branch = "└──" if (i == len(profiles)-1 and not loads) else "├──"
            print(f"{branch} :{p}  [{', '.join(pkgs)}]")
    if loads:
        for i, l in enumerate(loads):
            branch = "└──" if i == len(loads)-1 else "├──"
            print(f"{branch} {l}")
    print()
    print(f"packages: {len(all_packages)}개  output: {output} (ELF)")
