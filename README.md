# freelang-v11-fx2 (AFJ Runtime)

Part of [AFJ Ecosystem](<AFJ_ECOSYSTEM_PROFILE_URL>)

> Building a programming language by building real software.

## About

AFJ Runtime builds and runs FreeLang-native programs with native-like execution.
It is the first-class execution layer used by AFJ tooling in this phase.

## Validation Status

| Component | Build | Test | Verify | Measure |
|-----------|-------|------|--------|---------|
| Runtime | ⚠ no GitHub workflow configured | ⚠ report path 미정 | ⚠ 수동 점검 필요 | ⚠ 수동 점검 필요 |
| Documents | `BUILD.md`, `TESTING.md`, `LANGUAGE-REVIEW-POLICY.md` | `DOCS-INDEX.md`, `INTRODUCTION.md` | `SPEC.md`, `CHANGELOG.md` | `reports/` 폴더 없음 |

## Related Projects

- Upstream: [freelang-v11](https://gogs.dclub.kr/kim/freelang-v11)
- Core: [AFL-Core-2](https://gogs.dclub.kr/kim/AFL-Core-2)
- Storage: [afl-db](https://gogs.dclub.kr/kim/afl-db)
- Frontend: [freelang-front](https://gogs.dclub.kr/kim/freelang-front)

## Quick Links

- [getting-started](INTRODUCTION.md)
- [architecture](DOCS-INDEX.md)
- [language spec](SPEC.md)
- [roadmap](ROADMAP.md)
- [validation report](reports/latest.md)

## Validation Report

- Build/Test/Verify pipeline: 현재 레포 내 표준 `reports/` 아티팩트 미생성
- 자동화 우선순위: `Build -> Test -> Verify -> Measure` 순서로 표준 리포트 폴더 정착 필요

## How to run

```bash
bash fl-build.sh sample.fl out.bin
./out.bin
# 또는
bash fl-run.sh sample.fl
```

## Version

`2.0` (`SPEC` 상의 버전 표기 기반)
