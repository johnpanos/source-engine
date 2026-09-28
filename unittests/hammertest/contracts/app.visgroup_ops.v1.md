# Contract: `app.visgroup_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/visgroup_ops.h` · Impl: `hammer/core/app/ops/visgroup_ops.cpp`
Conformance: `unittests/hammertest/app/test_visgroup_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The one owner of visgroup editing: the visgroup tree in
`DocumentSettings::visgroups`, membership (`EditorInfo::visgroupIds`) and
visgroup visibility (`EditorInfo::visgroupShown`, read by
`scene::IsVisible`). Consumers: the visgroups presenter, the command layer,
the map check (its fix files hidden orphans in a visgroup) and the clipboard
(it creates visgroups a pasted fragment names). Required.

## 2. Accepted inputs

A `DocumentEdit`; visgroup ids (0 = top level for parents); object ids
(any kind; unknown ids are skipped); names (non-empty).

## 3. Results and guarantees

- `NextVisgroupId` is one past every id in the tree and every id an object
  names, so a dangling reference never adopts a new visgroup.
- `CreateVisgroup` appends under the parent; duplicate names are allowed
  (legacy). `RenameVisgroup` refuses empty names; the same name is `Nothing`.
- `DeleteVisgroup` moves its children to its parent at its position, in
  order, removes its id from every object and shows objects left hidden with
  no covering visgroup (legacy `CheckVisibility`).
- `MoveVisgroup` appends under the new parent; the visgroup itself or a
  descendant as parent is refused (cycle).
- Membership: a solid of a brush entity stands for its entity (legacy
  `VisGroups_ObjectCanBelongToVisGroup`); a group stands for itself. The
  "move" form drops other memberships first. Membership changes never change
  visibility, except that `RemoveFromVisgroup` shows objects left hidden
  without a covering visgroup.
- Visibility: a visgroup touches the objects listing its id or a descendant's
  id, and everything they contain. Hiding clears `visgroupShown` on all of
  them. Showing sets `visgroupShown` and `visgroupAutoShown` on all of them
  except objects suppressed by another visgroup W outside the subtree whose
  members not touched by this show exist and are all hidden: an object in
  several visgroups is shown only while each is shown. A visgroup whose
  members are all shared stores no state and never suppresses.
- `VisgroupVisibility`: Shown, Hidden, Mixed over the subtree's members;
  Empty for no members or an unknown id.
- Refusals: unknown ids, empty names, cycles (`Rejected`); no-ops (`Nothing`).
  Dangling visgroup ids on objects are left for the map check.

## 4. Ownership, threading

Pure functions over the edit; no state. Single sequence (the edit's).

## 5. Invariants

The tree keeps unique ids. Every result passes `ValidateEdit`. The visibility
decision for a show is made against the state before the operation.

## 6. Side effects and performance

Settings are copied on first write. Showing is O(objects x visgroups per
object x members of suppressing visgroups) with per-visgroup caching.

## 7. Conformance suite and providers

`test_visgroup_ops.cpp` (72 checks, gcc and clang, `-Wall -Wextra -Werror`):
tree edits, membership, show/hide over subtrees and group contents, the
several-visgroups rule and its documented limit, presenter states, delete
with orphan restoration, and negative checks for unknown ids, empty names,
cycles and no-ops.
