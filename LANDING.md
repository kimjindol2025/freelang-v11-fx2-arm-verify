# freelang-v11-fx2 Landing

**Part of AFJ Ecosystem** | [Profile](https://gogs.dclub.kr/kim)

- **Mission:** Building AFJ Runtime with native execution confidence.
- **Role:** Runtime / Execution Engine
- **State:** Active

## At a Glance

- Lisp-style language implementation compiled via C backend
- Focus on deterministic build/run behavior
- Includes build, run, and extension tooling

## Why it exists

`freelang-v11-fx2` turns AFJ language artifacts into executable binaries and provides a practical runtime story.

## Validation Snapshot

- Build: current scripts-driven flow, no standard GitHub workflow yet
- Test: per docs and package scripts
- Verify: release gate documents and manual checks
- Measure: to be standardized as `reports/`

## Quick Start

```bash
bash fl-build.sh hello.fl hello-bin
./hello-bin
bash fl-run.sh hello.fl
```

## For Visitors

- Use this repo to validate whether a language concept is production-runnable.
- Good entry for users asking “can I run this now?”

## Links

- [Project README](./README.md)
- [Introduction](./INTRODUCTION.md)
- [Spec](./SPEC.md)
- [Release Gate](./RELEASE-GATE.md)
