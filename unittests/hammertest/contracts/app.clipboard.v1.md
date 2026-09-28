# Contract: `app.clipboard.v1`

Module: `hammer.app`
Header: `public/hammer/app/clipboard.h` · Impl: `hammer/core/app/clipboard.cpp`
Conformance: `unittests/hammertest/app/test_clipboard.cpp`
Migration: `R08-DOMAIN`
Depends on: `app.transform_ops` (`TransformedSolid`, `TransformedEntity`),
`app.visgroup_ops` (`NextVisgroupId`), `scene.map_queries`

## 1. Purpose, consumers, scope

Copy, Paste, Paste Special and Duplicate (legacy `CMapDoc::Copy`, `Paste`,
`OnEditPastespecial`, Clone) over `MapFragment`, a detached value that holds
copied objects and the visgroups they name. Consumers: the command layer,
the Edit menu and the selection tool (Shift-drag clone). The system clipboard
and a text serialization of fragments are out of scope. Required.

## 2. Accepted inputs

`Copy`: any reader and ids (unknown ids skipped). `Paste`: a fragment from
any document (or a copy of one) and `PasteOptions`: finite offset, optional
finite (pitch, yaw, roll) degrees, 1..1024 copies, `group`, a name fix
(Keep / Suffix / Prefix) and its text.

## 3. Results and guarantees

- Copy expands like `scene::ExpandObjects`; references to objects not copied
  are dropped (a solid copied without its brush entity pastes as a world
  solid, an object without its group at the top level). Visgroup memberships
  are carried with the named visgroups' definitions; undefined ids are not.
- Paste gives every object a new runtime id and fresh VMF ids (solid, each
  side, entity, group), remaps owners and groups inside the fragment, and
  remaps overlay `sides`/`sides2` ids of sides pasted in the same copy
  (other ids kept). Visgroups: same id and name, else first by name, else
  created at the top level with the name and color.
- Copy k (1-based) uses k x offset and k x angles about the fragment's bounds
  center, with texture lock. `group` wraps each copy in its own new group.
- Name fix: prefix/suffix, then legacy `IncrementStringName` until unused in
  the target and by earlier pasted names; key values, connection targets and
  parameters equal (case-insensitively) to an old name are rewritten within
  the copy; outside and wildcard references are untouched.
- `created` receives what the paste selects: each copy's group, or each
  copy's top-level objects, in creation order.
- Refusals: empty fragment (`Nothing`); copy count out of range, non-finite
  offset or angles, degenerate transforms (`Rejected`), all before anything
  is staged.

## 4. Ownership, threading

Fragments are plain values with no reference into a document. Paste is a
pure function over the edit. Single sequence.

## 5. Invariants

Every result passes `ValidateEdit` and commits; VMF ids stay unique; the
source document is never modified (Duplicate reads the edit it writes to).

## 6. Side effects and performance

Linear in fragment size x copies; one visgroup scan per fragment visgroup.

## 7. Conformance suite and providers

`test_clipboard.cpp` (84 checks, gcc and clang, `-Wall -Wextra -Werror`):
copy rules, same- and cross-document paste, id freshness, overlay remap,
visgroup matching and creation, cumulative offsets and rotation, per-copy
groups, name fixes and reference updates, Duplicate, and negative checks
that stage nothing.
