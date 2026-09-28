# Contract: `app.decal_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/decal_ops.h` · Impl: `hammer/core/app/ops/decal_ops.cpp`
Conformance: `unittests/hammertest/app/test_decal_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Decal and overlay placement (legacy `CToolDecal`, `CToolOverlay`,
`CMapOverlay` basis, handles and transform): infodecal and info_overlay
entities with complete keys, face-list and size edits, and the overlay basis
transform. Consumers: the decal and overlay tools, the face inspector, the
clipboard/prefab/instance paths (`TransformedOverlay`) and the command layer.
Overlay clipping and rendering, shoreline and water overlays, and the
per-axis flip toggles are out of scope. Required.

## 2. Accepted inputs

Faces as `FaceRef` (solid id + side VMF id) that exist in the edit; a
non-empty material; finite points; finite positive width and height; at
most 64 distinct faces per overlay (duplicates collapse).

## 3. Results and guarantees

- `PlaceDecal`: an `infodecal` with `texture` = material and `origin` = the
  point.
- `PlaceOverlay`: an `info_overlay` with `material`, `sides` (face side ids),
  `RenderOrder` 0, `StartU`/`EndU`/`StartV`/`EndV`, `BasisOrigin`/`BasisU`/
  `BasisV`/`BasisNormal`, `uv0`..`uv3` and `origin`. Basis from the first
  face: N = unit outward normal; initial U = +X when N's major axis is Y or
  Z, +Y when it is X; V = normalize(N x U); U = normalize(V x N); origin =
  the point projected onto the face plane; texture coordinates 0/1/1/0, or
  1/0/0/1 when U's and V's largest components differ in sign; corners
  (-w/2,-h/2), (-w/2,h/2), (w/2,h/2), (w/2,-h/2) with third component 0.
- `SetOverlayFaces` replaces `sides`; the basis is rebuilt about the entity
  origin only when the first face changes; corners are kept.
  `SetOverlaySize` rewrites the corners keeping their third components.
  Unchanged requests are `Nothing`.
- `TransformedOverlay`: `BasisOrigin` mapped as a point; rigid linear parts
  rotate the axes; others keep normalized U, V and map the corners
  (`CMapOverlay::DoTransform`).
- Refusals (`Rejected`, nothing staged): unknown or missing faces, too many
  faces, empty material, bad sizes, non-finite points, degenerate first
  faces, non-overlay targets.

## 4. Ownership, threading

Pure functions over the edit. Single sequence.

## 5. Invariants

The basis is orthonormal with U and V in the first face's plane; every
result passes `ValidateEdit`.

## 6. Side effects and performance

Constant per face; one entity write per operation.

## 7. Conformance suite and providers

`test_decal_ops.cpp` (58 checks, gcc and clang, `-Wall -Wextra -Werror`):
orthonormal in-plane bases on all box faces and a sloped plane, the axis
and flip rules on floor, wall and ceiling, full key sets, face-list and size
edits, rotation/scale/translation of the basis, the 64-face limit, and
negative checks that stage nothing. Seeded fault detected: the initial-U
axis rule changed.
