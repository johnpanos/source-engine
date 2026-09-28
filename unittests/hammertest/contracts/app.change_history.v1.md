# Contract: `app.change_history.v1`

Module: `hammer.app`
Header: `public/hammer/app/change_history.h` · Impl: `hammer/core/app/change_history.cpp`
Conformance: `unittests/hammertest/app/test_change_history.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The labeled undo history of one document: change sets with labels and the
selection before and after, counters for revision and saved position, and an
optional depth limit. Consumers: `EditSession`, the history presenter.
Required.

## 2. Accepted inputs

Entries pushed after the current position; limit 0 means unbounded.

## 3. Results and guarantees

- A push discards the redo tail; if the saved position was in it, the document
  stays modified until the next save.
- The revision increases on every push, step and reset.
- Undoing back to the saved position is unmodified.
- Trimming drops only entries before the position and shifts the position and
  saved position; a trimmed saved position leaves the document modified.

## 4. Ownership, threading

Owns its entries. Single sequence.

## 5. Invariants

`0 <= position <= size`; `size <= limit` when bounded.

## 6. Side effects and performance

None.

## 7. Conformance suite and providers

`test_change_history.cpp` (21 checks, gcc and clang), with negative checks for
stepping past either end.
