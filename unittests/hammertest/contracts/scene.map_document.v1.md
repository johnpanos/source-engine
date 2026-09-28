# Contract: `scene.map_document.v1`

Module: `hammer.scene`
Headers: `public/hammer/scene/map_objects.h`, `map_document.h` ·
Impl: `hammer/core/scene/map_objects.cpp`, `map_document.cpp`
Conformance: `unittests/hammertest/scene/test_map_document.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

The one authoritative content model of an open map: solids with sides (authored
plane points, texture axes, verbatim displacement data), entities (ordered keys,
parsed connections), groups, and document settings (world keys, visgroups,
cordons, cameras, unknown blocks). Consumers: every Hammer domain module,
codecs, viewport extraction. Required.

## 2. Accepted inputs

Objects are values. `Put` accepts an object whose id carries the document's
serial and is not already used by another kind. Connection text uses ',' or
0x1B separators with exactly five fields.

## 3. Results and guarantees

- `Side::Plane()` uses the legacy winding `(p0 - p1) x (p2 - p1)`.
- Entity key accessors read and write the one ordered key list; typed accessors
  reject malformed values.
- `AllocateId` never reissues an id, including ids restored by `Put`.
  `AllocateVmfId` stays above every persistent id the document has seen.
- `Put` refuses foreign serials, the invalid id and cross-kind id reuse.
- `Validate()` lists every violated invariant (below); empty means consistent.
- `SameContent` compares objects and settings, not counters.

## 4. Ownership, threading

The document owns its objects by value. Not internally synchronized: one
sequence (the session's) mutates it.

## 5. Invariants

Solid owners are entities; group references are groups; groups are acyclic;
side VMF ids are unique; solids have at least four sides.

## 6. Side effects and performance

None. Lookup is O(log n) by id.

## 7. Conformance suite and providers

`test_map_document.cpp` (46 checks, gcc and clang), with negative checks for
foreign ids, cross-kind ids, malformed keys and connections and each seeded
invariant violation.
