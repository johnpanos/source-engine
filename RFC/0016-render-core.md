# RFC 0016: Render Core: Device, Render Graph, GPU Scene and Materials Beneath the Legacy Material System

- Status: Proposed (2026-09-26); no implementation gate complete. Revised
  the same day at the user's direction: the device contract serves several
  backends (Vulkan first, OpenGL second), ToGL stays for mods, and the RFC
  now fixes the directory layout, the ports and the layer contract, and
  states each gate as objective tests. The decisions in
  [Decisions](#decisions-2026-09-26) can be revisited before K1.
  Implementation started the same day: the layout, the layer contract and
  the runtime wiring exist as the R86-LAYOUT slice, recorded in the
  [progress record](0016-progress.md). Where that slice settled a proposed
  spelling differently, this RFC now says what was built. Every K0 check
  passes (2026-09-26), with the Portal 2 monitor view declared absent: the
  Portal 2 client compiled monitors out (drawn since 2026-09-29; the K0
  Portal 2 scenario, `sp_a1_wakeup`, has no camera)
- Date: 2026-09-26
- Scope: The engine's renderer beneath the frozen material-system API: an
  explicit, backend-neutral GPU device port with Vulkan, OpenGL and null
  adapters; a per-frame render graph; a persistent GPU scene with views and
  draw lists; material families; engine-owned frame and view orchestration;
  clustered lights and a shadow atlas; and one legacy frontend that runs
  today's `IMatRenderContext`, `IShaderAPI` and shader-DLL stream as passes
  inside that graph
- Platform: [RFC 0001](0001-capability-based-platform-architecture.md) owns
  render providers, presentation bridges, capabilities, profiles and quirks
  (R15, R16), and composition. Its render migration step 9 keeps "the
  existing shader API command model until a separate renderer RFC replaces
  it". This is that RFC. Its step 10 ("validate a genuinely different
  backend through the same provider and conformance contracts") is K10
- Execution: [RFC 0003](0003-dependency-aware-job-system.md) owns CPU job
  graphs and executors. It lists "introducing a GPU render graph" as a
  non-goal; this RFC owns the GPU graph and runs its CPU work on RFC 0003
  executors
- Ownership and synchronization:
  [RFC 0006](0006-modern-cpp-ownership-and-synchronization.md) owns
  completion tokens, bounded queues and publication rules. This RFC applies
  them; it does not restate them
- Materials and lighting: [RFC 0007](0007-physically-based-lighting-pipeline.md)
  owns the PBR family's semantics, `pbr_brdf.h`/`pbr_brdf.glsl` and the
  light baker. [RFC 0011](0011-runtime-indirect-lighting.md) owns the probe
  volume, the runtime light set and indirect producers.
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
  (Q-ARCH, Q-PRESENTATION, Q-FOUNDATION, Q-JOBS, Q-PRODUCT)

## Decision and boundary

Today the legacy material system owns the frame. `CViewRender` in the client
DLL decides the stages, `IMatRenderContext` and the shader DLLs issue
D3D9-shaped state and draws, and the native Vulkan backend translates that
stream into Vulkan. Every modern feature (the WMSH world, PBR, lightmaps,
probe volumes, SDF shadows, reflection probes, compute) reaches the device
through a string-named `QueryInterface` side channel and runs as more
methods on the same translation object. That design got native Vulkan
running on four device families, but it cannot express passes, transient
targets, GPU skinning, shadow maps, parallel recording, a second scene or a
second graphics API, and every feature added to it makes the translation
layer larger.

This RFC inverts the relationship and builds the renderer as ports and
adapters:

1. **A render core owns the frame.** A family of strict, portable C++20
   libraries under `render/` and `public/render/` records each frame as a
   render graph over a device port. The engine, not the client DLL, owns
   frame and view orchestration.
2. **Ports are narrow, backend-neutral contracts; adapters implement them.**
   The device port (`render.device.v2`) knows no graphics API. Vulkan,
   OpenGL and null adapters implement it. Other ports (the frame and
   feature port, visibility providers, the scene service) follow the same
   rule. Only application roots name an adapter.
3. **The legacy API becomes one adapter.** `IMaterialSystem`,
   `IMatRenderContext`, `IMaterial`, `IMesh`, `IShaderAPI`, the shader-DLL
   ABI and material proxies keep their vtables and behavior. Their stream is
   recorded into *legacy stream passes* at the view stage where it happens
   today. At the inversion gate (K3) every pixel family is byte-identical
   to the current backend.
4. **Vulkan is the first device adapter; OpenGL is the second.** The OpenGL
   adapter (K10) proves the port is backend-neutral, as RFC 0001 step 10
   requires. Apple runs through MoltenVK. The D3D9 path is not ported.
   ToGL stays on the legacy D3D9 profiles, where it runs mod shader DLLs'
   D3D bytecode (user decision, 2026-09-26).
5. **A render graph (`render.graph.v1`).** Passes declare the resources
   they read and write. The graph orders passes, derives abstract resource
   transitions, allocates transient resources (aliasing them where the
   adapter can), merges passes where measured, and records passes in
   parallel where the adapter can. A serial executor in declaration order is
   its oracle.
6. **A persistent GPU scene (`render.scene.v1`).** Render objects are owned
   by one scene authority and updated by change sets committed at a frame
   boundary. Views are extracted, culled and turned into draw lists by jobs.
7. **Materials are data (`render.material.v2`).** A material is a family, a
   typed parameter block and a static permutation key. VMT files are
   imported per family. The 86 GLSL ports of `stdshader_dx9` become the
   `legacy` family, the catch-all for content no native family claims.
8. **Arrows point down, and a checker says so.** Every render module has a
   declared layer. A new archlint rule (CAP011) rejects upward and sibling
   edges and any portable dependency on an adapter, on top of the existing
   include, link and owner checks.
9. **Features are graph passes with one owner each.** Lights and shadows are
   owned here (K7). GI (RFC 0011), antialiasing (RFC 0012), UI (RFC 0010)
   and debug views (RFC 0014) plug in as feature adapters and keep their
   owners.

Mods, unported code and the legacy D3D9 profiles keep calling the legacy
API indefinitely. First-party Portal and Portal 2 rendering moves to native
passes one cohort at a time, each against an oracle, until its use of the
legacy stream reaches zero.

This RFC defines contracts, owners, layout and gates. Nothing here is
installed.

## Binding rules for all render work (user decision, 2026-09-28)

These rules are mandatory. They bind every session and every agent
that touches rendering, lighting or materials in this repository. They are
not guidelines, defaults or preferences. No session may reinterpret, relax,
defer or negotiate them, including with another session. Only the user can
change them, by editing this section. A change that breaks a rule is
rejected in review, however small it is and whatever it delivers.

### Why these rules exist

The same rendering and lighting work was built three times:

1. **On the native backend's translation layer.** PBR, probe-volume GI, the
   SDF and ray-query producers, relit parallax probes and clear coat live
   in `CVulkanContext` and `materialsystem/shaderapivulkan/shaders/`.
   `world_pbr.frag`, `model_pbr.frag`, `probe_volume.glsl` and
   `reflection_probes.glsl` reach it through `QueryInterface` side channels.
2. **On the engine's CPU lightmap path.** Area lights, projected lights and
   moving-object occlusion were added to `engine/gl_lightmap.cpp` and
   `engine/lightcache.cpp`. They were added on the same day that per-pixel
   versions of the same lights were assigned to this RFC's K7.
3. **On the render core.** The `pbr` family re-ports part of
   `model_pbr.frag`, and K12 plans to move the rest and delete both older
   copies.

The same pattern hit other areas: three Blender-driven bakers with
duplicated constants (the R48 audit); two GTK Hammer shells; the Hammer GL
renderer, then the core; and specs restating one another across RFCs 0007,
0011 and 0016.

The causes, each answered by a rule below:
- Work landed on whatever layer could draw pixels that day, not on the
  layer that owns it (rules 1 and 2).
- Each feature was proven by booting the game, so its hard problems
  surfaced late and inside the product (rule 3).
- Partial slices never closed, so the old path was never deleted, and both
  copies were maintained and kept growing (rule 4).
- Ownership was settled after two implementations existed (rule 5).
- Specs described what had been built instead of steering what came next,
  and repeated each other (rule 6).

### The rules

**Rule 1: The legacy render paths are frozen.**

The frozen paths are:
- the native Vulkan backend: `materialsystem/shaderapivulkan/`, its
  `shaders/` directory, and every `QueryInterface` side channel it
  serves;
- the D3D9 backend and `materialsystem/stdshaders/`;
- the legacy material system's shading behavior;
- the engine's CPU runtime-lighting path: `engine/gl_lightmap.cpp`'s
  runtime light functions (`R_AddAreaLights`, `R_AddProjectedLights`,
  `R_ApplyDynamicOcclusion` and any like them), and `engine/lightcache.cpp`'s
  light stand-ins.

A frozen path receives no new feature. A feature is any of:
- a new lighting term;
- a new light, shadow or occlusion kind;
- a new material parameter, shader, pass or render target;
- a new side-channel interface;
- a new console variable that changes what is drawn;
- any look the path did not produce before.

Only these three changes are allowed on a frozen path:

1. **A defect fix.** The path does not match the behavior it already
   claims: a crash, a leak, a race, a validation error, or pixels that
   differ from its own oracle (its D3D9 or retail reference, or its
   recorded fixture). The fix restores the claimed behavior and nothing
   more. Making the path match retail or D3D9 behavior it claims is a fix.
   Giving it an appearance it never had is a feature, even when retail has
   that appearance through a different path.
2. **Plumbing for the core.** Changes that K3, K5–K8 or K12 require for the
   core to take work over (a slot, a handle, a flag, a handover), or that
   delete frozen code the core has replaced.
3. **An explicit user request.** The user asks for that specific change on
   that specific frozen path. Before making it, the agent tells the user in
   one sentence that the change lands on a frozen path and will be rebuilt
   on the core. The progress record keeps the user's words and the date.
   A general goal ("make it look better", "Source 2 quality", "fix the
   lighting") is not such a request. Work toward a general goal goes to the
   core.

A commit that changes a frozen path names its exception in the message:
`Frozen-path: defect <oracle>`, `Frozen-path: core plumbing <gate>`, or
`Frozen-path: user request <date>`. A change that fits none of the three
does not land.

**Rule 2: New render work lands on the core, in its owning module.** Every
new lighting term, material capability, pass or render feature is built in
the `render/` module that the layer contract and this RFC assign it to. If
that module or its prerequisite gate is not ready, the work waits or goes to
`render_lab` (rule 3). It never goes onto a frozen path to show results
sooner.

**Rule 3: Hard parts are proven in `render_lab` before any integration.**
A term, family or pass is first built and proven in `render_lab`, with no
engine in the process, against its C++ oracle and its Cycles reference, with
its negative control failing (K11). Until its K11 check passes, it gets no
product wiring: no engine hook, no ConVar and no launcher flag. Game boots
are integration evidence (K12), never the first proof that the math is
right. Hard problems belong in the lab, where a render takes seconds and
nothing else varies.

**Rule 4: The old copy is deleted in the change that replaces it.** The
change that makes the core own a term for a set of surfaces must also:
- set the `RuntimeLight` flag (or its equivalent) for exactly those
  surfaces;
- stop the old path from producing that term for them;
- delete every old-path code path that no longer has a caller.

No later cleanup change is planned for this: a deletion that can be made
now is made now. Code that still serves surfaces the core does not own stays
frozen under rule 1 until the core owns them. K12's "One copy of the math"
check then requires it gone.

**Rule 5: One owner per concept, settled before code.** Before starting
render work, a session:
- finds the owner of the layer and term in this RFC and in the roadmap row;
- records its slice in `RFC/0016-progress.md`;
- confirms with the session that owns the module (today the render-core
  session) that nobody else is building the same term.

Two sessions never build the same term, and a second implementation of an
owned term is never started "for now".

**Rule 6: One definition per concept.** Each contract, algorithm, constant
and unit is defined in exactly one RFC section or owner header. Other RFCs
link to it and never restate it. When a definition moves, the old text is
replaced by a link in the same change. This section's lighting model table
follows the rule: it names each term's owner and defines only the terms this
RFC owns.

**Rule 7: Look first, then optimize. Performance gates never block work.**
Everywhere, on every profile and in every gate, the order is fixed:
1. get the effect looking right, against its oracle and Cycles reference;
2. integrate it;
3. optimize it.

A performance check (frame time, GPU time, submission cost, memory, a
budget row, a speedup rule, the frame allowance) never blocks:
- landing a change;
- closing a quality check;
- integrating a proven term;
- starting a dependent row;
- turning an effect on.

Performance is still measured and recorded wherever a gate lists it, on the
hardware available. A miss is recorded on the row as an optimization item
with its numbers. It is not a failure of the work, and it is never a reason
to cut, simplify or turn off an effect that looks right. An effect is
turned off on a profile only when the profile lacks a required capability
(declared by name), or when the user decides it for that profile's shipped
default. Correctness checks that happen to involve time still block: a
hang, a timeout, a frame that never presents, or a race.

Measuring is not optional (user decision, 2026-09-29). Every slice that
changes what is drawn records, in its progress entry:
- frame time on the desktop profile and on the Fold7, with the command and
  the numbers;
- or, for a device that was unavailable, "unavailable" and the reason.

The numbers go into the rows of `quality/budgets/render-v1.json` as they
exist, so the performance debt stays visible while it is not blocking.

### Enforcement

- **Review.** Every change under `materialsystem/`, `engine/gl_lightmap.cpp`,
  `engine/lightcache.cpp` or `render/` is checked against rules 1–6 before
  it lands. A missing or false `Frozen-path:` line rejects the change.
- **Freeze ratchet** (`render.legacy-freeze`, installed 2026-09-29, first
  step of the delivery order, owned by R95). The command is
  `python3 tools/render/retirement_scans.py legacy-freeze`. It compares the
  frozen paths exactly with `tools/render/legacy_freeze_ratchet.json`:
  - the files of `materialsystem/shaderapivulkan/shaders/` and each one's
    line count;
  - the source files of `materialsystem/stdshaders/`;
  - the interface names the frozen backends answer in `QueryInterface`;
  - the functions defined in `engine/gl_lightmap.cpp` and
    `engine/lightcache.cpp`;
  - the ConVars, console commands and launch switches defined in the
    frozen paths.

  Any growth fails, and so does a removal that isn't recorded. A set may
  grow only in a `Frozen-path:` commit whose exception is 1 or 3, and that
  commit rewrites the ratchet (`--write`) itself, so the growth is visible
  in review. A deletion under rule 4 records the removal the same way.
  In a checkout shared with other sessions, record from your own commit
  (`--write --rev <commit>`), so another session's uncommitted work is never
  recorded under your commit. `--rev HEAD` checks committed source only.
  Seeded faults in `render.retirement-scans.sensitivity` cover each set.
  The legacy material system's shading behavior has no static scan, and
  review covers it.
- **Gates.** K11 is the proof gate for rule 3. K12's "One copy of the math"
  and K9's "Dead code removed" are the deletion gates for rule 4.

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

### Other graphics paths

- **ToGL** (`togl/linuxwin/`, about 27.6k lines, and `togles/`) emulates
  D3D9 on OpenGL and translates D3D9 shader assembly to GLSL at run time
  (`dx9asmtogl2.cpp`). `--use-togl` defaults on outside Windows
  (`wscript:482`), and Vulkan builds turn it off (`wscript:288`). The SDL2
  legacy-renderer profiles use it: `linux-i386-legacy` and
  `freebsd-legacy` through ToGL, `android-armv7a-legacy` through ToGLES
  (`scripts/build-android-armv7a.sh`).
- **DXVK Native** runs `shaderapidx9` on Vulkan for the legacy D3D9
  profile.

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

- **Dependency direction is whitelisted, not layered.** Each capability
  module in `architecture/modules.json` lists `allowedEdges`; CAP002/CAP005
  check includes against them, CAP006 checks links, and CAP004 rejects
  permission cycles. Nothing declares which module is above which, so a
  lower module can be granted an edge to a higher one as long as no cycle
  forms, and sibling adapters can depend on each other.
- `render.vulkan.core` is a `backend` module; `shaderapivulkan.cpp`,
  `vulkan_compute.cpp`, `vulkan_shader_library.cpp`, `vulkan_mesh_layout.*`
  and `shaders/` have no module owner, and the `shaderapivulkan` target sits
  in the `render-legacy` group owned by R46.
- **The job system has no capability module.** `public/jobsystem/` and
  `jobsystem/` are unowned and the target is in the `engine-and-tiers`
  group. A strict module cannot include it without failing CAP002/CAP005.
- **The shader compiler is not pinned.** `quality/baseline.json` records
  `glslc --version` without an expected value; the SPIR-V is committed as
  generated headers (`material_spv.h`, `legacy_spv.h`).
- **Layout convention.** Existing portable libraries put public headers in
  `public/<lib>/` and sources in `<lib>/` (`mapgeometry`, `kvtext`, `vmf`),
  with the owning module found by longest path prefix. Contract documents
  live beside their suites (`unittests/rendertest/contracts/*.v1.md`).

## Goals

- One owner of the frame, in the engine, with views, passes and resources
  declared rather than inferred.
- Ports that name no graphics API, with at least two real device adapters
  (Vulkan and OpenGL) passing one shared suite, and bad adapters failing it.
- GPU resources released only by completion tokens (RFC 0006), and no
  device-wide or queue-wide idle wait on a frame path.
- A four-bind-group ceiling, so every material family runs on every device
  the profiles declare.
- Byte-identical pixels at the inversion gate; afterwards, every behavior
  change is a versioned decision with its own oracle.
- GPU skinning, shadow maps for the sun, spot lights and flashlights,
  clustered dynamic lights, float and MRT targets.
- One lighting model at Source 2 quality or better
  ([Lighting model](#lighting-model-renderlightingv1-amended-2026-09-28)):
  every term with an owner, an oracle and a neutral value, proven against
  Cycles in `render_lab` before it is integrated (K11, then K12).
- Parallel command recording in products: passes record as jobs on the
  root's compute pool into their own encoders, the command stream equals the
  serial executor's, and frame recording time falls as workers are added
  (K5 "Pooled recording", K9 "Recording scales"; RFC 0003 goals J1 to J6).
- The render sequence off the main thread in every shipped native profile,
  so the main thread builds scene change sets and `FrameDesc` and records no
  draws (K9 "Render off the main thread").
- A second scene in the same process (Hammer's viewport, material previews,
  thumbnails) without globals.
- A layer contract that a checker enforces, with seeded violations
  rejected.
- Submission cost (`render_submission`: draws, recording and submission on
  the submitting sequence) reduced against its recorded budget (K5), and no
  regression of the Apple TV 60 fps budget. These are optimization goals:
  they never block a change or a gate (binding rule 7).
- The legacy API, content and mod shader DLLs keep working on the profiles
  that support them today.

## Non-goals

- Changing any frozen interface, content format or gameplay behavior.
- A native D3D9, D3D12, Metal or WebGPU adapter. The port admits them; each
  needs its own decision and evidence.
- Porting `shaderapidx9`, or retiring ToGL. ToGL stays for mod shader DLLs
  on the legacy D3D9 profiles (user decision, 2026-09-26). The OpenGL
  adapter runs the core, not D3D bytecode.
- Running D3D bytecode from mod shader DLLs on the core
  ([Mod shader DLLs](#mod-shader-dlls)).
- Temporal antialiasing or upscalers (RFC 0012 keeps these out of scope).
- Screen-space global illumination. RFC 0011 rejects it; the lighting
  model's screen-space reflections are glossy reflections over the probes,
  not diffuse GI.
- Mesh shaders, bindless descriptors or GPU-driven culling, until measured
  need ([Later work](#later-work)).
- A new UI toolkit, particle system or animation system. Their rendering
  moves; their simulation does not.
- Rewriting `viewrender.cpp` in one change.

## Ports and adapters

A *port* is a contract module: interfaces, value types and numbered
obligations, with no implementation that depends on a backend. An *adapter*
implements one port for one technology. Portable code depends on ports.
Only application roots and test fixtures name adapters, and they compose
them through typed descriptors (RFC 0001 composition).

| Port (module, contract id) | Consumers | Adapters |
| --- | --- | --- |
| Device: `render.device`, `render.device.v2` | graph, shader library, materials, scene, feature passes, legacy frontend | `render.device.vulkan` (K1), `render.device.gl` (K10), `render.device.null` (K1); later ones need their own decision |
| Presentation: `render.contracts`, `render.presentation.v1` (R16) | application roots, renderer | `render.bridge.sdl3-vulkan` (exists), `render.bridge.sdl3-gl` (K10), headless |
| Shader artifacts: `render.shader-library`, `render.shader-artifacts.v1` | materials, feature passes | the build-time artifact store; a development reload source (RFC 0014) |
| Visibility: `render.scene`, `render.visibility.v1` | scene | BSP leaf and area-portal visibility (engine), WMSH cluster and occlusion culling, later RFC 0008 F8 visibility |
| Frame and features: `render.frame`, `render.frame.v1` | engine and client roots, Hammer | renderers: `render.renderer` (full), later a wireframe or reference renderer with declared narrower fidelity; features: `render.pass.*`, `render.legacy-frontend` |
| Scene service: `render.scene`, `render.scene.v1` | engine, game, Hammer, previews | one implementation; fakes for tests |

Each port has a contract document with numbered clauses, one shared suite
that runs against every claiming adapter and fake, and deliberately bad
adapters that must each fail a named clause, as `platform.task-runner.v1`
does (`public/platform/contracts/task_runner.h`,
`unittests/platformtest/task_runner/`).

## Directory layout and modules

New code follows the house layout (`public/<lib>/` for public headers,
`<lib>/` for sources), nested under one `render/` root so the family is
visible in the tree. Ownership is by longest path prefix, so
`public/render/device/vulkan/` belongs to the Vulkan adapter, not the port.
The existing flat headers in `public/render/` stay in `render.contracts`
(C++11, ABI-facing) and are not moved.

```text
public/render/                    render.contracts (existing flat headers; unchanged)
public/render/math/               render.math
public/render/device/             render.device                 port
public/render/device/vulkan/      render.device.vulkan          adapter: factory header only, no Vulkan types
public/render/device/gl/          render.device.gl              adapter: factory header only, no GL types
public/render/device/null/        render.device.null            adapter: factory header only
public/render/graph/              render.graph
public/render/shaderlib/          render.shader-library
public/render/resources/          render.resources              texture and mesh residency
public/render/material/           render.material
public/render/scene/              render.scene
public/render/frame/              render.frame                  port: IRenderer, IRenderFeature, FrameDesc
public/render/renderer/           render.renderer               adapter of render.frame
public/render/pass/<feature>/     render.pass.<feature>         adapter of IRenderFeature
public/render/legacy/             render.legacy-frontend        adapter of IRenderFeature; legacy-interop
public/render/composition/        render.composition            assembles a core for application roots

render/<module>/                  sources and private headers of each module above
render/device/vulkan/             Vulkan adapter (VMA lives here)
render/device/gl/                 OpenGL adapter
render/device/null/               recording adapter
render/material/families/<name>/  native family definitions and their GLSL
render/shaders/common/            shared GLSL (pbr_brdf.glsl moves here), owned by render.material
render/pass/<feature>/            each feature's sources and its own GLSL
render/legacy/family/             the 86 legacy ports
render/bridge/sdl3-vulkan/        presentation bridge (moved from materialsystem/shaderapivulkan/sdl3/)
render/bridge/sdl3-gl/            presentation bridge (K10)

unittests/rendertest/core/<module>/   suites, fakes and bad adapters per module
unittests/rendertest/contracts/       render.device.v2.md, render.graph.v1.md, …
tools/render/                         shader artifact builder, independent graph model (Python)
```

[Appendix A](#appendix-a-file-level-layout-and-wiring) lists every proposed
header and source file, and shows how the family hooks into Waf, the
module manifest, the launcher, the engine, the client DLL, the material
system, Hammer and the test runner.

- **Shaders live with their owner.** A family's GLSL sits in its family
  directory; a feature pass's GLSL sits in its pass directory; the legacy
  ports move with the frontend. `materialsystem/shaderapivulkan/shaders/`
  empties as its files move and is deleted at K9.
- **Adapter public headers are factories.** `public/render/device/vulkan/`
  declares one typed provider descriptor and its options. It includes no
  Vulkan or SDL header, which CAP007 (standalone compile) and CAP005
  (transitive includes) check.
- **Existing Vulkan code moves by extraction.** `render.vulkan.core` files
  move into `render/device/vulkan/` as the K1 adapter takes them over; what
  remains in `materialsystem/shaderapivulkan/` is the legacy
  `IShaderDevice` shell until K3 moves it into `render/legacy/`.
- **One Waf static library per module**, each declaring `arch_module`,
  built with `features='cxx capability_strict'` and
  `env=bld.strict_cpp20_env()`, and listed as `cxx20` in
  `quality/toolchain/policy.json`. The legacy frontend is the exception: it
  is `legacy-interop` and may use `cxx20-permissive` because it includes
  legacy headers.
- **Vocabulary.** Portable modules use `foundation::Expected`,
  `foundation::Error`, `StrongId` and `ScopedResource`. `render.contracts`
  stays C++11 and ABI-facing; the core adds no vocabulary types to it, and
  CAP010 keeps foundation types out of preserved-ABI headers.
- **Math.** `render.math` is private to the render family. Hammer's
  `mapgeometry` keeps its editor tolerances and double precision (AGENTS.md
  DRY rule). The legacy frontend converts from `Vector` and `VMatrix`.

## Layer contract and import rules

Dependencies point down. The render family declares its layers, and every
edge is checked three ways: the declared `allowedEdges` must respect the
layers (new rule CAP011), the code's includes must stay inside the declared
edges (CAP002 direct, CAP005 transitive), and the links must too (CAP006).

| Layer | Modules | May depend on |
| --- | --- | --- |
| 8 Test fixtures | `render.core-tests` (`unittests/rendertest/core/`) | anything below, including adapters |
| 7 Applications | `render.composition`; engine and client roots, Hammer, tools | anything below, including adapters |
| 6 Features and renderers | `render.renderer`, `render.pass.*`, `render.legacy-frontend` | layers 0–5; legacy headers only for the frontend |
| 5 Frame port | `render.frame` | layers 0–4 |
| 4 Scene | `render.scene` | layers 0–3 |
| 3 Materials | `render.material` | layers 0–2, `content.keyvalues-text` |
| 2 Core services | `render.graph`, `render.shader-library`, `render.resources` | layers 0–1; `render.resources` also `content.texture-contract` and the texture readers |
| 1 Ports | `render.device`, `render.legacy-provider-contract` | layer 0 |
| 0 Vocabulary | `foundation`, `render.math`, `render.contracts`, `jobs.graph` | nothing in the render family |
| Adapters (column) | `render.device.vulkan`, `render.device.gl`, `render.device.null`, `render.bridge.*` | `render.device`, layer 0, and their native SDK grants |

Rules CAP011 enforces over `architecture/modules.json`:

1. **Down only.** An edge from a layer-*n* module may target only modules in
   layers below *n*, or declared external bases (`foundation`, content
   format libraries). An edge to the same or a higher layer fails.
2. **Siblings are independent.** Modules in one independence group have no
   edges among them. The groups are the features and renderers of layer 6
   (so a pass never depends on the renderer or another pass) and the
   adapters (so the GL adapter never depends on the Vulkan adapter).
3. **Nothing portable depends on an adapter.** Only layer 7 may name an
   adapter module. An adapter may depend only on the port it implements,
   layer 0 and its native grants.
4. **Every render module has a layer.** A module whose id starts with
   `render.` and appears in no layer or adapter column fails.
5. **No backend identity in portable code.** Portable modules may not
   compare the device's diagnostic backend identifier. Behavior follows
   capabilities; the identifier exists for logs and evidence only.

The layers live in one new `layerContracts` section of
`architecture/modules.json`, next to the module rows, so the manifest stays
the single owner of dependency permissions (AGENTS.md). CAP011 ships with
self-test fixtures, one per rule, each a seeded violation that must fail
with its rule number: an upward edge (scene → frame), a sibling edge
(`render.pass.shadows` → `render.renderer`), a portable edge to an adapter
(`render.graph` → `render.device.vulkan`), an adapter-to-adapter edge, an
unlayered `render.*` module, and a backend-identity comparison in a pass.

## Device port (`render.device.v2`)

`render.backend.v1` stays as the lifetime subset; its suite becomes part of
the v2 suite. v2 adds what a renderer needs to record work, in terms no
graphics API owns.

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
	virtual const DeviceFacts &Facts() const = 0;                 // capabilities, limits, artifact format
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

### Semantics every adapter provides

- **Immutable descriptions.** A pipeline is created from shader artifacts,
  a bind-group layout list, vertex input, raster, depth, stencil and blend
  state, attachment formats and sample count. Nothing is mutable after
  creation. An adapter whose API has no pipeline objects (OpenGL) caches
  programs and applies state differences itself; that is invisible above
  the port.
- **Four bind groups at most**, in a fixed role order: frame, view,
  material, draw. Vulkan guarantees four bound sets. OpenGL maps each group
  to a fixed range of its flat binding slots, and later APIs map them to
  their own tables.
- **Completion tokens** carry a queue, a monotonic value and the device
  epoch (RFC 0006). `Release` never frees before its token completes. After
  device loss, tokens from the old epoch are invalid. How an adapter
  implements tokens (timeline semaphores, fence objects) is private.
- **Abstract resource states.** Encoders and the graph speak in usages
  (sampled, storage read, storage write, color attachment, depth read,
  depth write, resolve, copy source, copy destination, present, vertex,
  index, indirect, uniform). The adapter turns a usage change into its
  API's barriers, layout transitions or nothing. No stage mask, access
  mask or image layout appears in the port.
- **Conventions.** Clip-space depth is 0 to 1, clip-space Y points up,
  framebuffer and texture origins are top-left, and texel centers are at
  half-integers. An adapter whose API differs corrects it privately
  (OpenGL uses `glClipControl`). Shaders never branch on the backend.
- **Upload rings** are adapter-owned. Each occupied range records the token
  that allows its reuse. Exhaustion defers the upload to the next
  submission and counts it; it never overwrites and never idles the device.
- **Memory.** Adapters suballocate and report per-heap budget and use.
  Aliasing of transient resources is a capability.
- **Encoders.** One encoder is recorded by one thread at a time; diagnostic
  builds check this with `platform::SequenceChecker`. Encoders are
  submitted in the order given to `Submit`.
- **Queues.** Graphics is required. Compute and transfer queues are
  optional capabilities; the graph falls back to the graphics queue.
- **Facts, profiles and quirks** follow RFC 0001. Facts are immutable.
  Profiles are policy chosen by the composition root. Quirks are data with
  a reason and an adapter range. `Present` no longer reads ConVars; policy
  arrives through the frame description.
- **Presentation** stays `render.presentation.v1` (R16). The swapchain or
  default framebuffer is an imported graph resource; the bridge keeps
  native handles.

### Capabilities

`DeviceFacts` reports what the adapter can do. Families and features
declare what they require. The composition root either selects a declared
fallback or fails composition with a structured error that names the
missing capability. Nothing falls back silently or after partial setup.

| Capability | Vulkan adapter | OpenGL 4.5 adapter | If absent |
| --- | --- | --- | --- |
| Compute shaders and storage buffers | yes | yes | features that require them fail composition, or use a declared fallback (CPU skinning is the skinning fallback) |
| Transient memory aliasing | yes | no | transient resources are pooled by description |
| Parallel native recording | yes | no | encoders record CPU command lists that replay on the submitting sequence |
| Async compute and transfer queues | per device | no | work runs on the graphics queue |
| Ray query | per device | no | RFC 0011 producers that need it are unavailable |
| Sample counts, formats, limits | per device | per device | per feature rule |

### Shader artifacts

- **One source language, one artifact per target.** GLSL stays the source.
  The pinned compiler builds SPIR-V, and pinned SPIRV-Cross produces other
  targets from it (GLSL 4.50 for the OpenGL adapter; later ESSL or MSL if a
  profile needs them). `DeviceFacts` names the artifact format an adapter
  accepts.
- Artifacts are keyed by source, compiler identity, target format and
  permutation. Reflection checks each one against its family's or pass's
  declared layout at build time. Shipping products never compile shaders;
  development compositions may reload them (RFC 0014).
- Artifacts are build outputs produced by a Waf task with the pinned tools.
  The committed generated headers (`material_spv.h`, `legacy_spv.h`) are
  deleted at K4, once the pinned build reproduces them byte for byte.

### Vulkan adapter

- **Required features:** timeline semaphores, synchronization2 and dynamic
  rendering (each core in Vulkan 1.3 and available as an extension
  earlier). K1 records each declared profile's support. A profile without
  one fails composition with a structured error, not a silent fallback.
- **Memory:** Vulkan Memory Allocator, pinned, private to the adapter.
- **Source:** extracted from `CVulkanContext` and the test-only
  `vulkan_render_backend.cpp`, so the shipping path and the contract become
  one stack.

### OpenGL adapter

- **Target:** OpenGL 4.5 core on Linux desktop, which gives compute,
  `glClipControl`, direct state access and fence sync objects. macOS
  OpenGL (4.1) is out of scope. ESSL targets for GLES are a later
  capability-narrowed profile, needing their own decision.
- **Tokens** are fence sync objects behind the same monotonic token
  semantics.
- **Threading:** one GL context is owned by the render sequence. Encoders
  from other jobs record CPU command lists that the render sequence
  replays in submission order.
- **Profile:** an optional desktop product profile (proposed
  `portal-linux-gl`), registered in `quality/baseline.json` like the other
  optional rows. It does not replace ToGL.

### Shared suite and bad adapters

One suite runs against the Vulkan, OpenGL and null adapters and fakes. It
includes the `render.backend.v1` cases, plus a conventions section that
renders known geometry and checks where it lands (depth range, Y
direction, origin, texel centers). Each deliberately bad adapter must fail
a named clause:

- recycles an upload range or releases a resource before its token
  completes;
- reports different facts after creation;
- completes submissions out of order on one queue;
- accepts a token from a previous device epoch;
- creates a pipeline whose layout does not match its artifact's reflected
  bindings;
- leaks a partially created resource when creation fails;
- accepts a fifth bind group;
- allows two threads to record one encoder without a diagnostic;
- claims a capability it does not implement (aliasing that corrupts);
- flips the Y convention or uses a −1 to 1 depth range.

## Render graph (`render.graph.v1`)

### Building

A graph is built each frame on the render sequence:

- **Passes**: graphics, compute, copy, present, and *host* (a CPU callback
  that records into an encoder the graph provides; the legacy stream pass is
  one). Each pass has a name, a queue preference and a record function.
- **Resources** are *imported* (the swapchain image, persistent textures and
  buffers, history targets, named legacy render targets) or *transient*
  (created from a description for this frame only).
- **Accesses** declare read or write, the port's abstract usage and a
  subresource range. A write creates a new version of the resource.
- **Side effects.** A pass that writes an imported resource, presents or
  reads back is never culled.

### Compiling

- **Validation.** A resource version has one writer; nothing reads an
  undefined version; there is no cycle; accesses match the resource's
  declared usages. A violation is an error value that names the pass.
- **Order.** Passes execute in declaration order. The compiler may move a
  pass only onto another queue, and only when it proves the pass
  independent. Declaration order is also the serial oracle.
- **Transitions** are computed per subresource as abstract usage changes.
  The adapter turns them into its API's synchronization.
- **Transient lifetimes.** Transients get memory by lifetime interval,
  aliased where the adapter reports aliasing and pooled otherwise.
- **Culling** removes passes whose outputs nothing reads, unless they have
  side effects.
- **Pass merging.** Adjacent graphics passes with compatible attachments may
  merge into one rendering scope, the job the current sRGB/UNORM merge does
  by inference. On the Fold7 the existing merge cut passes from 31 to 5
  without changing GPU time
  ([frame pacing record](0001-native-vulkan-frame-pacing-progress.md#mobile-gpu-cost-render-pass-breaks-2026-09-23)),
  so merging is a measured, per-profile option.
- **Caching.** A compiled graph is reused while the frame's shape is
  unchanged, the way `DeclaredFrameGraph` reuses its sealed graph.

### Executing

- **Recording** runs passes as jobs on the RFC 0003 executors when the
  adapter records natively in parallel, and as CPU command lists otherwise.
  Submission order is declaration order. The serial executor is the oracle
  and the low-capacity mode.
- **Views are subgraphs** with declared inputs and outputs. Recursive views
  (portals, mirrors) are nested subgraphs with a declared depth limit.
- **Traces.** Every compile can emit a trace (passes, resources, versions,
  transitions, aliasing, merges, timings) for RFC 0014's debug controls and
  RenderDoc labels.

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

- **Visibility adapters** implement `render.visibility.v1`: BSP leaf and
  area-portal visibility (the engine), WMSH cluster and occlusion culling
  (moved from `gl_rsurf.cpp`'s `worldmesh_cull`), and later RFC 0008 F8's
  visibility. The scene does not own BSP.
- **Per view**, jobs run visibility, frustum and occlusion culling, then
  build draw lists per pass kind (depth, opaque, translucent, shadow,
  capture) with sort keys. The serial path is the oracle.
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
  stays as the oracle, as the fallback where compute is unavailable, and
  for materials that need software skinning.

## Materials (`render.material.v2`)

- **A family** declares a parameter schema (names, kinds, color encodings,
  defaults; RFC 0007's `pbr_material_schema.h` is the model), the pass
  kinds it supports, its static permutation axes, its material bind-group
  layout, its render-state rules and the device capabilities it requires.
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
  and `legacy`. The `legacy` family runs the 86 ports keyed by program and
  is defined in the frontend module.
- **One model; legacy cases are degenerate (user direction, 2026-09-28).**
  The core keeps the legacy material system's intent and exceeds it. Its
  families converge on one general surface model whose terms each have a
  neutral value, so each legacy shader and branch is a parameter point:
  - unlit is lighting fixed at one;
  - a flat lightmap is a bumped lightmap with a flat normal;
  - WorldVertexTransition with blend zero is one layer;
  - no env map is reflectance zero, and no detail is a neutral detail;
  - VertexLitGeneric is the same surface lit by model lighting instead of a
    lightmap.
  Legacy quirks (the 0.7 alpha-test reference, the 2.0 overbright, gamma
  rules) are parameter defaults with one owner each, not code paths.
  The narrow families above (`unlit`, `lightmapped`, `vertexlit`, `pbr`)
  are stepping stones folded into that model. A claim that refuses a
  material is a gap in the model to close, not a boundary to keep.
  Passes (world, props, models) never name a family: they resolve a
  material to a program through one resolver. The plan and its phases are
  in "The surface model" below.
- **No escape hatches (user direction, 2026-09-28).** A claim is exact: the
  model takes a material only when it reads every variable the material
  sets, or when each variable it does not read holds legacy's neutral value
  for its shader (the shader's own initialization of a material without
  that variable). The material flags count as variables. Legacy draws only
  what the model does not claim: a material or a mod the model can't port
  yet, named as a gap. Once the core claims work (a material, a view), a
  failure to draw it is fatal (`r_core_world_strict 1`, the default). With
  the switch off, the failure is reported and the surfaces stay undrawn.
  It never falls back to legacy.
- **Material proxies.** `IMaterialProxy::OnBind` keeps its timing: the
  legacy frontend calls proxies when a renderable's draw is extracted, which
  is when the legacy path binds the material. `IMaterialVar` writes land in
  the renderable's parameter block for that draw.
- **Missing shaders.** A material whose shader no family or legacy port
  implements is reported when the material loads, with the material name,
  and is drawn with the error family. Today such draws are dropped and
  counted in a census. Content validation (RFC 0015) reports it at build
  time.

### Mod shader DLLs

Mod shader DLLs (`ShaderDLL004`) ship D3D shader bytecode, which the core
does not run. On core profiles a mod shader DLL still loads through the
existing extension host; its shaders register as `unsupported-on-profile`
and their materials follow the missing-shader rule. The legacy D3D9
profiles keep running them: native D3D9 on Windows, DXVK on Linux, and
ToGL or ToGLES on the SDL2 legacy-renderer profiles. ToGL stays for exactly this reason (user decision,
2026-09-26). Retiring it would be a separate decision about those profiles.

### The surface model: legacy materials as degenerate cases (plan, 2026-09-28)

User direction (2026-09-28): keep the legacy material system's intent and
exceed it, with its special cases as degenerate cases of one expanded model;
Source 2 parity or better; no escape hatches. This section is the plan. Its
progress lives in RFC/0016-progress.md, and K4, K5, K7 and K8 carry the
gates.

#### One surface, terms with neutral values

A material is one program, `surface`, with a parameter block. Every term
has a neutral value, and a term at its neutral value is exactly the term
absent: bitwise, checked per term. Each legacy shader and branch is then a
point in the parameter space. The per-material static permutation is the
set of non-neutral terms (specialization constants, a bounded axis set per
pass kind), so a neutral term costs nothing at runtime.

| Term | Inputs | Neutral | Legacy points it covers | Modern point |
| --- | --- | --- | --- | --- |
| Layers | up to two base layers, blend from vertex alpha, `$blendmodulatetexture` | one layer (blend 0) | LightmappedGeneric; WorldVertexTransition | height-blended layers |
| Albedo | base texture, `$color`, `$color2`, vertex color, `$srgbtint` | white | every shader's base pass | base color |
| Detail | texture, scale, blend mode (0–9), factor, tint, alpha mask | no texture (the mode's identity) | `$detail` by `$detailblendmode` | detail as a second layer or micro-normal |
| Normal | normal map with transform; ssbump basis weights | flat (0,0,1) | `$bumpmap`, `$ssbump`, `$bumpmap2` | tangent-space normal |
| Diffuse light | a baked basis (1 page flat, 3 pages RNM), probes (ambient cube, SH L1), runtime lights (clustered), lightwarp ramp | lighting one (unlit) | UnlitGeneric (one); flat lightmap = basis 1; bumped lightmap = RNM with the normal; VertexLitGeneric = probes plus model lights; `$lightwarptexture` = transfer ramp (identity neutral); `$halflambert` = a wrap parameter | RFC 0007 SH L1 or RNM from the Cycles baker, RFC 0011 probe volume, RFC 0016 K7 clustered and area lights |
| Emission | mask (base alpha, `$selfillummask`, detail modes 5/6), tint, fresnel | zero | `$selfillum`, `$selfillumtint`, `$selfillumfresnel`, detail self-illum | emissive radiance in scene units, and an area light (RFC 0011) when it should light its surroundings |
| Specular image | reflection source (`env_cubemap`, named cube, RPRB probe set), mask source (base alpha, normal alpha, `$envmapmask` with transform), tint, legacy fresnel, contrast, saturation, roughness | reflectance zero | `$envmap` and all its masks and knobs: legacy is roughness 0, mip 0, additive (no diffuse energy compensation), `$fresnelreflection` lerp, contrast/saturation as a color transform (identity at 0/1) | GGX split-sum IBL from parallax-corrected, relit probes (R50) with energy compensation |
| Specular lobe (runtime lights) | exponent or roughness, mask, fresnel ranges, boost, tint, rim | none | `$phong*`, `$rimlight*` (Blinn-Phong points, not energy conserving) | GGX from the same roughness |
| Coverage | base alpha × `$alpha` × vertex alpha; alpha test and reference; alpha to coverage; blend state (opaque, alpha, additive, mod2x) | opaque, no test | `$alphatest`, `$translucent`, `$additive`, `$vertexalpha`, `$allowalphatocoverage`, DecalModulate | the same, plus MSAA coverage (RFC 0012) |
| View terms (frame) | fog (range, height), output encoding, tone scale | no fog | legacy fog modes, `$nofog` | exponential height fog, volumetrics later |
| Projected lights | flashlight and `env_projectedtexture` through the view's projector list (K7, [Projected lights](#projected-lights-a-per-view-projector-list-amended-2026-09-28)) | empty list | the legacy flashlight pass per material | shadowed projected lights for every material |

Quirks are defaults with one owner: the 0.7 alpha-test reference, the
2.0 overbright, the LDR 2^2.2 and HDR 16 lightmap scales, the sRGB rules,
the half-Lambert wrap and the D3D9 half-pixel offset.

#### Exact legacy points, then modern points

Each legacy point reproduces its legacy port (the native backend's GLSL
ports of stdshader_dx9, R32-LEGACY-SHADERS) within the family's pixel
tolerance. The ports are the oracle, and the oracle compares against the
port through the same frame, so the core's arithmetic can't drift from
legacy. A modern point is the same program at other parameter values with
richer inputs (Cycles SH lightmaps, relit parallax probes, clustered and
area lights). Modernizing a material is then data, not code:

- a legacy material imports to its exact legacy point by default;
- an opt-in rule table (per game, per material family, reviewed) maps
  legacy parameters to modern ones: `$phongexponent` to roughness, env map
  tint and mask to F0 and roughness, `$selfillum` to emissive radiance and
  an area light, bump plus RNM to normal plus SH L1. It is checked against
  Cycles references (RFC 0007 G) with negative controls;
- per-material overrides are authored beside the VMT (a `.surface` sidecar
  owned by RFC 0015's asset graph). The VMT itself stays untouched, so the
  legacy path and mods keep working.

#### Order, from the inventory

The numbers come from an inventory of 6,000 Portal VMTs (Portal plus the
HL2 VPKs it mounts) and 3,738 Portal 2 VMTs: patches, fallbacks and
conditionals applied, and only keys that change rendering counted
(`tools/render/material_inventory.py`).

- Portal materials: LightmappedGeneric 2,200, VertexLitGeneric 1,550,
  UnlitGeneric 1,177. Portal 2: VertexLitGeneric 1,211, UnlitGeneric 1,039,
  LightmappedGeneric 811.
- The current model (opaque base, lightmap, vertex color, `$color`/`$alpha`,
  alpha test) covers 22.7% of Portal's materials and 19.8% of Portal 2's.
- Portal's chambers are 97.3% LightmappedGeneric by world area. Portal 2's
  area is dominated by huge `tools/toolsblack` faces, so it is weighted by
  faces.
- Through S8 the model takes 92.9% of Portal's materials and 88.3% of
  Portal 2's. The rest is the long tail below.

Cumulative coverage after each phase (`tools/render/material_inventory.py
--phases`; world area and faces from testchmb_a_00–11 and Portal 2's 63
`sp_a*` maps):

| Phase | Terms | Portal materials / world area | Portal 2 materials / world faces | Notes |
| --- | --- | --- | --- | --- |
| S0 current | base, lightmap or lighting one, vertex color and alpha, `$color`/`$alpha`, alpha test; opaque | 22.7% / 2.2% | 19.8% / 41.6% | step 4b |
| S1 Coverage and state | blending in the translucent stage (alpha, additive, mod2x), `$decal`, `$nocull`, `$ignorez`, `$nofog`; the fog view term | 45.4% / 2.4% | 55.7% / 42.0% | the biggest step by material count: decals and UI. A second world slot at the translucent stage. Fog admits the escape maps' views |
| S2 Specular image | `$envmap` (env_cubemap patches, named cubes), base-alpha, normal-alpha and `$envmapmask` masks, tint, contrast, saturation, `$fresnelreflection`, Portal 2's `$envmaplightscale` | 53.4% / 7.4% | 57.3% / 42.6% | needs cube textures in the port and in `ICoreTextures` |
| S3 Normal and basis light | `$bumpmap` with RNM (three bumped lightmap pages), `$ssbump`, texture transforms and proxy-driven parameters | 58.2% / 57.5% | 62.2% / 77.6% | per-frame parameter blocks for the 383 Portal and 173 Portal 2 materials with proxies |
| S4 Detail | `$detailblendmode` 0 (1,061 Portal materials), then 7, 2, 5, 10, 1, 8 | 75.4% / 89.5% | 64.9% / 97.8% | mode 0 is mod2x in gamma; 7 is its linear twin |
| S5 Emission | `$selfillum`, `$selfillummask`, tint, fresnel, detail modes 5 and 6 | 80.7% / 97.6% | 72.6% / 98.1% | emissive surfaces can register RFC 0011 area lights (the lit test-chamber sign tests) |
| S6 Layers | WorldVertexTransition, `$blendmodulatetexture`, `$seamless_scale` | 81.7% / 97.6% | 73.0% / 98.5% | Portal 2's world area to 97.3% |
| S7 Model surfaces | VertexLitGeneric through probes, the ambient cube and clustered lights; `$phong` (exponent, exponent texture, boost, fresnel ranges, albedo tint), `$rimlight`, `$halflambert`, `$lightwarptexture`, `$color2` | 85.4% | 84.4% | with K5's props and K6's skinned models |
| S8 Unlit points | Sprite, UnlitTwoTexture, SubRect, Sky (HDR encodings), distance-field alpha | 92.9% | 88.3% | lighting one; SubRect is a texture rectangle |
| S9 Modern points | the rule table and sidecars, checked against Cycles references | — | — | per game, opt-in, reviewed; never changes a legacy point |

Portal's world needs S2 to S4 together: its largest term sets are bump,
ssbump and detail mode 0 (28% of area), and bump, ssbump and env map with
tint and contrast (38%).

Legacy draws each phase's materials until that phase's claims take them,
and only then (no escape hatches).

Each phase closes when:
- the term's neutral value is bitwise the term absent (a suite with
  seeded mutants);
- each legacy point it adds matches its port on the material pixel families
  and in the isolated world oracle (`r_core_world_isolate`), within the
  recorded tolerance;
- the claim rules take exactly the materials whose variables the term
  reads (`UnreadVariable`, no escape hatches), and the coverage row per game
  is recorded;
- frame time and permutation count are measured against the K5 Submission
  budget per profile (desktop, Fold7, iPhone, Apple TV) and recorded; a miss
  is an optimization item, not a blocker (binding rule 7).

#### What stays outside the model

- Mods' own shader DLLs: the D3D9/ToGL legacy profiles.
- The long tail, shaders whose legacy output is not a lit surface:
  SpriteCard (83 Portal / 143 Portal 2), Water (46 / 59),
  Refract and PortalRefract (44 / 44), Eyeball, Eyes, Teeth and EyeRefract
  (88 / 0), Cable (14 / 5), Portal (7 / 7), SolidEnergy (0 / 14), PaintBlob
  (0 / 4), and about 100 engine-internal materials. Each gets its own
  family on the same terms (refract and glass as a transmission term, water as transmission
  plus reflection plus flow, sky, spritecard, eyes), not a new model.

## Frame and views (`render.frame.v1`)

- The engine owns `IRenderer::RenderFrame(const FrameDesc &)`. A frame
  description lists views, their stages and the frame's policy (indirect
  view, probe sampling, debug view, sample count), replacing the ConVar
  reads in `Present`.
- **Features** implement `IRenderFeature`: they declare their required
  capabilities and stages, and add passes to a view's subgraph. The
  renderer never depends on a concrete feature; the application root
  composes the renderer with the features it selects.
- **Stages** name the legacy order: monitors, 3D skybox, shadow depth,
  water reflection, water refraction, world opaque, renderables opaque,
  translucent, portals, view models, post, screen effects, HUD.
- **Migration path.** `CViewRender::RenderView` keeps its code. At K3 each
  of its stages opens a stage on the current view, and everything it draws
  lands in that stage's legacy stream pass. From K5 on, stages switch to
  native passes one cohort at a time, and `CViewRender` stops drawing what
  the scene now draws.
- **Stage hooks.** Game code adds passes through `IRenderStageHooks` with
  declared accesses (Portal's portal views, Portal 2 paint). A hook reaches
  the device only through the graph.
- **Portals.** Until the portal cohort (K8) moves, portal views run through
  the legacy stencil path inside the translucent stage, bit-exact. The
  native path makes portals and mirrors view generators whose subgraphs
  render to per-level targets or stencil, keeping the recursion limit and
  the texture fallback's behavior.
- **Frame-buffer copies become explicit.** When a legacy material needs
  `_rt_FullFrameFB` or `_rt_PowerOfTwoFB`, the frontend ends the pass and
  declares a copy pass at the same point in the stream. A native family
  that needs the scene reads the graph's scene-color resource instead.
- **Other renderers.** A wireframe renderer for Hammer or a reference
  renderer is another `IRenderer` adapter. It declares its narrower
  fidelity (AGENTS.md: a wireframe renderer cannot claim material
  fidelity) and passes the frame port's shared suite for what it claims.

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

### Projected lights: a per-view projector list (amended 2026-09-28)

`env_projectedtexture` is a light of RFC 0011's model
(`render.projected-light.v1`, `public/render/projected_light.h`), published
in the light set's `Snapshot::projected`, never in `lights`. The core shades
projected lights per pixel from a projector list in the view bind group.
They are not assigned to froxels.

- **The list.** It holds at most `projected_light::kMaxProjectedLights` (16)
  records, in the snapshot's order. Each record holds:
  - world-to-projector clip: the matrix `BuildFlashlightShadowView` builds
    from the light's basis, fields of view, near and far, so the cookie and
    the shadow share one projection;
  - color (linear, style folded in), attenuation (constant, linear,
    quadratic) and far, for `projected_light::Attenuation`'s falloff and end
    falloff;
  - the cookie's layer in the view's cookie array;
  - the shadow tile (`ShadowTileGpu`), or none when the light has
    `shadows` false or the atlas refused it;
  - `lightsWorld`: when false, world surfaces skip the record and models
    take it.
- **Evaluation.** The surface's projected-light term loops over the list.
  For each record it takes the projector-space position, rejects points
  outside the frustum or nearer than its near plane (`projected_light::Project`),
  samples the cookie at that point, and applies `IrradianceAt`'s rule times
  the shadow sample (`shadow_sample.glsl`). The neutral value is an empty
  list, and a view with no projector binds an empty list. Diffuse takes the
  contract's Lambert rule. Families with a specular lobe (the `pbr` point
  and S7's `$phong` point) add their lobe for the light's direction, as
  they do for clustered lights.
- **Cookies.** Each view gets one 2D array texture with one layer per record,
  at a size each profile sets (desktop 512², mobile 256² provisional, RGBA8
  sRGB). A cookie is copied into its layer, resampled, when its name, frame
  (`cookieFrame`) or version changes, not every frame. Unused layers are
  white. One binding keeps the four-group ceiling, and it fits the OpenGL
  adapter's minimum of 16 texture units per stage, which 16 separate cookie
  bindings would use up. The price is a capped cookie resolution per
  profile.
- **Shadows.** Each shadowed record asks `PlanShadowAtlas` for a tile,
  ranked by screen coverage like spot lights. Casters are the scene's
  instances and the world, so moving objects shadow through the atlas
  depth. The CPU path's 4-unit disk occluders are not carried over: their
  penumbra comes from the lens radius (`kSourceRadius`), the atlas's from
  its filter. That difference is part of the handover oracle's tolerance,
  not a new behavior switch, because the legacy flashlight pass drew
  projected light unshadowed and RFC 0011 already changed that.
- **Why not froxels.**
  - The count is bounded at 16, and a frame usually has one or two.
  - A projector is a frustum with separate horizontal and vertical fields
    of view. The cluster kernel's cone test would reach froxels outside it,
    or would need frustum-plane tests.
  - `ClusterLightGpu` (32 bytes) would have to carry a matrix, a cookie and
    a tile. The assignment kernel and its independent reference would need
    a third light kind.

  A per-pixel loop with an early frustum reject is cheaper to build and to
  prove. Revisit it as an optimization (binding rule 7) if a profile's
  measured frame time with projectors on screen exceeds its K7 budget; then
  assign projectors to froxels by their
  frustum planes, and the list stays the source of the records.
- **Handover from the CPU path.** `IRenderCoreWorld::RuntimeLight(surface)`
  sets `kProjectedLights` for exactly the surfaces the core shades with the
  list. The same change makes the core evaluate projected light, and
  `R_AddProjectedLights` then skips those surfaces. Models drawn by a family
  that reads the list stop taking the light cache's projected stand-in in
  the same way.
- **Not covered yet.** Lights limited to a target entity stay on the legacy
  flashlight path. The contract has no target key, and adding one is a
  `render.projected-light` version change. Displacements follow the world
  pass's displacement support.

## Lighting model (`render.lighting.v1`) (amended 2026-09-28)

User direction (2026-09-28): the core's lighting must reach Source 2
quality or better. Build the hard parts first and prove them in renders
outside the game, then integrate them. This section fixes the complete
model and names one owner per term. Gates K11 (prove) and K12 (integrate)
carry it. The [binding rules](#binding-rules-for-all-render-work-user-decision-2026-09-28)
govern all of it.

### The model

Every surface the core shades evaluates one sum. Legacy materials are
points of it (the surface model above), and the frame applies media and
output afterwards:

```
L_out = emission
      + sum over lights of  f(n, v, l) * L_in * visibility * (n . l)   direct
      + diffuse * (1 - E_spec) * E_indirect(n) * ao_diffuse               indirect diffuse
      + E_spec' * L_specular(r, roughness) * ao_specular                  indirect specular
then: participating media (fog, volumetric scattering), exposure and tone map, output encoding
```

`f` is the one BRDF. `E_spec` is its directional albedo and `E_spec'` the
energy-compensated form. `roughness` is RFC 0012's filtered roughness,
which feeds every term that reads roughness.

Per rule 6, a term defined by another RFC is linked, not restated. This
table defines only the terms this RFC owns; for the others it says what the
core implements.

| Term | Definition (the one owner) | What the core implements | Neutral |
| --- | --- | --- | --- |
| BRDF | RFC 0007 [Shading model](0007-physically-based-lighting-pipeline.md#shading-model); code `public/render/pbr_brdf.h`, GLSL mirror `render/shaders/common/pbr_brdf.glsl` | includes the mirror; never a second copy | the legacy point's own lobe (Lambert, half-Lambert, Blinn-Phong `$phong`, per the surface model) |
| Filtered roughness | RFC 0012 [Specular antialiasing](0012-antialiasing-msaa-specular-alpha-coverage.md#specular-antialiasing-renderpbr-specular-aav1) | feeds every roughness consumer in this table | input roughness |
| Runtime lights (points, spots, dlights, spark lights) | RFC 0011 [Runtime light set](0011-runtime-indirect-lighting.md#runtime-light-set-renderlight-setv1) | clustered evaluation (this RFC, [Lights and shadows](#lights-and-shadows-renderlightsv1-rendershadowsv1)): both lobes on every family, world surfaces included | no lights |
| Sun | this RFC, `render.shadows.v1`: cascaded shadow maps, practical splits, bounding-sphere cascades, texel snapping, a blend band between cascades | the cascade pass and receiver | no sun |
| Area lights | RFC 0011 [Area lights](0011-runtime-indirect-lighting.md#area-lights-light-set-v2-amendment-2026-09-28) (the rectangle, its radiance and `area_light::IrradianceAt`) | per-pixel linearly transformed cosines (Heitz et al. 2016) for the diffuse and GGX lobes, clipped to the horizon, LUTs in the frame group | no area lights |
| Projected lights | RFC 0011 `render.projected-light.v1` (the light and its rule) | the per-view projector list ([above](#projected-lights-a-per-view-projector-list-amended-2026-09-28)) | empty list |
| Direct visibility | this RFC, `render.shadows.v1`: atlas depth with a filtered comparison for spots, projectors and the sun; optional point-light cube shadows; moving objects as atlas casters. RFC 0011 decision 4 owns SDF shadows for unbaked lights without a tile | the atlas, caster passes and `shadow_sample.glsl`; one visibility per light and surface | visibility one |
| Indirect diffuse, static surfaces | values: RFC 0007 [Shading model](0007-physically-based-lighting-pipeline.md#shading-model); encoding: RFC 0008; runtime layer and change volume: RFC 0011 [Indirect-light policy](0011-runtime-indirect-lighting.md#indirect-light-policy-renderindirect-policyv1) | samples the lightmap basis at the mapped normal and applies the policy | lighting one (unlit) |
| Indirect diffuse, dynamic surfaces | RFC 0011 [Probe volume contract](0011-runtime-indirect-lighting.md#probe-volume-contract-renderprobe-volumev1) | samples the volume with visibility; the ambient cube where no volume covers the point | the ambient cube |
| Image-based specular | RFC 0007 [Image-based lighting](0007-physically-based-lighting-pipeline.md#image-based-lighting) (probes, parallax, relighting, distance-based roughness, split sum) | samples the probes by that definition | reflectance zero |
| Screen-space reflections | this RFC, `render.pass.ssr` (proposed): a hierarchical-depth trace for roughness below a cutoff (0.4 provisional), blended over the image-based specular by hit confidence (screen edge, thickness, roughness fade), spatial filtering only (RFC 0012 keeps TAA out) | the pass | no SSR: image-based specular alone, bitwise |
| Ambient occlusion | this RFC, `render.pass.ao` (proposed): material AO times GTAO (Jimenez et al. 2016) from depth and normals, with its multi-bounce fit, applied to indirect light only, never occluding twice what the bake already occludes | the pass | one |
| Specular occlusion | this RFC: from AO and roughness (Lagarde and de Rousiers 2014), applied to indirect specular only | in the surface program | one |
| Emission | the surface model ([above](#the-surface-model-legacy-materials-as-degenerate-cases-plan-2026-09-28)); RFC 0011 area lights for surfaces that light their surroundings | emissive radiance in scene units | zero |
| Participating media | this RFC, `render.pass.volumetric` (proposed): the legacy range and height fog as the legacy point; volumetric fog on a frustum-aligned froxel volume matching the cluster grid, with density from height fog and fog volumes, in-scattering from the light set, the sun's cascades and the projectors (cookies and shadows included), a Henyey-Greenstein phase and energy-conserving front-to-back integration (Hillaire 2015, after Wronski 2014). The volume may reproject its own history and drops it on a camera cut; that is not screen TAA | the pass and its application to opaque and translucent surfaces | density zero: legacy fog alone, bitwise |
| Output | this RFC, [`render.output.v1`](#output-renderoutputv1-amended-2026-09-28): exposure, one tone map (`tone_map.glsl`) and one output encoding (`color_encoding.glsl`) for the presentation's range and headroom | `render.pass.output` | the legacy point: scene peak 1 and headroom 1, the clip and the sRGB encoding alone |

Rules for the whole model:

- **One owner and one oracle per term.** Each term has a C++ reference
  that shares no code with its shader. A term at its neutral value is
  bitwise the term absent, checked with seeded mutants.
- **Each light counts once per surface.** `IRenderCoreWorld::RuntimeLight`
  generalizes to every term that the CPU lightmap path also produces. A
  term moves to the core in the same change that sets its flag.
- **One copy of the math.** The model's GLSL lives in `render/shaders/common`
  and the families. `world_pbr.frag`, `model_pbr.frag`, `probe_volume.glsl`
  and `reflection_probes.glsl` in the native backend move there as the one
  copy, and their C++ oracles are unchanged. Binding rule 4 deletes each old
  copy per surface set as its term moves; K12 and K9 check that none
  remains.
- **Profiles declare, they don't skip.** A term that a profile cannot
  support (a missing capability) is declared off by name in that profile's
  capability record. Being over budget is not a reason to turn a term off
  (binding rule 7); only the user can turn a term off for a profile's
  shipped default. A term that a profile declares is never silently
  dropped.

### Output (`render.output.v1`) (amended 2026-09-28)

User request (2026-09-28): HDR on the iPhone and the Apple TV, proven in
`render_lab` first. Presentation (RFC 0001, `render.presentation.v1`
"Dynamic range") owns the swapchain's format and color space and reports
the display's headroom H, in multiples of SDR white, every frame. This term
owns the values written for them, in `render.pass.output`:

1. **Exposure.** The linear scene times the frame's exposure. The legacy
   chain computes the exposure (auto exposure, the tone-map scale); the
   term applies it.
2. **Tone map** (`render/shaders/common/tone_map.glsl`, the one copy). Each
   channel is clipped to the scene peak P, the brightest value the scene is
   graded to (at most 10000/203, PQ's range). Then max(R, G, B) is mapped
   from [0, P] onto [0, H] by the ITU-R BT.2390 EETF: the Hermite knee in the
   SMPTE ST 2084 (PQ) domain, with reference white at 203 cd/m^2 (ITU-R
   BT.2408) and black at 0. The three channels take one scale, so hue is
   kept, and values below the knee pass unchanged. When H >= P, the term is
   the clip alone.
3. **Encoding** (`render/shaders/common/color_encoding.glsl`, the one
   copy), chosen by the target: an 8-bit UNORM target gets the sRGB curve;
   an 8-bit sRGB view gets linear values that its attachment encodes; a
   half-float target gets linear values in extended linear sRGB (scRGB,
   1.0 = SDR white) for a `kExtendedLinear` presentation. 8-bit targets
   take H = 1 only.

The legacy point is P = 1, H = 1: exactly the legacy clip and the sRGB
encoding, checked byte for byte. SDR is that degenerate case, not a second
path. A debug view (RFC 0014, "Post-processing is bypassed") gets the
encoding alone. BT.2390 is chosen because it is the standard curve for
showing graded HDR on a less capable display. It takes one input from the
display (its peak), and it is the identity whenever the display covers the
scene.

Not in this term: PQ (HDR10) or HLG output encodings, which a presentation
range needing them would add; a paper-white or brightness setting; auto
exposure.

Oracles:
- `render.output` (Linux GPU): a double-precision C++ reference, sharing
  no code with the GLSL, and six seeded fragment programs.
- `render.lab.hdr` (iPhone, Apple TV): the lab's presented swapchain image
  judged against the same reference.

The obligations are in `unittests/rendertest/contracts/render.output.v1.md`.

### Hard parts first, proven outside the game

Integration into the game is the cheap part once each hard part works; the
reverse order is how renderers get stuck at "almost". So the hard parts of
the model are built and proven first in **`render_lab`** (proposed), a
headless program that composes the core with no engine, no material system
and no legacy frontend:

- It reads a scene from the formats the core already serves: BSP2 maps
  through the `mapcontainer` readers (world mesh, `LMAP`, `PRBV`, `RPRB`,
  entity lights), studio models through `mdl`, and materials through
  `render.material`'s importers. It builds a `render.scene`, a light set
  and a `FrameDesc`, and renders through the Vulkan adapter to an image.
- Its fixtures are versioned scenes with cameras and Cycles references
  rendered by RFC 0007's pinned baker. They extend the RFC 0011 gallery
  (`quality/fixtures/gi/`, `tools/quality/gi_gallery.py`,
  `gi_oracles.py`) with a lighting set: a Cornell box with a rough and a
  polished floor, an area-lit room, a projector with a cookie, a sun
  through a colonnade, a foggy spot-lit hall, a mirror corridor, a clear
  coat and metal material sweep, and a Portal chamber and a Portal 2
  chamber rebuilt from their maps.
- "Source 2 quality" is judged objectively: against Cycles path-traced
  references (ground truth, which Source 2 itself does not reach) and by
  the relational oracles of the gallery, with a negative control for every
  term. No Valve Source 2 asset is used.
- Each hard part is done in the lab before its integration starts, and it
  stays in the lab's required suite after. The lab is also the place to
  tune a term: a lab render takes seconds, a game boot a minute.

The hard parts, in order of risk: the assembled surface program with
every term; LTC area lights; clustered lights with both lobes and atlas
shadows together; GTAO without double occlusion on baked light;
screen-space reflections over the probes; volumetric fog with shadowed
projectors and the sun. Their combined cost comes after (binding rule 7):
it is measured from the start, and optimized once they look right.

## Threading

| Sequence | Work |
| --- | --- |
| Game/main | Scene change sets, `FrameDesc`, legacy calls into `IMatRenderContext` (queued or immediate as today) |
| Render | Scene snapshot acquire, graph build and compile, legacy frontend replay, submission; the OpenGL context (the `MatQueue` thread in `mat_queue_mode 2`, the main thread in mode 0) |
| Compute pool | Culling, draw-list builds, pass recording, parallel emit conversion |
| GPU | Queues; completion observed only through tokens |

- `CMatQueuedRenderContext` is unchanged. The frontend runs on the render
  sequence, where device work runs today, so queued ordering is kept.
- Nothing recycles by frame index; per-slot fences are replaced by tokens.
- The serial configuration (one thread, serial executors) remains a
  supported low-capacity mode and the oracle for every parallel path.
- The core starts no threads (amended 2026-09-28). Its compute work
  (culling, draw lists, pass recording) runs on the root's workers,
  `RenderCoreConfig::computeWorkers`: in products the engine's compute pool,
  lent through `CreateComputePoolWorkerBackend`. A render-sequence caller
  never waits on a pool its own sequence runs on. RFC 0003 goal J3's thread
  census checks the first rule.
- Pass recording in parallel needs an encoder per job. The Vulkan adapter
  gives each worker its own command pools and records passes into separate
  command buffers, submitted in declaration order. Adapters without native
  parallel recording (OpenGL) record CPU command lists in parallel and replay
  them on the render sequence.

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

Every gate is a list of objective checks. Each check names the suite or
command that runs it (proposed ids; the conformance manifest records the
real ones when they are installed) and a pass condition a machine decides.
A gate passes only when every check passes on its required profiles, with
negative controls detected and evidence recorded (revision, profile,
inputs, counts, first divergence, reproduction commands). Missing required
hardware leaves the gate unverified, never passed.

**Performance checks do not block (binding rule 7).** Every check in these
tables that judges time, cost, memory, speedup or a budget (frame time,
submission cost, recording scales, budgets, the frame allowance) is a
performance check, marked *(perf)*. Each is run and recorded, with its
numbers, wherever its hardware is available. A failing or unmeasured
performance check neither holds its gate open, nor blocks integration or a
dependent row: it becomes an optimization item on the row. The other checks
decide whether a gate passes.

**Required profiles** for every gate: Linux desktop native Vulkan on
Wayland and X11 (`linux-native-vulkan-gpu` runner) and the headless core
runner. The Fold7 is required where a gate names it. Apple rows are
optional (AGENTS.md): run and record them when the runner is available.

**Common definitions.**

- *Pixel families*: every family in `tools/quality/material_pixel_conformance.py`,
  in both HDR modes.
- *K0 views*: the view oracle set captured in K0.
- *Byte-identical*: every pixel of every compared image equal.
- *Within tolerance*: inside the per-case tolerance recorded, versioned,
  in the fixture before the comparison runs.
- *Frame allowance* (perf): `portal-frame-pacing-v1` warm median at most
  1.05× and p99 at most 1.10× the K0 record for the same profile, unless
  `quality/budgets/render-v1.json` records a tighter row. It is an
  optimization target, not a blocker.

### K0: Prerequisites and frozen oracles

| Check | Runs as | Passes when |
| --- | --- | --- |
| Layer contract rule exists | `archlint` CAP011 and its fixtures (`python3 -m unittest discover -s tools/archlint/tests`) | the six seeded violations each fail with their rule; `archlint check --all` passes on the tree |
| Job system has a module | `archlint check --all`; `archlint targets --verify` | no unowned file under `public/jobsystem/` or `jobsystem/`; the `jobsystem` target declares `arch_module = jobs.graph` and has left `engine-and-tiers` |
| Shader compiler pinned | `shader.toolchain-pin` | the pinned compiler rebuilds every committed SPIR-V module byte-identically; a different compiler version fails the check |
| Frozen headers listed | CAP010 over `legacyAbi.paths` | the material, shader-API and studio headers above are listed and CAP010 passes |
| vtable fixtures | `legacy.render-abi` (legacy-cxx11 dialect) | slot offsets for `IMaterialSystem`, `IMatRenderContext`, `IMaterial`, `IMaterialVar`, `ITexture`, `IMesh`, `IMaterialProxy`, `IStudioRender` match their recorded tables; one seeded slot reorder per interface is detected (8 of 8) |
| View oracles captured | `render.view-oracles` | captures exist for portal recursion at every depth up to the limit, water reflection and refraction, a monitor, glass, the legacy-ports view set, on `testchmb_a_00`, `testchmb_a_08` and one Portal 2 map; removing any single draw from any view is detected |
| Per-draw fixtures captured | `render.draw-state` | per-draw state is recorded for the same maps; a seeded state change in one draw is detected |
| Budgets recorded (perf) | budget script over `quality/budgets/render-v1.json` | desktop Wayland and Fold7 rows exist with p50, p95, p99, GPU time and main-thread submission time, and the script passes |

### K1: Device port and the Vulkan and null adapters

| Check | Runs as | Passes when |
| --- | --- | --- |
| Port suite | `render.device.v2` on the Vulkan (GPU runner) and null (headless) adapters | all clauses pass; the check count is at least the manifest's `min_checks` |
| Bad adapters | `render.device.v2.sensitivity` | each of the ten bad adapters fails its named clause (10 of 10) |
| Port is backend-neutral | CAP007 and CAP005 with `--compile-deps`; CAP011 | no header under `public/render/device/` reaches a Vulkan, GL or SDL header; adapter public headers compile alone |
| One Vulkan stack | static scan `render.vulkan.allocation-sites` | `vkAllocateMemory`, `vkCreateBuffer` and `vkCreateImage` appear only under `render/device/vulkan/` |
| No idle waits on frame paths | static scan `render.vulkan.idle-waits` | every `vkDeviceWaitIdle` and `vkQueueWaitIdle` call site is in the reviewed list (teardown, mode change, loss recovery); a seeded call site elsewhere fails |
| Pixels unchanged | `material_pixel_conformance.py` | pixel families byte-identical to K0 |
| Boots and resize | `portal_boot.py` on `testchmb_a_01`, `testchmb_a_08`, `escape_00`, `escape_02` in `mat_queue_mode` 0 and 2; `--resize-stress` on Wayland and X11 | every run passes |
| Feature support recorded | `render.device.vulkan-features` evidence | timeline, synchronization2 and dynamic rendering support recorded for Linux desktop and the Fold7 (required) and for the iPhone and Apple TV when their runners exist |
| Frame time (perf) | `frame_pacing.py` | within the frame allowance on desktop and the Fold7; `tvos-portal-frame-pacing-60` still passes when its runner exists |

K1 also supplies the resource, upload and synchronization contract evidence
R32's done condition names.

### K2: Render graph

| Check | Runs as | Passes when |
| --- | --- | --- |
| Graph suite | `render.graph.v1` | all clauses pass on the null adapter and the Vulkan adapter |
| Independent model agrees | `render.graph.v1` (G7: an independent C++ reference model in the suite, sharing no code with the compiler) | culling, transitions, lifetimes and alias sets agree with the compiler on 1,000 seeded random graphs |
| Bad graphs caught | `render.graph.v1.sensitivity` | a missing transition, overlapping live aliases, a culled side-effect pass, a reordered dependent pass and a read of an undefined version are each detected (5 of 5) |
| Synchronization validated | Vulkan validation layer with synchronization validation during the pixel families and one `portal_boot` run | zero validation messages |
| Serial equals pooled | `render.graph.recording` on the null adapter | serial and pooled recording produce identical command streams (hash equal) on the 1,000 seeded graphs |
| Pixels unchanged | pixel families | byte-identical to K0 |

The present blit and gamma, MSAA resolve, scene capture and queued compute
move onto the graph in K2.

### K3: Inversion

| Check | Runs as | Passes when |
| --- | --- | --- |
| Pixels and views unchanged | pixel families; `render.view-oracles`; `render.draw-state` | all byte-identical to K0, in both queued modes |
| Side channels gone | static scan | no `"WorldMeshUpload007"`, `"RenderLightSetConsumer001"` or `"RenderGpuCompute001"` lookup remains, and `render_capability_queue.cpp` is deleted |
| Record replay gone | static scan | `CVulkanContext::BeginFrame`'s record replay has no caller |
| Products boot | `portal_boot.py` (K1 set) and a Portal 2 boot, both queued modes; `--resize-stress` | every run passes |
| Threading | the queued TSan lane | no signature outside the K0 triage list |
| Frame time (perf) | `frame_pacing.py` | within the frame allowance on desktop and the Fold7, in both queued modes (mode 2 added 2026-09-28: the products ship it) |

### K4: Shader library and materials

| Check | Runs as | Passes when |
| --- | --- | --- |
| Material suite | `render.material.v2` | schema, parameter-block and revision clauses pass; a seeded wrong key mapping and a stale-revision block are detected |
| VMT corpus | `render.material.vmt-corpus` over the Portal and Portal 2 VPKs | every VMT imports or is reported with its reason; zero crashes; the per-family count and the `unsupported` count are recorded |
| Families match ports | pixel families per family | `lightmapped`, `vertexlit`, `unlit` and `pbr` within tolerance of their legacy ports; `legacy` byte-identical to K3 |
| Proxy corpus | `render.material.proxies` | every proxy registered in Portal and Portal 2 runs on a fixture material and its output equals the legacy `IMaterialVar` value |
| Bind-group ceiling | family build validation | every shipped family uses at most four groups; a five-group fixture family fails the build |
| Artifacts per target | `render.shader-artifacts` | SPIR-V and GLSL 4.50 artifacts build for every family; reflection matches every declared layout; a seeded layout mismatch fails; the committed `*_spv.h` headers are deleted and the build reproduces them |

### K5: Scene, views, world and static props

| Check | Runs as | Passes when |
| --- | --- | --- |
| Scene suite | `render.scene.v1` | change-set, snapshot and revision clauses pass; a seeded publication without release ordering is caught by the TSan stress lane |
| Culling matches | `render.scene.culling` on the K0 views | visible world groups and props equal the legacy sets exactly, except items legacy's own view-frustum test rejects (amended below) |
| Serial equals pooled | same suite | serial and pooled culling give identical draw lists |
| Pixels | K0 views, world and prop draws | within tolerance |
| Two scenes | `render.scene.multi` | two scenes with different content render independently in one process, and destroying one leaves the other's handles valid |
| Submission cost (perf) | `frame_pacing.py` `render_submission` (amended below), both queued modes | on desktop, the median in `mat_queue_mode 2` at least 30 % below the K0 binaries measured interleaved in the same session (target set here, before optimizing), and mode 0 no worse than K0; within the frame allowance on the Fold7; the emit figure (`submission_*`) recorded beside it |
| Pooled recording | the K0 views in the product, `-render-core-record serial` against pooled (proposed switch) | the core's world and prop passes recorded on the compute pool give command streams hash-equal to serial recording on every view; `render_submission` recorded at 1 and 4 workers |

Culling amendment (2026-09-28, agent decision under the user's standing
instruction): legacy tests a BSP leaf in an area it sees through an area
portal against that area's frustum and not the view's, so it keeps leaves
wholly outside the view, which draw no pixel. The core culls every leaf with
the view's own planes. The first run found 757 such leaves across the four
K0 scenarios, every one outside the view frustum under legacy's own box
test, and no other difference. The check therefore requires equality except
for those items, and lists and verifies each one.

Submission amendment (2026-09-28, agent decision under the user's standing
instruction, agreed with the render-core owner): the check first read
"main-thread submission time", and `frame_pacing.py`'s `submission_*` is the
backend's emit alone, on whichever thread runs the backend. Neither measures
the cost K5 is meant to cut. A world the core draws is never emitted, so emit
falls even if the core's own recording costs as much. In `mat_queue_mode 2`
the main thread submits nothing, so moving work onto the render sequence would
pass with no saving. The check therefore reads `render_submission`: per frame,
the time the submitting sequence spends in `mesh_draw` (the legacy draws with
their emits), `record` (the stream replay, with the core's passes recorded
inside it) and `submit`, from `-vkframestats`. Work the core runs on the
compute pool counts as the time the sequence waits for it. A core pass
recorded outside `record` must add its own cost kind to the sum before its
numbers count. Mode 2 decides, because the products ship it. The reference is
the K0 binaries measured interleaved in the same session, as for K3's frame
time, since the K0 record holds only the emit figure.

### K6: Skinned models

| Check | Runs as | Passes when |
| --- | --- | --- |
| GPU equals CPU skinning | `render.skinning.corpus` over every model in the K0 views and the Portal 2 character set, captured from studiorender's software path | position error at most the larger of 1e-3 units and 4 ulp of the coordinate, and normal and tangent error at most 1e-3 per component, against the CPU oracle (tolerance version 2) |
| Defects caught | same suite | a seeded bone-index error and a seeded flex-weight error are detected |
| Pixels | skinning and model-light pixel families | within tolerance |
| CPU skinning retired where possible | runtime census on the K0 views | zero CPU-skinned draws on native profiles except materials that set `NeedsSoftwareSkinning` |
| Cost recorded | `frame_pacing.py` with `-vkframestats` | per-vertex skinning cost recorded next to the quoted 90 ns |

Tolerance version 2 (amended 2026-09-28, agent decision under the user's
standing instruction): the first corpus run put Portal 2 models near 7,000
units, where fp32 spacing is 4.9e-4. The legacy path blends bone matrices and
the kernel blends transformed points, so the two round differently by up to
3 ulp (1.46e-3 units). A flat 1e-3 bound therefore failed on rounding, not on
skinning. The seeded bone-index kernel misses by more than 2,000 times the
tolerance. Normals and tangents keep the flat 1e-3.

### K7: Lights and shadows

| Check | Runs as | Passes when |
| --- | --- | --- |
| Light assignment | `render.lights.clusters` | over 1,000 seeded scenes, no light that reaches a froxel is missing from it (zero false negatives), and the false-positive rate is recorded |
| Shadow oracles | `render.shadows` | a caster darkens its receiver, a non-caster does not, and cascade transitions stay within tolerance of a single-cascade reference render |
| Flashlight | Portal flashlight scene on native | shadowed pixels match the reference within tolerance; the `SetFlashlightState` census is zero |
| Projected lights | `render.lights.projected` (proposed) on native; `sp_a2_core` with `texturelight_wheatly_chamber` | judged pixels match `projected_light::IrradianceAt` times the shadow oracle; each surface's light is the same, within the cross-path tolerance, with `kProjectedLights` set (core) and unset (CPU lightmap); forcing both paths fails as doubled light; seeded defects are detected: a mirrored cookie axis, an ignored `cookieFrame`, the end falloff dropped, a shadow tile off by one |
| Area lights | `render.lights.area-ltc` (proposed) | diffuse within tolerance of `area_light::IrradianceAt` over seeded rectangles and receivers (horizon-crossing included); GGX specular within tolerance of a Monte Carlo integral over roughness 0.05 to 1; the LUTs regenerate byte-identically from their generator; seeded defects detected (no horizon clip, a transposed LUT, a one-sided light lit from behind); at most 0.3 ms at 1080p on desktop for 8 lights (RFC 0011's target) |
| Behavior decision | the dlight switch | the per-pixel and legacy dlight modes each match their own reference, and the decision is recorded. The per-pixel mode evaluates both lobes (diffuse and the surface's specular) for world surfaces, as for models |
| Budgets (perf) | `render-v1.json` atlas rows | pass on desktop and the Fold7 |

### K8: Remaining cohorts

For each cohort (particles, decals and overlays, sprites, beams and ropes,
post and screen effects, UI as a graph pass, water, sky, and portals,
mirrors and monitors as view generators):

| Check | Runs as | Passes when |
| --- | --- | --- |
| Pixels | the cohort's pixel families and K0 views | within tolerance |
| Legacy stream use | runtime census over the K0 views and `portal-frame-pacing-v1` | zero legacy-stream draws from the cohort on native Portal and Portal 2 |
| Portal recursion (portal cohort) | K0 recursion views | every depth up to the limit within tolerance |

### K9: Retirement

| Check | Runs as | Passes when |
| --- | --- | --- |
| First-party use is zero | runtime census plus a static ratchet | zero first-party legacy-stream draws on native Portal and Portal 2 over the K0 views and the frame-pacing workload; the ratchet rejects a new first-party caller |
| Frozen ABI holds | `legacy.render-abi` | all vtable fixtures pass |
| Mods still render | `render.legacy.mod-fixture` | a mod-style client DLL that draws through `IMatRenderContext` renders its reference image on the native profile |
| Dead code removed | static scan | the D3D9 translation code left in `materialsystem/shaderapivulkan/` has no caller, and `materialsystem/shaderapivulkan/shaders/` is empty and deleted |
| Recording scales (perf) | `frame_pacing.py` `render_submission` on `portal-frame-pacing-v1`, compute pool at 1, 2 and 4 workers (RFC 0003 J5's control) | desktop: 4 workers at most 0.6x of 1 worker (target set 2026-09-28, before measuring); the Fold7 and the iPhone 16 Pro: 2 workers no slower than 1; 1 worker within the frame allowance of serial recording |
| Render off the main thread | launcher census and `frame_pacing.py` per shipped native profile (Linux, Android, iOS, tvOS) | every shipped native profile runs the render sequence off the main thread (`mat_queue_mode 2` or its successor) (its frame budget is recorded as a performance check, rule 7); the main thread records no draws (zero `mesh_draw` and `record` cost on it) |

### K10: OpenGL adapter

| Check | Runs as | Passes when |
| --- | --- | --- |
| Port suite | `render.device.v2` on the OpenGL 4.5 adapter | every clause the adapter claims passes, including the conventions section; unclaimed capabilities are reported, not failed |
| No portable changes needed | CAP011; the backend-identity scan | CAP011 passes, and no portable module compares the backend identifier |
| Capability negotiation | `render.composition.capabilities` | with compute masked off, composition selects CPU skinning and reports each disabled feature by name; with a required feature missing, composition fails with a structured error |
| Pixels | pixel families on `portal-linux-gl` | within the cross-backend tolerance recorded per case against the Vulkan result (not byte-identical, per AGENTS.md) |
| Product boot | `portal_boot.py` on the GL profile | `testchmb_a_01` boots and renders in both queued modes |
| ToGL untouched | legacy renderer profile build | the ToGL legacy profile still builds and its existing checks pass |

### K11: Lighting model proven in `render_lab`

Every check runs in `render_lab` against its fixtures'
Cycles references, with no engine in the process. K11 needs only K1, K2
and K4 (all done) plus the K7 passes it drives, so it starts now and runs
ahead of the product rows.

| Check | Runs as | Passes when |
| --- | --- | --- |
| Lab composes the core alone | `render.lab.composition` (proposed); link map | `render_lab` links no engine, material system, legacy frontend or SDL; it renders a BSP2 fixture and a studio model through the Vulkan adapter with sync validation silent |
| Model assembly | `render.lighting.terms` (proposed) | one surface program evaluates every term of the model's table; each term's neutral value is bitwise the term absent (a seeded mutant per term detected); each term matches its C++ oracle on its synthetic cases |
| Ground truth | `render.lab.cycles` (proposed) over the lighting set and the RFC 0011 gallery | each fixture within its recorded per-fixture tolerance of Cycles (mean and 99th-percentile error in linear light, tolerances fixed before the run); every relational oracle of the gallery holds; each term's negative control (the term removed or seeded wrong) fails its fixture |
| Area, clustered and shadowed light together | the area-lit room, the spot-lit hall and the colonnade | 64 area lights and 256 clustered lights in one view within tolerance of Cycles; shadow edges of every light class in the right place (judged pixels as in `render.shadows.pixels`) |
| Ambient occlusion | `render.lab.gtao` (proposed) | a flat open plane gives AO one bitwise; crease and corner cases within tolerance of a ray-traced visibility reference; a direct-light-only scene is unchanged bitwise (AO touches indirect only); a fully baked static fixture is not darker than Cycles beyond its tolerance, and the double-occlusion control (AO over the bake with no rule) fails |
| Screen-space reflections | `render.lab.ssr` (proposed) on the mirror corridor | on-screen hits within tolerance of Cycles; off-screen and occluded rays fall back to the probes with no seam larger than the R50 walk gate's step; surfaces rougher than the cutoff are unchanged bitwise; seeded defects detected (thickness ignored, no edge fade, the wrong mip) |
| Volumetric fog | `render.lab.volumetric` (proposed) on the foggy hall | a homogeneous medium's transmittance is exp(-sigma_t d) within tolerance; single scattering from a point light matches a numerical integral; a shadowed projector's shaft is absent inside its shadow; density zero is bitwise the fog-only frame; after a camera cut no history remains |
| Portal and Portal 2 chambers | the two rebuilt chambers, legacy points and modern points (the S9 rule table) | legacy points match the product's native ports within the family tolerances; modern points within tolerance of Cycles |
| Output on a display | `render.output` (Linux GPU); `render.lab.hdr` on the iPhone and the Apple TV through `render_lab`'s presenting host (`render/lab/app`) | `render.output.v1`'s clauses and seeded programs pass; each chart patch presented on the device equals the oracle at the frame's headroom; the extended range is granted and the headroom rises above 1; on tvOS the TV switches into HDR; a debug view presents untouched; a standard presentation shows the legacy point |
| Gallery for review | a gallery page per term, published when the term's lab checks pass | each term's `render_lab` renders shown beside their Cycles references, with the term's negative control; the user's review is recorded in the progress entry. The user judges "looks right", which the tolerances cannot |
| Lab budgets (perf) | `render_lab --time` (proposed), `render-v1.json` lighting rows | set before measuring: all terms at once at most 8 ms GPU at 1080p on the desktop runner for the heaviest fixture; per term: GTAO 0.5 ms, SSR 1.0 ms, volumetric 1.0 ms, LTC 0.3 ms; Fold7 rows recorded; a miss is an optimization item, and no term is turned off for it |

### K12: Lighting model integrated in the product

| Check | Runs as | Passes when |
| --- | --- | --- |
| One light per surface | the `RuntimeLight` census and the doubled-light control | every term the core evaluates has its flag set on exactly the surfaces it shades; forcing both paths fails as doubled light for each term |
| One copy of the math | static scan | the model's GLSL exists only under `render/`; `world_pbr.frag`, `model_pbr.frag`, `probe_volume.glsl` and `reflection_probes.glsl` are gone from the native backend |
| Game matches lab | the Portal and Portal 2 chambers booted with the lab's cameras | each in-game frame within tolerance of the same scene's `render_lab` frame |
| Game output | Portal and Portal 2 on the iPhone and the Apple TV | the product presents through `render.presentation.v1` (no backend-owned swapchain), its frames reach it through `render.pass.output` at the presentation's headroom, and each profile records its declared range |
| Frame budgets (perf) | `portal-frame-pacing-v1` and a Portal 2 workload | within the frame allowance of K0 with every declared term on, per profile; the declared-off terms are listed per profile |

**Dependencies.** K0 is ready now (R02, R05, R10 and R16 are done). K1
needs K0. K2 needs K1. K3 needs K2. K4 needs K3. K10 needs K4, because the
OpenGL adapter needs per-target artifacts and runs the legacy frontend. K5
needs K4. K6 and K7 need K5 and are independent of each other. K8 needs
K5, and its UI cohort needs RFC 0010's draw list. K9 needs K6, K7, K8 and
K12. K11 needs K1, K2 and K4 and builds the K7 passes it needs; it does not
wait for K5–K7 in the product. K12 needs K11, K5, K6 and K7.

**The RFC is done** when K0–K12 have passed on their required profiles,
CAP011 and every suite above are required rows in the conformance manifest
and `quality/baseline.json`, rows R86–R92, R95 and R96 are `done` with linked evidence,
and the optional Apple rows are recorded as passing or unavailable.

## Roadmap

AGENTS.md owns ranks and states. Agent decision under the user's standing
instruction, 2026-09-26: rows are added as `planned` and ranked directly
after R47, because the user asked for a real graphics system and every
later render row (R65, R56, R50, R63, R36) would otherwise be built twice,
once on the translation layer and again on the core. R92 (OpenGL) is
ranked directly after R88: it needs R88's artifacts, and proving the port
with a second backend before the scene and lights build on it is cheaper
than finding Vulkan assumptions later.

| Phases | Row |
| --- | --- |
| K0–K1 | R86 |
| K2–K3 | R87 |
| K4 | R88 |
| K10 | R92 |
| K5–K6 | R89 |
| K7 | R90; R56 depends on it for clustered dynamic lights |
| K11 | R95, ranked directly after R88 (user direction, 2026-09-28: hard parts first, proven outside the game) |
| K12 | R96, ranked directly after R90 |
| K8–K9 | R91; needs R96 |

## Risks and mitigations

| Risk | Mitigation |
| --- | --- |
| The inversion changes pixels in ways the families don't cover | K0 adds view oracles and per-draw state fixtures before K1; K3 requires byte identity on all of them |
| Material proxy timing changes | Proxies run at draw extraction, the legacy bind point; the proxy corpus is a K4 gate |
| Portal recursion breaks | Portals stay on the legacy stencil path until K8; K0 captures every recursion depth |
| Queued-mode ordering breaks | `CMatQueuedRenderContext` is unchanged; the frontend runs where device work runs today; both modes are in every gate |
| Vulkan assumptions leak into the port | Abstract usages and conventions in the port; CAP005/CAP007 on port headers; the OpenGL adapter at K10 |
| The port sinks to the lowest common denominator | Capabilities, not a minimum API: Vulkan-only features stay available where declared, and features negotiate fallbacks |
| The core grows a second D3D9 state machine | The device port has immutable pipelines only; the D3D9 translation lives in the frontend and shrinks as cohorts move |
| Layers erode over time | CAP011 on declared edges plus CAP002/CAP005/CAP006 on real includes and links, all with seeded fixtures |
| Mobile GPUs regress | Pass merging is per-profile and measured; the Fold7 is in K1, K3, K5 and K7 gates; the Apple TV budget is kept |
| Required Vulkan features are missing on a profile | K1 records support per profile; a missing feature fails composition with a structured error |
| Parallel recording or culling races | Serial oracles for every parallel path; TSan lanes for the pooled executors |
| Two authorities during migration | Each cohort switches its stage from the legacy stream to the scene in one change; no object is drawn by both |
| Scope creep toward a new engine | Non-goals exclude new UI, particle and animation systems and GPU-driven rendering; each K gate is bounded |

## Alternatives considered

### Keep extending `CVulkanContext`

This is what got native Vulkan running. Every new feature adds a record
kind, a descriptor pool or a side channel, and nothing can express transient
targets, parallel recording, a second scene or a second API. The D3D9 state
machine stays at the center. Rejected as the long-term owner; kept as the
source of the Vulkan adapter's extracted code.

### A Vulkan-shaped port

The first draft of this RFC required timeline semaphores, synchronization2
stages and SPIR-V at the port. That is simpler for one backend and would
have made OpenGL, and any later API, a rewrite of the graph. Rejected at
the user's direction (2026-09-26); those are now Vulkan adapter details.

### SDL_GPU as the device layer

SDL3 is already the platform stack and SDL_GPU covers Vulkan, Metal and
D3D12. It does not expose completion tokens, memory aliasing or ray query,
which RFC 0006 and RFC 0011 need, and its resource model would sit between
the graph and the API. Rejected for the core; a future adapter could
implement `render.device.v2` over it if a profile needs one.

### An existing RHI library (NVRHI, Diligent, bgfx)

Each would add a large dependency with its own lifetime and threading
model beneath contracts this repository already defines, and none owns the
legacy frontend problem, which is most of the work. Rejected.

### Replace ToGL with the OpenGL adapter

The OpenGL adapter could carry the legacy frontend on GL, but only for the
GLSL ports. ToGL is what runs mod shader DLLs' D3D bytecode on GL.
Rejected for now (user decision, 2026-09-26); ToGL stays for mods.

### Replace the material system outright

Breaks mods, content and the frozen interfaces AGENTS.md preserves.
Rejected.

## Decisions (2026-09-26)

Decisions 1 and 2 are the user's (2026-09-26). The rest were taken by the
agent under the user's standing instruction ("do what you deem as
recommended for long-term improvements"). Each favors one owner and no
later migration over the cheapest first slice.

### 1. A backend-neutral port with Vulkan first and OpenGL second

The port names no graphics API; adapters implement it. Vulkan is the first
adapter because every north-star target runs it. OpenGL 4.5 is the second,
to prove the port (K10). Later adapters need their own decision.

### 2. ToGL stays for mods

The legacy D3D9 profiles keep ToGL, because it runs mod shader DLLs' D3D
bytecode. The OpenGL adapter serves the core only.

### 3. Four bind groups

Frame, view, material, draw. It is the Vulkan guaranteed minimum, it maps
to OpenGL's flat bindings, and the existing grouped PBR stages already fit
in three. The seven-set `$phong` layout moves to the `vertexlit` family's
four-group layout in K4.

### 4. Vulkan adapter: VMA, timeline semaphores, synchronization2, dynamic rendering

VMA is the standard suballocator (MIT), supports aliasing and budget
queries, and runs on MoltenVK and Android. The three features remove
render-pass object management and give the adapter one barrier model and
one token mechanism. All four stay private to the adapter; support is
recorded per profile at K1.

### 5. GLSL source, per-target artifacts

There are 47 core shaders and 86 ports in GLSL, and `pbr_brdf.glsl` is
checked against `pbr_brdf.h`. The pinned compiler plus pinned SPIRV-Cross
produce each adapter's format. A second source language would add a second
compiler pin for no current need.

### 6. Fixed conventions at the port

Depth 0 to 1, Y up in clip space, top-left origins, half-integer texel
centers. Adapters correct their APIs privately, so shaders and passes never
branch on the backend.

### 7. A layer contract checked by archlint (CAP011)

Layers are declared in `architecture/modules.json` and checked against the
declared edges, while the existing rules check the code against those
edges. A layer list in a README would drift; a manifest rule with seeded
fixtures does not.

### 8. The legacy frontend runs on the render sequence

That is where device work runs in both queued modes today, so ordering and
the thread-ownership census stay valid. The OpenGL context lives there too.

### 9. Portals move last

Portal recursion is the highest-risk legacy behavior and the most visible.
It stays on the bit-exact legacy path until the scene, materials and
lights are proven (K8).

## Later work

- GPU-driven culling, indirect draws and bindless material tables, after
  K5's measurements show submission still dominates.
- Mesh shading where profiles support it, after GPU-driven culling.
- ESSL artifacts and a GLES profile for the OpenGL adapter.
- A native Metal or D3D12 adapter, each with its own decision.
- A shared foundation math module, when a second strict consumer needs
  `render.math`'s types.
- Asynchronous compute scheduling beyond the optional queue fallback, per
  measured profile.

## Amendments to other RFCs

- **RFC 0001**: render migration step 9's "separate renderer RFC" is this
  RFC, and step 10's "genuinely different backend" is K10. The legacy
  render-services adapter (step 7) is the legacy frontend's input side.
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
- **Lighting model (2026-09-28).** RFC 0007: the BRDF, bake values and
  reflection probes keep their owner; its image-based lighting gains
  screen-space reflections over the probes and GTAO-based specular
  occlusion, both implemented here, and its Cycles references are
  `render_lab`'s ground truth. RFC 0011: per-pixel LTC area lights, the
  projector list and volumetric in-scattering are consumers of its light
  set; screen-space GI stays rejected. RFC 0012: the filtered roughness
  feeds the clustered, LTC, projected and SSR lobes too.

## Appendix A: File-level layout and wiring

Every name below is proposed; the first implementation of each module fixes
its spellings. The phase in brackets is the gate that creates the file.
Nothing listed here exists yet unless it is marked *existing*.

### A.1 Public headers

Public headers are the only way into a module. Ports hold interfaces and
value types; adapter headers hold one factory each.

```text
public/render/
├── render_backend.h … (existing, render.contracts; C++11, ABI-facing, unchanged)
├── math/                                   render.math                [K1]
│   ├── vector.h          float2, float3, float4
│   ├── matrix.h          float3x4, float4x4, composition and inverse
│   ├── bounds.h          Aabb, Sphere
│   └── frustum.h         Frustum, plane and bounds tests
├── device/                                 render.device (port)       [K1]
│   ├── device.h          IRenderDevice2, DeviceState
│   ├── provider.h        IRenderDeviceProvider2, DeviceRequest, DeviceProviderDescriptor
│   ├── facts.h           DeviceFacts, Capability, Limits, ArtifactFormat, diagnostic backend id
│   ├── resources.h       BufferDesc, TextureDesc, SamplerDesc, BufferId, TextureId, SamplerId
│   ├── pipeline.h        PipelineDesc, VertexLayout, RasterState, DepthStencilState, BlendState
│   ├── bind_group.h      BindGroupRole (frame, view, material, draw), BindGroupLayoutDesc, BindGroup
│   ├── encoder.h         CommandEncoder, RenderingScope, draw, dispatch, copy, clear
│   ├── usage.h           ResourceUsage (abstract states), SubresourceRange
│   ├── completion.h      CompletionToken, QueueKind, SubmitWaits
│   ├── conventions.h     the fixed conventions, as named constants
│   ├── errors.h          DeviceStatus, DeviceOperation, DeviceError
│   ├── vulkan/provider.h render::device::vulkan::Describe(), VulkanAdapterOptions   render.device.vulkan
│   ├── gl/provider.h     render::device::gl::Describe(), GlAdapterOptions           render.device.gl     [K10]
│   └── null/provider.h   render::device::null::Describe(), RecordedStream access    render.device.null
├── graph/                                  render.graph               [K2]
│   ├── graph_builder.h   GraphBuilder, PassDesc, PassKind, ResourceRef, Access
│   ├── compiled_graph.h  CompiledGraph, GraphError
│   ├── executor.h        SerialGraphExecutor, PooledGraphExecutor, RecordContext
│   └── trace.h           GraphTrace (passes, versions, transitions, aliasing, merges)
├── shaderlib/                              render.shader-library      [K4]
│   ├── artifact.h        ShaderArtifactRef, ArtifactKey (source, compiler, format, permutation)
│   ├── artifact_source.h IShaderArtifactSource (port: build store, development reload)
│   ├── permutation.h     PermutationAxis, PermutationKey
│   └── pipeline_recipe.h PipelineRecipe → device::PipelineDesc for a given artifact format
├── resources/                              render.resources           [K4]
│   ├── texture_cache.h   ITextureCache: residency, formats, revision, release by token
│   └── mesh_cache.h      IMeshCache: vertex and index residency, streaming uploads
├── material/                               render.material            [K4]
│   ├── family.h          FamilyDesc, ParameterSchema, PassKindSet, CapabilityRequirements
│   ├── material.h        MaterialId, MaterialInstance, revision
│   ├── parameter_block.h ParameterBlock, typed setters, dynamic block
│   ├── registry.h        FamilyRegistry (families register at composition)
│   └── vmt_import.h      ImportVmt(): Expected<MaterialDesc, ImportError>
├── scene/                                  render.scene               [K5]
│   ├── scene.h           IRenderScene, SceneFactory
│   ├── objects.h         MeshInstanceDesc, SkinnedInstanceDesc, LightDesc, ProbeVolumeDesc,
│   │                     ViewGeneratorDesc, typed handles
│   ├── change_set.h      ChangeSet
│   ├── snapshot.h        SceneSnapshot (immutable per frame)
│   ├── view.h            ViewDesc, SceneView
│   ├── visibility.h      IVisibilityProvider (port), VisibilityResult
│   └── draw_list.h       DrawList, SortKey, DrawPassKind
├── frame/                                  render.frame (port)        [K3]
│   ├── renderer.h        IRenderer, FrameDesc, FramePolicy
│   ├── feature.h         IRenderFeature, FeatureRequirements, FeatureContext
│   ├── stages.h          Stage (the legacy order)
│   └── stage_hooks.h     IRenderStageHooks
├── renderer/                               render.renderer            [K3]
│   └── renderer_factory.h CreateRenderer(RendererDeps): Expected<unique_ptr<IRenderer>, …>
├── pass/                                   render.pass.<feature>, one module each
│   ├── present/feature.h    present blit and gamma                    [K2 graph pass, K3 feature]
│   ├── post/feature.h       bloom, tone mapping, resolve              [K2 graph pass, K3 feature]
│   ├── world/feature.h      WMSH world, lightmaps, probes, reflection  [K5]
│   ├── props/feature.h      static props and brush models             [K5]
│   ├── skinning/feature.h   compute skinning, flex and morph          [K6]
│   ├── lights/feature.h     clustered light lists                     [K7]
│   ├── shadows/feature.h    shadow atlas                              [K7]
│   ├── indirect/feature.h   RFC 0011 producers' GPU work              [K3]
│   ├── particles/feature.h, decals/, sprites/, water/, sky/, ui/, portals/   [K8]
│   └── (each feature.h declares Create<Name>Feature(options) and its requirements)
├── legacy/                                 render.legacy-frontend     [K3]
│   ├── core_backend.h    RenderCore legacy shader provider (the LegacyShaderProvider pattern)
│   └── stage_markers.h   IRenderStageMarkers, "RenderStageMarkers001"
│                         (C++11, no foundation types, listed in legacyAbi.paths)
└── composition/                            render.composition         [K3]
    └── render_core.h     RenderCoreConfig, RenderCore, RenderCoreBinding,
                          RenderCore_Create/Destroy (C entry points for roots)
```

### A.2 Sources

```text
render/
├── wscript                         recurses into each module; client, tool and test builds only
├── math/            wscript, *.cpp
├── device/          wscript, validation.cpp, descriptor_hash.cpp       (port helpers)
│   ├── vulkan/      wscript, instance.cpp, adapter_select.cpp, device.cpp, queues.cpp,
│   │                allocator.cpp (VMA), upload_ring.cpp, pipeline_cache.cpp, bind_groups.cpp,
│   │                encoder.cpp, transitions.cpp, completion.cpp, loss.cpp, facts.cpp
│   ├── gl/          wscript, context.cpp, program_cache.cpp, state_cache.cpp, command_list.cpp,
│   │                bindings.cpp, fences.cpp, conventions.cpp, facts.cpp                  [K10]
│   └── null/        wscript, recording_device.cpp
├── graph/           wscript, builder.cpp, validate.cpp, transitions.cpp, lifetimes.cpp,
│                    merge.cpp, executor_serial.cpp, executor_pooled.cpp, trace.cpp
├── shaderlib/       wscript, artifact_store.cpp, permutation.cpp, recipe.cpp
├── resources/       wscript, texture_cache.cpp, mesh_cache.cpp
├── material/        wscript, registry.cpp, parameter_block.cpp, vmt_import.cpp
│   └── families/<name>/   family.cpp, *.vert, *.frag, *.comp
├── shaders/common/  pbr_brdf.glsl, …                                   (owned by render.material)
├── scene/           wscript, scene.cpp, change_set.cpp, snapshot.cpp, visibility.cpp,
│                    cull.cpp, draw_list.cpp
├── frame/           wscript, stages.cpp
├── renderer/        wscript, renderer.cpp, view_graph.cpp
├── pass/<feature>/  wscript, feature.cpp, *.glsl
├── legacy/          wscript, shader_device.cpp, shader_api.cpp, shader_shadow.cpp,
│   │                dynamic_state.cpp, legacy_stream_pass.cpp, named_targets.cpp,
│   │                framebuffer_copies.cpp, mesh.cpp, emit.cpp, emit_convert.h,
│   │                mesh_layout.cpp, stage_markers.cpp, core_backend.cpp
│   └── family/      legacy_programs.cpp, legacy_constants.cpp, *.vert, *.frag, *.glsl  (86 ports)
├── bridge/
│   ├── sdl3-vulkan/ wscript, presentation.cpp, surface_host.cpp          (moved)
│   └── sdl3-gl/     wscript, presentation.cpp, context_host.cpp          [K10]
└── composition/     wscript, render_core.cpp, feature_catalog.cpp, product.cpp
```

### A.3 Where today's files go

| Today | Destination | Phase |
| --- | --- | --- |
| `vulkan_device.cpp`: instance, device, queues, memory, uploads, pipelines, completion | `render/device/vulkan/` | K1 |
| `vulkan_render_backend.{h,cpp}` (test-only timeline provider) | `render/device/vulkan/completion.cpp`; the file is deleted | K1 |
| `vulkan_descriptor_groups.{h,cpp}` | `render/device/vulkan/bind_groups.cpp` | K1 |
| `vulkan_debug_utils.*`, `vulkan_frame_stats.h`, `vulkan_adapter.*` | `render/device/vulkan/` | K1 |
| `vulkan_device.cpp`: present blit, gamma pass, MSAA resolve | graph passes at K2; `render/pass/present/` and `render/pass/post/` features at K3 | K2–K3 |
| `vulkan_scene_capture.cpp` | `render/legacy/framebuffer_copies.cpp` (explicit copy passes) | K2–K3 |
| `vulkan_compute.cpp` | device dispatch in `render/device/vulkan/encoder.cpp`; producer work in `render/pass/indirect/` | K1–K3 |
| `shaderapivulkan.cpp` (`IShaderAPI`, `IShaderShadow`, `IShaderDevice`, emit, CPU skinning) | `render/legacy/` | K3 |
| `vulkan_mesh_layout.*`, `vulkan_emit_convert.h` | `render/legacy/` | K3 |
| `vulkan_legacy_pipeline.cpp`, `vulkan_legacy_programs.*`, `shaderapivulkan_legacy.*`, `shaders/legacy/` | `render/legacy/family/` | K3–K4 |
| `vulkan_texture_image.*` | `render/resources/texture_cache.cpp` | K4 |
| `vulkan_world_pbr.cpp`, `vulkan_model_pbr.cpp`, the PBR GLSL | `render/material/families/pbr/` | K4 |
| `shaders/pbr_brdf.glsl` | `render/shaders/common/` | K4 |
| `material_spv.h`, `material_spv_index.h`, `legacy_spv.h`, `regen_*_spv.py` | deleted; build-time artifacts from `tools/render/shader_artifacts.py` | K4 |
| `vulkan_world_mesh_upload.cpp`, `vulkan_world_lightmap.cpp`, `vulkan_world_reflection_probes.cpp` | `render/resources/mesh_cache.cpp` and `render/pass/world/` | K5 |
| `sdl3/` presentation bridge | `render/bridge/sdl3-vulkan/` | K1 |
| `materialsystem/render_capability_queue.{h,cpp}` | deleted | K3 |
| `public/render/world_mesh_upload.h`, `gpu_compute.h`, the side-channel parts of `light_set.h` | deleted once no caller remains; the light-set types stay in `render.contracts` | K3–K7 |
| `demo_*` shaders and bring-up code in `vulkan_device.cpp` | deleted (the bring-up suite moves to the port suite) | K1 |
| everything left in `materialsystem/shaderapivulkan/` | deleted | K9 |

### A.4 Tests, tools and quality records

```text
unittests/rendertest/
├── contracts/            render.device.v2.md, render.graph.v1.md, render.scene.v1.md,
│                         render.frame.v1.md, render.material.v2.md, render.visibility.v1.md,
│                         render.shader-artifacts.v1.md
└── core/
    ├── device/           device_conformance.h (shared suite, driver-parameterized),
    │                     test_device_null.cpp, test_device_vulkan.cpp, test_device_gl.cpp,
    │                     test_device_negative.cpp (the ten bad adapters)
    ├── graph/            graph_conformance.h, test_graph.cpp, test_graph_negative.cpp,
    │                     test_graph_recording.cpp
    ├── material/, resources/, scene/, frame/, legacy/, pass/<feature>/
    └── fakes/            fake scene, fake visibility provider, fake features
tools/render/
├── shader_artifacts.py   pinned compiler + SPIRV-Cross driver, reflection check
└── tests/
tools/archlint/           CAP011 in capabilities.py; fixtures under tools/archlint/tests/
quality/
├── conformance.manifest.json   one row per suite (render.device.v2, ….sensitivity, …)
├── profiles/linux-native-gl-gpu.json                                   [K10]
├── product_profiles/portal-linux-gl.json                               [K10]
└── budgets/render-v1.json      desktop and Fold7 rows                  [K0]
```

Suites follow the `platform.task-runner.v1` pattern: `testing::Checks`,
clause-prefixed check ids, a driver interface per adapter, and one
`checks-v1` record per run.

### A.5 Build wiring

- **Root `wscript`** adds `render` to the projects of client, tool and test
  builds. Dedicated builds do not add it, so a dedicated product cannot link
  it by construction.
- **`render/wscript`** recurses into each module directory. Adapter
  directories are conditional: `device/vulkan` and `bridge/sdl3-vulkan` when
  the Vulkan backend is configured, `device/gl` and `bridge/sdl3-gl` when
  the OpenGL device is configured, `device/null` always.
- **Each module's `wscript`** is a static library in the house form:

```python
def build(bld):
	bld.stlib(
		env=bld.strict_cpp20_env(),
		features='cxx capability_strict',
		arch_module='render.graph',
		target='render_graph',
		source=bld.path.ant_glob('*.cpp'),
		includes=['../../public'],
		use=['render_device', 'render_math', 'jobsystem'],
	)
```

- **Options.** `--render-core-device` (`null`, `vulkan`, `gl`; default `null`
  until K1) and `--render-core-features` (default `legacy-stream,present`) set
  the launcher's defaults; `--render-core-gl` builds the OpenGL adapter and
  fails configure until K10 adds it.
- **Dialects.** Every module target is `cxx20` in
  `quality/toolchain/policy.json`, `render_legacy` included: it keeps the
  strict environment and adds the platform's defines for its one legacy
  header, and it uses the core's standard-library ABI rather than the
  engine's `_GLIBCXX_USE_CXX11_ABI=0` (nothing ABI-sensitive crosses).
- **Pinned dependencies.** VMA and SPIRV-Cross are pinned archives in the
  product profiles, unpacked under `dependencies/` like DXVK Native. VMA is
  a `uselib` grant of `render.device.vulkan` only. SPIRV-Cross is a host
  tool used by the artifact task, never linked into a product.
- **Shader artifacts** are a Waf task per family and pass: GLSL in, one
  artifact per target format out, keyed by `ArtifactKey`, with the reflection
  check as part of the task.
- **The product library.** The existing `shaderapivulkan` product target
  links the core's static libraries from K1. At K3 it is renamed
  `rendercore` and declares `arch_module='render.composition'`, leaving the
  `render-legacy` group. The rename is reviewed in `sharedLibraries` (it
  stays in `ios-static-debt` until it is a static library everywhere).
  Under `--static-composition` it is a module object like today's backend,
  and `tools/quality/static_composition.py` lists it.

### A.6 Manifest wiring (`architecture/modules.json`)

Module rows follow the existing form. A portable module, an adapter and the
frontend:

```json
{ "id": "render.graph",
  "paths": ["public/render/graph/", "render/graph/"],
  "allowedEdges": ["foundation", "render.math", "render.device", "jobs.graph"] }

{ "id": "render.device.vulkan", "kind": "backend",
  "paths": ["public/render/device/vulkan/", "render/device/vulkan/"],
  "allowedEdges": ["foundation", "render.math", "render.contracts", "render.device"],
  "externalHeaders": ["vulkan/vulkan.h", "vk_mem_alloc.h"],
  "uselib": ["VULKAN", "VMA"] }

{ "id": "render.legacy-frontend", "kind": "legacy-interop",
  "paths": ["public/render/legacy/", "render/legacy/"],
  "allowedEdges": ["foundation", "render.math", "render.contracts", "render.device",
                   "render.graph", "render.shader-library", "render.resources",
                   "render.material", "render.scene", "render.frame"],
  "legacyIncludes": ["materialsystem/", "shaderapi/", "tier0/", "tier1/", "mathlib/"] }
```

The layer contract is one new section, read by CAP011. As installed
(R86-LAYOUT), abbreviated:

```json
"layerContracts": [ {
  "id": "render", "rfc": "0016", "prefix": "render.",
  "layers": [
    ["foundation", "render.math", "render.contracts", "jobs.graph"],
    ["render.device", "render.legacy-provider-contract"],
    ["render.graph", "render.shader-library", "render.resources"],
    ["render.material"],
    ["render.scene"],
    ["render.frame"],
    ["render.renderer", "render.pass.*", "render.legacy-frontend"],
    ["render.composition"],
    ["render.core-tests"] ],
  "externalBases": ["testing.contracts", "content.keyvalues-text", "content.texture-contract",
                    "content.ktx2-reader", "content.vtf-reader"],
  "independent": [ ["render.renderer", "render.pass.*", "render.legacy-frontend"],
                   ["render.device.vulkan", "render.device.gl", "render.device.null"] ],
  "adapters": { "render.device": ["render.device.vulkan", "render.device.gl", "render.device.null"] },
  "adapterConsumers": ["render.composition", "render.core-tests"],
  "planned": ["jobs.graph", "render.device.gl"],
  "outside": [ { "owner": "R91", "reason": "…", "modules": ["render.vulkan.core", "render.bridge.sdl3-vulkan", …] } ],
  "backendIdentity": { "identifier": "diagnosticBackend",
                       "allowedModules": ["render.device", "render.composition", "render.core-tests"] } } ]
```

`planned` lists ids declared before their module exists; an entry that
becomes a module must leave it. `outside` lists the render modules from before
the core (the native Vulkan backend in `shaderapivulkan`, its bridges and
tests, the legacy provider glue). They are exempt from rule 4, a layered
module may not depend on them, and the group shrinks as Appendix A.3 moves
their files; it is empty at K9.

Application code outside the manifest's modules (the launcher, Hammer's
roots, test fixtures) reaches adapters only through `render.composition`,
or directly in a test fixture. CAP011 checks the declared edges; CAP002 and
CAP005 check each file's includes against them, and CAP006 checks each
target's links.

### A.7 Runtime wiring

```text
launcher (composition root)
  │
  ├─ RenderCore_Create(config)                          render.composition
  │     config.device    ← -render-device vulkan | gl | null (default: Waf --render-core-device)
  │     config.bridge    ← the presentation pair for that device and the window provider
  │     config.features  ← -render-features (default: Waf --render-core-features)
  │     builds: device adapter → graph, shader library, resources → material registry and
  │             families → renderer and features → legacy frontend
  │
  ├─ MaterialSystem_BindShaderProvider(materialsystem, core legacy provider)
  │     existing call; the frontend wraps the backend -renderer selected and keeps its
  │     id; -norendercore composes no core; the material system keeps its API
  │
  ├─ Engine_BindRenderCore(RenderCoreBinding)            new, like Engine_BindLinkedGameModules
  │     IRenderer, SceneFactory, IRenderStageMarkers
  │
  └─ AddSystem(stage markers, "RenderStageMarkers001")   the client finds it through its
                                                         existing appSystemFactory lookup
```

Engine hooks (engine code includes port headers only; it links no render
module, so every implementation arrives through the binding):

| Engine file | Today | After |
| --- | --- | --- |
| new `engine/render_core_host.{h,cpp}` | — | Owns the world's `IRenderScene`, the BSP visibility adapter and the per-view `FrameDesc` [K3–K5] |
| new `engine/render_visibility_bsp.cpp` | WMSH culling inside `gl_rsurf.cpp` | `IVisibilityProvider` for BSP leaves, area portals and WMSH clusters [K5] |
| `engine/modelloader.cpp` | WMSH, lightmap, probe and RPRB uploads through `"WorldMeshUpload007"` | Scene objects and `render.resources` uploads [K3 typed, K5 scene] |
| `engine/gl_rsurf.cpp` | Draws world batches through the side channel | World drawn by `render.pass.world` from the scene [K5] |
| `engine/staticpropmgr.cpp` | Draws props through `IMatRenderContext` | `MeshInstance` objects [K5] |
| `engine/l_studio.cpp` | `IStudioRender` draws | `SkinnedInstance` objects [K6] |
| `engine/indirect_light_host.cpp` | Probe volume through the side channel, compute through `"RenderGpuCompute001"` | `ProbeVolume` object and `render.pass.indirect` [K3] |
| `engine/light_set_publisher.cpp` | `"RenderLightSetConsumer001"` | Publishes into the scene's light set, consumed by `render.pass.lights` [K3, K7] |
| `engine/host_frame_graph.cpp` | Host phases | Adds a render-extract node that commits change sets [K5] |
| `engine/host_render_steps.h` | Host render steps | `EngineFrameBegin` and `EngineFrameEnd` begin and end the core's frame (R86-LAYOUT) |
| `engine/gl_rmain.cpp` | `CRender::Push3DView`, `PopView` | Mark each 3D view's begin and end, so views nest without touching the client's push sites (R86-LAYOUT); builds `FrameDesc` and views [K3] |
| `engine/gl_rmisc.cpp` | `R_LevelInit`, `R_LevelShutdown` | Create and drop the world's scene (R86-LAYOUT) |
| `engine/view.cpp` | View setup for the material system | Builds `FrameDesc` and views [K3] |

Client, material system and other products:

| Place | Change |
| --- | --- |
| `game/client/cdll_client_int.cpp` | Looks up `"RenderStageMarkers001"` at init (R86-LAYOUT) |
| `game/client/viewrender.cpp` | Marks the skybox, opaque, translucent, view-model, post-process and HUD stages (R86-LAYOUT); stops drawing each cohort the scene takes over [K5–K8] |
| `game/client/portal/PortalRender.cpp`, `game/client/portal2/portal/portalrender.cpp` | Portal views as `IRenderStageHooks`, then view generators [K8] |
| `materialsystem/cmaterialsystem.cpp` | Side-channel `QueryInterface` branches and the side-channel fields of `LegacyShaderServices` removed [K3]; the public API is unchanged |
| `materialsystem/cmatrendercontext.cpp`, `cmatqueuedrendercontext.cpp` | Unchanged |
| Hammer | `hammer.adapters.render` projects the document into a scene the editor owns (R86-LAYOUT); the GTK viewport (R17) links `render.composition` to draw it; OpenGL in a `GtkGLArea` or the dmabuf bridge is R17's decision |
| Dedicated server | Links no render module and never calls `Engine_BindRenderCore`; its link map is the evidence |
| iOS and tvOS | `rendercore` is a module object; the static composition check lists it; nothing is loaded by name |

### A.8 Recipes

**Adding a feature pass.** Create `public/render/pass/<name>/feature.h` and
`render/pass/<name>/`, add a module row with edges to layers 0–5 only, add
the target to `policy.json` as `cxx20`, declare the feature's capability
requirements, add its suite under `unittests/rendertest/core/pass/<name>/`
and a manifest row, and add it to the feature catalog in
`render/composition/feature_catalog.cpp`. No other module changes.

**Adding a device adapter.** Create `public/render/device/<api>/provider.h`
(a factory with no native types) and `render/device/<api>/`, add a `backend`
module row with edges to `render.device` and layer 0 plus its native
grants, add it to the adapter list of the layer contract, run
`render.device.v2` and its sensitivity suite against it, add a
presentation bridge under `render/bridge/`, and register the adapter in
`render/composition/`. No portable module changes; if one has to, the port
is missing a capability, and the port changes first.

## Source references

- Yuriy O'Donnell, "FrameGraph: Extensible Rendering Architecture in
  Frostbite", GDC 2017.
- Hans-Kristian Arntzen, "Render graphs and Vulkan — a deep dive" (Granite
  blog, 2017).
- Unreal Engine documentation, "Render Dependency Graph".
- Alistair Cockburn, "Hexagonal Architecture" (ports and adapters), 2005.
- Ola Olsson, Markus Billeter and Ulf Assarsson, "Clustered Deferred and
  Forward Shading", HPG 2012.
- Rouslan Dimitrov, "Cascaded Shadow Maps", NVIDIA, 2007.
- Khronos, Vulkan specification, "Required Limits" (`maxBoundDescriptorSets`
  minimum 4), `VK_KHR_dynamic_rendering`, `VK_KHR_synchronization2`,
  timeline semaphores.
- Khronos, OpenGL 4.5 core specification (`glClipControl`, direct state
  access, sync objects).
- KhronosGroup/SPIRV-Cross.
- GPUOpen, Vulkan Memory Allocator.
- KhronosGroup/MoltenVK, portability subset documentation.

## Proposed decision

Adopt the render core as ports and adapters beneath the frozen material
API, with the layers enforced by CAP011. Deliver it through K0–K10 with the
inversion at K3 held to byte-identical pixels and the OpenGL adapter at K10
proving the port. Keep the legacy frontend permanently for mods, and ToGL on
the legacy profiles. Add rows R86–R92 as `planned`, ranked after R47, with
R92 after R88.
