# Build

## Source of Truth
- `CLAUDE.airc`
- `docs/CANON.md`
- `docs/SPEC.md`
- `docs/WORKFLOW.md`
- `docs/CONFORMANCE.md`

## Standard Build
```bash
CGC_BIN=/root/freelang-v11/bin/cgc-bin.bak bash ./fl-build.sh <input.fl> <output-bin>
```

Use a runnable `cgc-bin` for the current host. On this host, the arm64-compatible binary is `/root/freelang-v11/bin/cgc-bin.bak`.

## Run
```bash
./<output-bin>
```

## Notes
- `fl-build.sh` performs preprocessing, syntax checks, and link steps.
- Prefer relative paths inside the repo when possible.
- Keep build commands short enough to paste into `DEBUG.md` when reproducing failures.
