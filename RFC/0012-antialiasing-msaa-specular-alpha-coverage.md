# RFC 0012: Antialiasing — 4x MSAA, Specular AA and Alpha to Coverage

- Status: Proposed (2026-09-24); no implementation gate complete
- Date: 2026-09-24
- Scope: The antialiasing policy of the native Vulkan renderer: the
  multisampled back buffer's target and memory policy (4x as the quality
  target), alpha to coverage for alpha-tested legacy and PBR materials,
  geometric specular antialiasing in the PBR family, offline normal-variance
  roughness and coverage-preserving alpha mips for new textures, and the
  per-profile default
- Render seams and platforms: [RFC 0001](0001-capability-based-platform-architecture.md)
  owns the native Vulkan backend (R28/R32), `render.sample-count.v1`, the
  Video options record (R32-VIDEO-OPTIONS P6) and platform profiles (R29/R36)
- Shading: [RFC 0007](0007-physically-based-lighting-pipeline.md) owns the
  `PBRMetalRough` family, its BRDF and its schema (R47); this RFC amends its
  shading model (see [Amendments](#amendments-to-rfc-0001-rfc-0007-and-rfc-0008))
- Textures: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) F3
  owns the KTX2 writer and reader (R55)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)

**Binding render rules (user decision, 2026-09-28).** All rendering, lighting and
material implementation for this RFC is bound by
[RFC 0016's binding rules](0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28).
The legacy render paths are frozen. New work lands on the render core,
is proven in `render_lab` before any integration, and deletes the old copy
in the change that replaces it. Where this RFC names
`materialsystem/shaderapivulkan/` or the engine's CPU lighting path as the
place for new work, read the render core instead. These rules are not
negotiable, and only the user can change them.
Performance acceptance follows RFC 0016's
[hard render budgets](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01)
(user decision, 2026-10-01, superseding the earlier nonblocking rule 7).
A miss requires optimization and prevents the affected performance acceptance;
it never permits cutting an effect or weakening quality.

## Render performance obligations (user decision, 2026-10-01)

This RFC follows RFC 0016's
[high-performance clustered lighting requirement](0016-render-core.md#high-performance-clustered-lighting-user-decision-2026-10-01)
and [hard render budgets](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01).
Computation placement follows RFC 0003's binding
[CPU/GPU policy](0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01).
Those owners define the policies and numbers; this RFC defines its domain's
obligations and does not certify implementation by this amendment.

High performance evidence uses the declared MSAA, coverage and specular-AA
policy and judges fine detail through motion. A single-sample image, reduced
normal detail or disabled coverage/shadows cannot stand in for the High frame
or satisfy its budget.

## Decision and boundary

The render core is the
[clustered Forward+ renderer](0016-render-core.md#rendering-architecture-clustered-forward).
Every material family, including the PBR world and model shaders, shades
forward into the graph's multisampled scene-color target. MSAA therefore
antialiases geometry edges without a material G-buffer. It can't fix the
aliasing that comes from shading inside a triangle or from discarded fragments.
This RFC completes antialiasing around MSAA rather than replacing it:

1. **4x MSAA is the quality target** where a profile's budgets allow it. The
   target and memory policy is explicit per profile and measured on the device,
   not assumed from a tiled-GPU model.
2. **Alpha to coverage** turns alpha-tested edges (fences, foliage, grates,
   signs) into coverage. Legacy materials keep D3D9's opt-in rule. The new PBR
   family uses it by default.
3. **Specular AA** widens GGX roughness by the screen-space variance of the
   shading normal. This removes the highlight shimmer from normal maps and
   curved glossy surfaces that MSAA shades once per pixel.
4. **Offline filtering** bakes normal-map variance into the roughness mips
   and keeps alpha-tested coverage stable across mips, for new KTX2 content.

Temporal implementation remains outside this RFC. The proposed
[temporal upscaling contract](0019-temporal-upscaling-contract.md) owns the
motion, jitter, history and portal requirements for a later opt-in mode. This
RFC must leave the MSAA path correct beside it (see
[Future temporal work](#future-temporal-work)).

Legacy shader families keep their D3D9-defined appearance. They gain alpha to
coverage only through the flag D3D9 already honours, and they don't gain
specular AA.

## Observed starting point

Observed at `980565cd` plus the dirty tree (2026-09-24), by reading source;
nothing here was measured for this RFC:

- **MSAA exists (R32-VIDEO-OPTIONS P6, done).** `ApplySampleCount` and
  `CreateMsaaTargets` (`materialsystem/shaderapivulkan/vulkan_device.cpp:6798`,
  `:6832`) build one multisampled color image and one depth/stencil image at
  the back buffer's size. Every pipeline family takes the pass's sample count
  (`PipelineKey`, bits 33–35). `mat_antialias` goes through
  `render::ClampSampleCount` (`public/render/render_sample_count.h`,
  `render.sample-count.v1`). Autoconfig recommends 4x on the desktop GPUs
  tested.
- **Memory and traffic.**
  - Both multisampled images come from plain `DEVICE_LOCAL` memory, not
    transient or lazily allocated.
  - Each attachment pass stores color, depth and stencil
    (`CreateAttachmentPass`, `vulkan_device.cpp:875`: `STORE` for all three).
  - At the Fold7's native 2448×1848, 4x color plus a 4-byte depth/stencil is
    at least 145 MB of device memory.
  - Resolution is a separate `vkCmdResolveImage` (`ResolveBackBuffer`,
    `vulkan_device.cpp:6993`). It runs before every read of the back buffer:
    target copies (`_rt_FullFrameFB`, refraction), scene captures, readback,
    the present blit and the gamma pass.
- **Mobile cost is unknown.** P6 recorded tile-based resolve cost as
  unmeasured. The frame-pacing record found that the Adreno 840 didn't save
  GPU time when render passes fell 31 → 5
  ([device result](0001-native-vulkan-frame-pacing-progress.md#device-result-galaxy-z-fold7-adreno-840-2026-09-23)).
  Bandwidth savings on that driver therefore can't be assumed. `frame_pacing.py`
  pins `mat_antialias 0`, so no MSAA frame has been measured on any profile.
- **Glass loses scene depth under MSAA.** `RecordSceneCapture`
  (`vulkan_scene_capture.cpp:325`) skips the depth copy when the back buffer
  is multisampled, so depth-aware refraction silently degrades at 4x.
- **Render targets are single-sampled** (P6 decision). Portal 1's portal
  views are stencil-drawn into the back buffer and are antialiased. Water and
  monitor render targets are not.
- **Alpha to coverage is a stub.** `CShaderShadowVulkan::EnableAlphaToCoverage`
  is `VK_UNIMPLEMENTED()` (`shaderapivulkan.cpp:4652`), and the dynamic
  `Enable/DisableAlphaToCoverage` are empty (`:2383`). The legacy request
  comes from `$allowalphatocoverage` (`MATERIAL_VAR_ALLOWALPHATOCOVERAGE`,
  `materialsystem/shaderlib/BaseShader.cpp:209`). D3D9 keeps it only when
  alpha test is on and alpha blend is off
  (`shaderapidx9/shadershadowdx8.cpp:1796`), and only with MSAA enabled on a
  capable device (`shaderapidx8.cpp:2294`).
- **The PBR family has no specular or alpha AA.**
  - Roughness is `max( mrao.g, 0.02 )` in `world_pbr.frag:220`,
    `model_pbr.frag:199` and `world_pbr_glass.frag:86`, and the clear coat
    uses `max( w, 0.02 )` (`world_pbr.frag:248`).
  - Alpha test is a `discard` at the material reference
    (`world_pbr.frag:200`).
  - No texture tool adjusts roughness mips for normal variance, and no tool
    preserves alpha-tested coverage across mips.
- **Output is display-referred.** The shaders write linear radiance with the
  integer-HDR exposure scale straight into an 8-bit swapchain format. There
  is no tone curve and no floating-point scene target, so the resolve already
  averages clipped, display-range values (no bright-edge fireflies).
- **API level.** The instance requests Vulkan 1.1 (`vulkan_device.cpp:208`).
  Depth/stencil resolve and render pass 2 are extensions there, and core in
  1.2.

Rechecked at `d6260d90` (2026-09-25), by reading source. No A0–A5 work
exists, and the points above hold except the API level:

- Alpha to coverage is still the `VK_UNIMPLEMENTED()` stub
  (`shaderapivulkan.cpp:5385`), with empty dynamic calls (`:2656`).
- No PBR shader filters roughness: `max( mrao.g, 0.02 )` is at
  `world_pbr.frag:366`, `model_pbr.frag:177` and `world_pbr_glass.frag:88`.
  The clear coat clamps in `PbrClearCoat` (`pbr_brdf.glsl:101`), at
  `world_pbr.frag:450` and at `model_pbr.frag:214`. Alpha test is still a
  `discard` (`world_pbr.frag:344`).
- No `render.msaa-targets.v1`, `render.alpha-coverage.v1` or
  `render.pbr-specular-aa.v1` owner, fixture or manifest suite exists.
  `frame_pacing.py` still pins `mat_antialias 0`.
- The MSAA code is unchanged in kind but has moved: `ApplySampleCount`
  (`vulkan_device.cpp:7609`), `CreateMsaaTargets` (`:7643`),
  `CreateAttachmentPass` (`:985`), `ResolveBackBuffer` (`:7806`), and the
  scene-capture depth skip (`vulkan_scene_capture.cpp:331`).
- **API level changed.** The instance now requests Vulkan 1.2 when the loader
  offers it, else 1.1 (`vulkan_device.cpp:238`), for RFC 0011 G5's compute
  feature chain. The backend still uses neither depth/stencil resolve nor
  render pass 2, so open decision 2 stands.

## Goals

1. Antialiased geometry, alpha-tested and specular edges in the native PBR
   and legacy paths at 4x, with the 1x path unchanged.
2. Alpha to coverage with one semantic rule shared by the legacy and PBR
   families, identical to alpha test at one sample.
3. PBR specular AA that is exactly the identity for constant normals and
   moves normal-mapped results toward a supersampled reference.
4. A per-profile multisample memory policy chosen by measurement, including
   a mobile path that doesn't need a persistent 145 MB target if the device
   allows it.
5. Scene depth available to glass under MSAA, or declared absent.
6. Default sample counts recommended per profile only within declared frame
   time and memory budgets.

## Non-goals

- Temporal AA, ML upscalers, and post-process AA (FXAA, SMAA, CMAA).
  Post-process AA is an optional later addition, not required by any gate
  here.
- Supersampling, sample-rate shading, and multisampled render targets
  (water, monitors). They remain open decisions.
- Specular AA in legacy shader families. That would change D3D9-defined
  appearance and needs a versioned behavior decision of its own.
- DXVK/D3D9 captures or parity work. The D3D9 source defines the legacy alpha
  to coverage rule, and the oracles here are analytic or native-only.
- A new settings UI. The existing `mat_antialias` Video option remains the
  control.

## Layers and owners

| Knowledge | Owner | Consumers |
| --- | --- | --- |
| Sample-count clamp and support | `public/render/render_sample_count.h` (`render.sample-count.v1`, existing) | Shader API, device manager, autoconfig clamp |
| Multisampled target obligations and memory policy | new `render.msaa-targets.v1` contract; native backend implements | Pass builder, scene capture, readback, present, frame stats |
| Alpha to coverage rule and coverage-alpha function | new `public/render/render_alpha_coverage.h` (`render.alpha-coverage.v1`) | Native shadow state, PBR shaders (GLSL mirror), oracles |
| Filtered roughness | new `public/render/pbr_specular_aa.h` (`render.pbr-specular-aa.v1`), beside `pbr_brdf.h` | `pbr_specular_aa.glsl` include used by world, model and glass PBR shaders; pixel oracle |
| PBR schema default for alpha to coverage | `public/render/pbr_material_schema.h` (RFC 0007) | Material loaders, Hammer catalog |
| Normal-variance roughness mips and coverage-preserving alpha mips | KTX2 writer (RFC 0008 F3, R55) | Map/texture exporters, `tools/portal_pbr` remaster |
| Default sample count per adapter | existing dxsupport/autoconfig policy (`public/render/dxsupport_policy.h`) | Video options, first-run config |

The GLSL functions mirror the C++ owners. They aren't a second authority: the
pixel suite checks each shader against its C++ owner, and each owner's
constants carry a filter version recorded in evidence.

## Multisampled targets (`render.msaa-targets.v1`)

The contract codifies P6 behavior and adds the missing obligations:

- **Resolve before read.** Every read of the back buffer (copy, capture,
  readback, present, gamma) observes the resolved image of all draws before
  it. The resolve is recorded in frame stats (`m_resolveCount`, existing).
- **Scene depth.** A scene capture under MSAA provides a single-sampled depth
  or reports that depth is absent, and the glass shader's existing
  `sceneDepthValid` flag carries that. The depth value is sample 0, taken
  through `VK_RESOLVE_MODE_SAMPLE_ZERO_BIT` when depth/stencil resolve is
  available, or a shader copy otherwise. A3 chooses one path per profile.
- **Sample positions.** The provider uses the standard sample locations and
  reports `standardSampleLocations`. A device without them fails the coverage
  oracle's exact bounds and declares its own tolerance.
- **Occlusion queries** count samples. The exposure histogram's calibration
  (existing) stays valid at every supported count.
- **Resize, sample-count change and loss** rebuild targets at a frame
  boundary. They never present an unresolved or partially rebuilt image. P6
  already applies changes at the next frame.
- **Memory policy is per profile**, one of:
  - `persistent`: today's behavior, stored multisampled images.
  - `transient`: multisampled attachments with `TRANSIENT_ATTACHMENT` usage in
    lazily allocated memory where offered, resolved inside the last pass
    through `pResolveAttachments`, with depth and stencil `DONT_CARE` at the end.
    It's valid only for frames whose back-buffer passes need no reload of
    multisampled data. Mid-frame reads (refraction copies) break that, so
    the policy has to state what those frames do.
  - `render-to-single-sampled`: `VK_EXT_multisampled_render_to_single_sampled`
    where the device exposes it. There's no multisampled image. The driver
    resolves on tile and reloads resolved data after a pass break, so edges
    drawn before a mid-frame copy stay resolved but lose their sub-sample
    detail. The coverage oracle declares that difference.

  The profile records the chosen policy and the device facts behind it:
  extension list, memory types, measured frame time and memory. Nothing is
  selected on a vendor-name guess.

  Lazily allocated multisampled attachments are one mechanism, owned by
  RFC 0016's [GPU-driven submission](0016-render-core.md#gpu-driven-submission-plan-2026-10-05-user-direction) phase S7
  (`kLazyAttachments`); this policy selects it per profile and does not
  define a second one.

## Alpha to coverage (`render.alpha-coverage.v1`)

**Rule.** Alpha to coverage is effective for a draw if and only if all of
these hold:

- the material requests it: legacy through `$allowalphatocoverage`; PBR
  through its schema, default on;
- alpha test is enabled;
- alpha blending is disabled;
- the pass has more than one sample.

This is D3D9's aggregate rule, with the extra PBR default. Otherwise the draw
is exactly today's alpha test.

**Coverage alpha.** When effective, the shader doesn't discard at the
reference. It writes a sharpened alpha,
`saturate( ( a - ref ) / max( fwidth( a ), eps ) + 0.5 )`, which crosses 0.5 at
the reference and ramps over about one pixel. It discards only below a small
floor, so fully transparent texels still skip depth writes. The pipeline sets
`alphaToCoverageEnable`, a new raster-state key bit.

- **Legacy.** The native backend implements the shadow state. The legacy
  families' alpha-test shader paths emit the same sharpened alpha when the
  rule holds. `EnableAlphaToCoverage()`/`DisableAlphaToCoverage()` on the
  dynamic API remain no-ops, as the rule is static.
- **Invariants** (oracle-checked):
  - One sample: pixels are byte-identical to today for every family.
  - Blended or non-alpha-tested draws are unchanged at every count.
  - A material without the request is unchanged at every count.

## Specular antialiasing (`render.pbr-specular-aa.v1`)

**Filtered roughness.** Geometric specular AA after Kaplanyan et al. (2016)
and Tokuyoshi and Kaplanyan (2019, 2021):

- the kernel variance is estimated from `dFdx`/`dFdy` of the final shading
  normal, after normal mapping;
- it is added in GGX `alpha²` space and clamped by a threshold;
- the result converts back to perceptual roughness.

Filament's defaults (screen variance 0.15, threshold 0.2) are the starting
point. The values used are fixed by A2's oracle and versioned in the owner
header.

**Consistency.** One filtered roughness feeds every roughness consumer in a
fragment:

- direct GGX distribution and visibility;
- the split-sum table lookup (and so directional albedo and the diffuse
  weight);
- probe and `$envmap` mip selection;
- glass rough refraction;
- on the render core (RFC 0016 `render.lighting.v1`, amendment 2026-09-28),
  also the clustered, LTC area-light and projected-light lobes and the
  screen-space reflection cutoff and mip.

The clear coat filters its own roughness with the geometric normal it already
uses. The debug `INDIRECT_VIEW` variant is unchanged.

**Identity.** With zero normal derivatives the function returns its input
exactly, so flat, unmapped fixtures are byte-identical. That keeps the R47
Cycles comparisons of constant-normal cases untouched. Normal-mapped
fixtures change only as the C++ owner predicts.

**Cost.** A few ALU operations and two derivatives per fragment. A2 records
the measured cost.

## Offline filtering (KTX2 writer)

These apply to new content only. Legacy VTFs load unchanged and rely on the
runtime path.

- **Normal-variance roughness mips.** For each mip, the writer measures the
  variance of the normal-map texels each texel covers (Toksvig-style, in GGX
  `alpha²` space) and adds it to the MRAO roughness channel. The normal map
  and MRAO must share UV mapping and resolution, declared by the material. A
  mismatch is reported and that material falls back to the runtime path. It
  is never silently skipped or guessed.
- **Coverage-preserving alpha mips** (Castaño 2010). For alpha-tested
  textures, each mip's alpha is scaled so the fraction of texels passing the
  material's reference matches mip 0. Otherwise foliage and fences thin out
  with distance.
- **Filter version.** The writer records a filter version in the KTX2
  metadata and in the R57 cache key, so changed filters rebuild.

## Delivery plan and gates

### A0: Baseline, fixtures and budgets

- Pixel fixtures, native only and headless on the GPU (`material_pixel_conformance`
  families):
  - `msaa-edge`: analytic half-plane edges at several angles, with the
    resolved value checked against exact area coverage within the
    sample-pattern quantization bound. Negative control: 1x fails.
  - `alpha-coverage`: an alpha ramp and a fence texture, legacy and PBR,
    1x/4x, with blend-on and no-request controls.
  - `specular-aa`: a bumpy sphere and a high-frequency normal-mapped plane.
- Shimmer metric: N frames under sub-pixel camera jitter; per-pixel temporal
  luminance standard deviation, reported as a distribution.
- Supersampled reference: the same scene at 8× linear resolution without
  specular AA, box-downsampled.
- Frame-time and memory budgets set before any default changes:
  `frame_pacing.py` at 1x/2x/4x on the Linux GPU profile and on the Fold7,
  interleaved A/B.
  Within each sample policy, collect the
  [baseline/candidate resolution sweep](0016-render-core.md#optimization-resolution-sweep-user-decision-2026-10-03)
  before optimizing and when judging the result. Record GPU shading/resolve cost,
  CPU work, complete-frame intervals and memory per point; changing sample count
  is a separate policy comparison, not an equivalent-quality optimization.
- Device fact capture on the Fold7:
  - `VK_EXT_multisampled_render_to_single_sampled`;
  - depth/stencil resolve modes;
  - lazily allocated memory types;
  - `standardSampleLocations`;
  - the depth format.

### A1: Alpha to coverage

- Rule owner header plus the shadow-state and pipeline-key bit.
- The sharpened-alpha path in the legacy alpha-test shaders and the PBR
  shaders.
- `pbr_material_schema.h` default, with `$alphatocoverage 0` to opt out.
- Suite: the rule table exhaustively, plus the A0 pixel fixture.
- Negative providers: one ignores the request, one enables coverage with
  blending, one discards at the reference under coverage. All must fail.

### A2: PBR specular AA

- `pbr_specular_aa.h` and `pbr_specular_aa.glsl`, used by world, model and
  glass PBR shaders and every variant.
- Oracles:
  - identity: constant-normal cases byte-identical;
  - prediction: normal-mapped cases match the C++ owner within the existing
    PBR tolerance;
  - shimmer: temporal deviation falls by a declared factor at 1x and 4x;
  - reference: L1 error against the supersampled reference is no worse than
    without AA, and mean highlight energy stays within tolerance.
- Negative providers: kernel disabled (fails shimmer), roughness biased
  upward without derivatives (fails identity and reference), and the filter
  applied to only one roughness consumer (fails prediction).

**A2 progress (2026-10-06, first slice, render core only).** Installed:

- the owner `public/render/pbr_specular_aa.h` (version 1, screen variance
  0.15, threshold 0.2) and its GPU copy
  `render/shaders/common/pbr_specular_aa.glsl`;
- the surface program's pbr point (world and model, every variant) filters
  the one roughness after normal mapping, so every lobe, the split-sum
  lookup, probe mip, specular occlusion and the SSR target read it. It is
  view feature `kSurfaceViewSpecularAa` (a specialization constant),
  selected by `r_core_specular_aa` (default 1);
- `render.pbr-specular-aa` (12 checks: owner against an independent double
  formula, GLSL against owner over 256 cases, bitwise identity at zero
  derivatives, widening, threshold) and its sensitivity row (disabled,
  biased and unclamped kernels each fail their clause), both passing through
  the shared runner on Linux native Vulkan (Radeon 8060S);
- `tools/render/term_sweep.py --terms specular_aa`: on `sp_a1_intro4_relit`
  at 1920x1080 over five demo views, filter off against on: 0.02 ms core
  view (within the run's +-0.23 ms drift), mean dE 0.07, p95 0.6, 0.04 % of
  pixels over 2.3, concentrated on perforated panels, tile grout and curved
  glossy trim.

The other lab suites are unchanged by it: debug-views, lighting-controls and
shadow-receiver-perf fail identically with the filter compiled out (a
pre-existing failure in the shared tree), and the rest pass.

Shimmer in motion (2026-10-06, `tools/render/shimmer_sweep.py`, 8060S):
`sp_a1_intro4_relit` at 1920x1080, four demo views, 12 noclip strafe steps of
0.25 units, fixed exposure (`mat_dynamic_tonemapping 0`,
`mat_force_tonemap_scale 1`), the filter on and off captured at each paused
pose in one session; a 3840x2160 run box-downsampled as the reference.
Result: **no measurable shimmer reduction.**

- On the filter's footprint (0.9-18 % of pixels per view), on and off differ
  by 0.0001-0.0003 linear luminance. Frame-to-frame change, on over off, is
  0.993-1.006 per view; the declared shimmer clause (<= 0.9) fails.
- Against the reference, both settings are within 0.3 % of each other
  (1.0024): the reference is a separate boot, and its scene-state difference
  (about 0.005 mean error) is an order of magnitude above the filter's
  effect, so it cannot judge this slice.
- Energy: +0.06 % (limit 1 %), passes.
- Method notes: separate boots for on and off differed by 2-3 % in mean
  luminance, and auto-exposure shifted whole frames; both are removed above.

So, at version 1's constants and these views, geometric specular AA is
correct and nearly free but does not reduce the motion artifacts measured
here. Next: the RFC's controlled lab fixture (a high-frequency normal-mapped
plane and bumpy sphere at low roughness) to see whether the filter acts where
aliasing is known to exist, and whether the remaining in-game sparkle comes
from another source (normal-map minification without variance-aware mips
(R66), probe or lightmap sampling). The constants stay at version 1 until
that oracle sets them.

Lab image oracle (2026-10-06, `render.pbr-specular-aa.image`, recorded
known fail): `render_lab suite specular-aa-image` draws the pbr point on a
bumpy metal plane (8-texel bumps up to about 25 degrees, normal mips
box-filtered as stored bytes, as Source content has them) under one point
light, from a grazing view strafed 16 steps of 0.5 units at 128x128,
against a 4x4 supersampled render of the same poses, display-referred
(Reinhard per sample, then the box filter; a first raw-HDR configuration
was dropped because a few radiance peaks of several hundred dominated every
mean). A flat plane is bit for bit the same with and without the filter
(passes). With the filter, at version 1's constants:

| Roughness | Shimmer on/off | Reference L1 on/off | Mean on / off / reference |
| --- | --- | --- | --- |
| 0.10 | 4.35 | 7.59 | 0.173 / 0.056 / 0.057 |
| 0.25 | 1.53 | 1.75 | 0.280 / 0.239 / 0.226 |

Without the filter, the mipmapped normals already match the reference's
mean. A sweep of the constants (screen variance 0.15, 0.05, 0.02, 0.005 at
threshold 0.2; 0.05/0.02; 0.01/0.01) only reduces the damage as the kernel
shrinks; no setting beats off. The threshold never binds here.

Interpretation (not yet tested): this RFC applies the kernel to the final
shading normal after normal mapping. Screen derivatives of a high-frequency
normal map are noisy per 2x2 quad, so the widened lobe itself flickers and
brightens. Tokuyoshi and Kaplanyan's 2019 projected-space filter is defined
on the geometric (interpolated) normal; normal-map detail belongs to normal
variance (Toksvig-style, A4/R66, or a runtime estimate from the filtered
normal's length). Proposed amendment, pending a curved-geometry fixture:
derivatives of the interpolated normal only, plus a normal-variance term
for the map. Until then the product filter (`r_core_specular_aa 1`) has no
measured benefit in game and a measured cost on bumpy low-roughness metal
in the lab.

Open: the amendment above and its curved fixture, the
one-consumer negative provider, the clear coat's own filter, the frozen
native backend's `world_pbr`/`model_pbr` copies (not changed, by RFC 0016's
binding rules), VGPR/occupancy and the resolution sweep on the 8060S and RTX
3070, and Fold7 and Apple runs.

### A3: Multisampled target policy

- Scene depth under MSAA (glass `sceneDepthValid` true at 4x, with a
  refraction pixel case).
- In-pass resolve for the final back-buffer pass.
- End-of-frame depth/stencil `DONT_CARE`.
- The mobile memory policy experiment: `transient` versus
  `render-to-single-sampled` versus `persistent` on the Fold7, measuring
  frame time, GPU time, device memory and GPU busy. Use a GPU profiler if
  the counters disagree, as the frame-pacing record advises.
- One policy is recorded per profile with its evidence. Existing
  material-pixel families stay byte-identical at 1x.

### A4: Offline filtering (R66)

- Normal-variance roughness mips and coverage-preserving alpha mips in the
  KTX2 writer, with pairing validation and a filter version.
- Callers: the PBRT and World Stage exporters and `tools/portal_pbr`.
- Oracles:
  - shimmer and reference at distance improve over runtime-only;
  - alpha coverage at mip N matches mip 0 within tolerance;
  - negative fixtures: mismatched pairing is reported, and a writer that
    skips the variance fails the distance oracle.

### A5: Product defaults

- The dxsupport/autoconfig policy recommends 4x only on profiles whose A0
  budgets pass at 4x, and 2x or 1x otherwise.
- Installed Portal smoke at the recommended count on the Linux native Vulkan
  profile and the Android profile.
- Video options still offer every supported count.

## Future temporal work

If R63 or a later RFC adds a floating-point scene target with tone mapping
after the resolve, the resolve must become tonemap-aware (inverse-luminance
weighted) to avoid bright-edge aliasing. That change would reopen this
contract. RFC 0019 owns the proposed [temporal upscaling
contract](0019-temporal-upscaling-contract.md). A temporal method replaces 4x
MSAA in the selected High profile once its edge, alpha, shimmer and
portal-boundary quality gates pass, per the user's
2026-10-03 [target cutover](0019-temporal-upscaling-contract.md#target-cutover-user-direction-2026-10-03).
RFC 0016's [FSR temporal exception](0016-render-core.md#binding-rules-for-all-render-work-user-decision-2026-09-28)
makes a performance miss advisory for this cutover. Until then the current 4x
MSAA policy remains the High gate. Specular AA remains useful under temporal
methods.

## Roadmap

- **R65, runtime antialiasing (A0–A3, A5).** Ranked directly after R47. It is
  bounded, and it closes a live `VK_UNIMPLEMENTED` stub on the product path.
  Specular AA also costs least while the PBR shaders are still being built:
  added after the R47 fixtures settle, it would reopen them. Prerequisites:
  R02, R32, R47.
- **R66, offline texture filtering (A4).** Ranked directly after R55, whose
  KTX2 writer it extends. Prerequisites: R55, R65.

Mobile default claims also need R29-class device evidence; a missing device
run leaves that profile's A5 unverified.

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| Specular AA over-blurs and breaks Cycles comparisons | Exact identity for constant normals; parameters fixed by the supersampled-reference oracle, not by eye |
| Mobile 4x costs more than budgets allow | Budgets first (A0); per-profile policy; A5 recommends lower counts where budgets fail |
| Transient attachments conflict with mid-frame refraction copies | Policy states the fallback per frame shape; `render-to-single-sampled` evaluated as the alternative |
| Alpha to coverage changes legacy appearance | D3D9's opt-in rule only; byte-identical at 1x and without the request |
| Derivative noise at triangle edges or 2×2 quad boundaries | Clamp threshold; bumpy-sphere and silhouette cases in the fixture |
| Offline and runtime filters double-count variance | Runtime kernel measures screen-space change only; the reference oracle runs with both enabled |

## Alternatives considered

- **TAA / DLSS / FSR / XeSS.** These give the highest quality on content
  built for them, but need motion vectors, jitter, history, an HDR target and
  portal-aware reprojection. Deferred to R63-era work.
- **FXAA/SMAA only.** Cheap, but blurs texture detail and misses sub-pixel
  geometry. MSAA is already implemented and correct for the clustered Forward+
  renderer.
- **Supersampling or sample-rate shading.** Fixes every source of aliasing
  at 2–4× fragment cost. That's too expensive as a default on mobile, and it
  stays an open decision for desktop.
- **Specular AA only offline (Toksvig).** Misses curvature from geometry and
  legacy VTF content. Runtime and offline are complementary.

## Open decisions and required evidence

1. The `transient` versus `render-to-single-sampled` policy on the Fold7,
   decided by the A3 measurement.
2. The scene-depth path: depth/stencil resolve (1.2 or extensions) versus a
   shader copy. It depends on the profile's API level and MoltenVK's support.
3. Whether water and monitor render targets get multisampling. This needs a
   measured visual case first.
4. Whether 8x is offered on desktop profiles beyond the clamp that already
   exists.
5. macOS/iOS through MoltenVK: sample counts, memoryless storage and alpha
   to coverage, all unverified until R29 runners exist.

## Amendments to RFC 0001, RFC 0007 and RFC 0008

- **RFC 0001 (R32 Video options P6):** `render.msaa-targets.v1` supersedes
  P6's implicit target obligations. The "alpha-to-coverage stays an
  unimplemented stub" limitation moves to R65 A1.
- **RFC 0007 shading model:** GGX roughness is the filtered roughness of
  `render.pbr-specular-aa.v1` for every consumer in a fragment. `$alphatest`
  PBR materials use alpha to coverage under multisampling by default.
- **RFC 0008 F3:** The KTX2 writer adds normal-variance roughness mips,
  coverage-preserving alpha mips, pairing validation and a filter version in
  metadata and cache keys.

## Source references

- A. Kaplanyan, S. Hill, A. Patney, A. Lefohn, *Filtering Distributions of
  Normals for Shading Antialiasing*, HPG 2016.
- Y. Tokuyoshi, A. Kaplanyan, *Improved Geometric Specular Antialiasing*,
  I3D 2019, and *Stable Geometric Specular Antialiasing with Projected-Space
  NDF Filtering*, JCGT 2021.
- M. Toksvig, *Mipmapping Normal Maps*, JGT 2005.
- I. Castaño, *Computing Alpha Mipmaps*, 2010.
- Vulkan specification: multisampling, `alphaToCoverageEnable`, transient
  attachments and lazily allocated memory, `VK_KHR_depth_stencil_resolve`,
  `VK_EXT_multisampled_render_to_single_sampled`.

## Proposed decision

Adopt 4x MSAA with alpha to coverage and PBR specular AA as the native
renderer's antialiasing policy. Deliver it as R65 (runtime, after R47) and
R66 (offline texture filtering, after R55). Choose mobile target memory
policies and default sample counts only from measured per-profile evidence.
