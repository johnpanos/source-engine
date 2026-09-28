# Contract: `presenters.face_inspector.v1`

Module: `hammer.presenters`
Header: `public/hammer/presenters/face_inspector.h` · Impl: `hammer/core/presenters/face_inspector.cpp`
Conformance: `unittests/hammertest/presenters/test_face_inspector.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The Face Edit sheet and texture bar model (legacy `faceedit*.cpp`,
`texturebar.cpp`): aggregates of the selected faces and texture edits.
Consumers: the GTK face panel, the texture tool.

## 2. Accepted inputs

A session and an optional `IMaterialInfo`. Edits: `TextureValues`, shift
deltas, a justification (with treat-as-one and fit counts), an alignment, a
material name.

## 3. Results and guarantees

- Fields material, shift U/V, scale U/V, rotation and lightmap scale are
  Unset/Single/Mixed over the live selected faces (exact comparison).
- `materialKnown` is false only when the port says a Single material is
  missing.
- Each edit is one `Execute` through `ops::SetTextureValues`,
  `ShiftTexture`, `JustifyTexture`, `AlignTexture` or `ApplyMaterial`, with a
  readable label ("Set texture scale", "Set texture values", "Shift texture",
  "Justify texture fit", "Align texture to world", "Apply material M").
- No faces, a missing port for justify, or an operation refusal changes
  nothing and sets `LastError()`.

## 4. Ownership, threading

Observes; owns no document state. Single sequence. RAII subscription.

## 5. Invariants

`Faces()` lists only faces that exist.

## 6. Side effects and performance

Rebuilds on every session event: O(selected faces).

## 7. Conformance suite and providers

`test_face_inspector.cpp` (27 checks, gcc and clang) with the fake material
port; negative checks for empty selection, zero scale, missing materials,
missing port and replacement.
