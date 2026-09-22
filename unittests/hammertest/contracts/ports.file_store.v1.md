# Contract: `ports.file_store.v1`

Module: `hammer.ports` · Interface: `hammer::ports::IFileStore`
Header: `public/hammer/ports/file_store.h`
Real provider: `hammer::adapters::platform::DiskFileStore`
(`hammer/adapters/platform/disk_file_store.{h,cpp}`)
Shared suite: `unittests/hammertest/ports/file_store_contract.h`
Conformance suite: `unittests/hammertest/ports/test_disk_file_store.cpp`
Migration: `HAM-DISKSTORE-001`

`IFileStore` is an explicitly polymorphic contract: save orchestration and codecs
depend on whole-file behavior, not on a concrete filesystem, MFC, or tier0. This
record is the behavioral specification every implementation must satisfy, and the
shared suite runs against **every** claiming implementation — the real
`DiskFileStore`, the `InMemoryFileStore` test fake, and a deliberately broken
provider — to prove substitutability (Liskov) and that the suite detects violations.

## 1. Purpose, consumers, required vs optional

Whole-file primitives for the editor's persistence path (`SaveDocument`,
`EditorDocument::Save/Load`). All five operations are **required**: `Exists`,
`Read`, `Write`, `Rename`, `Remove`.

## 2. Accepted inputs, encoding, limits

- Paths are opaque strings interpreted by the provider (real OS paths for
  `DiskFileStore`; arbitrary keys for the in-memory fake). Content is an arbitrary
  byte string (`std::string`), stored and returned verbatim (binary-safe).

## 3. Results and guarantees (success / failure)

- `Exists(p)`: true iff a file is currently stored at `p`.
- `Read(p, out)`: returns true and fills `out` with the exact stored bytes; returns
  **false and leaves `out` unchanged** when `p` is absent or unreadable.
- `Write(p, content)`: creates or replaces `p` with `content` wholesale, returns
  success. **A failed write must not partially corrupt an existing file at `p`.**
  `DiskFileStore` honors this by staging to a sibling temp file and renaming into
  place.
- `Rename(from, to)`: atomically replaces `to` with `from`; returns success. On
  failure both paths retain their prior contents. Renaming an **absent source**
  returns false.
- `Remove(p)`: removes `p`, returns success (whether removing an absent file is
  success is provider choice; the orchestrator does not depend on the distinction).

## 4. Ownership, threading, ordering

- The store owns no borrowed caller state. Providers are not required to be
  internally synchronized; the editor uses one store from the app thread.

## 5. State invariants and legal sequences

- After `Write(p, c)`: `Exists(p)` and `Read(p) == c`.
- After a successful `Rename(a, b)`: `!Exists(a)`, `Exists(b)`, `Read(b)` == the
  prior contents of `a`.
- After `Remove(p)`: `!Exists(p)`.

## 6. Permitted side effects

- Filesystem I/O for a real provider; none observable beyond the store for a fake.
  No logging, no globals, no application singletons or backdoors (fake-provider rule).

## 7. Negative-provider requirement

The sensitivity suite runs a `ForgetfulFileStore` that reports write success but
never persists (Read never returns stored content). The shared suite must flag the
round-trip and rename/remove clauses against it while the real fake passes,
proving the suite pins persistence rather than trivially succeeding.
