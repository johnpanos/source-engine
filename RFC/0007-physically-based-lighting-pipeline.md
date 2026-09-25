# RFC 0007: Physically Based Lighting Pipeline

- Status: Accepted for planning (2026-09-22); implementation partial (see
  [progress](0007-progress.md))
- Amended: 2026-09-24 by [RFC 0011](0011-runtime-indirect-lighting.md)
  (proposed): separated direct/indirect lightmap outputs, probe outputs in the
  `render.probe-volume.v1` encoding with visibility, and the optional `RTRN`
  radiosity transfer precompute. Amended passages say so inline
- Date: 2026-09-22
- Scope: Map compile tools (vbsp, vvis, vrad), a substitutable light baker with a
  Cycles provider, a PBR material family on the native Vulkan backend, image-based
  lighting, open-format scene interchange, and Hammer compile/preview integration
- Formats: [RFC 0008: Canonical World Data and Runtime Formats](0008-canonical-world-data-and-runtime-formats.md)
  owns the World Stage, BSP2, KTX2, and lighting data encodings this RFC produces
- Native authoring: [RFC 0009: USD-Native Map Authoring](0009-usd-native-map-authoring.md)
  owns the editable USD source; this RFC's baker consumes the compiled stage
- Related: [RFC 0001: Capability-Based Platform Architecture](0001-capability-based-platform-architecture.md)
  (render seam, native Vulkan, tool/process cleanup),
  [RFC 0002: Hammer Responsibility Factorization](0002-hammer-responsibility-factorization.md)
  (headless editor core, compile/run workflow),
  [RFC 0003: Dependency-Aware Job System](0003-dependency-aware-job-system.md)
  (in-tool parallelism)
- Verification: [RFC 0005: Quality and Correctness Harnesses](0005-quality-and-correctness-harnesses.md)
- Language and synchronization: [RFC 0006: C++20, Ownership, and Synchronization](0006-modern-cpp-ownership-and-synchronization.md)
- Evaluated dependencies (source inspected, not built or pinned; see
  [Pre-implementation findings](#pre-implementation-findings-2026-09-22)):
  Cycles standalone `a456b761034dda42c32eef9f4aae0fa5a5c9f604` (2026-09-08,
  Apache-2.0); Open Image Denoise `6602ee2ca38a1a2a02135beed8f6e68eed630180`
  (2026-08-14, Apache-2.0); OpenUSD `fc590b38e24770408e31b56442d31b8029729ee7`
  (2026-09-22, Tomorrow Open Source Technology License 1.0). Each is pinned with
  a license record before its phase begins.

## Summary

This RFC proposes a higher-quality static lighting pipeline for Source maps.
It computes the lighting data whose formats RFC 0008 defines, in four separable
parts:

1. **A substitutable light baker.** vrad's scene gathering and lump writing are
   extracted from its radiosity solver. The solver moves behind an `ILightBaker`
   contract whose input is built from the RFC 0008 World Stage. The legacy vrad
   solver remains as the parity provider and oracle. A Cycles-based provider
   supplies path-traced quality and produces the canonical lighting: SH L1
   lightmaps, exact RNM bases, probe-volume irradiance and visibility
   (RFC 0011), and HDR reflection probes.
2. **A PBR material family** on the native Vulkan backend: a metal/roughness
   shader with a GGX specular BRDF, consuming the same baked lighting as legacy
   shaders plus prefiltered image-based lighting. It is a new, opt-in family.
   Legacy shader families keep their D3D9-defined behavior.
3. **Image-based lighting**: reflection probes rendered by the baker and
   prefiltered for GGX roughness (RFC 0008 `RPRB`). Legacy maps get a runtime
   prefilter of their existing cubemaps.
4. **Reference rendering**: the pinned Cycles standalone renders the RFC 0008
   World Stage directly. It serves as the oracle for the PBR runtime and as the
   open path into DCC tools. Interchange formats themselves belong to RFC 0008.

vvis is not affected by the Cycles work: it computes visibility, not lighting.
It is ported and parallelized as an independent track with an exact-output
oracle.

The first bounded delivery is a feasibility spike: bake direct lighting for
one corpus map with Cycles into Source's own luxel layout. It is written by the
shared lump writer and compared against vrad and analytic cases. That spike
decides whether Cycles is used as a library or replaced by a direct
Embree-based tracer behind the same contract (see [Alternatives](#alternatives-considered)).

## Motivation

vrad's patch radiosity produces the characteristic artifacts of Source maps:
light leaks at brush seams, blotchy indirect light, stair-stepped shadows from
chopped patches, and long `-final` compile times on CPU (with VMPI distribution
that no longer exists in this tree). Artists compensate by hand-tuning light
falloff and bounce behavior.

A path tracer with modern sampling, soft area/sky light, denoising, and GPU
backends can produce cleaner lightmaps in less time. Blender's Cycles is a
mature, Apache-2.0 licensed production path tracer with CPU (Embree) and GPU
backends, and its Principled BSDF is the de facto metal/roughness reference.

Better baked lighting is only half of the visible result. Legacy Source
materials are Phong-with-cubemap approximations whose specular response is
tuned per material. Without a physically based material model, the runtime frame
cannot match what the baker (or Blender) shows, and exporting materials to open
formats is a lossy guess. PBR materials make the runtime, the baker, and the
interchange formats describe the same surface.

An open interchange format lets the editor's authored scene leave the Source
tool ecosystem for look-development, reference renders, and review, without
making Blender (GPL) a build or runtime dependency.

## Goals

- Keep every existing map and material rendering as before unless it opts in.
- Produce canonical lighting data in the RFC 0008 formats. Legacy lighting lumps
  are derived from it only by RFC 0008's single legacy exporter, so
  quality-baked maps still render on legacy renderers when a package requests
  that payload.
- Make the baker a substitutable provider with one shared conformance suite,
  including a parity provider (legacy vrad) and deliberately bad fixtures.
- Give physically based materials an independent, meaningful oracle: Cycles
  reference renders and analytic BRDF checks, not screenshots of themselves.
- Keep one authority for units, color encoding, luxel layout, and lump format.
- Make compile tools build under Waf on declared profiles, run headlessly, and
  integrate with the new Hammer's compile/run workflow.

## Non-goals

- Replacing or "upgrading" legacy shader families (`LightmappedGeneric`,
  `VertexLitGeneric`, ...). Their fidelity is owned by RFC 0001 R32/R36 and
  defined by the D3D9 implementation.
- Implementing real-time GI or hardware ray-traced runtime lighting.
  [RFC 0011](0011-runtime-indirect-lighting.md) owns runtime indirect light and
  its producers; RFC 0008 F10 owns the measured visual gate. This RFC's bake
  supplies RFC 0011's baked producer data, the separated direct/indirect
  layers, and the radiosity transfer precompute, and its reference renders
  remain the comparison oracle.
- Defining containers, encodings, or interchange schemas. RFC 0008 owns them;
  this RFC owns the values computed into them.
- Replacing vbsp's BSP construction, or making vvis use Cycles.
- Linking Blender or any GPL component into engine, tools, or products.
- Importing USD/glTF back into VMF as an editable document. RFC 0009 instead
  defines native USD authoring, with VMF as a compatibility import and an
  explicit loss policy.
- Requiring a GPU for map compilation. CPU is always a supported bake profile.

## Evidence and current architecture

Observed at the working tree of 2026-09-22 (branch `subsystem-refactor`):

- `utils/vbsp`, `utils/vvis`, and `utils/vrad` exist as VPC-era sources with
  **no Waf build** (no `wscript` in any of the three). No current profile builds
  or runs the map compile pipeline. vrad is about 21k lines in `utils/vrad`.
- vrad writes these lumps (`public/bspfile.h`): `LUMP_LIGHTING` (8),
  `LUMP_LIGHTING_HDR` (53), `LUMP_WORLDLIGHTS` (15) / `_HDR` (54),
  `LUMP_LEAF_AMBIENT_LIGHTING` (56) / `_HDR` (55) with their index lumps (52/51),
  `LUMP_DISP_LIGHTMAP_ALPHAS` (32) and `LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS` (34).
  It also writes per-face lightmap offsets in `LUMP_FACES` / `LUMP_FACES_HDR` (58),
  and static prop vertex lighting files `sp_%d.vhv` / `sp_hdr_%d.vhv` into the
  pak lump (`vradstaticprops.cpp`).
- Luxel layout is defined by vbsp, not vrad: `texinfo_t::lightmapVecsLuxelsPerWorldUnits`
  and `dface_t::m_LightmapTextureMinsInLuxels` / `m_LightmapTextureSizeInLuxels`.
  Each face carries up to `MAXLIGHTMAPS` (4) light styles, and bumped faces
  store four samples per luxel (flat plus three basis directions from
  `public/mathlib/bumpvects.h`), encoded as `ColorRGBExp32`.
- vrad depends on vvis output: its visibility matrix and light culling
  (`vismat.cpp`) use the PVS in `LUMP_VISIBILITY`.
- Cubemaps: vbsp records `env_cubemap` samples in `LUMP_CUBEMAPS` (42,
  `dcubemapsample_t`). Their images are rendered in-engine by `buildcubemaps`
  and stored as VTFs in the pak lump. They are not roughness-prefiltered.
- Legacy Hammer's lighting preview drives vrad through `public/ivraddll.h`
  (`IVRadDLL::DoIncrementalLight`, `Interrupt`, `GetPercentComplete`) from
  `hammer/bsplighting.cpp`. This is the existing in-editor bake consumer.
- The headless Hammer core already prepares compile input:
  `hammer/core/formats/map_export.cpp` (`PrepareForCompile`: instance expansion,
  VisGroup filtering, cordon). It reads materials through
  `hammer/core/formats/material_catalog.cpp` and VTF/VPK readers in the same
  directory. There is no compile/run workflow in the GTK host yet (RFC 0002 H5).
- At the initial 2026-09-22 baseline, the native Vulkan backend had no new PBR
  material path. A synthetic direct-specular pixel path is now recorded in
  [progress](0007-progress.md); the complete material family remains open. The
  repository already has a separate `PBR` shader in
  `materialsystem/stdshaders/pbr_dx9.cpp` and its FXC sources; its name and
  existing content semantics must be preserved. The native shader work
  reproduces legacy families against D3D9 oracles. The material pixel harness
  (`tools/quality/material_pixel_conformance.py`) defines families `lightmap`,
  `exposure`, `skinning`, and `portal`, captured on D3D9 and compared on native
  Vulkan. See [native Vulkan progress](0001-native-vulkan-progress.md).
- The native backend implements integer HDR, auto-exposure, and tonemapping
  consistent with D3D9. The PBR family writes into that chain rather than adding
  a second one.

## Pre-implementation findings (2026-09-22)

These findings come from reading source only. The upstream repositories were
shallow-cloned at the revisions in the header into a scratch directory, not
into this tree. Nothing was built or run. Each finding narrows an open decision
but does not replace the phase gate that must demonstrate it.

### Cycles accepts externally supplied bake points (answers Phase C's main question in principle)

- The standalone host API supports baking with caller-chosen surface points.
  `OutputDriver::read_render_tile` (`src/session/output_driver.h`) is documented
  as the hook where the host fills the `PASS_BAKE_PRIMITIVE`, `PASS_BAKE_SEED`,
  and `PASS_BAKE_DIFFERENTIAL` passes for each pixel.
  `integrator_init_from_bake` (`src/kernel/integrator/init_from_bake.h`) then
  shades exactly that triangle and barycentric coordinate. A primitive of `-1`
  skips the pixel.
- The bake "image" is therefore just a list of samples: pixel positions carry no
  geometric meaning. `LightmapSampleLayout` can pack lightmap and vertex samples in
  any order. No UV unwrap or rasterization is involved.
- Barycentrics use Blender's convention and are converted inside the kernel. The
  provider must write Blender-convention `(u, v)`. This is covered by a
  registration test.
- **Constraint: one bake target object per render.** `BakeManager::device_update`
  (`src/scene/bake.cpp`) selects the first object flagged `is_bake_target`, and
  primitive indices are offsets within it. All bakeable world geometry must be
  merged into one target mesh, or baked in several sessions. Occluders that are
  not targets may be separate objects.
- Antialiasing jitter uses the supplied differentials and is kept inside the
  source triangle. Source faces are polygons that we triangulate, so the provider
  sets differentials to zero and does its own luxel supersampling by emitting
  several sample points per luxel, which gives us control of the footprint.
  Luxels whose centers fall outside the face polygon must be moved inside it by
  `LightmapSampleLayout` (vrad does the equivalent today).
- Surface bakes shade with a ray arriving along the shading normal (`use_camera`
  off), so view-dependent terms are irrelevant for diffuse passes.
- `PASS_DIFFUSE_DIRECT` / `PASS_DIFFUSE_INDIRECT` are divided by
  `PASS_DIFFUSE_COLOR` unless albedo is included (`src/scene/pass.cpp`). With a
  white Lambertian closure, the result is irradiance divided by π. `LightingPolicy`
  owns that factor.

### Radiosity normal mapping bases need one session per basis direction

The Diffuse BSDF's normal comes from the shader graph, not from each pixel. The
normal-override approach therefore needs four bakes per light style (flat plus
three bases). Each uses a shader whose Diffuse BSDF normal is the world-space
basis direction derived from the tangent frame. Lambertian evaluation clamps
`dot(N, wo)` at zero (`src/kernel/closure/bsdf_diffuse.h`), which matches the
clamped cosine lobe of Source's bases. The basis oracle must still confirm how
light arriving below the *geometric* normal is rejected. The one-pass
alternative remains a kernel patch. Cost is four sessions times the style count,
reusing the BVH between sessions.

### The Cycles Principled BSDF differs from textbook "GGX + Schlick"

From `src/kernel/svm/closure.h`, `src/kernel/closure/bsdf_microfacet.h`,
`src/kernel/closure/bsdf_util.h`, and `src/scene/shader_nodes.cpp`:

| Term | Cycles Principled (inspected revision) | Consequence for the runtime family |
| --- | --- | --- |
| Diffuse | Lambert when `Diffuse Roughness` ≈ 0 (default 0.0); Oren-Nayar otherwise | Lambert; references pin roughness 0 |
| Layering | Diffuse weight scaled by `1 − directional albedo` of the specular layer (`closure_layering_weight`) | Runtime needs a directional-albedo table (shares the split-sum precompute) or accepts a documented tolerance |
| Specular distribution | GGX; **default `multi_ggx`** (energy preserving) | Either implement multi-scatter compensation or render references with `ggx` |
| Visibility | Height-correlated Smith, `1 / (1 + Λ(i) + Λ(o))` | Use the same form |
| Roughness mapping | `alpha = roughness²` | Same |
| Dielectric Fresnel | Real dielectric Fresnel remapped to F0…F90 (negative-exponent case), IOR 1.5 default (F0 = 0.04) | Schlick differs at grazing angles: implement the remap or give Fresnel its own tolerance |
| Metallic Fresnel | F82-tint model; reduces to Schlick with F0 = base color when specular tint is white | Schlick with a white tint is exact; tinted metals are out of v1 scope |

The PBR reference fixtures therefore record Principled inputs explicitly
(distribution, IOR, `Specular IOR Level` 0.5, `Diffuse Roughness` 0, white
specular tint). Each BRDF term gets its own tolerance.

### VTF can already carry BC4/BC5, but not BC6H/BC7

- `public/bitmap/imageformat.h` defines `IMAGE_FORMAT_ATI2N` (BC5, two-channel
  normal maps) and `IMAGE_FORMAT_ATI1N` (BC4). There is no BC6H or BC7 format, so
  either would be a versioned VTF format change.
- ATI2N is decoded by `vtf/s3tc_decode.cpp` and `bitmap/colorconversion.cpp`,
  and mapped by the D3D9/DXVK provider (`materialsystem/shaderapidx9/colorformatdx8.cpp`,
  `dxvk_imageformat.cpp`).
- **Gaps:** the native Vulkan provider maps only DXT1/DXT3/DXT5 to BC1–BC3
  (`shaderapivulkan.cpp`). The Hammer core VTF reader (`hammer/core/formats/vtf_image.cpp`)
  handles DXT only. **Encoding to ATI1N/ATI2N is disabled** (`ConvertToATIxN`
  is `#if 0`, backed by a Windows-only prebuilt ATI compressor). Authoring BC5
  normal maps on Linux needs a new encoder (for example a pinned open-source
  BC encoder) before the PBR family can use BC5.
- `TEXTUREFLAGS_SRGB` exists in `public/vtf/vtf.h`. How it is honored per
  provider must be verified before relying on it for slot classification.

### Denoising determinism is undocumented

The inspected OIDN revision documents quality modes (`OIDN_QUALITY_FAST`/
`BALANCED`/`HIGH`) but no determinism or bit-reproducibility guarantee. Until
it is measured, the `Exact` CPU bake profile runs **without** denoising, and
denoised output is `Statistical`.

### Cycles standalone renders USD without Blender

With `WITH_USD`, the standalone app loads non-XML scene files through
`HdCyclesFileReader` (`src/app/cycles_standalone.cpp`), and Cycles ships a Hydra
render delegate (`src/hydra`, requiring USD 25.11 or newer). The Phase G
interchange oracle can therefore render the exported USD with pinned Cycles
itself. Blender remains an optional human-facing check and is not part of the
automated pipeline.

### Dependency weight

The Cycles CMake build requires zlib, Threads, OpenImageIO, OpenEXR,
pugixml (XML reader), Embree, TBB, Open Image Denoise, and zstd, plus optional OSL,
OpenColorIO, OpenVDB/NanoVDB, OpenSubdiv, Alembic, USD, and GPU SDKs
(`src/cmake/external_libs.cmake`). The upstream route is Blender's
precompiled library repositories, which are git submodules
(`lib/linux_x64`, `lib/macos_arm64`, ...) fetched by `make update` over Git LFS.
To satisfy the pinned-dependency rule, the provider's profile pins those
submodule revisions (or builds the minimal set with OSL, USD, VDB, Alembic, and
OpenSubdiv disabled). It records them, and never fetches them at build time from
a moving branch. The dependency is limited to the bake tool on desktop host
profiles and is never linked into engine products.

### OpenUSD license

OpenUSD is distributed under the Tomorrow Open Source Technology License 1.0,
which the repository describes as Apache 2.0 with a different Section 6
(Trademarks). It is compatible with a tool-only exporter dependency. The license
record is kept with the pin.

## Design principles

- **Canonical outputs, derived legacy.** Bakers write canonical lighting (RFC 0008
  lighting layer). Legacy lumps are a derived compatibility payload, never a
  second source of truth. The one exception is the vrad provider's native
  legacy output, which exists for the byte-identity gate and the oracle.
- **Parity and quality are separate, declared modes.** A physically based bake
  changes how existing maps look. It is selected explicitly per map or compile
  profile and recorded in the BSP, never substituted silently for vrad.
- **One owner per fact.** Lightmap sample placement is owned by the World
  Stage's lightmap charts (RFC 0008) and read through one `LightmapSampleLayout`.
  Units and radiometric conversion are owned by one lighting policy module.
  Encodings are owned by RFC 0008's writers. Solvers own none of these.
- **The World Stage is the bake input.** `LightBakeScene` is a typed, immutable
  view built from the OpenUSD stage's geometry layer through the generated
  schema API (RFC 0008 USD decision). Bakers never parse BSP faces or VMF, and
  never write geometry. The Cycles provider builds its Cycles scene from
  `LightBakeScene` rather than through the Hydra delegate, because baking needs
  control of the single bake target and the sample table. Reference rendering
  (Phase G) uses Cycles' own USD loading.
- **New material families negotiate capability.** A renderer that cannot render
  the PBR family does not claim it. Materials declare a fallback, which is
  validated.

## Proposed architecture

```
 Hammer core (headless)
   PrepareForCompile ──> VMF
                          │
                          ▼
                   vbsp2 (Waf port, split) ──> gameplay lumps (planes, nodes, leafs, brushes,
                          │                     portals, phys, entities)  [RFC 0008 BSP2]
                          │                ──> World Stage geometry layer + lightmap charts
                          ▼                                                 [RFC 0008]
                   vvis (Waf port, job graph) ──> visibility
                          │
                          ▼
              LightBakeScene (typed, immutable view of the World Stage)
                          │ + LightingPolicy
                          ▼
                 ILightBaker (contract + shared suite)
                ├─ LegacyRadiosityBaker   (vrad solver; parity, oracle, legacy-only profile)
                ├─ CyclesBaker            (quality; CPU and GPU profiles)
                └─ test fakes / deliberately bad providers
                          │ BakeResult (typed, linear, unencoded)
                          ▼
         RFC 0008 writers: lighting layer (SH L1 atlas per style, exact RNM, probe volume,
                           HDR reflection probes) ──> packer ──> BSP2 + KTX2
                                                  └─> LegacyLightingExporter (optional payload)
                          │
                          ▼
   engine: native Vulkan (BSP2 world path; legacy families + PBR family)
           D3D9/DXVK and legacy shaders (via derived legacy payload)
```

### Components and owners

| Component | Owner / location (proposed) | Responsibility |
| --- | --- | --- |
| `LightmapSampleLayout` | `utils/lighting/` (new strict target) | Reads the World Stage's lightmap charts (RFC 0008). Enumerates each atlas texel covered by a chart, with world position, geometric and smoothed normal, tangent frame, footprint, and the owning triangle/barycentrics. The single source of sample placement for every baker and test. |
| `LightingPolicy` | `utils/lighting/` | Units (inches, `metersPerUnit = 0.0254`), light entity → radiometric conversion, falloff model, exposure and overbright conventions, and the π factor of Cycles diffuse passes. Versioned. Encodings are not here; they belong to RFC 0008. |
| `LightBakeScene` | `utils/lighting/` | Immutable input built from the stage: triangles with material bindings, lightmap samples, vertex samples, probe positions, reflection probe positions, lights (by style), sky, texture lights, visibility clusters, bake settings. |
| `ILightBaker` | `public/lighting/light_baker.h` | Contract: `Bake(const LightBakeScene&, const BakeRequest&, IBakeProgress&) -> Expected<BakeResult, BakeError>`; cancellation, progress, determinism declaration. |
| `LegacyRadiosityBaker` | `utils/vrad/` | Existing solver adapted to the contract. Parity mode, oracle, and the legacy-only compile profile. Its native legacy-lump output is kept for the byte-identity gate. |
| `CyclesBaker` | `utils/lighting/cycles/` | Private Cycles dependency; builds a Cycles scene from `LightBakeScene`, runs bake and probe renders, returns linear results. |
| Lighting-layer writer, `LegacyLightingExporter` | RFC 0008 | Encode `BakeResult` into canonical formats and, on request, the derived legacy payload. |
| PBR shader family | `materialsystem/shaderapivulkan/` + material system shader DLL | Runtime PBR material; declared capability; fallback. |
| Runtime cubemap prefilter | native Vulkan backend | GGX-prefiltered mips for **legacy** maps' `env_cubemap` VTFs; cached by content hash. |

`utils/lighting/` is a new strict C++20 target (RFC 0006) with no Windows, MFC,
or VPC dependencies. The legacy vrad executable continues to exist during
migration and is rebuilt on top of these components (see [Phase B](#phase-b-compile-tools-on-waf-extract-scene-and-writer-from-vrad)).

## The bake contract

### Inputs

`LightBakeScene` is built once from the World Stage geometry layer (after vbsp2
and vvis) plus game content (materials, models). It is immutable during a bake.
Every sample the bake must produce is enumerated in the input:

- **Lightmap samples:** for each covered atlas texel and each light style that
  reaches its chart: position, geometric normal, smoothed normal (smoothing
  groups), tangent frame, and footprint (for supersampling and antialiasing).
  Displacements are charted like any other surface.
- **Vertex samples:** static prop vertices (per LOD as vrad does) and detail
  prop placements, for props that are not lightmapped.
- **Probe samples:** probe grid positions, including relocation offsets
  (structure owned by RFC 0008 `PRBV`, semantics by RFC 0011).
- **Reflection probe samples:** positions from `env_cubemap` entities and their
  resolution.
- **Emitters:** `light`, `light_spot`, `light_environment` (sun and sky
  ambient, `-softsun` angle), `light_dynamic` excluded from bake, texture lights
  from `lights.rad` and material emission, with light style assignments and the
  worldlight records the runtime also receives.
- **Surfaces:** triangles with material bindings. Bounce albedo comes from the
  material: flat `dtexdata_t::reflectivity` in parity mode, per-texel albedo
  in quality mode. Alpha-tested and translucent shadowing
  (`-textureshadows`) is also needed.
- **Settings:** mode (parity or quality), sample budget, bounce limit, LDR/HDR
  outputs requested, denoise on/off, seed.

### Outputs

`BakeResult` holds **linear, unencoded** radiometric values in the
`LightingPolicy` units:

- per lightmap sample, per style: flat irradiance and the three RNM
  basis-projected irradiances (exact, for the legacy payload), plus the SH L1
  irradiance coefficients fitted from those four directional irradiances (the
  canonical data; see [RNM and SH L1 from one bake](#rnm-and-sh-l1-from-one-bake));
- per vertex sample: irradiance (and bumped basis for props that use it);
- per lightmap sample, per style *(amended by RFC 0011)*: the direct and
  indirect irradiance separately, whose sum is the total above. The
  `RuntimeIndirect` policy consumes the direct layer alone;
- per probe, per style *(amended by RFC 0011)*: irradiance in the
  `render.probe-volume.v1` encoding and visibility (mean and mean-squared hit
  distance per octahedral direction). SH L2 coefficients may be kept as a
  bake-side intermediate;
- optionally *(RFC 0011 G4)*: the radiosity transfer precompute for `RTRN`
  (surface patches, sparse visibility-weighted form factors, probe gather
  weights, and per-patch visibility of each baked light), computed with the
  same ray tracer and scene as the lightmap;
- per reflection probe: HDR radiance cube faces (prefiltering belongs to the
  RFC 0008 writer);
- diagnostics: sample counts, variance/noise estimate, invalid samples
  (inside solid, backfacing), timing.

Only RFC 0008's writers quantize, encode, and place data. Every provider's
results are encoded identically, which makes provider comparison meaningful.

### Obligations (LSP)

Every provider, including fakes, passes one shared suite:

- produces exactly one value for every enumerated sample, with no extras;
- honors light styles (a style-0-only bake is not a valid provider);
- cancellation returns promptly with `BakeError::Cancelled` and no partial
  result; progress is monotonic and callable from another thread (preserves the
  `IVRadDLL::Interrupt`/`GetPercentComplete` semantics Hammer relies on);
- declares determinism: `Exact` (same input, seed, and profile gives
  bit-identical output) or `Statistical` (bounded variance, tested with
  tolerances). CPU providers must be `Exact` per profile;
- obeys ownership: no retained references to the scene after `Bake` returns;
  no global state; independent concurrent bakes in one process are allowed or
  explicitly rejected by capability.

Deliberately bad providers (drops a style, swaps two basis vectors, offsets
luxels by half a luxel, returns gamma-encoded values, ignores cancellation)
must each fail the suite.

## Cycles provider

### Integration shape

The provider links **Cycles standalone** as a private library. It never uses
Blender. It translates `LightBakeScene` into a Cycles scene: meshes, shader
graphs for material albedo, alpha, and emission, lights, and a background for
sky ambient. It then runs bake passes whose per-pixel inputs are our enumerated
samples.

Cycles baking evaluates lighting at surface points given as primitive IDs and
barycentric coordinates. The provider supplies these from `LightmapSampleLayout`
directly, instead of letting any UV unwrap or rasterization choose sample
points. Source inspection shows the host API supports this
(`OutputDriver::read_render_tile`; see [findings](#cycles-accepts-externally-supplied-bake-points-answers-phase-cs-main-question-in-principle)).
The Phase C spike must demonstrate it on a built, pinned revision. Because of
the single-bake-target rule, the provider merges each bake's receiving geometry
into one target mesh and keeps a primitive-index map back to `LightmapSampleLayout`
samples.

### Radiosity normal mapping bases

Source bumped lightmaps store, per luxel, the irradiance weighted by a clamped
cosine lobe around each of three tangent-space basis vectors
(`public/mathlib/bumpvects.h`), plus flat irradiance. Candidate approaches, in order of
preference:

1. **Normal-override bakes:** bake diffuse irradiance four times with the
   shading normal forced to the flat normal and each world-space basis
   direction. This needs no kernel changes. The shader normal is per shader, not
   per pixel, so this is four bake sessions per light style with the BVH reused.
   Risks: hemisphere clipping against the geometric normal, shadow-terminator
   offsets, four times the cost.
2. **Single pass with a custom output pass** that accumulates all four
   projections from each path's first-bounce direction. It is faster, but it
   is a Cycles kernel patch that must be carried or upstreamed.

The chosen approach must pass the basis oracle: a single distant light placed
exactly along each basis vector produces the analytically expected four values.

### RNM and SH L1 from one bake

The canonical lightmap (RFC 0008 `LMAP`) stores SH L1 irradiance, and the
legacy payload stores RNM. Both come from the same four directional irradiance
values per sample, so no second bake is needed:

- RNM: the three basis irradiances and the flat irradiance are written
  unchanged (exact).
- SH L1: irradiance for a normal `n` under an L1 radiance field is
  `E(n) = c0·L0 + c1·(L1·n)`. The flat normal and the three basis directions
  are four non-coplanar directions, so the four L1 coefficients per channel
  follow from a 4×4 solve that is fixed per tangent frame.

The fit ignores the radiance field's higher bands, which the clamped cosine
partially passes. The SH oracle bounds that error: analytic environments
(a single distant light at varying angles, and a uniform sky) compare the
fitted `E(n)` with the exact clamped-cosine irradiance for normals across the
hemisphere, with a declared tolerance. If the error is too large, the provider
bakes additional directions and switches to a least-squares fit. That changes
cost, not format.

### Units and falloff

`LightingPolicy` converts Source light entities into Cycles emitters: `_light`
color and brightness, `_constant_attn`/`_linear_attn`/`_quadratic_attn`,
`_fifty_percent_distance`/`_zero_percent_distance`, and spot cone angles and
exponents. Some Source falloff models are not physical. The policy defines,
per mode:

- **parity-oriented quality mode (default for existing maps):** emitter
  intensities and falloff reproduce vrad's direct-light values on analytic
  cases. Only the transport (bounces, shadows, sky sampling) improves;
- **physical mode (opt-in):** inverse-square point, spot, and area lights in
  physical units. Non-physical attenuation keys are rejected with a diagnostic
  instead of being approximated silently.

The calibration is a test, not a constant copied into code: a point light over
a plane at known distances must produce vrad's analytic direct-light value
within tolerance in parity-oriented mode.

### Other features

| Feature | Cycles realization | Notes |
| --- | --- | --- |
| Sky ambient / sun | Background shader + sun light with angular diameter | `light_environment` pitch/angles; skybox visibility (`-noskyboxrecurse` semantics) |
| Texture lights | Emission shader on faces from `lights.rad` / material | Parity with vrad's texlight chopping is statistical |
| Alpha shadows | Transparent BSDF from base texture alpha | Matches `-textureshadows` scope; per-material opt-in preserved |
| Displacements | Displacement triangles charted in the atlas like other surfaces | Seam continuity oracle across displacement/brush boundaries |
| Light styles | One bake per style with other styles' emitters disabled | At most four styles per chart, matching `MAXLIGHTMAPS`, so the legacy payload is always derivable |
| Probe volume (RFC 0011 encoding) | Low-resolution panoramic radiance and depth renders at each probe (`PANORAMA_EQUIRECTANGULAR` camera in `src/kernel/types.h`), convolved to octahedral irradiance; hit distances give the visibility moments | No proxy geometry is needed, because probes are camera renders, not surface bakes. Legacy ambient cubes are derived from the volume by RFC 0008's exporter |
| Reflection probes | Six 90° cube-face renders (or `PANORAMA_EQUIANGULAR_CUBEMAP_FACE`) in HDR at each `env_cubemap` | Replaces in-game `buildcubemaps`; prefiltering by the RFC 0008 writer |
| Static prop lighting | Vertex bake points (barycentric corners are nudged inward by the kernel) | `-StaticPropLighting`, `-StaticPropPolys`, texture shadows; props are merged into the bake target or baked in separate sessions |
| Denoising | Open Image Denoise on lightmap atlases with albedo/normal guides | Per-face borders must be denoised without bleeding across unrelated faces; optional and recorded; excluded from the `Exact` profile until determinism is measured |
| Visibility culling | Not needed for correctness | vvis PVS remains an input for vrad parity and for probe placement |

## vvis track

vvis is ported to Waf and to the RFC 0003 job graph as an independent track.
Its oracle is exact: for the corpus, the ported vvis (serial graph and parallel
graph) must produce byte-identical `LUMP_VISIBILITY` to the legacy executable
built from the same revision. Portal flow per portal is independent, which makes
this a natural graph cohort. Any change that alters the PVS, such as a faster
approximate mode, is a separately versioned opt-in mode.

## PBR material family

### Family definition

A new shader family named `PBRMetalRough` (distinct from the existing D3D9
`PBR` shader) with VMT parameters:

| Parameter | Meaning | Encoding |
| --- | --- | --- |
| `$basetexture` | Base color / albedo | sRGB |
| `$mraotexture` | R: metalness, G: roughness, B: ambient occlusion | Linear |
| `$bumpmap` | Tangent-space normal map | Linear; KTX2 transcoded per profile (BC5 / ASTC / EAC RG11, RFC 0008) |
| `$emissiontexture`, `$emissionscale` | Emission | sRGB texture, linear scale |
| `$alphatest`, `$alphatestreference`, `$translucent` | As legacy | Unchanged semantics |
| `$envmap` | `env_cubemap` or explicit cubemap | Consumed through IBL prefilter |
| `$fallbackmaterial` | Path to a legacy VMT used when a provider lacks the capability | Required material reference |

The VMT schema, the parameter defaults, and the sRGB/linear classification of
each slot have one owner: a versioned schema to be consumed by the material
system, the Hammer `MaterialCatalog`, the baker (for albedo), and the exporters.
The first schema, editor-side fallback validator, and temporary runtime shader
registration are recorded in [progress](0007-progress.md). The temporary
fallback path has end-to-end VMT and pixel checks on DXVK and native Vulkan.
Both loaders now reject missing required fields, absent or self fallbacks,
traversal references, unsupported or PBR fallback shaders, and a patch cycle.
Fallback patch-include path validation and malformed VMT syntax still need
full runtime coverage.
New PBR textures are KTX2
(RFC 0008), which covers BC7, BC6H, and ASTC. VTF lacks BC6H/BC7 and its BC5
encoder is unavailable on Linux (see
[findings](#vtf-can-already-carry-bc4bc5-but-not-bc6hbc7)). PBR materials may
still reference legacy VTFs, which load unchanged.

### Shading model

The model tracks the Cycles Principled BSDF as inspected (see
[findings](#the-cycles-principled-bsdf-differs-from-textbook-ggx--schlick)):

- Diffuse: Lambertian (Principled `Diffuse Roughness` 0), weighted by
  `1 − E_spec(μ)`, the specular layer's directional albedo, as Cycles'
  layering does. `E_spec` comes from the same precomputed table as the split-sum
  BRDF term.
- Specular: GGX with `alpha = roughness²` and height-correlated Smith
  visibility. Dielectrics use F0 = 0.04 (IOR 1.5). v1 uses Schlick Fresnel with a
  per-term tolerance against Cycles' remapped dielectric Fresnel; implementing
  the remap is a measured follow-up. Metals use Schlick with F0 = base color,
  which is exact against Cycles' F82-tint model with a white tint.
- Multi-scattering energy compensation is a declared option recorded in the
  capability. References are rendered with both `ggx` (exact-model check) and
  `multi_ggx` (Blender default, tolerance check).
- Static lighting: on BSP2 maps, evaluate the SH L1 lightmap (per style layer)
  at the normal-mapped normal. On legacy maps, reconstruct irradiance from the
  RNM basis exactly as `LightmappedGeneric` does.
- Dynamic objects: the RFC 0011 probe volume on BSP2 maps, ambient cube on legacy
  maps, plus dynamic lights through the clustered path (RFC 0008), evaluated
  with the same BRDF.
- Specular environment: split-sum IBL from the blended, parallax-corrected
  reflection probes (BSP2) or runtime-prefiltered `env_cubemap` (legacy),
  attenuated by specular occlusion derived from AO.
- Output: linear HDR into the existing exposure/tonemap chain.

### Capability and fallback

The PBR family is a render capability (for example `material.pbr.v1`) declared
by providers that implement it at tested fidelity. Initially that is the native
Vulkan backend only. The D3D9/DXVK compatibility provider does not claim it, and
PBR materials render through their validated fallback. A provider may add
support later only by passing the same PBR suite. Materials whose fallback is
missing or invalid fail content validation, not rendering.

### Image-based lighting

Two sources, one shading path:

1. **Baked reflection probes (BSP2 maps):** the Cycles provider renders HDR
   radiance cubes at `env_cubemap` positions. The RFC 0008 writer prefilters
   them into GGX roughness mips and stores them as KTX2 in `RPRB`, with
   influence and parallax boxes. Nothing is computed at load. This removes the
   dependency on `buildcubemaps` running the game renderer correctly.
2. **Runtime prefilter (legacy maps, no content change):** at map load, the
   native backend prefilters each legacy `env_cubemap` VTF with a compute pass.
   Results are cached by source hash and filter version.

The BRDF integration and directional-albedo table is computed once per filter
version and shared by both.

**Amendment (2026-09-25, R50-RELIGHT; RFC 0011 open decision 5): relightable
baked probes.** Without this, runtime light never reaches specular. The
lightmap and probe volume follow the runtime light set and the indirect
producers (RFC 0011), but a baked probe keeps reflecting the room as baked.
So a switched-on lamp lights a wall and not the wall's reflection.

- **Data.** A baked probe may carry relight bands (`RPRB` v2), a G-buffer of
  what its capture saw, rendered in the same Cycles faces:
  - the diffuse albedo (the Diffuse Color pass);
  - the ray distance (the Depth pass the proxy fit already uses);
  - the world normal (the Normal pass);
  - all in the radiance chain's mip layout, box-filtered.
- **Shading.** At a probe lookup the shader reconstructs the point the
  capture saw and adds albedo times the scene's diffuse-light change since
  the bake at that point. The point is capture + direction × distance, with
  the band's normal. The change is whatever the world itself adds in that
  shader variant: the producer's change volume and the unbaked lights'
  SDF-shadowed direct light. It is exact in the baked state and for a mirror
  reflection of a Lambertian surface.
- **Source and deviations.** McAuley, "Rendering the World of Far Cry 4"
  (GDC 2015), relights an albedo and normal cubemap from the sun, sky and
  probes, then prefilters it at runtime. It has no shadows, because it
  stores no depth.
  - Here the stored distance lets the relit point be shadowed and lit by
    point lights.
  - The change is added at lookup time rather than re-prefiltered, so a
    rough lookup takes the change at the lobe's centre (a documented
    approximation).
- **Scope.** The glossy part of a surface seen in a probe stays as baked,
  and so does light a probe's own emitters or the sky change. Legacy
  `env_cubemap` probes have no G-buffer and are not relit.

### Platform tiers

Mobile and MoltenVK profiles declare a PBR quality tier: fewer prefiltered mips,
no multi-scatter compensation, possibly per-vertex probe evaluation.
Budgets for power, thermal, and frame time are set before optimization and
measured on the R29/R36 device runners. A tier that changes appearance is a
declared variant with its own tolerances, not a silent degradation.

## Interchange and reference rendering

RFC 0008 owns interchange: the World Stage *is* the open-format description of
the compiled world, and glTF is its per-asset export. This RFC adds one
consumer: the Cycles reference renderer. It renders the stage (geometry,
materials, lights, and, for runtime comparison, the baked lighting layer) with
the pinned Cycles standalone to produce PBR oracle images. Opening the stage in
Blender is a manual, non-gating check.

## Hammer integration

- **Compile/run:** the GTK host's compile workflow (RFC 0002 H5) runs the RFC
  0008 build graph (`PrepareForCompile` → vbsp2 → vvis → bake → pack) as
  separate tool processes with structured argv and cancellation (RFC 0001
  retirement E / R40). Unchanged steps are cache hits. The bake provider and
  mode come from the map's compile profile and are recorded in the lighting
  layer and the BSP2 package manifest.
- **Lighting preview:** a progressive, cancellable bake of the current document
  replaces `IVRadDLL::DoIncrementalLight` for the new host. The legacy
  `IVRadDLL` ABI stays for legacy Hammer until its retirement gate (R43).
- **Export:** "Export scene" writes the World Stage (RFC 0008) and needs no
  lighting bake.

## Validation and acceptance criteria

Harness families from RFC 0005: Q-CONTENT (lumps, exporters, corpus),
Q-PRESENTATION (PBR pixels, IBL), Q-JOBS (vvis graph), Q-PRODUCT (compile
workflow). Required runs fail on zero samples, missing corpus assets, missing
providers, or incomplete output.

### Structural and format

- RFC 0008's independent readers validate every canonical output (atlas
  coverage, style layers, probe counts, KTX2 validity). For the legacy payload,
  an independent BSP reader checks sizes, per-face offsets,
  `(w+1)×(h+1)` luxels × bump factor × styles, HDR/LDR pairs, and index lumps.
- Byte-identical round trip: `LegacyRadiosityBaker` through the new scene
  builder and lump writer equals the legacy vrad executable's output for the
  corpus at the same revision and flags. This is the Phase B gate.

### Analytic oracles

- Point light over a plane: direct irradiance at known luxels.
- Furnace: a closed box of uniform albedo ρ and uniform emission converges to
  the analytic infinite-bounce radiosity. This checks bounce energy.
- Basis oracle: a distant light exactly along each RNM basis vector produces the
  expected flat and basis values.
- SH oracle: fitted SH L1 irradiance vs exact clamped-cosine irradiance over the
  hemisphere for analytic environments. Probe-volume irradiance (RFC 0011
  encoding) vs analytic irradiance of a single distant light and a uniform sky.
- Probe visibility (RFC 0011): the `thin-wall` fixture's dark room stays below
  the leak bound, and hit-distance moments match the analytic distances of the
  `probe-grid` fixture.
- Direct/indirect separation (RFC 0011): per texel, direct plus indirect equals
  the total within the noise band, and the furnace's indirect layer matches the
  analytic bounce energy.
- Luxel registration: a light with a sharp shadow edge at a known world
  coordinate lands in the analytically expected luxel.
- BRDF: GGX/Smith/Fresnel values against an independent reference
  implementation, white-furnace energy test, and reciprocity.

### Comparative oracles

- **Cycles vs vrad (statistical):** per-face mean luminance ratio within a
  declared band in parity-oriented mode on the corpus, plus a seam metric
  (discontinuity across shared edges) that must not regress. Identical values
  are neither expected nor required.
- **PBR runtime vs Cycles reference:** small scenes rendered offline by Cycles
  (versioned fixtures with the Cycles revision recorded) compared to native
  Vulkan PBR frames lit by the same baked data. Per-term tolerances (diffuse,
  specular IBL, direct light) apply, and structural checks come before
  metrics. This becomes a new `pbr` family in the material pixel harness.

### Negative fixtures

Each must fail its oracle: a half-luxel offset, swapped basis vectors, a missing
style, gamma-encoded output, double-applied overbright, a BRDF without Fresnel,
an unnormalized GGX distribution, prefiltering with the wrong roughness mapping,
and a PBR material without fallback on a non-PBR provider. RFC 0011 adds:
swapped direct and indirect layers, probes without visibility, and a transfer
matrix whose per-patch sums exceed one.

### Product

- Quality-baked corpus maps render on native Vulkan through the BSP2 world path,
  and through the derived legacy payload on the D3D9/DXVK provider.
- Compile from the GTK host produces a runnable map, and cancellation leaves
  no partial BSP.
- Bake time, memory, and GPU/CPU profile are recorded per corpus map. Budgets
  are set before optimization.

## Delivery plan

Phases are bounded slices with their own gates. Phases A and D do not depend on
the compile-tool port and can start first.

### Phase A: PBR shading core with synthetic lighting

BRDF library with analytic tests. PBR family on native Vulkan lit by synthetic
SH L1 and RNM lightmaps, synthetic probe volumes, and a test reflection probe. The `pbr` pixel harness family
with Cycles-rendered reference fixtures and negative controls. Capability
declaration and fallback validation.
*Gate:* analytic BRDF tests, pixel family vs Cycles fixtures, negative controls
fail, fallback path renders on the D3D9/DXVK provider.

### Phase B: Compile tools on Waf; extract scene and writer from vrad

Port vbsp, vvis, and vrad to Waf on `linux-headless-core`. Record baseline
outputs and timings for the corpus (`room.vmf` plus declared Portal 2 fixtures).
Extract vrad's scene gathering, policy, and lump writing, and put the vrad
solver behind `ILightBaker`. The World Stage emitter and lightmap charts are
RFC 0008 F2, built alongside this phase. Until they exist, the vrad provider
runs on a `LightBakeScene` built from legacy BSP faces.
*Gate:* byte-identical legacy lumps versus the legacy executable; the shared
provider suite passes for the legacy provider and fails for the bad providers.

### Phase C: Cycles feasibility spike

Pin Cycles standalone (and OIDN). Bake flat direct lighting only (point, spot,
sun) into the lightmap charts of one corpus map (RFC 0008 F2), then encode
through the shared writer.
*Gate and decision:* external bake points demonstrated on the built revision
(source inspection says the API supports it), the luxel registration and
point-light oracles pass, seams measured, and CPU/GPU timings recorded. The
recorded decision is Cycles-as-library, a patch, or the Embree alternative.

### Phase D: Units, color, and material schema owners

`LightingPolicy` versioned and shared by baker, runtime, and writers. The PBR
VMT schema and sRGB/linear slot classification (parameter semantics and
interchange mapping owned by RFC 0008), consumed by the material system and
Hammer `MaterialCatalog`.
*Gate:* one owner. Tests show each consumer routes through it, and a seeded
duplicate conversion constant is detected.

### Phase E: Full quality bake

Bounces, sky, texture lights, RNM bases and the SH L1 fit, styles,
displacements, alpha shadows, probe volume with visibility and separated
direct/indirect layers (RFC 0011), reflection probe renders,
static and detail prop lighting, and optional denoising.
*Gate:* all analytic (including SH) and negative oracles, the statistical
comparison with vrad, and runtime rendering on the BSP2 path and through the
legacy payload.

### Phase F: Image-based lighting

BRDF and directional-albedo tables. Parallax-corrected, blended reflection
probes from `RPRB`, and the runtime prefilter for legacy maps with caching.
Relightable baked probes (the 2026-09-25 amendment above).
*Gate:* IBL pixel fixtures against Cycles, cache invalidation tests, and
existing maps unchanged on legacy families. For relighting: a mirror floor
reflecting an unbaked light matches Cycles, and the probes as baked fail.

### Phase G: Stage reference rendering

Pinned Cycles standalone renders World Stages for the PBR oracle. Corpus scenes
compare the engine's frames with the Cycles reference within per-term
tolerances.
*Gate:* reference fixtures versioned with Cycles revision and stage hashes; a
seeded material-mapping error is detected.

### Phase H: Hammer workflow and platforms

GTK compile/run and progressive preview. Bake profiles on declared platforms
(bake is a desktop/tool activity; the runtime PBR family follows R29/R36 for
mobile).
*Gate:* Q-PRODUCT compile workflow with cancellation and recovery; PBR tier
budgets on device runners where R29 provides them.

### vvis track

Waf port (with Phase B), then the job-graph flow with serial/parallel/legacy
byte equivalence.

### Program sizing

Rough, for one engineer familiar with the codebase, excluding waiting on
dependencies: A 3–5 weeks, B 4–8 weeks, C 3–4 weeks, D 2–3 weeks,
E 2–3 months, F 4–6 weeks, G 2–3 weeks, H depends on R25/R29. The vvis
track takes 4–8 weeks. RFC 0008's format work is sized separately. The largest uncertainty is Phase C, and E's estimate is
not meaningful until C's decision is recorded.

## Roadmap

Tracked in the AGENTS.md ranked roadmap (added 2026-09-22): A/D → R47,
B and the vvis port → R48, C/E → R49, F → R50, G → R51, H and the vvis job
graph → R52. AGENTS.md owns their ranks and states. The rows are ranked after
R32, so legacy-content fidelity on native Vulkan stays ahead.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Standalone Cycles bake path fails in practice despite the API (single target, precision) | Phase C demonstrates on a built revision before investment; patch or Embree alternative behind the same contract |
| Cycles dependency stack (OIIO, OpenEXR, Embree, TBB, OIDN, ...) is heavy to pin | Pin Blender precompiled library submodule revisions or build a minimal set; tool-only, desktop host profiles |
| Quality bakes change the look of existing maps | Parity-oriented default calibration; explicit per-map mode recorded in BSP; vrad provider retained |
| Luxel misregistration produces seams | `LightmapSampleLayout` single owner; registration and seam oracles; negative half-luxel fixture |
| SH L1 fit from four directions loses accuracy | SH oracle with declared tolerance; more directions plus least squares if needed (cost, not format) |
| Denoiser bleeds across unrelated faces in atlas | Per-face padding and guides; seam metric; denoise optional and recorded |
| GPU bake is not reproducible | CPU `Exact` profile is authoritative for releases; GPU is declared `Statistical` |
| PBR content breaks on D3D9/DXVK or mobile | Capability negotiation; validated fallback; declared mobile tier |
| Units/color drift between baker, runtime, and exporters | One `LightingPolicy`; seeded-duplicate detection test |
| Cycles upstream churn | Pinned revision; provider-private dependency; upstream patches minimized and recorded |
| Runtime GI work obscures the baker gate | Keep bake acceptance independent; RFC 0008 F10 owns runtime quality and performance gates |

## Alternatives considered

### Drive Blender itself as the baker

This is the fastest way to prototype: export USD, bake with Blender's Python
API headlessly, and read the images back. It is rejected as the product path.
It makes a Blender installation a compile dependency, Blender's bake pipeline
chooses sample placement through UV rasterization instead of our luxels, and
automation depends on Blender's Python API across versions. The automated
Phase G reference is the pinned Cycles standalone reading USD. Blender remains
a manual cross-check for artists.

### Direct Embree (+ OIDN) tracer

A diffuse-only path tracer for lightmaps is comparatively small and gives full
control over sample points, RNM projection, and probes. It lacks Cycles' GPU
backends, light-sampling machinery, and a shader graph that serves as the
Principled BSDF reference. It is a first-class fallback if Phase C finds the
Cycles bake API unsuitable, and uses the same contract, writer, and oracles.

### Vulkan ray-query baker on the native backend

This aligns with the Vulkan north star and reuses the device work. It requires
building sampling, BVH, and denoising quality that Cycles already has, and it
makes baking depend on RT-capable GPUs. It could become a later provider behind
`ILightBaker`, admitted by the same suite.

### Improve vrad in place

Better sampling and leak fixes inside the radiosity solver are possible and
cheaper per change. They do not address GPU acceleration, physically based
transport, or the PBR/interchange goals. Phase B's extraction also makes
isolated vrad improvements easier, so this is not exclusive.

### RNM only, SH later

Keeping RNM as the only lightmap basis would avoid a new format. Once the user
authorized format upgrades (RFC 0008), SH L1 became the canonical basis: better
directional quality for PBR, and a standard representation. RNM remains the
exact legacy payload, produced by the same bake at no extra cost.

## Open decisions and required evidence

1. Cycles-as-library vs patch vs Embree. *Narrowed:* the host bake API supports
   external sample points, and probes use camera renders, so no proxy geometry
   is needed. Phase C must still demonstrate it on a built revision, including
   the single-target merge.
2. RNM basis method. *Narrowed:* normal-override needs four sessions per style
   (the shader normal is per shader). A custom single-pass output is the
   kernel-patch alternative. Decided by the basis oracle and cost.
3. Default mode for existing maps: parity-oriented quality vs vrad. Needs
   artist review of corpus comparisons.
4. ~~PBR family name~~ *Resolved:* `PBRMetalRough` preserves the existing
   D3D9 `PBR` shader. VMT parameter names and fallback syntax remain open;
   they must be compatible with the material system and Hammer catalog.
5. ~~VTF block-compression support~~ *Resolved:* BC4/BC5 exist as
   `ATI1N`/`ATI2N`; BC6H/BC7 do not. *Remaining work:* ATI2N in native Vulkan
   and the Hammer VTF reader, plus a Linux BC5 encoder (the in-tree path is
   disabled and Windows-only).
6. ~~USD vs glTF first~~ *Moved to RFC 0008* (World Stage in USD, glTF as
   per-asset export; USD-in-compile-path is RFC 0008 open decision 6).
7. Whether quality mode uses per-texel albedo for bounces (physically better) or
   flat reflectivity (closer to vrad); corpus comparison.
8. ~~Probe placement~~ *Moved to RFC 0008* (probe volume structure, open
   decision 3).
12. Whether bakers need more than four directions for an acceptable SH L1 fit,
    from the SH oracle.
9. Runtime Fresnel for dielectrics: Schlick with tolerance (v1) vs Cycles'
   remapped dielectric Fresnel; multi-scatter compensation on or off by
   default. Decided from pixel-oracle error measured per term.
10. How to pin Cycles' dependency stack: Blender precompiled library
    submodule revisions vs a minimal self-built set; decided by build cost
    and profile reproducibility.
11. OIDN determinism on the CPU profile, which decides whether denoising may be
    part of an `Exact` bake.

## Source references

- `public/bspfile.h`: lump IDs, `texinfo_t`, `dface_t`, `MAXLIGHTMAPS`, `dcubemapsample_t`
- `public/mathlib/bumpvects.h`: RNM basis vectors
- `public/ivraddll.h`, `hammer/bsplighting.cpp`: legacy incremental lighting ABI and consumer
- `utils/vrad/`: `vrad.cpp` (options and pipeline), `lightmap.cpp`, `vismat.cpp`,
  `leaf_ambient_lighting.cpp`, `vradstaticprops.cpp`, `vraddetailprops.cpp`,
  `disp_vrad.cpp`, `vraddisps.cpp`, `trace.cpp`
- `utils/vbsp/`: `writebsp.cpp`, `cubemap.cpp`, `faces.cpp`, `disp_vbsp.cpp`
- `utils/vvis/`: `vvis.cpp`, `flow.cpp`
- `hammer/core/formats/map_export.cpp`, `material_catalog.cpp`, `vtf_image.cpp`
- `materialsystem/shaderapivulkan/`, `tools/quality/material_pixel_conformance.py`

## Proposed decision

Accepted for planning together with RFC 0008 on 2026-09-22 (roadmap rows
R47–R52). Recommended first work is
**Phase A** (PBR shading core with synthetic lighting and a Cycles-reference
pixel family) and **Phase B** (compile tools on Waf with the extracted bake seam
and byte-identical legacy provider). Neither changes shipped content or renderer
defaults. Commit to the
Cycles provider only after Phase C records its feasibility decision.
