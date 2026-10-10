# RFC 0024: Direct3D 12 Device Adapter for the Render Core

- Status: **Withdrawn (2026-10-10): the Direct3D 12 adapter and the render-d3d12-windows profile were deleted by user direction; see [RFC 0016, Adapter freeze and scene first](0016-render-core.md#adapter-freeze-and-scene-first-user-direction-2026-10-10).** The text below is the record of the design as it stood.
- Previous status: Proposed (2026-10-07); X0–X5 implemented (see [Progress](#progress)).
- Date: 2026-10-07
- User direction (2026-10-07): "add a DX12 device adapter and backend".
  This is the separate decision RFC 0016's non-goals require for a native
  D3D12 adapter ([non-goals](0016-render-core.md#non-goals)).
- Render architecture: [RFC 0016](0016-render-core.md) owns
  `render.device.v2`, its conventions, capability negotiation, shader
  artifacts and the binding rules. This RFC adds an adapter; it changes no
  port clause and no portable module.
- Verification: [RFC 0005](0005-quality-and-correctness-harnesses.md)
  Q-PRESENTATION. The shared `render.device.v2` suite and its bad adapters
  are the oracle, as for Vulkan (K1) and OpenGL (K10).
- Tracking: no ranked row. It is a sibling of R92 (K10) and a provider for
  R97 (device switching). Ranking it is the user's decision.

## Why

The core's adapters are Vulkan and OpenGL 4.5/ES 3.1. Windows is a
preserved compatibility profile, and D3D12 is its native API: it is the
path for drivers whose Vulkan is weak, for Xbox-family hardware, and for
SDKs that ship D3D12-only builds (RFC 0021 lists
`amd_fidelityfx_upscaler_dx12.dll`). A third, structurally different API is
also the strongest check that the port really is API-neutral.

"Backend" here means the core's adapter plus its presentation bridge. It
does not mean porting `shaderapidx9` or the legacy material stream to D3D12;
those stay frozen (binding rule 1). The legacy frontend reaches D3D12 only
once R91 retires the legacy stream, exactly as for GL.

## Decisions (agent decisions under the user's standing instruction)

1. **Module.** `render.device.d3d12` in `render/device/d3d12/` with
   `public/render/device/d3d12/`, provider name `"d3d12"`, registered in the
   composition-owned provider catalog. Declared in `architecture/modules.json`
   in the adapter column; CAP011 forbids any portable edge to it. No D3D,
   DXGI or COM type appears in a port header.
2. **Mapping.**
   - Completion tokens: one `ID3D12Fence` per queue, token = fence value.
     Resource retirement follows the token (RFC 0006), never frame counts.
   - Bind groups: one root signature per pipeline layout derived from the
     four bind groups, each group a descriptor table in shader-visible
     CBV/SRV/UAV and sampler heaps; per-draw constants as root constants.
   - Memory: D3D12MA (pinned), private to the adapter; placed resources
     give the graph's aliasing.
   - Barriers: enhanced barriers where the device reports them, legacy
     resource-state barriers otherwise; the graph's sync validator stays the
     judge.
   - Rendering passes: `BeginRenderPass` (no render-pass objects needed).
   - Indirect draws (D30/D31): `ExecuteIndirect` with a command signature.
   - Async compute: a compute queue, matching `render.graph.v1` G12.
3. **Shaders** (revised 2026-10-07 during X2). New `ArtifactFormat::kHlsl`:
   GLSL → SPIR-V (unchanged) → HLSL by the pinned SPIRV-Cross, checked at
   build time by the pinned DXC (Linux). The adapter compiles the HLSL at
   pipeline creation with the pinned DXC library (`dxcompiler.dll`, shader
   model 6.6) and caches each program, as the GL adapter compiles GLSL.
   Ahead-of-time DXIL was rejected: DXIL has no specialization constants,
   so every constant combination would need its own binary; SPIRV-Cross
   spells each constant as a macro the adapter defines. The artifact
   contract (registers, spaces, root constants at `space15`, specialization
   comment lines, `TEXCOORD<location>` inputs) is in
   `tools/render/d3d12_lane.py`, which owns the form until it joins
   `tools/render/shader_artifacts.py` as `HLSL_GENERATED`.
4. **Presentation.** An SDL3–D3D12 bridge under `render/bridge/`, owning
   the DXGI flip-model swapchain, its tearing/vsync policy
   (`render.present-policy.v1`) and HDR (scRGB FP16 for the extended-linear
   range of `render.presentation.v1`). Native handles stay in the bridge.
5. **Build and test hosts.** The required lane is Linux: MinGW-w64 with its
   D3D12/DXGI headers, run under Wine with vkd3d-proton
   (`tools/render/d3d12_lane.py run`). X5 also runs under Wine (user
   decision, 2026-10-07: "use wine for x5"); no Windows runner is needed for
   any gate. This proves the adapter against the D3D12 API as vkd3d-proton
   implements it, not against a Windows driver; that limit is stated with
   every result.
6. **Shared recording.** The adapter's command list, encoder, upload ring
   and validator started as copies of the GL adapter's. source-engine-3b is
   extracting them into `render.device` (`public/render/device/recording.h`,
   RFC 0025 decision 9); this adapter migrates to that one copy and deletes
   its own when the helper lands.

## Gates

| Gate | Done looks like |
| --- | --- |
| X0 | DirectX-Headers, D3D12MA, DXC, Wine and vkd3d-proton pinned by archive in a profile (`quality/product_profiles/render-d3d12-windows.json`); `render.device.d3d12` builds under MinGW; CAP011 rejects a seeded portable→d3d12 edge |
| X1 | Device, queues, fences, buffers, textures, heaps and upload ring pass the resource and completion clauses of `render.device.v2` under Wine+vkd3d-proton; the bad adapters are caught; debug layer (or vkd3d validation) silent |
| X2 | `kDxil` artifacts for every core program; graphics/compute pipelines, bind groups, passes, indirect and timestamp clauses pass; conventions section passes |
| X3 | The five core pixel families pass within the recorded cross-backend tolerance of Vulkan; `render.graph.v1` on the adapter, sync validation silent |
| X4 | SDL3–D3D12 bridge passes the shared presentation suite (resize, zero-size, loss, multi-surface, delayed completion); the render core's device draws into a back buffer the bridge presents (`render_lab`'s presenting path, `render/lab/app`) |
| X5 | X1–X4 and the resolution-sweep frame times under Wine with vkd3d-proton (user decision, 2026-10-07) |

No product (`play`, `play_p2`) selects `"d3d12"` before R91; until then the
legacy host pins the process to Vulkan (see R97).

## Non-goals

- D3D12 for `shaderapidx9`, mod shader DLLs or the frozen native backend.
- DirectX Raytracing, mesh shaders or work graphs; each needs its own slice
  after X3.
- Xbox (GDK) packaging.

## Open questions for the user

- Rank: whether this outranks R92/R97 work.

## Progress

### X0–X2 on the Linux lane (2026-10-07)

- Module `render/device/d3d12/` (`device.cpp`, `encoder.cpp`, `execute.cpp`,
  `formats.cpp`, `pipelines.cpp`), public header
  `public/render/device/d3d12/provider.h`, provider `"d3d12"`.
- Pins: DXC v1.9.2609, Linux `linux_dxc_2026_09_28.x86_x64.tar.gz` (sha256
  `96faadc7…aa39a1`, 13,297,763 bytes) and Windows `dxc_2026_09_29.zip`
  (sha256 `ad31b1fc…a4a7e7f1`, 32,287,592 bytes), fetched into
  `dependencies/shader-toolchain/archives` and verified by the lane.
- Mapping: one direct queue and one fence (token = fence value); per-
  submission command lists replayed from the recorded lists; whole-resource
  legacy barriers (vkd3d-proton reports no enhanced barriers); tight port
  buffer layouts copied through 256-byte-pitch scratch buffers; bind groups
  as descriptor tables in shader-visible heaps, written once; draw constants
  and SPIRV-Cross's base vertex/instance as root constants; indexed indirect
  draws through `ExecuteIndirect` with a signature per stride.
- Claimed: compute, storage buffers, BC, cube arrays, timestamps,
  multi-draw indirect, indirect count. Not claimed: indirect first instance
  (SV_InstanceID omits it), async compute/transfer, external images,
  transient aliasing, ray query.
- Evidence (Wine 11.0 Staging, vkd3d-proton from GE-Proton11-5, AMD Radeon
  8060S, RADV): the shared `render.device.v2` suite with real pixels in
  three configurations (default, 256 KiB ring, debug layer), 1,141 checks,
  0 failures, 0 skips, debug layer silent. Before pipelines (X1 only):
  862 checks with 27 failures, all pipeline creation.
- Sensitivity: two bad adapters (`D3d12AdapterOptions::Sensitivity`), each
  run through the whole shared suite, must fail it: `unsafe-upload-reuse`
  (ring ranges retire at submission; a 256 KiB ring, because D10's 96 KiB
  uploads bypass a 64 KiB one) and `release-before-token` (released
  resources free at once; the adapter refuses a held submission naming a
  freed resource as a loss instead of dereferencing it). Both are caught;
  the whole run is 1,161 checks, 0 failures (`d3d12_lane.py run`).

### Registration, shared recording and D38 (2026-10-07)

- D38 line fill: `kFillModeLines` claimed (`D3D12_FILL_MODE_WIREFRAME`); the
  suite's line-fill pixels pass, and a run with the capability masked
  (`d3d12-no-line-fill`) is refused by name. 1,563 checks, 0 failures.
- The command list, encoder, upload-ring bookkeeping and validator are now
  source-engine-3b's shared `render.device` helper
  (`public/render/device/recording.h`, `87de76c3e`); this adapter's copies
  are deleted (decision 6 met). Same suite after the migration: 1,563/0.
- DXC is an import of the executable (`-ldxcompiler`, `dxcompiler.dll`
  beside it), so the adapter has no native loader site (`archlint
  inventory --verify` lists 9 uninstrumented sites, none of them here).
  The DXC pin has one owner, `quality/toolchain/dxc.json`, read by
  `shader_toolchain.dxc_release`.
- Waf: `render/device/d3d12/wscript` (`render_device_d3d12`, `arch_module
  render.device.d3d12`), built on every Windows target (`RENDER_CORE_D3D12`,
  `--render-core-device=d3d12`), composed by `render.composition` (`FindDevice`
  "d3d12"). Waf has no MinGW profile and the MSVC runner is optional, so no
  Waf build of it ran here; the lane is the build that ran.
- `architecture/modules.json`: module `render.device.d3d12` (backend; edges
  to `foundation` and `render.device`), its target, the composition's edges
  and the render layer contract's adapter lists. `archlint check --all`
  reports nothing for it (the 211 findings are existing `games/csgo` and
  other debt).
- Conformance manifest: `render.device.v2.d3d12` (1,563 checks, 52 s through
  `conformance.py check`).

### X3: pixel families and the render graph (2026-10-07)

- HLSL joined the shared artifacts: `shader_artifacts.hlsl_compile` owns the
  form (moved from the lane), every core program has an `_hlsl.h` twin
  (`HLSL_GENERATED`) and `kHlsl` store entries; all 55 rows translate and
  compile with the pinned DXC.
- Family harness: `RENDERTEST_FAMILY_D3D12` builds a D3D12 device;
  `RENDER_FAMILY_RECORD_DIR` and `RENDER_FAMILY_REFERENCE_DIR` let a one-
  device build judge every pixel against the Vulkan frame of the same case,
  recorded by the Vulkan suite on the same tree in the same run, with
  `quality/fixtures/render-families/cross-backend-d3d12-v1.vdf`.
- Measured: 46 of 51 cases within the port tolerance (unlit and water
  byte-identical); five lightmapped bump/env-map cases differ only at
  isolated pixels on the procedural pattern's discontinuity curves (141-202
  pixels; a one-pixel shift raises them tenfold, so no offset): recorded as
  outliers with about 1.25x headroom. Port pixels: worst difference 0 in
  every case.
- `render.graph.v1` on D3D12 (`RENDERTEST_GRAPH_D3D12`): 1,000 random graphs
  compile, validate and run on both executors, transients pooled, single-
  sample and 4x MSAA scene-color capture pixel-exact, debug layer silent
  (19 checks). G12 skipped: no async compute queue is claimed.
- Through `conformance.py check`: unlit 113, water 74, lightmapped 348,
  vertexlit 176 and graph 19 checks pass. `render.family.pbr.d3d12` fails
  only `claim.refuses-an-environment-map-by-name`, which fails identically on
  Vulkan at a clean `HEAD` (an existing VMT claim-rule failure, not the
  adapter's); every PBR pixel check passes.

### X4: the SDL3-D3D12 presentation bridge (2026-10-07)

- `render/device/d3d12/backend_v1/`: render.backend.v1 over D3D12 (one
  direct queue and fence per device; completion gate for tests is a queue
  `Wait` on a fence the CPU signals on release, so held work is genuinely
  incomplete on the GPU). Its endpoint gives the bridge native textures,
  recorded submissions and `Port()`: render.device.v2 hosted on the same
  `ID3D12Device` and queue (`public/render/device/d3d12/host_device.h`
  `CreateHosted`, `ImportTexture`; hosted devices refuse `Recover` as
  `kFatal`, the host recreates both).
- `render/bridge/sdl3-d3d12/`: presentations render into back buffers of
  their own; `Present` copies into the current buffer of a DXGI flip-model
  swapchain on the window's `HWND` and presents, after the frame's work on
  the same queue. The swapchain follows the window's drawable
  (`ResizeBuffers` once its last use completes); its last use is a fence
  signal after the present. vkd3d-proton drains the queue when a swapchain
  is released, so a destroyed presentation's swapchain is released once the
  device is idle, and parked (listening to the surface in the
  presentation's place) only while the queue is held. kExtendedLinear is an
  scRGB RGBA16F swapchain; where the display cannot show it (the headless
  session), creation fails `kSurfaceIncompatible` by name.
- Pin: SDL3 3.4.16's own MinGW release (`SDL3-devel-3.4.16-mingw.tar.gz`,
  sha256 `c7ef65bd…f18a9c`, 5,293,881 bytes) in
  `quality/product_profiles/render-d3d12-windows.json`, the profile that
  also lists the adapter's required checks.
- `render.presentation.sdl3-d3d12` (`unittests/rendertest/test_sdl3_d3d12_presentation.cpp`,
  run by `d3d12_lane.py suite --sdl3 --session` under Wine inside a private
  headless mutter session): the shared render.presentation.v1 suite 52/52;
  native pixels (two windows present their own colors, the negative
  control, the swapchain after an aspect change); and the core's device
  clearing an imported back buffer the bridge presents, in two colors, with
  no import left live. 64 checks, 0 failures.
- `render_lab` itself still builds only with the Vulkan adapter (its KTX
  readers have no MinGW build); the presenting path it uses is what the
  `core.*` checks exercise on D3D12.
- Found on the way: the lane rebuilt objects only on their own source's
  change; it now tracks every included file (`-MMD`).

### X5: the resolution sweep under Wine (2026-10-07)

- Measured on the benchmark machine only (bazzite: RTX 3070, NVIDIA
  615.71.09; D3D12 under Proton 11.0's Wine, its vkd3d-proton and DXVK
  `dxgi`), never the development host. `d3d12_lane.py sweep --remote` builds
  both suites here, runs them there back to back, and refuses a run while
  another compute process holds the GPU (`CONTENDED`).
- Workload: every case of the lightmapped (26) and PBR (55) families drawn
  full-frame at each resolution, 30 timed draws per case between port
  timestamps (`RENDER_FAMILY_SWEEP`, `family_pixel_cases.cpp`). Sums of the
  per-case GPU medians (p95 in parentheses), in ms. A per-draw cost of the
  core's material programs, not a gameplay frame:

  | family | resolution | Vulkan | D3D12 | D3D12/Vulkan |
  | --- | --- | --- | --- | --- |
  | lightmapped | 1280×720 | 0.925 (0.941) | 1.066 (1.091) | 1.15 |
  | lightmapped | 1920×1080 | 1.990 (2.008) | 2.158 (2.191) | 1.08 |
  | lightmapped | 2560×1440 | 3.482 (3.501) | 3.695 (3.713) | 1.06 |
  | lightmapped | 3840×2160 | 7.640 (7.771) | 8.111 (8.180) | 1.06 |
  | pbr | 1280×720 | 2.290 (2.335) | 2.762 (2.819) | 1.21 |
  | pbr | 1920×1080 | 4.921 (4.958) | 5.672 (5.722) | 1.15 |
  | pbr | 2560×1440 | 8.557 (8.597) | 9.671 (9.728) | 1.13 |
  | pbr | 3840×2160 | 19.041 (19.078) | 21.265 (21.312) | 1.12 |

  The D3D12 path (HLSL → DXIL → vkd3d-proton's DXIL→SPIR-V) costs 5–21% more
  GPU time than the same programs as SPIR-V, the gap shrinking with
  resolution. Evidence: `sweep.json` in the lane's output.
- Found on NVIDIA: specialization constants as DXC defines let DXC fold them
  and round differently from the generic program
  (`specialized.identical-pixels.pbr_bumped`); the adapter compiles with
  `-Gis` (strict IEEE), and both GPUs pass. Proton's Wine hangs a second
  program started in a reused prefix in `CreateDXGIFactory2`: the remote
  script uses a fresh prefix per program.
- No product frame time: no product composes the D3D12 device before R91.

### MSVC under Wine (2026-10-08, user direction)

`tools/render/d3d12_lane.py --compiler msvc` builds the adapter and any
suite with the pinned MSVC 19.44 (RFC 0027's `tools/windows/msvc_wine.py`;
static C runtime, `/W3 /WX`) and runs it under Wine with vkd3d-proton,
beside the MinGW default. `render.device.v2` passes 1,563/0 and the `pbr`
family matches its Vulkan frames 199/0, as with MinGW (both rerun). Fixes
it needed: `NOMINMAX` before `<d3d12.h>` in the adapter's two headers;
generated shader headers as literals MSVC accepts (16,000-character
chunks, or a character list past 64 KB); 64-bit `ReportConformance`
counts. `--sdl3` with MSVC is refused by name until an SDL3 MSVC pin
exists.
