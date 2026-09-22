# Contract: `app.document_history.v1`

Module: `hammer.app` · Type: `hammer::app::DocumentHistory`
Header: `public/hammer/app/document_history.h` · Impl: `hammer/core/app/document_history.cpp`
Conformance: `unittests/hammertest/app/test_document_history.cpp` (+ `_negative`)
Migration: `HAM-HISTORY-001`

The independent document edit-transaction / history reference model the RFC 0002
verification strategy calls for. It pins the undo/redo and modified-state
semantics ahead of the full H3 application authority; it models the bookkeeping
(positions, revision, saved state), not the reversible change payloads, and is
not yet the live editor history.

## 1. Purpose, consumers, required vs optional

Own the history position, revision counter, and saved position for one document,
and the transitions between them. Consumers: the future edit-transaction owner,
the modified indicator, and undo/redo actions. All clauses **required**.

## 2. Accepted inputs

- `Commit( changed )`: `changed == false` marks a no-op request; `true` a real
  committed change unit. (Payloads are out of scope for v1.)
- `Undo`/`Redo`/`MarkSaved` take no inputs and are always safe to call.

## 3. Results and guarantees

- `Commit( true )`: drops any redo tail, advances the position by one, bumps the
  revision, returns true.
- `Commit( false )`: records nothing, does not change the revision, does not
  change the modified state, returns false.
- `Undo`/`Redo`: move the position by one when possible (`CanUndo`/`CanRedo`) and
  bump the revision; return whether they moved.
- `IsModified()`: true iff the current position differs from the saved position.
- `Revision()`: monotonic; strictly increases on every commit and every undo/redo
  move; never decreases.

## 4. Ownership, threading, ordering

- Value-like object owning only counters/positions. No globals, no I/O, not
  internally synchronized. Independent instances are fully independent (supports
  the two-document requirement).

## 5. Invariants and legal sequences

- **No-op neutrality**: a no-op commit changes nothing observable.
- **Redo-tail drop**: committing after undo discards the unreachable redo tail.
- **Monotonic revision**: an undo is a state transition and still advances the
  revision; the counter never goes backwards.
- **Revision != save position**: `IsModified` is driven by the saved *position*,
  not the revision. Undoing back to the saved position clears modified even though
  the revision has advanced past the save. This is the clause most implementations
  get wrong.

## 6. Side effects and performance

- None beyond its own counters; O(1) per operation.

## 7. Conformance suite and providers

- `test_document_history.cpp`: no-op neutrality, commit/undo/redo, redo-tail drop,
  and the revision-vs-saved-position clause.
- `test_document_history_negative.cpp`: a provider that decides modified from the
  revision counter must be **detected** — proving the oracle pins the rule.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
