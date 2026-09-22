# Contract: `geometry.brush.v1`

Module: `hammer.geometry` · Types: `hammer::geometry::Plane`, `BrushFace`, `BrushSolid`, `WorldScene`
Header: `public/hammer/geometry/brush.h` · Impl: `hammer/core/geometry/brush.cpp`
Conformance suite: `unittests/hammertest/geometry/test_brush.cpp`
Migration: `HAM-GEOMETRY-001`

This is the RFC 0002 required contract record for the headless VMF → convex brush
geometry bridge — the renderable form of a Source brush that the GTK desktop shell
and any viewport consume. It depends only on the C++ standard library and
`hammer.formats` (the VMF/keyvalues model): no MFC, tier0, platform.h, PCH, or GPU.
Any substitute implementation must satisfy every clause here and pass the shared
suite.

## 1. Purpose, consumers, required vs optional

Convert a parsed VMF document (blocks of `world`/`entity` → `solid` → `side` with
`plane` values) into per-face convex polygons with outward normals. Consumers: the
3D viewport of the `linux-gtk-desktop` shell (mesh upload) and the outliner (entity
summary). All capabilities below are **required**; there are no optional ones in v1.

## 2. Accepted inputs, units, spaces, encoding, limits

- A VMF `plane` value is exactly three parenthesised points
  `(x1 y1 z1) (x2 y2 z2) (x3 y3 z3)`; components are decimal (optionally signed /
  fractional). Whitespace between tokens is flexible. Wrong point count, a
  non-numeric or short component, or unbalanced parentheses is **rejected**
  (`ParsePlanePoints` → `nullopt`), never partially accepted.
- Coordinates are world units (`double` internally for clip robustness), well within
  the Source map bound of ±32768; the seed quad half-extent is 1e5.
- A `solid` needs **at least four** valid, non-degenerate side planes to bound a
  finite region; fewer yields a solid with no faces.

## 3. Results, error taxonomy, guarantees (success / failure / cancel)

- All operations are total and synchronous; there is no cancellation channel.
  Parsing failures surface as `std::optional` empties; geometric failure (a plane
  set that does not bound a finite convex region) surfaces as an **empty face list**,
  never as an unbounded or partial polygon.
- `PlaneFromPoints` returns `nullopt` for collinear points (degenerate plane).
- `BuildSolidFromPlanes`/`BuildSolidFromBlock` produce, for a well-formed brush,
  one convex polygon per contributing plane; redundant planes (that clip to no
  face) are dropped.

## 4. Convex-intersection and orientation clauses (core)

- **Convex intersection:** each face is the seed plane clipped by every other
  plane's interior half-space, so every produced vertex lies within the brush
  bounds. (The `.sensitivity` suite proves this by catching an unclipped provider.)
- **Outward orientation is winding-independent:** the outward direction of each
  side plane is derived from a point known to be interior to the brush (the
  centroid of the side reference points), **not** from the VMF point winding. The
  same solid is produced whether the source planes are wound outward or inward.
- **Unit normals:** every `BrushFace::plane.normal` is unit length; every face
  vertex satisfies `dot(normal, v) == dist` within tolerance.
- **CCW winding:** face vertices are ordered counter-clockwise about the outward
  normal (positive signed area along the normal), so front-face culling is
  consistent for a viewport.
- **Materials:** when the side count matches, face *i* carries side *i*'s
  `material` string (empty when absent); face order follows side order.

## 5. Scene import

- `BuildSceneFromDocument` imports every `solid` under `world` and under each
  `entity` (brush entities) into `WorldScene::solids`, accumulating overall bounds
  over resolved vertices only.
- Each `entity` is summarised once (`classname`, `targetname`, parsed `origin` when
  present, and its solid count). A point entity (no solids) contributes an entity
  summary but no geometry. Empty/degenerate solids are counted in the entity's
  `solidCount` but contribute no faces.

## 6. Ownership, threading, side effects, performance

- Pure value transforms over caller-owned inputs; no heap ownership of external
  state, no globals, no I/O, allocation-only-for-results, no logging. Not
  internally synchronized; distinct calls are independent and order-free.
- Per solid: O(sides²) clipping — bounded and small for authored brushes.

## 7. Conformance suite and executing providers

- `test_brush.cpp` runs the positive suite against the real provider: plane-value
  parsing (valid/invalid), plane derivation (incl. collinear rejection), cube
  reconstruction from both explicit planes and VMF `solid` text (6 quads, exact
  `[0,64]³` bounds, unit outward normals, convexity, CCW winding, materials), the
  <4-plane empty case, and whole-document scene import (world + brush-entity solids,
  entity summary, origin parsing).
- `test_brush_negative.cpp` runs the convex-intersection clause against a
  deliberately-broken provider (`BuggyBuild`, seed quads never clipped) and asserts
  the oracle **detects** the violation — proving it is not vacuous.
- Both run headlessly via `unittests/hammertest/run_headless.sh` under gcc and clang
  with `-Wall -Wextra -Werror`, on the `linux-headless-core` profile.

## Compatibility notes

`v1` covers brush (plane) geometry only. Displacements, texture UV/lightmap axes,
and non-planar side data are preserved verbatim in the keyvalues tree by
`hammer.formats` but are **not** part of this contract; they are later migrations.
