# RFC 0021: External Render SDK Integration and Runtime Provider Selection

- Status: Proposed (2026-10-03); no 0021 gate complete and no mechanism
  implemented. The FSR 4.1.1 provider and its RFC 0019 opt-in paths already
  exist and are this RFC's reference implementation, not its work product.
- Date: 2026-10-03
- Scope: The engine-side mechanism for integrating closed or externally
  versioned render compute SDKs (AI upscaling first), and for selecting a
  provider at runtime. This RFC owns intake, the two execution routes, the
  provider catalog and the input-convention contract. It does not own temporal
  upscaling product policy, which
  [RFC 0019](0019-temporal-upscaling-contract.md) keeps.
- User direction (2026-10-03): more AI scaling providers are wanted, and they
  must be selectable at runtime rather than compiled in by name.
- Render architecture: [RFC 0016](0016-render-core.md) owns frame/view, graph,
  device, shader-artifact and output semantics. Its binding rules, scope and
  complexity discipline and hard render budgets apply to all work here.
- Antialiasing policy: [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md)
  remains the owner of MSAA, alpha to coverage and specular AA.
- Platform composition: [RFC 0001](0001-capability-based-platform-architecture.md).
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md),
  Q-PRESENTATION and Q-PRODUCT, plus one shared suite per claiming provider.
- Tracking: no ranked row yet. Ranking is a user decision; candidate attachment
  is R95/R96 for the temporal use and R86/R88 for the mechanism.

## Decision

Performance comparisons between providers and execution routes follow RFC 0016's
[baseline/candidate resolution sweep](0016-render-core.md#optimization-resolution-sweep-user-decision-2026-10-03).
Match output extent and scene, record each provider's input extent, quality/mode
and bridge/transfer/synchronization cost, and attribute CPU/GPU limits per point.
A provider's low-resolution result cannot decide its 4K benefit; kernel timing
alone cannot establish a complete-game gain. RFC 0019 still owns temporal quality
and promotion policy, including the FSR timing exception.

External render SDKs enter through exactly two sanctioned execution routes, are
declared in one composition-owned provider catalog, and are refused by name
before the graph mutates. There is no third path, no global service locator and
no vendor identity in portable code.

Two decisions follow from the library research in this RFC and are recorded
here because they change what "add more providers" can mean today:

1. **Every current candidate is closed, vendor-gated or non-redistributable.**
   AMD FSR 4.1.1 ships as a signed binary DLL and requires an AMD 7000/9000-series
   GPU [S3]; Intel XeSS 3 is binary-only under a licence that forbids
   modification and reverse engineering [S4][S5]; NVIDIA DLSS ships as
   prebuilt, non-redistributable plugin DLLs that must come from a release zip,
   and its DLSS-G plugin cannot be built from source at all [S1][S2].
2. **The FSR provider already in the tree is GPL-2.0-or-later.** The pin is a
   community Linux port, not an AMD release, and its own notice states it is
   "not an MIT-licensed SDK for unrestricted inclusion in a proprietary game"
   [S7]. That is a licensing decision for the user before anything ships; this
   RFC makes it a gate (V0) instead of an accident.

## Observed starting point (2026-10-03)

FSR 4.1.1 is already integrated and is not redone by this RFC. What exists:

- The provider: `ITemporalUpscaler` (`public/render/device/temporal.h:32`) with
  `CreateFsr411` and `FsrProvider` (`render/device/vulkan/fsr.cpp:97`, `:191`),
  compiled only under `--render-fsr411` (`wscript:500`), pinned through
  `external/fsr411`.
- The lab evidence: `render.lab.temporal-fsr411` and its seeded sensitivity
  suite `render.lab.temporal-fsr411.sensitivity` in
  `quality/conformance.manifest.json`, driven by `render/lab/temporal_suite.cpp`,
  with recorded results under `quality-results/fsr-2026-10-03`.
- The opt-in game path: `CoreTemporal` (`render/composition/core_temporal.h:45`),
  the temporal request/capture stream in `render/composition/core_world.cpp:1960`,
  and the engine controls `r_temporal_scale` and `r_temporal_capture`
  (`engine/render_core_host.cpp:107`, `:613`), which the Portal 2 video menu
  already reads. Records:
  [lab](0019-fsr-lab-2026-10-03.md), [game](0019-fsr-game-wip-2026-10-03.md).
- What it is not: no product profile sets the FSR build option or the temporal
  scale (no `fsr411` in `quality/product_profiles/*.json`, no launcher passes it,
  the convar default is 0), no RFC 0019 acceptance gate is complete, and
  RFC 0016's FSR timing exception keeps its timing miss advisory.

What this RFC changes about that implementation, and nothing else:

1. V0 records the licensing and provenance facts of the pin (see the FSR section
   below). It does not re-implement the provider.
2. V3 replaces the provider's private `ComputeInterop` duplication with the
   shared documented bridge contract, so the second provider does not copy it.
3. V4 makes FSR the first entry in the catalog with declared conventions, so it
   becomes *selectable* rather than constructed by name at one call site
   (`render/composition/core_world.cpp:2005`).

## Library-level findings

These are the facts the mechanism must satisfy. Each is quoted or paraphrased
from the vendor sources listed in [Sources](#sources); line-level citations are
to those documents, not to this repository.

### AMD FSR (FidelityFX SDK, `ffxFsr2` API)

- Integration is through the AMD FSR API and "the signed binary distribution"
  [S3]. Shading-language requirement is HLSL `CS_6_6` [S3], so the compute
  programs are not portable GLSL/SPIR-V source we can recompile.
- Hardware gate: "FSR4 requires an AMD 7000 series discrete GPU or AMD 9000
  series GPU or later" [S3].
- Scaling ratios are named by the vendor and must not be hardcoded per preset:
  NativeAA 1.0x, Quality 1.5x, Balanced 1.7x, Performance 2.0x, Ultra 3.0x,
  with jitter sequence lengths 18/23/32/72 and `ceil(8n^2)` custom [S3].
- Jitter is reported in **pixel space**, must be applied to *all* rendering
  (opaque, alpha-transparent, raytraced), and the sequence must never emit a
  null (0,0) vector [S3].
- Motion vectors are 2-component float in **render pixels**, range
  `[-width, width] x [-height, height]`, **without** jitter unless
  `FFX_UPSCALE_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION` is set; a
  `motionVectorScale` field converts other spaces [S3].
- Depth is single-sample float; inversion and infinite far plane are declared by
  context-creation flags (`FFX_UPSCALE_ENABLE_DEPTH_INVERTED`,
  `..._DEPTH_INFINITE`) [S3].
- Exposure is two distinct values: pre-exposure (divides the input, undoing
  packing) and exposure (multiplies the result), and it "should match that which
  the application uses during any subsequent tonemapping passes" [S3].
  `FFX_UPSCALE_ENABLE_AUTO_EXPOSURE` substitutes an ISO-100 formula and costs
  performance [S3].
- FSR 4 "no longer requires the title to generate either a Reactive mask or a
  Transparency and Composition mask, though they may still be provided", and a
  resource-requirements query returns a required/optional bitmap per context
  version [S3].
- Mip bias is a vendor formula: `log2(renderResolution/displayResolution) - 1.0`,
  giving -1.58/-1.76/-2.0/-2.58 for the four ratios [S3].
- Frame time delta is milliseconds, ~16.6 at 60 fps, and drives auto-exposure
  [S3]. HDR needs `FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE`; non-linear sRGB or PQ
  input needs its own flags [S3].
- Memory is queryable *before* context creation
  (`ffxQueryDescUpscaleGetGPUMemoryUsageV2`): 81 MB working set at 1920x1080,
  318 MB at 3840x2160, 1274 MB at 7680x4320 [S3].
- Reference cost: 352 us at 1080p and 1316 us at 4K on an RX 9070 XT, but
  ~1998 us at 1080p on an RX 7600 [S3]. At our 120 FPS target of 8.33 ms, a
  low-end-AMD GPU can spend a quarter of the frame in the upscaler alone.
- Placement guidance is explicit: screen-space reflections, GTAO and denoisers
  before; film grain, chromatic aberration, vignette, tonemapping, bloom, depth
  of field and motion blur after [S3]. A separate TAA pass is redundant [S3].

Our current pin, for the record: `external/fsr411` is
`johnpanos/FSR-4.1.1-linux` at `46a56a6`, an unofficial native Linux/Vulkan port
whose shaders and weights were extracted from AMD's
`amd_fidelityfx_upscaler_dx12.dll` (FidelityFX SDK 2.3.0, `60f4ea8`) under AMD's
MIT exception. Its notice records GPL-2.0-or-later for the runtime, disclaims any
AMD affiliation, and states the runtime is not an MIT-licensed SDK for
unrestricted inclusion in a proprietary game [S7]. Its README states the shader
set needs Vulkan 1.3, FP16/INT8/INT16 arithmetic, integer dot products,
formatless storage-image writes, `VK_KHR_push_descriptor`,
`VK_KHR_compute_shader_derivatives` and
`VK_VALVE_shader_mixed_float_dot_product`, with output limited to 3840x2160 and
no automatic fallback [S7]. That is a capability gate, not a preference.

### Intel XeSS 3 (XeFX, `xess_vk.h`)

- Distribution is **binary only**, under the Intel Simplified Software License
  (October 2022): redistribution in binary form is allowed without
  modification provided the notices travel with it, and "no reverse
  engineering, decompilation, or disassembly of the Software is permitted, nor
  any modification or alteration of the Software or its operation at any time,
  including during execution" [S4]. The SDK also states Intel "may make changes
  to the Software, at any time without notice" [S4].
- The published tree ships `bin/libxess.dll`, `libxess_dx11.dll`, `libxess_fg.dll`
  and `libxell.dll` and requires the MSVC redistributables; the stated platform
  requirement is Windows 10/11 x64 [S4][S5]. There is no Linux, Apple or Android
  binary in this package, which excludes it from our Linux desktop, macOS, iOS,
  tvOS and Android profiles unless Intel publishes one.
- Vulkan requirements are queryable and must be met before instance/device
  creation: Vulkan 1.1 plus `shaderStorageImageWriteWithoutFormat` and
  `mutableDescriptorType` (`VK_EXT_mutable_descriptor_type`); `shaderInt8` and
  `shaderIntegerDotProduct` are requested when supported for performance [S5].
  `xessVKGetRequiredInstanceExtensions`/`...DeviceExtensions` return the exact
  list, and failure is `XESS_RESULT_ERROR_UNSUPPORTED_DRIVER` [S5].
- Super Resolution runs on any GPU with SM 6.4 and hardware DP4a, so it is not
  vendor-locked, unlike FSR 4 [S4][S5].
- **Jitter units differ from every other candidate**: the sub-pixel offset is in
  `[-0.5, 0.5]` and is applied as a shear transform, not in pixels [S5].
- Motion vectors are `R16G16_FLOAT` in screen-space pixels without jitter
  motion; NDC input needs `XESS_INIT_FLAG_USE_NDC_VELOCITY`. They may be
  low-res (default; XeSS then dilates them and **requires a depth texture**) or
  high-res at output resolution, in which case the host must supply **dilated**
  high-res motion vectors [S5].
- Depth: any format such as `D32_FLOAT` or `D24_UNORM`; smaller-is-nearer by
  default, `XESS_INIT_FLAG_INVERTED_DEPTH` otherwise [S5].
- Exposure: a value, an exposure-scale texture, `XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE`
  (with a stated performance cost), or `xessSetExposureMultiplier` fed the
  **inverse** of pre-exposure [S5].
- A responsive pixel mask (RPM) is optional, at input resolution, unjittered,
  float in the R channel, and internally clamped to 0.8 by default [S5].
- XeSS performs **no memory synchronization** itself: the host must deliver
  inputs in `VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL` and the output in
  `VK_IMAGE_LAYOUT_GENERAL`, with compute-shader-stage access [S5]. Input
  textures must be typed [S5].
- Ratio, mip bias and jitter-length values "should not be hardcoded per quality
  preset in the game code"; the SDK's own queries and formulas are to be used
  (mip bias -1 at 2.0x; 1.3 Performance -1.202, Ultra Performance -1.585) [S5].
- It is not thread safe; all calls must come from the thread that initialized the
  context [S5].

### NVIDIA DLSS (through Streamline 2.14.1)

- Streamline is the vendor's recommended, open-source cross-IHV integration
  layer, and it also carries NIS, Reflex and DLSS Frame Generation [S1][S2].
- `slInit` must run before any DXGI/D3D/Vulkan API use, `slSetVulkanInfo` binds
  the Vulkan device, and `slShutdown` must precede destroying the Vulkan
  instance and device [S2]. That is a hard lifecycle ordering against our
  device ownership rules.
- `pref.applicationId` is documented as "Provided by NVDA, required if using NGX
  components (DLSS 2/3)" [S2], so DLSS depends on an NVIDIA-issued identifier.
- Availability is queried per adapter through `slIsFeatureSupported(kFeatureDLSS,
  adapterInfo)` with an adapter identity, and driver/OS-out-of-date are distinct
  failures [S2].
- The host asks the SDK for the render size and sharpening:
  `slDLSSGetOptimalSettings`, which also reports dynamic-resolution source-size
  bounds [S2]. Mip-bias and ratio policy therefore live in the SDK, not the game.
- Resources are tagged, not transitioned: color in/out, depth, motion vectors and
  an optional 1x1 exposure texture, with "SL manages resource states so there is
  no need to transition tagged resources" [S2]. Lifecycles are declared per tag
  (`eOnlyValidNow`, `eValidUntilPresent`) and resources deletable within a frame
  must be marked volatile [S2].
- Constants: SL matrices are row-major and must not contain jitter; motion scale
  is `{1/w, 1/h}` for pixel-space vectors and `{1, 1}` for NDC; jitter must be
  in pixel space [S2].
- Multiple viewports allocate and free feature resources per viewport and
  require a flush before freeing [S2]; DLSS low-resolution motion vectors are
  inferred from the tagged extent [S2].
- Presets churn: A-D are gone, E/F are deprecated legacy, G-I are reserved, and
  K/L/M are in use [S2]. A profile must therefore name a *mode*, never a preset
  letter.
- Shipping requirements: production builds only, and either the original
  NVIDIA-signed SL DLLs or our own signing system, "otherwise SL plugins could be
  replaced with potentially malicious modules" [S2].
- Since SL 2.7.32 the binary artifacts (DLSS feature DLLs and Streamline DLLs) are
  not in the Git repository and must be fetched from a release zip; the DLSS-G
  plugin is prebuilt-only [S2]. Prerequisites are a GPU supporting DX11 and
  Vulkan 1.2+, Windows-oriented build tooling, and driver 512.15+ on NVIDIA [S2].
- DLSS Super Resolution is the cross-vendor-unavailable option: frame generation
  needs 40-series or newer, and multi-frame generation needs RTX 50-series or
  RTX PRO Blackwell [S1].

### Where they disagree

This table is the reason the mechanism below exists.

| Concern | FSR 4.1.1 | XeSS 3 | DLSS (Streamline) |
| --- | --- | --- | --- |
| Distribution | Signed binary; our pin is a GPL-2.0 Linux port | Binary only, no modification/reverse engineering | Prebuilt signed DLLs from a release zip, not in-repo |
| Jitter units | Render pixels | `[-0.5, 0.5]` shear | Render pixels |
| Motion units | Render pixels, `[-w,w]x[-h,h]` | Screen pixels, optional NDC flag | NDC by default, scaled by `mvecScale` |
| Motion resolution | Render (or display, flagged) | Low-res plus depth, or host-dilated high-res | Inferred from tagged extent |
| Depth | Float, inversion and infinite flags | Any format, inverted flag | Tagged buffer |
| Exposure ownership | Engine supplies pre-exposure and exposure, or auto | Value, texture, auto, or exposure multiplier | Exposure texture or auto-exposure flag |
| Masks | Not required by FSR 4; required/optional query | Optional responsive pixel mask, clamped 0.8 | Not part of the DLSS interface |
| Mip bias and ratio | Vendor formula, engine applies | SDK queries, engine applies | SDK optimal-settings query |
| Resource synchronization | Host declares and inserts | Host inserts layouts and stages | SL manages tagged states |
| Thread affinity | Adapter sequence | Calls from the initializing thread | Per frame token and viewport |
| Hardware gate | AMD 7000/9000-series or later | Any SM 6.4 / DP4a | NVIDIA for frame generation; SR is broader |

## Goals

- One intake path for an externally versioned or closed render compute SDK:
  pinned, licensed, declared, buildable on exactly the profiles that claim it.
- Two documented execution routes, a recorded reason for choosing the private
  one, and one shared contract suite that every provider on either route passes.
- One provider catalog with runtime selection, capability negotiation, refusal
  by name before mutation, and no global lookup.
- One input-convention contract so the engine knows what it must produce and each
  provider states what it consumes.
- Engine ownership of everything a provider cannot supply for itself: masks,
  motion coverage, history identity, image-quality references and budgets.

## Non-goals

- Frame generation, low-latency/present contracts and Reflex/XeLL-class latency
  control. RFC 0019 places frame generation outside its contract; this RFC does
  not widen that.
- Shipping any provider on a profile before its licensing decision (V0) and its
  gates pass. A cross-build or a lab image proves nothing about distribution.
- Runtime shader compilation in shipping products, and any provider that needs
  it.
- A second antialiasing policy. RFC 0012 still governs MSAA, alpha coverage and
  specular AA where their policies apply.
- Reimplementing an upscaler in engine GLSL. FSR 2-style spatial techniques
  (EASU/RCAS) are a legitimate provider but are engine-owned code, not an
  external SDK, and are not this RFC's intake subject.

## Owners and layers

| Authority | Obligation |
| --- | --- |
| Composition root | Build the provider catalog from descriptors, select a mode before the frame, record refusal and the declared native fallback |
| `render.pass.temporal-upscale` (RFC 0019) | Validate portable inputs, place one pass per selected view, own per-view history metadata |
| Provider adapter | Implement the contract for its route, own private persistent images, report failure without touching another view |
| `render.shader-library` | Resolve artifacts for the device's format; own the foreign-artifact intake rules |
| `render.device` adapter | Route 1: create pipelines from resolved artifacts. Route 2: lend a command buffer and declared accesses, never a handle above the adapter |
| `render.graph` | Declare accesses, synchronize, and retire provider resources behind completion tokens |
| Product profile | Own the default provider, mode, render extent and fallback for its target |
| `render.pass.output` | Keep owning exposure, tone map and presentation encoding after reconstruction |

`archlint` CAP011 keeps portable code free of vendor identity: only the adapter
column may name a vendor SDK, an external header or a native handle.

## Route 1: the standard pipeline path

The engine already has the standardized path this RFC reuses:
`PipelineRecipe` -> `Resolve(recipe, artifacts, device.Facts().artifactFormat)`
-> `device::PipelineDesc` -> `IRenderDevice2::CreatePipeline`
(`public/render/shaderlib/pipeline_recipe.h:24`, `public/render/device/pipeline.h`).
Every core pass uses it; `render/pass/ao/ao.cpp:104-121` is the canonical
six-step sequence. Artifacts are build-time only, generated by
`tools/render/shader_artifacts.py` from in-tree GLSL with the pinned compiler
(`quality/toolchain/shader-compiler.json`) and checked against
`render/shaders/layouts.json`; shipping products never compile shaders.

Limits a provider must fit or refuse by name: at most four bind groups
(`kMaxBindGroups = 4`), roles frame/view/material/draw, 128 bytes of draw
constants, and `BindingKind` limited to uniform buffer, storage buffer, sampled
texture, storage texture and sampler. Device-reported facts
(`DeviceFacts::artifactFormat`, `CapabilitySet`, `Limits`) come from the device,
never from the consumer.

## Route 2: the private adapter bridge

`ComputeInterop` (`render/device/vulkan/vulkan_device.h:378`) already exists as
an adapter-private bridge: declared input images with usages, `Record` on the
device sequence, and `Submitted`/`Aborted`/`DeviceDestroyed` for retention. FSR's
adapter uses it and writes its own `VkDependencyInfo` barriers
(`render/device/vulkan/fsr.cpp:32-57`), which is exactly the duplication this RFC
stops.

Route 2 becomes a documented adapter-side contract with one shared suite:
declared input extents/formats/usages, explicit barrier responsibility, history
retained through the completion token, device-loss and teardown callbacks, and
single-thread recording on the adapter sequence. Choosing Route 2 requires a
recorded reason naming what Route 1 could not express, and the reason is reviewed
like any other architecture claim.

Route 2 is Vulkan-shaped today. A GL or MoltenVK claim requires either a Route 1
implementation or an explicit statement that the provider is unavailable on that
backend; portability is never inferred.

## Provider catalog and runtime selection

The catalog is a composition-owned value, constructed by the root and passed
down. It is not a global, not a service locator and not a filename lookup. Each
descriptor declares:

- identity and version: id, vendor SDK name and version, contract revision;
- route: Route 1 or Route 2, with the recorded reason for Route 2;
- required device capabilities and formats, expressed as device-reported facts;
- supported render/output extent ranges and ratios;
- required and optional inputs, including which masks the host must produce;
- the input-convention block below;
- supported backends and platforms, and the build option that includes it;
- a memory estimate or a query for one, in the provider's terms;
- whether the provider owns jitter, mip bias and preset policy, or asks the host
  to apply values it returns.

Selection rules:

1. The product profile names a default provider and mode; a convar overrides it
   for A/B and diagnosis.
2. Selection happens before the graph mutates. A provider that cannot run on the
   current device, backend or profile is **refused by name**, with the missing
   capability named.
3. A refusal never becomes a silent fallback to a different quality claim. The
   profile's separately qualified native path is used only when the profile says
   so, and the chosen path is reported.
4. Two providers may be constructed in one process; only the selected one records
   work. Provider construction and teardown are not per-frame operations.
5. Preset letters, ratio tables and mip-bias formulas are provider policy. The
   engine stores the selected mode and extents and asks the provider for values
   when the provider owns them [S3][S5][S2].

## Input-convention contract

`TemporalDispatch` today hardcodes one convention set, including asserted
formats (`render/device/vulkan/fsr.cpp:126-140`). The catalog replaces that with
a declared block, validated before a pass is recorded:

| Field | Values |
| --- | --- |
| Depth kind | Non-linear, linear eye depth, reversed/inverted; near/far convention; finite or infinite |
| Motion units | Render pixels, screen pixels or NDC; with or without jitter; render- or output-resolution buffer; dilation state |
| Exposure ownership | Host supplies pre-exposure and exposure, host supplies a 1x1 exposure texture, or provider manages it |
| Color | Linear, sRGB-encoded or PQ input; HDR flag; output must match the input's color space and format |
| Jitter | Units (pixel or `[-0.5, 0.5]` shear), whether the host or the provider generates the sequence, and whether a null vector is possible |
| Mip bias | Provider-owned formula or host-supplied constant |
| Masks | Reactive, transparency/composition, responsive: required, optional, or unsupported, with the neutral value when omitted |
| Sample count | Single-sample color and depth, and the declared MSAA resolve policy |

A provider whose declared convention the engine cannot currently produce is
refused by name at composition, with the missing producer named. Mask producers
and motion coverage are engine-side and per-view, behind RFC 0019's `ViewKey`;
they are never per-vendor.

## Intake policy for an external SDK

- A pinned archive or gitlink with the dirty-tree check already used for
  `external/fsr411` (`render/device/vulkan/wscript:30-51`); a pinned host
  toolchain entry when the SDK ships buildable source; no unpinned network
  fetches at build or run time.
- One capability module per SDK in `architecture/modules.json` with
  `allowedEdges: []`, its external headers and uselib, following
  `external.fsr411`. Vendor code never becomes a portable module.
- One Waf configure option per SDK (`--render-fsr411` is the model), recorded in
  the product profile's `configure_options`, so a profile's build declares
  exactly which providers exist in it.
- Licence and provenance recorded per SDK, including redistribution terms,
  modification and reverse-engineering terms, and any per-file exception.
  Provenance warnings already in the tree stay.
- A loaded external binary is a loader event: it appears in the loader inventory
  and telemetry, and iOS static composition keeps first-party loader discovery
  out of the target.
- Vendor binaries that must be fetched rather than committed are pinned by hash
  and verified, and their absence is reported as "provider unavailable", never as
  a build break in unrelated products.

## Delivery plan

Each gate is machine-decided with negative controls. No gate accepts skipped
coverage, and no gate closes a render row.

| Gate | Work | Evidence |
| --- | --- | --- |
| V0 | Licensing and provenance decision per candidate, recorded before code; FSR 4.1.1 GPL-2.0 status resolved as ship/lab-only/not-shipped; DLSS application-id and signed-DLL requirements recorded; per-platform binary availability recorded for every candidate | Written decisions with the licence text cited; a check that refuses composition for a provider whose licence decision is absent |
| V1 | Foreign-artifact intake: a build-time artifact source for pinned SDK SPIR-V with SDK-version compiler identity, reflection through the pinned SPIRV-Cross, layouts declared in `layouts.json`, and `render.shader-artifacts` extended with negative controls | Seeded wrong-reflection, wrong-layout, wrong-compiler-identity and missing-artifact faults all detected; a shipping product contains no shader-compilation path |
| V2 | Route 1 proof: one complete provider built only from `PipelineRecipe` -> `Resolve` -> `CreatePipeline`, including a limits self-check that refuses by name when a vendor program exceeds four groups, draw-constant bytes or binding kinds | Shared suite passes on null, Vulkan and GL; sync validation silent; limits refusal detected by a seeded violation |
| V3 | Route 2 as a documented adapter-side contract, replacing FSR's private `ComputeInterop` duplication, with one shared suite over create/record/submit/abort/resize/loss/recover/teardown and two simultaneous views; FSR's existing adapter runs it unchanged | Suite passes for FSR and for two seeded bad bridges, each detected on its own clause |
| V4 | Provider catalog, conventions block, runtime selection and refusal by name; two providers coexist in one process; convar and profile default; no global lookup (archlint-enforced) | Catalog suite over every descriptor field; seeded lying descriptors (unsupported format, wrong convention, missing capability) each rejected; selection A/B in one process without restart |
| V5 | Engine-side mask and motion-coverage producers behind `ViewKey`, driven by each provider's required/optional declarations | Per-cohort coverage oracle; a provider declaring a required mask and not receiving it fails the frame instead of proceeding |
| V6 | A second real provider end to end on whichever route its intake needs, with the shared suite, lab oracles and image comparison against the same engine-owned 4x MSAA reference | Matched captures across motion, cutouts, transparency and portal boundaries; provider version and settings in the evidence |
| V7 | Product and profile evidence: per-profile default, A/B capture pair, refusal reporting on unsupported hardware, and native evidence per claimed platform; cost and memory measured against the profile budget with the complete image enabled | Frame time, GPU time, CPU record time and memory recorded per provider per profile; a provider claiming a platform it was not measured on stays unverified |

## Risks and mitigations

| Risk | Effect | Mitigation |
| --- | --- | --- |
| FSR 4.1.1 pin is GPL-2.0-or-later | A shipped proprietary client could be infected | V0 licensing decision first; the provider stays lab-only until decided; provenance notice preserved and re-checked by a tool |
| Vendor binary is not redistributable or not obtainable | A profile silently loses its provider | Pinned-by-hash archive with a verified absence reported as unavailable; never a hard build break in unrelated products |
| Provider requires hardware or driver features the target lacks | Fails at runtime on a device we claim | Capability descriptors queried before composition; refusal by name; per-platform native evidence, never inferred |
| Convention mismatch (jitter units, motion space, depth kind, exposure) | Ghosting, smearing or a silently wrong image, hard to diagnose | Declared conventions block validated before recording; per-convention conformance clauses; motion and mask oracles in `render_lab` |
| Route 2 duplication per vendor | Barriers, retention and device loss re-implemented and got wrong each time | One documented bridge contract with a shared suite (V3); Route 1 is the default and Route 2 needs a recorded reason |
| Upscaler cost eats the frame budget | 120 FPS target missed, or effects cut to compensate | Vendor-reported cost and memory recorded per profile before promotion; the budget stays owned by `linux-desktop-high-120`; no effect is reduced to pass |
| Preset and SDK churn | Profiles bake vendor preset letters that vanish | Profiles name a mode; values come from SDK queries; SDK version pinned and recorded in evidence |
| Vendor SDK updates change behavior silently | Image oracles stop meaning anything | SDK version in every artifact key, evidence record and conformance log |
| Frame-generation scope creep | Latency and present contract invented ad hoc | Explicitly out of scope until a latency contract is proposed and accepted |

## Alternatives considered

| Alternative | Why not |
| --- | --- |
| One hardcoded factory call per provider at its own call site | The current state: no selection, no refusal, duplicated barrier and lifetime code per vendor, and every provider re-litigates conventions |
| Force every vendor through the material family pipeline path | The four-group family layout and legacy-frontend material model are not an SDK host; a pass-route pipeline is the correct shape |
| A global provider registry or capability bag | Forbidden by the architecture rules in `AGENTS.md`; composition owns selection |
| Runtime shader compilation so vendor code need not be pinned | Shipping products never compile shaders; store and platform rules forbid the JIT assumptions this needs |
| Bind only one vendor forever (FSR) | FSR 4 is AMD-7000/9000-plus gated and our Linux, Apple and Android targets are mostly not; a single-vendor plan cannot serve the declared profiles |
| Vendor-per-profile selection with no cross-provider image reference | Quality would be judged per vendor; the engine-owned 4x MSAA reference is what makes providers comparable |

## Open decisions for the user

1. Is the GPL-2.0 FSR 4.1.1 port lab-only, or do we replace it with a
   redistributable provider (FSR 3.1-class, or XeSS where a Linux binary exists)?
2. Do we pursue DLSS at all, given a non-redistributable binary, an
   NVIDIA-issued application id, a signed-DLL integrity requirement and Windows/
   desktop-only reach?
3. Is one default provider per platform acceptable, or should selection follow
   the detected GPU vendor?
4. May Route 2 be standardized as a named adapter-side contract, or must every
   provider keep its own private bridge?
5. Which provider covers the mobile and Apple profiles, given that no current
   candidate ships there in a supported form?
6. Do we accept a research-only path for a redistributable, non-ML temporal
   upscaler on mobile (spatial upscale or an FSR 3.1-class provider) as the
   profile-appropriate answer, rather than claiming FSR 4?

## Roadmap

No ranked row yet; ranking is the user's decision. Candidate attachment: R86
(device contract) and R88 (shader library and materials) for the mechanism, R95
and R96 for the temporal use, and R91 for anything touching the products'
provider choice. V0 is dependency-ready and blocks any shipping claim; V1 to V5
are engine work that needs no vendor binary, so they can proceed while V0 is
being decided.

## Sources

- **[S1]** NVIDIA, DLSS product page: technology list, DLSS 4/4.5 transformer
  models, Multi Frame Generation on RTX 50 / RTX PRO Blackwell, DLAA, Streamline
  as the recommended integration. <https://developer.nvidia.com/rtx/dlss>
- **[S2]** NVIDIA RTX, *Streamline* 2.14.1 README and *ProgrammingGuideDLSS.md*:
  open-source cross-IHV framework, DLSS-G prebuilt-only, binary artifacts absent
  from git since 2.7.32, prerequisites, signed-DLL shipping requirement, lifecycle
  ordering, `applicationId`, `slIsFeatureSupported`, `slDLSSGetOptimalSettings`,
  resource tagging, `mvecScale`, jitter in pixel space, per-viewport resource
  allocation, preset availability, troubleshooting.
  <https://github.com/NVIDIA-RTX/Streamline> and
  `docs/ProgrammingGuideDLSS.md` in that repository.
- **[S3]** AMD FidelityFX SDK v2.3.0, *AMD FSR Upscaling 4.1.1*: signed binary
  distribution, `CS_6_6`, hardware limitation, scaling modes and jitter sequence
  lengths, input resource table, motion vector space and scale, depth flags,
  exposure and auto-exposure, optional masks and the resource-requirements query,
  placement guidance, jitter and mip-bias formulas, frame time delta, HDR and
  non-linear color flags, memory table and queries, reference timings, debug
  checker and debug view, version history.
  <https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/v2.3.0/Kits/FidelityFX/docs/techniques/super-resolution-ml.md>
- **[S4]** Intel, *Intel Simplified Software License* (October 2022) as shipped
  with the XeSS SDK: binary-form redistribution conditions, no reverse
  engineering, decompilation, disassembly or modification, third-party software,
  no support commitment.
  <https://github.com/intel/xess/blob/main/LICENSE.txt>
- **[S5]** Intel, *XeSS 3 SDK* README and *XeSS-SR Developer Guide*: binary
  contents and MSVC runtime, Windows platform requirement, SM 6.4/DP4a reach,
  Vulkan 1.1 feature and extension requirements with runtime queries, jitter in
  `[-0.5, 0.5]` shear units, motion vector formats and low-res versus dilated
  high-res, depth, exposure options, responsive pixel mask and its 0.8 clamp,
  resource layouts and the host's synchronization duty, typed input textures,
  ratio/mip-bias/jitter-length ownership, thread affinity.
  <https://github.com/intel/xess> and `doc/xess_sr_developer_guide_english.md`
  in that repository.
- **[S6]** Intel, XeSS 3 SDK tree listing (`bin/libxess.dll`, `libxess_dx11.dll`,
  `libxess_fg.dll`, `libxell.dll`, samples). <https://github.com/intel/xess>
- **[S7]** This repository's pinned FSR provider, `external/fsr411` at
  `46a56a66b639d4e3f84033640d1e51a8655ebac4`
  (`github.com/johnpanos/FSR-4.1.1-linux`): `LICENSE` (GPL-2.0),
  `NOTICE.md` (unofficial port, AMD asset extraction under AMD's MIT exception,
  "not an MIT-licensed SDK for unrestricted inclusion in a proprietary game"),
  `README.md` (Vulkan 1.3 and extension requirements, 3840x2160 output limit, no
  automatic fallback, `Fsr411::Upscaler::Record` interface).

## Proposed decision

Adopt the two-route intake, one catalog and declared input conventions as the
engine mechanism, with the licensing decision (V0) taken before any provider
ships. FSR 4.1.1 stays as the implemented reference provider and moves onto the
shared bridge contract and the catalog; it is not rewritten. Take the user's
decisions on the six open questions before V6, since they determine whether the
second provider is a redistributable one or another closed binary. Until V0 is
decided, the installed FSR provider remains an opt-in lab and development path
that no product profile enables, and no shipping claim rests on it.
