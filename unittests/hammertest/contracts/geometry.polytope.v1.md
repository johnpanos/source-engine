# Contract: `geometry.polytope.v1`

Module: `world.map-geometry`
Header: `public/mapgeometry/polytope.h` · Impl: `mapgeometry/polytope.cpp`
(plus `BrushFace::sourcePlane` in `brush.h`)
Conformance: `unittests/hammertest/geometry/test_polytope.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Convex polytope predicates and constructions for brush editing: plane-side
classification, containment, closed-solid detection, convex hulls (the vertex
tool's rebuild), separating-axis overlap (carve), and the integer snap policy
for computed vertices. Consumers: `hammer.scene` geometry and validation,
`hammer.app` clip/carve/vertex operations, viewport picking. Required.

## 2. Accepted inputs

Planes as unit normal plus distance (outside positive). Point sets of any size.
Tolerances are explicit parameters: `kPlaneEpsilon` (0.01 units) and
`kIntegerSnapEpsilon` (1e-4).

## 3. Results and guarantees

- `ClassifyPoints` reports Front/Back/On/Spanning with the stated epsilon.
- `IsClosedSolid` is true only for plane sets that bound a solid with at least
  four faces inside +/-65536 (an open set's clipped faces reach the working
  quad and are rejected).
- `ConvexHullPlanes` returns one outward plane per hull face or nothing for
  fewer than four or coplanar points.
- `SolidsOverlap` is false for touching or separated solids.
- `BuildSolidFromPlanes` faces name their input plane in `sourcePlane`.
- `SnapNearIntegers` snaps coordinates within epsilon of an integer and
  recomputes bounds.

## 4. Ownership, threading

Pure functions over values; thread-safe.

## 5. Invariants

Hull planes contain every input point; overlap is symmetric.

## 6. Side effects and performance

The hull is O(n^4) in the number of points, intended for brush-sized sets
(tens of points).

## 7. Conformance suite and providers

`test_polytope.cpp` (23 checks, gcc and clang), with negative checks for open
plane sets, coplanar and too-small hull inputs, and a cut corner that only an
edge axis separates.
