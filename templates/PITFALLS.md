# fx2 함정 목록 (2026-07-04 기준 실측)

새 앱을 fx2로 짜기 전에 읽을 것. 전부 오늘 실제로 겪고 고친 것들이다
(지어낸 게 아니라 재현 명령까지 남아있음 — CODEX-DIRECTIVE.md 참고).

## 1. 이름 충돌 — 변수명이 내장 함수와 겹치면 링크 에러

- `$first`, `$ok?`, `$err?` 같은 이름을 변수로 쓰면 안 된다. `first`,
  `ok?`(→`ok_p`, HTTP 상태체크용 내장) 등과 심볼이 겹쳐서
  `multiple definition` 링크 에러가 난다.
- Result 타입을 직접 만들 땐 `ok?`/`err?` 대신 `result-ok?`/`result-err?`
  같은 이름을 써라 (fl-common/fx2.fl도 이렇게 고쳐져 있음).
- 새 변수명을 지을 때 `grep -n "FLValue <이름>\b" runtime/aliases.c
  runtime/*.c`로 먼저 겹치는지 확인하는 습관을 들일 것.

## 2. try/catch, set_interval — 아예 없음

- `try`/`catch`를 쓰면 `[CGC-ERR] unsupported IR kind=try`로 컴파일이
  깨진다. Result 타입 패턴(`{"ok" bool "value"/"error" ...}`)으로 대체.
- `set_interval`(타이머)도 없다. 주기적 작업이 필요하면 지금은 fx2로
  못 만든다 — 이 제약이 풀리기 전엔 다른 방법을 써라.

## 3. dead code elimination이 없다

- `(load "lib.fl")` 하면 그 파일 안의 **모든 함수가 호출 여부와 무관하게
  컴파일**된다. lib 안에 컴파일 안 되는 함수가 하나만 있어도 그 파일
  전체를 load조차 못 한다. lib를 만들 땐 함정 1·2를 반드시 피할 것.

## 4. server-json — 이제 안전함 (2026-07-04부터)

- 예전엔 `(server-json {맵})`처럼 맵을 직접 넘기면 body가 조용히
  비어버렸다. **지금은 고쳐져서 안전하다** — `runtime/http.c`의
  `server_json`이 맵/벡터를 자동으로 `json_stringify` 한다.
  `fx-server-json`은 하위호환용으로 남아있지만 이제 안 써도 된다.

## 5. json-parse / json-stringify — 이제 진짜 동작함 (2026-07-04부터)

- 오늘 오전까지 `self/codegen-c.fl`에 "S18에서 실 구현" 미완성 stub이
  남아있어서 `json-parse`가 파싱을 안 하고 원본 문자열을 그냥 통과시켰다.
  지금은 고쳐져서 정상 동작한다. `json-try-parse`(에러를 Result로 감싸는
  버전)도 계속 정상.

## 6. json_stringify — 제어문자 이스케이프 (2026-07-04부터 완전해짐)

- `pm2 list` 같은 ANSI 색상 출력을 그냥 json-stringify하면 예전엔
  제어문자(ESC 등)를 이스케이프 안 해서 클라이언트에서 JSON 파싱 실패가
  났다. 지금은 0x00-0x1F 전부 `\u00XX`로 이스케이프한다.

## 7. server_static 없음 — server-file로 파일 하나씩

- v11 FL-Front의 `server_static`(디렉토리 전체 마운트)과 헷갈리지 말 것.
  **fx2엔 그런 함수가 없다.** 정적 파일은 `server-file "path"`로 라우트를
  하나씩 등록해야 한다 (`templates/static-page/app.fl` 참고).
- `server-file`의 MIME 타입 감지가 `.html`/`.json`/`.txt`/`.xml`을
  놓치고 있었던 것도 오늘 고쳤다(`runtime/http.c: mime_of`) — 최신
  cgc-bin/libfx.a 기준이면 문제없음.

## 8. 클로저 캡처 / 연산자 first-class / defn 다중본문 / 맵 키 충돌

- 전부 2026-07-04에 수정 완료 확인됨 (패치 #1/#2/#3/#5). 지금 버전
  기준으로는 걱정 안 해도 된다. 오래된 백업 cgc-bin을 실수로 다시 쓰면
  재발할 수 있으니 `bin/cgc-bin` 타임스탬프를 확인할 것.

## 9. 동시 세션 작업 시 git 주의

- 이 저장소는 여러 세션(Claude/Codex)이 동시에 작업할 수 있다.
  `git add -A`로 커밋하면 다른 세션의 미커밋 변경까지 같이 쓸려 들어갈
  수 있다 — 반드시 `git status`/`git diff --stat`로 스테이징될 파일을
  확인하고 명시적으로 `git add <path>`. 커밋 전엔 `git fetch` +
  `git merge-base --is-ancestor origin/master HEAD`로 원격 확인.

---

**템플릿**: `templates/http-json-api/`, `templates/static-page/` —
둘 다 위 함정을 전부 피해서 짰고, 실제로 빌드+실행+curl까지 검증됨.
새 앱은 여기서 복사해서 시작할 것.
