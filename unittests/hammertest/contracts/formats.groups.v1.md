# Contract: `formats.groups.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/groups.h` · Impl: `hammer/core/formats/groups.cpp`
Conformance: `unittests/hammertest/formats/test_groups.cpp` (+ `_negative`)
Migration: `HAM-GROUP-001`
Depends on: `hammer.formats` (keyvalues codec)

Resolves Hammer object groups: `group` blocks (with nesting via `editor.groupid`)
and each solid's `editor.groupid` membership, so selecting one member selects its
whole top-level group. Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own group hierarchy + group-selection. Consumers: the shell's selection tool
(group-select) and collective transform. Required for grouped editing.

## 2. Accepted inputs

- `ParseGroups( root )`: a parsed VMF document → a `GroupTable` (`solidGroup`,
  `groupParent`).
- `ExpandGroupSelection( root, selectedSolidIds )`: the selected solid ids.

## 3. Results and guarantees

- `GroupTable::TopLevelGroupOfSolid(id)` returns the top-level ancestor group of a
  solid (0 when ungrouped), following the `groupParent` chain with a cycle guard.
- `ExpandGroupSelection` returns the selection plus every world solid sharing a
  top-level group with any selected solid; an ungrouped selected solid brings only
  itself. Result is sorted and de-duplicated.

## 4. Ownership, threading

- Pure functions over the input tree; no globals, no I/O.

## 5. Invariants

- **Top-level resolution**: a solid in a nested child group resolves (and selects)
  by its top-level ancestor, so members of sibling sub-groups under one top-level
  group select together (a direct-group-only expansion fails this).
- **Discrimination**: an ungrouped solid selects only itself (a pull-all expansion
  fails this).
- **Bounded**: a cyclic `groupParent` chain terminates (each id visited once).

## 6. Side effects and performance

- One pass to build the table + one over solids; O(objects + groups). No I/O.

## 7. Conformance suite and providers

- `test_groups.cpp`: nested hierarchy → top-level; select pulls group siblings;
  lone-group and ungrouped stay singular; multi-select unions.
- `test_groups_negative.cpp`: pins nested top-level expansion, ungrouped
  singularity, and cyclic-chain termination.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Resolves group membership + selection expansion for world solids. It does not
  expand entity group membership, create/dissolve groups, or persist group edits;
  those are later increments.
