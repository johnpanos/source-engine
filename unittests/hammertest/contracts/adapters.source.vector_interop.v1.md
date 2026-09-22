# Contract: `adapters.source.vector_interop.v1`

Module: `hammer.adapters.source` · Header: `hammer/adapters/source/vector_interop.h`
Conformance suite: `unittests/hammertest/adapters/test_vector_interop.cpp`
(runs on the MSVC/Wine parity lane, not on linux-headless-core)
Migration: `HAM-SOURCEADAPTER-001`

This is the RFC 0002 contract record for the boundary seam between the Source
engine value types and the strict, MFC-free Hammer geometry library. It is the
single named owner of the `Vector <-> Vec3` conversion policy (project DRY rule).
Both thin UI siblings -- the MFC shell (`hammer.adapters.mfc`) and the GTK shell
(`hammer.adapters.gtk`) -- route Source geometry through this seam rather than
open-coding component copies.

## 1. Purpose, consumers, required vs optional

Marshals `Vector` and legacy `BoundBox` to/from `hammer::geometry::Vec3` and
`hammer::geometry::AxisAlignedBox`. Consumers: any UI-sibling or Source-facing
caller that wants to run a reusable geometry operation on data that currently
lives in a Source/MFC-era object. All four conversions are **required**:
`ToVec3`, `ToVector`, `ToAABB`, `FromAABB`.

## 2. Accepted inputs, units, spaces, encoding, limits

- `Vector` and `Vec3` are three IEEE-754 `float` components in `x,y,z` order, in
  the same world space and units. No unit conversion, scaling, or axis
  remapping is performed or implied.
- `ToAABB( const BoundBox & )` reads `bmins`/`bmaxs` only; it does not read or
  require any other legacy state and does not depend on box validity.
- `FromAABB` writes `bmins`/`bmaxs` only.

## 3. Results, guarantees (success / failure / cancel)

- All conversions are total, synchronous, and infallible; there is no failure
  channel or cancellation.
- **Exactness:** for finite components, `ToVector( ToVec3( v ) )` reproduces `v`
  bit-for-bit, and `ToVec3( ToVector( u ) )` reproduces `u` bit-for-bit. The
  conversion introduces no precision loss because both sides are `float x,y,z`.
- `ToAABB(b).mins == ToVec3(b.bmins)` and `.maxs == ToVec3(b.bmaxs)`, componentwise.
- After `FromAABB(a, b)`, `b.bmins == ToVector(a.mins)` and `b.bmaxs == ToVector(a.maxs)`.

## 4. Ownership, borrow validity, threading, ordering, reentrancy

- Pure functions over values; no heap ownership, no globals, no internal state.
- `ToAABB` borrows its `BoundBox` argument by `const &` for the call only and does
  not retain it. `FromAABB` borrows its `BoundBox` out-parameter mutably for the
  call only. No lifetime extends beyond the call.
- Reentrant and thread-compatible: distinct arguments are independent; the
  functions add no synchronization requirement of their own.

## 5. State invariants and legal operation sequences

- The seam is stateless, so any order of calls is legal. It never mutates the
  strict library types except by constructing new values, and mutates a legacy
  `BoundBox` only through the explicit `FromAABB` commit.
- **Round-trip through the library preserves legacy semantics:** for any legacy
  `BoundBox b` and any reusable geometry operation `op`, running `op` on the legacy
  object produces the same `bmins`/`bmaxs` as
  `FromAABB( op_on_AABB( ToAABB( b ) ), b )`. This is the substitution guarantee
  that lets a UI sibling delegate geometry to the reusable library. It is proven
  by the shared parity oracle against the real MSVC-compiled legacy `BoundBox`.

## 6. Permitted side effects and performance

- No allocation, no logging, no I/O, no global reads/writes. Header-only inline
  conversions; cost is a few float copies per call.

## 7. Negative-provider requirement

The conformance suite includes a deliberately-broken adapter (e.g. one that swaps
`y`/`z` or drops a component on `ToVec3`) and asserts the round-trip / library
substitution checks **fail** against it, proving the oracle actually detects a
boundary-policy violation rather than trivially passing.
