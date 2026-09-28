# Contract: `scene.map_queries.v1`

Module: `hammer.scene`
Header: `public/hammer/scene/map_queries.h` · Impl: `hammer/core/scene/map_queries.cpp`
Conformance: `unittests/hammertest/scene/test_map_queries.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Structural queries written once over `DocumentReader`, so they run on a
committed document and a staged edit: membership, expansion to leaves,
containers, bounds, the one visibility rule, name/class lookup and side lookup.
Required.

## 2. Accepted inputs

Any ids; unknown ids are skipped or yield nothing. Name patterns are
case-insensitive with an optional trailing '*'.

## 3. Results and guarantees

- A brush entity's solids belong to the entity, not to a group.
- `ExpandObjects` is deduplicated and in id order; `ExpandToLeaves` drops
  groups.
- `IsVisible` is false when the object or any container is quick-hidden or its
  visgroups are hidden.
- `ObjectBounds` uses +/-`pointHalfSize` for point entities and nothing for an
  originless entity.

## 4. Ownership, threading

Pure functions of a reader.

## 5. Invariants

Walks up containers are bounded, so a malformed cycle cannot loop.

## 6. Side effects and performance

Membership queries scan the document (O(n)); an index keyed by revision is a
later optimization if measurements need it.

## 7. Conformance suite and providers

`test_map_queries.cpp` (31 checks, gcc and clang), with negative checks for
unknown ids, empty patterns, originless entities and hidden containers.
