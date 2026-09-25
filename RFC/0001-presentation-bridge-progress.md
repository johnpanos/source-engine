# RFC 0001 presentation bridges (roadmap R16, rank 9)

Updated: 2026-09-25

R16 asks for four things:

- remove native window interpretation from the new device contract;
- implement pair-specific presentation-bridge factories for the supported pairs
  and headless-null;
- test repeated resize, orientation and aspect change, minimize/zero-size
  recovery, multiple windows and destruction order;
- show that delayed GPU completion prevents early reuse (RFC 0006 M3).

This record states what now holds, the evidence, and what stays open.

## What changed

| Piece | Where | Role |
| --- | --- | --- |
| Presentation contract | `public/render/render_presentation.h` | `IRenderSurface` (portable size, status, generation, one listener), `IRenderPresentation` (`BeginFrame`/`Present`/`CancelFrame`/`ResizeTo`, completion-gated retirement), `IRenderPresentationBridgeFactory` (pair id, structured creation errors, `ReleaseDevice` ordering) and `SelectPresentationBridge`. No native type. |
| Device contract | `public/render/render_backend.h` | `IRenderDevice` no longer creates presentations, and `RenderProviderCaps` no longer claims them. `IRenderBackendProvider::OwnsDevice` lets bridges reject foreign devices. New structured statuses. |
| Headless-null pair | `unittests/rendertest/fakes/headless_presentation.*` | Headless window system and a bridge that presents through the portable device contract alone; 14 injectable defects. |
| SDL3 window-system surfaces | `platform/sdl3/render_surface/` | `Sdl3RenderSurfaces`: opaque surfaces over borrowed `SDL_Window`s, lifecycle-event mapping (background, foreground, window destroyed), Android native-window generation tracking. The SDL types stay in this private endpoint. |
| Vulkan provider endpoint | `materialsystem/shaderapivulkan/vulkan_render_backend_native.h` | Typed `VulkanRenderBackend::FindDevice` and a versioned `VulkanDeviceEndpoint`: sized images registered as device resources, semaphore-aware submission, and a timeline-semaphore completion gate that only tests use. |
| SDL3–Vulkan pair | `materialsystem/shaderapivulkan/sdl3/` | `Sdl3VulkanPresentationBridge` (new contract: VkSurfaceKHR, swapchain, blit present, back-buffer and swapchain retirement behind device tokens, parked native objects released by the window listener or `ReleaseDevice`). `MakeSdl3LegacySurfaceHost` (legacy membrane): the only interpretation of `SetMode`'s window reference. |
| Native core | `vulkan_device.{h,cpp}`, `vulkan_surface_host.h` | `CVulkanContext::Init` takes an `IVulkanSurfaceHost`. The core and `shaderapivulkan.cpp` include no SDL header. Android surface replacement is kept through the host's availability and generation. |

The pair and its dependency are explicit in the build graph:
`shaderapivulkan/wscript` names `sdl3/sdl3_vulkan_surface_host.cpp`, and
`architecture/modules.json` registers the modules. The Vulkan core
(`render.vulkan.core`) may include only `vulkan/vulkan.h`. Only
`render.bridge.sdl3-vulkan` may include both SDL3 and Vulkan. The render
contracts and their conformance suites stay portable. Seeding an SDL include
into the core, or a Vulkan include into the contract, is rejected (`CAP002`).
Both negative cases were run and then reverted.

## Evidence (this host: AMD RADV STRIX_HALO, Mesa WSI, mutter 50.4)

Native runs used a **private headless mutter** (`--headless --virtual-monitor`),
never the user's session. The Wayland profile used a socket with `DISPLAY`
unset; the X11 profile used that compositor's own Xwayland display and auth file.

- `render.presentation.headless`: 46 checks, 0 failures.
  `render.presentation.sensitivity`: 14 of 14 defects detected by their named
  checks. Both pass under g++ and clang++ through the conformance runner.
- `render.backend.null`: 24 checks. `render.backend.sensitivity`: 7 of 7
  defects detected, including the new `lifetime.owns_device`. The full manifest
  passes (93 of 93) and the runner self-tests pass (279).
- `render_presentation_sdl3_vulkan_conformance` (Waf, `build-r16`):
  - **X11 profile:** 48 shared checks, 0 failures, no skips.
  - **Wayland profile:** 46 checks, 0 failures, and 2 recorded skips
    (`zero.surface_suspends` and `zero.resume_after_surface`; see limits).
  - **Both profiles:** 5 pixel checks pass. Two windows show their own solid
    color at their drawable size. The swapchain follows an aspect-changing
    window resize. The negative control holds, and the bridge releases the
    device.
  - Stable across 3 of 3 runs per profile.
  - The held-completion checks run against work genuinely held on the GPU by a
    timeline semaphore (`IsGpuWorkPending` polls fences). They confirm that the
    replaced or destroyed back buffer stays live, that retirement is pending,
    and that everything is released after the GPU completes.
- The existing native suites, built by Waf and run in isolation, all pass:
  `native_vulkan_bringup_conformance` (25, through the legacy surface host),
  `render_backend_vulkan_conformance` (24), `material_facing_vulkan_conformance`
  (13) and `material_equivalence_vulkan_conformance` (31). The last two load
  the real `libshaderapivulkan.so` through `SetMode`.
- Portal on native Vulkan (`tools/quality/portal_boot.py --renderer
  native-vulkan --require-sdl3 --require-wayland`, private `build-r16`,
  isolated compositor): **pass**. Map and player were active, the capture has
  scene detail, and the exit was clean. Evidence is in
  `quality-results/r16/portal-native-vulkan-boot.json` with the captured frame
  (local, git-ignored). `--require-vulkan` does not apply: it attests the DXVK
  `vulkan-compat` provider.
- Architecture: `archlint check --all` reports no `CAP` findings. Its remaining
  ARCH105 findings predate this work (see below). The archlint self-tests pass.
  Style: all new files and edited regions pass stylelint (clang-format 22.1.8).
  Superseded (2026-09-25): at `d6260d90`, `archlint check --all` reports five
  `CAP002` findings in `render.vulkan.core`. `vulkan_device.h` includes
  `vulkan_compute.h`, `vulkan_debug_utils.h`, `vulkan_frame_stats.h` and
  `vulkan_shader_library.h`, and `vulkan_render_backend.cpp` includes
  `vulkan_compute.h`. None of them is registered in the module manifest. The
  core still includes no SDL header.

## Platform findings

- **Wayland cannot restore a minimized window** (xdg-shell has no unminimize).
  The SDL3 harness declares `CanToggleVisibility() == false` there, and the
  suite records skips rather than passes. The X11 profile covers minimize and
  restore.
- **Hiding a Wayland window after a FIFO present closes the connection**
  (mutter: "wl_surface already has a buffer committed"). The hide commit stays
  latched behind the FIFO barrier of a surface that no longer repaints.
  Releasing the swapchain first, or waiting up to 100 ms, does not help.
  Non-FIFO present modes are unaffected. The SDL3 launcher does not hide
  presented windows. Any future hide path must not do so on Wayland, or must use
  a non-FIFO mode first.
- **Mesa's `vkQueuePresentKHR` waits on the CPU** for the present's wait
  semaphore, so presenting held GPU work blocks. The contract gained
  `CancelFrame`, and the held-completion checks cancel rather than present.

## Scope limits and open items

- **R14 prerequisite.** R16 consumes R14's surface-ownership slice
  (`IRenderSurface` and the SDL3 window system's `Sdl3RenderSurfaces`), which is
  delivered here. The rest of R14 is not: event, input, cursor, clipboard and
  gamepad contracts, the SDL2 adapter, and normalized events. R14 stays
  `planned`.
- **D3D9/DXVK pair.** It still presents through the preserved legacy `SetMode`
  ABI. Its SDL window interpretation stays confined to
  `shaderapidx9/winutils.cpp`. A new-contract bridge for it needs a D3D9 device
  on the new contract, which does not exist yet. The legacy provider claims no
  devices.
- **Product composition.** The engine's product path still creates the native
  context through `SetMode`, now via the bridge's legacy host. Migrating
  material-system presentation onto `IRenderPresentation` belongs to R28 and
  R32.
- **Unverified.** Android, macOS and iOS surfaces: the Android generation path
  is implemented but has not been run on a device. The Android arm64
  cross-build with the moved surface code passes: `./build-android-apk.sh --abi
  arm64-v8a` builds and packages the APK, and `android_apk` passes (29
  libraries). That is build evidence only, not device evidence. Also unverified: real
  `VK_ERROR_DEVICE_LOST`, validation layers (unavailable on this host), and a
  required CI lane. Native suites stay Waf targets outside the headless manifest.
  (2026-09-25: the manifest's `linux-native-vulkan-gpu` profile now runs 15
  other GPU suites; the presentation and bring-up suites are not among them.)
- **Pre-existing, not changed here.** The archlint ARCH105 baseline records
  `shaderapivulkan.cpp` `SetMode` at lines 359 and 1381. Before this work it was
  already at 882 and 1948. That drift, plus unrelated `vphysics_box3d` and
  `test_vphysics_conformance` entries, needs a separate reviewed baseline
  update.

## Reproduction

```sh
python3 tools/quality/conformance.py check --suite render.presentation.headless \
    --suite render.presentation.sensitivity --suite render.backend.null --suite render.backend.sensitivity
WAFLOCK=.lock-waf-r16 python3 waf configure --platform-provider=sdl3 \
    --render-backend=native-vulkan --build-games=portal --disable-warns -T release -o build-r16
WAFLOCK=.lock-waf-r16 python3 waf build
# Isolated compositor: never run the window tests on a live session.
dbus-run-session -- mutter --headless --no-x11 --wayland-display=r16-isolated --virtual-monitor 1280x800 &
env -u DISPLAY WAYLAND_DISPLAY=r16-isolated SDL_VIDEO_DRIVER=wayland \
    build-r16/unittests/shaderapivulkantest/render_presentation_sdl3_vulkan_conformance
# X11 profile: start mutter without --no-x11 and use its Xwayland display and auth file.
```
