# fx2 언어 상태 및 Codex 인수인계

기준일: 2026-08-11

이 문서는 fx2의 현재 구현 수준을 다음 Codex가 빠르게 파악하고 이어서 작업할 수 있도록 정리한 상태 문서다.

## 결론

fx2는 단순한 문법 실험 단계가 아니다. 언어 코어, 클로저, 컬렉션, 네이티브 컴파일, GC/RC 기반 런타임, HTTP 서버, SSR 예제, 패키지 시스템, 검증 명령이 이미 구현되어 있다.

현재의 주요 공백은 서버 언어 기능이 아니라 다음 개발 표면이다.

- 브라우저 JavaScript/WASM 컴파일 타깃
- DOM과 Web API 런타임 바인딩
- LSP, formatter, 디버거, 자동완성
- 정적 타입 또는 선택적 타입 검사
- 더 넓은 패키지 생태계와 안정적인 버전 해결

따라서 fx2는 현재 **네이티브 백엔드·SSR·CLI·데이터 처리 언어**로 평가하는 것이 정확하며, 브라우저 언어로 확장하려면 별도 타깃을 추가해야 한다.

## 구현 근거

### 언어 코어

[`SPEC.md`](../SPEC.md)는 다음을 정의한다.

- `define`, `defn`, `fn`, `let`, `if`, `cond`, `try/catch`, `do`
- 클로저와 고차함수
- `map`, `filter`, `reduce`, `apply`
- list, vector, map
- `atom`, `deref`, `reset!`, `swap!`
- `->`, `->>` threading macro
- truthiness와 기본 타입 규칙
- int64, IEEE 754 double, UTF-8 문자열
- 패키지 선언과 빌드 형식

실행 파이프라인은 다음과 같다.

```text
program.fl
  -> cgc-bin
  -> C source
  -> gcc
  -> native ELF executable
```

근거: [`INTRODUCTION.md`](../INTRODUCTION.md), [`fl-build.sh`](../fl-build.sh)

### 런타임

[`runtime/runtime.h`](../runtime/runtime.h)에 다음 ABI와 런타임 계층이 정의되어 있다.

- `FLValue`와 기본 값 타입
- vector/map
- closure 환경
- 고차함수
- heap retain/release
- 산술·비교·논리
- 파일 I/O
- HTML escape
- JIT 관련 인터페이스

HTTP 기능은 [`runtime/http.c`](../runtime/http.c)에 구현되어 있다.

- GET, POST, PUT, PATCH, DELETE 라우트
- HTML, text, JSON, status, redirect 응답
- route parameter, query, body, header
- 동시 연결과 요청 크기 제한

그 외에도 다음 런타임 모듈이 존재한다.

- `sqlite.c`, `mariadb.c`
- `json.c`
- `regex.c`
- `http_client.c`
- `websocket.c`, `sse.c`
- `process.c`, `net.c`
- `pdf_img.c`, `pdf_ttf.c`

### 웹과 SSR

fx2에는 실제 SSR 애플리케이션 예제가 있다.

- [`ssr-app/app.fl`](../ssr-app/app.fl): 상태, 폼, HTML 렌더링, HTTP 처리
- [`search-app/app.fl`](../search-app/app.fl): 검색형 서버 앱
- [`demo-web/app.fl`](../demo-web/app.fl): 웹 서버 데모

현재 웹 지원은 브라우저에서 fx2를 실행하는 방식이 아니라 다음 형태다.

```text
fx2 -> native HTTP server -> HTML/JSON -> browser
```

따라서 SSR/API는 이미 구현 영역이고, DOM을 직접 조작하는 CSR은 별도 구현 영역이다.

### 패키지와 레지스트리

패키지 메타데이터와 소스가 [`packages/`](../packages)에 있다.

현재 확인되는 패키지 범주:

- 문자열: `str-lines`, `str-count`, `str-truncate`, `str-indent`, `str-rpad`
- 수학: `math-clamp`, `math-lerp`, `math-round-n`, `math-sign`
- 시간: `time-now-ms`, `time-elapsed`
- 경로: `path-basename`, `path-dirname`

패키지 등록·설치·테스트 로직은 [`fx2-registry/modules.fl`](../fx2-registry/modules.fl)에 있다.

현재 생태계가 없는 것은 아니지만 패키지 수와 배포 안정성은 아직 초기 단계다. 특히 레지스트리 구현은 `shell-run`, 로컬 경로, 외부 HTTP 서비스에 의존하므로 운영 배포 전에는 경로·권한·실패 처리를 보강해야 한다.

### CLI와 검증

[`fl`](../fl)은 다음 명령을 제공한다.

- `fl new`
- `fl build`
- `fl check`
- `fl plan`
- `fl graph`
- `fl profile list`
- `fl profile add`

검증 진입점은 [`tools/fl-verify.sh`](../tools/fl-verify.sh)이고, 언어 기능 검증은 [`spec/conformance.fl`](../spec/conformance.fl)에 있다.

이것은 “빌드 도구가 없다”는 상태가 아니다. 다만 다음 도구는 별도 구현 근거가 확인되지 않았다.

- LSP 서버
- formatter
- IDE 확장
- 소스 수준 디버거
- 정적 타입 검사기

## 현재 분류

| 항목 | 분류 | 상태 |
|---|---|---|
| 함수·클로저·컬렉션 | 언어 기능 | 구현됨 |
| 네이티브 컴파일 | 컴파일러/빌드 | 구현됨 |
| HTTP/SSR | 런타임/API | 구현됨 |
| SQLite/MariaDB/JSON | 런타임 | 구현됨 또는 연결부 존재 |
| 패키지 | 생태계 | 초기 구현 |
| `fl check`/conformance | 검증 | 구현됨 |
| DOM | 런타임 타깃 | 미확인/미구현 |
| JavaScript/WASM 출력 | 컴파일 타깃 | 미확인/미구현 |
| LSP/formatter/debugger | 개발 도구 | 미확인/미구현 |
| 정적 타입 | 언어 기능 | 현재 동적 타입 |

## 다음 Codex 작업 순서

브라우저 기능을 목표로 한다면 전체 프론트를 바로 이식하지 말고 다음 순서로 진행한다.

1. fx2로 HTML 문자열을 반환하는 최소 SSR 페이지를 만든다.
2. `server_json`을 사용하는 코드 검사 API를 만든다.
3. 기존 `freelang-front`의 `/playground` 화면을 fx2 SSR/API에 연결한다.
4. 브라우저 동작이 필요할 때만 작은 JavaScript 어댑터를 추가한다.
5. 이후 fx2의 WASM/JavaScript 타깃과 DOM 바인딩을 별도 프로젝트로 설계한다.

브라우저 타깃을 구현할 때 필요한 최소 API는 다음과 같다.

- `dom-get`, `dom-set`, `dom-append`
- `on-click`, `on-input`, `on-submit`
- `fetch`와 Promise/비동기 결과 처리
- 문자열·JSON·오류 위치 변환
- 브라우저용 메모리와 문자열 런타임

## 검증 기록

문서 조사에서는 fx2 소스·런타임·예제·패키지·검증 스크립트를 확인했다.

추가로 `ssr-app/app.fl`과 `freelang-front/_app.fl`을 `fl-build.sh`에 통과시키는 실행 검증을 시도했으나, 현재 환경에서는 명령이 진단 출력 없이 종료 코드 `182`로 끝났다. 따라서 이 문서는 정적 소스 근거와 저장소 구조에 기반하며, 해당 환경에서의 완전한 네이티브 재빌드 성공을 주장하지 않는다.

## 변경 분류

이 문서는 코드 동작을 변경하지 않는 **NON-BREAKING 문서 추가**다.

