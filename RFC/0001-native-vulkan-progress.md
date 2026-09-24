# RFC 0001 native Vulkan backend progress

Updated: 2026-09-24

This record tracks the *native* Vulkan material backend (`shaderapivulkan`),
distinct from the DXVK compatibility waypoint documented in
[the Portal SDL3/Vulkan slice](0001-portal-vulkan-progress.md). DXVK Native
translates the retained D3D9 material implementation to Vulkan; this work is the
ground-up native path (roadmap R28 bootstrap, then R32 functional MVP). The two
are separate providers and separate evidence.

The Video options on this backend (display modes, vsync, brightness, DirectX 95
caps, recommended configuration, MSAA, and the pending flashlight shadow depth)
are tracked in [the video options record](0001-native-vulkan-video-options-progress.md).

## Correction (2026-09-22): Portal does NOT yet render a real scene

An earlier version of this document claimed `portal_boot.py` "passes" and Portal
"renders a material-shaded scene." **That was a false positive and is retracted.**
The `has_scene_detail` gate counts distinct colors / midtone fraction, and it was
being satisfied by the screenshot buffer being filled with **uninitialized heap
memory** (the 4-arg `IShaderAPI::ReadPixels` overload the engine's `+screenshot`
path actually calls was an empty stub, so the TGA was random noise that trivially
has "32 distinct colors"). Inspecting the actual frame showed multicolor noise,
not geometry.

With the screenshot readback fixed (see below), the gate now correctly reports
the frame is **near-black / lacks scene detail**: the native backend brings up the
device, the material system now drives its real draw path, but the **world
geometry does not yet render** (only a handful of small overlay draws reach the
mesh path, and the frame's brightest channel is ~5/255). Real Portal rendering
remains unfinished; the material-math equivalence work below is verified in
isolation by the oracle, not yet by a real scene.

### Real bugs found and fixed while chasing this (all genuine improvements)

1. **`ReadPixels` (4-arg `Rect_t`/stride overload) was an empty stub** — the
   screenshot path wrote nothing, so captures were uninitialized memory (the
   "noise"/false pass). Now implemented: honors src/dst rects and destination
   stride, converts format, copies the captured frame.
2. **The index buffer was discarded** — index `Lock` handed out a single bogus
   `int` (`m_nIndexSize = 0`), so all authored indices collapsed and `Draw` drew
   vertices sequentially as a raw triangle list. Now a real index buffer is kept
   and `Draw` assembles triangles from it (indexed geometry).
3. **The matrix stack was fully stubbed** — `cModelViewProj` (committed from
   `MatrixMode`/`LoadMatrix`) never reached the shader, so geometry had no world
   transform. Now implemented (model/view/proj compose to the MVP; convention
   matched to CMatRenderContext's transposed hand-off and the GLSL column-vector
   shader).
4. **`IsUsingGraphics()` returned `false`** — the material system treated the
   backend as a headless/null device and skipped drawing entirely (`IMesh::Draw`
   was never called). Now returns true once the device is up, so the draw path
   actually runs.
5. **Common texture formats were skipped** — `BGRX8888` (most opaque textures),
   `I8`, and `BGR888` uploads were unsupported and dropped. Now handled.

These are real fixes, but they are not yet sufficient for the world to render;
the remaining gap is the world-surface geometry/material path (and lighting).

### Update: the world geometry now renders through the material path

A second, larger fix wired the **real material draw path**, which was entirely
bypassed. In the D3D9 backend, `IMesh::Draw` calls `IShaderAPI::DrawMesh` ->
`material->DrawMesh` -> `CShaderSystem::DrawElements` -> `IShaderAPI::BeginPass`
(selects the shader) -> the shader sets its constants/textures ->
`IShaderAPI::RenderPass` (draws the geometry with that state). This backend's
`CEmptyMesh::Draw` short-circuited all of it, drawing geometry with a fixed
default shader, so materials never executed (`BeginPass` was never called, only
`TakeSnapshot`). Now:

- `IShaderAPI::Bind(material)` records the material; `CEmptyMesh::Draw` records the
  mesh and calls `material->DrawMesh`, running the real shader path; `RenderPass`
  emits the geometry through the native dynamic path with the shader's selected
  pipeline, modulation, blend and alpha state.
- Result: the engine now binds the **real Source world/model shaders**
  (`LightmappedGeneric`, `vertexlit_and_unlit_generic`, `skin`, `refract`, ...)
  through `BeginPass`, and the world geometry flows to the GPU — from ~30 overlay
  draws to **8000+ draws / ~12M vertices per frame**. A diagnostic that forced the
  textured shader to output a solid color confirmed the geometry **fills the view
  with the correct transform** (the matrix stack is working for world geometry).
- Per-draw texture binding was added (each managed texture gets its own descriptor
  set; each draw binds its own), and the batch `CreateTextures` (which the texture
  manager uses for most VTFs) was implemented instead of returning null handles.

**Honest remaining gaps (the frame is not yet a recognizable scene):**
1. **World textures are not resident.** The world's base textures are not being
   allocated/downloaded through this path, so `CTexture::Bind` takes its
   not-allocated branch and calls `BindStandardTexture(TEXTURE_WHITE)`, which is a
   no-op here — leaving the built-in debug texture bound. The world therefore
   renders as a solid fill of the debug texture's color rather than its real
   textures. (A white standard texture reveals the lit geometry, but binding it
   destabilized the boot run and needs the residency/threading path finished.)
2. **No lighting.** Lightmaps (sampler1) and per-vertex/ambient lighting are not
   applied; only the base texture stage is sampled.
3. **Performance.** With ~12M vertices/frame now flowing through the naive
   per-draw dynamic path (a fresh interleaved vertex vector per draw, per-draw
   push constants and descriptor binds, re-rendered on demand for the screenshot),
   `portal_boot` sometimes exceeds its timeout. Batching/static residency is
   needed.

So the material-system integration is now genuinely exercised end to end (real
shaders, real geometry, correct transform), which is the substantive unblock; the
remaining work to a recognizable Portal frame is world-texture residency, lighting,
and draw-path performance.

## What is delivered and verified

A genuine native Vulkan **device bring-up and presentation core** now backs the
`shaderapivulkan` provider:

- [`materialsystem/shaderapivulkan/vulkan_device.{h,cpp}`](../materialsystem/shaderapivulkan/vulkan_device.cpp)
  owns the real Vulkan objects with explicit ownership and reverse-order
  teardown: instance (optional Khronos validation layer + debug messenger),
  SDL3 window surface, physical-device selection (graphics + present +
  swapchain, discrete preferred, `requireDiscreteGpu` honored), logical device
  and graphics/present queues, swapchain and image views, a clear render pass,
  framebuffers, per-frame command buffers, and per-frame + per-image
  synchronization. It presents real cleared frames, recreates the swapchain on
  `OUT_OF_DATE`/resize/zero-size, and can copy the rendered color image into
  host-visible memory for pixel-level verification.
- The core depends only on `<vulkan/vulkan.h>`, the SDL3 Vulkan surface entry
  points, and the C++ standard library, so the same code path is exercised by
  both the engine backend and the standalone smoke test with no ambient engine
  state.
- The backend's device manager (`CShaderDeviceMgrVulkan::SetMode`) brings this
  context up against the engine's SDL window (a `void*` that is an `SDL_Window*`
  on this SDL3 build, matching `shaderapidx9/winutils.cpp`), and the device's
  `Present()`, `GetWindowSize()`, `GetBackBufferDimensions()`, and
  `SpewDriverInfo()` report from and drive the real context.

A first **draw path** is also delivered and verified as the next increment
toward material rendering (R32): `InitDemoTriangle()` creates real SPIR-V shader
modules, a graphics pipeline matching the swapchain render pass (dynamic
viewport/scissor so it survives resize), and a host-visible GPU vertex buffer
with an interleaved position+color triangle, then `vkCmdDraw`s it inside the
frame. This exercises every building block the material renderer needs —
shader-module creation, pipeline layout/state, vertex input, buffer allocation
and upload, and draw submission. The SPIR-V is compiled from
[`materialsystem/shaderapivulkan/shaders/demo_triangle.{vert,frag}`](../materialsystem/shaderapivulkan/shaders)
with `glslc` and embedded in
[`demo_triangle_spv.h`](../materialsystem/shaderapivulkan/demo_triangle_spv.h);
regenerate by recompiling those files and re-running the embed step (the header
documents the exact commands). This demo pipeline is a bounded proof, not the
material path.

The **texture path** is delivered and verified too (`InitTexturedQuad()`): a
device-local RGBA texture uploaded through a host-visible staging buffer with
`UNDEFINED → TRANSFER_DST → SHADER_READ_ONLY` layout transitions and a
one-time-submit command buffer, a sampler, a combined-image-sampler descriptor
set layout/pool/set, and a pipeline that samples the texture onto a quad. This
exercises image creation and staged upload, image-layout barriers, samplers, and
descriptor sets. Shaders: `shaders/demo_texquad.{vert,frag}`.

Finally the **index-buffer and shader-constant path** (`InitIndexedUbo()`): an
index buffer drawn with `vkCmdDrawIndexed`, and a uniform (constant) buffer whose
color is bound through a descriptor set and written by the fragment shader. This
is the primitive the material renderer leans on most — every material sets shader
constants. Shaders: `shaders/demo_indexed_ubo.{vert,frag}`.

And **depth buffering** (`InitDemoDepth()`): a per-swapchain-image depth
attachment in the render pass, and two overlapping triangles drawn near-then-far
with depth testing on, proving the depth buffer resolves occlusion (the near
triangle wins regardless of draw order) — the capability real 3D scene rendering
needs. Shaders: `shaders/demo_depth.vert` (reusing `demo_triangle.frag`).

Together these cover the core rendering primitives a material renderer needs:
swapchain present, graphics pipelines and SPIR-V shaders, vertex and index
buffers, textures/samplers/descriptors, uniform/constant buffers, and a
depth-tested 3D pipeline.

### Outcome-driven evidence (this machine, 2026-09-22)

The bring-up is verified by pixels, not by a clean exit. The smoke test
([`unittests/shaderapivulkantest/test_vulkan_bringup.cpp`](../unittests/shaderapivulkantest/test_vulkan_bringup.cpp))
brings up the context on a real SDL3 window, presents cleared frames, reads the
presented image back, and asserts it matches the requested clear color:

```
[NativeVulkan] device 'AMD Radeon 8060S Graphics (RADV STRIX_HALO)' vendor=0x1002 320x240 images=4
[NativeVulkan] demo triangle pipeline ready
[NativeVulkan] textured quad pipeline ready
[NativeVulkan] indexed + uniform-buffer pipeline ready
[NativeVulkan] depth-test pipeline ready
native Vulkan bring-up: 20 checks, 0 failures    (exit 0)
```

The twenty checks include: context validity, a non-empty reported adapter name,
a non-zero swapchain extent, the presented center **and** corner pixels reading
back as the requested red, a **negative control** that fails if the verifier
would accept a wrong (green) expectation, a green frame after a live resize with
the swapchain rebuilt; with the demo triangle drawn over a red clear, the frame
**center reading back green (real geometry was rasterized) while the corner
stays red**; with the textured quad, the quad **center reading back magenta
(the uploaded texture was really sampled)**; with the indexed/uniform-buffer
quad, the center **reading back blue (the constant buffer reached the shader)**;
with the depth demo, the near blue triangle **occluding the later far green one
(depth test works)**; and context invalidity after `Shutdown()`. A clear-only
path could produce none of the geometry/texture/constant/depth results.
When no Vulkan device or display is available the test exits 77 (explicit skip),
so a missing lane cannot be counted as a pass.

### Reproduction

```sh
python3 waf configure -T release --platform-provider=sdl3 \
  --render-backend=native-vulkan \
  --product-profile=quality/product_profiles/portal-linux-wayland-native-vulkan.json \
  --build-games=portal
python3 waf build --targets=shaderapivulkan,native_vulkan_bringup_conformance
./build/unittests/shaderapivulkantest/native_vulkan_bringup_conformance
```

The `shaderapivulkan` shared library links the device core and builds clean in
tree. Like the DXVK presentation suite, this native-GPU suite is a waf target run
directly; it is **not** in `quality/conformance.manifest.json`, whose runner is
the headless-core compile-and-run model with no display/SDL3/Vulkan link.

## Generalizing the material system: a substitutable backend contract (LSP)

Beyond the primitives, the native Vulkan backend is now a conforming provider of
the **backend-agnostic render capability contract**
([`public/render/render_backend.h`](../public/render/render_backend.h),
RFC 0001 render.contracts) — the seam the material system depends on. This is
the "generalize the material system" half of the goal: the backend is
architecture-dependent below this line and portable above it, and D3D9/null and
Vulkan are interchangeable implementations of one behavioral contract.

- [`materialsystem/shaderapivulkan/vulkan_render_backend.{h,cpp}`](../materialsystem/shaderapivulkan/vulkan_render_backend.cpp)
  implements `render::IRenderBackendProvider` / `IRenderDevice` /
  `IRenderPresentation`, genuinely Vulkan-backed where the contract is a GPU
  fact: real instance and physical-device enumeration (real adapter id/name/
  vendor/memory/software-flag and semantic feature set), a real logical device
  and queue, real device-memory-backed buffer/image resources, and real command-
  buffer submission with `VkFence` completion. `PollCompletion()` waits the
  oldest submission's fence and advances completion in submission order, so the
  RFC 0006 rule ("a frame counter is not proof of GPU completion") holds against
  a real GPU. Presentation and device-loss are the contract's lifecycle state
  model (surfaceless here; real windowed pixels are proven by `CVulkanContext`).
- **The same shared conformance suite runs against it.** The Vulkan provider
  passes the exact `render::conformance::RunRenderBackendConformance` suite the
  null provider runs (`unittests/rendertest`) — **33 checks, 0 failures** on this
  machine (2 adapters enumerated: AMD RADV + llvmpipe). This is the LSP proof the
  AGENTS.md policy requires: one suite, every claiming implementation, plus the
  existing negative test proving the suite detects a deliberately broken
  provider. Target: `render_backend_vulkan_conformance` (built at C++17, skips
  with exit 77 when no Vulkan device is present).

```sh
python3 waf build --targets=render_backend_vulkan_conformance
./build/unittests/shaderapivulkantest/render_backend_vulkan_conformance
# render.backend.vulkan: 33 check(s), 0 failure(s) -> PASS
```

Routing the material system's own call sites off the legacy `IShaderAPI`
interfaces and onto this contract is the remaining migration (below); the
substitutable contract and its verified Vulkan provider are the foundation that
makes it possible.

## Material equivalence: an oracle, and faithful UnlitGeneric (2026-09-22)

"Comparable to DXVK/D3D9" is only a claim if something *measures* it. Until now
there was no oracle comparing the native output against the behavior the D3D9
shaders define -- so equivalence was unfalsifiable. This slice adds one and
brings the first shader family to real material fidelity against it.

- **Equivalence oracle** (`unittests/shaderapivulkantest/test_material_equivalence.cpp`,
  target `material_equivalence_vulkan_conformance`, **10 checks, 0 failures**).
  The reference is the actual Source shader source, not a guess:
  `unlitgeneric_ps2x.fxc` defines `result = i.vColor0 * tex2D( base, i.vTexCoord0 )`
  and `unlitgeneric_vs20.fxc` defines `vColor0 = cModulationColor`,
  `vTexCoord0 = mul( v.vTexCoord0, cBaseTextureTransform )`, so the shaded pixel
  is `cModulationColor * baseTexture( cBaseTextureTransform * uv )`. The oracle
  drives that formula through the material interfaces and reads pixels back:
  base-texture sampling, per-channel modulation `(1,0,0)*white -> red`,
  modulation of a real texture `(0,1,1)*yellow -> green`, monotone magnitude
  attenuation (`0.5x` strictly darker than `1x`), and the `cBaseTextureTransform`
  UV remap moving a sampled region. Checks are swapchain-format agnostic (0/1
  channel masks and blue/yellow texels survive sRGB and UNORM alike; the one
  magnitude check asserts only monotonicity), so the equivalence claim is honest
  across devices.
- **Real Source register conventions.** Every constant is set at its documented
  register (`common_vs_fxc.h`): `cModelViewProj` c4-c7, `cModulationColor` c37,
  `cBaseTextureTransform` c38-c39. `CShaderAPIVulkan::SetVertexShaderConstant`
  now keeps a register file and derives the material state from those real
  numbers (c0-c3 retained as a legacy alias for the pre-register harnesses), so
  the backend honors the D3D9 constant contract rather than a bespoke
  convention -- the substitutability the LSP seam promises. Two **negative
  controls** keep the oracle honest: it rejects the un-modulated (white) result,
  and modulation written to the wrong register (c20) is proven to have no effect.
  The pre-slice backend (no modulation, identity UV) fails four of these checks,
  which is the required proof that the oracle detects a non-conforming provider.
- **Faithful UnlitGeneric pipeline.** `shaders/demo_dyn_tex.{vert,frag}` now
  implement the exact D3D9 math: the vertex stage applies `cModelViewProj`,
  transforms the coordinate by the 2x4 `cBaseTextureTransform`, and passes
  `cModulationColor`; the fragment stage returns `modulation * texture(base,uv)`.
  The push-constant block carries `{ mat4 cModelViewProj; vec4 cModulationColor;
  vec4 cBaseTextureTransform[0]; [1] }` (112 bytes, within the guaranteed 128).
  `portal_boot.py` still passes (this change is additive: unset modulation/UV
  default to white/identity, so existing geometry is unchanged, and the c0 alias
  preserves the transform path).

- **Blend modes (`$translucent` / `$additive`).** The material records its blend
  state on `IShaderShadow` (`EnableBlending`/`BlendFunc`); `TakeSnapshot` now
  classifies it into the native compositing mode and carries it in the snapshot,
  and `BeginPass` selects the matching textured-pipeline variant (opaque, alpha
  blend `src.a*src + (1-src.a)*dst`, or additive `src + dst`, with depth-write off
  for blended geometry as Source does). The oracle verifies the D3D9 blend
  equations at 0/1 channels (format-agnostic): additive blue over a red clear
  reads magenta (`src + dst`) while the opaque control reads blue; `$translucent`
  with source alpha 0 shows the clear through and alpha 1 shows the source. Total
  oracle: **14 checks, 0 failures**.

This is one shader family reaching real fidelity against a reusable oracle, not
full equivalence. What the oracle does **not** yet cover, and the backend does
not yet implement faithfully: alpha test (`$alphatest`); the
`MatrixMode`/`LoadMatrix` matrix stack that commits `cModelViewProj` for
world/model geometry (still stubbed, so the transform depends on whatever sets
c0/c4 directly); sRGB/fog/tonemap `FinalOutput`; and the rest of `stdshader_dx9`
(VertexLitGeneric, LightmappedGeneric, ...). Each is a next slice, verified the
same way: extend the oracle with the D3D9-defined behavior, then implement until
it passes.

## Engine integration attempt (empirical `portal_boot.py` findings)

Driving the *actual engine* through this backend (advertising the adapter via
`GetAdapterCount`/`GetAdapterInfo`/`GetModeInfo` and bringing the device up in
`CShaderAPIVulkan::SetMode`) moved Portal boot forward through several real,
diagnosed blockers:

1. **Physics, not rendering, blocked boot first.** `MDLCache004` failed at the
   CONNECTION stage because `g_pPhysicsCollision` was null — an earlier change
   had pointed `portal_boot.py` at `-physics vphysics_box3d`, but Box3D (RFC 0004,
   still planned) provides no `IPhysicsCollision`. Restoring the working IVP
   `vphysics` (the build's `PHYSICS_BACKEND=ivp`) cleared CONNECT. Physics choice
   is orthogonal to the rendering goal; this is a harness fix, not a Box3D claim.
2. **The material system calls `IShaderAPI::SetMode`, not the device manager's.**
   The device was never brought up because bring-up was wired only into
   `CShaderDeviceMgrVulkan::SetMode`; `CMaterialSystem::SetMode` calls
   `g_pShaderAPI->SetMode`. Wiring `CShaderAPIVulkan::SetMode` brought the native
   device up (`AMD RADV`, 2880×1620, 4 material pipelines), and material
   `CreateTexture` calls then succeeded (0 failures, down from dozens).
3. **Process-exit segfault** was the global `CVulkanContext` destructor tearing
   down the swapchain at `atexit`, after SDL/Wayland was already gone
   (`wsi_wl_swapchain_destroy` → libwayland crash). Fixed by tearing the device
   down in `CShaderDeviceMgrVulkan::Shutdown()` during ordered shutdown while the
   surface is alive; the destructor is then a no-op.
4. **Blank screenshot** had two causes, both fixed: `IShaderAPI::ReadPixels` was a
   stub (the engine's `+screenshot` read nothing), and the dynamic mesh path
   batched a whole frame into one draw with the *last* transform/shader instead
   of per-object state. Fixes: `ReadPixels` renders the frame's queued geometry
   on demand and copies it back (format-converted); each `IMesh::Draw` now records
   its own transform/shader/constants and the frame replays every record with its
   own state; and the geometry queue is cleared at frame start (`ClearBuffers`),
   not after Present, so the last frame stays available for the screenshot.

Result: **`portal_boot.py` passes** — the scene renders with detail and the
process exits cleanly. Remaining fidelity work (full Source shader library,
render targets/lightmaps, remaining `IShaderShadow` state, VTF mip chains) is
tracked below; it improves accuracy but is no longer blocking a rendered frame.

## Scope boundaries — what is NOT done

This is the R28 bootstrap slice. It does not close R28 fully and does not begin
to satisfy R32:

- **Material rendering covers clear+present+mesh-geometry+shader-selection, not
  yet a full material scene.** The material-system-facing legacy interfaces drive
  native Vulkan through the faithful Source flow: `IShaderDeviceMgr::SetMode`
  brings up the device and the dynamic-mesh pipelines, `IShaderAPI::ClearColor4ub`
  sets the clear color, `IMesh` lock/write/unlock/`Draw` uploads a real
  position+color vertex list to a growable GPU buffer, the shader-binding path
  (`IShaderShadow::SetPixelShader` → `IShaderAPI::TakeSnapshot` → `BeginPass`)
  selects which material shader the draw uses, and `IShaderDevice::Present`
  presents. `material_facing_vulkan_conformance` drives those exact interfaces
  (via `ShaderBackend_Create`) and reads pixels back: a chosen clear color, then
  the **same blue-vertex mesh rendered blue (passthrough shader), green
  ("greenify"), and yellow ("constant color" shader driven by a pixel-shader
  constant via `IShaderAPI::SetPixelShaderConstant`)**, and finally **moved
  off-center by a model→projection matrix set in vertex constants c0–c3
  (`SetVertexShaderConstant`)**, and a **UV-sampled 2-tone texture under a
  `$basetexture` shader** (left red, right green) — proving shader selection
  (distinct native pipelines), shader constants, the vertex transform, and
  texture sampling all reach the GPU through the material interfaces —
  including **material-supplied textures** created and uploaded via
  `IShaderAPI::CreateTexture`/`TexImage2D`/`BindTexture` — both uncompressed
  (RGBA/BGRA/RGB888) and **block-compressed DXT1/DXT5 → BC1/BC3 sampled natively**
  (a DXT1 block reads back correctly), which is Portal's actual VTF texture
  format. What is still stubbed: the rest of `IShaderShadow` render state
  (blend/depth/cull), render targets, full constant-register banks, arbitrary
  `VertexFormat_t` layouts, VTF file loading / mip chains, and — the dominant
  piece — the real Source shader library (this uses a bounded four-entry SPIR-V
  catalog, not `stdshader_dx9`). So a real material-shaded scene (Portal) is not
  yet drawn.
- **No masquerade.** `CShaderDeviceMgrVulkan::GetAdapterCount()` and
  `CShaderDeviceVulkan::IsUsingGraphics()` are deliberately left reporting the
  stub values. Flipping them to advertise a working graphics backend while the
  material path is empty would drive the engine into paths the stub cannot
  satisfy and would violate the RFC's rule against unsupported required behavior
  succeeding silently. `portal_boot.py` (which requires a real rendered scene,
  an active player, and a fresh screenshot) therefore cannot pass on this path
  yet; that is an R32 gate, not a configuration fix.
- **Validation layer unavailable on this host.** `VK_LAYER_KHRONOS_validation`
  is not installed here, so validation ran off. The code enables it opportun-
  istically and can require it (`-vkvalidate`, or the test's
  `--require-validation`); a run with the layer present, and Vulkan validation /
  sanitizer lanes, remain required evidence before R28 closure.
- **One host, one GPU.** Verified only on this machine's AMD RADV device on
  native Wayland. Other adapters, the required-profile-failure diagnostics on a
  device that genuinely lacks a feature, and the four north-star platforms are
  unverified.

## Path to DXVK-comparable rendering (R32 → R36)

The goal is native Vulkan rendering comparable to the DXVK compatibility path
(which renders Portal by translating D3D9 at runtime). The verified primitives
above are the foundation; the remaining, genuinely large, work is the
material-system integration, in dependency order:

0. **Clear + present through the material interfaces — DONE.**
   `SetMode`/`ClearColor4ub`/`Present` on the legacy contract drive native
   Vulkan, verified by `material_facing_vulkan_conformance` (pixel readback).
1. **Meshes and buffers — first slice DONE.** `IMesh` lock/write/unlock/`Draw`
   now uploads a position+color vertex list to a growable GPU buffer and
   rasterizes it via native Vulkan (verified: a mesh-drawn triangle reads back).
   Remaining: honor arbitrary `VertexFormat_t` layouts, index buffers,
   `CreateVertexBuffer`/`GetDynamicVertexBuffer`, and static-mesh residency.
2. **Shaders — binding mechanism DONE, first real shader mapped, library port
   remaining.** The faithful binding flow (`IShaderShadow::SetPixelShader` →
   `TakeSnapshot` → `BeginPass` → pipeline selection) works, and the **first real
   Source shader name is mapped**: `unlitgeneric_ps*` routes to the native
   textured pipeline (UnlitGeneric samples `$basetexture` with the material
   transform, which that pipeline implements) — verified by driving the real name
   through the material interfaces. Remaining: the rest of the Source shader
   library (VertexLitGeneric, LightmappedGeneric, and the hundreds of others with
   their static/dynamic combos) needs SPIR-V equivalents and the permutation/
   selector mapping, plus the material-system/map-loading integration that feeds
   real materials. This is the largest single piece; DXVK avoids it by translating
   D3D9 bytecode at runtime, so a native renderer either ports the shader library
   to SPIR-V or grows a bytecode→SPIR-V translator.
3. **State and constants.** Translate render/sampler/blend/depth state and the
   shader constant registers `IShaderAPI` sets into pipeline state + descriptor/
   push-constant updates, with a pipeline cache keyed on state.
4. **Textures and render targets.** Wire `IShaderAPI` texture creation and VTF
   upload onto the texture path above, plus render-target/depth attachments.
5. **Enable the backend.** Only once the material path can satisfy the engine do
   `GetAdapterCount()`/`IsUsingGraphics()` flip to advertise graphics; then
   `portal_boot.py` becomes the real end-to-end gate, and R36 adds per-platform
   materials, loss/recovery, budgets, and packaging.

Each step is independently testable the same outcome-driven way (build the
primitive, drive it, read the pixels back). None is delivered yet.

## Restoring the DXVK compatibility renderer (2026-09-22)

The `vulkan-compat` (shaderapidx9 -> DXVK Native) renderer had stopped working
entirely. Three independent defects, all now fixed:

1. **The backend was no longer built.** An exploratory change removed
   `materialsystem/shaderapidx9` from the top `wscript` project list, and its
   pinned DXVK Native 2.7.1 tree had been deleted along with the build output it
   lived inside. The SDK is restored from the profile-pinned archive (sha256
   matches `quality/product_profiles/portal-linux-wayland.json`) and now extracts
   to a repo-level `dependencies/` directory instead of inside a product's output
   directory, so one product's staging no longer scans another's dependency.
2. **Colliding provider entry points.** `shaderapidx9`, `shaderapivulkan`, and
   `shaderapiempty` each exported `ShaderBackend_Describe`/`ShaderBackend_Create`,
   and `shaderapivulkan` exported a second `NullShaderBackend_Describe`. A product
   links several of these at once, so the dynamic linker bound every caller to
   whichever module it resolved first and the rest became unselectable: the
   launcher's catalog had exactly one real entry and `-renderer vulkan-compat`
   reported "not available in this product". Each backend now exports its OWN
   named entry point (`Dx9ShaderBackend_*`, `NativeVulkanShaderBackend_*`,
   `NullShaderBackend_Describe`), the composition root enumerates every backend
   the build linked, and `CMaterialSystem` re-selects the provider it actually
   bound instead of consulting an ambient symbol.
3. **Inverted portal occlusion-query guards.** In `PortalRender.cpp`,
   `AllocPortalViewIDNode` skips creating the query handle under
   `#ifndef TEMP_DISABLE_PORTAL_VIS_QUERY`, but the three function bodies that
   consume it guarded their early `return;` the same way, so defining the
   "disable" macro *enabled* the consumers on a handle that was never created.
   The D3D9 path survived only because `ShouldUseStencilsToRenderPortals()`
   returned first; the native backend reports no stencil bits, took the texture
   path, and segfaulted in `COcclusionQueryMgr::BeginOcclusionQueryDrawing`.
   Those three guards are now `#ifdef`, so the macro disables the feature
   consistently with the five call sites that always guarded it correctly.

Evidence: `portal_boot.py --renderer vulkan-compat --require-vulkan
--require-sdl3 --require-wayland` **passes** on `testchmb_a_01` with a real
textured, lit frame (1024x768, midtone fraction 0.999, 32 distinct colors). The
native product boots without the segfault and still fails only its own
scene-detail gate, which is the open work above. `DXVK_WSI_DRIVER=SDL3` is
required by both the product and `dxvk_presentation_conformance`; `portal_boot.py`
sets it only under `--require-vulkan`.

Suites: `material_binding_conformance` 248/0 (both products),
`builtin_shader_conformance` 298/0, `dxvk_presentation_conformance` 86 checks,
`material_equivalence_vulkan_conformance` 17/0, `material_facing_vulkan_conformance`
13/0, `native_vulkan_bringup_conformance` 20/0, `render_backend_vulkan_conformance`
33/0.

`material_binding_conformance` gained a catalog oracle that mirrors the
composition root: every linked provider must have a distinct id, module name, and
**factory pointer**, and each factory must return its own module's services. A
negative control that adds a duplicate catalog entry (simulating the collision)
produces 5 failures, so the suite detects exactly this defect class.

## Native backend: two more fixes, and where it is still blocked (2026-09-22)

Fixed:

1. **The world rendered black because unwritten constants were zero.** Only the
   UnlitGeneric family publishes modulation and the base-texture transform through
   `c37`/`c38-c39`. `LightmappedGeneric`, `vertexlit_and_unlit_generic`, `skin` and
   the rest never touch those registers, yet the native textured pipeline
   multiplies by them on every draw, so a zero-initialized register file produced
   `colour * 0` for the entire world and collapsed every UV to the origin.
   `VsConstantFile` now starts at the neutral values (modulation white, transform
   identity) and both are committed on every draw. Measured before/after on a real
   frame: `mod=(0,0,0,0)` -> `mod=(1,1,1,1)`, `uv0=(0,0,0,0)` -> `uv0=(1,0,0,0)`.
2. **The portal occlusion-query segfault** described above, which killed the
   native run outright (`SIGSEGV` in `COcclusionQueryMgr::BeginOcclusionQueryDrawing`).

Verified good along the way, so these are no longer suspects: the committed
transform (world vertices project to sane NDC, e.g. `(0.779, -0.726, 0.995)`,
inside the frustum and in front of the 1.0 depth clear), the draw path itself
(~90-170 draws and 80k-190k vertices per frame reach the queue), the render-pass
clear value, render area, viewport/scissor, framebuffer indexing, swapchain
extent and image count, and the absence of any swapchain recreation loop.

**Open blocker: the swapchain -> capture readback yields one column.** Every
capture of an in-game frame comes back black except the final pixel column, at
both 2880x1620 (Wayland) and 1920x1080 (X11). The engine's `+screenshot` reads the
same captured frame, which is why `portal_boot.py` reports a blank capture even
though the window itself shows geometry. Bisected as follows:

- Replacing the frame copy with `vkCmdClearColorImage` on the capture image
  returns a perfect full-extent green readback, so the capture image, its linear
  tiling, `rowPitch` (exactly `width * 4`), memory mapping, invalidation and the
  BGRA conversion are all correct.
- `vkCmdBlitImage` from the swapchain image produces the identical one-column
  result as `vkCmdCopyImage`, so it is not specific to the copy command.

That isolates the fault to reading the swapchain image as a transfer source.
Next steps: obtain the Khronos validation layer (`vulkan-validation-layers` is not
installed here and the flatpak runtime ships only the manifest, not the library)
and run with `-vkvalidate`; failing that, render the scene into an owned offscreen
colour image and blit that to the swapchain, which removes the dependency on
swapchain images being readable and is needed for render targets anyway.

Also observed: the naive per-draw path is slow enough (~50-200 ms/frame at
190k vertices) that the boot run is sometimes killed at its timeout.

## Configuration notes for follow-up

Two product build trees are configured side by side, and the render backend is a
configure-time choice because each needs a different SDK:

- `build/` — `--platform-provider=sdl3 --render-backend=native-vulkan
  --build-games=portal --disable-warns -T release -o build`
- `build-portal-vulkan/` — `--platform-provider=sdl3 --render-backend=vulkan
  --dxvk-root=dependencies/dxvk-native-2.7.1/usr --build-games=portal
  --disable-warns -T release -o build-portal-vulkan
  --prefix=/tmp/source-engine-portal-vulkan`

Reconfiguring does not delete a removed project's `c4che/<project>_cache.py`, and
`portal_boot.py` reads every cache it finds; a stale one made staging report
"DXVK staging requires one configured product profile and dependency root".
Delete the orphaned cache and output directory when a project leaves a product.

`tools/quality/portal_boot.py` also carries an edited physics module argument
from earlier exploration; that remains independent of this slice.

## Measuring the backend instead of guessing at the frame (2026-09-22)

Four conformance suites were green (bring-up 20, LSP 33, material-facing 13,
material-equivalence 17) while the product frame was black. That combination is
the finding: those suites exercise paths written on purpose, and the backend
answered a large part of `IShaderAPI` with do-nothing bodies that no suite and
no log could see. A silent no-op is indistinguishable from correct behavior at
the call site, so a wrong frame carried no information about which dropped call
produced it.

A static audit of `shaderapivulkan.cpp` counted **166 empty bodies and 76
trivial returns against 76 implemented methods**. (The first pass of that audit
reported 136 because its signature matcher only handled single-line signatures;
it therefore missed the entire render-target family — `SetRenderTarget`,
`SetRenderTargetEx`, `CopyRenderTargetToTextureEx`, `CopyTextureToRenderTargetEx`,
`CopyRenderTargetToScratchTexture`, `SetScissorRect` — whose signatures wrap.
The blocker below was found through the texture-residency counters instead, and
the census matcher has since been fixed: `SetRenderTargetEx` alone records
**30,927 ignored calls** per run.) Rather than pre-judge which of
the 136 mattered, each now records itself in an unimplemented-entry census
(pointer-keyed on `__func__`, no allocation) reported at the screenshot, next to
a primitive-type histogram and per-draw base-texture residency. One boot of
`testchmb_a_01` then ranked the entire gap by call count. That measurement
replaced three standing hypotheses, two of which were wrong:

- **Wrong:** "the screen-covering draws use `BindStandardTexture`". `BindWhite`,
  `BindLightmap`, `BindFBTexture` and the rest recorded **zero** calls.
- **Wrong:** "depth state is missing". `EnableDepthTest`/`DepthFunc` are indeed
  empty, but `InitDynamicMesh` already hardcodes depth test + write, and its
  blend variant correctly disables depth write.
- **Right, and quantified:** `SetPrimitiveType` was an empty body called 36,719
  times, and **15,514 of 36,456 draws (43%) were `TRIANGLE_STRIP`** while every
  native pipeline rasterizes `TRIANGLE_LIST`. That is the "bunch of broken
  triangles" reported against this backend, measured.

### Defects found and fixed

1. **Primitive topology ignored.** `CEmptyMesh` now records the declared
   `MaterialPrimitiveType_t` and `EmitToNativeQueue` assembles the triangles it
   implies — strips with alternating winding and degenerate-stitch rejection,
   polygons fanned, quads split — instead of assuming the indices already form a
   list. Line and point topologies are dropped rather than assembled as
   triangles. A missing bound on the vertex lock was also fixed: a lock larger
   than `VERTEX_BUFFER_SIZE / kMeshVertexStride` overran a heap buffer.

2. **The default texture was a debug pattern.** The textured shader *multiplies*
   by its sample, so the only correct default is opaque white — the
   multiplicative identity, matching what D3D9 gets from `TEXTURE_WHITE`. The
   built-in was a 2x1 red/green texture, which turned every texture-residency
   gap into whole-frame corruption that read as a raster bug. Changing it moved
   the capture from 3 distinct colors at 99.8% midtone to 32 colors.

3. **The world was multiplied by zero vertex color.** `m_pVertexMemory` was
   uninitialized heap, and most Source vertex formats — the lightmapped world
   format above all — carry no color, so the mesh builder never writes that
   field. The shader multiplied the entire world by whatever the allocation
   held. The color field is now seeded to opaque white at lock. This is the same
   error as (2): **a multiplicative input defaulting to zero.**

4. **A stale "skip the next draw" flag.** `BindTexture` set a global consumed by
   the following emit, inside an unbraced `if` that swallowed the assignment, so
   a draw that bound no texture inherited the previous draw's decision. It also
   ran *before* the census, which undercounted the real problem by 8x. Replaced
   with a decision made at emit time from the state it depends on; the flag and
   its `BindStandardTexture` producer are gone.

Boot status moved from `fail` ("engine capture lacks scene detail") to
**`portal_boot.py --renderer native-vulkan`: pass**, `has_scene_detail: true`,
32 distinct colors, midtone fraction 0.059. All four suites remain green (one
regression was caught and fixed in the process: the residency gate initially
suppressed draws from the vertex-color/greenify/constant-color pipelines, which
do not sample `$basetexture` at all).

### The named, quantified blocker: no render targets

With residency accounting per draw, every texture ever sampled while empty is
named. There are exactly six, all render targets:

| binds while empty | texture |
| --- | --- |
| 14,332 | `_rt_portal1` |
| 2,591 | `_rt_smallfb0` |
| 1,819 | `_rt_portal2` |
| 698 | `_rt_fullframefb` |
| 457 | `_rt_shadows` |
| 363 | `_rt_smallfb1` |

**57% of all draws sample a render target this backend does not implement.**
Inspecting the capture confirms the other half of the same gap: the frame shows
real textured geometry composited from several camera viewpoints at once,
scattered and overlapping. With `SetRenderTarget` unimplemented, Source's portal
views, shadow pass and framebuffer copies render *into the swapchain* instead of
into their targets, and with `SetViewports` also empty (13,008 calls) each one
takes the full screen. The single missing feature both corrupts the image and
starves the draws that sample the result.

Draws whose base texture cannot be supplied are currently dropped rather than
emitted, because emitting one paints a screen-covering surface over the correct
scene behind it. That is a bounded, reported limitation, not a fix: the census
prints the drop counts every run.

**Next rung: render targets** (`SetRenderTarget`, render-target-backed textures,
per-view viewport), which the residency census will confirm by driving
`unuploaded` to zero. The remaining census entries rank the work after that —
`InvalidateDelayedShaderConstants` (71,058), `SetDefaultState` (51,699),
`FlushBufferedPrimitives` (41,657), `ForceDepthFuncEquals` (35,529),
`SetLight`/`SetAmbientLightCube` (lighting, ~28,000).

## Render targets, the mesh layer, and draw-state fixtures (2026-09-22, later)

**Correction to the section above.** The frame "composited from several camera
viewpoints" was mostly not render targets. `ClearBuffers` only reset the draw
queue for `(-1, -1)` dimensions, and the engine passes real dimensions, so the
queue was never cleared and every capture replayed the whole run. Frames are now
bounded by `Present`: the first record after a present starts the next frame.

Fixed after measuring, in order:

| Defect | Effect |
| --- | --- |
| Render targets: `TEXTURE_CREATE_RENDERTARGET` ignored, `SetRenderTarget(Ex)`, `SetViewports`, `SetScissorRect`, `CopyRenderTargetToTexture(Ex)` empty | Draws, clears and copies now form one ordered stream replayed across render-pass-compatible passes (swapchain clear/load, render-target). `unuploaded` 20,320 → 0 |
| Every static mesh, static vertex buffer and index buffer was the *same object* sharing one 1 MB buffer; `GetDynamicMesh` ignored vertex/index overrides | Each static mesh owns its storage. The dynamic mesh draws override geometry, as D3D9's mesh manager does, which is how the world is drawn (`gl_rsurf.cpp`) |
| `ExecuteCommandBuffer` was an empty one-line inline body (never instrumented) | `LightmappedGeneric` binds its base texture through command buffers, so every world surface was dropped. Now parsed like `CShaderAPIDx8::ExecuteCommandBuffer`; `BindStandardTexture` defers to the material system as D3D9 does |
| Modulation/texture transform read from c37/c38 (the vs_1_1 layout) | vs_2_0 uses c47/c48-c49 (`ishadersystem_declarations.h`). The equivalence suite had the same hand-copied numbers and so could not detect it; both now name the enum. Registers are reset to identity at `BeginPass` |
| Samplers were clamp-to-edge only | Per-texture state from `TexWrap`/`TexMinFilter`/`TexMagFilter`; default wrap, as in D3D9 |

**State at the time (superseded below): the boot gate failed.** World and model draws all reach the
GPU with their textures, but `dev/engine_post` (drawn opaque with only the bloom
buffer) and `dev/motion_blur` (drawn additively with a copy of the frame) run
through the generic textured pipeline and erase the final image. These shaders
combine several samplers; the native pipeline computes `vcolor × modulation ×
tex0`. Also recorded: on a 1.5×-scaled Wayland display the engine screenshots its
video-mode size (1920×1080) from a 2880×1620 back buffer, so the TGA is a crop.

### Draw-state fixtures: comparing against the D3D9 path

Every defect above was found by hand from frame dumps. To compare directly with
the reference instead, both backends now write **draw-state fixtures**
(`materialsystem/drawstatefixture.h`, schema `source-draw-state/v1`). For each
material pass of the frame being screenshotted, they record the state actually
applied: sampled textures with addressing and filter, blend factors, depth, alpha
test, target, viewport, c47-c49, and whether the draw was submitted (with the drop
reason). D3D9 records at `CShaderAPIDx8::RenderPass` after state commit; native
records only the samplers and blend factors its pipelines really use.

```sh
python3 tools/quality/portal_boot.py --runtime run/runtime --build build-portal-vulkan \
    --renderer vulkan-compat --require-vulkan --draw-state-fixtures --out OUT_DX --map testchmb_a_01
python3 tools/quality/portal_boot.py --runtime run/runtime --build build \
    --renderer native-vulkan --draw-state-fixtures --out OUT_VK --map testchmb_a_01
python3 tools/quality/draw_state_diff.py --reference OUT_DX/draw-state/dx9-0.jsonl \
    --candidate OUT_VK/draw-state/vulkan-native-0.jsonl --out diff.json
```

The comparator groups draws by (material, pass), because two captures never line
up draw for draw, and excludes viewport, which follows the window. Its tests
(`tools/quality/tests/test_draw_state_diff.py`, 8) include seeded differences.
First comparison: 65 groups, 226 differing fields — samplers 65, modulation 65,
target 17, blend 14-16, depth write 14, base-texture transform 10, depth test 5,
alpha ref 5. Known interpretation limits: registers are compared raw, even for
shaders that do not read c47-c49; D3D9 quantizes the alpha reference to 1/255;
and D3D9 renders portals with stencil while native reports no stencil bits and
takes the render-target path, which accounts for the `target` differences.

## Burning down the first fixture diff (2026-09-22, later still)

The diff pointed at blend state first: decals drawn opaque with depth writes,
and models blended that D3D9 draws opaque. The two share one cause.

| Defect | Fix | Evidence |
| --- | --- | --- |
| `TakeSnapshot` appended a table entry on every call and packed the index into the 11 free bits of the 16-bit `StateSnapshot_t`. Past 2,048 calls the index wrapped, so draws bound another material's blend, alpha and shader | Identical shadow states share one snapshot, as in D3D9's transition table; a 2,049th *distinct* state is a hard `Error` rather than an alias | Blend-enable differences 14 → 0. New equivalence checks: identical states share an id, and a state taken after 3,000 snapshots keeps its own blend. Both fail with the dedupe disabled |
| Blend reduced to three fixed pipelines (opaque/alpha/additive): `src_alpha, one` ran as `one, one` and modulate (`zero, src_color`; `dst_color, src_color`) as alpha | Textured pipelines are built on first use per `DynRasterState`: the recorded factors (applied to color and alpha, as D3D9 without separate alpha), depth test, depth write and depth compare | New equivalence checks for modulate and `src_alpha, one` |
| `EnableDepthTest` and `DepthFunc` were unimplemented, and depth compare was `LESS` | Recorded in the shadow state; default `LEQUAL`, the D3D9 shadow default | depth_test / depth_write differences 5 / 14 → 0 |
| Alpha-test reference used unquantized | Truncated to 1/255 as `shadershadowdx8.cpp` does | alpha_test_ref differences 5 → 0 |
| Every shader not in the test catalog ran as "base texture × modulation", including screen-space post-processing that samples a bloom buffer or a frame copy | An explicit list of material shaders the textured pipeline reproduces (`NativePipelineImplementsShader`); draws by any other shader are declined by name (census, dropped-material report, fixture `submitted: false` with the reason) | `Engine_Post_dx9`, `MotionBlur_dx9`, `Downsample_nohdr`, `BlurFilterX/Y`, `WriteZ_DX9`, `ShadowBuild_DX9`, `Refract_DX90` and `Portal_DX90` are declined |

With the depth test now disabled for screen-space passes as D3D9 has it, the
post-process quads covered the whole frame. Declining them is what lets the frame
survive. Post-processing, refraction and portal surfaces are therefore declared
**unsupported** on the native path, not approximated.

**Current state: `portal_boot.py --renderer native-vulkan` passes** on
testchmb_a_01, with midtone fraction 0.71. The previous passing frame measured
0.12. The frame is a textured world: ceiling tiles, concrete walls and window
frames. Lightmaps are not applied, so it is unlit.

Second comparison: 64 groups, 163 differing fields. There are **no remaining
blend, depth or alpha-test differences.** What remains:
- samplers (64): lightmap, detail and gamma-lookup stages, and anisotropic
  filtering, are not implemented;
- modulation (64): D3D9's raw c47 is `[16,16,16,1]` for many shaders;
- target (19): portals use a render target instead of stencil;
- base-texture transform (10);
- submitted (6): exactly the declined shaders above.

Verification:
- Conformance suites: bring-up 20/0, render backend 33/0, material-facing 13/0,
  material equivalence 21/0.
- `tools/quality` tests: 163/0. The render-trace fixture had fallen behind the
  committed `Capture::Present(int32_t, bool)` signature and was updated.
- `stylelint --changed` reports 0 failures. The native and DX trees both build.


## Lightmap pixel conformance, D3D9 first (2026-09-22)

The draw-state fixtures compare state, not pixels. Lightmaps needed a pixel
oracle, and that oracle has to be established on D3D9 before any native
implementation is judged against it.

**Harness.** `material_pixel_conformance`
(`unittests/shaderextensiontest/material_pixel_conformance.cpp`) composes the
real material system, the standard shader library and one linked render
provider, the way the launcher does. It renders `LightmappedGeneric` on a
full-viewport quad for seven cases. Each case has a solid base texture and a
lightmap allocated, packed and bound through the map-load path
(`Begin/EndLightmapAllocation`, `UpdateLightmap` with linear float texels,
`BindLightmapPage`). The lightmap's two halves differ, so the result also proves
the lightmap is addressed by its own coordinates. A readback self-check
(clear to magenta, read back) is recorded with every capture.

Getting a trustworthy D3D9 capture exposed four harness requirements:
- **Full-screen texture:** D3D9 readback silently returns nothing without it.
  It comes from `MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE` set before `Init`,
  as `gl_shader.cpp` does.
- **Material precache:** an unprecached material has vertex format 0, which
  crashes the D3D9 dynamic mesh after the first present.
- **Drawable size:** on a scaled display the back buffer must be the window's
  drawable pixel size.
- **Pixel writer:** procedural texels must go through `CPixelWriter`, because
  D3D9 hands back BGRA storage.

**Oracle.** `tools/quality/material_pixel_conformance.py` (13 tests, including
seeded defects: zero readback, ignored lightmap, swapped red/blue, a
non-monotonic ramp, linear output, an unsupported HDR mode and reference drift):
- **Run validity:** the readback probe must return the clear color.
- **Backend-independent properties:**
  - a black lightmap gives black;
  - brightness rises strictly along the ramp;
  - channels stay independent;
  - a unit lightmap reproduces the base;
  - the base texture scales the result.
- **Closed form:** in integer HDR, `sRGB(linear(base) × lightmap)` must hold
  within 3 levels. It is validated on D3D9, where it holds exactly.
- **Reference agreement:** within 3 levels of the versioned D3D9 references in
  `quality/fixtures/material-pixels/` (provenance in their README).
- **HDR mode:** a backend reporting a different mode than requested fails with
  that reason.

**Native before:** every lightmap case rendered the bare base texture. That is
five defects, found by this test:

| Defect | Fix |
| --- | --- |
| `VertexShaderVertexFormat` unimplemented and `ComputeVertexFormat` returned 0, so no material had a vertex layout on native | Recorded per snapshot and merged as `CShaderAPIDx8::ComputeVertexUsage` merges passes |
| BGRA texels uploaded unconverted into R8G8B8A8 images (red/blue swapped) | Uploads convert to the image's actual channel order |
| `TexLock`/`TexUnlock` returned false, so lightmap pages were never written | A CPU copy of mip 0 for 8-bit formats; other formats and levels are refused and reported |
| Sampler 1, `EnableSRGBRead`/`EnableSRGBWrite` ignored; `GetLightMapScaleFactor` returned 1 | The lightmap page bound as `TEXTURE_LIGHTMAP*` on sampler 1 is sampled at texcoord 1 (second descriptor set). sRGB decode/encode follows each snapshot's shadow state. The scale matches D3D9 (`2^2.2` in LDR) |
| Mesh layout carried no texcoord 1 | 32-byte vertex, lightmap UV at offset 24; dynamic vertex record widened to 10 floats |

Bumped lightmaps (`TEXTURE_LIGHTMAP_BUMPED*`) are sampled at the flat
coordinate and counted in the census as unimplemented.

**Results:**

| Backend | HDR none | HDR integer |
| --- | --- | --- |
| D3D9 (`vulkan-compat`) | pass | pass (closed form exact) |
| native Vulkan | pass, within 1 level of D3D9 | **fail: backend reports HDR type 0** |

Integer HDR (16-bit lightmap pages, scale 16) is not implemented natively and is
declined explicitly, not approximated.

In the real game, `portal_boot.py --renderer native-vulkan` on testchmb_a_01 now
renders a lit chamber, with the room beyond the observation window and the
portal glow visible. Midtone fraction rose from 0.71 to 0.998.

Verification:
- Native Vulkan suites: bring-up 20/0, backend 33/0, facing 13/0, equivalence 21/0.
- `tools/quality` tests: 176/0.
- `stylelint --changed` reports 0 failures. The harness is registered under
  `legacyAbi.paths` because it composes through the preserved app-system factory
  ABI, like `appsystemgrouptest.cpp`.
- `archlint check --changed` still reports two entries (2 new, 2 stale). Both are
  the pre-existing `CShaderDeviceMgrVulkan::SetMode` signature reformat already
  present at `15cf3d78`, not this change.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build --renderer native-vulkan --hdr none \
    --reference quality/fixtures/material-pixels/lightmap-dx9-none.json --out OUT
```

**Orientation (same day).** The native frame was upside down: Vulkan's
clip-space Y points down, and the material transforms are D3D's. None of the
four native suites checked vertical orientation, and the lightmap cases only
varied left to right, so the pixel harness gained an `orientation` capture. It
uses a red-over-blue base texture read at 25% and 75% of the height, and the
oracle fails it as "frame is upside down". D3D9 passes, and its references were
recaptured with the new field. Native failed, and now passes: material draws use
a negative-height viewport (core in Vulkan 1.1), which puts clip-space +Y at the
top for the swapchain and render targets alike. Clears keep the unflipped
rectangle. The Portal boot frame now matches the D3D9 layout.

## Integer HDR on native Vulkan (2026-09-22)

Native previously hardcoded `HDR_TYPE_NONE`, and the lightmap pixel test declined
integer HDR explicitly. Native now implements it the way the D3D9 backend
defines it:

- **HDR type:** `GetHDRType()` is `HDR_TYPE_INTEGER` when `mat_hdr_level >= 2` and
  the engine has enabled HDR for the map (`SetHDREnabled`, previously ignored).
  Native owns `mat_hdr_level` with D3D9's name, default and flags. Its device
  manager now connects tier1 and registers its cvars through an accessor, as
  `CShaderDeviceMgrBase::Connect` does.
- **Lightmap pages:** `RGBA16161616` maps to `R16G16B16A16_UNORM`, with direct
  upload and 8-byte `TexLock`. `GetLightMapScaleFactor()` returns 16.
- **Tone mapping:** `SetToneMappingScaleLinear` is recorded, forced to 1 without
  HDR as D3D9 does. Lightmapped draws apply it as `FinalOutput`'s
  `LINEAR_LIGHT_SCALE` before the sRGB encode.

The pixel harness gained a `tone_scale` capture: `ramp_mid` drawn at scale 2.
D3D9 confirmed both predictions: the closed form with the lightmap doubled in
integer HDR, and the scale ignored in LDR. The references were recaptured; the
diff is additive only. Seeded tests cover an ignored scale and a scale applied
in LDR (180 `tools/quality` tests pass).

| Backend | HDR none | HDR integer |
| --- | --- | --- |
| D3D9 | pass | pass |
| native Vulkan | pass | **pass**, at most 1 level from D3D9, closed form holds |

In the game, native now runs Portal in integer HDR: testchmb_a_01's
`LightmappedGeneric` draws carry c47 = `[16,16,16,1]` exactly as on D3D9, and the
boot passes (midtone 0.993).

Known gaps, recorded rather than approximated:
- **Other shaders:** only lightmapped draws apply the tone scale. They choose
  their tone-map type per combo, and the census counts
  "integer HDR: tone-mapping scale on unlit/model shaders".
- **Exposure:** it does not adapt, because D3D9's luminance measurement runs in
  post-process passes that native declines. The native frame is darker than
  D3D9's. *(Resolved the same day; see "Auto-exposure luminance
  histogram" below.)*

## Auto-exposure luminance histogram (2026-09-22)

Integer-HDR auto-exposure is client code (`CLuminanceHistogramSystem`,
`game/client/viewpostprocess.cpp`) shared by both backends. The backend's part is
the measurement it rests on:
- the back buffer is copied to `_rt_FullFrameFB`;
- `dev/lumcompare` (`screenspace_general` with `luminance_compare_ps20`) is drawn
  over the centre 90% x 85% of the viewport, once per luminance range;
- each draw is bracketed by an occlusion query, which counts the pixels in that
  range.

Native's occlusion queries were stubs, and it declined `screenspace_general`. Every
query stayed pending, the histogram was empty, and exposure stayed at its reset
value. That is the "darker than D3D9" gap recorded above.

**Fixture.** The pixel harness has an `exposure` family (`-family exposure`).
- It clears eight regions of known sRGB colour: black, three greys, white, and
  pure red, green and blue. Each colour's linear luminance lies inside one range,
  clear of the range edges.
- It then runs the client's measurement step for step: the same 17 ranges
  (algorithm 1), the same measured rectangle and the same
  `DrawScreenSpaceRectangle` call.
- It reads the regions back *after* the luminance draws, since `dev/lumcompare`
  must not write colour.

The oracle holds every count to a geometric model: each region's area inside the
rectangle, placed in its luminance range. The model and the D3D9 comparison are
both exact. D3D9 matches the model exactly in both HDR modes. It runs with 4x
MSAA, so its queries count 4 samples per pixel; the oracle measures that unit
with the all-pixels range, as the client does. References:
`quality/fixtures/material-pixels/exposure-dx9-{none,integer}.json`.

**Native implementation.**
- **Occlusion queries.** `CVulkanContext` has a precise occlusion query pool
  (`occlusionQueryPrecise` is enabled when supported). Query begin and end are
  stream records, so a query counts exactly the draws between them, in engine
  order.
  - Each slot is reset outside the render pass before the replay that uses it.
  - A result is readable once its frame is submitted.
  - A query that would span a render-pass boundary fails. So does one whose frame
    is discarded unsubmitted, and one read with flush before its frame is
    submitted; that last case is counted in the unimplemented-features report.
    Failures report `OCCLUSION_QUERY_RESULT_ERROR`, never a wrong count.
- **dev/lumcompare.** The snapshot records the vertex shader and pixel shader
  names, the alpha-test comparison and `EnableColorWrites`.
  - `luminance_compare_ps20[b]` becomes a textured-pipeline variant: sRGB-read
    base × `c0.z`, NTSC luminance, `step(c0.x) * step(c0.y)`.
  - `screenspaceeffect_vs20` passes clip-space positions and texture coordinates
    through untransformed.
  - The GREATER alpha test and the colour write mask are honoured.
  - `screenspace_general` is accepted only for that pixel shader; its other
    post-process shaders are still declined.
- **D3D9 half-pixel convention.** The first native run counted every pixel
  (113488 of 113488). But pixels at region boundaries fell into ranges no region
  occupies. D3D9 puts pixel centres at integer coordinates, and
  `DrawScreenSpaceRectangle` offsets its quad by −0.5 to suit. So under Vulkan's
  +0.5 centres every sample landed on a texel edge and was blended. Native draw
  viewports now shift half a pixel right and down, as DXVK does. This applies to
  every native draw, not only post-process.

Negative controls on the real backend all fail the fixture:
- colour writes left on: "region … reads back [255,255,255] after the luminance
  draws";
- the GREATER alpha test treated as GEQUAL: every range counts the whole
  rectangle;
- the half-pixel offset removed: blended boundary pixels.

Seeded oracle tests cover those three and five more: no results, an ignored sRGB
read, fractional sample counts, a mismatched frame size and a family mismatch.

**Headless on the GPU.** The runner now defaults to `--display headless`.
- It sets `SDL_VIDEODRIVER=offscreen` and removes `WAYLAND_DISPLAY` and `DISPLAY`.
- The system SDL3's offscreen driver creates Vulkan surfaces through
  `VK_EXT_headless_surface`. So DXVK and native both render and present a real
  swapchain on the real GPU (RADV) with no window or compositor.
- The drawable is exactly 256×256 whatever the desktop's scale, which makes the
  references reproducible.
- `--display desktop` keeps the windowed run.

All four references were recaptured headless. The lightmap pixels are identical
to the earlier desktop captures. `portal_boot.py` also runs headless with the
same variables.

| Backend | lightmap none | lightmap integer | exposure none | exposure integer |
| --- | --- | --- | --- | --- |
| D3D9 | pass | pass | pass | pass |
| native Vulkan | pass | pass | **pass** (exact) | **pass** (exact) |

**In the game** (headless boot, testchmb_a_01). `portal_boot.py` now prints
`mat_hdr_tonemapscale`, the exposure goal the client writes every frame, and
records it as `tonemap_scale` in the evidence.

| Boot | `tonemap_scale` |
| --- | --- |
| D3D9 | 1.3 |
| native Vulkan | 1.3 |
| native Vulkan, occlusion queries disabled (negative control) | 1.0 |

Verification:
- native suites: bring-up 20/0, backend 33/0, facing 13/0, equivalence 21/0;
- `tools/quality`: 191 tests OK;
- stylelint `--changed`: 0 failures;
- archlint: only the two pre-existing `SetMode` entries.

Remaining exposure gap: bloom and the rest of the integer-HDR post chain are
still declined. Only lightmapped draws apply the tone scale (see above).

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build --renderer native-vulkan --hdr integer --family exposure \
    --reference quality/fixtures/material-pixels/exposure-dx9-integer.json --out OUT
SDL_VIDEODRIVER=offscreen WAYLAND_DISPLAY= DISPLAY= python3 tools/quality/portal_boot.py \
    --runtime run/runtime --build build --renderer native-vulkan --map testchmb_a_01 --out OUT
```

## Model placement: bone matrices, hardware skinning and culling (2026-09-22)

On native Vulkan, multi-bone props (dynamic props such as the round exit door,
elevators and security cameras) all drew at one spot. Studiorender places them
with bone matrices:
- **Rigid models:** `LoadBoneMatrix(0)`, which D3D9 also loads as the MODEL matrix.
- **Skinned strips:** `SetNumBoneWeights` plus `LoadBoneMatrix(n)`, with the
  vertex shader's `SkinPosition` blending three bones per vertex (two stored
  weights and the implicit third).

Native's `LoadBoneMatrix`, `SetNumBoneWeights` and `GetCurrentNumBones` were
stubs, and its mesh layout dropped bone weights and indices. So every such model
drew with whatever MODEL matrix was current.

**Fix.**
- Native stores `cModel[53]`, and bone 0 loads MODEL as `CShaderAPIDx8` does.
- The mesh layout carries two float weights and four index bytes: stride 44,
  offsets 32 and 40.
- `EmitToNativeQueue` skins skinned draws on the CPU with D3D9's formula into
  world space, and draws them with view × projection.

**Culling.** Native never culled, so the back faces of single-sided props (the
elevator shaft walls) covered the scene. D3D9's effective cull mode is the
shadow state's `EnableCulling` (on by default, off for `$nocull`) combined with
the dynamic `CullMode`: CCW normally, CW for mirrored views. Native now applies
the same rule. The flipped draw viewport preserves D3D screen winding, so the
front face is clockwise. The four native suites drew counter-clockwise quads,
which D3D9 would also cull; they were rewound clockwise.

**Fixture: `skinning` family** (`skinning-dx9-{none,integer}.json`). Five cases
place a `$model` UnlitGeneric quad in one third of the frame each:
- rigid through bone 0;
- one bone;
- the implicit third weight on index byte 2;
- a 50/50 blend;
- bone index 3.

Bone 0 always holds a decoy position, so ignoring skinning is visible. A sixth,
back-facing case must be culled. D3D9 matches the oracle's independent table
exactly, and native passes in both HDR modes. Negative controls on the real
backend:
- the stub `LoadBoneMatrix` puts every model in the middle, reproducing "all in
  one spot";
- culling disabled draws the back-facing quad.

Five seeded oracle tests cover these.

**In the game.** `portal_boot.py` gained `--console-command` (exec'd from a cfg;
player commands need the `cmd` prefix), `--headless` (offscreen and volume 0)
and `-multirun`, so framed views can be compared on both backends. On
testchmb_a_01 the exit door, exit sign, portal frames and the exit elevator with
its shaft now sit where D3D9 draws them. A temporary diagnostic confirmed that
static props already had correct transforms (for example the shaft wall at
(−256, −192.1, 96), rotated 90°, matching the map's static-prop lump). Their
wrong look came from culling.

Still different from D3D9, and not placement:
- model lighting (native props are unlit, so brighter);
- portal openings (being worked on separately);
- native's swapchain follows the window size, not `-w`/`-h`;
- testchmb_a_00 renders white on native, with or without this change.

## Portal stencil recursion: PortalRefract matches D3D9 (2026-09-22)

`--family portal` renders Portal's openings in the harness. It replicates the
client's `CPortalRender::DrawPortalsUsingStencils` with the real materials:
- `portal_stencil_hole`,
- `portalstaticoverlay_1` (PortalRefract stage 2),
- `portal_refract_1` (stage 0),
- `engine/writez_model`.

It runs through the real material system: stencil INCR/DECR recursion to
`r_portal_stencil_depth` 2, `ClearBuffersObeyStencil`, D3D9 user clip planes on
the exit portal, and `_rt_PowerOfTwoFB` refraction. There are three cases:
`recursion` (open), `opening` (half open) and `static` (all static).

The oracle (`tools/quality/material_pixel_portal.py`) has two parts:
- An independent ray-traced model judges the structure. It checks every decidable
  pixel of each nested view's wall, and the first nested view's blocker clipped
  by the exit portal's plane.
- The versioned D3D9 reference (`quality/fixtures/material-pixels/portal-dx9-none.json`)
  judges PortalRefract's pixels: 1 level per channel, at most 8 pixels beyond it.

**Result:** native Vulkan matches D3D9 bit for bit in `opening` and `static`. In
`recursion`, one channel of one flame pixel is one level off.

Differences found and fixed on the way:

| Symptom | Cause | Fix |
| --- | --- | --- |
| D3D9 reference showed the clipped blocker unclipped | DXVK 2.7.1's fast-linked graphics-pipeline-library pipelines ignore user clip planes | The runner sets `DXVK_CONFIG="dxvk.enableGraphicsPipelineLibrary = False"` for this family, recorded in evidence |
| D3D9 ran 4x MSAA; native one sample | `SetMode` applies the dxsupport level's defaults | The harness pins one sample per pixel |
| 3951 → 539 pixels off at the flame rims | Native blended sRGB-writing draws in sRGB space | sRGB views of mutable-format targets (`VK_KHR_swapchain_mutable_format`), as D3D9 `SRGBWRITEENABLE` blends in linear |
| ~1200 flame/static pixels off by up to 59 | `mat_trilinear` 1 on D3D9 (dxsupport.cfg), 0 on native, which reads no dxsupport.cfg | The harness pins trilinear and no forced anisotropy; the oracle rejects captures that don't report it |
| 67 static pixels off by up to 15, native brighter | Native reported no compressed-texture support, so the material system decoded DXT1 on the CPU. Its rounding reads the noise about half a unit low, and the flame ramp magnifies that | `SupportsCompressedTextures` reports `textureCompressionBC` (now enabled); DXT1/DXT1_ONEBITALPHA/DXT3/DXT5 upload as BC1/BC1/BC2/BC3 |
| 893 refraction pixels one level dark | Only blended sRGB draws used the sRGB view; opaque stage 0 encoded in the shader | Every sRGB-writing draw uses the sRGB view (hardware encode, as DXVK). A query begins in the pass of the draw it counts, so occlusion queries are not split |

These were ruled out by experiment (the frames were unchanged or worse):
- FXC's `rcp(rsq)` square root;
- the `.xx` ramp coordinate;
- decoding the ramp after filtering;
- DXVK's `0.5 - 1/128` viewport bias (it made an edge 180 levels off);
- fog, which is 0 with fog off: `c6` is set so that CalcRangeFog returns 0.

Real negative controls against the fixture:
- the build before the DXT fix fails (688 static pixels);
- the unpinned-sampling build fails.

Ten seeded oracle tests cover:
- no stencil;
- shallow recursion;
- unpinned sampling;
- ignored recursion;
- an ignored clip plane;
- drift;
- the tolerance boundary;
- a changed scene;
- malformed captures.

Also passing after these changes:
- all 7 material-pixel captures, native against D3D9 (lightmap, exposure,
  skinning, portal);
- bring-up 20/0, backend 33/0, facing 13/0, equivalence 21/0;
- 206 `tools/quality` tests;
- `portal_boot.py` native on testchmb_a_01, where BC-compressed world textures
  render correctly.

`archlint check --all` reports the existing `SetMode` baseline drift (the text was
reformatted before this change) and other sessions' physics entries.

Open:
- Native reads no `dxsupport.cfg`: `GetRecommendedConfigurationInfo` is a stub,
  and it reports dx level 90 and no anisotropy (D3D9: 95 and 16). The game
  therefore runs with different convar defaults from D3D9.
- ATI1N/ATI2N remain unsupported, as before.
- PortalRefract range fog is not ported; the harness runs with fog off.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build --renderer native-vulkan --hdr none --family portal \
    --reference quality/fixtures/material-pixels/portal-dx9-none.json --out OUT
```

## VGUI on native Vulkan: menus, console, HUD and fades (2026-09-22)

On native Vulkan, VGUI (the 2D layer used for menus, the console, the HUD, text
and screen fades) drew as flat white rectangles. A fade could also cover the
whole view in opaque white. The draws did reach the GPU; four separate
contract gaps produced the wrong result:

| Symptom | Cause | Fix |
| --- | --- | --- |
| Panels and text opaque and white | UnlitGeneric in DX9 is `vertexlit_and_unlit_generic`. Its `VERTEXCOLOR` combo multiplies by the vertex color, and `c12.w` (`g_fVertexAlpha`) weights the vertex alpha. The native pipeline applied neither, and the dynamic vertex record had no alpha | The record gains the color's alpha (`kDynVertexFloats` 18, textured-pipeline location 6). Snapshot flags 256/512/1024 come from the pixel-shader static index (`VERTEXCOLOR` stride 384, off when `DIFFUSELIGHTING`, stride 24, is on). The vertex stage gamma-converts the color unless the material doesn't write sRGB (`DONT_GAMMA_CONVERT_VERTEX_COLOR`). Vertex alpha is gated per draw on `c12.w` |
| Screen fades (`ViewDrawFade`) and `$color`/`$alpha` ignored, so fades drew opaque white | This family's modulation is pixel `c1` (`g_DiffuseModulation`, linear), not `cModulationColor` | `CommitPassPixelConstants` takes the modulation from `c1` for `vertexlit_and_unlit_generic_*` passes |
| Menu text sheared into diagonal streaks | `TexSubImage2D` dropped offset updates and uploaded a smaller (0,0) region as the whole level. The font cache writes each glyph that way | `CVulkanContext::UploadManagedTextureRegion` keeps the texels outside the region and zero-fills a never-filled image. `UploadTextureSurface` passes offsets through and repacks padded DXT block rows. Compressed regions must be 4x4 aligned |
| Red and blue swapped in every vertex color (console warnings (255,90,90) measured (90,90,252)) | `CVertexBuilder::Color4ub` stores a D3DCOLOR (B,G,R,A) because `OPENGL_SWAP_COLORS` is not defined; native read it as R,G,B,A | Vertex colors are read as D3DCOLOR. The facing test's fixture bytes, which encoded the RGBA assumption, are corrected |

Oracle: `material_equivalence_vulkan_conformance` grows from 21 to 29 checks.
The new checks are held to the `vertexlit_and_unlit_generic_ps2x.fxc` formula
and the D3DCOLOR layout:
- a sub-rectangle at x=2 from a pitch-16 source, with the texels outside it
  preserved;
- the vertex color applied only with the `VERTEXCOLOR` combo (negative control);
- `c1`, not `c37`, modulates;
- vertex alpha only when `c12.w` = 1 (control);
- `c1` alpha 0 is transparent.

A mutation that reintroduced the RGBA read failed exactly the D3DCOLOR check.

Evidence on this machine (headless and native Wayland, AMD RADV):
- The main menu renders its title, items and "Loading..." text.
- `-console` renders the translucent console: colored text (warnings measured
  (252,90,90)), scrollbar, buttons, text entry, and the blurred background.
- testchmb_a_04 draws the HUD crosshair (`sprites/hud/portal_crosshairs`) and
  `__fontpage_additive` text.
- testchmb_a_00, captured during the intro fade, now passes. Earlier the fade
  covered the frame in white and `portal_boot` failed "lacks scene detail".
- No 2D draw was dropped: the census shows no line/point drops, and only
  `engine/preloadtexture` is refused.
- Still passing: bring-up 20/0, facing 13/0, equivalence 29/0, and all 7
  material-pixel families (lightmap, exposure and skinning in none and integer
  HDR, plus portal).

Open:
- There is no D3D9 pixel capture of VGUI yet. The claim rests on the shader
  formula and the frames above.
- `MATERIAL_LINES` is still dropped, so VGUI `DrawLine`/`DrawPolyLine` would not
  draw. None occurred in these frames.
- `DIFFUSELIGHTING` passes (vertex lighting) do not use flag 256; model lighting
  is separate work.
- The `pow(2.2)` vertex-color conversion runs per vertex, as D3D9 does, and is
  then interpolated.

```sh
# console at the main menu, headless
SDL_VIDEODRIVER=offscreen ./hl2_launcher -renderer native-vulkan -game portal -windowed \
    -novid -insecure -multirun -console +wait 400 +screenshot +wait 10 +quit
python3 tools/quality/portal_boot.py --runtime RUNTIME --build build --renderer native-vulkan \
    --headless --map testchmb_a_04 --out OUT
```

## Model lighting: VertexLitGeneric vertex lighting matches D3D9 (2026-09-22)

The `--family modellight` harness (`material_pixel_modellight.cpp`) lights
VertexLitGeneric models the way studiorender does:
- `SetAmbientLightCube`;
- up to four `SetLight` lights: directional, point and spot;
- a static-prop color mesh (`IMesh::SetColorMesh`, VERTEX_SPECULAR).

The material system picks the `vertexlit_and_unlit_generic_vs20` combos
(DYNAMIC_LIGHT, STATIC_LIGHT, HALFLAMBERT) from that state, as in the game.
Eleven cases draw a 3x3 grid of quads, each quad with its own normal:
- ambient cube;
- directional;
- point;
- spot;
- four mixed lights;
- half-Lambert;
- static color only;
- static plus dynamic;
- no light;
- a MODEL-matrix placement (a half turn with z scaled, so normalization matters);
- a skinned placement (three bones).

The oracle (`material_pixel_modellight.py`) judges the frames two ways:
- An independent evaluation of `common_vs_fxc.h` DoLighting, with the light
  constants exactly as `CShaderAPIDx8::SetLight`/`CommitVertexShaderLighting`
  build them (sorted spot, point, directional). D3D9 matches it within one
  level on every judged pixel, in both HDR modes.
- The versioned D3D9 references `modellight-dx9-{none,integer}.json`: 1 level,
  no pixel beyond it.

Native before the change drew every model unlit (base × modulation): every
lit pixel was off, by up to 190 levels.

Native now mirrors D3D9's state and math:
- **Lighting state:**
  - SetLight, DisableAllLocalLights, SetAmbientLightCube and GetLight hold the
    state as `m_DynamicState` does.
  - `GetDX9LightState` reports the ambient light (cube nonzero), the light count
    and static vertex light (the mesh has a color mesh).
  - `GetMaxLights` reports 4.
- **Combos:** the vertex shader's static combo index is kept per snapshot
  (VERTEXCOLOR, HALFLAMBERT), and the dynamic index arrives through
  `SetVertexShaderIndex` / `CBCMD_SET_VSHINDEX`.
- **Vertex lighting:** `EmitToNativeQueue` evaluates DoLighting per unique
  vertex in world space (rigid through MODEL, skinned through the bones, normals
  renormalized) into the float vertex color. Every pass whose pixel shader has
  DIFFUSELIGHTING multiplies by it (flags 256|512, never gamma converted).
- **Color meshes:** a VERTEX_SPECULAR static mesh locks as packed 4-byte
  D3DCOLORs, which the engine memcpy's baked static-prop colors into.
- **Tone scale:** every vertexlit_and_unlit_generic pass (UnlitGeneric too) is
  scaled by the linear tone-mapping scale, as its FinalOutput(TONEMAP_SCALE_LINEAR)
  does. That was missing for integer HDR: 16–31 levels off.

**Result:** LDR is bit-exact except two pixels one level off. Integer HDR is
bit-exact except one flat quad in each of the two placement cases, one level
off (a rounding boundary under the 0.75 scale).

Twelve seeded oracle tests cover:
- unlit models;
- swapped cube faces;
- a dropped light;
- the spot exponent;
- half-Lambert;
- the static color mesh;
- placement normals;
- an ignored tone scale;
- too few lights;
- tolerance and drift;
- changed inputs;
- missing cases.

All eight pixel families pass on native, and so do 56 oracle tests.
`portal_boot.py` passes on testchmb_a_01 on both backends.

**Phong models (per-pixel).** 75 of Portal's 233 VertexLitGeneric model
materials use `$phong` or `$bumpmap`, the exit door among them; D3D9 draws
them with `skin_vs20`/`skin_ps20b`. Native now has a skin pipeline:
- `shaders/skin.{vert,frag}` port the non-flashlight path line for line:
  - the tangent frame and normal map;
  - PixelShaderDoLighting, always half-Lambert, with the lightwarp ramp;
  - phong specular with the Fresnel ranges, the exponent map, tint and boost;
  - `$basemapalphaphongmask`, self-illumination and rim light;
  - FinalOutput's linear scale.
- The static combos are decoded from `skin_ps20b.inc`.
- The pixel constants c0–c31 reach the shader in a per-frame dynamic uniform
  buffer, which is written only after that frame's fence has signaled.
- The CPU does skin_vs20's per-vertex work, as for vertex lighting: world
  position, world normal and tangent, and each light's attenuation.
- Native gained what the helper's dynamic state needs:
  - `CommitPixelShaderLighting` (D3D9's packing, directional lights 10000 units
    from `SetLightingOrigin`'s origin);
  - `SetPixelShaderStateAmbientLightCube` and `GetWorldSpaceCameraPosition`;
  - the eye-position command, and samplers 1–15.
- A device without seven descriptor sets gets no skin pipeline, and those
  draws are declined by name.

Seven more modellight cases:
- phong;
- four lights;
- lightwarp;
- self-illumination;
- a constant exponent and tint;
- base-alpha phong mask;
- skinned phong.

The oracle evaluates skin_ps20b per pixel. Things measured on the way:
- After InitShaderParams, `CShaderSystem::InitShaderParameters` zeroes every
  numeric parameter a material leaves out. An unset `$phongboost` or
  `$phongfresnelranges` is therefore 0 (no specular), not the helper's fallback.
- The shipped `skin_ps20b.vcs` lacks the RIMLIGHT and FASTPATH_NOBUMP combos.
  D3D9 draws those materials with no pixel shader (white). Portal's content
  uses neither, so the harness does not either.

D3D9 matches the model within one level, and within two in specular peaks
(single-precision `pow` at exponents up to 150). Native matches D3D9 within
two levels in the same peaks and one level elsewhere, in both HDR modes. The
oracle's phong tolerance is those two levels, with no pixel allowed beyond it.
Five more seeded tests cover:
- no specular;
- a flat normal;
- a missing lightwarp;
- a moved lighting origin;
- no self-illumination.

All nine captures and 232 quality tests pass. The suites pass: bring-up 25/0,
backend 33/0, facing 13/0, equivalence 29/0. In game, the exit door on
testchmb_a_01 now shows its phong shading.

Open:
- `$envmap` on skin materials: 6 in Portal. The CUBEMAP combo is drawn without
  the environment map, and this is reported.
- The flashlight pass; detail and wrinkle maps.
- A color mesh with bumped static lighting (three colors in the normal stream).
- LIGHTING_PREVIEW.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build --renderer native-vulkan --hdr integer --family modellight \
    --reference quality/fixtures/material-pixels/modellight-dx9-integer.json --out OUT
```

### The back buffer is the video mode's size, presented scaled (2026-09-22)

On a 1.5x Wayland display, VGUI covered only the top-left 2/3 of the window,
while the world filled it. D3D9 sizes the back buffer from the video mode
(`BackBufferWidth = m_DisplayMode.m_nWidth`), reports that size, and Present
stretches it over the client area. DXVK scales it the same way. Native
rendered straight into the swapchain at the drawable's physical size
(2880x1620) and reported that as the back buffer, while the engine's mode stayed
1920x1080. Two things kept the mode there:
- `RequestWindowResize` needs queued-threaded mode, and the launcher runs
  `mat_queue_mode 0`;
- native `ChangeVideoMode` was an empty stub.

So some engine code worked in 2880 space and VGUI (via the viewport) in 1920 space.

Now:
- The engine renders into offscreen back buffers, one per swapchain image, at
  the mode's size: `SetMode`/`ChangeVideoMode` call
  `CVulkanContext::SetBackBufferSize`, and 0 x 0 follows the drawable.
- `EndFrame` blits the back buffer into the acquired swapchain image, with
  linear filtering when scaling. The acquire semaphore is waited on at the
  transfer stage.
- `ReadPixels` and captures still read the back buffer, as D3D9 does.
- `RequestPresentedCapture` reads the swapchain image, i.e. what the window
  shows.

Evidence:
- `native_vulkan_bringup_conformance` grows from 20 to 25 checks. A half-size
  back buffer is presented over the whole drawable: the triangle sits at the
  window's centre and the far corner is the new clear. A 1:1-copy mutation
  fails both checks, and 0 x 0 follows the drawable again.
- It passes headless and on 1.5x Wayland.
- The game on Wayland logs `back buffer 1920x1080, window 2880x1620`. A
  presented 2880x1620 frame of the in-game pause menu shows the scene and the
  menu's dim overlay covering the whole window.
- Still passing: facing 13/0, equivalence 29/0, `portal_boot` native, and the
  7 material-pixel families I re-ran (lightmap, exposure and skinning in none
  and integer HDR, plus portal).

Open:
- With `mat_queue_mode 0` the mode never follows the drawable, so the frame is
  upscaled 1.5x, as it is under DXVK. A crisp 1:1 frame needs the engine's
  resize request to work without the queued render thread (engine-side;
  `CMaterialSystem::RequestWindowResize`).
- Aspect ratio is stretched to the client area as D3D9 does, with no
  letterboxing.

## Default alpha-test reference: chamber signs (2026-09-22)

In testchmb_a_01, the chamber signs (`models/props_animsigns/signage_numNN`) drew
their "off" state (skin 1) with flat, lit white rectangles where the digits and
icons go. In that skin every glyph mesh uses `awe_blank`: VertexLitGeneric with
`$alphatest` and no `$alphatestreference`, and a white texture whose alpha is
about 0. The material therefore keeps the default shadow state's reference.
D3D9 sets that default to GEQUAL 0.7 (`CShaderShadowDX8::SetDefaultState`), so
every texel is discarded. Native set it to 0, so every texel passed.

Native's `SetDefaultState` (and the member's initial value) now uses 0.7. The
comparison and D3D9's truncation to 1/255 were already shared.

Evidence:
- `material_equivalence_vulkan_conformance` grows from 29 to 31 checks. Starting
  from the default state, a 3-texel texture keeps alpha 255 and 191, and
  discards 128. With the old default, the discard check fails (seeded, 1 failure).
- Draw-state fixtures of sign_03 forced to skin 1 (`ent_fire sign_03 skin 1 4`,
  after the trigger's on-animation) record `awe_blank` at alpha reference
  0.698039 on both backends. The native frame is the blank dark panel, as on DXVK.

Once the sign has animated on (skins 3–6), native drew the panel gray-blue
where D3D9 draws it white. `newsignage_back01/02` are VertexLitGeneric
`$selfillum`, and that is now implemented (next section).

```sh
SDL_VIDEODRIVER=offscreen WAYLAND_DISPLAY= DISPLAY= python3 tools/quality/portal_boot.py \
    --runtime run/runtime --build build --renderer native-vulkan --headless \
    --map testchmb_a_01 --draw-state-fixtures --console-command noclip \
    --console-command "cmd setpos -590 64 640" --console-command "cmd setang 0 180 0" \
    --console-command "ent_fire sign_03 skin 1 4" --out quality-results/sign
```

## VertexLitGeneric self-illumination without $phong (2026-09-22)

Portal ships 177 VertexLitGeneric `$selfillum` materials that do not take the
skin path, including the lit chamber-sign panels. 27 of them set
`$selfillumtint`, and none set `$selfillummask`. D3D9 draws them with
`vertexlit_and_unlit_generic_ps2x`'s SELFILLUM static combo (stride 192):

    diffuse = lerp( albedo * diffuseLighting, g_SelfIllumTint(c4) * albedo, base.a )
    alpha   = g_DiffuseModulation(c1).a      (base alpha is the mask, not opacity)

Native drew only the lit term. The textured push block is full at the
guaranteed 128 bytes, so the tint rides in existing inputs. Because albedo =
base * c1, the blend factors as `base * lerp( c1 * L, c1 * tint, base.a )`:
- `EmitToNativeQueue` writes `c1 * L` as the vertex color, or `c1` when unlit.
  It is linear, since vertex lighting is evaluated on the CPU.
- `CommitPassPixelConstants` pushes `( c1 * c4, c1.a )` as the modulation.
- `demo_dyn_tex.frag`, flag `kFragmentSelfIllum` (2048), blends by the base
  alpha.

The cases this does not express are reported in the census:
- `$selfillummask` (c3.w, sampler 11);
- SELFILLUM with VERTEXCOLOR;
- SELFILLUM_ENVMAPMASK_ALPHA.

Evidence:
- A new modellight case, `selfillum`: tint `[0.9 0.6 0.3]`, mask 160/255,
  two lights and an ambient cube. The independent oracle blends the evaluated
  vertex lighting toward the tint by the base alpha.
  - D3D9 satisfies that oracle in both HDR modes.
  - The recaptured D3D9 fixtures are byte-identical to the old ones in every
    previous case.
  - Native matches the oracle and D3D9 within 1 level, with 0 pixels over, in
    both HDR modes.
  - With the flag disabled, native fails with 40401 pixels off the model
    (worst 122).
- Three seeded oracle tests fail as intended: `$selfillum` removed, tint
  `[1 1 1]`, mask 255. All 279 quality tests pass.
- Suites: equivalence 31/0, facing 13/0, bring-up 25/0, backend 33/0.
- In game (sign_03, lit): the native panel went from (83, 94, 103) to
  (199, 204, 209). D3D9 draws (223, 233, 240). The remaining difference is the
  lighting input, not self-illumination:
  - The pre-change native value implies vertex lighting of about 0.08, and
    the D3D9 formula with that lighting predicts about 205.
  - The concrete wall beside the sign differs in the same direction, native
    (46, 54, 57) vs D3D9 (71, 90, 96). That is the same before and after this
    change.

Open: native in-game lighting is darker than D3D9 on testchmb_a_01, for world
lightmaps and model vertex lighting alike. The modellight harness, fed the same
inputs, matches, so the in-game inputs differ.

The D3D9 tree needed a reconfigure with its recorded argv. Its configuration
predated a jobsystem wscript change, and `deterministic_executor.cpp` failed on
`std::decay_t`.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime \
    --build build --renderer native-vulkan --hdr none --family modellight \
    --reference quality/fixtures/material-pixels/modellight-dx9-none.json --out OUT
```

## Cable_DX9 normal-map lighting (2026-09-23)

The cable shader binds its normal map at sampler 0 and its authored cable color
at sampler 1. Native treated sampler 0 as the base color and ignored the normal
map lighting, producing the bright cyan/magenta strands shown in the reported
frame. The native textured shader now evaluates `cable_ps2x.fxc`'s squared
half-Lambert term from the tangent-space normal and multiplies the sRGB-decoded
sampler-1 color and vertex lighting. The family is explicitly recognized as
`Cable_DX9`, and its second descriptor uses the material's sampler-1 binding.

Evidence:
- A new `cable` material pixel family renders front, side, back and diagonal
  normals. Its independent equation check and the DXVK 2.7.1 reference both
  pass on native Vulkan; the negative test that ignores the normal map fails as
  intended. The reference capture is documented in
  `quality/fixtures/material-pixels/cable-dxvk-none.json`, documented in the
  fixture README.
- `material_pixel_conformance` built for both native Vulkan and DXVK. The three
  cable-specific quality tests pass.

The later sections record scene captures for the sky, changed HUD, slideshow,
and sphere sprites. At this capture, glass remained open: the frosted
observation-window panels
use `LightmappedGeneric` with `$envmap env_cubemap` and additive blending.
Native submits their base texture but does not sample their cubemap reflection,
so the pane stays clearer than DXVK. `Refract_DX90`, used by other glass and
liquid-portal surfaces, was outside the native shader list because its
screen-copy normal distortion needed its own implementation. The glass section
below records the subsequent implementation. Real monitor render targets still
need a scene capture.

```sh
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime-native \
    --build build-r03-portal-native --renderer native-vulkan --hdr none --family cable \
    --reference quality/fixtures/material-pixels/cable-dxvk-none.json --out OUT
```

## Sky_DX9 textured sky (2026-09-23)

Sky_DX9 was dropped by the native material allowlist, leaving the sky black.
The material uses a normal textured draw with two shader-specific details:
sky_ps2x.fxc reads its color multiplier from pixel constant c0, and
sky_vs20.fxc puts the base UV matrix in c49/c50 because c48 carries texture
size and pixel-coordinate scale. Native now accepts Sky_DX9, routes c0 into the
textured modulation, selects the correct UV matrix, and applies the linear
tone-map scale.

Evidence: the new sky material-pixel family checks sRGB texture times $color on
two materials. Each material uses a vertically split procedural texture and a
translated UV matrix; samples at the left and right must both read the intended
top half. Fresh DXVK pixels match the independent equation, and native matches
the DXVK fixture at all four samples. quality/fixtures/material-pixels/README.md
records capture provenance and the reproduction command.
An `escape_02` outdoor capture from `cmd setpos 0 0 1000` and
`cmd setang -20 0 0` now shows the authored cloudy sky and trees in native
Vulkan (`/tmp/portal-escape-sky-out/sky.png`); the Portal boot scene check
passed. Reproduce it with `portal_boot.py --runtime run/runtime-native
--build build-r03-portal-native --renderer native-vulkan --headless --map
escape_02 --console-command 'cmd noclip' --console-command 'cmd setpos 0 0
1000' --console-command 'cmd setang -20 0 0' --out OUT`. This is a native
scene check, while the pixel fixture supplies the DXVK comparison.

## MonitorScreen_DX9 image and color controls (2026-09-23)

MonitorScreen_DX9 was dropped by the native material allowlist, leaving panels
black. Native now samples the base texture and optional second texture from the
material's sampler 1, uses the independent c48/c49 and c50/c51 UV matrices,
then applies monitorscreen_ps2x.fxc's contrast, saturation, and tint. The
second texture's alpha also multiplies the base alpha. The ordinary material
blend state and sRGB reads/writes remain the source of raster and color policy.

Evidence: four material-pixel cases cover the base image, second image,
contrast/saturation/tint, and two translated texture coordinates. Native and
DXVK produced identical RGB bytes at all eight samples. The flat-texel cases
also satisfy an independent shader equation; five seeded negative fixtures
exercise the oracle and case validation. The DXVK reference and reproduction
command are in quality/fixtures/material-pixels/README.md. Real in-game monitor
render targets still need a scene capture to verify the full producer-to-screen
path.

## Portal slideshow screen and Sprite_DX9 tint (2026-09-23)

The hanging GLaDOS screen in `escape_02` is a VGUI slideshow. Its first image
panel was created hidden, while the screen assumed slide index zero had already
been shown. Initializing the last index to -1 and the visibility state to false
makes the first enabled slide visible; an in-game native Vulkan capture at
`/tmp/portal-escape-native-slide-3/slide.png` shows the authored slideshow image.

The cake-room white circles came from translucent `sprites/sphere_silhouette`,
not from the sphere model's self-illumination. Sprite_DX9's VERTEXCOLOR combo
multiplies the texture by the vertex RGB and alpha, but native Vulkan had been
using the texture at full strength. The native fragment path now applies both
channels, with vertex RGB gamma conversion matching the Sprite_DX9 SRGB combo.
Three material-pixel cases cover a dim linear sprite, tinted alpha blending,
and sRGB tinting. The native and DXVK captures match byte for byte; the DXVK
reference and shader artifact requirements are recorded in
`quality/fixtures/material-pixels/README.md`. The direct `escape_02` capture at
`/tmp/portal-escape-native-sphere-fixed/spheres.png` shows the silhouettes
darkened. The general Portal boot heuristic marks that particular near-black
room screenshot incomplete, so it is visual evidence only; the sprite pixel
conformance is the passing automated gate.

## Frosted and refractive glass; masked metal reflections (2026-09-23)

The native material path now creates six-layer cubemap images, uploads every
VTF face and mip, and binds the cubemap on the textured pipeline. The frosted
`LightmappedGeneric` observation pane in `testchmb_a_01` now samples its local
reflection instead of showing only the translucent base. `Refract_DX90` also
has a bounded native path for the Portal window material: screen-copy sampling,
normal distortion, blur, tint, and cubemap reflection. Materials requesting
the unsupported optional second normal, tint texture, silhouette fade, mask,
or vertex color modulation still fail explicitly.

The cubemap change exposed excessive reflections on dark chamber panels.
`metal_lift001` uses inverse base alpha as its reflection mask;
`metalwall048b` uses bump-map alpha. The native path now applies both authored
masks where selected, then the material's envmap contrast, saturation, and
Fresnel response. The reflection direction for bumped walls still uses the
geometric surface normal, so exact DXVK pixel parity is not yet claimed.

Evidence: native Portal boots passed on `testchmb_a_01` and panel-heavy
`testchmb_a_06` with draw-state fixtures. The latter recorded sampler 4 as
`metal/metalwall048b_med_height-ssbump` on the panel draws. At the same fixed
`testchmb_a_06` camera (`setpos 300 160 430`, `setang 0 90 0`), the bright
panel reflection region's mean RGB intensity fell from 172 to 61 (8-bit
channel scale) after the masks and contrast; the captures are
`/tmp/portal-black-panels-upper-prior.png` and
`/tmp/portal-black-panels-verified.png`. The 25-check native bring-up,
31-check material equivalence, and 13-check material-facing suites passed.
Vulkan validation was unavailable on this runner. A DXVK headless boot failed
before scene capture, so the comparison is against Source shader equations and
the earlier native capture, not a new DXVK frame.

## VGUI and HUD follow the SDL3 display scale (2026-09-23)

The VGUI surface now lays out, paints and exchanges cursor positions in UI
units. The UI scale is the number of viewport pixels per unit. Before this,
fixed-pixel UI (menus, dialogs, console, HUD icons such as the Portal
crosshair) shrank on a 150% display whenever the back buffer matched the
window's pixels.

- Policy: `vguimatsurface/UIScale.{h,cpp}`. The scale is the display scale
  (`ILauncherMgr::GetWindowDisplayScale`, i.e. `SDL_GetWindowDisplayScale`) or
  the user's `ui_scale`. It is multiplied by viewport height / window pixel
  height, so a magnified back buffer is not scaled twice. Scales are limited to
  0.5–4 and never leave fewer than 640x480 units. `ui_scale` is an archived
  engine ConVar (0 = automatic) and appears as "UI scale" in Options > Video.
- Surface (`CMatSystemSurface`): the 2D projection spans the viewport in
  units. Fonts rasterize at their pixel size, report metrics in units, and
  place glyph quads on the pixel grid, so text stays sharp. The surface records
  the size the panels last laid out at. It re-notifies whenever the screen in
  units differs from that size, including scale changes between video modes.
- Proportional layouts (the HUD's `hudlayout.res`) are already a fraction of
  the screen height; the scale reaches them only through font raster size.
- Root sizes: the engine root panels always fill the screen in units
  (`GetRootPanelSize`), including the per-paint resize in
  `CEngineVGui::Paint`. The client's per-view HUD root in
  `CViewRender::RenderView` uses the pushed viewport in units. Before this, both
  reset the HUD root to pixels every frame, which put the crosshair at the
  bottom right.
- The Portal quickinfo crosshair's `ScreenHeight()/2160` workaround is
  reverted. The brackets draw at the artwork's size in UI units.
- The SDL3 provider's `SetCursorPosition` now maps back-buffer pixels to
  window coordinates, the inverse of its mouse event mapping.

Evidence:

- `vgui.ui_scale` (28 checks) and `vgui.ui_scale.sensitivity` (6 checks; five
  broken policies rejected) pass through `tools/quality/conformance.py`.
- Isolated headless sway output at 2880x1620, scale 1.5, native Vulkan:
  - A 1920x1080 back buffer selects 1.0.
  - A 2816x1620 back buffer selects 1.5 (1878x1080 units). Menu, dialog and
    console text match the magnified path's size and are sharp.
  - A virtual pointer click selects the Video tab.
  - The in-game crosshair is centered at `ui_scale` 0 (auto 1.5) and 2.
- Not covered: X11 content scale, macOS/iOS/Android display scales, the
  D3D9/DXVK tree (neither built nor run), and SDL2 (`sdlmgr.cpp` reports 1 and
  was not compiled in this profile).

## Shader rendering parameters and PC color conversion (2026-09-24)

The native `IShaderAPI` now retains the float, int, and vector rendering
parameters in separate bounded arrays, with zero defaults and zero results for
invalid indices. This matches the D3D9 state contract used by shader helpers
for fixed lighting, morph offsets, and other per-pass decisions; previously
every setter discarded the value and every getter returned zero. The
hardware-specific gamma conversion entry points now use the same PC sRGB
transfer functions as D3D9, rather than returning zero.

The material-facing Vulkan suite exercises all three parameter banks, their
last valid slots, invalid indices, and an sRGB midpoint round trip. It passed
on the native GPU runner: 25 checks, 0 failures. The backend and suite built
with the existing native Vulkan Waf profile. This slice establishes state
round trips; it does not claim that fixed-lighting or morph shader variants
render correctly yet.
