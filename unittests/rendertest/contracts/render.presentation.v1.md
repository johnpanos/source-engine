# Contract: `render.presentation.v1`

Module: `render.contracts` (public, backend-agnostic)
Contract header: `public/render/render_presentation.h`
(`render::IRenderPresentationBridgeFactory`, `IRenderPresentation`, `IRenderSurface`)
Conformance suite: `unittests/rendertest/conformance/render_presentation_conformance.{h,cpp}`
Reference bridge / oracle: `unittests/rendertest/fakes/headless_presentation.{h,cpp}`
Positive main: `unittests/rendertest/test_render_presentation.cpp`
Sensitivity main: `unittests/rendertest/test_render_presentation_negative.cpp`
Native main: `unittests/shaderapivulkantest/test_sdl3_vulkan_presentation.cpp`
Migration: `REND-PRESENT-001` · RFC 0001 rank 9 (roadmap R16), RFC 0006 M3;
verified under RFC 0005 Q-PRESENTATION

RFC 0001 says a window system supplies opaque surfaces, a render provider
supplies devices, and a **pair-specific presentation bridge** joins the two. The
composition root selects the bridge. Only the bridge interprets native window,
surface or swapchain handles. No device creates presentations, and neither
public header exposes a native type. The `-I public`-only headless build proves
that absence mechanically, and `architecture/modules.json` enforces it.

## 1. Consumers and providers

| Pair (`windowSystem`, `renderBackend`) | Bridge | Evidence |
| --- | --- | --- |
| `headless`, `null` | `rendertest::MakeHeadlessPresentationBridge` | `render.presentation.headless`, `render.presentation.sensitivity` (conformance runner) |
| `sdl3`, `vulkan` | `render_vulkan::Sdl3VulkanPresentationBridge` | `render_presentation_sdl3_vulkan_conformance` (Waf, native GPU and compositor) |

The legacy product path (`IShaderAPI::SetMode` in `shaderapivulkan`) reaches the
window through the same pair's other half,
`render_vulkan::MakeSdl3LegacySurfaceHost` (`IVulkanSurfaceHost`). The legacy
D3D9/DXVK pair keeps its SDL window interpretation inside
`shaderapidx9/winutils.cpp`, behind the preserved `SetMode` ABI.

## 2. Obligations

- **Composition.** `SelectPresentationBridge` returns the bridge for exactly the
  requested pair, or fails with `kUnsupportedPair` and no fallback. A bridge
  rejects a device from another provider and a surface from another window
  system with `kForeignObject`. It rejects a second presentation on one surface
  with `kSurfaceBusy`, a destroyed window with `kSurfaceLost`, a lost device with
  `kDeviceUnavailable`, and the limit plus one with `kTooManyPresentations`.
- **Frames.** `BeginFrame` hands out a live back-buffer device resource.
  `Present` presents it. `CancelFrame` closes it without presenting. Any other
  order reports `kInvalidSequence`. `ResizeTo` during an open frame is rejected
  and changes nothing.
- **Resize without device loss.** Repeated back-buffer resizes, orientation and
  aspect changes, and window-driven resizes never recreate or lose the device.
- **Suspension, not failure.** A zero-area back buffer, a minimized window
  (zero drawable) and an unavailable native surface all return `kSuspended`
  without opening a frame. Presentation resumes when the condition clears.
- **Native surface replacement.** When the window system replaces the native
  surface (a new `GetGeneration()`), the presentation rebuilds against it.
- **Loss.** A destroyed window or a lost device yields `kLost` without faulting
  the device. After recovery, a new presentation on the same surface presents.
  Other presentations keep working.
- **GPU completion before reuse (RFC 0006 M3).** A back buffer or swapchain
  replaced by a resize, or released by `DestroyPresentation`, stays live and
  counts in `GetPendingRetirementCount()` while its GPU work is incomplete. It is
  released only after the device's completion token for an ordering submission
  completes, never on a CPU event, a flag or a frame count. The suite tests this
  against work that is genuinely held: a timeline semaphore on the native pair.
- **Native lifetime.** The window system calls the attached
  `IRenderSurfaceListener` synchronously before a native surface goes away. On
  return the bridge holds no native object built on it. The harness counts every
  window destroyed while a native object survived, and the count must stay zero.
- **Destruction order.** Destroy presentations, then call
  `bridge.ReleaseDevice(device)`, then destroy the device. `ReleaseDevice` refuses
  while a live presentation uses the device, and afterwards waits for and
  releases the native objects the bridge still holds. Windows may close at any
  point. After all of this, the device holds no presentation resources.
- **Multiple surfaces.** One presentation per window, up to the bridge limit, on
  one device. Frames interleave, resizes stay independent, and destruction may
  happen in any order.

## 3. Threading

v1 is single-threaded, like `render.backend.v1`. The window system calls the
listener on the thread that drives presentation.

## 4. Sensitivity

`render.presentation.sensitivity` injects 14 bridge defects, one per
obligation, and requires the named check to fail for each. Examples: recycling on
resize, a resize that loses the device, a fatal zero size, ignoring window
destruction, ignoring an unavailable surface, extent crosstalk, accepting a
foreign device, a second presentation on one surface, a leak on destroy, a stale
listener, no limit, keeping a native binding past release, resizing during a
frame, and `ReleaseDevice` while a presentation is live. The native test adds a
pixel negative control.

## 5. Profile limits (recorded, not hidden)

- **Wayland cannot restore a minimized window programmatically** (xdg-shell has
  no unminimize request). The SDL3 harness therefore reports
  `CanToggleVisibility() == false` on the `wayland` video driver. The suite then
  records `zero.surface_suspends` and `zero.resume_after_surface` as **skips**,
  never passes. The X11 profile covers both.
- **Hiding a window after a FIFO present is not exercised.** On Wayland (mutter
  50.4, Mesa RADV WSI), hiding a window whose swapchain presented in FIFO mode
  kills the Wayland connection (`xdg_wm_base` error 4: "wl_surface already has a
  buffer committed"). The hide commit stays latched behind the FIFO barrier of a
  surface that never repaints, and re-showing it is a protocol error. Non-FIFO
  present modes do not reproduce it. The engine's SDL3 launcher does not hide
  presented windows. This is a platform finding, not covered behavior.
- **A native present may wait on the CPU** for the GPU work it depends on: Mesa
  WSI waits in `vkQueuePresentKHR`. The held-completion step therefore cancels
  the held frame instead of presenting it.

## 6. Not covered

- Concurrent or multi-threaded presentation, and HDR color spaces.
- Fullscreen and exclusive modes, and display-mode changes.
- Real device loss (`VK_ERROR_DEVICE_LOST`). Loss is simulated through the
  device contract.
- Android, macOS and iOS surfaces. The Android native-window replacement path is
  implemented in the SDL3 surface and the legacy host, but has not been run on a
  device.
- A D3D9/DXVK presentation through this contract. That pair still presents
  through the legacy `SetMode` ABI.
