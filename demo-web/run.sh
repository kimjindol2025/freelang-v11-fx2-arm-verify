#!/bin/bash
# 한 줄로 끝:
#   git clone gogs.dclub.kr/kim/freelang-v11-fx2
#   cd freelang-v11-fx2/demo-web
#   bash run.sh
bash "$(dirname "$0")/../fl-run.sh" "${1:-run}"
