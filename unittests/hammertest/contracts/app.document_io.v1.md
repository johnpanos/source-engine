# Contract: `app.document_io.v1`

Module: `hammer.app`
Header: `public/hammer/app/document_io.h` · Impl: `hammer/core/app/document_io.cpp`
Conformance: `unittests/hammertest/app/test_document_io.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Opening and saving a session's document through the persistence ports: the
injected `IMapCodec`, `IFileStore`, and the transactional save policy.
Consumers: the session command layer (`open`, `save`, `build_map`) and UI hosts.
Required.

## 2. Accepted inputs

A session, a codec, a file store and a store path.

## 3. Results and guarantees

- Open reads and decodes into a detached document with a new serial (old ids
  never resolve) and replaces the session's document only on success; the
  result is unmodified with an empty history and carries the codec warnings.
- Read, decode (with its line) and guard-veto failures leave the document.
- Save encodes a copy, writes it atomically, and only then marks the session
  saved. Encode and write failures leave the session modified and the previous
  file in place.
- The map version bump (world `mapversion` + 1, mirrored in `versioninfo`) is
  bookkeeping: no history entry, not modified, and undo never rolls it back.

## 4. Ownership, threading

Borrows every argument for the call. Single sequence (the session's).

## 5. Invariants

The saved position moves only after a successful write.

## 6. Side effects and performance

File I/O only through the store; one temporary file per save.

## 7. Conformance suite and providers

`test_document_io.cpp` (23 checks) with the in-memory codec and file store
fakes, including injected encode and write failures.
