# Contract: `geometry.transform.v1`

Module: `world.map-geometry`
Header: `public/mapgeometry/transform.h` · Impl: `mapgeometry/transform.cpp`
Conformance: `unittests/hammertest/geometry/test_transform.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The one owner of map-space rotation and affine math: Source Euler angles to and
from matrices, axis rotations, scales and mirrors, composition, inversion and
plane transformation. Consumers: `hammer.app` transform operations, the
`content.vmf` placement transform (`vmf::AngleMatrix` routes here), viewport
cameras. Required.

## 2. Accepted inputs

- Angles in degrees (pitch, yaw, roll; Source convention: columns are the
  rotated forward, left and up axes). Any finite values.
- Matrices act on column vectors. `Affine` is `linear * p + translation`.

## 3. Results and guarantees

- Quarter-turn `AxisRotation` is exact (no sin/cos rounding), so grid points
  stay on the grid.
- `MatrixToAngles(AngleMatrix(a))` reproduces the matrix; at gimbal lock roll
  is 0.
- `Compose(a, b)` applies `b` first. `Inverse` returns nothing for a singular
  linear part.
- `TransformPlane` keeps the image of the original outside as the new outside,
  including under mirrors and non-uniform scale; nothing for a singular map.

## 4. Ownership, threading

Pure value functions; no state, no allocation; thread-safe.

## 5. Invariants

`Affine::About(m, pivot)` fixes `pivot`. `Mirrors()` is true exactly when the
determinant is negative.

## 6. Side effects and performance

None. Constant time.

## 7. Conformance suite and providers

`test_transform.cpp` (27 checks, gcc and clang): convention checks, angle
round trips, exact quarter turns, pivot/compose/inverse, plane orientation under
translation, mirror and scale; negative checks for singular maps.
