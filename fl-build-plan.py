#!/usr/bin/env python3
"""fl build --plan: 의존성/파일 구조 출력 (빌드 없이)"""
import sys, os, re, json, subprocess

def collect_loads(path, base, visited=None):
    if visited is None: visited = set()
    abs_path = os.path.abspath(path)
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
            result.extend(collect_loads(lp, base, visited))
    return result

fl_input  = sys.argv[1]
script_dir = sys.argv[2]
output     = sys.argv[3] if len(sys.argv) > 3 else os.path.basename(fl_input).replace('.fl','')

base_dir   = os.path.dirname(os.path.abspath(fl_input))
profiles_dir = os.path.join(script_dir, "profiles")

# module 파싱
r = subprocess.run(
    ["python3", os.path.join(script_dir, "fl-module-parse.py"), fl_input],
    capture_output=True, text=True
)
mod = json.loads(r.stdout) if r.returncode == 0 else {}

profiles = mod.get("use", [])
all_packages = []
seen = set()
profile_detail = {}
for pname in profiles:
    pfile = os.path.join(profiles_dir, f"{pname}.json")
    if os.path.exists(pfile):
        pkgs = json.load(open(pfile)).get("packages", [])
        added = [p for p in pkgs if p not in seen]
        seen.update(added)
        all_packages.extend(added)
        profile_detail[pname] = pkgs
    else:
        profile_detail[pname] = ["(프로파일 없음)"]

loads = collect_loads(fl_input, base_dir)

# ── 출력 ──
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
