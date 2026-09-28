# Contract: `viewport.picking.v1`

Module: `hammer.viewport` (reads `hammer.scene`, optional `hammer.ports`
`IEntityCatalog`)
Header: `public/hammer/viewport/picking.h`, `public/hammer/viewport/view_policy.h`
· Impl: `hammer/core/viewport/picking.cpp`, `hammer/core/viewport/view_policy.cpp`
Conformance: `unittests/hammertest/viewport/test_picking.cpp`
Migration: R08-DOMAIN (supersedes `EditorController`'s pick-by-ray and the GTK
shell's 2D projection pick once they are hooked to `hammer.viewport`)

Hit testing for the viewports: what lies under a 2D click, along a 3D ray, and
inside a 2D marquee, with the one hit-ordering policy RFC 0002 requires tools
to share.

## 1. Purpose, consumers, scope

- `Pick2D`, `PickRay`, `MarqueeSelect`, `SelectionTarget`.
- Shared policies in `view_policy.h`: the visibility default and override,
  entity marker boxes, the unique-edge rule.
- Consumers: `hammer.tools` (selection, entity placement on surfaces, face
  picks), the session command layer (`raycast`). Required.
- Out of scope: selection state (callers pass results to the session), face
  picking in 2D, displacements, studio-model hulls.

## 2. Accepted inputs

- A `DocumentReader` (committed document or staged edit), a `Camera2D` with a
  viewport for 2D picks and marquees, a ray origin and a non-zero direction.
- `PickOptions`/`MarqueeOptions`: optional catalog (marker sizes), visibility
  predicate (empty = `scene::IsVisible`), fallback marker half-size (8),
  edge tolerance (3 px), centre-handle radius (4 px), `maxDistance` (ray),
  solid/entity filters, and `ownersForBrushEntities` (marquee, default true).
- Non-finite pixels, rectangles or rays, a zero direction and a camera without
  a viewport yield no hits; negative tolerances count as zero.

## 3. Results and guarantees

- Hittable: solids with faces and point entities with a parsable origin that
  the predicate shows. Hidden, visgroup-hidden and hidden-container objects are
  not hittable by default. Brush entities are hit through their solids; groups
  never directly.
- 2D (legacy rule): a solid within the edge tolerance of a projected edge
  (`SolidEdge`) or within the handle radius of its projected bounds centre
  (`SolidCenter`), the nearer reported; a point entity inside or within the edge
  tolerance of its projected marker (`EntityMarker`, distance 0 inside).
  Clicking inside a solid away from edges and handle misses.
- Markers: catalog `boxMins/boxMaxs` around the origin (unrotated), else
  +/- half-size.
- Every hit names `object` (solid or point entity) and `owner` (the brush
  entity of a solid, else invalid); `SelectionTarget` is the owner when valid.
- Ordering: 2D by distance, then projected bounds area, then id; ray by t,
  then id. Each object at most once.
- Ray: entry into a solid's convex geometry (`scene::BuildGeometry`) or a
  marker box, with t in world units along the normalized direction, the entry
  point, the outward normal of the entered face and, for solids, the `FaceRef`
  (solid id, side VMF id). A ray starting inside a volume does not hit it;
  hits beyond `maxDistance` are dropped.
- Marquee: projected bounds `Inside` (inclusive) or `Touching` the rectangle;
  with owner mapping a brush entity is selected when every (Inside) or any
  (Touching) shown solid qualifies. Results are unique and in id order.

## 4. Ownership, threading

- Pure functions of borrowed inputs; results are owned values. The catalog is
  borrowed for the call and is immutable (safe to share). The predicate must
  not mutate the document.

## 5. Invariants

- The same visibility, marker and edge policies serve picking and extraction
  (`view_policy.h`), so what is drawn is what can be clicked.
- Hit lists satisfy the documented order.

## 6. Side effects and performance

- None. Each call is O(number of objects), building each solid's geometry once
  per call; no caching (extraction owns the cached geometry).

## 7. Conformance suite and providers

- `test_picking.cpp` (62 checks) with a local in-memory catalog: edge, centre
  and marker picks with tolerances; area and id tie-breaks; brush-entity
  solid/owner hits (an origin on a brush entity is not a marker); catalog box
  sizes in Top and Front views; quick-hidden, visgroup-hidden and originless
  objects; predicate override; ray hits sorted by t with t, point, normal and
  the side's `FaceRef`; inside-origin, beside, `maxDistance`, zero and NaN
  rays; marquee Inside/Touching, corner order, owner mapping and its opt-out,
  hidden objects.
- Negative control: a reversed hit list fails the ordering oracle.
- Runs headlessly with g++ and clang++ (`-std=c++20 -Wall -Wextra -Werror`).
