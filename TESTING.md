# Testing

## Minimum Bar
- Build must succeed.
- Fixpoint must pass.
- New API must have a success case and a failure case.

## Common Checks
```bash
bash ./verify-fixpoint.sh
bash ./fl-build.sh spec/conformance.fl conformance-bin
./conformance-bin
```

## Guidance
- Add regression coverage when behavior changes.
- Keep tests small and specific.
- Test the highest-risk path first, then the happy path.
