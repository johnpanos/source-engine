# Contract: `formats.entity_property_sheet.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/entity_property_sheet.h` · Impl:
`hammer/core/formats/entity_property_sheet.cpp`
Conformance: `unittests/hammertest/formats/test_entity_property_sheet.cpp` (+ `_negative`)
Migration: `HAM-PROPSHEET-001`
Depends on / composes: `hammer.formats` FGD (`ResolveClass`) + the keyvalues codec.

Builds the property-grid row model for a selected entity: the FGD-driven data an
entity property sheet widget renders. This is the **headless model** behind the
"FGD-driven entity property editing" UI — the GTK/MFC widget only renders these
rows. Strict, MFC-free, GPU-free.

## 1. Purpose, consumers, required vs optional

Own the entity property model. Consumer: the entity property-sheet widget in the
shell. Required for entity editing UI.

## 2. Accepted inputs

- `BuildPropertySheet( entity, fgdClasses )`: one entity keyvalues block and the
  parsed FGD class table.

## 3. Results and guarantees

- Returns `EntityPropertySheet{classname, classInFgd, rows}`.
- **Schema rows first**: for the resolved FGD class (base classes first, in
  declaration order) one `PropertyRow` each with FGD `type`, `displayName`,
  `choices`, `inSchema=true`, the entity's value if set else the FGD `default`, and
  `isSet` reflecting whether the entity carries the key. Key matching is
  case-insensitive.
- **Extra rows next**: any entity key not in the schema (and not a meta key)
  becomes a row with `type="string"`, `inSchema=false`, `isSet=true`.
- **Meta excluded**: `classname` and `id` are never rows; the `editor` block (a
  child, not a pair) is never a row.
- `classInFgd=false` when the classname is unknown; then only extra rows appear.

## 4. Ownership, threading

- Pure function; no globals, no I/O.

## 5. Invariants

- **Defaults surfaced**: an unset schema key still appears, carrying its FGD
  default (a "show only set keys" view fails).
- **Meta hidden**: `classname`/`id` are not rows (a "show all pairs" view fails).
- **Schema order preserved**, inherited keys included via `ResolveClass`.

## 6. Side effects and performance

- One class resolve + one pass over the entity's pairs; O(properties + keys).

## 7. Conformance suite and providers

- `test_entity_property_sheet.cpp`: inherited `targetname` (set), unset `_light`
  showing its default, a `choices` row, an extra `customkey`, meta exclusion, and
  an unknown-class entity yielding only extra rows.
- `test_entity_property_sheet_negative.cpp`: pins unset-rows-show-defaults, meta
  exclusion, and no spurious rows.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Produces the row model. It does not apply edits back to the entity, validate
  values against types, resolve `input`/`output` (I/O) or spawnflag bit layout, or
  order rows into FGD "smartedit" groups. Those are later increments; writing edits
  back is the app/history layer's concern.
