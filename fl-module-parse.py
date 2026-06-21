#!/usr/bin/env python3
"""FL 파일에서 (module ...) form 파싱 → JSON 출력"""
import sys, json, re

def parse_module_form(src):
    # 주석 줄 제거
    clean_lines = []
    for line in src.splitlines():
        if line.lstrip().startswith(';'):
            continue
        clean_lines.append(line)
    src = '\n'.join(clean_lines)

    # (module name ...) 블록 추출 — 중첩 괄호 대응
    start = src.find('(module')
    if start == -1:
        return None

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

    block = src[start:end + 1]

    # 이름 추출
    name_m = re.match(r'\(module\s+(\S+)', block)
    if not name_m:
        return None
    name = name_m.group(1)

    result = {"name": name, "use": [], "version": "1.0.0"}

    # :use [...] 추출
    use_m = re.search(r':use\s+\[(.*?)\]', block, re.DOTALL)
    if use_m:
        profiles = re.findall(r':(\w[\w-]*)', use_m.group(1))
        result["use"] = profiles

    # :version "..." 추출
    ver_m = re.search(r':version\s+"([^"]+)"', block)
    if ver_m:
        result["version"] = ver_m.group(1)

    return result


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(1)
    try:
        src = open(sys.argv[1]).read()
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    result = parse_module_form(src)
    if result:
        print(json.dumps(result))
    else:
        sys.exit(1)
