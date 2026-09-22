# Contract: `formats.cordon.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/cordon.h` · Impl: `hammer/core/formats/cordon.cpp`
Conformance: `unittests/hammertest/formats/test_cordon.cpp` (+ `_negative`)
Migration: `HAM-CORDON-001`
Depends on: `hammer.formats` (keyvalues codec), `hammer.geometry` (`Vec3d`)

Filters a VMF document to a cordon box — the world-space bounds that restrict
which geometry a partial compile/export includes (Hammer's "cordon tools").
Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own cordon filtering. Consumers: the partial-compile/export path and the editor's
cordon toggle. Required for cordoned compiles.

## 2. Accepted inputs

- `ApplyCordon( root, box )`: a parsed VMF document and a `CordonBox{mins,maxs}`.

## 3. Results and guarantees

- Returns the filtered `document` plus counts (`solidsKept/Removed`,
  `entitiesKept/Removed`). Keep rules:
  - a **world solid** is kept iff its AABB (union of side-plane points) intersects
    the box (inclusive edges);
  - a **point entity** (`origin` present) is kept iff its origin is inside the box;
  - a **brush entity** (`solid` children) is kept iff any of its solids' AABBs
    intersect the box;
  - a **positionless entity** (no `origin`, no solids) is always kept.
- A box with `mins > maxs` on any axis intersects nothing (all positional geometry
  removed). Non-world/non-entity blocks (versioninfo, cameras, …) are preserved
  verbatim.

## 4. Ownership, threading

- Pure function over the input tree; no globals, no I/O, no shared state.

## 5. Invariants

- **Discrimination**: an all-encompassing box removes nothing; an empty/inverted
  box removes all positional geometry (a constant keep-all or remove-all
  implementation fails these).
- **Inclusive edges**: a solid whose face exactly touches the box boundary is kept.

## 6. Side effects and performance

- One pass over blocks + a bounds pass per solid; O(total plane points). No I/O.

## 7. Conformance suite and providers

- `test_cordon.cpp`: inside/outside/straddling solids, inside/outside point
  entities, a positionless entity, edge-touching inclusivity, and metadata
  preservation.
- `test_cordon_negative.cpp`: proves the filter discriminates by position —
  universe box keeps all, empty/inverted and far-away boxes remove all.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Uses each solid's AABB (union of side-plane points) for the intersection test,
  matching Hammer's brush-bounds cordon; it does not do exact convex-hull vs box
  clipping, and it does not split solids at the cordon boundary. Those are later
  increments.
