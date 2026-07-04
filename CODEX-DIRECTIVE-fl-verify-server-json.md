# 지시서 — fl-verify에 "server-json 맵 직접전달" 함정 규칙 추가

**발행**: 2026-07-04 (Claude Code)
**배경**: fx2가 오늘 완성 선언됐다. 다음 단계는 기능 추가가 아니라 "AI가 실수하기
쉬운 표현을 gcc 전에 미리 잡아주는" AI-safe 게이트 강화다. 이 지시서는 그 첫
사례 하나를 실제로 박아넣는 작업이다.

## 재현된 실제 사고 (오늘 두 번 반복됨)

1. 오늘 오전 fl-common/fx2.fl P2 검증 중 발견: `(server-json {"ok" true ...})`처럼
   **맵을 직접** `server-json`에 넘기면 body가 비어버림. `runtime/http.c:168`의
   `server_json(FLValue json_str)`은 인자를 이미 문자열이라고 가정하고 `strval()`만
   호출 — 맵이 오면 조용히 빈 문자열이 됨. 해결책은 `fx-server-json`(맵/벡터면
   자동 json_stringify, `runtime/aliases.c` 근처 `fx_server_json` 참고) 사용.
2. 오늘 오후, 코덱스에게 별도 세션으로 fx2 URL 단축기 앱을 새로 만들게 시켰는데
   **똑같은 함정에 또 걸렸다** — `(server-json $row)`로 맵을 직접 넘겨서 body가
   비어있었음(HTTP 200, Content-Length: 0). 같은 날 같은 코드베이스를 다루면서도
   재발했다는 게 이 작업의 존재 이유다: 사람이 매번 발견하는 대신 컴파일 전에
   자동으로 잡아야 한다.

## 할 일

`spec/fx.rules`(fl-rules-runner.fl이 로드하는 규칙 파일, 이미 있는 파일 —
`_line-has`, `check-error` 헬퍼 그대로 재사용)에 새 규칙을 추가한다.

패턴: `(check-error "규칙이름" (fn [$src $lines] ...))` — 이미 파일에 있는
"range 단인자 금지", "str-includes는 boolean" 규칙을 그대로 참고해서 스타일 맞출 것.

### 새 규칙 — server-json에 맵 리터럴/변수 직접 전달 감지

```lisp
;; 금지: server-json에 맵을 직접 전달 (body가 비어버림 — fx-server-json 써야 함)
(check-error
  "server-json 맵 직접전달 금지"
  (fn [$src $lines]
    (if (_line-has $lines "(server-json {")
      "server-json does not accept a map literal directly and silently returns an empty body. Use fx-server-json instead: (fx-server-json {...})"
      nil)))
```

**주의**: 위는 `(server-json {`(맵 리터럴이 바로 뒤따르는 경우)만 잡는 최소
버전이다. 실제로는 `(server-json $row)`처럼 **변수를 통해** 맵을 넘기는 경우가
더 흔하고 더 위험하다(오늘 두 사고 다 이 패턴이었음). 하지만 `$row`가 진짜
맵인지 문자열인지는 텍스트 패턴만으로는 확정 불가 — 아래 두 가지 중 하나로
확장할 것:

1. **휴리스틱 확장(권장, 빠름)**: 같은 함수/파일 안에서 `$row`가
   `(let [$row (...)]`처럼 바인딩되고 그 우변이 맵 리터럴(`{`로 시작)이거나
   맵을 반환하는 걸로 보이는 패턴이면 경고. 오탐 가능성 있으니 `check-error`
   대신 `check-warn`(경고, 빌드는 막지 않음)으로 시작할 것.
2. **런타임 방어(대안, 더 견고함)**: `runtime/http.c`의 `server_json` 자체를
   고쳐서, 인자가 `FL_MAP`/`FL_VECTOR`면 자동으로 `json_stringify`를 거치게
   만드는 방법도 있다 — 사실상 `server_json`을 `fx_server_json`과 동일하게
   만드는 것. **이게 근본 해결책에 가깝다.** 다만 이건 런타임 동작 변경이라
   fl-verify 규칙 추가보다 범위가 크다 — 이번 작업 범위에 넣을지는 판단해서
   진행하되, 넣는다면 반드시: (a) 기존 `(server-json <string>)` 정상 케이스가
   깨지지 않는지 회귀 테스트, (b) conformance/ssr-app/search-app 재확인.

**권장 순서**: 1번(check-warn 휴리스틱)을 먼저 하고, 여유 있으면 2번(런타임
방어)까지. 최소 완료 기준은 1번이다.

## 검증 (실제로 걸리는지 직접 재현할 것 — 자기 보고만 하지 말 것)

### Case A — 잡아야 하는 것 (아래 두 소스로 각각 확인)

```lisp
;; /tmp/verify_bad1.fl
(server-json {"ok" true})
```
```lisp
;; /tmp/verify_bad2.fl (오늘 실제 버그와 동일한 패턴)
(defn handle [$req]
  (let [$row {"code" "abc" "url" "https://x.com"}]
    (server-json $row)))
```

```bash
cd /home/kimjin/freelang-v11-fx2
node /home/kimjin/freelang-v11/bootstrap.js run fl-rules-runner.fl spec/fx.rules /tmp/verify_bad1.fl
node /home/kimjin/freelang-v11/bootstrap.js run fl-rules-runner.fl spec/fx.rules /tmp/verify_bad2.fl
```

`verify_bad1.fl`은 위 최소 규칙으로 바로 잡혀야 한다(`error:...:server-json does
not accept...`). `verify_bad2.fl`은 휴리스틱 확장을 했다면 최소 warn이라도 떠야
한다 — 확장을 안 했다면 이 케이스는 못 잡는 게 정상이니 "못 잡음, 이유: 변수
경유라 정적 패턴으로 확정 불가"라고 정직하게 기록할 것.

### Case B — 잡으면 안 되는 것 (오탐 금지, 반드시 확인)

```lisp
;; /tmp/verify_good.fl — 정상 패턴, 규칙에 안 걸려야 함
(server-json (json-stringify {"ok" true}))
(fx-server-json {"ok" true})
```

```bash
node /home/kimjin/freelang-v11/bootstrap.js run fl-rules-runner.fl spec/fx.rules /tmp/verify_good.fl
```
아무 에러/경고도 없이 조용히 끝나야 한다. 만약 여기서 규칙이 오발동하면
정규식/패턴을 좁혀야 한다 — 오탐 나는 채로 완료 보고 금지.

### Case C — 실전 회귀 (오늘 만든 실제 앱으로)

```bash
# 오늘 실제로 버그 있었던 버전 재현용 백업이 있다면 그걸로, 없다면 아래처럼
# 일부러 버그 버전을 만들어서 규칙이 잡는지 최종 확인
cp /home/kimjin/kim/Desktop/kim/01_Active_Projects/fx2-url-shortener/app.fl /tmp/urlshortener_test.fl
sed -i 's/(fx-server-json \$row)/(server-json $row)/' /tmp/urlshortener_test.fl
node /home/kimjin/freelang-v11/bootstrap.js run fl-rules-runner.fl spec/fx.rules /tmp/urlshortener_test.fl
# 그 다음 정상본(fx-server-json 그대로)도 규칙 통과하는지 확인
node /home/kimjin/freelang-v11/bootstrap.js run fl-rules-runner.fl spec/fx.rules /home/kimjin/kim/Desktop/kim/01_Active_Projects/fx2-url-shortener/app.fl
```

## 완료 기준

- [ ] spec/fx.rules에 새 규칙 추가 (check-error 또는 check-warn)
- [ ] Case A 최소 케이스(맵 리터럴 직접 전달) 잡힘 확인 — 실행 로그 그대로 첨부
- [ ] Case B 정상 패턴 오탐 없음 확인 — 실행 로그 그대로 첨부
- [ ] Case C 실전 회귀 확인 — 실행 로그 그대로 첨부
- [ ] (선택) runtime/http.c server_json 자체 방어 로직 추가 시 conformance/
      ssr-app/search-app 재확인 결과 첨부
- [ ] 에러 메시지가 "무엇이 문제고 뭘 써야 하는지"(fix-instruction 형태)인지
      스스로 재확인 — "규칙 위반" 같은 모호한 메시지면 재작성
- [ ] 커밋은 하지 말 것 (검증 후 Claude Code가 커밋함)

## 보고 형식

작업 끝나면 다음을 그대로 출력:
1. spec/fx.rules에 추가한 diff
2. Case A/B/C 각각의 실제 커맨드 + 실제 출력 (요약 말고 그대로)
3. 겪은 문제가 있었다면 그대로 (없으면 "없음"이라고 명시, 지어내지 말 것)
