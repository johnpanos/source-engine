# Contract: `app.update_hint.v1`

Module: `hammer.app` · Type: `hammer::app::UpdateHint`
Header: `public/hammer/app/update_hint.h` · Impl: `hammer/core/app/update_hint.cpp`
Conformance suite: `unittests/hammertest/app/test_update_hint.cpp`
Migration: `HAM-UPDATEHINT-001`

Behavioral specification for the reusable core of the legacy MFC `CUpdateHint`:
the aggregate collected while mutating a document that records which nodes changed,
under which notify code, and the world-space region affected, for the view-update
step. Any substitute implementation must satisfy every clause here and pass the
shared suite. This model composes two already-extracted strict libraries
(`hammer::geometry::AxisAlignedBox`, `hammer::scene::NodeHandle`) and has no MFC,
tier0, PCH, or GPU dependency.

## 1. Purpose, consumers, required vs optional

Consumers: the document-mutation path (which reports changes) and the view-update
path (which reads the aggregate). All capabilities are **required**: bucketing by
notify code, the bounded number of codes, the affected-region union, and reset.

## 2. Accepted inputs, units, spaces, encoding, limits

- Nodes are `hammer::scene::NodeHandle` values; the model stores them opaquely and
  never resolves them. Duplicate handles are permitted (the caller's concern).
- Notify codes are arbitrary `int`s. At most `kMaxNotifyCodes` (= 16, preserved
  from legacy `MAX_NOTIFY_CODES`) **distinct** codes may be held at once.
- Regions are `hammer::geometry::AxisAlignedBox` in the document's world units. No
  unit conversion is performed.

## 3. Results, error taxonomy, guarantees (success / failure / cancel)

- `PreUpdate( bounds )`: unions `bounds` into the affected region; records no node
  or code. Total, infallible.
- `PostUpdate( node, code, bounds )`: appends `node` to the bucket for `code`
  (creating the bucket if `code` is new and capacity remains), unions `bounds`,
  and returns `true`. Returns `false` and **records nothing** (no node, no bucket,
  no region change) when `code` is new and all `kMaxNotifyCodes` buckets are in
  use. An **existing** code always succeeds, even at capacity.
- `UpdateRegion()`: the union of every region passed since the last `Reset`;
  invalid (`IsValidBox() == false`) when nothing has been reported.
- `NotifyCodeCount()` / `NotifyCodeAt(i)` / `NodesAt(i)`: read the buckets in the
  order their codes were first seen; `NodesAt(i)` preserves report order.

## 4. Ownership, borrow validity, threading, ordering, reentrancy

- Value type; owns its buckets and region by value. No heap ownership of external
  objects, no globals, no borrowing beyond each call's arguments.
- Not internally synchronized; distinct instances are independent. Mutators
  observe prior mutations on the same instance.

## 5. State invariants and legal operation sequences

- A default-constructed or `Reset` hint has zero codes and an invalid region.
- The number of buckets is monotonic between resets and never exceeds
  `kMaxNotifyCodes`.
- The affected region only grows (it is a union); no operation shrinks it except
  `Reset`.
- Rejecting an over-capacity new code leaves every observable value unchanged.

## 6. Permitted side effects and performance

- No allocation beyond the buckets/node vectors, no logging, no I/O, no globals.

## 7. Relationship to legacy and evidence classification

The legacy `hammer/updatehint.cpp` is **orphaned dead code**: it is in no `.vpc`
build project and does not compile (unbalanced parentheses, references to
non-members `m_Objects`/`nIndex`). There is therefore no live legacy behavior to
byte-match; this contract is a faithful reconstruction of the **intended** model
described by `hammer/updatehint.h` (the `NotifyList_t[MAX_NOTIFY_CODES]` buckets,
`m_nListEntries`, and `m_UpdateRegion`), rebuilt on the strict libraries. This is
recorded as `evidenceKind: decision` in the migration ledger, not `observed`.

## 8. Negative-provider requirement

The sensitivity suite (`test_update_hint_negative.cpp`) runs an `UnboundedUpdateHint`
that ignores the capacity limit; the oracle asserts the real model enforces the cap
and the broken one is caught, proving the clause is actually pinned.
