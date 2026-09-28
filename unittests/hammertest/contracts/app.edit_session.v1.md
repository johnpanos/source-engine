# Contract: `app.edit_session.v1`

Module: `hammer.app`
Header: `public/hammer/app/edit_session.h` · Impl: `hammer/core/app/edit_session.cpp`
Conformance: `unittests/hammertest/app/test_edit_session.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The per-document editing authority (RFC 0002's `EditorSession`): one document,
its selection and its history. Every content change goes through `Execute`.
Consumers: tools, presenters, the command layer and UI hosts. Required.

## 2. Accepted inputs

Operations are functions over a `DocumentEdit` returning `EditResult`; an
optional selection-after. Selection requests in any mode.

## 3. Results and guarantees

- A refused operation, or an edit that fails `ValidateEdit` (`Invalid`), leaves
  content, selection, history and observers untouched.
- A no-op edit records nothing and does not mark the document modified.
- A commit records one history entry, prunes the selection and publishes one
  `Edited` event with the change set.
- Undo/redo apply the recorded change set and restore the selection recorded
  with it; `JumpTo` walks the history.
- Selection changes record no history, publish `SelectionChanged`, and pass
  every selection guard first (a guard may veto: `Vetoed`).
- `Replace` resets history and selection after the guards.

## 4. Ownership, threading

Owns the document, selection and history. Single sequence. Observers run
synchronously; a mutation requested from an observer or guard is `Busy`.
Subscriptions are RAII and may outlive the session.

## 5. Invariants

The selection only names live objects. One history authority per document.

## 6. Side effects and performance

Observers are called once per committed change. Validation cost is
proportional to the touched objects plus a document scan when side ids or
removals need it.

## 7. Conformance suite and providers

`test_edit_session.cpp` (34 checks, gcc and clang), with negative checks for
refusal, invalid edits, redo past the end, jumps out of range, vetoes and
reentrancy.
