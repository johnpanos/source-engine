# Contract: `app.editor_controller.v1`

Module: `hammer.app` · Type: `hammer::app::EditorController`
Header: `public/hammer/app/editor_controller.h` · Impl: `hammer/core/app/editor_controller.cpp`
Conformance suite: `unittests/hammertest/app/test_editor_controller.cpp`
Migration: `HAM-WORKFLOW-001`

This is the RFC 0002 required contract record for the headless editor **interaction
authority** — the single owner of the core Hammer UX flows (H3/H4's "one
selection/mutation/history owner"). The GTK desktop shell is a thin presenter that
translates gestures/keys into these calls; the conformance UI test drives the same
calls with simulated input, so the tested logic is the shipped logic. It depends
only on `hammer.geometry` (brush bridge + rounding owner), `hammer.formats` (VMF
codec), and `hammer.app` (`DocumentHistory`): no MFC, tier0, platform.h, or GPU.

## 1. Purpose, consumers, required vs optional

Own an editable map of axis-aligned brush boxes and mutate it through normalized
tool input. Consumers: the GTK shell's viewports/tool palette/menu, and this
conformance test. All clauses below are **required** in v1.

The editable brush model is an axis-aligned box (`mins`/`maxs`) plus a material.
This is exactly what the Block tool produces and what serialises to a six-sided
VMF solid. A non-box legacy brush loads as its bounding box — a **declared
limitation** of this slice, not a silent data model.

## 2. Accepted inputs, units, spaces, encoding, limits

- Pointer input is normalized: `PointerDown/Drag/Up(view, u, v)` where `(u, v)` are
  **world** coordinates in that view's two axes (`Top`=X/Y, `Front`=X/Z,
  `Side`=Y/Z). `Camera`-view input is navigation-only and is ignored here.
- `SetGridSize(g)` accepts a positive grid; non-positive is ignored. `SetBlockDepth`
  clamps to ≥ 1.
- VMF text for `LoadVmf` is the `hammer.formats` keyvalues grammar; a parse failure
  returns `false` with the diagnostic and leaves the controller unchanged-or-empty
  per §5.

## 3. Tools and flows (required behavior)

- **Grid snapping:** every pointer coordinate is snapped through the shared
  `hammer::geometry::RoundHalfAwayFromZero` owner (round half away from zero) before
  use, so created/moved brush bounds land on the grid. (The `.sensitivity` suite
  proves this by catching a no-snap provider.)
- **Block tool:** `PointerDown`→`Drag`→`Up` defines a pending rectangle in the
  active view; `Commit()` turns it into a brush extruded by the block depth along
  the view's free axis, selects it, and records **one** history unit. A degenerate
  (zero-area) rectangle creates nothing (`Commit` returns `false`).
- **Selection tool:** `PointerDown` picks the topmost brush whose projection
  contains the point (or clears the selection on empty space); dragging moves the
  selected brush by the snapped delta in the view's two axes; `PointerUp` records
  the move as **one** history unit only if it actually moved. `DeleteSelection()`
  removes the selected brush as one history unit.
- **Switching tool** clears any pending rectangle and in-progress drag.

## 4. History, results, guarantees

- `Undo`/`Redo` move one history unit and restore the recorded brush state; they
  clear the transient selection and pending rectangle. A new mutation after an undo
  discards the redo tail. `IsModified` follows `DocumentHistory` (undoing back to
  the saved/base position clears it). History is the **single** stack: spatial
  operations share it (and future property edits must too).
- All operations are total and synchronous; failure is signalled by a `false`
  return or an unchanged model, never by a partial mutation.

## 5. VMF round-trip

- `ToVmf()` serialises the map to a VMF document (`versioninfo` + a `world` with one
  six-sided `solid` per brush). `LoadVmf(ToVmf())` reproduces the same brushes (as
  boxes) **in order**, and the loaded state is an unmodified base (`IsModified()`
  is false, history reset). Save/load in the host route through the shared
  `SaveDocument` orchestrator and `DiskFileStore` (this contract owns the text
  transform, not the file I/O).

## 6. Ownership, threading, side effects

- Owns its map, selection, pending, and history. No globals, no I/O (text in/out
  only), no logging, no GPU. Not internally synchronized; one controller per editor.
- `BuildScene()` produces a renderable `WorldScene` (every brush + the pending box
  as solid id `kPendingSolidId`) through the shared geometry bridge, so the viewport
  reuses one geometry path.

## 7. Conformance suite and executing providers

- `test_editor_controller.cpp` drives the real controller with simulated input:
  block create + snapping, multi-view extrusion, select/move/deselect/delete,
  undo/redo (incl. redo-tail invalidation and modified-flag reset), input guards
  (degenerate, camera-view, tool-switch), and the headline **build-a-simple-map +
  VMF save round-trip** flow.
- `test_editor_controller_negative.cpp` runs the grid-snapping clause against a
  deliberately-broken no-snap provider and asserts the oracle **detects** it.
- Both run headlessly via `unittests/hammertest/run_headless.sh` under gcc and clang
  with `-Wall -Wextra -Werror`, on the `linux-headless-core` profile.

## Compatibility notes

`v1` covers spatial box editing and VMF box round-trip. Entity/keyvalue property
editing, non-box brush editing, vertex/clip tools, and multi-select are later
migrations; when added they must route through this same authority and history.
