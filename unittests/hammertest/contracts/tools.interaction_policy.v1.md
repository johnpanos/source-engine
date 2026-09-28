# Contract: `tools.interaction_policy.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/interaction_policy.h`, `public/hammer/tools/box_handles.h`, `public/hammer/tools/preview.h` · Impl: `hammer/core/tools/interaction_policy.cpp`, `box_handles.cpp`, `preview.cpp`
Conformance: `unittests/hammertest/tools/test_interaction_policy.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The one owner of the interaction policies tools share (RFC 0002: snapping,
drag thresholds, transform constraints and hit ordering have one owner),
the 2D box handles the Selection and Block tools share, and gesture previews.
Consumers: every tool and the camera controller.

## 2. Accepted inputs

Screen points, plane points and world points; the grid policy; modifiers;
view kinds and keys; a `DocumentReader` and a camera for picking.

## 3. Results and guarantees

- Drag threshold: more than 3 px on either axis (legacy per-axis rule).
- Snapping through `viewport::GridPolicy`; Alt disables it (legacy). Drags
  snap a reference point: `DragDelta` lands reference + delta on the grid.
- Shift keeps the dominant view axis (ties keep u).
- Angles step 0.5 degrees, Shift 15, Alt free.
- Nudge: arrows along the view axes (3D: world X/Y), grid size, or one unit
  with Ctrl, `NudgeStep::Unit` or snapping off.
- Hit ordering: the viewport's first hit resolved by `app::ResolvePick`;
  marquee results resolved and deduplicated.
- Box handles: 8 (scale) or 4 corner (rotate) handles 6 px outside the box,
  hit within 4 px, corners first; resizing snaps each moved edge.
- Previews stage the committing operation on a scratch edit and describe
  touched objects (solid edges, entity markers); nothing on refusal.

## 4. Ownership, threading

Pure functions. No state.

## 5. Invariants

The same inputs give the same results; previews never change the document.

## 6. Side effects and performance

None. Picking cost is the viewport's.

## 7. Conformance suite and providers

`test_interaction_policy.cpp` (51 checks, gcc and clang): thresholds, snapping
with ties and Alt, constraints, rotation steps, nudges per view, `GridFrom`,
box handles and hit ordering over groups and brush entities. Negative checks:
sub-threshold motion, disabled grids, non-arrow keys, misses.
