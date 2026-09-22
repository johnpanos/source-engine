# Contract: `geometry.displacement.v1`

Module: `hammer.geometry`
Header: `public/hammer/geometry/displacement.h` · Impl: `hammer/core/geometry/displacement.cpp`
Conformance: `unittests/hammertest/geometry/test_displacement.cpp` (+ `_negative`)
Migration: `HAM-DISP-001`
Depends on: `hammer.formats` (keyvalues codec), `hammer.geometry` brush `Vec3d`

Turns a VMF `dispinfo` block plus its host quad face into the subdivided,
per-vertex-displaced surface Source uses for terrain. This is the "later
migration" the brush contract (`geometry.brush.v1`) explicitly excludes. Strict,
headless, MFC-free, GPU-free core: it produces geometry a renderer or scene
bridge consumes; it owns no presentation.

## 1. Purpose, consumers, required vs optional

Own displacement geometry. Consumers: the scene/render bridge (a displaced face
renders as this surface instead of a flat polygon) and, later, the sculpt tools.
Required for any map using displacements.

## 2. Accepted inputs

- `ParseDispInfo( dispBlock )`: a `dispinfo` keyvalues block. Required keys:
  `power` (integer in `[1,4]`), `startposition` (`x y z`, brackets tolerated),
  child blocks `normals` and `distances`. Optional: `elevation` (default 0),
  `offsets` (default all-zero). Grids are `row0`..`row<side-1>`, each row a
  whitespace-separated list — `side` scalars for `distances`, `side*3` for the
  vector grids `normals`/`offsets`, where `side = 2^power + 1`.
- `BuildDisplacementSurface( corners, faceNormal, disp )`: the host face's four
  quad corners in winding order, the face normal (for elevation), and a parsed
  `DispInfo`.

## 3. Results and guarantees

- `ParseDispInfo` returns a `DispInfo` with `power`, `startPosition`, `elevation`,
  and row-major `normals`/`distances`/`offsets` of length `side*side`; or
  `nullopt` on malformed input (see §7).
- `BuildDisplacementSurface` returns a `DisplacementSurface` with `side*side`
  row-major vertices and `2*(side-1)^2` triangles (two per grid cell).
- Vertex position:
  `v(r,c) = bilerp(cornersRotatedToOrigin, s, t) + offset(r,c) + normal(r,c)*distance(r,c) + faceNormal*elevation`,
  with `s = c/(side-1)`, `t = r/(side-1)`.
- **Orientation**: the grid's `(0,0)` corner is whichever of the four corners is
  nearest `startPosition`, matching Hammer/VBSP, so a round-tripped displacement
  keeps its orientation.

## 4. Ownership, threading

- Pure value transforms; no globals, no shared state, no threading concerns.

## 5. Invariants

- **Grid size**: `side == 2^power + 1`; vertex count `side*side`; triangle count
  `2*(side-1)^2`.
- **Flat identity**: with all distances/offsets zero and `elevation` 0, every
  vertex lies exactly on the bilinear base quad (planar for a planar face).
- **Uniform push**: a constant normal `n` and distance `d` push every vertex by
  `d*n` from its base.

## 6. Side effects and performance

- Allocates the vertex/triangle vectors; O(side^2). No I/O, logging, or globals.

## 7. Conformance suite and providers

- `test_displacement.cpp`: grid sizes for powers 2–4; flat-surface bilinear
  interpolation (origin/center/far-corner/planarity); uniform normal displacement;
  and startposition-driven grid orientation. Drives `ParseDispInfo` through the
  real `hammer::formats` keyvalues codec.
- `test_displacement_negative.cpp`: proves `ParseDispInfo` **rejects** a bad power,
  a wrong-length row, a missing row, and an absent `normals` grid, while a
  well-formed block parses (oracle soundness).
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via the shared
  conformance runner on `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Consumes `normals`, `distances`, `offsets`, `elevation`, `power`,
  `startposition`. Does not yet interpret `alphas`, `triangle_tags`, `subdiv`,
  `offset_normals`, or `allowed_verts` (preserved verbatim in the keyvalues tree
  by `hammer.formats`); those are later increments.
