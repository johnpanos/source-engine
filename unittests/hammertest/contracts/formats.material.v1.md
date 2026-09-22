# Contract: `formats.material.v1`

Module: `hammer.formats`
Header: `public/hammer/formats/material.h` · Impl: `hammer/core/formats/material.cpp`
Conformance: `unittests/hammertest/formats/test_material.cpp` (+ `_negative`)
Migration: `HAM-MATERIAL-001`
Depends on: `hammer.formats` (keyvalues codec)

Parses a VMT material *definition* — the shader name, its parameters, and whether
it carries proxies or is a patch — the material metadata the editor and renderer
consume. Strict, MFC-free, GPU-free. This is definition parsing, **not** VTF pixel
loading or rendering.

## 1. Purpose, consumers, required vs optional

Own material-definition parsing. Consumers: the texture browser / face material
picker (shader + base texture + surface prop) and the renderer's material lookup.
Required for material-aware editing.

## 2. Accepted inputs

- `ParseMaterial( vmtText )`: VMT source (keyvalues syntax). The first top-level
  block is the shader; its pairs are the parameters.

## 3. Results and guarantees

- On success a `Material` with `shader` (the block name), `parameters` (its
  pairs), and `hasProxies` (a `Proxies` sub-block present).
- `Param(name)`/`HasParam(name)` are **case-insensitive** top-level lookups (VMT
  parameter names ignore case). `IsPatch()` is true for the `patch` shader; a
  patch's `replace`/`insert` blocks are captured in `patchReplace`/`patchInsert`.
- `ResolvedParam(name)` resolves in the order **replace → insert → top-level**, so
  a patch's overridden value wins; for a non-patch material it equals `Param`.
  `BaseTexture()`/`SurfaceProp()` resolve through `ResolvedParam`, so a patched
  `$basetexture` is returned.
- Returns `nullopt` when there is no top-level shader block or keyvalues parsing
  fails.

## 4. Ownership, threading

- Pure function over the input string; no globals, no I/O (does not read the VTF).

## 5. Invariants

- **Case-insensitive params**: `Param("$BaseTexture") == Param("$basetexture")`.
- **Definition-only**: no texture data is loaded; a patch material reports
  `IsPatch()` and exposes `include` without resolving it.

## 6. Side effects and performance

- One keyvalues parse; O(text). No I/O or globals.

## 7. Conformance suite and providers

- `test_material.cpp`: a `LightmappedGeneric` material (shader, `$basetexture`,
  `$surfaceprop`, `$detail`, `Proxies`), case-insensitive lookup, and a `patch`
  material (`IsPatch`, `include`, no direct base texture).
- `test_material_negative.cpp`: proves a well-formed material parses while empty,
  whitespace-only, and unterminated-block inputs are rejected.
- Both run headlessly (gcc + clang, `-Wall -Wextra -Werror`) on
  `linux-headless-core`.

## 8. Declared scope limits (this slice)

- Parses the top-level shader + parameters, captures a patch's `replace`/`insert`
  blocks, and resolves patched parameters via `ResolvedParam`. It does not follow
  the patch `include` (load the base VMT it patches), evaluate material proxies,
  load VTF textures, or render. Those are later increments (include resolution +
  VTF decode). The material catalog (`hammer.formats.material_catalog`,
  `source-engine-87`) consumes this parser and owns `include` resolution + VTF.
