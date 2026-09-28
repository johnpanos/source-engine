# Contract: `presenters.outliner.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/outliner.h` (+ `object_label.h`) · Impl: `hammer/core/presenters/outliner.cpp`
Conformance: `unittests/hammertest/presenters/test_outliner.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The object tree (legacy `grouplist.cpp`, Source 2's filterable Outliner) as
rows a tree widget binds to, with selection and quick-hide actions.

## 2. Accepted inputs

A session; a filter text; expand/collapse per id; row ids to select with a
`SelectMode`; an id whose visibility to toggle.

## 3. Results and guarantees

- Pre-order rows: `world` root; under it and each group: groups, entities,
  loose solids in id order; brush entities contain their solids.
- Labels from `ObjectLabel` (targetname or classname; `solid <vmfId>`;
  `group <vmfId>`); child counts after filtering; `visible` from
  `scene::IsVisible`; `selected` when the object or a container above it is
  selected.
- Filter: case-insensitive substring over label/classname/targetname; matches
  keep their ancestors; non-matching children of a match are dropped; the
  root always stays.
- Expansion persists per id across rebuilds and filters, and is cleared on
  document replacement. Containers default collapsed, the root expanded.
  `DisplayRows()` hides rows under collapsed ancestors unless a filter is
  active.
- `Select` forwards to `SelectObjects` (the world row selects nothing);
  `ToggleVisibility` is one `ops::SetHidden` step ("Hide L"/"Show L"); unknown
  ids are refused.

## 4. Ownership, threading

Owns only filter and expansion view state. Single sequence. RAII subscription.

## 5. Invariants

Every row's parent precedes it; child counts equal the kept children.

## 6. Side effects and performance

Rebuilds on every session event: O(objects) plus O(depth) visibility checks
per row.

## 7. Conformance suite and providers

`test_outliner.cpp` (30 checks, gcc and clang); negative checks for unknown
ids, the world row, no-match filters, replacement and destruction order.
