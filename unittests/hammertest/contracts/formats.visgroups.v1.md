# Contract: `formats.visgroups.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/visgroups.h` · Impl: `hammer/core/formats/visgroups.cpp`
Conformance: `unittests/hammertest/formats/test_visgroups.cpp` (+ `_negative`)
Migration: `HAM-VISGROUP-001`
Depends on: `hammer.formats` (keyvalues codec)

Parses a VMF's VisGroup tree and computes object visibility given a set of hidden
groups — the "functional VisGroups" feature (the checkbox list that shows/hides
map objects). Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own VisGroup organization + visibility. Consumers: the VisGroup panel and any view
that renders only visible objects. Required for VisGroup show/hide.

## 2. Accepted inputs

- `ParseVisGroups( root )`: the top-level `visgroups` block → a tree of
  `VisGroup{id, name, children}` (nested `visgroup` sub-blocks).
- `ApplyVisGroupFilter( root, hiddenGroupIds )`: a document and the ids the user
  has hidden. An object's memberships come from its `editor { visgroupid N }`
  sub-block (an object may have several).

## 3. Results and guarantees

- `hiddenGroupIds` is expanded down the tree: hiding a parent hides all its
  descendant groups. An object (world solid or entity) is removed iff any of its
  memberships is in the expanded hidden set; an object with no membership is kept.
- Result carries the filtered `document` plus `visibleObjects`/`hiddenObjects`
  counts. Non-world/entity blocks are preserved verbatim. A hidden id naming no
  group still hides its direct members. A non-numeric membership is ignored
  (treated as unassigned).

## 4. Ownership, threading

- Pure functions over the input tree; no globals, no I/O.

## 5. Invariants

- **Descendant expansion**: hiding a parent hides members of child groups (a
  direct-membership-only implementation fails this).
- **Discrimination**: an empty hidden set (or an unknown id) hides nothing (a
  hide-all implementation fails this).

## 6. Side effects and performance

- One pass over the tree + one over the objects; O(objects + groups). No I/O.

## 7. Conformance suite and providers

- `test_visgroups.cpp`: nested tree parse; hide-parent-hides-child-member;
  hide-leaf; unassigned objects stay; empty set keeps all; counts.
- `test_visgroups_negative.cpp`: pins descendant expansion, empty/unknown
  keep-all, and non-numeric-membership handling (the properties a naive filter
  gets wrong).
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Filters top-level world solids and entities by visgroup membership. It does not
  yet handle `group`-block (object grouping) membership, per-object
  `visgroupshown`/`visgroupautoshown` persisted flags, or auto-visgroups; those
  are later increments.
