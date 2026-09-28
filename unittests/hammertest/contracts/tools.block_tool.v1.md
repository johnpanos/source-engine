# Contract: `tools.block_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/block_tool.h` · Impl: `hammer/core/tools/block_tool.cpp`
Conformance: `unittests/hammertest/tools/test_block_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Draw a pending box and create a primitive or arch from it (legacy
`ToolBlock.cpp`, Source 2's workplane Block tool). All views; Left button.

## 2. Accepted inputs

2D drags (new box, handle resize, move inside); 3D base drags on the surface
workplane and height drags on the pending box; Enter and Escape;
`EditorSettings` face texture, primitive and arch; `ToolSettings::makeArch`.

## 3. Results and guarantees

- The pending box is tool state; no edit until Enter.
- 2D: snapped corners in the view's axes; depth from the current or previous
  box, else the selection bounds, else [0, grid]. Handles snap moved edges;
  a drag inside moves (Shift constrains, Alt frees). Clicks and flat drags
  keep the previous box.
- 3D: the workplane is perpendicular to the hit normal's dominant axis at the
  snapped hit coordinate (no hit: z = 0); the base grows along the normal one
  grid step tall; a height drag moves the far face along the height axis
  (snapped), flipping past the base. No workplane under the pointer: `Failed`.
- Enter: one `ExecuteSelecting` ("Create block", "Create primitive" or
  "Create arch") selecting the new object, primitive height axis = the
  drawing view's depth axis or the workplane axis; a refusal is `Failed` and
  keeps the box. Escape cancels a drag (box restored) or discards the box.
  Deactivation discards it.

## 4. Ownership, threading

Owns the pending box and the previous box (the depth rule). Single sequence.

## 5. Invariants

A pending box always has positive extents on all axes.

## 6. Side effects and performance

None before Enter.

## 7. Conformance suite and providers

`test_block_tool.cpp` (56 checks, gcc and clang). Negative checks: clicks,
flat drags (Error preview), Escape and focus loss mid-drag, a refused
primitive, Enter without a box, a workplane behind the camera.
