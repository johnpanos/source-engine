# Contract: `app.editor_document.v1`

Module: `hammer.app` (integrates `hammer.formats`, `hammer.ports`)
Header: `public/hammer/app/editor_document.h` · Impl: `hammer/core/app/editor_document.cpp`
Conformance: `unittests/hammertest/app/test_editor_document.cpp` (+ `_negative`)
Migration: `HAM-WORKFLOW-001`

The headless editor document: the RFC 0002 reference workflow (open a VMF, edit,
undo/redo, save, reopen, compare) realized by composing the VMF codec, the history
model, and transactional save. It is the integration seam that gives the other
modules a working consumer.

## 1. Purpose, consumers, scope

Own one document's authored content, its single history authority, and its save
lifecycle. The in-memory content is a keyvalues tree for this first slice; typed
scene content addressed by `NodeHandle` is a later step. Required.

## 2. Accepted inputs

- `LoadFromText( vmf, &error )` / `Load( store, path, &error )`: VMF text or a
  stored file.
- `SetFirstBlockKey( key, value )`: a bounded example edit (sets/adds a key on the
  first top-level block).
- `Save( store, path )`: writes the content via the transactional save policy.

## 3. Results and guarantees

- **Detached load**: parsing builds a candidate; only a successful parse replaces
  the content, and it does so with a fresh, saved (unmodified) history. A failed
  load leaves the current document unchanged and sets `error`.
- **One edit, one history unit**: a content-changing edit commits exactly one
  history unit and marks the document modified.
- **No-op neutrality**: an edit that does not change content returns false, adds no
  history, and does not mark modified.
- **Undo restores recorded content**: undo/redo move the single history position
  and restore the recorded content snapshot (not a numeric inverse). Undo to the
  saved position clears modified.
- **Save advances the saved position**: a successful save marks the document
  unmodified; a subsequent undo makes it modified again.
- **Round-trip fidelity**: saving then reopening yields semantically equal content
  (per `formats.keyvalues.v1`).

## 4. Ownership, threading

- Owns its content and snapshots; borrows an `IFileStore` only for the duration of
  a load/save call. One history authority (no duplicate undo stacks). Not
  internally synchronized.

## 5. Invariants

- One authoritative content and one history position at all times.
- Snapshots align with history positions; a new edit drops the redo tail.

## 6. Side effects and performance

- File I/O only through the injected store, only during load/save. Snapshots are
  full-content copies in this slice (a lossless-patch representation is a later
  optimization).

## 7. Conformance suite and providers

- `test_editor_document.cpp`: the full open/edit/undo/redo/save/reopen/compare
  workflow, no-op neutrality, and the save-position/modified interaction, using
  the in-memory file-store fake.
- `test_editor_document_negative.cpp`: a document whose undo does not restore the
  content snapshot must be **detected** by the undo-restores-content predicate.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.
