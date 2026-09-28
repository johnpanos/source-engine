# Contract: `scene.document_index.v1`

Module: `hammer.scene`
Header: `public/hammer/scene/document_index.h` · Impl: `hammer/core/scene/document_index.cpp`
Conformance: `unittests/hammertest/scene/test_document_index.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

A derived snapshot index for whole-map passes (the map check, panels) that
would otherwise ask a scanning query of every object. Required for passes over
real maps (thousands of objects).

## 2. Accepted inputs

Any `DocumentReader` (a committed document or a staged edit).

## 3. Results and guarantees

- `EntitySolids`, `GroupMembers` and `EntitiesNamed` return exactly what the
  scanning queries in `map_queries.h` return, in id order.
- `WithVmfId` lists every object of a kind with a persistent id;
  `SolidsWithSide` every solid using a side id, once per side.

## 4. Ownership, threading

Owns its maps; immutable after construction. Valid until the reader changes;
callers that keep one across edits key it by the session revision.

## 5. Invariants

Lists are in id order.

## 6. Side effects and performance

Built in O(n log n); lookups are O(log n) (prefix patterns O(log n + k)).

## 7. Conformance suite and providers

`test_document_index.cpp` compares every answer with the scan on a generated
document and shows a stale snapshot differs after an edit while a rebuilt one
agrees.
