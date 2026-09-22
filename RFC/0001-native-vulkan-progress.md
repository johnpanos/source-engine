# RFC 0001 native Vulkan backend progress

Updated: 2026-09-22

This record tracks the *native* Vulkan material backend (`shaderapivulkan`),
distinct from the DXVK compatibility waypoint documented in
[the Portal SDL3/Vulkan slice](0001-portal-vulkan-progress.md). DXVK Native
translates the retained D3D9 material implementation to Vulkan; this work is the
ground-up native path (roadmap R28 bootstrap, then R32 functional MVP). The two
are separate providers and separate evidence.

## Headline: Portal renders through the native Vulkan backend

`portal_boot.py` **passes** on this backend: the engine boots with the native
Vulkan device driving the material system, loads `testchmb_a_00`, renders the
scene, and the screenshot has real scene detail (32 distinct colors, ~65%
midtone, `has_scene_detail: True`), exiting cleanly (returncode 0). This is the
project's own gate for "Portal renders." Reproduce:

```sh
python3 tools/quality/portal_boot.py --runtime /home/john/source-engine-portal-runtime \
  --build build --out /tmp/vk_portal --timeout 40   # -> Portal boot: pass
```

Getting here required diagnosing and fixing a chain of concrete blockers (physics
backend providing no collision, the material system's real `SetMode` entry point,
process-exit shutdown order against a dead Wayland surface, stubbed `ReadPixels`
screenshot readback, per-draw state batching, and frame-boundary geometry
clearing) — see "Engine integration" below. Fidelity is not full DXVK/D3D9
material equivalence: geometry renders with per-draw transforms and a bounded
native shader/texture path (UnlitGeneric-style `$basetexture` sampling, DXT
textures), not the complete Source shader library. But a real, material-driven
Portal scene now renders natively and passes the boot gate.

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

## Configuration notes for follow-up

The working tree is configured for the native-Vulkan product profile
(`--render-backend=native-vulkan`, `DXVK=False`). An exploratory earlier change
removed `materialsystem/shaderapidx9` from the top `wscript` project list and
edited `tools/quality/portal_boot.py`'s physics module argument; those are
independent of this slice and should be reviewed on their own merits before the
DXVK compatibility path or the boot harness is relied on again.
