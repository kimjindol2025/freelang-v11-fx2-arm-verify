# Project Implementation Rules

This document defines the repository-specific implementation rules for `freelang-v11-fx2`.

Read this file after `AI_OPERATING_MANUAL.md` and before changing code.

## 1. Truth Source

Implementation truth in this repository comes from:

- actual repository code under `runtime/`, `lib/`, `packages/`, and build scripts
- `SPEC.md` for language semantics and syntax rules
- `BUILD.md` for the supported build chain
- `TESTING.md` for the minimum verification bar
- `SECURITY.md` for security constraints
- executable verification such as conformance and fixpoint checks

Rules:

- if docs and implementation disagree, verify against code and runnable checks
- do not claim support for a feature unless code or tests prove it
- use `SPEC.md` as the semantic source of truth when implementing language behavior

## 2. Architecture Rules

This repository is structured as a language/runtime/build pipeline project.

Key boundaries:

- `fl-build.sh` is the standard build entrypoint
- `runtime/` contains runtime behavior and native bindings
- `lib/` contains language-level library code
- `spec/` contains conformance coverage and specification examples
- demos and apps are consumers, not the language core

Rules:

- preserve the existing build pipeline unless a change is explicitly required
- do not move language-core logic into demos or app examples
- keep runtime, stdlib, and build-tool responsibilities separate
- prefer extending existing patterns over adding parallel subsystems

## 3. Implementation Rules

Repository-specific implementation rules:

- do not guess parser, runtime, or stdlib behavior
- verify any claimed language rule against `SPEC.md` and implementation
- preserve existing naming and file layout conventions
- keep changes scoped to the layer being modified
- for behavior changes, add or update regression coverage where possible
- for new API behavior, include both success and failure-path verification

Rules:

- no speculative implementation
- no fake completion claims
- no architecture drift without a stated reason

## 4. Verification Rules

Minimum completion bar for this repository:

- build must succeed
- fixpoint must pass when relevant
- behavior changes must be checked with the most relevant runnable command
- new API behavior must have a success case and a failure case

Preferred verification commands:

```bash
bash ./verify-fixpoint.sh
CGC_BIN=/root/freelang-v11/bin/cgc-bin.bak bash ./fl-build.sh <input.fl> <output-bin>
./<output-bin>
bash ./fl-build.sh spec/conformance.fl conformance-bin
./conformance-bin
```

Rules:

- do not mark work complete without real verification
- if verification is partial, state exactly what ran and what did not
- if a check fails twice, stop papering over it and reassess

## 5. Security Rules

Follow `SECURITY.md` as the repository baseline.

Required constraints:

- escape user-controlled HTML output
- do not embed raw JSON into inline scripts without safe escaping
- use prepared SQL APIs for external input
- validate route params, query params, and request bodies before use
- treat all external input as untrusted
- do not hardcode secrets or tokens into tracked files

## 6. Handoff Rules

After meaningful work:

- update `Tasks` with the real current state
- update `Blog` when the work changes design understanding, exposes mistakes, or leaves continuation context
- leave exact next steps when the work is incomplete
- record failing checks explicitly

The next agent should be able to continue without rediscovering:

- what changed
- what remains
- what was verified
- what is still risky

## 7. Project-Specific Commands

Preferred commands for this repository:

```bash
# build
CGC_BIN=/root/freelang-v11/bin/cgc-bin.bak bash ./fl-build.sh <input.fl> <output-bin>

# run
./<output-bin>

# conformance
bash ./fl-build.sh spec/conformance.fl conformance-bin
./conformance-bin

# fixpoint
bash ./verify-fixpoint.sh
```

## 8. Completion Definition

Work is complete only when all of the following are true:

1. implementation is updated
2. the relevant verification ran successfully
3. any changed behavior is described accurately
4. `Tasks` reflects the latest state
5. `Blog` handoff notes are added when needed

## 9. Repository Notes

- `BUILD.md` documents the host-compatible `CGC_BIN` path currently expected on this host
- this repository mixes language core, runtime, demos, and apps, so scope control matters
- treat documentation as guidance, but verify against the implementation before making claims
