# Contract: `formats.fgd_entity_catalog.v1`

Module: `hammer.formats` (implements `ports.entity_catalog.v1`; uses `formats.fgd.v1`)
Header: `public/hammer/formats/fgd_entity_catalog.h` · Impl: `hammer/core/formats/fgd_entity_catalog.cpp`
Conformance: `unittests/hammertest/formats/test_fgd_entity_catalog.cpp`
(+ the shared port suite and its sensitivity suite `test_entity_catalog_negative.cpp`)
Migration: `HAM-FGD-001` (R08-DOMAIN)

The FGD-backed entity catalog: loads a game's FGD files and answers the entity
catalog port from them.

## 1. Purpose, consumers, scope

Turn an FGD entry file and its `@include` files into an immutable, resolved
class table behind `hammer::ports::IEntityCatalog`. Consumers: the composition
root, which hands it to the application as the port. Required for entity
editing with game schema; tests use the fake instead.

## 2. Accepted inputs

- `Load( entryName, loader )`: the entry file name and a loader
  `std::optional<std::string>( const std::string &name )` that returns a file's
  text (called with the entry name, then with names as written in `@include`).
- `FromTexts( { { name, text }, ... } )`: in-memory files, loaded in order as
  entries; `@include` names resolve among the list.
- FGD text as accepted by `formats.fgd.v1` (CRLF or LF).

## 3. Results and guarantees

- **Includes.** Each file's includes are loaded depth-first, before its own
  classes, and each file at most once (names compare case-insensitively with
  `/` and `\` equivalent), so include cycles and repeats terminate. `Files()`
  lists the files read, includes before their includer.
- **Overrides.** A later class with the name of an earlier one (any case)
  replaces it. Classes are resolved after all files are read, so a base class
  redefined later applies to every class using it.
- **Resolution.** Every non-base class is resolved once with `ResolveClass`
  (inherited keys first; own keys, inputs and outputs override by name in
  place; own helpers replace inherited helpers of the same name).
- **Translation.** Kind: `@PointClass` → `Point`, `@SolidClass` → `Solid`,
  `@NPCClass`/`@FilterClass`/`@KeyFrameClass`/`@MoveClass`/`@KeyValueClass` →
  `Other`. Keys: `type` from `KeyTypeFromName`, `typeName` the FGD word,
  display name, default, help, `readOnly` from the `readonly` modifier;
  choices with `defaultOn` for flags whose third field is non-zero.
  I/O: name, type, help.
- **Display hints** (first helper of each kind, after resolution):
  `size(x1 y1 z1, x2 y2 z2)` → `boxMins`/`boxMaxs` (component-wise min/max);
  `size(x y z)` → a box of that size centered on the origin; `color(r g b)`;
  `studio("path")` / `studioprop("path")` → `model`, or, without a path, the
  `model` key's default; `iconsprite("path")` → `sprite`. A malformed size or
  color gives no hint.
- **Errors** (`FgdCatalogError{ message, file, line }`), and no catalog:
  unreadable entry file (`file` = entry, `line` 0); unreadable include (`file`
  = the includer, `line` = the `@include` line); a parse error (that file and
  its line); an unknown base class (the using class's file and directive line).
- Everything in `ports.entity_catalog.v1`.

## 4. Ownership, threading

- Owns its resolved classes; borrows the loader only during `Load`. Immutable
  afterwards, safe to share across threads. Move-only; a move keeps `Find`'s
  pointers valid (the class vector's buffer moves with it). Nothrow move, as
  `foundation::Expected` requires.

## 5. Invariants

- The table holds no base class; names are unique case-insensitively and sorted.
- A load either returns a complete catalog or an error; no partial catalog.

## 6. Side effects and performance

- I/O only through the loader, once per file. One parse per file and one
  resolution per class at load time; `Find` is a binary search. No logging or
  globals. The real Portal 2 set (4 files, 421 classes) loaded in 0.07 s in an
  unoptimized (-O0) g++ build on the development host (one run, 2026-09-28).

## 7. Conformance suite and providers

- `test_fgd_entity_catalog.cpp`: the shared port suite on the FGD catalog and on
  the fake populated with the same classes, and a field-by-field agreement
  check between them; hints (inherited size and color, centered single-point
  size, studio path and `studio()` fallback, icon sprite), flag defaults,
  read-only override, all four `Other` kinds, a later-file override, move
  stability; includes (cycle, repeat, case/slash spelling, `FromTexts`
  order); every error kind with its file and line, including a CRLF file shaped
  like Portal 2's header whose first include is missing (line 7).
- Optional: with `HAMMER_FGD_CORPUS_DIR` set to a directory holding
  `portal2.fgd` and its includes, the real set loads and passes the port suite
  (1,945 checks on the Steam Portal 2 `bin/`). The repository copy
  `external/portal2_steam2_scripts/841_1/bin/portal2.fgd` lacks its includes and
  fails with `cannot read included FGD 'base.fgd'` at `portal2.fgd:7`.
- `test_entity_catalog_negative.cpp` shows the port suite flags bad catalogs.

## 8. Declared scope limits

- `iconsprite()` without a path and `sprite()` give no sprite hint; other
  helpers (`sphere`, `line`, `cylinder`, `lightcone`, ...) are kept in the FGD
  parse but not translated into hints.
- `@include` is honored at the head of the file regardless of its position.
- `@MaterialExclusion`, `@AutoVisGroup` and `@mapsize` are not exposed.
