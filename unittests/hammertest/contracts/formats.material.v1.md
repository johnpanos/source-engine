# Contract: `formats.material.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/material.h`
Impl: `hammer/core/formats/material.cpp`
Conformance: `unittests/hammertest/formats/test_material.cpp` (+ `_negative`)
Migration: `HAM-ASSET-001`

VMT material parsing plus a material catalog that resolves a VMF-authored
material name to its shader, `$basetexture`, and a decoded RGBA base image. This
is the clean-core replacement for the legacy Hammer path, which drove everything
through the engine `IMaterial` / `IMaterialSystem` globals.

## 1. Purpose, consumers

Enumerate `materials/**.vmt` in an asset source and resolve a material to its
base texture image. Consumers: the GTK material browser and the textured
viewport. Depends on the keyvalues codec, the VTF decoder, and the
`IAssetSource` port. Required for material browsing.

## 2. Accepted inputs

- `ParseMaterial( vmtText, out, includeOut, error )`: VMT keyvalues text. Reads
  the top block's shader name and `$basetexture` case-insensitively, searching
  nested blocks (so a patch `insert`/`replace` sub-block is covered). For a
  `patch` shader, `includeOut` receives the referenced material.
- `CanonicalizeMaterialName( name )`: lower-cases, converts back- to forward
  slashes, strips a leading slash, a `materials/` prefix, and a `.vmt`/`.vtf`
  extension.
- `MaterialCatalog( source )`, then `MaterialNames()`, `Material( name )`,
  `BaseTextureImage( name )` — `name` in any authored spelling.

## 3. Results, error taxonomy

- `ParseMaterial` returns false with a diagnostic on non-keyvalues text or an
  empty document. A material with no `$basetexture` parses with an empty base
  texture — never a fabricated one.
- `MaterialNames()` is sorted and de-duplicated.
- `Material()` follows one level of `patch` include (shader/base texture taken
  from the include when the patch does not override). Returns `nullptr` when the
  `.vmt` is absent or unparseable; the miss is cached.
- `BaseTextureImage()` decodes `materials/<baseTexture>.vtf`. Returns `nullptr`
  when the material, its base texture, or the decode is missing/invalid; the
  result (hit or miss) is cached and the pointer is stable for the catalog's life.

## 4. Ownership, threading

- The catalog borrows its `IAssetSource` (must outlive it) and owns its caches.
  Query methods are non-const (they populate caches); a single catalog is not
  intended for concurrent mutation. No globals, no engine material system.
