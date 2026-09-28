# Contract: `scene.change_set.v1`

Module: `hammer.scene`
Header: `public/hammer/scene/change_set.h` · Impl: `hammer/core/scene/change_set.cpp`
Conformance: `unittests/hammertest/scene/test_change_set.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Staged edits and lossless change sets, the unit every operation targets and
undo replays (RFC 0002 "Edit transactions and history"). Consumers:
`hammer.app` operations and `EditSession`; viewport caches read change sets.
Required.

## 2. Accepted inputs

A `DocumentEdit` borrows a base document that must not change while it lives.
Writes go through `Mutable*`, `Add`, `Put`, `Remove` and `MutableSettings`.

## 3. Results and guarantees

- The base is never modified by staging. Reads see staged state.
- `Finish()` records per-object before/after values and drops changes whose
  after equals before; an edit that returns to its base is empty.
- `Apply(Forward)` then `Apply(Backward)` restores the exact prior content.
- A change set whose ids carry another document's serial is refused with no
  partial mutation.
- `CommitEdit` advances the document's id counters past the edit's
  allocations even when nothing changed.
- `ValidateEdit` reports invariant violations of touched objects and live
  references to removed objects.

## 4. Ownership, threading

Change sets own copies of the values they record. Single sequence.

## 5. Invariants

Created ids are allocated above the base's counters, so they never collide
with base ids or retired ids.

## 6. Side effects and performance

Copy-on-write: only touched objects are copied. `ValidateEdit` scans all solids
only when side ids or removals need a document-wide check.

## 7. Conformance suite and providers

`test_change_set.cpp` (38 checks, gcc and clang), with negative checks for
foreign change sets, removing dead objects, wrong-kind mutation and seeded
`ValidateEdit` violations (open solid, side id clash, orphaned solid, group
cycle).
