# freelang-v11-fx2 (AFJ Runtime)

Part of [AFJ Ecosystem](https://github.com/kimjindol2025)

> Building a programming language by building real software.

## About

AFJ Runtime compiles FreeLang-native programs into native executables and runs them with a practical execution layer used by the AFJ stack.

## Validation Status

| Component | Build | Test | Verify | Measure |
|-----------|-------|------|--------|---------|
| Runtime   | ⚠ `bash fl-build.sh` (manual) | ⚠ `bash fl-run.sh` (manual) | ⚠ reports not standardized in branch | ⚠ metrics pipeline pending |
| Docs      | `BUILD.md` + `DOCS-INDEX.md` | `TESTING.md` + `DEBUG.md` | `CHANGELOG.md` + `ROADMAP.md` | `reports/` not yet stabilized |

## Related Projects

- Upstream: [freelang-v11](https://github.com/kimjindol2025/freelang-v11)
- Core: [AFL-Core-2](https://github.com/kimjindol2025/AFL-Core-2)
- Storage: [afl-db](https://github.com/kimjindol2025/afl-db)
- Frontend: [freelang-front](https://github.com/kimjindol2025/freelang-front)

## Quick Links

- [Getting Started](INTRODUCTION.md)
- [Architecture Notes](DOCS-INDEX.md)
- [Language Spec](SPEC.md)
- [Roadmap](ROADMAP.md)
- [Validation Policy](PROJECT_IMPLEMENTATION_RULES.md)

## Validation Report

- Build/Test/Verify/Measure pipeline is being consolidated to match AFJ repository convention.
- Runtime examples exist in `search-app/` and `ssr-app/`.

## How to run

```bash
bash fl-build.sh sample.fl out.bin
./out.bin
# 또는
bash fl-run.sh sample.fl
```

- Minimal example:

```lisp
;; hello.fl
(define msg "Hello, fx2!")
(println msg)
```

```bash
bash fl-build.sh hello.fl hello-bin
./hello-bin
```

## fx2 Playground 안전 실행

`fx2-playground/app.fl`은 요청마다 임시 작업공간과 산출물 경로를 만들고, 소스 크기 1MB, 실행 시간 15초, 동시 검사 8개로 제한합니다. 빌드는 `--no-net`으로 실행되며 요청 종료 후 작업공간을 정리합니다. 서버 시작 시 워밍업 빌드를 수행해 첫 요청도 제한 시간 안에서 처리합니다.

`POST /api/check`는 성공 200, 소스 초과 413, 컴파일 오류 422, 동시 요청 초과 429, 실행 시간 초과 408을 반환합니다.

## Structure snapshot

```text
freelang-v11-fx2/
├── fl-build.sh
├── fl-run.sh
├── INTRODUCTION.md
├── SPEC.md
├── README.md
├── runtime/
├── search-app/
├── ssr-app/
├── spec/
└── tools/
```

## Version

**v2.0** (2026-06-22)

- `SPEC.md` 정식화
- `INTRODUCTION.md` 정비
- `verify.sh` 검증 경로 정립
- `spec/conformance.fl` 기반 기본 conformance 샘플
