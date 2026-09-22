# Contract: `formats.overlay.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/overlay.h` · Impl: `hammer/core/formats/overlay.cpp`
Conformance: `unittests/hammertest/formats/test_overlay.cpp` (+ `_negative`)
Migration: `HAM-OVERLAY-001`
Depends on: `hammer.formats` (keyvalues codec), `hammer.geometry` (`Vec3d`)

Parses `info_overlay` entities (a material projected onto brush faces via a stored
basis) into a structured `Overlay` and computes the world-space quad a renderer
would draw. Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own overlay geometry. Consumers: the renderer (draws the projected quad) and the
overlay edit tool. Required for maps using `info_overlay`.

## 2. Accepted inputs

- `ParseOverlay( entity )`: one entity block. Required keys: `classname` ==
  `info_overlay`, `material`, non-empty `sides` (space-separated face ids),
  `BasisOrigin`/`BasisU`/`BasisV`/`BasisNormal` (vectors), `uv0`..`uv3` (corner
  vectors). Optional: `StartU`/`EndU`/`StartV`/`EndV` (default 0/1/0/1).
- `ExtractOverlays( root )`: every valid `info_overlay` in a document.

## 3. Results and guarantees

- On success an `Overlay` with material, `sides`, the four basis vectors, the four
  `uv` corners, and the texture range.
- `WorldCorners()` = `BasisOrigin + BasisU·uv.x + BasisV·uv.y + BasisNormal·uv.z`
  per corner.
- `ParseOverlay` returns `nullopt` when the entity is not an `info_overlay` or a
  required field is missing/malformed. `ExtractOverlays` skips non-overlay and
  invalid entities.

## 4. Ownership, threading

- Pure functions over the input tree; no globals, no I/O.

## 5. Invariants

- **Projection**: a corner's world position is the basis-weighted sum of its uv.
- **Completeness**: an overlay missing material/sides/a basis vector/a uv corner
  does not parse (no partial overlay is produced).

## 6. Side effects and performance

- O(entities); no I/O or globals.

## 7. Conformance suite and providers

- `test_overlay.cpp`: material, `sides` list, basis, and `WorldCorners()` for a
  known basis (checked corner positions), with a non-overlay entity ignored.
- `test_overlay_negative.cpp`: proves a complete overlay parses while missing
  material/sides/BasisU/a uv corner, a two-component basis vector, and a
  non-`info_overlay` classname are all rejected.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Parses the overlay's geometry/basis and projects its quad. It does not clip the
  quad to the target `sides` faces, render the material, or handle
  `info_overlay_transition` (water overlays). Those are later increments.
