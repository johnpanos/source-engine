# RFC 0001 render provider seam (roadmap R15, rank 8)

Updated: 2026-09-22

Rank 8 adds the render provider, device, capability and profile contracts,
structured creation errors, a `LegacyRenderBackendProvider` around
`IShaderDeviceMgr`, a scoped legacy render-services bundle and a conforming
null backend. R15 is done when provider, capability and profile selection are
explicit, the null and legacy contract suites pass, and the material consumer is
tested without new shader globals.

## What exists

| Piece | Where | Notes |
| --- | --- | --- |
| Provider/device/presentation contract | `public/render/render_backend.h` | Existing (`render.backend.v1`). Adapter descriptions now also carry `deviceId`, `driverVersion` and `driverApi` for quirk matching. The header no longer includes `<cstring>`, which legacy targets forbid. |
| Feature profile and quirks | `public/render/render_profile.h` | New (`render.profile.v1`): request, profile, quirk, structured `RenderProfileError`, and `SelectRenderFeatureProfile`. C++11-compatible. |
| Legacy provider | `materialsystem/legacy_render_backend_provider.{h,cpp}` | `LegacyRenderBackendProvider` adapts a bound `LegacyShaderServices` bundle. Identity comes from the catalog entry, adapter identity from `IShaderDeviceMgr::GetAdapterInfo`, and semantic facts from the backend's optional `describeAdapter` hook. It depends only on adapter enumeration (`ILegacyAdapterSource`). It claims no offscreen device or presentation; those stay on `SetMode` until R16. |
| Legacy quirk table and selection | same file | `LegacyRenderQuirks()` has one documented entry, `gl.float-normalization-cubemaps`. `SelectLegacyRenderProfile` is the single owner of "provider + adapter + request → profile". |
| Backend facts | `shaderapiempty`, `shaderapidx9`, `shaderapivulkan` | Null: one software adapter, `driverApi=none`, no features. The null manager previously reported zero adapters. D3D9: sampled sRGB and offscreen render from the adapter's actual caps; `driverApi` is `vulkan` under DXVK and `opengl`/`d3d9` otherwise. Native Vulkan: `driverApi=vulkan`, no features claimed until its own path implements them (R32). |
| Scoped legacy services | `render::LegacyShaderServices` (`public/render/legacy_shader_provider.h`) | Existing. It is borrowed for one binding and cleared on `Disconnect`. The optional `describeAdapter` hook was added. |
| Explicit selection | `launcher/launcher.cpp`, `dedicated/sys_{linux,windows}.cpp` | The roots select the provider (existing) and now state a render profile request before `Connect` (`MaterialSystem_SetRenderProfileRequest`). |
| Material system | `materialsystem/cmaterialsystem.cpp` | `Init` selects the profile after the adapter and manager are initialized. It fails `Init` on a missing required feature and logs `Render profile for provider '<id>' adapter <n>: features=... quirks=...`. The profile is a `CMaterialSystem` member injected into `ITextureManager::Init`; no new global. |
| First migrated consumer | `materialsystem/texturemanager.cpp` | The normalization-cubemap format now asks `profile.workarounds.Has( kFloatNormalizationCubemaps )` instead of `IsOpenGL()`. |

The shared render-backend suite is now C++11-compatible, so legacy-dialect tests
run the same code as the headless and Vulkan targets.

## Evidence (revision `a02987bb` plus the working tree)

| Oracle | Result |
| --- | --- |
| `render.profile` (conformance runner, g++ `-std=c++20 -Werror`; also clang++ ASan/UBSan) | 134 checks, 0 failures |
| `tools/quality/tests/test_render_profile_sensitivity.py` | Unmodified header passes; all 8 seeded defects detected |
| `render.backend.null`, `render.backend.sensitivity` | Pass (suite converted to C++11 without behavior change) |
| `legacy_render_provider_conformance`, DXVK tree | 94 checks, 0 failures. Real null module through the shared suite; 5 bad legacy backends rejected by it |
| `legacy_render_provider_conformance`, native-Vulkan tree | 111 checks, 0 failures, including the native Vulkan module through the shared suite |
| `material_binding_conformance` (DXVK and native-Vulkan trees) | 259 checks, 0 failures, including request binding before/after `Connect` and no profile before `Init` |
| Portal boot, DXVK, current full build (`boot-dxvk-fixed`) | Pass (map and player active, visible scene); logs `vulkan-compat ... features=sampled-srgb,offscreen-render quirks=none` |
| Portal boot, null renderer (`boot-null`, `boot-null-isolated`) | Profile `null ... features=none quirks=none`, clean exit. The runner's screenshot gate fails as expected with a null renderer. |
| Portal boot, DXVK, only the four R15 modules over the earlier known-good runtime (`boot-dxvk-isolated`) | Pass |
| archlint | No findings in R15 files. New modules: `render.contracts` and `render.legacy-backend-provider` |
| stylelint (branch diff from `8c2c4268`) | R15 files clean. The two remaining hunks in `engine/sys_getmodes.cpp:2440` and `materialsystem/texturemanager.cpp:1895` are another session's code. |
| Dedicated tree build | R15 lines compile. The only error is the existing `dedicated/sys_linux.cpp:277` protected `LoadModule` (R12). |

Artifacts: `quality-results/r15-render-seam-20260922/` (git-ignored) and
`quality-results/conformance.20260923T004151Z.json`. Build trees:
`build-rfc0008` (DXVK, `WAFLOCK=.lock-waf-rfc0008`), `build-r15-native`
(native Vulkan, `WAFLOCK=.lock-waf-r15-native`), `build-rfc0008-dedicated`
(`WAFLOCK=.lock-waf-rfc0008-ded`).

Reproduce:

```sh
python3 tools/quality/conformance.py check --suite render.profile --suite render.backend.null --suite render.backend.sensitivity
python3 -m unittest tools/quality/tests/test_render_profile_sensitivity.py
WAFLOCK=.lock-waf-rfc0008 python3 waf build --targets=legacy_render_provider_conformance,material_binding_conformance
# run both with LD_LIBRARY_PATH covering the tree's .so directories and the DXVK SDK lib directory
DXVK_WSI_DRIVER=SDL3 SDL_VIDEODRIVER=wayland python3 tools/quality/portal_boot.py --runtime <staged runtime> --map testchmb_a_01 --out <dir>
```

## Resize loop found and fixed during verification

The first full-build DXVK boot crashed after the window grew ×1.5 per frame
(1024 → 1536 → … → 17496 px) on this 1.5-scale Wayland display. That boot uses
`mat_queue_mode 0`. The cause was concurrent resize work. The immediate
(non-queued) path resets the D3D9 device on the main thread, and
`CShaderDeviceDx8::ResizeWindow` fires mode-change callbacks only on the main
thread. `AdjustForModeChange` then called `AdjustWindow`, which passed the pixel
extent to `SDL_SetWindowSize` as window units. `AdjustForModeChange`
(`engine/sys_getmodes.cpp`) now returns early when the mode, windowed state and
drawable already agree. Explicit mode and fullscreen changes still differ in one
of those and are unaffected. After the fix, the log shows the single expected
HiDPI adoption (1024×768 → 1536×1152) and the boot passes.

## Not verified / remaining

- The D3D9 module has not been run through the shared suite. Its manager
  enumerates adapters only after a material-system `Connect`, so it is covered
  by the product boot's profile selection instead.
- The OpenGL quirk path in a real togl build: no togl profile was built here.
  Selection is unit-tested with a fake OpenGL-translating adapter.
- No required CI lane runs these suites (R02).
- Rendering-capability completion beyond rank 8 remains open. RFC 0001 asks for
  a material system with no process-global shader interfaces (`g_pShaderAPI`
  and related globals remain), separate caps from DX-level policy (step 8),
  presentation bridges (R16) and adapter limits in selection.
