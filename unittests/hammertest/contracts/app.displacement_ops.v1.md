# Contract: `app.displacement_ops.v1`

Module: `hammer.app`
Headers: `public/hammer/app/ops/displacement_ops.h`, `public/hammer/scene/displacement_geometry.h` ·
Impl: `hammer/core/app/ops/displacement_ops.cpp`, `hammer/core/scene/displacement_geometry.cpp`
Conformance: `unittests/hammertest/app/test_displacement_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Displacement editing on the typed `scene::Displacement`: create and destroy,
power changes, sculpting (raise, lower, set, smooth), alpha painting,
elevation and sewing. Consumers: a displacement tool, the face inspector, the
command layer. Required.

## 2. Accepted inputs

Face references (solid id + side VMF id); powers 2..4; a spherical brush
(center, radius > 0, amount, mode, optional direction, falloff).

## 3. Results and guarantees

- Creation needs quad faces that are not displaced; the start corner is the
  smallest (x, y, z) corner; every array is sized for the power.
- Sculpting edits the displacement vector (normal x distance) of vertices whose
  displaced position is within the radius, with linear falloff by default, and
  stores it back as a unit normal and a distance; offsets are unchanged.
- Legacy triangle tags follow every geometric edit: walkable at normal z >= 0.7,
  buildable at >= 0.8; force bits are kept.
- Power changes resample vectors, offsets and alphas bilinearly, so heights at
  grid points shared by both powers are kept.
- Sewing moves coincident base vertices of different faces to the average of
  their displaced positions.
- A malformed displacement, a non-quad face, a power out of range or a radius of
  0 refuses the operation; a brush that reaches nothing is Nothing.

## 4. Ownership, threading

Pure functions over a borrowed staged edit.

## 5. Invariants

Array lengths always match the power.

## 6. Side effects and performance

Surfaces are rebuilt per operation (O(vertices)).

## 7. Conformance suite and providers

`test_displacement_ops.cpp` (44 checks, gcc and clang), with refusals for each
invalid input.
