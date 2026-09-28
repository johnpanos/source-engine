# Contract: `tools.selection_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/selection_tool.h` · Impl: `hammer/core/tools/selection_tool.cpp`
Conformance: `unittests/hammertest/tools/test_selection_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Selection, move, scale and rotate (legacy `ToolSelection.cpp` and `Box3D.cpp`,
Source 2 ergonomics). All views; Left button.

## 2. Accepted inputs

Pointer presses on handles, objects or empty space; drags; clicks; Ctrl for
toggling and adding; Shift constraint; Alt free; arrows, Delete, Escape.

## 3. Results and guarantees

- Selection changes happen on release, never on press. Click: select
  (Replace); Ctrl toggles; clicking a selected object cycles the 2D handle
  mode Scale <-> Rotate; a click on empty space clears (Ctrl keeps).
  Below-threshold drags are clicks.
- 2D drag on an object: move (one `Translate` "Move", snapped reference =
  bounds minimum); moving an unselected object selects it in the same unit.
- 2D drag on empty space, or with Ctrl: marquee (settings' mode; Ctrl adds).
- Scale handles: one `ScaleToBox` "Scale" (inverted or flat: `Failed`, no
  edit). Rotate handles: one `Rotate` "Rotate" about the bounds center in the
  view plane (0.5 degree steps, Shift 15, Alt free). Legacy's shear is not
  offered. The mode resets to Scale when the selection changes.
- 3D: click selection by ray; a drag makes no edit.
- Arrows nudge ("Nudge"), Delete deletes ("Delete"), Escape cancels a gesture
  or clears the selection.
- Escape, focus/capture loss and tool switches mid-gesture: no edit, same
  selection.

## 4. Ownership, threading

Owns the gesture, the handle mode and the hover handle. Single sequence.

## 5. Invariants

At most one `Execute` per gesture; previews come from the same operation the
release commits.

## 6. Side effects and performance

A preview stages the operation per overlay request (copies of the moving
objects).

## 7. Conformance suite and providers

`test_selection_tool.cpp` (80 checks, gcc and clang). Negative checks:
Escape/focus/capture loss mid-drag, inverted scale `Failed`, other buttons and
hovers `Ignored`, 3D drags make no edit, keys with nothing selected `Ignored`.
