# Contract: `tools.face_tool.v1`

Module: `hammer.tools`
Header: `public/hammer/tools/face_tool.h` · Impl: `hammer/core/tools/face_tool.cpp`
Conformance: `unittests/hammertest/tools/test_face_tool.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Face selection and texture application in the 3D view (legacy Toggle Texture
Application, Source 2 Faces mode). 3D view only.

## 2. Accepted inputs

Left and Right clicks with Ctrl/Shift/Alt; Escape.

## 3. Results and guarantees

- Actions happen on the release of a press that stayed under the threshold;
  a press that drags is abandoned (`CaptureReleased`, no effect).
- Left: select the first solid face hit (Replace); Ctrl toggles; Shift takes
  every face of the solid (Ctrl adds); Alt lifts the face texture
  (`LiftedTexture()` and the lift callback; the host writes the settings);
  a click on nothing clears (Ctrl keeps).
- Right: "Apply material" (`ApplyMaterial`, the editor's active material).
  Alt+Right: "Apply texture" (`ApplyTextureFrom` MaterialValues) from the
  first selected face (lowest FaceRef) with the active material.
- Right click on nothing, no active material, Alt+Right without a selected
  face: `Failed`, no edit.

## 4. Ownership, threading

Owns the press and the lifted texture. Never writes `EditorSettings`.

## 5. Invariants

At most one edit per click.

## 6. Side effects and performance

One ray per click.

## 7. Conformance suite and providers

`test_face_tool.cpp` (40 checks, gcc and clang). Negative checks: failures
above, drags, Escape and focus loss, 2D events `Ignored`.
