# FreeLang fx2 Conformance

## Release and completion gate

Completion uses the same gate as release; the difference is only the framing.

Release is blocked unless all of these are true:

- `SPEC.md` and runtime semantics match.
- `tools/fl-verify.sh official <input.fl>` passes for representative sources.
- Conformance sources pass.
- Build and run paths remain unchanged.
- Known constraints are documented.

## Required checks

- runtime unit tests
- conformance smoke test
- verification rule pass
- build path pass
- demo path pass

## Matrix summary

The conformance matrix is anchored to `spec/conformance.fl` and checks:

- prefix evaluation
- `define`, `let`, `defn`, `fn`, `apply`
- truthiness
- list/vector/map basics
- string helpers
- numeric helpers
- time/path helpers

## Intentional limitations

- `range` single-argument handling is not the standard path.
- `str-includes` boolean behavior is enforced by verification rules.
- closure capture remains limited and is managed as a documented limitation.
- anything not covered by conformance or spec is not complete yet.

## Verification commands

- `bash tools/fl-verify.sh conformance`
- `bash tools/fl-verify.sh official spec/conformance.fl`
