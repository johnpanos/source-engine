# Contract: `ports.entity_catalog.v1`

Module: `hammer.ports`
Header: `public/hammer/ports/entity_catalog.h` · Helpers: `hammer/core/ports/entity_catalog.cpp`
Conformance: `unittests/hammertest/formats/entity_catalog_conformance.h` (shared suite),
run by `test_fgd_entity_catalog.cpp`; sensitivity: `test_entity_catalog_negative.cpp`
Migration: `HAM-FGD-001` (R08-DOMAIN)

The entity class schema port: what the editor knows about an entity class. The
application, viewport and presenters consume it; the composition root supplies a
provider (`formats.fgd_entity_catalog.v1`, or the test fake).

## 1. Purpose, consumers, scope

Answer, for a class name: its kind, its resolved keys (types, defaults, choices,
read-only flag), its inputs and outputs, and display hints (box, color, model,
sprite). Consumers: entity placement defaults and class changes (`hammer.app`),
the map check, the class palette and entity inspector (`hammer.presenters`),
entity markers (`hammer.viewport`). Required for entity editing.

## 2. Accepted inputs

- `Find( name )`: any class name, in any case; the empty string and unknown
  names are valid inputs (answer: `nullptr`).
- `ClassNames()`: no input.
- `EntityClassInfo::FindKey( key )`, `HasInput( name )`, `HasOutput( name )`:
  any name, in any case.
- `KeyTypeFromName( word )`: any FGD type word, in any case.

## 3. Results and guarantees

- **C1 listing.** `ClassNames()` lists every entity class exactly once, sorted
  case-insensitively (byte order of the lower-cased names), and returns the
  same list on every call.
- **C2 lookup.** `Find` is case-insensitive and returns the class of that name;
  every call for the same class returns the same pointer.
- **C3 base classes.** Base classes (FGD `@BaseClass`) are not entity classes:
  they are neither listed nor found. Unknown names are not found.
- **C4 resolved keys.** `keys` lists inherited keys first (bases in declared
  order, recursively), then the class's own; an own key with an inherited
  key's name (case-insensitive) replaces it in the inherited key's position.
  No two keys share a name. `kind` is `Point` for point classes, `Solid` for
  brush entities and `Other` for NPC, filter, keyframe and move classes.
- **C5 names.** `FindKey`, `HasInput` and `HasOutput` compare case-insensitively
  and reject unknown names. `inputs` and `outputs` are merged by name the same
  way as keys.
- Display hints are absent (`nullopt` / empty) when the schema declares none.
- `KeyTypeFromName` maps unknown words to `KeyType::Other`.

## 4. Ownership, threading

- The catalog owns its `EntityClassInfo` values; pointers stay valid for the
  catalog's lifetime. A catalog is immutable after construction and safe to
  share across threads (the test fake is mutable only while it is populated).

## 5. Invariants

- A name is listed iff `Find` returns non-null for it.
- The listing is strictly increasing case-insensitively (no duplicates).
- Resolved keys have unique names; an override keeps the base key's position.

## 6. Side effects and performance

- Lookups do no I/O, logging or allocation beyond the returned name list.
  `Find` is expected to be sub-linear (the FGD provider binary-searches;
  the fake uses a map).

## 7. Conformance suite and providers

- `entity_catalog_conformance.h`: `RunEntityCatalogConformance( checks, catalog,
  expectations )` checks C1–C5 against the expectations a fixture declares
  (class names, base names, unknown names, per-class kind, key order and I/O,
  overriding defaults). Its case folding is written independently of the
  providers.
- Providers run through it in `test_fgd_entity_catalog.cpp`: the FGD catalog
  built from a two-file fixture, and `hammertest::FakeEntityCatalog`
  (`unittests/hammertest/fakes/fake_entity_catalog.h`) populated with the same
  classes. An optional run covers the real Portal 2 FGD set
  (`HAMMER_FGD_CORPUS_DIR`).
- `test_entity_catalog_negative.cpp`: a good fake passes, and nine bad catalogs
  are each flagged: case-sensitive `Find`, byte-sorted names, a listed base
  class, a dropped class, unstable `Find` pointers, base-last key order, an
  appended (duplicate) override, a wrong kind and a missing output.
- All run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.
