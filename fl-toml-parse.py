#!/usr/bin/env python3
"""fx.toml 파서 — [project] / [runtime] 섹션만 지원 → JSON 출력"""
import sys, re, json, os

def parse_toml(text):
    data = {}
    section = None
    i = 0
    lines = text.splitlines()
    while i < len(lines):
        line = lines[i].strip()
        # 빈 줄 / 주석
        if not line or line.startswith('#'):
            i += 1; continue
        # 섹션 헤더
        m = re.match(r'^\[([^\]]+)\]$', line)
        if m:
            section = m.group(1).strip()
            data.setdefault(section, {})
            i += 1; continue
        if section is None:
            i += 1; continue
        # key = value
        m = re.match(r'^(\w[\w\-]*)(\s*)=(\s*)(.*)', line)
        if not m:
            i += 1; continue
        key = m.group(1)
        val_raw = m.group(4).strip()
        # 인라인 배열 (multi-line 포함)
        if val_raw.startswith('['):
            arr_str = val_raw
            while ']' not in arr_str:
                i += 1
                if i >= len(lines): break
                arr_str += ' ' + lines[i].strip()
            # 배열 파싱
            inner = re.sub(r'^\[|\]$', '', arr_str.strip()).strip()
            if inner:
                items = [s.strip().strip('"').strip("'")
                         for s in re.split(r',', inner) if s.strip().strip('"').strip("'")]
            else:
                items = []
            data[section][key] = items
        # 문자열
        elif val_raw.startswith('"') or val_raw.startswith("'"):
            q = val_raw[0]
            data[section][key] = val_raw.strip(q)
        # 정수/숫자
        elif re.match(r'^\d+$', val_raw):
            data[section][key] = int(val_raw)
        else:
            data[section][key] = val_raw
        i += 1
    return data

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else 'fx.toml'
    if not os.path.exists(path):
        print(json.dumps({})); return
    text = open(path).read()
    data = parse_toml(text)
    print(json.dumps(data, ensure_ascii=False))

if __name__ == '__main__':
    main()
