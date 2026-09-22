# Contract: `formats.prefab.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/prefab.h` · Impl: `hammer/core/formats/prefab.cpp`
Conformance: `unittests/hammertest/formats/test_prefab.cpp` (+ `_negative`)
Migration: `HAM-PREFAB-001`
Depends on: `hammer.formats` (keyvalues codec) and the shared VMF transform
`hammer/formats/vmf_transform` (the ONE owner of the QAngle placement math, also
used by `func_instance` — DRY).

Instantiates a prefab: a stored VMF fragment placed into a map at an origin/angles,
transforming its world solids and entities. Strict, MFC-free, GPU-free. Unlike a
`func_instance` (a live file reference expanded through a file store), a prefab is
instantiated inline from an already-parsed fragment.

## 1. Purpose, consumers, required vs optional

Own prefab placement. Consumers: the prefab library / "insert prefab" tool.
Required for prefab editing.

## 2. Accepted inputs

- `InstantiatePrefab( prefabDoc, Placement{origin, pitch, yaw, roll} )`: a parsed
  VMF fragment and a Source QAngle placement.

## 3. Results and guarantees

- Returns a `PrefabInstance{solids, entities}` — the fragment's world solids and
  entities transformed (rotate then translate) through `vmf_transform`
  (`TransformSolid`/`TransformEntity`), ready to merge into the map.
- Solid/entity counts are preserved. An identity placement (origin 0, angles 0)
  returns the fragment's objects unchanged in value.

## 4. Ownership, threading

- Pure function; no globals, no I/O (the fragment is already parsed).

## 5. Invariants

- **Placement applied**: a translated placement moves geometry (a no-op fails).
- **Identity preserved**: identity placement changes nothing (a spurious transform
  fails).
- **Count preservation**: no solid or entity is dropped or duplicated.
- **DRY transform**: uses the same `vmf_transform` owner as `func_instance`, so
  prefab and instance placement agree bit-for-bit.

## 6. Side effects and performance

- O(objects) copies; no I/O or globals.

## 7. Conformance suite and providers

- `test_prefab.cpp`: translate (solid + entity coordinates) and yaw-90 placement,
  with counts preserved.
- `test_prefab_negative.cpp`: pins placement-applied (translate moves),
  identity-preserved, and count preservation.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Instantiates world solids + entities with position; entity `angles` composition
  and texture-axis rotation are the shared `vmf_transform` limits (later
  increments). It does not manage a prefab library on disk or id-remap merged
  objects; those are later increments.
