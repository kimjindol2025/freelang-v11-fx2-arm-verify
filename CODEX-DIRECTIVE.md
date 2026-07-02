# CODEX 작업 지시서 — fx2 안정화

**발행**: 2026-07-02 (Claude Code 검증 세션 기반)
**전제**: CODEX-MEMORY.md의 작업 규칙을 따른다. 이 문서는 "무엇을"이고, MEMORY는 "어떻게"다.

---

## 미션

fx2 = **v11 문법 + 단일 바이너리 배포**. 이 결합이 완성되면 fx는 은퇴하고,
신규 경량 서비스는 fx2로 간다. 지금 막는 것은 빌드 파이프라인 버그와
런타임 함정이지 설계가 아니다.

## 현재 검증된 것 (2026-07-02 실측 — 재검증 불필요)

- ssr-app E2E 전체 통과: `/` SSR, `/api/state`, `/tasks/add|toggle|clear` (커밋 40fc138)
- `atom`/`swap!` + `fn`(defn 파라미터 캡처) + 재귀 순회 + `html-escape` 정상
- 이 노드(aarch64)에서 `CGC_BIN=/root/freelang-v11/bin/cgc-bin.bak` 빌드 성공 (248K ELF)
- 패키지 14개(str/math/time/path) 동작 — 253에서 검증됨

---

## P0 — fl-build.sh 버그 2개 (재현 확정, 미수정)

### P0-1. app.o 컴파일 실패를 삼키고 링크 강행

`fl-build.sh` app.o 컴파일 블록: gcc 실패 시 에러 로그를
`fl_gcc_placeholder`로 옮기기만 하고 **링크 단계로 그냥 진행**한다.
결과: 진짜 에러 대신 `cannot find /tmp/fl_app_cache/<name>.o` 라는
엉뚱한 링커 에러만 보인다.

**수정 방향**: app.o 실패 시 즉시 원본 gcc 로그 출력 + exit 1.
placeholder 우회 제거.

### P0-2. `--no-net`이 libfx.a에 stub을 실제로 안 넣음

`--no-net` 시 `NET_LIBS`는 비우지만, libfx.a 빌드 SRCS에서
`http.c/http_client.c/crypto`를 stub(`http-stub.c` 등)으로 교체하는
분기가 없다. 결과: `--no-net`인데 `SSL_accept undefined reference`.

**수정 방향**: `NO_NET=1`일 때 SRCS에서 실물 3개 제외 + stub 3개 포함.
libfx.a 해시에 NO_NET이 이미 들어가 있으니 캐시는 자동 분리된다.

**P0 완료 기준**: `--no-net` 빌드가 openssl/curl 없는 환경에서
한 번에 통과하고, app.o 실패 시 FL 소스 줄번호가 보이는 에러가 나온다.

---

## P1 — 클로저 함정: fix 또는 컴파일 타임 거부

`fn`이 outer `let` 바인딩을 캡처하지 못한다 (defn 파라미터만 가능).
지금은 **조용히 틀린 값**이 나온다 — FL 최악 패턴(조용한 실패)이다.

우선순위: ① 캡처 구현(정공법) → 어려우면 ② cgc 단계에서
"fn이 let 바인딩 참조" 감지 시 **컴파일 에러**로 격상.
조용한 오동작만은 제거한다.

## P2 — fl-common/fx2.fl 실기 검증

`/root/kim/fl-common/fx2.fl` (gogs kim/fl-common)에 공통 API가 작성돼
있으나 fx2에서 미검증. Result 타입, safe-nth, require-*, res-*,
safe-tick-interval, db-one, tpl-render 전부 빌드+실행으로 확인하고
안 맞는 것은 fx2.fl 쪽을 고쳐 fl-common에 push.

---

## 검증 규율 (이번 세션에서 피 본 것)

1. **서버 검증 전 `ps aux | grep <bin이름>` 필수.**
   graceful shutdown 30s 때문에 kill 직후에도 프로세스가 살아 있고,
   SO_REUSEPORT로 **여러 프로세스가 같은 포트를 나눠 받는다.**
   실제로 "toggle이 태스크를 삭제한다"는 유령 버그를 3번 추적하게 만든 원인.
   증상이 이상하면 코드보다 관측 환경(잔존 프로세스)부터 의심할 것.
2. 검증 서버는 시작 전에 이전 바이너리 전부 kill(PID 지정, `pkill -f` 금지 — 자기 세션 죽는다).
3. 빌드 산출물(`runtime/libfx.a`, `runtime/.libfx_hash`, `/tmp/fl_app_cache/*`)은 커밋 금지.
4. 통과 선언은 E2E 실행 결과로만. 빌드 성공 ≠ 동작.

## 금지

- `/root/freelang-v11` (운영 v11) 수정 금지. `cgc-bin.bak`은 읽기 전용으로만 사용.
- 검증 실패 2회 → 해당 단계 롤백 후 처음부터 (부분 수정 금지).
- 원격 push 전 `git ls-remote origin master`로 HEAD 확인 (다른 작업자 병행 중).

## 완료 정의 (전체)

- [ ] P0-1, P0-2 수정 + 재현 케이스가 통과하는 검증 스크립트 동봉
- [ ] P1: 클로저 캡처 구현 **또는** 컴파일 에러 격상 (조용한 오동작 0)
- [ ] P2: fl-common/fx2.fl 전 함수 실기 검증 로그
- [ ] 각 단계 Gogs 커밋 + 블로그(blog.dclub.kr) 기록
