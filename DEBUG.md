# Debug

## When a Build Fails
1. Re-run the build with the smallest command that still fails.
2. Capture the generated C and the compiler output.
3. Use `gcc -fsyntax-only` before linking.

## Low-Level Reproduction
```bash
CGC_BIN=/root/freelang-v11/bin/cgc-bin.bak bash ./fl-build.sh <input.fl> <output-bin> --no-net
```

If the wrapper hides the error, reproduce manually:
```bash
/root/freelang-v11/bin/cgc-bin.bak <input.fl> /tmp/out.c
gcc -fsyntax-only -I ./runtime /tmp/out.c -w
```

## Notes
- `fl-build.sh` may clean temporary logs on failure.
- If that happens, rerun the lower-level commands immediately.
- Prefer direct compiler output over wrapper summaries when diagnosing type errors.
