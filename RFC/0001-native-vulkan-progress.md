# RFC 0001 native Vulkan backend progress

Updated: 2026-09-22

This record tracks the *native* Vulkan material backend (`shaderapivulkan`),
distinct from the DXVK compatibility waypoint documented in
[the Portal SDL3/Vulkan slice](0001-portal-vulkan-progress.md). DXVK Native
translates the retained D3D9 material implementation to Vulkan; this work is the
ground-up native path (roadmap R28 bootstrap, then R32 functional MVP). The two
are separate providers and separate evidence.

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
