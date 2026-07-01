# Codex Memory

This file records the working rules for Codex on this repository.

## Active Rules

- Follow `11절` coding rules from [`docs/CODING_RULES_11.md`](/root/freelang-v11/docs/CODING_RULES_11.md).
- Work in this order: research -> plan -> document -> task log -> implement -> verify.
- Ask before architecture or interface changes when collaboration is needed.
- Prefer small stages. If verification fails twice, treat rollback as the default.
- Use git for change tracking. Pushes go to Gogs by default.
- Keep security checks first-class, especially for secrets and user-controlled data.

## Repo Notes

- This repository is `freelang-v11-fx2`.
- Current baseline includes SSR demo apps, runtime bindings, package registry code, and conformance checks.
- Treat `README.md` and `docs/` as documentation, but verify behavior against the implementation.

## Reminder

This file is repository-scoped working memory, not global assistant memory.
