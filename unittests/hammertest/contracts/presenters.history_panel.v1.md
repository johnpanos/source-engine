# Contract: `presenters.history_panel.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/history_panel.h` · Impl: `hammer/core/presenters/history_panel.cpp`
Conformance: `unittests/hammertest/presenters/test_history_panel.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The undo history list and the Edit menu's Undo/Redo labels (Source 2's
undo-history jump).

## 2. Accepted inputs

A session; a history position to jump to; undo/redo requests.

## 3. Results and guarantees

- One row per entry: `position = k + 1`, the label, `done` when applied,
  `current` for the last applied; the redo tail stays listed, not done.
- `UndoLabel()` "Undo <label>" / "Undo"; `RedoLabel()` likewise; `CanUndo`,
  `CanRedo`, `IsModified` mirror the history.
- `JumpTo(position)` goes through `EditSession::JumpTo` (0 undoes all); out of
  range is refused and changes nothing.
- Selection-only events do not move `Revision()`; saves and replacement do.

## 4. Ownership, threading

Observes; the history belongs to the session. Single sequence. RAII
subscription.

## 5. Invariants

Rows equal the session history's entries in order.

## 6. Side effects and performance

O(entries) per history change.

## 7. Conformance suite and providers

`test_history_panel.cpp` (17 checks, gcc and clang); negative checks for
nothing to undo, out-of-range jumps, selection-only changes and destruction
order.
