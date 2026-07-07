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
