# FreeLang fx2 Workflow

## Official order

1. Read `CANON.md`.
2. Read `SPEC.md`.
3. Read `CONFORMANCE.md`.
4. Verify with `bash tools/fl-verify.sh official <input.fl>`.
5. Build with `bash fl-build.sh <input.fl> [output]`.
6. Run with `bash fl-run.sh`.

## Verification modes

- `bash tools/fl-verify.sh official <input.fl>`
- `bash tools/fl-verify.sh conformance`
- `bash tools/fl-verify.sh local <input.fl>`

## Completion definition

- The language is complete when the canon is stable, the verifier is singular, and conformance matches runtime behavior.
- Anything still listed as a constraint must be intentionally supported or intentionally frozen.

## Working rules

- Working implementation comes first.
- Prefer the simplest change that preserves behavior.
- Prefer modification over addition.
- Prefer merging over splitting.
- Verification must fail before build.
- Build must fail before run.
- Conformance sources are verified with the conformance rule set.

## Quick mental model

- `SPEC.md` defines what the language is.
- `CONFORMANCE.md` defines what must pass.
- `WORKFLOW.md` defines what to do first.
- `CANON.md` defines the canonical decision order and file policy.

