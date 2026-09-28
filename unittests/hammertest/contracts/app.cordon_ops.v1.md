# Contract: `app.cordon_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/cordon_ops.h` · Impl: `hammer/core/app/ops/cordon_ops.cpp`
Conformance: `unittests/hammertest/app/test_cordon_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The Cordon tool's commands over `DocumentSettings::cordons`,
`cordonsActive` and `cordonForm`, and the one cordon query that builds,
tools and the viewport use (`ActiveCordonBoxes`, `InsideCordons`).
Required for cordoned builds.

## 2. Accepted inputs

A `DocumentEdit`; cordon and box indices; names (any text); boxes that are
finite with `mins < maxs` on every axis.

## 3. Results and guarantees

- `AddCordon` appends an active cordon with one box and reports its index.
  `RemoveCordon`, `RenameCordon`, `SetCordonBox`, `AddCordonBox`,
  `RemoveCordonBox`, `SetCordonActive` and `SetCordonsEnabled` do what they
  name; a cordon keeps at least one box.
- Form: when the content no longer fits the older single `cordon` block (one
  unnamed cordon, one box, its flag equal to the document enable), or cordons
  are first added to a map with none, the form becomes `List`; it never
  switches back to `Single`. In the `Single` form the cordon flag and the
  document enable are one key: either setter moves both.
- Query: when enabled, the boxes of every active cordon, in cordon then box
  order; an object's bounds are inside when they intersect any active box,
  edges inclusive (legacy `IsCulledByCordon`). No active box: no restriction.
- Refusals (`Rejected`): bad cordon or box index, degenerate or non-finite
  boxes, removing a cordon's last box. No-ops are `Nothing`.

## 4. Ownership, threading

Pure functions over the edit's settings; no state. Single sequence.

## 5. Invariants

Every cordon has at least one valid box after any operation here, and the
recorded form can express the content (the VMF codec's rule).

## 6. Side effects and performance

Settings are copied on first write; the query is linear in boxes.

## 7. Conformance suite and providers

`test_cordon_ops.cpp` (49 checks, gcc and clang, `-Wall -Wextra -Werror`):
add/remove/rename, box edits, flags, the query (inclusive edges, straddling,
disabled and inactive cordons), the Single/List form rule from a Single-form
document, and negative checks for indices, degenerate/NaN/infinite boxes and
the last box.
