# RFC 0001 native Vulkan backend progress

Updated: 2026-09-22

This record tracks the *native* Vulkan material backend (`shaderapivulkan`),
distinct from the DXVK compatibility waypoint documented in
[the Portal SDL3/Vulkan slice](0001-portal-vulkan-progress.md). DXVK Native
translates the retained D3D9 material implementation to Vulkan; this work is the
ground-up native path (roadmap R28 bootstrap, then R32 functional MVP). The two
are separate providers and separate evidence.

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
regenerate by recompiling those two files and re-running the embed step (the
header documents the exact commands). This demo pipeline is a bounded proof, not
the material path.

### Outcome-driven evidence (this machine, 2026-09-22)

The bring-up is verified by pixels, not by a clean exit. The smoke test
([`unittests/shaderapivulkantest/test_vulkan_bringup.cpp`](../unittests/shaderapivulkantest/test_vulkan_bringup.cpp))
brings up the context on a real SDL3 window, presents cleared frames, reads the
presented image back, and asserts it matches the requested clear color:

```
[NativeVulkan] device 'AMD Radeon 8060S Graphics (RADV STRIX_HALO)' vendor=0x1002 320x240 images=4
native Vulkan bring-up: 9 checks, 0 failures    (exit 0)
```

The nine checks include: context validity, a non-empty reported adapter name, a
non-zero swapchain extent, the presented center **and** corner pixels reading
back as the requested red, a **negative control** that fails if the verifier
would accept a wrong (green) expectation, a green frame after a live resize with
the swapchain rebuilt, and context invalidity after `Shutdown()`. When no Vulkan
device or display is available the test exits 77 (explicit skip), so a missing
lane cannot be counted as a pass.

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

## Scope boundaries — what is NOT done

This is the R28 bootstrap slice. It does not close R28 fully and does not begin
to satisfy R32:

- **No material rendering.** The `IShaderAPI`/`IShaderShadow`/mesh/shader/render-
  target paths are still the inherited empty-stub implementation. Nothing is
  drawn *into* the frame; only the clear/acquire/submit/present cycle is real.
  Graphics pipelines, SPIR-V shaders, vertex/index buffers, textures, samplers,
  descriptor sets, and the D3D9-equivalent state/draw translation remain to be
  built (roadmap R32).
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

## Configuration notes for follow-up

The working tree is configured for the native-Vulkan product profile
(`--render-backend=native-vulkan`, `DXVK=False`). An exploratory earlier change
removed `materialsystem/shaderapidx9` from the top `wscript` project list and
edited `tools/quality/portal_boot.py`'s physics module argument; those are
independent of this slice and should be reviewed on their own merits before the
DXVK compatibility path or the boot harness is relied on again.
