# Contract: `presenters.status_bar.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/status_bar.h` · Impl: `hammer/core/presenters/status_bar.cpp`
Conformance: `unittests/hammertest/presenters/test_status_bar.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The status bar texts: selection summary and size, pointer coordinates, grid
and snap, active tool and a transient message slot.

## 2. Accepted inputs

A session and the `app::EditorSettings` owner; the pointer's world point with
its `viewport::ViewKind`; a tool name; errors or messages.

## 3. Results and guarantees

- Summary: solids, entities, groups, faces with singular/plural nouns joined
  by ", "; the entity count names the class when all share one ("2 solids, 1
  entity (light)"); "No selection" when empty.
- Size: "w <x> h <z> d <y>" extents of the selection's object bounds and
  selected face polygons; "" without extent.
- Pointer: the view's two axes (Top x y, Front x z, Side y z) or all three in
  3D, rounded to 0.01.
- Grid and snap read `EditorSettings` live ("Grid 64", "Snap on/off").
- The message stays until cleared, replaced, or the next committed document
  change; `Report` shows only failures.

## 4. Ownership, threading

Owns pointer/tool/message view state; grid and snap belong to
`EditorSettings`. Single sequence. RAII subscription.

## 5. Invariants

Selection texts always describe the session's current selection.

## 6. Side effects and performance

O(selected objects) per event, plus geometry of solids with selected faces.

## 7. Conformance suite and providers

`test_status_bar.cpp` (26 checks, gcc and clang); negative checks for the
empty selection, success reports and destruction order.
