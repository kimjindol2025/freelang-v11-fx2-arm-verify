# FreeLang Load Balancer 1E-A: socket lifetime registry

## Decision

The prototype selects **existing fd API plus an internal generation registry**.

An fd alone has an ABA hazard: after close, the kernel may reuse the same integer
for an unrelated socket. The internal key is therefore `(fd, generation)`, where
generation is allocated monotonically by the registry. Every internal lookup and
I/O accounting operation requires the complete key. A stale key is rejected even
when its fd has been reused.

The opaque-handle alternative would make stale references safer for new callers,
but it would require an adapter for every existing fd API and a wider migration.
The adapter would still need the same registry, generation, and reference rules.
The selected model preserves the current fd ABI, keeps the implementation scope
small, and protects all new/internal paths. Legacy raw-fd calls remain explicitly
legacy: they cannot claim generation safety until they enter the registry with a
key. The isolated prototype demonstrates the safety properties below.

## Identity and state

Each entry contains a socket kind (`LISTENER`, `INBOUND_CONNECTION`, or
`OUTBOUND_CONNECTION`), OS fd, monotonic generation, state, reference count, and
payload-free I/O counters: read/write bytes, calls, partial calls, and the last
status (`ok`, `eof`, `timeout`, `error`, or `refused`). The identity is exactly
`(fd, generation)`; kind is an attribute and is never inferred from fd.

State transitions are:

`OPEN -> CLOSING -> CLOSED -> unregistered`.

Only one caller can claim `OPEN -> CLOSING`. A lookup accepts only a registered
`OPEN` entry with an exact key. Close completion marks `CLOSED`; removal waits for
the registry reference and all acquired I/O references to drain.

## Lock and lifetime rules

One registry mutex protects the entry list, generation allocator, state,
close-claim bit, references, counters, and registry counts. Lookup/acquire,
accounting, release, registration, and unregister all hold it only for their
short in-memory critical section. No `socket`, `poll`, `recv`, `send`, or OS
`close` is called while the mutex is held.

Close claims the entry under the mutex, performs the OS close after unlocking,
then reacquires the mutex to mark it closed and release registry ownership.
Concurrent close callers observe the claim and do nothing. An entry is freed only
after it is closed and its reference count reaches zero.

Unregister is idempotent and occurs only after close completion; subsequent
lookups, including stale-generation lookups, fail. Re-registering the same fd
allocates a new generation from the registry-wide monotonic counter, so old
statistics cannot cross into the new entry.

Listener shutdown closes/unregisters only the exact listener key. Connection
close does the same for the exact connection key; neither operation walks or
removes entries of another kind.

## Prototype evidence

`registry_prototype.c` is deliberately separate from production runtime files.
It verifies kind separation, generation increment, stale lookup rejection,
close-once under 100 concurrent callers, I/O/close reference races, no statistics
crossover after fd reuse, listener/connection isolation, and a 100-entry
concurrent register/account/close drain. The fake OS-close counter represents the
single OS close side effect without introducing blocking syscalls into the test.

The prototype stores no payload bytes. ASan checks lifetime safety; TSan checks
the mutex-protected registry and statistics paths when supported by the host.
