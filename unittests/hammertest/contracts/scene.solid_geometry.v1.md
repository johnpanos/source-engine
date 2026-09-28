# Contract: `scene.solid_geometry.v1`

Module: `hammer.scene`
Header: `public/hammer/scene/solid_geometry.h` · Impl: `hammer/core/scene/solid_geometry.cpp`
Conformance: `unittests/hammertest/scene/test_solid_geometry.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Geometry of document solids through one tolerance policy: face polygons with
side mapping, bounds, side points from polygons and planes, side normalization,
box solids and world-aligned texture axes. Consumers: viewport, picking,
operations, codecs. Required.

## 2. Accepted inputs

Solids with any number of sides. Polygons counter-clockwise from outside with
at least three vertices.

## 3. Results and guarantees

- `BuildGeometry` faces carry `sourcePlane` (side index) and the side's
  material; vertices within 1e-4 of an integer are snapped to it.
- `PointsFromPolygon`/`PointsFromPlane` produce points whose `Side::Plane()` is
  the polygon's or plane's outward plane.
- `NormalizeSides` re-derives points from built faces and drops redundant
  sides, keeping each surviving side's data; it refuses sides that do not
  bound a closed solid.
- `MakeBoxSolid` sides face outward with legacy world-aligned axes.

## 4. Ownership, threading

Pure functions over values.

## 5. Invariants

A normalized solid's sides each bound exactly one face.

## 6. Side effects and performance

Geometry is rebuilt on each call; callers that draw every frame cache it
(viewport extraction).

## 7. Conformance suite and providers

`test_solid_geometry.cpp` (19 checks, gcc and clang), with a negative check for
open side sets.
