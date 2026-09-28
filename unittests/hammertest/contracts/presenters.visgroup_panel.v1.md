# Contract: `presenters.visgroup_panel.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/visgroup_panel.h` · Impl: `hammer/core/presenters/visgroup_panel.cpp`
Conformance: `unittests/hammertest/presenters/test_visgroup_panel.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The VisGroups tab (legacy `CFilterControl`): the visgroup tree with
visibility and membership, and the visgroup actions.

## 2. Accepted inputs

A session and its `app::SessionCommands`; visgroup ids, names, parents, the
exclusive flag, visibility and a selection mode.

## 3. Results and guarantees

- Rows in pre-order with id, name, color, depth, parent row, child count,
  visibility (`ops::VisgroupVisibility`), recursive and direct member counts
  and the selected-members state (None/Some/All).
- Every change goes through the `visgroup_*` commands (one undo step each);
  `ToggleVisible` shows Hidden/Mixed and hides Shown visgroups and refuses
  Empty ones; `SelectMembers` runs `select` with the recursive members and
  refuses an empty visgroup. Unknown ids are refused by the commands with
  nothing changed.
- Expansion persists per visgroup id (default expanded) and clears on
  replacement.

## 4. Ownership, threading

Owns only expansion. Single sequence. RAII subscription.

## 5. Invariants

Rows mirror `DocumentSettings::visgroups`.

## 6. Side effects and performance

Rebuild per event: O(visgroups x objects) through the membership queries.

## 7. Conformance suite and providers

`test_visgroup_panel.cpp` (26 checks, gcc and clang); negative checks for
empty visgroups, unknown ids, replacement and destruction order.
