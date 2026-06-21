#!/usr/bin/env python3
"""fl-str-split.py — 전처리된 FL 파일에서 900B 초과 문자열 리터럴을 (str ...) 로 자동 분할"""
import sys, re

LIMIT = 900

def split_long_strings(src):
    def replacer(m):
        s = m.group(1)
        if len(s.encode()) <= LIMIT:
            return m.group(0)
        parts = []
        while s:
            parts.append(s[:LIMIT])
            s = s[LIMIT:]
        inner = " ".join('"' + p + '"' for p in parts)
        return "(str " + inner + ")"

    result = []
    count = 0
    for line in src.splitlines():
        # 이미 (str "...") 형태면 스킵
        if re.match(r'\s*\(str\s+"', line):
            result.append(line)
            continue
        new_line = re.sub(r'"((?:[^"\\]|\\.){900,})"', replacer, line)
        if new_line != line:
            count += 1
        result.append(new_line)
    if count:
        print(f"[fl-build] 긴 문자열 {count}개 자동 분할 (>{LIMIT}B)", file=sys.stderr)
    return "\n".join(result)

if __name__ == "__main__":
    path = sys.argv[1]
    src = open(path).read()
    result = split_long_strings(src)
    open(path, "w").write(result)
