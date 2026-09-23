# RFC 0008: Canonical World Data and Runtime Formats

- Status: Accepted for planning (2026-09-22); F1 active, F2 partial ([progress](0008-progress.md))
- Date: 2026-09-22
- Scope: The compiled-world interchange stage, runtime map and model resources,
  world render data (mesh, lightmaps, probes, reflection probes), the texture
  container, PBR material parameter semantics, the native Vulkan world path,
  modern visual capabilities, and incremental map builds
- Related: [RFC 0007: Physically Based Lighting Pipeline](0007-physically-based-lighting-pipeline.md)
  (produces the lighting data defined here),
  [RFC 0009: USD-Native Map Authoring](0009-usd-native-map-authoring.md)
  (owns the future editable USD source and its native compiler),
  [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
  (render seam, native Vulkan, platform profiles, tool/process cleanup),
  [RFC 0002: Hammer Responsibility Factorization](0002-hammer-responsibility-factorization.md)
  (authoring document and compile workflow),
  [RFC 0003: Dependency-Aware Job System](0003-dependency-aware-job-system.md)
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Evaluated dependencies: OpenUSD v25.11, pinned and built for the Linux tools
  slice in [the host profile](../quality/product_profiles/openusd-linux-tools.json)
  (Tomorrow Open Source Technology License 1.0); KTX-Software (host-tool source revision, build options, and
  toolchain pinned in [the F3 feasibility profile](../quality/product_profiles/ktx2-linux-tools.json);
  Apache-2.0 for project files, with bundled component licenses under
  `LICENSES/`); Cycles as recorded in RFC 0007. The KTX tool build is feasibility
  evidence, not a runtime integration or platform support claim.

## Summary

The user directed that the renderer, BSP, and intermediate formats may be
upgraded to the best-supported data structures, so that the world is not
converted again every time a tool or the runtime touches it. This RFC records
that decision as the versioned content-compatibility change that AGENTS.md
requires, and defines its boundaries.

The world has exactly **four representations**, each with one owner. Every
transformation between them happens once, is keyed by content hash, and is
cached:

1. **Authoring:** VMF plus source assets for the current compatibility path;
   RFC 0009 defines editable OpenUSD as the source for new maps. RFC 0002 owns
   editor operations in either path. Authored USD and compiled USD are different
   representations even though they use the same file technology.
2. **Compiled World Stage:** an OpenUSD stage emitted by a geometry compiler. It is
   the single compiled-world description read by the light baker (RFC 0007),
   reference rendering, DCC tools, and the runtime packer. Lighting is a separate
   USD layer, so rebaking never rewrites geometry.
3. **Runtime package (BSP2):** a new map container with 64-bit offsets, a 4CC
   lump directory, per-lump versioning, compression and content hashes, and
   GPU-ready alignment. F1 carries legacy gameplay lumps with unchanged
   semantics; F8 can add versioned native gameplay/spatial payloads. New render
   lumps include a pre-batched world mesh, SH L1 lightmap
   atlases, an SH L2 probe volume, and prefiltered reflection probes. Textures
   use **KTX2**.
4. **GPU-resident:** what the native Vulkan world path uploads. Render lumps are
   laid out so that loading is a copy, not a rebuild.

Legacy maps (BSP v19–21, VTF, VMT) keep loading unchanged forever. BSP2 packages
may also carry derived legacy lighting lumps, so that the D3D9/DXVK provider and
legacy shaders can still render new maps. That derivation lives in one exporter
and is the only place new data is converted to legacy data.

The installed `vbsp2` slice emits this compiled stage from VMF. That path is a
bootstrap and compatibility producer, not a requirement that all future World
Stages originate in VMF or BSP. RFC 0009 adds the native USD source producer;
the baker, packer, and runtime format should depend on the compiled-stage
contract rather than on its producer.

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
- A compiled-stage contract that accepts either the legacy VMF producer or
  RFC 0009's native USD producer without losing object/surface provenance.
- Open, widely supported formats wherever an equivalent exists: OpenUSD (scene),
  KTX2 (textures), UsdLux (lights), UsdPreviewSurface / glTF metallic-roughness /
  OpenPBR subset (materials).
- Runtime data that the native Vulkan provider uploads without rebuilding it,
  on desktop and mobile profiles.
- Unchanged gameplay semantics: collision, visibility, areaportals, entities,
  physics, and the dedicated server see the same data as today.
- Legacy content stays loadable, and legacy renderers can render new maps
  through a declared, derived compatibility payload.
- Complete the USD-native map authoring path in RFC 0009: world solids and
  placed models remain distinct while editing, and a native compiler produces
  playable maps without a VMF or precompiled BSP input.
- Evolve spatial, collision, visibility, and gameplay data where the old lump
  layout limits native USD geometry or map scale. New versioned encodings must
  preserve declared gameplay behavior; byte-identical carriage remains the
  rule for legacy BSP inputs, not for newly authored USD maps.
- Add a modern model asset path for static, dynamic, and physics props, including
  source import, material and collision data, animation where required, and a
  versioned compiled runtime resource. Keep existing MDL/VVD/VTX assets usable.
- Deliver modern visual and editing results together: PBR materials, authored
  lights, reflections, transparency, shadows, dense geometry, responsive
  preview, and build iteration are assessed in the same representative scenes.
  Real-time GI and virtualized or streamed geometry are explicit candidate
  capabilities with measured quality, cost, and platform gates.
- Support direct USD scene loading in a desktop development runtime for live
  iteration. Evaluate it separately on mobile against package, memory, startup,
  and lifecycle limits; the installed compiled package remains supported.

## Compatibility and phase boundaries

F1–F7 establish the container, compiled stage, textures, renderer, lighting,
build graph, and mobile package path. RFC 0009 owns editable USD maps and the
native map compiler. F8–F11 below own later runtime/content modernization.
Their separate gates prevent an early BSP2 or lighting slice from claiming the
complete visual and authoring outcome.

Legacy BSP lumps and MDL/VVD/VTX resources retain their declared readers and
compatibility behavior. New map and model encodings are versioned rather than
reinterpreting those bytes. Direct USD loading is a development workflow; it
does not make OpenUSD a mandatory dependency of clients, dedicated servers, or
mobile packages. A measured mobile profile may opt in after native lifecycle
and package evidence. Real-time GI and virtualized geometry must earn their
place through visual and performance oracles; neither is assumed to require a
particular API or algorithm.

## The four tiers

```
AUTHORING                    COMPILED WORLD STAGE             RUNTIME PACKAGE           GPU
VMF + legacy assets ──► vbsp2 ─┐
editable USD + assets ──►      ├──► geometry.usdc ──┐
  native USD compiler (0009) ──┘    lighting.usdc ──┼──► packer ──► map.bsp2 ──► Vulkan
                                    probes.usdc   ──┘     │          + KTX2
                                          │                └──► optional legacy payload
                                          ▼                         ──► D3D9/DXVK
                                 DCC tools / Cycles reference
```

### Conversion ledger

Every arrow above is one row. No other code performs these transformations.

| Transformation | Sole owner | Cache key |
| --- | --- | --- |
| VMF → compiled geometry layer + gameplay lumps | `vbsp2` compatibility producer (RFC 0007 Phase B) | VMF semantic hash, referenced asset hashes, tool version |
| Authored USD → compiled geometry layer + gameplay data | Native USD map compiler (RFC 0009; planned) | Composed source-layer and referenced-asset hashes, schema/tool versions |
| Authored model source → compiled model resource | Model asset compiler (F9; planned) | Composed source and dependency hashes, schema/tool/profile versions |
| Portals → visibility | `vvis` | Portal lump hash, tool version |
| Geometry layer → lighting/probe layers | `ILightBaker` providers (RFC 0007) | Geometry layer hash, light prims hash, material albedo/emission hashes, bake settings, provider version |
| Texture master → KTX2 UASTC (LDR/HDR) | `TextureEncoder` | Source image hash, encode settings, encoder version |
| KTX2 UASTC → profile format (BC*/ASTC/ETC2) | `TextureTranscoder` in the packer | KTX2 hash, profile, transcoder version |
| Stage layers → BSP2 render lumps | `WorldPacker` | Layer hashes, profile, packer version |
| Canonical lighting → legacy lumps | `LegacyLightingExporter` | Lighting layer hash, exporter version |
| BSP2 lumps → GPU resources | native Vulkan world path | None (direct upload) |
| Authored USD → live development world | F11 provider using the same validator/compiler contracts | Source revision, dependency hashes, compiler/profile versions |

Build steps are nodes in a dependency graph that uses the RFC 0003 job system
inside tools. The input producer is part of each graph's cache key. Changing
only a light re-runs the bake and pack steps. Changing
only a material parameter re-runs pack (and bake if albedo or emission changed).
Changing only a texture re-runs encode, transcode, and pack. Geometry changes
re-run the whole graph.

### Later content and runtime boundaries

F8 may add versioned spatial/gameplay payloads to BSP2 when USD-native meshes,
larger maps, or prop placement cannot be represented faithfully in legacy
lumps. The native producer owns those payloads; the runtime map reader selects
their declared version. Collision, traces, PVS, areaportals, entities, save
state, and dedicated-server behavior are checked against semantic fixtures.
The legacy producer still carries its original lumps byte-for-byte. Neither
producer silently changes gameplay to fit a render-only mesh.

F9 treats a model as an authored asset with a compiled runtime resource.
OpenUSD or another declared source import can carry geometry and composition;
the model compiler owns material bindings, LOD, collision, rig and animation
where the prop role requires them. The runtime resource has one versioned
reader and profile-specific GPU payloads. Existing MDL/VVD/VTX readers remain
for legacy content. Static, dynamic, and physics placements in RFC 0009 select
the appropriate model capabilities through a validated reference, not filename
or mesh inspection. A separate contract record is needed before implementation
to pin animation, networking, persistence, and `studio.h` compatibility.

F10 measures visual results on authored scenes that expose material response,
glass/transmission, reflections, shadows, indirect light, and dense geometry.
The implementation may choose the GI and dense-geometry algorithms after
measuring quality and per-profile cost; hardware ray tracing and virtualized
geometry are options, not assumed prerequisites. The gate requires a real-time
indirect-light response to moving lights or objects on a declared desktop
profile, a measured geometry-scaling method for dense scenes, and explicit
fallbacks on profiles that lack those capabilities. An attractive static
screenshot alone cannot close it. Reference images, interaction sequences,
frame-time, memory, and load budgets are recorded before choosing a technique.

F11 is an opt-in development composition. It loads and validates the authored
stage, then invokes the same native compiler contracts into private runtime
data for rapid edit/reload/play; it does not create another map document or
fork gameplay semantics. The production package path remains the oracle.
Desktop implementation is required. A mobile build is separately selected
only after native memory, startup, lifecycle, static composition, and package
checks; inability to fit OpenUSD on a declared mobile profile does not block
its compiled-package support.

## World Stage (OpenUSD)

### Why USD

It is the most widely supported open scene description for production tools.
Cycles standalone reads it directly (RFC 0007 findings). It has standard light
(`UsdLux`: `SphereLight`, `DiskLight`, `RectLight`, `DistantLight`, `DomeLight`,
`ShapingAPI` for spot cones) and material (`UsdShade`, `UsdPreviewSurface`,
MaterialX) schemas. Its layer composition lets the baker add lighting as a
sublayer without rewriting geometry. The inspected revision includes the
`usdchecker` validation tool (`pxr/usdValidation/bin/usdchecker`).

### Compiled-stage structure

```
/World                               (Xform; metersPerUnit = 0.0254, upAxis = Z)
  /Geometry
    /WorldSpawn/Mesh_<materialBatch> (Mesh; triangulated faces and displacements)
    /BrushEntities/<entity>/Mesh_... (func_* models, with entity reference)
  /Props/<id>                        (model references with explicit source role)
  /Lights/<entity>                   (UsdLux prims + source: attributes)
  /Materials/<vmt path>              (UsdShade Material + source: attributes)
  /Probes
    /ReflectionProbes/<id>           (position, influence box, parallax box)
    /ProbeVolume                     (probe positions, leaf association)
  /Entities/<id>                     (non-geometric entities as typeless prims + key-values)
```

- Geometry meshes carry primvars: `st` (material UV), `lightmap:st` (atlas UV
  from the lightmap chart packer), and source surface identity. The current
  VMF-derived slice writes `source:faceId` to map triangles back to BSP faces
  for decals, overlays, and legacy export, plus `source:smoothingGroup`.
  A native USD source need not have BSP face IDs; RFC 0009 owns stable authored
  IDs and the compiler's triangle-to-surface mapping. New compiled-stage
  consumers must not infer that every mesh is named after a BSP face.
- The **lightmap chart layout is authored in the geometry layer**, so bakers,
  runtime, and exporters share one layout. It replaces per-face
  `lightmapVecsLuxelsPerWorldUnits` as the canonical source. The legacy face
  layout is derived from it by the legacy exporter when requested.
- Every Source-specific datum without a standard schema is kept under the
  `source:` namespace, so a round trip through DCC tools preserves it.
- The lighting layer (`lighting.usdc`) references the lightmap and probe
  payloads as KTX2 asset paths and records provider, mode, policy version, and
  sample statistics as metadata.

The installed schema's `source:chartFaceIds`, `primvars:source:faceId`, and
`source:materialPath` are VMF/BSP/VMT provenance. They cannot be required for
all producers. Before F2 closes, evolve the versioned compiled schema so
charts and triangles map to canonical source surface IDs and materials have a
producer-independent asset identity; retain the legacy fields when available.
An independent fixture with authored surface IDs and no BSP face IDs must pass
schema validation and chart/triangle provenance checks. Schema readers and
the future baker/packer must not synthesize face ID zero or infer a VMT path
to accept that fixture. The compiled entity index remains an output identity,
not an editable-map object ID.

### USD decision

Decided by the user on 2026-09-22: the World Stage uses OpenUSD directly in the
compile path, not an in-house format with a USD exporter. Consequences:

- `vbsp2`, the bakers, the packer, the reference renderer, and Hammer's export
  and preview link the OpenUSD C++ API (`pxr/usd`, `usdGeom`, `usdLux`,
  `usdShade`) on desktop tool profiles. F1–F7 installed client/server and
  mobile profiles read compiled packages without OpenUSD; R12/R53 link evidence
  checks that boundary. F11 adds a separately declared development-runtime
  provider and evaluates an optional mobile profile.
- A single OpenUSD revision is pinned per tool profile with its license record
  (TOST 1.0). It must be 25.11 or newer to match the Cycles Hydra delegate's
  stated requirement, so one USD build serves both.
- The stage schema is a generated USD schema. Readers use the schema API, not
  string attribute lookups.
- `usdchecker` runs in the validation lane for every emitted stage.
- `.usdc` in build caches and `.usda` for reviewed fixtures, with
  deterministic prim ordering so fixture diffs are reviewable.

A typed compiled-stage schema (`SourceWorldAPI` and friends) is generated with
`usdGenSchema` so attribute names and types have one definition. RFC 0009's
editable-map schema is versioned separately; generated chart and BSP IDs must
not become required authored fields. Fixtures use `.usda`; build caches use
`.usdc`.

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
BSP2. For F1, gameplay, collision (`engine/cmodel*.cpp`), and the dedicated
server keep calling it by legacy lump identity. F8 adds a typed versioned path
for native spatial/gameplay payloads while preserving consumer behavior. The
dedicated server reads only gameplay data and must not link render or texture
dependencies (checked by R12's link evidence).

### New render lumps

| 4CC (proposed) | Content | Replaces at runtime (for BSP2 maps on the new path) |
| --- | --- | --- |
| `WMSH` | World render mesh: vertex/index buffers in the GPU layout (position, octahedral normal/tangent, material UV, lightmap UV), meshlet clusters with bounds and normal cones, draw batches by material, per-leaf cluster ranges (PVS culling), triangle → canonical source surface map (optional legacy face map) | `WorldStaticMeshCreate` rebuild at load |
| `LMAP` | Lightmap atlas pages: SH L1 irradiance (L0 HDR RGB + L1 directional), one layer per light style; KTX2 assets in the asset table | `ColorRGBExp32` lightmaps, CPU `R_BuildLightMap` compositing, lightmap page allocation at load |
| `LSTY` | Light style table: style id → atlas layer, per-chart style masks | Per-surface `styles[MAXLIGHTMAPS]` compositing on the CPU |
| `PRBV` | Probe volume: probe positions, SH L2 irradiance per probe (per style), leaf → probe tetrahedra/cell index, validity masks | Leaf ambient cubes |
| `RPRB` | Reflection probes: position, influence and parallax boxes, blend priority, KTX2 prefiltered HDR cube (GGX roughness mips) | `env_cubemap` VTFs from `buildcubemaps` |
| `MTBL` | Material table: canonical material asset identity, optional legacy VMT path, family, shader capability requirement, hashes | `texdata` string table lookups for render batching |
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
master to the profile's GPU format. KTX-Software at the pinned host-tool revision
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
| F2 | Compiled World Stage schema and VMF `vbsp2` geometry-layer emitter (with RFC 0007 Phase B), lightmap chart packer | `usdchecker` clean; stage opens in Cycles standalone; semantic comparator vs BSP faces; seeded loss detected; face-ID-free compiled fixture validates with stable surface/material provenance; native USD source is a separate RFC 0009 gate |
| F3 | KTX2: container-neutral texture reader, UASTC encode/transcode in the packer, native Vulkan BC/ASTC/ETC2 formats, Hammer reader | `ktx validate`; per-format pixel fixtures; profile format negotiation fails correctly when a format is missing |
| F4 | `WMSH` + native Vulkan world path using legacy-equivalent lighting data | Legacy feature cohorts (decals, overlays, displacements, water, areaportals, fog, sky, props) each pass; load cost measured |
| F5 | `LMAP`/`LSTY`/`PRBV`/`RPRB` from RFC 0007 bakes, legacy exporter, light-style blending, clustered dynamic lights | Pixel oracles vs Cycles; legacy payload renders on D3D9/DXVK; style switching and dynamic light tests |
| F6 | Incremental build graph and cache; Hammer compile and preview use it | Cache-hit traces per change class; cancellation leaves the previous package intact |
| F7 | Mobile packaging: per-profile transcoding and packages for iOS/Android | Device format queries recorded; installed-package smoke tests on R29 runners |
| F8 | Versioned native map spatial/gameplay data for USD geometry | Closed-world, collision, traces, portals/PVS, areaportals, entities and server behavior pass independent semantic and negative fixtures; legacy BSP carriage remains byte-identical |
| F9 | Modern model asset compiler and runtime reader | Static/dynamic/physics roles use validated modern assets with materials, collision, LOD and required animation; MDL compatibility and client/server lifetime tests pass |
| F10 | Modern visual parity and geometry scalability | Representative maps pass registered image and interaction oracles for material response, reflections, transparent surfaces, shadows and lighting; real-time GI and dense-geometry methods pass measured quality, memory, frame-time and fallback gates on declared profiles |
| F11 | Direct USD development-runtime iteration | Desktop scene edit/reload/play loop uses the same validated source and compiled contracts without stale state; mobile opt-in decision records package, startup, memory and lifecycle evidence |

F1 and F3 are independent of the lighting work and can start first.

## Roadmap

Tracked in the AGENTS.md ranked roadmap (added 2026-09-22): F1 → R53, F2 → R54,
F3 → R55, F4–F5 → R56, F6 → R57, F7 → R58, F8 → R61, F9 → R62,
F10 → R63, F11 → R64. AGENTS.md owns their ranks and states.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Two world paths in the engine for a long time | Selected per map by capability; each migrated cohort deletes its legacy branch for BSP2 maps; legacy path retained only for legacy content |
| New maps unplayable on old engines/tools | Explicit, versioned decision; classic v21 export profile; legacy payload for legacy renderers |
| USD dependency weight in the compile path | Pin the revision and license record; keep F1–F7 installed products free of the dependency; F11 measures the isolated development loader and optional mobile profile before enabling either |
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
