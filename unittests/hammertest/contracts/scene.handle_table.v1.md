# Contract: `scene.handle_table.v1`

Module: `hammer.scene` · Types: `hammer::scene::NodeHandle`, `hammer::scene::HandleTable`
Headers: `public/hammer/scene/handle.h`, `public/hammer/scene/handle_table.h`
Impl: `hammer/core/scene/handle_table.cpp`
Conformance: `unittests/hammertest/scene/test_handle_table.cpp` (+ `_negative`)
Migration: `HAM-SCENE-001`

## 1. Purpose, consumers, required vs optional

Issue and validate stable runtime references to scene nodes without exposing raw
pointers. Consumers: the scene graph, selection/history, and any deferred result
that must re-check whether the node it referred to still exists. All clauses are
**required**.

A runtime handle is a distinct concept from a persistent VMF id, from history
identity, and from a GPU pick id (RFC 0002 state-ownership). This contract covers
only the runtime handle.

## 2. Accepted inputs

- `HandleTable( documentId )`: `documentId` is a non-zero document identity from
  the session's document-identity owner. `documentId == 0` yields a table that
  issues only the invalid handle (a misconfiguration guard, not a usage mode).
- `Free`/`IsValid` accept any `NodeHandle` value, including foreign, stale, or
  default-constructed handles; these are rejected, not undefined.

## 3. Results and guarantees

- `Allocate()` returns a handle for which `IsValid` is true until it is freed,
  with `document == documentId`, a valid slot `index`, and the slot's current
  `generation`. From a zero-id table it returns `kInvalidNodeHandle`.
- `Free(h)` returns true and invalidates `h` (and every copy of `h`) iff `h` was
  valid; otherwise returns false and changes nothing (double-free safe).
- `IsValid(h)` is true **iff** `h.document == documentId` (nonzero), `h.index`
  addresses a live slot, and `h.generation` equals that slot's current generation.

## 4. Ownership, threading, ordering

- `HandleTable` owns its slot storage. `NodeHandle` is a plain value that borrows
  nothing. The table is not internally synchronized; distinct tables are fully
  independent. No global state.

## 5. Invariants and legal sequences

- Freeing a slot strictly increments its generation (skipping 0), so a reused slot
  never validates a handle issued before the free (**stale-reference rejection**).
- A handle issued by one table never validates in another, even when both reuse
  the same slot index (**independent documents / reused-address guard**).
- `LiveCount()` equals allocations minus frees of valid handles.

## 6. Side effects and performance

- No I/O, logging, or global mutation. `Allocate`/`Free`/`IsValid` are O(1)
  amortized; storage grows by slot reuse via a free list.

## 7. Conformance suite and providers

- `test_handle_table.cpp`: allocate/free/resolve, slot-reuse generation
  rejection, independent-document isolation, invalid-handle handling.
- `test_handle_table_negative.cpp`: a generation-ignoring provider must be
  **detected** by the stale-rejection predicate — proving the oracle is not
  vacuous.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) via
  `unittests/hammertest/run_headless.sh` on `linux-headless-core`.
