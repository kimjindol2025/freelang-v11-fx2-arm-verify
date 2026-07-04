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
- `try`/`catch` is not part of the fx2 conformance surface.
- anything not covered by conformance or spec is not complete yet.

## Verification commands

- `bash tools/fl-verify.sh conformance`
- `bash tools/fl-verify.sh official spec/conformance.fl`

## Completion Declaration

Declared complete on 2026-07-04 after the release gate was rechecked on the
kimjin x86_64 node.

Evidence:

- `tools/fl-verify.sh official spec/conformance.fl` passed.
- `tools/fl-verify.sh conformance` passed.
- `spec/conformance.fl` built and ran with `== FX2 CONFORMANCE PASS ==`.
- `spec/conformance.fl --no-net` built and ran with `== FX2 CONFORMANCE PASS ==`.
- `tools/test-app-o-failure.sh` passed.
- `ssr-app/app.fl` and `search-app/app.fl` built with the rebuilt native
  `/home/kimjin/freelang-v11/bin/cgc-bin`.
- `/home/kimjin/freelang-v11/scripts/test-l2-fixpoint.sh` reached `L2 = L3`.
- Patch #5 first-class operator repros, including nested lambda operator
  value references for `<`, `<=`, `>`, `>=`, `=`, `!=`, `%`, and `/`, built and
  ran successfully.

Completion declaration commit: the commit containing this section.
