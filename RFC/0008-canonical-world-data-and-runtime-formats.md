# RFC 0008: Canonical World Data and Runtime Formats

- Status: Accepted for planning (2026-09-22); F1 prototype active ([progress](0008-progress.md))
- Date: 2026-09-22
- Scope: The compiled-world interchange stage, the runtime map container, world
  render data (mesh, lightmaps, probes, reflection probes), the texture container,
  PBR material parameter semantics, the native Vulkan world path that consumes
  them, and incremental map builds
- Related: [RFC 0007: Physically Based Lighting Pipeline](0007-physically-based-lighting-pipeline.md)
  (produces the lighting data defined here),
  [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
  (render seam, native Vulkan, platform profiles, tool/process cleanup),
  [RFC 0002: Hammer Responsibility Factorization](0002-hammer-responsibility-factorization.md)
  (authoring document and compile workflow),
  [RFC 0003: Dependency-Aware Job System](0003-dependency-aware-job-system.md)
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Evaluated dependencies (source inspected, not built or pinned): OpenUSD
  `fc590b38e24770408e31b56442d31b8029729ee7` (Tomorrow Open Source Technology
  License 1.0); KTX-Software `90967979cbb7e9401ee2401ff997f30b4b7507d6`
  (Apache-2.0 for project files; bundled components carry their own compatible
  licenses under `LICENSES/`); Cycles as recorded in RFC 0007.

## Summary

The user directed that the renderer, BSP, and intermediate formats may be
upgraded to the best-supported data structures, so that the world is not
converted again every time a tool or the runtime touches it. This RFC records
that decision as the versioned content-compatibility change that AGENTS.md
requires, and defines its boundaries.

The world has exactly **four representations**, each with one owner. Every
transformation between them happens once, is keyed by content hash, and is
cached:

1. **Authoring:** VMF plus source assets (RFC 0002 owns the document). Brush and
   CSG semantics have no open-standard equivalent, so VMF stays.
2. **World Stage:** an OpenUSD stage emitted once by the geometry compiler. It is
   the single compiled-world description read by the light baker (RFC 0007),
   reference rendering, DCC tools, and the runtime packer. Lighting is a separate
   USD layer, so rebaking never rewrites geometry.
3. **Runtime package (BSP2):** a new map container with 64-bit offsets, a 4CC
   lump directory, per-lump versioning, compression and content hashes, and
   GPU-ready alignment. It carries the legacy gameplay lumps with unchanged
   semantics, plus new render lumps: a pre-batched world mesh, SH L1 lightmap
   atlases, an SH L2 probe volume, and prefiltered reflection probes. Textures
   use **KTX2**.
4. **GPU-resident:** what the native Vulkan world path uploads. Render lumps are
   laid out so that loading is a copy, not a rebuild.

Legacy maps (BSP v19–21, VTF, VMT) keep loading unchanged forever. BSP2 packages
may also carry derived legacy lighting lumps, so that the D3D9/DXVK provider and
legacy shaders can still render new maps. That derivation lives in one exporter
and is the only place new data is converted to legacy data.

## Motivation: where the world is converted today

Observed in the current tree:

| Step | Where | What happens every time |
| --- | --- | --- |
| VMF → BSP faces | `utils/vbsp` | Brushes are split into polygon faces with per-face lightmap axes |
| BSP → vrad's own structures | `utils/vrad` | vrad rebuilds patches, samples and its own trace structures from the lumps |
| Lighting → `ColorRGBExp32` | vrad | Linear light is quantized to a shared-exponent 8-bit format with 4 RNM samples per luxel |
| Load → CPU lightmaps | `engine/gl_lightmap.cpp` (`R_BuildLightMap`, `R_RedownloadAllLightmaps`) | Each surface's lightmap is decoded, styles and dynamic lights are composited **on the CPU**, then uploaded into lightmap pages allocated at load |
| Load → world vertex buffers | `engine/matsys_interface.cpp` (`WorldStaticMeshCreate`) | World static meshes are rebuilt from faces at every map load |
| Cubemaps | `engine/buildcubemaps.cpp` | Rendered by the running game, written as VTF into the map; not roughness-prefiltered |
| Textures | VTF | No ASTC/ETC2 (so no native mobile formats); no BC6H/BC7; BC5 encoder disabled on Linux (RFC 0007 findings) |
| Container | `public/bspfile.h` | 32-bit `fileofs`/`filelen`, fixed `HEADER_LUMPS = 64` (most IDs used), no per-lump integrity |
| Interchange | none | Every tool has its own BSP reader; external tools need custom importers |

Each consumer reinterprets the same world in its own structures. Adding the
Cycles baker, the PBR renderer, and USD export on top would add three more
conversions. This RFC removes the repeated work instead.

## Goals

- One authoritative representation per tier, one owner per transformation, and
  a content-addressed cache so unchanged inputs are never recomputed.
- Open, widely supported formats wherever an equivalent exists: OpenUSD (scene),
  KTX2 (textures), UsdLux (lights), UsdPreviewSurface / glTF metallic-roughness /
  OpenPBR subset (materials).
- Runtime data that the native Vulkan provider uploads without rebuilding it,
  on desktop and mobile profiles.
- Unchanged gameplay semantics: collision, visibility, areaportals, entities,
  physics, and the dedicated server see the same data as today.
- Legacy content stays loadable, and legacy renderers can render new maps
  through a declared, derived compatibility payload.

## Non-goals

- Replacing VMF as the authoring document (RFC 0002 owns it), or storing brushes
  in USD.
- Changing the encoding or meaning of gameplay lumps (planes, nodes, leafs,
  brushes, areaportals, visibility, physics collision, entities, static-prop
  placement). They are carried into BSP2 byte-for-byte.
- Replacing the MDL/VVD/VTX model runtime format. Model modernization needs its
  own RFC because animation, gameplay, and networking depend on `studio.h`.
  This RFC only reads model geometry into the World Stage for baking and export.
- Putting OpenUSD in engine products or on mobile. USD is a desktop tool and
  compile dependency only.
- Real-time GI, virtual geometry, or other rendering research beyond what the
  new data requires.

## The four tiers

```
AUTHORING                 WORLD STAGE (tools)                 RUNTIME PACKAGE            GPU
VMF + models + VMT  ──►  geometry.usdc  (vbsp2)  ──┐
+ texture masters         lighting.usdc (baker)  ──┼──► packer ──► map.bsp2 ──► native Vulkan
                          probes.usdc   (baker)  ──┘     │          + KTX2       world path
                                │                        │
                                ▼                        └──► derived legacy lumps (optional)
                       DCC tools / Cycles reference                 ──► D3D9/DXVK, legacy shaders
```

### Conversion ledger

Every arrow above is one row. No other code performs these transformations.

| Transformation | Sole owner | Cache key |
| --- | --- | --- |
| VMF → geometry layer + gameplay lumps | `vbsp2` (vbsp ported and split, RFC 0007 Phase B) | VMF semantic hash, referenced asset hashes, tool version |
| Portals → visibility | `vvis` | Portal lump hash, tool version |
| Geometry layer → lighting/probe layers | `ILightBaker` providers (RFC 0007) | Geometry layer hash, light prims hash, material albedo/emission hashes, bake settings, provider version |
| Texture master → KTX2 UASTC (LDR/HDR) | `TextureEncoder` | Source image hash, encode settings, encoder version |
| KTX2 UASTC → profile format (BC*/ASTC/ETC2) | `TextureTranscoder` in the packer | KTX2 hash, profile, transcoder version |
| Stage layers → BSP2 render lumps | `WorldPacker` | Layer hashes, profile, packer version |
| Canonical lighting → legacy lumps | `LegacyLightingExporter` | Lighting layer hash, exporter version |
| BSP2 lumps → GPU resources | native Vulkan world path | None (direct upload) |

Build steps are nodes in a dependency graph that uses the RFC 0003 job system
inside tools. Changing only a light re-runs the bake and pack steps. Changing
only a material parameter re-runs pack (and bake if albedo or emission changed).
Changing only a texture re-runs encode, transcode, and pack. Geometry changes
re-run the whole graph.

## World Stage (OpenUSD)

### Why USD

It is the most widely supported open scene description for production tools.
Cycles standalone reads it directly (RFC 0007 findings). It has standard light
(`UsdLux`: `SphereLight`, `DiskLight`, `RectLight`, `DistantLight`, `DomeLight`,
`ShapingAPI` for spot cones) and material (`UsdShade`, `UsdPreviewSurface`,
MaterialX) schemas. Its layer composition lets the baker add lighting as a
sublayer without rewriting geometry. The inspected revision includes the
`usdchecker` validation tool (`pxr/usdValidation/bin/usdchecker`).

### Stage structure

```
/World                               (Xform; metersPerUnit = 0.0254, upAxis = Z)
  /Geometry
    /WorldSpawn/Mesh_<materialBatch> (Mesh; triangulated faces and displacements)
    /BrushEntities/<entity>/Mesh_... (func_* models, with entity reference)
  /Props/<id>                        (instanceable references to model geometry)
  /Lights/<entity>                   (UsdLux prims + source: attributes)
  /Materials/<vmt path>              (UsdShade Material + source: attributes)
  /Probes
    /ReflectionProbes/<id>           (position, influence box, parallax box)
    /ProbeVolume                     (probe positions, leaf association)
  /Entities/<id>                     (non-geometric entities as typeless prims + key-values)
```

- Geometry meshes carry primvars: `st` (material UV), `lightmap:st` (atlas UV
  from the lightmap chart packer), `source:faceId` (maps each triangle back to
  the BSP face for decals, overlays, and legacy export), `source:smoothingGroup`.
- The **lightmap chart layout is authored in the geometry layer**, so bakers,
  runtime, and exporters share one layout. It replaces per-face
  `lightmapVecsLuxelsPerWorldUnits` as the canonical source. The legacy face
  layout is derived from it by the legacy exporter when requested.
- Every Source-specific datum without a standard schema is kept under the
  `source:` namespace, so a round trip through DCC tools preserves it.
- The lighting layer (`lighting.usdc`) references the lightmap and probe
  payloads as KTX2 asset paths and records provider, mode, policy version, and
  sample statistics as metadata.

### USD decision

Decided by the user on 2026-09-22: the World Stage uses OpenUSD directly in the
compile path, not an in-house format with a USD exporter. Consequences:

- `vbsp2`, the bakers, the packer, the reference renderer, and Hammer's export
  and preview link the OpenUSD C++ API (`pxr/usd`, `usdGeom`, `usdLux`,
  `usdShade`) on desktop tool profiles. Engine products, the dedicated server,
  and mobile builds never link it; R12/R53 link evidence checks this.
- A single OpenUSD revision is pinned per tool profile with its license record
  (TOST 1.0). It must be 25.11 or newer to match the Cycles Hydra delegate's
  stated requirement, so one USD build serves both.
- The stage schema is a generated USD schema. Readers use the schema API, not
  string attribute lookups.
- `usdchecker` runs in the validation lane for every emitted stage.
- `.usdc` in build caches and `.usda` for reviewed fixtures, with
  deterministic prim ordering so fixture diffs are reviewable.

A typed schema (`SourceWorldAPI` and friends) is generated with
`usdGenSchema` so attribute names and types have one definition. Fixtures use
`.usda`; build caches use `.usdc`.

## Runtime package: BSP2

### Container

| Field | Content |
| --- | --- |
| Header | Magic (new, distinct from `VBSP`; see open decisions), container version, flags, map revision, lump count, 64-bit directory offset |
| Directory entry | 4CC id, lump version, flags (compression: none/zstd/lzma), alignment, 64-bit offset, 64-bit stored size, 64-bit uncompressed size, 128-bit content hash |
| Payload | Lumps aligned (at least 16 bytes, 4096 for bulk GPU data) so uncompressed lumps can be memory-mapped and uploaded directly |

- The lump count is unbounded, and unknown 4CCs are skipped. Adding data is a
  new lump, not a new container version.
- Legacy lumps are carried with their legacy structures and versions under
  reserved 4CCs (for example `L000`–`L063`, keeping the legacy index). Gameplay
  code reads them through the same accessor it uses today. The F1 prototype
  also carries the verbatim legacy header (`LHDR`) and any nonzero gap bytes
  (`LGAP`), so legacy → BSP2 → legacy is byte-identical. Absolute offsets inside
  legacy lumps (game lump dictionaries) are rebased with the lump's legacy
  origin.
- Lump override files (`public/lumpfiles.cpp`) keep working, keyed by 4CC.
- The embedded zip pak lump remains for legacy overrides and loose assets. New
  binary assets (KTX2) go into an **asset table lump**: aligned, hashed, and
  addressable without zip decompression.

### Engine access seam

`CMapLoadHelper` (`engine/modelloader.cpp`) is already the single point through
which the engine reads lumps. It becomes a container-neutral reader over a
`IMapContainer` interface with two implementations: legacy BSP (v19–21) and
BSP2. Gameplay, collision (`engine/cmodel*.cpp`), and the dedicated server keep
calling it by lump identity. The dedicated server reads only gameplay lumps and
must not link render or texture dependencies (checked by R12's link evidence).

### New render lumps

| 4CC (proposed) | Content | Replaces at runtime (for BSP2 maps on the new path) |
| --- | --- | --- |
| `WMSH` | World render mesh: vertex/index buffers in the GPU layout (position, octahedral normal/tangent, material UV, lightmap UV), meshlet clusters with bounds and normal cones, draw batches by material, per-leaf cluster ranges (PVS culling), triangle → face map | `WorldStaticMeshCreate` rebuild at load |
| `LMAP` | Lightmap atlas pages: SH L1 irradiance (L0 HDR RGB + L1 directional), one layer per light style; KTX2 assets in the asset table | `ColorRGBExp32` lightmaps, CPU `R_BuildLightMap` compositing, lightmap page allocation at load |
| `LSTY` | Light style table: style id → atlas layer, per-chart style masks | Per-surface `styles[MAXLIGHTMAPS]` compositing on the CPU |
| `PRBV` | Probe volume: probe positions, SH L2 irradiance per probe (per style), leaf → probe tetrahedra/cell index, validity masks | Leaf ambient cubes |
| `RPRB` | Reflection probes: position, influence and parallax boxes, blend priority, KTX2 prefiltered HDR cube (GGX roughness mips) | `env_cubemap` VTFs from `buildcubemaps` |
| `MTBL` | Material table: VMT paths, family, shader capability requirement, hashes | `texdata` string table lookups for render batching |
| `PKMF` | Package manifest: profile, formats chosen, source stage hashes, tool versions, derived-legacy flag | — |

Light styles stay a gameplay feature (switchable lights). On the new path,
styles become atlas layers blended in the shader by the style's current scalar,
instead of re-compositing lightmaps on the CPU when a style changes.

### Legacy compatibility payload

When a package profile requests it, `LegacyLightingExporter` derives and writes
the legacy lumps (`LUMP_LIGHTING[_HDR]`, face lightmap offsets and styles,
`LUMP_LEAF_AMBIENT_*`, worldlights) and legacy cubemap VTFs:

- RNM basis irradiance from the baker's direct RNM output (RFC 0007 bakes it
  exactly), not re-derived from SH;
- ambient cubes by evaluating SH L2 irradiance along the six axes;
- the legacy per-face luxel layout by resampling the atlas along each face's
  derived lightmap vectors.

A map that carries this payload renders on the D3D9/DXVK provider and on legacy
shader families. A map without it declares that it requires the
`world.bsp2.v1` render capability.

A separate "legacy BSP v21 export" profile writes a classic `VBSP` v21 file
from the same data for old engines and legacy Hammer. It is a derived artifact,
never an input.

## Textures: KTX2

### Decision

KTX2 is the texture container for new content. Masters are encoded once to
**UASTC** (LDR, or UASTC HDR for HDR data). The packer then transcodes each
master to the profile's GPU format. KTX-Software at the inspected revision
provides:

- `ktx encode` with codecs `basis-lz`, `uastc`, `uastc-ldr-4x4`,
  `uastc-hdr-4x4`, `uastc-hdr-6x6i`;
- `ktx transcode` from UASTC LDR to `bc1`, `bc3`, `bc4`, `bc5`, `bc7`, `astc`,
  `etc-rgb/rgba`, `eac-r11/rg11`, and from UASTC HDR to `bc6hu`, `astc-hdr-4x4/6x6`,
  `rgb16f`, `rgba16f`, `rgb9e5`;
- `ktx validate` for container validation.

One encode serves every platform, so no per-platform re-encoding of masters is
needed.

### Per-profile formats

Declared in each platform profile (RFC 0001 / AGENTS.md profiles). Each profile
queries device support and fails composition if a required format is missing.

| Slot | Desktop (Linux/macOS) | iOS / Android | Fallback |
| --- | --- | --- | --- |
| Base color (sRGB) | BC7 sRGB | ASTC 4×4 sRGB | ETC2 sRGB / RGBA8 |
| Normal | BC5 | ASTC 4×4 (two-channel) / EAC RG11 | RG8 |
| Mask (metal/rough/AO) | BC7 / BC4 per channel | ASTC 4×4 / EAC R11 | RGBA8 |
| HDR (lightmap L0, reflection probes) | BC6H | ASTC HDR where supported | RGB9E5 / RGBA16F |
| Directional (lightmap L1) | BC7 / BC5 | ASTC 4×4 | RGBA8 |

The exact device capabilities per profile (for example BC availability through
MoltenVK on Apple GPUs, or ASTC HDR on a given Android device class) are
recorded from device queries on R29 runners, not assumed here.

### Integration

- The material system gets a container-neutral texture reader: VTF (existing)
  and KTX2 (new) both produce the same in-memory texture description. Legacy
  VTF content is never converted.
- The native Vulkan provider adds the BC4/5/6H/7, ASTC, and ETC2 format mappings
  it lacks today (as of 2026-09-22 it maps only DXT1/DXT3/DXT5 to BC1–BC3).
- The Hammer core texture reader (`hammer/core/formats/vtf_image.cpp`) gains
  KTX2 through the same reader, so the editor sees exactly the runtime texture.

## Materials

VMT stays the material file. Proxies, fallbacks, and the material system parser
depend on it, and changing the container gains nothing. The PBR family's
parameters (RFC 0007) are defined as an **OpenPBR-compatible subset** with a
lossless mapping table maintained by one schema owner:

| Our parameter | OpenPBR | UsdPreviewSurface | glTF 2.0 | Cycles Principled |
| --- | --- | --- | --- | --- |
| base color | `base_color` | `diffuseColor` | `baseColorFactor/Texture` | `Base Color` |
| metalness | `base_metalness` | `metallic` | `metallicFactor` (B channel) | `Metallic` |
| roughness | `specular_roughness` | `roughness` | `roughnessFactor` (G channel) | `Roughness` |
| normal | `geometry_normal` | `normal` | `normalTexture` | `Normal` |
| emission | `emission_color` × `emission_luminance` | `emissiveColor` | `emissive*` (+ `KHR_materials_emissive_strength`) | `Emission Color/Strength` |
| opacity / alpha test | `geometry_opacity` | `opacity`, `opacityThreshold` | `alphaMode`, `alphaCutoff` | `Alpha` |
| ambient occlusion | — (baked input) | `occlusion` | `occlusionTexture` (R channel) | not a BSDF input; used by the runtime only |

The World Stage writes `UsdPreviewSurface` (and later MaterialX OpenPBR) with
the VMT path under `source:`. The exact OpenPBR/glTF channel conventions are
verified against the pinned specifications when the schema is implemented.

## Native Vulkan world path

For BSP2 maps on the native Vulkan provider:

- **Geometry:** `WMSH` buffers are uploaded once. Culling uses the PVS leaf →
  cluster ranges, cluster bounds, and normal cones. GPU-driven indirect draws
  are used where the profile supports them, with CPU culling and per-batch draws
  otherwise.
- **Materials:** descriptor indexing ("bindless") where supported, and
  per-batch descriptor sets otherwise. Both paths are one capability with
  shared tests.
- **Static lighting:** SH L1 atlas layers blended by style scalars in the shader.
- **Dynamic objects:** SH L2 probe volume interpolation replaces ambient cubes.
- **Dynamic lights:** clustered forward lighting evaluated with the PBR BRDF.
  This replaces CPU lightmap re-compositing for dynamic lights (`dlight_t` in
  `R_BuildLightMap`).
- **Reflections:** parallax-corrected, blended prefiltered reflection probes.
- **Engine features that must keep working:** decals and overlays (via the
  triangle → face map), displacements, water, areaportals, occlusion, fog, sky,
  detail props, and static props. Each is a named cohort with a test, not an
  assumption.

Legacy maps and the D3D9/DXVK provider keep the existing world path. The world
path is selected per map by the container and the provider's capabilities, not
by a global switch.

## Validation and acceptance criteria

- **Independent readers:** a Python BSP2 reader in `tools/quality/` (not
  sharing C++ code) validates the directory, alignment, hashes, and every
  lump's structure. `usdchecker` validates stages, `ktx validate` validates
  textures.
- **Lossless carriage:** legacy v21 → BSP2 → legacy v21 is byte-identical for
  gameplay lumps across the corpus. Server-side collision, trace, and vis
  queries give identical results on both containers.
- **Malformed input:** fuzzing the BSP2 directory and each new lump (Q-CONTENT)
  with bounds checks. Truncated or overlapping lumps and hash mismatches are
  rejected with structured errors, never crashes.
- **Rendering equivalence:** a BSP2 map with the legacy payload renders the same
  on D3D9/DXVK as the classic v21 export of the same data. On the new path, it
  matches the Cycles reference within RFC 0007's per-term tolerances.
- **Load cost:** map load time and memory are measured for classic vs BSP2 on
  the corpus. The world-mesh and lightmap stages must show that rebuilding is
  gone (for example, `WorldStaticMeshCreate` and `R_RedownloadAllLightmaps` are
  not called for BSP2 maps on the new path).
- **Incremental builds:** a light-only change re-runs only bake and pack, and a
  texture-only change re-runs only encode, transcode, and pack. This is proven
  by build-graph traces with cache hit counts.
- **Negative fixtures:** wrong alignment, a stale hash, an unknown required lump,
  sRGB/linear mismatch, a wrong style layer mapping, a lightmap UV off by half a
  texel, and a missing legacy payload on a legacy-only provider must each fail.

## Delivery plan

| Phase | Slice | Gate |
| --- | --- | --- |
| F1 | BSP2 container, `IMapContainer` seam in `CMapLoadHelper`, Python reader; legacy lumps only | Byte-identical carriage, server/client load, fuzzing, dedicated server link evidence |
| F2 | World Stage schema and `vbsp2` geometry-layer emitter (with RFC 0007 Phase B), lightmap chart packer | `usdchecker` clean; stage opens in Cycles standalone; semantic comparator vs BSP faces; seeded loss detected |
| F3 | KTX2: container-neutral texture reader, UASTC encode/transcode in the packer, native Vulkan BC/ASTC/ETC2 formats, Hammer reader | `ktx validate`; per-format pixel fixtures; profile format negotiation fails correctly when a format is missing |
| F4 | `WMSH` + native Vulkan world path using legacy-equivalent lighting data | Legacy feature cohorts (decals, overlays, displacements, water, areaportals, fog, sky, props) each pass; load cost measured |
| F5 | `LMAP`/`LSTY`/`PRBV`/`RPRB` from RFC 0007 bakes, legacy exporter, light-style blending, clustered dynamic lights | Pixel oracles vs Cycles; legacy payload renders on D3D9/DXVK; style switching and dynamic light tests |
| F6 | Incremental build graph and cache; Hammer compile and preview use it | Cache-hit traces per change class; cancellation leaves the previous package intact |
| F7 | Mobile packaging: per-profile transcoding and packages for iOS/Android | Device format queries recorded; installed-package smoke tests on R29 runners |

F1 and F3 are independent of the lighting work and can start first.

## Roadmap

Tracked in the AGENTS.md ranked roadmap (added 2026-09-22): F1 → R53, F2 → R54,
F3 → R55, F4–F5 → R56, F6 → R57, F7 → R58. AGENTS.md owns their ranks and
states.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Two world paths in the engine for a long time | Selected per map by capability; each migrated cohort deletes its legacy branch for BSP2 maps; legacy path retained only for legacy content |
| New maps unplayable on old engines/tools | Explicit, versioned decision; classic v21 export profile; legacy payload for legacy renderers |
| USD dependency weight in the compile path | Decided: USD is used (see [USD decision](#usd-decision)). Tool and desktop-host profiles only; pinned revision and license record; never linked into engine products or mobile builds |
| Decals/overlays/displacements regress on the new world path | Triangle → face map; per-cohort tests before enabling the path |
| Mobile format support differs by device | Profile-declared formats, device queries, composition failure instead of silent fallback |
| Hash/compression dependencies proliferate | One hash (BLAKE3 or XXH3-128) and one new compressor (zstd), each pinned; lzma kept for legacy lumps |
| Light-style semantics drift | Style layers carry the same style ids; gameplay style changes are tested for identical observable timing |

## Alternatives considered

### Keep BSP v21 and add data as game lumps

Game lumps (`LUMP_GAME_LUMP`, 4CC ids such as `sprp`) are extensible, and old
engines ignore unknown ids. This keeps old tools working, but it keeps 32-bit
offsets, no alignment for direct upload, no integrity hashes, and a nested
directory. It is rejected as the primary container. The legacy v21 export
profile covers old-engine needs.

### glTF as the World Stage

glTF is the most widely supported *runtime asset* format, with KTX2 support
(`KHR_texture_basisu`). It has no layer composition, only a limited light model
(`KHR_lights_punctual`), and no standard place for large Source-specific data.
It is kept as an **export** target for props and individual assets, not as the
world stage.

### Replace BSP entirely, including gameplay data

This would change collision, traces, vis, areaportals, and physics data, all of
which are gameplay-observable and owned by RFC 0004 and the engine. There is
nothing to gain for this RFC's goals, and the risk is large. Rejected.

### Basis-only textures transcoded at load

This gives the smallest packages, but transcoding at load costs CPU time on
mobile, and quality is capped by the universal format. It is kept as an option
for data downloads. Installed packages ship pre-transcoded profile formats.

### In-house stage format with a USD exporter

A smaller binary format would keep OpenUSD out of the compile path. It was
rejected by the user's decision (2026-09-22): it adds a second scene
representation and a conversion that USD makes unnecessary, and it loses
Cycles' and DCC tools' direct reading of the stage.

### MaterialX files instead of VMT

MaterialX is the richer open standard, but the material system, proxies, and
fallbacks are VMT-based. The PBR parameters already map one-to-one to OpenPBR,
and the stage can emit MaterialX when needed.

## Open decisions and required evidence

1. BSP2 magic and version numbering. Must not collide with Valve or other
   branch versions; recorded with a reader test for every known legacy version.
   *F1 prototype:* `"SRCBSP2\x1A"`, container version 1 (see
   [progress](0008-progress.md)).
2. Hash (BLAKE3 vs XXH3-128) and compression (zstd) dependencies. Pin and
   license records. *F1 prototype:* BLAKE2b-128 (RFC 7693, no new dependency,
   algorithm id in the header) and no compression until zstd is pinned.
3. Probe volume structure: adaptive grid vs tetrahedral (per-leaf) placement.
   Needs measured quality and memory on the corpus.
4. SH L1 lightmap encoding (L0 in BC6H and L1 normalized in BC7, or
   alternatives), decided by pixel-oracle error and memory.
5. Whether BSP2 packages carry the legacy payload by default during migration
   (recommended: yes until R56 closes).
6. ~~USD in the compile path vs an in-house stage format~~ *Decided
   2026-09-22 by the user:* OpenUSD is the World Stage in the compile path. See
   [USD decision](#usd-decision).
7. Lightmap chart packer: our own, or a pinned library (for example xatlas).
   Charts must respect smoothing groups and displacement seams.
8. Per-profile format tables, filled from device queries.

## Source references

- `public/bspfile.h`: header, `lump_t`, `HEADER_LUMPS`, `BSPVERSION`/`MINBSPVERSION`, lump ids
- `public/gamebspfile.h`: game lump 4CCs
- `public/lumpfiles.cpp`: lump override files
- `engine/modelloader.cpp`: `CMapLoadHelper`
- `engine/gl_lightmap.cpp`: `R_BuildLightMap`, `R_RedownloadAllLightmaps`
- `engine/matsys_interface.cpp`: `WorldStaticMeshCreate`
- `engine/buildcubemaps.cpp`
- `public/bitmap/imageformat.h`, `vtf/`, `hammer/core/formats/vtf_image.cpp`
- `materialsystem/shaderapivulkan/shaderapivulkan.cpp`: texture format mapping
- KTX-Software `tools/ktx/command_encode.cpp`, `command_transcode.cpp`, `command_validate.cpp`
- OpenUSD `pxr/usd/usdLux/schema.usda`, `pxr/usdValidation/bin/usdchecker`

## Proposed decision

Accepted for planning on 2026-09-22 with OpenUSD as the World Stage. The
four-tier data model and the versioned compatibility decision:
new maps compile to BSP2 with KTX2 textures and canonical SH lighting, and
legacy content and legacy renderers remain supported through carriage and a
single derived payload. Recommended first work is **F1** (R53, container and
reader seam) and **F3** (R55, KTX2 textures). Neither depends on the lighting
work, and both are testable on their own.
