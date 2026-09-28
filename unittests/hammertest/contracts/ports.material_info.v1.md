# Contract: `ports.material_info.v1`

Module: `hammer.ports` (provider: `hammer.formats` `MaterialInfoAdapter`)
Header: `public/hammer/ports/material_info.h` · Adapter: `public/hammer/formats/material_info_adapter.h`,
`hammer/core/formats/material_info_adapter.cpp`
Conformance: `unittests/hammertest/formats/test_material_info.cpp`
Migration: `HAM-ASSET-001` (R08-DOMAIN)

Material metadata for editing: which materials exist and their mapping size.
Decoded images and GPU bindings are separate concerns (`formats.material_catalog.v1`).

## 1. Purpose, consumers, scope

Texture alignment (fit and justify work in texels), the material browser (the
list of names) and the map check (does a name exist). Required for texture
tools; the composition root supplies the adapter, tests use
`hammertest::FakeMaterialInfo` (`unittests/hammertest/fakes/fake_material_info.h`).

## 2. Accepted inputs

- `Exists( name )`, `Size( name )`: any material name as authored in a map, in
  any case, with `/` or `\`. The adapter also accepts a leading `/` or
  `materials/` and a `.vmt` suffix (`CanonicalizeMaterialName`).
- `Names()`: no input.

## 3. Results and guarantees

- Names compare case-insensitively with `/` and `\` equivalent.
- `Size` is the mapping size in texels. The adapter reads it from the VTF
  header of the material's `$basetexture` (following one level of a `patch`
  include), without decoding pixels.
- A missing material is reported as missing (`Exists` false, `Size` nothing),
  never as a default size. An existing material may have no size: no base
  texture, a missing or unreadable VTF, or a KTX2-only base texture (declared
  limit: no header-level KTX2 reader exists in the strict core).
- `Names()` lists every known material, normalized and sorted; every listed
  name exists. For the adapter this is `MaterialCatalog::MaterialNames()`
  (`materials/**.vmt`); textures without a VMT are not materials.

## 4. Ownership, threading

- The adapter borrows a `MaterialCatalog` and the `IAssetSource` it reads; both
  must outlive it. It caches sizes per material, and the catalog caches on first
  use, so neither is internally synchronized: use one from one thread.
- The fake owns a name→size map.

## 5. Invariants

- `Exists( n )` iff `n` (normalized) is in `Names()`.
- `Size( n )` has a value only if `Exists( n )`.
- A cached answer does not change (the catalog's listing is also a snapshot).

## 6. Side effects and performance

- Reads through the asset source only: the VMT listing once, and one VMT and one
  VTF read per material on its first `Size`. No logging or globals.

## 7. Conformance suite and providers

- `test_material_info.cpp`: shared port clauses run against the adapter (over an
  in-memory asset source with VTFs from the independent fake VTF writer) and
  against the fake; adapter clauses: patch-include size, prefix and suffix
  spelling, absent base texture, unreadable VTF and KTX2-only texture have no
  size, orphan textures are not materials, `Names` equals the catalog listing,
  sizes are cached. Sensitivity: a provider that reports a default size for
  every name is flagged by the shared clauses.
- Runs headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.
