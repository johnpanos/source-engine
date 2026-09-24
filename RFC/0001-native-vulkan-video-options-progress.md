# Native Vulkan Video options — progress

Scope: make the in-game Video options take effect on the native Vulkan shader
API (`materialsystem/shaderapivulkan`), or report them unsupported. Roadmap
context: R32 (native Vulkan MVP), tracked as **R32-VIDEO-OPTIONS**. Phases
P0–P7 follow the approved plan. This record closes no roadmap gate.

Status (2026-09-23):

| Phase | Option | State |
| --- | --- | --- |
| P0 | Portable policy owners | done |
| P1 | Resolution list and current mode | done |
| P2 | Vsync (`mat_vsync`) | done, with a recorded Wayland limitation |
| P3 | Brightness (`mat_monitorgamma`) | done |
| P4 | DirectX level 95 caps | done |
| P5 | Adapter identity and recommended configuration | done |
| P6 | Antialiasing (`mat_antialias`) | done |
| P7 | Flashlight shadow depth (`r_flashlightdepthtexture`) | not started; still reported unsupported |

The evidence below was taken on AMD Radeon 8060S (RADV, Mesa), Fedora 44,
mutter 50.4, SDL3. The build tree is `build-video-native` (native Vulkan,
private `WAFLOCK=.lock-waf-video-native`). The DXVK tree is `build-portal-vulkan`.
Artifacts are under `quality-results/video-options/`.

## P0 — policy owners (`render.contracts`)

Five header-only owners. Each has a checks-v1 suite and a seeded-defect
sensitivity row in `quality/conformance.manifest.json` (profile
`linux-headless-core`) and a contract in `unittests/rendertest/contracts/`:

| Contract | Header | Positive checks | Seeded defect caught |
| --- | --- | --- | --- |
| `render.display-modes.v1` | `render_display_modes.h` | 12 | oversize mode |
| `render.gamma-ramp.v1` | `render_gamma_ramp.h` | 22 | reciprocal exponent |
| `render.present-policy.v1` | `render_present_policy.h` | 4 (all 8 combinations) | mailbox preferred |
| `render.sample-count.v1` | `render_sample_count.h` | 5 (exhaustive) | rounding up |
| `render.dxsupport-policy.v1` | `dxsupport_policy.h` | 13 | catch-all order swapped |

`conformance.py check` over the ten rows passes with g++ (default and
`-O2 -DNDEBUG`) and clang++: `p0-policy-*.json`, `p0-policy-final.json`.

D3D9 `CShaderDeviceDx8::SetHardwareGammaRamp` now builds its ramp through the
shared owner. An equivalence harness compared it with the removed loop over
620,544 entries (gamma 1.60–2.60, TV on and off, several TV ranges) and found
0 mismatches with g++ and clang++. The harness is
`p0-gamma-d3d9-equivalence.cpp` and its results are
`p0-gamma-d3d9-equivalence-*.txt`. One entry first differed because the
harness called double-precision `pow`. The shader API's translation unit uses
the float overload, which the owner also uses.

## P1 — display modes

- `CShaderDeviceMgrVulkan` reads the desktop from the launcher
  (`ILauncherMgr::GetNativeDisplayInfo`; no SDL include). It builds the mode
  list with `BuildBackBufferModeList` and reports the desktop as the current mode.
  - The earlier hardcoded 1920×1080@60 is gone.
  - So is the 0×0 current mode that made `matsys_interface.cpp` clamp the
    configured width to 0.
- Without a launcher (tools, tests) the manager offers no modes; it never
  invents a size.
- **Mode-change callbacks are now kept and invoked, as on D3D9.**
  `ChangeVideoMode` marks them pending, and the next `Present` on the main
  thread runs them (as `CShaderDeviceDx8::ResizeWindow` does). This is what
  lets windowed `mat_setvideomode` resize the window. The engine's
  drawable-following resize logic used to snap the back buffer straight back.
- Evidence:
  - The log reports "19 video modes for desktop 1920x1080@60 (640x360 ..
    1920x1080)" (isolated mutter) and 6 for the 1024×768 offscreen desktop.
  - A windowed `mat_setvideomode 800 600 1` gives an 800×600 screenshot.
  - `material_facing_vulkan_conformance` gains 3 checks for no modes and no
    current mode without a launcher, and an empty out-of-range mode.
- Limitation: modes are in window points, so HiDPI fullscreen is not crisp.

## P2 — vsync

- `VulkanContextConfig::vsync` and `RequestVSync` replace `preferMailbox`. The
  change applies at the next `BeginFrame` with exactly one swapchain rebuild.
- Both native swapchains select through `render.present-policy.v1`
  (`vulkan_present_mode.h`): the legacy context and the R16 SDL3 bridge.
  - Vsync on: FIFO.
  - Vsync off: IMMEDIATE, else MAILBOX, else FIFO.
- `-vkframestats` records the present mode whenever it changes.
- `cmaterialsystem.cpp`: a vsync toggle now reaches `ChangeVideoMode` in
  windowed mode too, through `ForceSingleThreaded`.
  - Before, only fullscreen toggles did (user decision).
  - DXVK regression: windowed toggles on→off→on plus gamma boot and screenshot
    cleanly (`portal_boot`, `run/runtime-dxvk`).
- Evidence: `native_vulkan_bringup_conformance` covers:
  - the surface's modes include FIFO;
  - a request waits for the next frame and rebuilds exactly once;
  - repeating a request does not rebuild;
  - off selects the policy's mode and on returns to FIFO.

  The surface offers IMMEDIATE on X11/Xwayland and only MAILBOX+FIFO on
  mutter Wayland. In game, the log shows the switches and
  `render_presentation_sdl3_vulkan_conformance` still passes.
- **Behavior change.** The native runtime's saved config has `mat_vsync 0`,
  so native no longer presents FIFO by default. It uses IMMEDIATE on X11 and
  MAILBOX on Wayland, as D3D9 always did (`D3DPRESENT_INTERVAL_IMMEDIATE`).
  `frame_pacing.py` and `portal_boot.py` now pin `+mat_vsync 0`, and
  `frame_pacing.py` also pins `+mat_antialias 0`, so their measurements stay
  comparable.
- **Wayland/mutter FIFO stalls: observed, not fixed.**
  - Under the isolated headless mutter, FIFO presents on Wayland intermittently
    leave `vkAcquireNextImageKHR` blocked in RADV's explicit-sync wait. This
    happens in the pre-change code too: a baseline built from `f5f1ef2a` hung
    2 of 10 bring-up runs, and a standalone FIFO loop hung until killed.
  - A normal-sized window that resizes under FIFO stalls repeatedly. The
    pre-change build avoided that case only because its single hardcoded mode
    made every window 1920×1080.
  - X11 shows no stalls (FIFO resize: 0 timeouts, 16 s).
  - Mitigation: acquire now uses a 1 s timeout. On timeout it skips the frame
    and rebuilds the swapchain (`AcquireTimeouts()`) instead of blocking
    forever. In the stalled case this turns a hang into roughly one frame per
    second.
  - Not verified on a real desktop session (tests never open windows there).
- The isolated test runner starts D-Bus with a private configuration that has
  no activatable services (no portals, a11y or gvfs).

## P3 — brightness

- `SetHardwareGammaRamp` builds the ramp through the shared owner and publishes
  it to the context from any thread (mutex plus release/acquire revision). The
  next present picks it up.
- **The ramp is applied at present time.** A present pass
  (`present_gamma.vert/frag`, 256-entry ramp in a per-frame-slot UBO) samples
  the back buffer into the swapchain image.
  - Filtering is nearest when unscaled and the blit's filter when scaled.
  - An 8-bit identity ramp (the default gamma of 2.2) keeps the plain blit.
  - Back-buffer reads (captures, `ReadPixels`) stay pre-gamma, like D3D9's
    hardware ramp. Presented captures include gamma.
- Evidence: the bring-up suite's presented pixels match the ramp within ±1 for
  gamma 1.6, 2.6 and TV mode. For example, back buffer (128, 64, 191) presents
  as (154, 93, 207) at 1.6 and (113, 50, 181) at 2.6.
  - Negative control: presented pixels differ from an unramped present.
  - Scaled present plus gamma is correct over the whole drawable.
  - The identity ramp uses the blit path.
  - Validation shows no new messages (the same 9 pre-existing performance
    warnings as the baseline).
  - In game, `mat_monitorgamma 1.8` logs "monitor gamma applied at present".
- Recorded divergence: D3D9's ramp only affects fullscreen, while native also
  applies it windowed (SDL3 has no window gamma ramp).

## P4 — DirectX level 95

- Caps now report 95. `GetDXSupportLevel` is `min( mat_dxlevel, caps )`, as
  `CHardwareConfig` does.
- `SupportsShaderModel_3_0` follows the effective level.
- `-vkdxlevel 90` restores the old caps (rollback, one release).
- Finding: the stdshaders choose `_ps30`/`_vs30` by `HasFastVertexTextures()`,
  not by level. That stays false on native, so model shaders keep the ps20b
  combos the native ports implement. At 95 the differences are
  `SupportsShaderModel_3_0`, VMT `<DX95` fallbacks, and the recommended row.
- Evidence: the pixel oracle passes every family at level 95. The harness now
  reports `dx_level` (`p4-pixels-dx95`, `p6-pixels`):
  lightmap, exposure, skinning and modellight (both HDR modes), portal, cable,
  sky, monitor and sprite.
  - `pbr-fallback` fails identically before the change (baseline `f5f1ef2a`
    fails it with one more error), so it is outside this work.

## P5 — adapter identity and recommended configuration

- `vulkan_adapter.{h,cpp}` (`render.vulkan.core`) owns the device-type
  ranking, depth-format choice and caps (identity, largest device-local heap,
  back-buffer sample mask, depth sampleability).
  - The live context and a surface-less pre-window probe both use it.
  - The material system asks before `SetMode`, so the probe answers then.
  - A probe/live mismatch is logged.
- `GetAdapterInfo` reports the real name and vendor/device IDs. For example,
  AMD `0x1002`/`0x1586`.
- `TextureMemorySize` reports the largest heap, capped at `INT_MAX`.
- **dxsupport.cfg layering is shared** (`materialsystem/dxsupport_keyvalues.{h,cpp}`,
  module `render.dxsupport-keyvalues`). It moved out of `CShaderDeviceMgrBase`:
  file read and override merge, group facts, `LoadConfig`, and host facts
  including the MEMORYSTATUS RAM semantics.
  - The group choice and order are `render.dxsupport-policy.v1`, also used by
    D3D9 `ReadHardwareCaps` and `ReadDXSupportLevels`.
  - Equivalence harness (verbatim old code against the new adapter, KeyValues
    serialized and compared): 673,440 cases over the real `dxsupport.cfg`
    (1,403 device IDs × levels × CPU × RAM × VRAM) and 2,880 over
    `dxsupport_linux.cfg`, with **0 mismatches**
    (`p5-dxsupport-d3d9-equivalence.txt`). The DXVK tree builds.
- The Vulkan recommendation uses `dxsupport.cfg`, the file DXVK reads, and is
  then clamped to what the backend supports:
  - `mat_antialias` through `ClampSampleCount`;
  - `r_flashlightdepthtexture` → 0 while unsupported;
  - `mat_hdr_level` ≤ 2.
- In game, autoconfig applies `mat_dxlevel 95`, aniso 16, trilinear, vsync 1,
  expensive water and clamped AA/flashlight shadows (`p5-autoconfig`).
  - The runtime's archived `config.cfg` keeps `mat_hdr_level 0` from an earlier
    session, as on D3D9.
- **User-visible one-time change.** The saved native `videoconfig_linux.cfg`
  recorded VendorID/DeviceID 0. The engine now sees a different adapter and
  applies the recommended configuration once, as it does after a GPU swap.

## P6 — antialiasing

- One multisampled color image and one depth/stencil image at the back
  buffer's size, with their own clear/load passes (plain and sRGB), built
  through the shared attachment-pass builder.
- `PipelineKey` (log2 samples in bits 33–35) and `PipelineRenderPass` are the
  single owners of pipeline keys and pass compatibility. Every material family
  (textured, WMSH, PBR direct, WMSH PBR, PortalRefract, skin) takes the pass's
  sample count. Render targets stay single-sampled.
- The pipeline store keeps the bits. Prewarm builds only the active count.
- **Resolve.** `ResolveBackBuffer` runs `vkCmdResolveImage` into the frame's
  back buffer before any read of it: a target copy from the back buffer
  (`_rt_FullFrameFB`, refraction), capture/`ReadPixels`, the present blit and
  the gamma pass.
- The request comes from `ShaderDeviceInfo_t::m_nAASamples`, is clamped to the
  device mask, and applies at the next frame. `SupportsMSAAMode` and
  `IsAAEnabled` report the truth, so Video options now lists 2x/4x/8x.
- Declined under MSAA: the test-catalog pipelines (passthrough, greenify,
  constant color) and the demo pipelines.
- Evidence (bring-up suite, X11):
  - Negative control: 0 blended edge pixels at 1x.
  - 263 blended edge pixels at 4x, with the interior and the clear intact.
  - Presented images are resolved.
  - A request for 64 clamps to 8x, and AA off returns to hard edges (99 checks).
  - Validation caught the back buffers missing `TRANSFER_DST`. That is fixed,
    and the run now shows only the 9 pre-existing performance warnings.
  - In game, `mat_antialias 4` logs 4x and Portal renders normally, including
    the refractive glass sampling the resolved frame (`p6-msaa-portal`).
  - The exposure pixel family now runs at 4x like its D3D9 reference, and its
    counts still match exactly.
- Not done: alpha-to-coverage stays an unimplemented stub. MoltenVK and mobile
  sample counts, and tile-based resolve cost, are unmeasured.

## Unverified / open

- P7 flashlight shadow depth: not started. `SupportsShadowDepthTextures` stays
  false and the recommendation clamps `r_flashlightdepthtexture` to 0.
- No hosted CI lane runs the GPU suites. Android, macOS and iOS present modes,
  sample counts and gamma are unmeasured.
- Wayland FIFO stalls under headless mutter (see P2) need a check on a real
  compositor session.
