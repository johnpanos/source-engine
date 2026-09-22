# Contract: `app.save_orchestrator.v1`

Module: `hammer.app` (over `hammer.ports`)
Ports header: `public/hammer/ports/file_store.h` (`IFileStore`)
Header: `public/hammer/app/save_orchestrator.h` · Impl: `hammer/core/app/save_orchestrator.cpp`
Conformance: `unittests/hammertest/app/test_save_orchestrator.cpp` (+ `_negative`)
Test fake: `unittests/hammertest/app/fake_file_store.h`
Migration: `HAM-SAVE-001`

Centralizes the transactional save policy the RFC requires: write a temporary
file, then atomically replace the target, so a failed save preserves the prior
file. It depends only on the `IFileStore` port; production and fake providers
implement the same port.

## 1. Purpose, consumers, required vs optional

Own document save orchestration. Consumers: the document save path (and, later,
autosave against a separate recovery path). Required.

`IFileStore` is a genuine polymorphic contract (real filesystem vs in-memory vs
fault-injecting fake); a virtual interface is appropriate.

## 2. Accepted inputs

- `SaveDocument( store, path, content, tempSuffix )`: any store implementing the
  port, a target path, whole-file content, and the sibling temp suffix.

## 3. Results and guarantees

- `kOk`: content is now at `path`; the temp file no longer exists.
- `kTempWriteFailed`: the temp write failed; **the prior file at `path` is intact**;
  no temp remains.
- `kCommitFailed`: temp written but the atomic rename failed; **the prior file is
  intact**; the temp is cleaned up.

## 4. Ownership, threading

- Stateless free function over an injected store. No globals. Thread-safety is the
  store's concern.

## 5. Invariants and legal sequences

- **Failure preserves prior**: neither failure path leaves the target truncated or
  replaced. This is the property the naive "truncate then write" approach violates.
- **Atomic publish**: the target only changes via the rename, never by a partial
  in-place write.
- **No temp leak**: on both failure paths the temp is removed best-effort.

`IFileStore` obligations: `Write` must not corrupt an existing file at the path on
failure (the orchestrator writes to a temp, so this is upheld structurally);
`Rename` is an atomic replace and leaves both paths intact on failure.

## 6. Side effects and performance

- Two store writes/renames per save; no logging or globals.

## 7. Conformance suite and providers

- `test_save_orchestrator.cpp`: clean save replaces, new-file create, temp-write
  failure preserves prior, commit failure preserves prior, temp cleaned up. Uses
  `InMemoryFileStore` with fault injection.
- `test_save_orchestrator_negative.cpp`: a truncate-then-write saver must be
  **detected** by the prior-preserved predicate.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
