# Contract: `formats.material_catalog.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/material_catalog.h`
Impl: `hammer/core/formats/material_catalog.cpp`
Conformance: `unittests/hammertest/formats/test_material_catalog.cpp` (+ `_negative`)
Migration: `HAM-ASSET-001`
Depends on: `hammer.formats` (material.h VMT parser, vtf_image.h decoder, keyvalues
codec), `hammer.ports` (asset_source.h)

Composes the VMT parser, the VTF decoder, and an `IAssetSource` into an editor
material catalog: enumerate materials, and resolve a material name to its base
texture image. It owns no format knowledge of its own beyond name canonicalization
and one-level `patch` include following; VMT syntax stays in `material.h`, pixel
decoding in `vtf_image.h`, asset bytes behind the port.

## 1. Purpose, consumers

Answer "what materials exist?" and "give me this material's base texture as RGBA".
Consumers: the GTK material browser / object-bar preview and the textured
viewport. Required for material browsing and textured rendering.

## 2. Accepted inputs

- `CanonicalizeMaterialName( name )`: any authored spelling (mixed case, either
  slash, optional leading slash, optional `materials/` prefix, optional
  `.vmt`/`.vtf`).
- `MaterialCatalog( source )`, then `MaterialNames()`, `ResolveBaseTexture( name )`,
  `BaseTextureImage( name )` — `name` in any authored spelling.

## 3. Results and guarantees

- `MaterialNames()` lists every `materials/*.vmt`, canonical, sorted, de-duplicated.
- `ResolveBaseTexture()` returns the canonical `$basetexture` name, following one
  level of a `patch` shader's `include`; empty when the material is
  absent/unparseable or names no base texture. Cached (hit and miss).
- `BaseTextureImage()` decodes `materials/<baseTexture>.vtf`; returns `nullptr`
  when the material, its base texture, or the decode is missing/invalid. Cached;
  the returned pointer is stable for the catalog's lifetime.

## 4. Ownership, threading

- Borrows the `IAssetSource` (must outlive the catalog) and owns its caches. Query
  methods populate caches (non-const); one catalog is not for concurrent mutation.
  No globals, no engine material system.

## 5. Declared scope limits (this slice)

- A `patch` material's own `$basetexture` (including a `replace`/`insert`
  override) is taken from `Material::BaseTexture()`, which resolves intra-patch
  overrides via the parser's `ResolvedParam`. When the patch declares none, this
  catalog follows its `include` one level and reads the base material's
  `$basetexture`. Deeper include chains (>1 level) are not followed.
- Resolves only `$basetexture`; other material params (bumpmaps, proxies) are not
  interpreted. HDR/float VTF formats and cubemaps follow the `vtf_image.v1` limits.
