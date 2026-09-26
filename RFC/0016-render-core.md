# RFC 0016: Render Core: Device, Render Graph, GPU Scene and Materials Beneath the Legacy Material System

- Status: Proposed (2026-09-26); no implementation gate complete. The
  decisions in [Decisions](#decisions-2026-09-26) were taken by the agent
  under the user's standing instruction and can be revisited before K1
- Date: 2026-09-26
- Scope: The engine's renderer beneath the frozen material-system API: an
  explicit GPU device contract (RHI), a per-frame render graph, a persistent
  GPU scene with views and draw lists, a material and shader-family model,
  engine-owned frame and view orchestration, clustered lights and a shadow
  atlas, and one legacy frontend that runs today's `IMatRenderContext`,
  `IShaderAPI` and shader-DLL stream as passes inside that graph
- Platform: [RFC 0001](0001-capability-based-platform-architecture.md) owns
  render providers, presentation bridges, capabilities, profiles and quirks
  (R15, R16), and composition. Its render migration step 9 keeps "the
  existing shader API command model until a separate renderer RFC replaces
  it". This is that RFC
- Execution: [RFC 0003](0003-dependency-aware-job-system.md) owns CPU job
  graphs and executors. It lists "introducing a GPU render graph" as a
  non-goal; this RFC owns the GPU graph and runs its CPU work on RFC 0003
  executors
- Ownership and synchronization:
  [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md) owns
  completion tokens, bounded queues and publication rules. This RFC applies
  them; it does not restate them
- Materials and lighting: [RFC 0007](0007-physically-based-lighting-pipeline.md)
  owns the PBR family, `pbr_brdf.h`/`pbr_brdf.glsl` and the light baker.
  [RFC 0011](0011-runtime-indirect-lighting.md) owns the probe volume, the
  runtime light set and indirect producers.
  [RFC 0012](0012-antialiasing-msaa-specular-alpha-coverage.md) owns MSAA
  policy, alpha to coverage and specular AA
- Formats: [RFC 0008](0008-canonical-world-data-and-runtime-formats.md) owns
  WMSH, LMAP, PRBV, RPRB, SDFV, KTX2 and the modern model resource.
  [RFC 0015](0015-asset-identity-content-build-graph.md) owns asset identity,
  packages and live reload
- UI and tooling: [RFC 0010](0010-portable-vgui-surface.md) owns the UI
  draw list. [RFC 0014](0014-native-vulkan-and-bsp2-debug-controls.md) owns
  debug views, draw bisection, shader reload and capture controls.
  [RFC 0002](0002-hammer-responsibility-factorization.md) owns the Hammer
  viewport host (R17)
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  (Q-PRESENTATION, Q-FOUNDATION, Q-JOBS, Q-PRODUCT)

## Decision and boundary

Today the legacy material system owns the frame. `CViewRender` in the client
DLL decides the stages, `IMatRenderContext` and the shader DLLs issue
D3D9-shaped state and draws, and the native Vulkan backend translates that
stream into Vulkan. Every modern feature (the WMSH world, PBR, lightmaps,
probe volumes, SDF shadows, reflection probes, compute) reaches the device
through a string-named `QueryInterface` side channel and runs as more
methods on the same translation object. That design got native Vulkan
running on four device families, but it cannot express passes, transient
targets, GPU skinning, shadow maps, parallel recording or a second scene,
and every feature added to it makes the translation layer larger.

This RFC inverts the relationship:

1. **A render core owns the frame.** A family of strict, portable C++20
   libraries under `render/core/` and `public/render/core/` records each
   frame as a render graph over an explicit device contract. The engine,
   not the client DLL, owns frame and view orchestration.
2. **The legacy API becomes one frontend.** `IMaterialSystem`,
   `IMatRenderContext`, `IMaterial`, `IMesh`, `IShaderAPI`, the shader-DLL
   ABI and material proxies keep their vtables and behavior. Their stream is
   recorded into *legacy stream passes* at the view stage where it happens
   today. At the inversion gate (K3) every pixel family is byte-identical
   to the current backend.
3. **An explicit device contract (`render.device.v2`).** Buffers, textures,
   samplers, shader artifacts, immutable pipelines, bind-group layouts,
   command encoders and queue submission with typed completion tokens. It
   has a Vulkan provider (desktop, Android, and Apple through MoltenVK) and
   a null/recording provider. The D3D9 path is not ported; it stays the
   legacy compatibility profile on the old stack.
4. **A render graph (`render.graph.v1`).** Passes declare the resources
   they read and write. The graph orders passes, places barriers and layout
   transitions, allocates and aliases transient resources, merges passes
   for tile-based GPUs where measured, and records passes in parallel. A
   serial executor in declaration order is its oracle.
5. **A persistent GPU scene (`render.scene.v1`).** Render objects (world
   mesh groups, props, skinned models, particles, decals, lights, probes,
   view generators) are owned by one scene authority and updated by change
   sets committed at a frame boundary. Views are extracted, culled and
   turned into draw lists by jobs.
6. **Materials are data (`render.material.v2`).** A material is a shader
   family, a typed parameter block and a static permutation key. VMT files
   are imported per family. The 86 GLSL ports of `stdshader_dx9` become the
   `legacy` family, the catch-all for content no native family claims.
7. **Features are graph passes with one owner each.** Lights and shadows are
   owned here (K7). GI (RFC 0011), antialiasing (RFC 0012), UI (RFC 0010)
   and debug views (RFC 0014) plug in as passes and keep their owners.

Mods, unported code and the legacy D3D9 profiles keep calling the legacy
API indefinitely. First-party Portal and Portal 2 rendering moves to native
passes one cohort at a time, each against an oracle, until its use of the
legacy stream reaches zero.

This RFC defines contracts, owners and gates. Nothing here is installed.

## Observed starting point (2026-09-26)

Observed by reading source at `6ce100f9` plus the dirty tree. Nothing was
measured for this RFC; measured numbers are quoted from their records.

### The native Vulkan backend

- **One object does everything.** `CVulkanContext`
  (`materialsystem/shaderapivulkan/vulkan_device.h:106`, implementation
  9.5k lines in `vulkan_device.cpp`) owns the instance, device, queues,
  swapchain, memory, uploads, pipelines, descriptors, render targets,
  texture residency, frame slots and draw replay. `shaderapivulkan.cpp`
  (11k lines) implements `IShaderAPI`, `IShaderShadow` and `IShaderDevice`
  over it, including D3D9 matrix, fog and constant emulation, CPU skinning
  and SpriteCard expansion in `CEmptyMesh::EmitToNativeQueue` (:4612).
- **The frame is a record stream replayed at present.** Draws append CPU
  records and bytes to per-slot stream buffers. `IShaderDevice::Present`
  (`shaderapivulkan.cpp:1581`) calls `BeginFrame` (`vulkan_device.cpp:6412`),
  which replays every record into one command buffer, then `EndFrame`
  (:8607), which submits and presents. `Present` also reads ConVars
  (`mat_indirect_view`, `r_probevolume`, …) and pushes them into the device
  as policy. The record stream is a proto render graph with one pass kind
  and implicit resources.
- **Passes are inferred.** A pass breaks when the target or the sRGB view
  changes (`CreateAttachmentPass` :1012, the merge added for mobile GPUs).
  Render targets are always the swapchain format with their own depth
  (`CreateRenderTargetTexture` :4633). There are no float targets and no MRT
  (`shaderapivulkan.cpp:9802`).
- **Synchronization is per slot.** `kMaxFramesInFlight = 3`
  (`vulkan_device.h:1222`) with one fence per slot plus a CPU serial. Large
  uploads wait with `vkQueueWaitIdle` (`vulkan_device.cpp:1740`); world mesh
  replacement waits with `vkDeviceWaitIdle` (:6110). Only textures and
  compute resources retire by serial.
- **Memory is one allocation per resource**: 16 `vkAllocateMemory` sites in
  `vulkan_device.cpp`, plus one each in `vulkan_compute.cpp` and
  `vulkan_scene_capture.cpp`. There is no suballocator.
- **Pipelines** are keyed by D3D9-shaped raster state, the sRGB bit, sample
  count and specialization bits (`BuildMaterialPipeline` :3506). One
  template builds every family.
- **Descriptors are split three ways.** PBR and GI stages use three grouped
  sets (`vulkan_descriptor_groups.h`), legacy ports two with their own pools
  (`vulkan_legacy_pipeline.cpp`), compute its own pool. The `$phong` skin
  layout still needs seven sets (`vulkan_device.cpp:3669`). Vulkan
  guarantees only four bound sets; MoltenVK allows eight.
- **Two Vulkan stacks exist.** The `render.backend.v1` provider
  (`vulkan_render_backend.cpp`) has timeline semaphores, completion tokens
  and `DestroyResourceWhenComplete`, but it is compiled only into test
  binaries (`unittests/shaderapivulkantest/wscript:57,81`). The shipping
  path does not use it.
- **API level.** The instance requests Vulkan 1.2 when the loader has it,
  else 1.1 (`vulkan_device.cpp:240-244`). Dynamic rendering and
  synchronization2 are not used.
- **Gaps recorded in code**: 21 `VK_UNIMPLEMENTED()` sites and 58 direct
  `NoteUnimplemented` calls. Categories: line and point draws dropped,
  flashlight state, MRT, vertex textures, `CopyTextureToTexture`, separate
  alpha blend, multi-view (`AddView`), volume textures, and an empty
  `EnableAlphaToCoverage` (`shaderapivulkan.cpp:2830`).
- **Measured costs** (quoted): on headless Portal the main thread spends
  4.8 of 7.4 ms per frame in draw submission
  ([scheduler nodes](0003-scheduler-nodes-progress.md)); CPU skinning costs
  about 90 ns per vertex and GPU skinning is named "the next structural
  step" ([frame pacing](0001-native-vulkan-frame-pacing-progress.md)). The
  Apple TV 4K holds 60 fps with a thin margin, on the one map measured
  (`tvos-portal-frame-pacing-60` in `quality/budgets/render-v1.json`).

### Contracts and side channels

- `render.backend.v1` (`public/render/render_backend.h`) covers adapters,
  resource lifetime, submission and completion only. It has no pipeline,
  binding or draw vocabulary.
- `render.contracts` is written as C++11 with no `allowedEdges`, because
  the legacy material system includes it. It cannot use `foundation::Expected`
  or `StrongId`.
- Modern features reach the device through `materials->QueryInterface`:
  `"WorldMeshUpload007"` (`engine/gl_rsurf.cpp:150`,
  `engine/modelloader.cpp:4554`, `engine/indirect_light_host.cpp:513`),
  `"RenderLightSetConsumer001"` and `"RenderGpuCompute001"`, forwarded by
  `cmaterialsystem.cpp:879` through frame-ordered queue adapters
  (`materialsystem/render_capability_queue.cpp`). A queued `DrawBatch`
  reports the previous frame's verdict.

### Legacy surface and frame orchestration

- **Interfaces**: `IMaterialSystem` (`VMaterialSystem081`, ~135 virtuals),
  `IMatRenderContext` (~195), `IMaterial`, `IMaterialVar`, `ITexture`,
  `IMesh`/`IVertexBuffer`/`IIndexBuffer` and the inline `CMeshBuilder`,
  `IMaterialProxy` (`_IMaterialProxy003`), `IShaderAPI` (`ShaderApi030`),
  `IShaderDynamicAPI`, `IShaderShadow` (`ShaderShadow010`),
  `IShaderDevice`, `IStudioRender` (`VStudioRender025`), `IVRenderView`,
  `IVModelRender`, `IShadowMgr` and the mod shader ABI (`ShaderDLL004`).
- **None of these headers is in `legacyAbi.paths`** in
  `architecture/modules.json`. Only the `ShaderDLL004` extension ABI has a
  frozen-consumer fixture (`unittests/shaderextensiontest`).
- **Consumers** (grep counts, `build*` excluded): 2,582
  `pRenderContext->` calls tree-wide, 1,112 of them in `game/client`
  (Portal 141, Portal 2 264) and 492 in `engine`; 416 `CMeshBuilder` uses;
  about 80 registered material proxies.
- **The client DLL orders the frame.** `CViewRender::RenderView`
  (`game/client/viewrender.cpp:1913`) runs monitors (`_rt_Camera`), the 3D
  skybox, `ViewDrawScene` (shadow depth and RTT shadows, water reflection
  and refraction views, world, opaque renderables, leaf-ordered
  translucency with stencil portal recursion), view models, engine post
  (bloom, tone mapping), screen effects and the HUD.
- **Portal recursion** (`game/client/portal/PortalRender.cpp`,
  `game/client/portal2/portal/portalrender.cpp`) uses stencil reference =
  recursion depth, limited by `r_portal_stencil_depth` and
  `MAX_PORTAL_RECURSIVE_VIEWS`, with a texture fallback and
  `_rt_DepthDoubler`.
- **Frame-buffer copies are implicit.** Materials that need the scene
  behind them (`NeedsPowerOfTwoFrameBufferTexture`,
  `NeedsFullFrameBufferTexture`) get `UpdateRefractTexture` or
  `CopyRenderTargetToTexture` into `_rt_PowerOfTwoFB`/`_rt_FullFrameFB`
  from about twenty call sites.
- **Lighting**: world dlights are added to lightmaps on the CPU each frame
  (`R_AddDynamicLights`); models get an ambient cube plus up to
  `MAXLOCALLIGHTS` lights (`engine/lightcache.cpp`, `r_studiolight.cpp`).
  Flashlights and projected textures use `CShadowMgr` and
  `CClientShadowMgr` depth textures, which native Vulkan does not implement.

### Architecture state

- `render.vulkan.core` is a `backend` module; `shaderapivulkan.cpp`,
  `vulkan_compute.cpp`, `vulkan_shader_library.cpp`, `vulkan_mesh_layout.*`
  and `shaders/` have no module owner, and the `shaderapivulkan` target sits
  in the `render-legacy` group owned by R46.
- **The job system has no capability module.** `public/jobsystem/` and
  `jobsystem/` are unowned and the target is in the `engine-and-tiers`
  group. A strict module cannot include it without failing CAP002/CAP005.
- **Shader compiler is not pinned.** `quality/baseline.json` records
  `glslc --version` without an expected value; the SPIR-V is committed as
  generated headers (`material_spv.h`, `legacy_spv.h`).

## Goals

- One owner of the frame, in the engine, with views, passes and resources
  declared rather than inferred.
- A device contract whose shared suite runs against every provider,
  including bad ones, and whose resources are released only by completion
  tokens (RFC 0006).
- No device-wide or queue-wide idle wait on a frame path.
- A four-bind-group ceiling, so every material family runs on every Vulkan
  device the profiles declare, including MoltenVK and Android.
- Byte-identical pixels at the inversion gate; afterwards, every behavior
  change is a versioned decision with its own oracle.
- GPU skinning, shadow maps for the sun, spot lights and flashlights,
  clustered dynamic lights, float and MRT targets, and parallel command
  recording.
- A second scene in the same process (Hammer's viewport, material previews,
  thumbnails) without globals.
- Main-thread draw-submission cost reduced against its recorded budget, and
  no regression of the Apple TV 60 fps budget.
- The legacy API, content and mod shader DLLs keep working on the profiles
  that support them today.

## Non-goals

- Changing any frozen interface, content format or gameplay behavior.
- A native D3D9, D3D12 or Metal provider. Apple runs through MoltenVK. The
  device contract does not preclude another provider later; adding one
  needs its own decision and evidence.
- Porting `shaderapidx9`. The D3D9/DXVK and togl profiles stay on the old
  stack.
- Running D3D bytecode from mod shader DLLs on the native core. Those mods
  keep the legacy D3D9 profile (see [Mod shaders](#mod-shader-dlls)).
- Temporal antialiasing or upscalers (RFC 0012 keeps these out of scope).
- Mesh shaders, bindless descriptors or GPU-driven culling, until measured
  need ([Later work](#later-work)).
- A new UI toolkit, particle system or animation system. Their rendering
  moves; their simulation does not.
- Rewriting `viewrender.cpp` in one change.

## Layers and owners

| Layer | Module (proposed) | Owns | Depends on |
| --- | --- | --- | --- |
| Math | `render.math` | Float value types (`float3`, `float4`, `float4x4`, `Frustum`, `Aabb`) used by the core; no SIMD claims | foundation |
| Device contract | `render.device` | `render.device.v2`: handles, descriptors, `IRenderDevice2`, encoders, queues, completion tokens, capability and limit facts, errors | foundation, `render.contracts` (profile and quirk types) |
| Null provider | `render.device.null` | Recording provider for tests and headless tools | `render.device` |
| Vulkan provider | `render.device.vulkan` (kind `backend`) | Instance, device, queues, allocator, upload rings, pipelines and cache, bind groups, timeline completion, loss; extracted from `CVulkanContext` | `render.device`, Vulkan SDK, pinned allocator |
| Render graph | `render.graph` | `render.graph.v1`: builder, compiler (barriers, lifetimes, aliasing, merging), executors, traces | `render.device`, `jobs.graph` |
| Shader library | `render.shader-library` | Shader artifacts, permutation keys, reflected layouts, pipeline recipes | `render.device` |
| Materials | `render.material` | `render.material.v2`: families, schemas, parameter blocks, instances, VMT import | `render.shader-library`, `content.keyvalues-text`, `content.texture-contract` |
| Scene | `render.scene` | `render.scene.v1`: render objects, change sets, views, visibility providers, draw lists | `render.material`, `render.graph`, `render.math`, `jobs.graph` |
| Feature passes | `render.pass.*` (for example `render.pass.shadows`, `.lights`, `.skinning`, `.world`, `.post`, `.ui`) | One feature's passes and GPU resources | `render.scene`, `render.graph`, `render.material`; each feature's own contract module (RFC 0011, 0012, 0010) |
| Renderer | `render.renderer` | `render.view.v1`: `IRenderer`, frame and view descriptions, stage hooks, composition of feature passes | the layers above |
| Legacy frontend | `render.legacy-frontend` (kind `legacy-interop`) | `IShaderAPI`/`IShaderShadow`/`IShaderDevice` over the core, legacy stream passes, the named render-target registry, frame-buffer copy semantics | `render.renderer`, legacy material and shader headers |
| Applications | engine and client roots, Hammer, tools | Selecting providers and profiles, composing the renderer | the layers above |

- **Arrows point down.** No core module includes a legacy header, the
  engine, the client or an application. The legacy frontend is the only
  module that sees both worlds, and it is `legacy-interop`.
- **Strict on both axes.** Every portable core module is a `cxx20` target
  in `quality/toolchain/policy.json` *and* builds with
  `features='cxx capability_strict'` and `env=bld.strict_cpp20_env()`,
  declares `arch_module`, and passes CAP002, CAP005, CAP006, CAP007 and
  CAP008. `mapcontainer` has the dialect without the hygiene; the core must
  have both.
- **Static libraries.** Each module is a Waf static library (CAP009). The
  Vulkan provider links into the existing `shaderapivulkan` product until
  K9, then into the renderer's product library.
- **Vocabulary.** Core APIs use `foundation::Expected`, `foundation::Error`,
  `StrongId` and `ScopedResource`. `render.contracts` stays C++11 and
  ABI-facing; the core does not add vocabulary types to it, and CAP010 keeps
  foundation types out of preserved-ABI headers.
- **Dedicated servers** link no core module (R12 link evidence).
- **Prerequisite:** `public/jobsystem/` and `jobsystem/` get a capability
  module (`jobs.graph`) with strict hygiene before `render.graph` depends on
  it (K0).
- **Math.** `render.math` is private to the render core. Hammer's
  `mapgeometry` types keep their editor tolerances and double precision
  (AGENTS.md DRY rule). The legacy frontend converts from `Vector` and
  `VMatrix`. A shared foundation math module is a later decision once a
  second strict consumer needs the same float types.

## Device contract (`render.device.v2`)

`render.backend.v1` stays as the lifetime subset; its suite becomes part of
the v2 suite. v2 adds everything a renderer needs to record work.

```cpp
// Proposed; spellings are fixed by the first implementation.
namespace render::device
{
using BufferId = foundation::StrongId<struct BufferTag, uint64_t>;   // generation in high bits
using TextureId = foundation::StrongId<struct TextureTag, uint64_t>;
using PipelineId = foundation::StrongId<struct PipelineTag, uint64_t>;
using BindGroupLayoutId = foundation::StrongId<struct BindGroupLayoutTag, uint64_t>;

class IRenderDevice2
{
public:
	virtual const DeviceFacts &Facts() const = 0;                 // caps and limits; immutable
	virtual Expected<BufferId, DeviceError> CreateBuffer( const BufferDesc & ) = 0;
	virtual Expected<TextureId, DeviceError> CreateTexture( const TextureDesc & ) = 0;
	virtual Expected<PipelineId, DeviceError> CreatePipeline( const PipelineDesc & ) = 0;
	virtual Expected<BindGroupLayoutId, DeviceError> CreateBindGroupLayout(
	    const BindGroupLayoutDesc & ) = 0;
	virtual void Release( ResourceId, CompletionToken releaseAfter ) = 0;
	virtual Expected<CommandEncoder, DeviceError> BeginEncoder( QueueKind ) = 0;
	virtual Expected<CompletionToken, DeviceError> Submit( QueueKind,
	    std::span<CommandEncoder> encoders, const SubmitWaits & ) = 0;
	virtual bool IsComplete( CompletionToken ) const = 0;
	virtual DeviceState State() const = 0;                        // available, lost, recovering, fatal
};
}
```

- **Descriptors are values and are immutable.** A pipeline is created from
  shader artifacts, a bind-group layout list, vertex input, raster, depth,
  stencil and blend state, attachment formats and sample count. Nothing is
  mutable after creation; there is no D3D9-style state machine at this
  boundary.
- **Four bind groups at most**, in a fixed role order: frame, view,
  material, draw. That fits Vulkan's guaranteed minimum
  (`maxBoundDescriptorSets` ≥ 4) and therefore every declared profile. A
  family that needs more fails its build-time validation, not a device.
- **Completion tokens** carry a queue, a timeline value and the device
  epoch (RFC 0006). `Release` never frees before its token completes. After
  device loss, tokens from the old epoch are invalid and cannot release
  new-epoch resources.
- **Upload rings** are provider-owned. Each occupied range records the
  token that allows its reuse. Exhaustion defers the upload to the next
  submission and counts it; it never overwrites and never waits for the
  device to go idle. Large uploads use a transfer queue when the device has
  one, and the graphics queue otherwise, with the same token rule.
- **Memory** is suballocated by the provider from typed heaps, and each
  heap reports its budget and use. The allocator also serves the graph's
  transient aliasing.
- **Encoders.** One encoder is recorded by one thread at a time. Encoders
  may be recorded in parallel and are submitted in the order given to
  `Submit`. Diagnostic builds check sequence ownership with
  `platform::SequenceChecker`.
- **Queues.** Graphics is required. Compute and transfer are optional
  capabilities; the graph falls back to the graphics queue when they are
  absent.
- **Facts, profiles and quirks** follow RFC 0001: facts are immutable,
  profiles are policy chosen by the composition root, quirks are data with
  a reason and a provider range. `Present` no longer reads ConVars; policy
  arrives through the renderer's frame description.
- **Required Vulkan features** for the core: timeline semaphores,
  synchronization2 and dynamic rendering (each core in Vulkan 1.3 and
  available as an extension earlier). K1 records each declared profile's
  support, including MoltenVK on the iPhone and Apple TV and the Fold7. A
  profile that lacks one fails composition with a structured error. It
  does not fall back to the old stack silently.
- **Presentation** stays `render.presentation.v1` (R16). The swapchain
  image is an imported graph resource; the bridge keeps native handles.

### Shared suite and bad providers

One suite runs against the Vulkan provider, the null provider and fakes. It
includes the `render.backend.v1` cases. Each deliberately bad provider must
fail a named clause:

- recycles an upload range or releases a resource before its token
  completes;
- reports different facts after creation;
- completes submissions out of order on one queue;
- accepts a token from a previous device epoch;
- creates a pipeline whose layout does not match its shader artifact's
  reflected bindings;
- leaks a partially created resource when creation fails;
- accepts a fifth bind group;
- allows two threads to record one encoder without a diagnostic.

## Render graph (`render.graph.v1`)

### Building

A graph is built each frame on the render sequence:

- **Passes**: graphics, compute, copy, present, and *host* (a CPU callback
  that records into an encoder the graph provides; the legacy stream pass is
  one). Each pass has a name, a queue preference and a record function.
- **Resources** are *imported* (the swapchain image, persistent textures and
  buffers, history targets, named legacy render targets) or *transient*
  (created from a description for this frame only).
- **Accesses** declare read or write, the usage (sampled, storage, uniform,
  vertex, index, indirect, color attachment, depth read, depth write,
  resolve, copy source, copy destination, present) and a subresource range.
  A write creates a new version of the resource.
- **Side effects.** A pass that writes an imported resource, presents or
  reads back is never culled.

### Compiling

- **Validation.** A resource version has one writer; nothing reads an
  undefined version; there is no cycle; accesses match the resource's
  declared usages. A violation is an error value that names the pass.
- **Order.** Passes execute in declaration order. The compiler may move a
  pass earlier only onto another queue, and only when the graph proves it
  independent. Declaration order is therefore also the serial oracle.
- **Barriers and layouts** are computed from consecutive accesses per
  subresource with synchronization2 stages and access masks.
- **Transient lifetimes and aliasing.** Transient resources get memory from
  aliased heaps by lifetime interval, with the barrier that aliasing
  requires.
- **Culling** removes passes whose outputs nothing reads, unless they have
  side effects.
- **Pass merging.** Adjacent graphics passes with compatible attachments
  may be merged into one rendering scope, the job the current sRGB/UNORM
  merge does by inference. On the Fold7 the existing merge cut passes from
  31 to 5 without changing GPU time
  ([frame pacing record](0001-native-vulkan-frame-pacing-progress.md#mobile-gpu-cost-render-pass-breaks-2026-09-23)),
  so merging is a measured, per-profile option, not an assumed win.
- **Caching.** A compiled graph is reused while the frame's shape (passes,
  resources, descriptions) is unchanged, the way `DeclaredFrameGraph`
  reuses its sealed graph.

### Executing

- **Recording** runs passes as jobs on the RFC 0003 executors: each pass or
  group of passes records its own encoder. Submission order is
  declaration order. The serial executor records everything on the render
  sequence and is the oracle and the low-capacity mode.
- **Views are subgraphs.** A view (the main view, the 3D skybox, a water
  reflection, a monitor, a portal) contributes a subgraph with declared
  inputs and outputs. Recursive views (portals, mirrors) are nested
  subgraphs with a declared depth limit.
- **Traces.** Every compile can emit a trace (passes, resources, versions,
  barriers, aliasing, merges, timings) that RFC 0014's debug controls and
  RenderDoc labels consume.

### Oracle

- An independent model computes barriers, lifetimes and alias safety from
  the same declarations and must agree with the compiler over seeded random
  graphs, as RFC 0003's graph model does for jobs.
- The Vulkan validation layer's synchronization checks report zero
  messages on the pixel families and Portal boots.
- Bad graphs must be caught: a missing barrier, overlapping live aliases,
  a culled side-effect pass, a reordered dependent pass, a read of an
  undefined version.

## GPU scene (`render.scene.v1`)

### Authority and updates

- A `RenderScene` owns the render objects of one world. The engine creates
  one per loaded map; Hammer (R17) and material previews create their own.
  No scene is global.
- Objects are typed handles: `MeshInstance` (world mesh groups, static
  props, brush models), `SkinnedInstance` (studio models: mesh, bone
  palette, flex weights), `ParticleDraw` (vertex streams the particle
  system produces), `Decal`, `Light`, `ReflectionProbe`, `ProbeVolume`
  (RFC 0011), `Sky`, `FogVolume`, and *view generators* (`Portal`,
  `Mirror`, `Monitor`, `WaterSurface`).
- **Change sets.** The engine and game record changes (create, destroy,
  transform, material, bone palette, visibility flags) during the frame and
  commit them at the render-extract boundary of the host frame graph. The
  scene publishes an immutable frame snapshot to the render sequence under
  RFC 0006's release/acquire rule. In queued mode the snapshot replaces the
  per-call copies that `render_capability_queue.cpp` makes today.
- **Derived GPU data** (instance buffers, bone palettes, light lists)
  carries the scene revision it was built from and is rebuilt only from the
  committed change set.

### Visibility and draw lists

- **Visibility providers** are injected: BSP leaf and area-portal
  visibility (the engine), WMSH cluster and occlusion culling (moved from
  `gl_rsurf.cpp`'s `worldmesh_cull`), and later RFC 0008 F8's USD-native
  visibility. The scene does not own BSP.
- **Per view**, jobs run visibility, frustum and occlusion culling, then
  build draw lists per pass kind (depth, opaque, translucent, shadow,
  capture) with sort keys. The serial path is the oracle; culled sets must
  match the legacy path's sets on captured views.
- **Translucency** keeps the legacy leaf order for content that depends on
  it: world translucency and renderables interleave per leaf, back to front.
  A native sort key replaces it only per cohort, as a recorded behavior
  change with an image oracle.
- **Instancing** merges draws with the same mesh, material and pipeline
  where the family allows it. It is measured, not assumed.

### Skinning

- Compute skinning writes per-frame skinned vertex buffers from bone
  palettes; flex and morph targets run as compute before skinning.
- The CPU path (`R_StudioSoftwareProcessMesh*`, today's emit skinning)
  stays as the oracle and for materials that need software skinning.
  Equivalence is judged per vertex with a declared tolerance.

## Materials (`render.material.v2`)

- **A family** declares a parameter schema (names, kinds, color encodings,
  defaults; RFC 0007's `pbr_material_schema.h` is the model), the pass
  kinds it supports, its static permutation axes, its material bind-group
  layout and its render-state rules.
- **A material instance** is a family, a parameter block and a static
  permutation key. It has a revision. Dynamic parameters are written into a
  per-frame block, never into shared state.
- **VMT import** is per family. The importer maps VMT keys to the family's
  schema, adds `materials/` prefixes, and reports unknown keys. It is also
  the material compiler RFC 0015 C1 calls `material.vmt`, so runtime and
  build share one mapping.
- **Initial families**, each with a pixel oracle against its legacy port:
  `pbr` (RFC 0007), `lightmapped` (LightmappedGeneric,
  WorldVertexTransition), `vertexlit` (VertexLitGeneric, including skin),
  `unlit` (UnlitGeneric, Sprite), `refract` (Refract, PortalRefract, glass),
  `water`, `sky`, `eyes` (Eyes, EyeRefract, Teeth), `spritecard`, `post`,
  and `legacy`. The `legacy` family runs the 86 ports keyed by program; it
  exists so every shipped material renders while native families arrive.
- **Material proxies.** `IMaterialProxy::OnBind` keeps its timing: the
  legacy frontend calls proxies when a renderable's draw is extracted, which
  is when the legacy path binds the material. `IMaterialVar` writes land in
  the renderable's parameter block for that draw.
- **Shader artifacts.** GLSL stays the source language. A pinned compiler
  builds SPIR-V artifacts offline, reflection checks each artifact against
  its family's declared layout, and artifacts are keyed by source, compiler
  identity and permutation. Shipping products never compile shaders.
  Development compositions may reload them (RFC 0014 D-phase).
- **Missing shaders.** A material whose shader no family or legacy port
  implements is reported when the material loads, with the material name,
  and is drawn with the error family. Today such draws are dropped and
  counted in a census. Content validation (RFC 0015) reports it at build
  time.

### Mod shader DLLs

Mod shader DLLs (`ShaderDLL004`) ship D3D shader bytecode. The core cannot
run it, and translating DXBC is out of scope. On native profiles a mod
shader DLL loads through the existing extension host, its shaders register
as `unsupported-on-profile`, and their materials follow the missing-shader
rule. The D3D9/DXVK profile keeps running them. This is recorded as a
profile limitation, not hidden.

## Frame and views (`render.view.v1`)

- The engine owns `IRenderer::RenderFrame(const FrameDesc &)`. A frame
  description lists views, their stages and the frame's policy (indirect
  view, probe sampling, debug view, sample count), replacing the ConVar
  reads in `Present`.
- **Stages** name the legacy order: monitors, 3D skybox, shadow depth,
  water reflection, water refraction, world opaque, renderables opaque,
  translucent, portals, view models, post, screen effects, HUD.
- **Migration path.** `CViewRender::RenderView` keeps its code. At K3 each
  of its stages opens a stage on the current view, and everything it draws
  lands in that stage's legacy stream pass. From K5 on, stages switch to
  native passes one cohort at a time, and `CViewRender` stops drawing what
  the scene now draws.
- **Stage hooks.** Game code adds passes through `IRenderStageHooks` with
  declared accesses (Portal's portal views, Portal 2 paint). A hook cannot
  reach the device except through the graph.
- **Portals.** Until the portal cohort (K8) moves, portal views run through
  the legacy stencil path inside the translucent stage, bit-exact. The
  native path makes portals and mirrors view generators whose subgraphs
  render to per-level targets or stencil. It keeps the recursion limit and
  the texture fallback's behavior.
- **Frame-buffer copies become explicit.** When a legacy material needs
  `_rt_FullFrameFB` or `_rt_PowerOfTwoFB`, the frontend ends the pass and
  declares a copy pass at the same point in the stream. The graph trace
  shows each copy, and a native family that needs the scene reads the
  graph's scene-color resource instead.

## Lights and shadows (`render.lights.v1`, `render.shadows.v1`)

- **One light authority.** Clustered lighting consumes RFC 0011's
  `render.light-set.v1` (and spark lights) as the only runtime light list.
  A froxel grid per view is filled by compute, and families read it through
  the view bind group.
- **Legacy dlights.** World dlights are added to lightmaps on the CPU
  today. On native world families they are evaluated per pixel. That is a
  visible change, so it ships as a versioned behavior decision with an image
  oracle and a switch until accepted.
- **Shadow atlas.** Spot lights and flashlights (closing the
  `SetFlashlightState` stubs and R32-VIDEO-OPTIONS P7), a directional
  cascade set for the map's sun, and optional point-light cube shadows.
  RTT blob shadows stay for legacy renderables. Each profile has an atlas
  size and caster budget.
- **Ownership.** RFC 0008 F5's clustered dynamic lights are delivered here
  (K7). RFC 0011 keeps producers and the light set.

## Threading

| Sequence | Work |
| --- | --- |
| Game/main | Scene change sets, `FrameDesc`, legacy calls into `IMatRenderContext` (queued or immediate as today) |
| Render | Scene snapshot acquire, graph build and compile, legacy frontend replay, submission (the `MatQueue` thread in `mat_queue_mode 2`, the main thread in mode 0) |
| Compute pool | Culling, draw-list builds, pass recording, parallel emit conversion |
| GPU | Queues; completion observed only through tokens |

- `CMatQueuedRenderContext` is unchanged. The frontend runs on the render
  sequence, where the device work runs today, so queued ordering is kept.
- Nothing recycles by frame index; per-slot fences are replaced by
  timeline tokens.
- The serial configuration (one thread, serial executors) remains a
  supported low-capacity mode and the oracle for every parallel path.

## Compatibility surface

**Frozen** (vtable order, version strings, observable behavior):

- `IMaterialSystem` (081), `IMatRenderContext`/`CMatRenderContextPtr`,
  `IMaterial`, `IMaterialVar`, `ITexture`, `IMesh`, `IVertexBuffer`,
  `IIndexBuffer`, `CMeshBuilder` and `MeshDesc_t` layout;
- `IMaterialProxy` (003) and its factory; `IMaterialSystemHardwareConfig`
  (012);
- `IStudioRender` (025), `IVRenderView` (014), `IVModelRender` (016),
  `IShadowMgr`;
- the mod shader ABI: `ShaderDLL004`, `IShaderDLL`, `CBaseShader`,
  `CBaseVSShader`, `IShaderShadow` (010), `IShaderDynamicAPI`,
  `IShaderInit`, `IShaderUtil` and the `.inc` combo contract;
- content: VMT, VTF, `.vcs`, the named render targets (`_rt_FullFrameFB`,
  `_rt_PowerOfTwoFB`, `_rt_Camera`, `_rt_Water*`, `_rt_Portal*`,
  `_rt_DepthDoubler`, `_rt_Shadows`), and material flags such as
  `NeedsPowerOfTwoFrameBufferTexture` and `NeedsSoftwareSkinning`;
- draw order on the legacy path: the stage order, leaf-ordered
  translucency, stencil portal recursion (reference = depth), frame-buffer
  copy timing, and queued-mode ordering.

**Free to replace** (internal): the `IShaderAPI` and `IShaderDevice`
implementations, `CShaderSystem`, `CMaterialSystem` internals,
`CMatRenderContext`, `CTexture` internals, the transition table, dynamic
VB/IB management, `render_capability_queue`, studiorender's skinning
internals, and the `QueryInterface` side channels (which are not frozen).

**First-party callers** of the legacy API (the ~1,100 `pRenderContext->`
calls in `game/client`, the 492 in `engine`) are migration targets, not
compatibility obligations. The API stays for them until each cohort moves.

## Delivery plan and gates

Each gate needs negative controls, recorded evidence (revision, profile,
inputs, counts, first divergence, reproduction commands) and the AGENTS.md
reporting rules. Pixel comparisons are exact where the gate says
byte-identical and use versioned per-case tolerances elsewhere.

### K0: Prerequisites and frozen oracles

- Give `public/jobsystem/` and `jobsystem/` a `jobs.graph` capability module
  with strict hygiene.
- Pin the GLSL compiler (shaderc) in the dependency pins, record its
  identity in `quality/baseline.json` with an expected value, and
  regenerate the committed SPIR-V to prove the pin reproduces it.
- Add the frozen material, shader-API and studio headers above to
  `legacyAbi.paths` (CAP010), and add vtable-slot fixtures for
  `IMaterialSystem`, `IMatRenderContext`, `IMaterial`, `IMaterialVar`,
  `ITexture`, `IMesh`, `IMaterialProxy` and `IStudioRender` built in the
  `legacy-cxx11` dialect.
- Capture the view oracles the later gates need: Portal recursion at each
  depth, water reflection and refraction, a monitor, glass (the existing
  glass oracle), the legacy-ports view set, and per-draw state fixtures on
  `testchmb_a_00`, `testchmb_a_08` and one Portal 2 map.
- Record R32-RENDER-BUDGETS rows for desktop Wayland and the Fold7,
  including main-thread submission time.
- Gate: the fixtures reject a seeded vtable reorder; the pin reproduces the
  committed SPIR-V byte for byte; each view oracle detects a seeded
  single-draw defect.

### K1: Device contract and Vulkan provider

- Deliver `render.device.v2`, the null provider, the shared suite and the
  bad providers.
- Extract `render.device.vulkan` from `CVulkanContext` and the test-only
  `vulkan_render_backend.cpp`: one allocator, token-gated upload rings,
  pipelines and cache, bind-group layouts with the four-group ceiling,
  timeline completion and loss. `CVulkanContext` then allocates, uploads
  and retires through it, so the shipping path and the contract are one
  stack.
- Remove `vkQueueWaitIdle` and `vkDeviceWaitIdle` from frame paths (world
  mesh replacement and large uploads use tokens); idle waits remain only in
  teardown, mode changes and loss recovery, each listed.
- Gate:
  - The shared suite passes on Vulkan and null; every bad provider is
    caught.
  - All material pixel families are byte-identical to K0.
  - `portal_boot` passes in both queued modes, and `--resize-stress`
    passes on Wayland and X11.
  - The idle-wait inventory matches its reviewed list.
  - Feature support (timeline, synchronization2, dynamic rendering) is
    recorded for every declared Vulkan profile, with a device run on the
    Fold7 and, where the runner is available, the iPhone and Apple TV.
  - The Apple TV 60 fps budget still passes when its runner is available.
- K1 supplies the resource, upload and synchronization contract evidence
  R32's done condition names.

### K2: Render graph

- Deliver `render.graph.v1`, its compiler, serial and pooled executors,
  traces, the independent model and the bad graphs.
- Move the passes the current backend runs outside the record stream onto
  the graph: present blit and gamma, MSAA resolve, scene capture, and
  queued compute.
- Gate: the model agrees on seeded graphs; every bad graph is caught; the
  validation layer's synchronization checks are silent on the pixel
  families; pixels are byte-identical; serial and pooled recording give
  identical command streams.

### K3: Inversion

- Deliver `render.legacy-frontend`: `IShaderAPI`, `IShaderShadow` and
  `IShaderDevice` over the core; the D3D9 state translation moved out of
  the device; legacy stream passes per stage; the named render-target
  registry (float targets and MRT now supported); explicit frame-buffer
  copy passes.
- Deliver `render.renderer` and `IRenderer` with stage markers, and wire
  `CViewRender` stages to them.
- Replace the three `QueryInterface` side channels with typed services the
  composition root injects. Delete `render_capability_queue`'s adapters
  when their consumers use scene change sets or the injected services.
- Gate:
  - All pixel families and K0 view oracles are byte-identical.
  - Per-draw state fixtures match K0.
  - Portal and Portal 2 boot in both queued modes; resize-stress passes.
  - No `materials->QueryInterface` render side channel remains.
  - `CVulkanContext`'s record replay has no caller.
  - Frame time is within the K0 budget allowance.

### K4: Shader library and materials

- Deliver `render.shader-library` and `render.material.v2`; move the
  `legacy` and `pbr` families onto them; add `lightmapped`, `vertexlit` and
  `unlit` with VMT importers.
- Gate: each family matches its legacy port's pixels within its recorded
  tolerance; the proxy corpus (every proxy registered in Portal and Portal 2
  exercised on a fixture material) matches; a seeded wrong schema mapping
  and a stale-revision parameter block are detected; the four-group
  validation rejects a five-group family.

### K5: Scene, views, world and static props

- Deliver `render.scene.v1` and `render.view.v1` change sets and snapshots;
  move the WMSH world, brush models and static props onto scene passes;
  move WMSH culling into a visibility provider.
- Gate: culled sets match the legacy path on captured views; world and prop
  pixels match within tolerance on the K0 views; serial and pooled culling
  agree; main-thread submission time improves against its K0 budget;
  Hammer creates a second scene in the same process in a test.

### K6: Skinned models

- Move studio models to `SkinnedInstance`; deliver compute skinning, flex
  and morph; add the `eyes` family.
- Gate: skinned vertices match the CPU oracle within tolerance; the
  skinning and model-light pixel families pass; seeded bone-palette and
  flex-weight defects are detected; the skinning cost is recorded against
  the quoted 90 ns per vertex.

### K7: Lights and shadows

- Deliver clustered lights over the light set, the shadow atlas (spot,
  flashlight, sun cascades, optional point), and the dlight behavior
  decision.
- Gate: analytic light-count and froxel tests; shadow oracles (a caster
  occludes, a non-caster does not, cascade seams bounded); flashlight
  scenes in Portal render with shadows on native; per-profile atlas budgets
  pass on desktop and the Fold7.

### K8: Remaining cohorts

- Particles (`spritecard`), decals and overlays, sprites, beams and ropes,
  post and screen effects, the UI draw list as a graph pass (RFC 0010 V6's
  native consumer), water, sky, and portals, mirrors and monitors as view
  generators.
- Gate, per cohort: its pixel families and K0 views pass; its legacy stream
  use on Portal and Portal 2 is zero in the inventory; portal recursion
  matches at every depth.

### K9: Retirement

- First-party legacy-stream use reaches zero on the native Portal and Portal
  2 profiles, enforced by an inventory ratchet.
- Delete the D3D9-shaped code in `CVulkanContext` and `shaderapivulkan.cpp`
  that has no remaining caller. The legacy frontend stays for mods and the
  shader-DLL ABI.
- Gate: the ratchet holds; the frozen ABI fixtures pass; a mod-style fixture
  (a legacy client DLL drawing through `IMatRenderContext`) renders on the
  native profile.

**Dependencies.** K0 is ready now (R02, R05, R10 and R16 are done). K1 needs
K0. K2 needs K1 and `jobs.graph`. K3 needs K2. K4 needs K3. K5 needs K4.
K6 and K7 need K5 and are independent of each other. K8 needs K5, and its UI
cohort needs RFC 0010's draw list. K9 needs K6, K7 and K8.

## Roadmap

AGENTS.md owns ranks and states. Agent decision under the user's standing
instruction, 2026-09-26: rows are added as `planned` and ranked directly
after R47, because the user asked for a real graphics system and every
later render row (R65, R56, R50, R63, R36) would otherwise be built twice,
once on the translation layer and again on the core.

| Phases | Row |
| --- | --- |
| K0–K1 | R86 |
| K2–K3 | R87 |
| K4 | R88 |
| K5–K6 | R89 |
| K7 | R90; R56 depends on it for clustered dynamic lights |
| K8–K9 | R91 |

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The inversion changes pixels in ways the families don't cover | K0 adds view oracles and per-draw state fixtures before K1; K3 requires byte identity on all of them |
| Material proxy timing changes | Proxies run at draw extraction, the legacy bind point; the proxy corpus is a K4 gate |
| Portal recursion breaks | Portals stay on the legacy stencil path until K8; K0 captures every recursion depth |
| Queued-mode ordering breaks | `CMatQueuedRenderContext` is unchanged; the frontend runs where device work runs today; both modes are in every gate |
| The core grows a second D3D9 state machine | The device contract has immutable pipelines only; the D3D9 translation lives in the frontend and shrinks as cohorts move |
| Mobile GPUs regress | Pass merging is per-profile and measured; the Fold7 is in K1, K5 and K7 gates; the Apple TV budget is kept |
| Required Vulkan features are missing on a profile | K1 records support per profile; a missing feature fails composition with a structured error |
| Parallel recording or culling races | Serial oracles for every parallel path; TSan lanes for the pooled executors |
| Two authorities during migration | Each cohort switches its stage from the legacy stream to the scene in one change; no object is drawn by both |
| Scope creep toward a new engine | Non-goals exclude new UI, particle and animation systems and GPU-driven rendering; each K gate is bounded |

## Alternatives considered

### Keep extending `CVulkanContext`

This is what got native Vulkan running. Every new feature adds a record
kind, a descriptor pool or a side channel, and nothing can express transient
targets, parallel recording, a second scene or explicit pass order. The
D3D9 state machine stays at the center. Rejected as the long-term owner;
kept as the source of the provider's extracted code.

### SDL_GPU as the device layer

SDL3 is already the platform stack and SDL_GPU covers Vulkan, Metal and
D3D12. It does not expose timeline tokens, memory aliasing or ray query,
which RFC 0006 and RFC 0011 need, and its resource model would sit between
the graph and Vulkan. Rejected for the core; a future portability provider
could implement `render.device.v2` over it if a profile needs one.

### An existing RHI library (NVRHI, Diligent, bgfx)

Each would add a large dependency with its own lifetime and threading
model beneath contracts this repository already defines, and none owns the
legacy frontend problem, which is most of the work. Rejected.

### Replace the material system outright

Breaks mods, content and the frozen interfaces AGENTS.md preserves.
Rejected.

### Keep translating D3D9 state, as DXVK does

This is today's design, and DXVK itself remains the legacy profile. A
translation layer cannot add shadows, GPU skinning or a scene without
inventing hidden passes. Rejected as the native path.

## Decisions (2026-09-26)

Taken by the agent under the user's standing instruction ("do what you deem
as recommended for long-term improvements"). Each favors one owner and no
later migration over the cheapest first slice.

### 1. Vulkan-only core

The core has one real provider. MoltenVK covers Apple, and D3D9 stays on
the old stack as the legacy profile. The device contract is written so that
another provider could implement it, but none is planned. Rationale: the
user directed native Vulkan focus (2026-09-24), and every north-star target
runs Vulkan.

### 2. Four bind groups

Frame, view, material, draw. It is the Vulkan guaranteed minimum, it fits
MoltenVK and Android without per-profile layouts, and the existing grouped
PBR stages already fit in three. The seven-set `$phong` layout moves to the
`vertexlit` family's four-group layout in K4.

### 3. Allocator: Vulkan Memory Allocator, pinned

VMA is the standard Vulkan suballocator (MIT), supports aliasing and budget
queries, and runs on MoltenVK and Android. It is pinned in the dependency
list and private to `render.device.vulkan`. Writing one is not justified.

### 4. Dynamic rendering, synchronization2 and timeline semaphores are required

They remove render-pass object management, give the graph one barrier
model and give tokens one mechanism. Support is recorded per profile at K1
before any code relies on it.

### 5. GLSL stays the shader language

There are 47 core shaders and 86 ports in GLSL, and `pbr_brdf.glsl` is
checked against `pbr_brdf.h`. Slang or HLSL would add a second language and
a second compiler pin for no current need.

### 6. The legacy frontend runs on the render sequence

That is where device work runs in both queued modes today, so ordering and
the thread-ownership census stay valid.

### 7. Portals move last

Portal recursion is the highest-risk legacy behavior and the most visible.
It stays on the bit-exact legacy path until the scene, materials and
lights are proven (K8).

## Later work

- GPU-driven culling, indirect draws and bindless material tables, after
  K5's measurements show submission still dominates.
- Mesh shading where profiles support it, after GPU-driven culling.
- A shared foundation math module, when a second strict consumer needs
  `render.math`'s types.
- Asynchronous compute scheduling beyond the optional queue fallback, per
  measured profile.

## Amendments to other RFCs

- **RFC 0001**: render migration step 9's "separate renderer RFC" is this
  RFC. The legacy render-services adapter (step 7) is the legacy frontend's
  input side.
- **RFC 0003**: the GPU render graph it excludes is owned here; its
  executors run the graph's CPU work.
- **RFC 0008**: F5's clustered dynamic lights are delivered by K7; F4's
  world path runs as scene passes from K5.
- **RFC 0010**: V6's optional native UI consumer is a graph pass on the
  core (K8).
- **RFC 0012**: MSAA targets, alpha to coverage and specular AA are
  implemented on the core's pipelines and graph resources; the empty
  `EnableAlphaToCoverage` stub is closed there, not in the old device.
- **RFC 0014**: debug views read graph traces and scene handles; draw
  bisection works on draw lists.

## Source references

- Yuriy O'Donnell, "FrameGraph: Extensible Rendering Architecture in
  Frostbite", GDC 2017.
- Hans-Kristian Arntzen, "Render graphs and Vulkan — a deep dive" (Granite
  blog, 2017).
- Unreal Engine documentation, "Render Dependency Graph".
- Ola Olsson, Markus Billeter and Ulf Assarsson, "Clustered Deferred and
  Forward Shading", HPG 2012.
- Rouslan Dimitrov, "Cascaded Shadow Maps", NVIDIA, 2007.
- Khronos, Vulkan specification, "Required Limits" (`maxBoundDescriptorSets`
  minimum 4), `VK_KHR_dynamic_rendering`, `VK_KHR_synchronization2`,
  timeline semaphores.
- GPUOpen, Vulkan Memory Allocator.
- KhronosGroup/MoltenVK, portability subset documentation.

## Proposed decision

Adopt the render core as the owner of the frame beneath the frozen material
API. Deliver it through K0–K9 with the inversion at K3 held to byte-identical
pixels. Keep the legacy frontend permanently for mods and the legacy
profiles. Add rows R86–R91 as `planned`, ranked after R47.
