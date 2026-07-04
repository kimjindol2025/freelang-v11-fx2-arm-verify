# FreeLang fx2 Canon

This file is the canonical index for the trimmed documentation set.

## Canonical docs

- `SPEC.md`
- `WORKFLOW.md`
- `CONFORMANCE.md`
- `CANON.md`
- `DEPRECATED.md`

## Source of truth

The current source-of-truth chain is:

1. `CANON.md`
2. `SPEC.md`
3. `WORKFLOW.md`
4. `CONFORMANCE.md`

## Current status

- compiled, Lisp-like, function-composition language
- docs are compressed to the minimal canonical set
- verification is available without compiling
- conformance is separated from general verification

## Language completion goal

FreeLang is complete when it is a stable canonical dialect with one verification entrypoint, one spec chain, and no unexplained runtime behavior.

Completion criteria:

- `SPEC.md` and runtime semantics match.
- `tools/fl-verify.sh` is the single verification entrypoint.
- representative sources and `spec/conformance.fl` pass consistently.
- build and run paths remain stable.
- every remaining limitation is either implemented or explicitly documented as intentional.
- no extra canonical docs or wrappers are needed for normal development.

## Engineering style baseline

Operate under this order:

1. Working implementation
2. Simplicity
3. Maintainability
4. Documentation
5. Abstraction

Core bias:

- minimize files and dependencies
- prefer one entry point
- prefer modification over addition
- prefer merging over splitting
- create new files only when required
- keep documentation short and canonical
- keep implementation ahead of documentation

## Quick commands

- `bash tools/fl-verify.sh official <input.fl>`
- `bash fl-build.sh <input.fl> [output]`
- `bash fl-run.sh`

## References replaced by this canon

The following older index/status documents are now deprecated by the trimmed canon:

- `SOURCE-OF-TRUTH.md`
- `CANONICAL-INDEX.md`
- `LANGUAGE-STATUS.md`
- `DOCS-TRIAGE.md`
- `DERIVED-DOCS.md`

