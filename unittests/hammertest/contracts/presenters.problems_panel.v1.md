# Contract: `presenters.problems_panel.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/problems_panel.h` · Impl: `hammer/core/presenters/problems_panel.cpp`
Conformance: `unittests/hammertest/presenters/test_problems_panel.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The Check for Problems panel (legacy `CMapCheckDlg`): `app::CheckMap` rows,
counts by severity, go-to and fixes. Consumers: the GTK problems panel, the
build flow.

## 2. Accepted inputs

A session, its `SessionCommands`, the optional `IEntityCatalog` and `IMaterialInfo` the commands were composed with; row indices for
go-to and fix.

## 3. Results and guarantees

- Rows keep `CheckMap`'s order with the code name, "error"/"warning", the
  message, objects and their `ObjectLabel`s, and `fixable`.
- `ErrorCount`, `WarningCount`, `FixableCount` match the rows.
- The scan reruns on edit/undo/redo when the session revision moved and on
  replacement; selection changes and saves never rescan; `Refresh()` forces
  one.
- `GoTo` selects the row's objects (Replace; guards apply); a map-wide
  problem is refused.
- `Fix` runs the `fix_problem` command with the row's code and first object
  (one undo step, "Fix <code name>"); an unfixable problem is refused with
  nothing changed. `FixAll` runs `fix_all` (one step, "Fix all problems");
  refused when nothing is fixable. Out-of-range rows are refused. Every
  action returns `app::CommandResult`.

## 4. Ownership, threading

Observes; the document belongs to the session. Single sequence. RAII
subscription.

## 5. Invariants

`CheckedRevision()` is the session revision the rows describe.

## 6. Side effects and performance

One `CheckMap` scan per document change.

## 7. Conformance suite and providers

`test_problems_panel.cpp` (22 checks, gcc and clang); negative checks for
unfixable problems, out-of-range rows, map-wide go-to, nothing to fix and
destruction order.
