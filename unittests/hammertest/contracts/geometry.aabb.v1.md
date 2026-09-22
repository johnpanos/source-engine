# Contract: `geometry.aabb.v1`

Module: `hammer.geometry` · Type: `hammer::geometry::AxisAlignedBox`
Header: `public/hammer/geometry/aabb.h` · Impl: `hammer/core/geometry/aabb.cpp`
Conformance suite: `unittests/hammertest/geometry/test_aabb.cpp`
Migration: `HAM-GEOMETRY-001`

This is the RFC 0002 required contract record for the first extracted editor port.
It is the authoritative behavioral specification `AxisAlignedBox` claims; any
substitute implementation must satisfy every clause here and pass the shared suite.

## 1. Purpose, consumers, required vs optional

An axis-aligned bounding box over three float axes, extracted from legacy
`hammer/BoundBox`. Consumers: scene bounds accumulation, selection/hit bounds,
grid snapping of extents, 90-degree rotation of extents. All capabilities below
are **required**; there are no optional capabilities in v1.

## 2. Accepted inputs, units, spaces, encoding, limits

- Points and extents are `Vec3` of IEEE-754 `float`, in the document's world units
  (unitless here; no unit conversion is performed or implied).
- `SnapToGrid( gridSize )`: `gridSize` is an integer grid spacing. `gridSize <= 0`
  is a **degenerate input** (see §5).
- `Rotate90( axis )`: `axis` is one of `kAxisX`/`kAxisY`/`kAxisZ`. Other values are
  a programmer error (the legacy `switch` falls through to the X/Y pairing).
- `operator[]` accepts axis index 0..2; out-of-range is a programmer error, not a
  runtime-handled path.

## 3. Results, error taxonomy, guarantees (success / failure / cancel)

- All operations are total and synchronous; there is no failure channel or
  cancellation. Query methods return `bool`/values; mutators return `void`.
- `IsValidBox()` is true iff `mins[i] <= maxs[i]` on every axis.
- `IsIntersectingBox( a, b )` uses **open faces**: boxes that only touch
  (`mins[i] == b.maxs[i]` or `maxs[i] == a.mins[i]`) do **not** intersect.
- `IsInsideBox( a, b )` and `ContainsPoint( p )` use **closed faces** (inclusive).
- `GetBoundsCenter` = `(mins + maxs) / 2` per axis; `GetBoundsSize` = `maxs - mins`.

## 4. Ownership, borrow validity, threading, ordering, reentrancy

- Value type; no heap ownership, no borrowing of external state, no globals.
- Not internally synchronized. A single instance is not safe under concurrent
  mutation; distinct instances are independent. Methods are reentrant and
  order-independent except that mutators observe prior mutations on the same
  instance.

## 5. State invariants and legal operation sequences

- A default-constructed (or `ResetBounds`) box is deliberately **invalid**
  (`mins = +99999`, `maxs = -99999`) so the first `UpdateBounds` seeds it exactly.
- `UpdateBounds` is monotonic: it only grows the box; it never shrinks it.
- `SnapToGrid` preserves the box **size** exactly and snaps the minimum corner to
  the nearest grid multiple (round-half-**away-from-zero**, matching legacy
  `V_rint` = `floor(f+0.5)` for `f>0` / `ceil(f-0.5)` for `f<0`; **not**
  `std::rint`'s round-half-to-even). Verified against the real MSVC-compiled
  legacy `Snap()` in `architecture/hammer_legacy_parity.json`.
- **Declared intentional difference from legacy:** `SnapToGrid( gridSize <= 0 )`
  leaves the box unchanged instead of dividing by the grid (legacy divided by
  zero). This is a characterized refinement, not a silent behavior change.

## 6. Permitted side effects and performance

- No I/O, allocation, logging, or global mutation. O(1) per operation.

## 7. Conformance suite and executing providers

- `test_aabb.cpp` runs the positive conformance suite against the real provider.
- `test_aabb_negative.cpp` runs the intersection/containment clauses against a
  deliberately-broken provider (`BuggyBox`, closed-face intersection) and asserts
  the suite **detects** the violation — proving the oracle is not vacuous.
- Both run headlessly via `unittests/hammertest/run_headless.sh` under gcc and
  clang with `-Wall -Wextra -Werror`, on the `linux-headless-core` profile.

## Compatibility notes

`v1` is the strict-module contract. The `Vector`↔`Vec3` conversion used when
legacy `BoundBox` callers are routed through this owner is a **separate** named
policy owned by the Source/MFC adapter and specified with that migration step; it
is not part of this contract.
