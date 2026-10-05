# RFC 0016 progress: render core

## Device heap snapshots and CPU mip feedback (2026-10-04)

`IRenderDevice2::ReadMemoryBudget()` now exposes a nonblocking per-heap
snapshot. Vulkan enables `VK_EXT_memory_budget` when available and reads VMA's
heap usage/budget; without the extension, usage is labeled estimated and the
budget is unknown. GL reports its own logical buffer/texture storage estimate,
and null reports the byte storage it owns. These reports do not create cache
eviction policy or alter legacy material residency.

`render.resources::MipFeedbackFrame` accepts visible texture footprints,
computes a conservative mip from projected pixel extent and UV span, and
aggregates repeated material/texture requests to the finest mip. The core world
record path now submits footprints for visible world surfaces and static/posed
model draws, aggregating views under the host frame. The Vulkan managed-texture
provider supplies dimensions only for uploaded, single-layer 2D textures;
unknown, render-target, cube and volume metadata produces no request. Geometry
crossing the eye plane also produces no request because its projected bounds
are not reliable. The requests are collected but are not yet connected to a
residency/streaming consumer, and scene-wide validation remains open. This work
does not close R95/R96 or any quality gate.

2026-10-05: with no consumer, `CoreWorld` no longer attaches a collector
(`m_MipFeedbackConsumer`, off), so record paths skip footprint projection.
The future residency/streaming consumer turns it on when it reads
`Requests()`. `sp_a1_intro4_relit` gameplay camera, 1024×768 headless, two
runs each: backend CPU 10.6/12.3 → 10.3/8.5 ms, interval 19.3/21.0 →
20.8/18.5 ms (within the shared-GPU noise after the committed object-box
projection); screenshots identical to the base runs (max 1, as base vs base).

LOD and mip audit (2026-10-05, source read, no pixels changed):

- Model LOD: the core draws the LOD the engine picked per view
  (`CModelRender::ComputeLOD` for posed models, `DrawStaticPropArrayFast` for
  static props; `RenderCoreWorldDraw_*` pass it through). `r_lod -1` (engine
  default) selects by `ComputePixelWidthOfSphere(origin, 0.5)` through the
  model's authored `$lod` switch metric, clamped to `m_RootLOD`; the
  High profile, `run.conf` and `play_p2` pin `r_lod 0`, so every model draws
  LOD 0 there. Static-prop fades and shadow LODs follow the legacy rules.
- Texture sampling: world and model surfaces sample the material system's
  managed textures with that texture's sampler state
  (`CVulkanContext::ManagedTextureSampler`): linear mips, and
  `mat_forceaniso` anisotropy (16 on High) where the material asks for it.
  `mat_picmip -1` loads every VTF mip.
- Residency: no per-mip residency or streaming exists. The legacy texture
  manager loads whole textures (optionally async); `MipFeedbackFrame` has no
  consumer (above), and `ITextureCache` residency is RFC 0016 layout only.
- Smallest change for coverage-based LOD: run the existing `r_lod -1` rule
  on High with the sphere radius taken from the model's bounds rather than
  0.5, so `$lod` thresholds follow projected size. That changes authored
  switch semantics and the shipped High quality, so it is a user decision;
  not applied here. For the P2:CE 4K/BC7 packs, the first residency slice is
  a consumer of `Requests()` that uploads mips coarse-first and raises a
  per-texture min-LOD clamp, then sets `m_MipFeedbackConsumer`.

Gameplay-frame follow-up (2026-10-05, same camera and harness):

- CPU: `perf record -F 999` showed a flat profile; the top symbol was VMT
  interpretation (`MapValues` 4.4 %, `BlockFor` 0.9 %), run twice per dynamic
  draw per view (claim and record). `WorldPass::State::Mapped` now memoizes
  `MapWorldMaterial` by `MaterialSnapshotKey` (which now also carries
  `translucent` and `hasProxy`), bounded at 4,096 entries, under its own
  lock. Backend CPU 8.5–10.3 → 8.4–8.8 ms (within noise); screenshots identical.
- GPU at the arrival camera (`cl_render_debug_gpu_timers`): 13.9 ms, of which
  the back-buffer pass is 13.0 ms: world PBR surfaces 6.6, static models 3.6,
  GTAO 1.1, posed models 0.8, prepass 0.3. Only one `_rt_poweroftwofb` copy
  (22 µs) at this pose; the 18 copies belong to other poses. The next target
  is the surface program's fragment cost.
- Far-field LTC (`kAreaFarDiffuse` 36, rough specular 144): exact vs fast on
  the seven elevator poses, two runs each. On run-stable pixels the
  difference is max 17–18 levels on 0.006–0.03 % of pixels (depart-back,
  depart-outside) and ≤1 elsewhere. The max 124 at `elev-arrive` is the
  elevator video showing a different movie frame, as large between two runs of
  the same build. Cutoff kept. GPU at `elev-arrive-up` 20–22 ms fast vs 26 ms
  exact.
- Unavailable: resolutions above 1024×768 (SDL offscreen cap), Fold7.

Main-thread follow-up: footprint boxes and queued Portal 2 (2026-10-05):

- Footprints (`c159ac943`): before the collector was turned off (above),
  `WorldPass::RecordBatch` was 55–76 % of CPU in every pose, almost all of it
  projecting every index of every drawn model surface to clip space. Model
  surfaces now project the eight corners of an object-space box with UV
  bounds. Static meshes' boxes are built in `SetWorld`. Posed models' boxes are
  measured once per pose per view. A corner behind the eye falls back to the
  exact per-vertex projection. 19 `INTRO4_CAPTURES` poses, default frame cap:
  chamber 50.1 → 22.2 ms frame interval, backend CPU 36.1 → 9.1 ms. Mip
  requests over 1,348 frames: 94 % identical, the rest 1–4 levels finer, never
  coarser (a box only enlarges the screen extent).
- Pooled work, measured and not kept: posing each model on the compute pool
  (`ParallelFor` over `CoreWorld::PoseModel`) was within noise of serial in
  every pose, so it was reverted. After the two changes above, draw recording
  (the frontend's translation, the encoder and `RecordBatch`) is about 10 % of
  samples, so pooled recording (K5) could save at most about 2 ms here.
- Overlap instead: the frame interval was about engine + backend CPU, both on
  the main thread, because `./play_p2` ran `mat_queue_mode 0` while `run.conf`
  already queues Portal. `./play_p2` now passes `+mat_queue_mode 2` for every
  launcher that starts through it (`./play_p2_fsr`, `./play_p2_coop`;
  `581d5603b`, `a8ccf29a7`, user decision). `QUEUE_ARGS=` or
  `+mat_queue_mode 0` rolls back.

  | Run (19 poses, `fps_max 0`) | Rounds | Summed median interval | Chamber |
  | --- | --- | --- | --- |
  | FSR off, unqueued → queued | 5 interleaved | 239 → 186 ms | 20.3 → 14.5 ms |
  | FSR quality (0.666667), unqueued → queued | 3 interleaved | 205 → 155 ms | 20.3 → 12.9 ms |

  Queued was faster in 75 of 95 pose-rounds with FSR off and 46 of 57 with it
  on. Every pose over 10 ms gains 25–40 %. With FSR on, three light poses
  (`arrival`, `instance-front`, `instance-right`, 3.6–4.9 ms) are up to 1.5 ms
  slower queued, which is the handoff cost at very short frames. Images:
  queued vs unqueued differ no more than two unqueued runs do (FSR off: max
  15 vs 12 levels, no pixel above 16; FSR on: max 55–70 levels in both
  comparisons from temporal accumulation, at most 0.007 % of pixels above 16).
  The heavy poses now sit near their GPU time (about 11.5 ms), so the next
  target is GPU cost, starting with the surface program.
- FSR refuses 4x MSAA (`reconstruction failed … samples=4`), which
  `r_core_world_strict` turns into a fatal error, queued or not. The FSR runs
  above set `mat_antialias 0`.
- Harness: `tools/quality/portal_boot.py` through `poses.py`-style drivers on
  `run/runtime-p2-fsr`, `-deterministicrender -vkframestats`, `host_framerate
  0.015`, Box3D, shared GPU and host (load about 7). Unavailable: an
  interactive desktop run, Fold7, resolutions above 1024×768, and a
  ThreadSanitizer run of queued Portal 2.

## FSR temporal reconstruction implementation (2026-10-03, in progress)

User-selected work on `codex/fsr-temporal`, in an isolated worktree, owns
RFC 0019's temporal input/history pass and the private FSR/Vulkan adapter.
The contract-authoring session confirmed it has no overlapping implementation.
The first consumer is `render_lab`; product selection and High remain unchanged
until the input, image, portal and lifetime gates qualify the cutover. Timing
is measured and reported under RFC 0016's FSR exception, never used to block
this path. Desktop and Fold7 evidence will be distinguished explicitly.

The experimental provider pins `johnpanos/FSR-4.1.1-linux` at
`46a56a66b639d4e3f84033640d1e51a8655ebac4` with its original notices: the Vulkan
runtime is GPL-2.0-or-later and the AMD shader/model assets carry their separate
MIT exception. It is an unofficial INT8 adapter, not AMD's signed distribution.
No platform or complete-image acceptance is claimed by adding the dependency.
The [lab implementation record](0019-fsr-lab-2026-10-03.md) links measured costs,
positive and seeded controls, reproduction commands and the open game boundary.

## Portal 2 launcher build repair (2026-10-01)

`./play_p2` exposed two build blockers after the GPU BVH port: the shared probe
shader constructed separate texture/sampler objects for the frozen native
frontend's combined samplers, and `render.pass.lights` still defined an unused
CPU cone test. The shared probe sampler now accepts both declared binding forms;
the obsolete CPU helper and its unused packing fields are removed. GPU BVH
construction and assignment remain the product path.

The Portal 2 Waf build succeeds, and the launcher reaches the native Vulkan menu
with core passes running ([launch log](../quality-results/play-p2-launch-fix.log)).
The GPU assignment suite passes 27 checks
([evidence](../quality-results/play-p2-gpu-bvh-fix.json)); changed-file style
checking reports zero failures. Full archlint still reports unrelated CAP002
and fstop ARCH105 findings. This repairs startup compilation; it does not close
R90/R95/R96/R91 performance or complete-image gates.

## Scope and complexity discipline (2026-10-01)

The user adopted the [scope and complexity discipline](0016-render-core.md#scope-and-complexity-discipline-user-decision-2026-10-01)
as binding rule 9. AGENTS.md links to that single definition. It governs render
slice selection and ownership without reducing declared quality, compatibility,
platform scope or budgets. R90/R95/R96/R91 remain open; this documentation change
adds no implementation or runtime evidence and closes no gate. Verification is
limited to the edited documentation's whitespace and local link targets.

## Clustered-lighting requirement and hard High budgets (2026-10-01)

The user requires high-performance clustered lighting, GPU execution whenever
it is faster, and a minimum of 120 FPS at High on this machine. They confirmed
1920×1080. All 18 primary RFCs now link to the single render requirement in
[RFC 0016](0016-render-core.md#high-performance-clustered-lighting-user-decision-2026-10-01)
and the engine-wide placement rule in
[RFC 0003](0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01).
[The P2:CE research note](0016-p2ce-clustered-lighting-2026-10-01.md) records
feasibility, implementation/quality differences, current CPU product assignment,
existing GPU conformance and retained performance. It supplies no matched P2:CE
FPS benchmark or renderer acceptance.

The new [hard budget policy](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01)
supersedes the historical nonblocking-performance decisions recorded below.
`linux-desktop-high-120` in
[`render-v1.json`](../quality/budgets/render-v1.json) owns numeric frame,
CPU/GPU/submission and component targets. Its declared
[High product profile](../quality/product_profiles/portal2-linux-native-vulkan-high.json)
owns settings, including required MSAA, AO, shadows and baked indirect. Historical
K0 records are retained; they cannot certify High. AGENTS.md records the new
required R90/R95/R96/R91 performance work and keeps their gates open.

The installed `frame_floor.py` consumes the existing laser route's budget-row
reference, forbids relaxed FPS/quality/resolution overrides and capped offscreen
qualification, applies/queries High, records actual Vulkan driver/API facts and
checks the actual back buffer and GPU. It judges every gameplay frame, catches
missing/corrupt/duplicate records and a slow frame flushed at process exit, maps
delayed GPU timestamps by frame identity, and requires complete CPU/GPU cost
samples. Maxima and tail costs are bounded; one slow frame cannot hide behind
p99. Preview evidence is marked separately. Timing success still requires
complete K11/K12/R91 image and representative-scene evidence for promotion.
Per-component lab verdict tooling remains proposed; no component pass is claimed.
No engine or shader optimization is implemented by this policy slice.

Verification in [the retained logs](../quality-results/clustered-policy-2026-10-01/):

- Budget declaration check passes: four rows, zero problems. The native driver
  probe records Radeon 8060S/RADV, Mesa 26.2.3 and Vulkan 1.4.354 in
  [graphics.json](../quality-results/clustered-policy-2026-10-01/graphics.json).
- Quality fixtures pass: 49 frame/floor/pacing, 15 budget, 27 product-profile
  and 38 gameplay-scenario tests (129 total). The installed `quality.selftest`
  also discovers the floor controls. Capped offscreen preflight exits 2 as
  required, before staging or launching a product.
- Architecture fixtures pass (162); loader inventory verification passes.
  Full check and baseline verification fail on the outside-slice fstop
  `CreateInterfaceFn` expansions; full check also reports outside-slice CAP002
  render/native includes. See the exact architecture logs; no ratchet is reset.
- Style fixtures pass (38). Shared-checkout `stylelint --changed --diff`
  reports 11 failures across concurrent C++ edits, outside this policy/tools
  slice. Scoped documentation/tool whitespace, JSON and policy-link checks pass.

Reproduce with `python3 tools/quality/render_budgets.py check` and the installed
`unittest discover -s tools/quality/tests` commands using patterns
`test_frame*.py`, `test_render_budgets.py`, `test_product_profile.py` and
`test_portal2_scenarios.py`. The High run command is documented in
[`tools/quality/README.md`](../tools/quality/README.md). A combined namespace
module invocation had three fixture-import errors; the supported discovery
commands above pass, and that command's log is retained separately.

**Acceptance remains unverified.** No new complete native 1920×1080 High run,
complete-image result or parent-row closure is claimed. R90 still needs the
product CPU/GPU assignment comparison and faster-path handoff; R95/R96/R91
still need complete images/cohorts and the hard frame target. Only the user may
relax the target or reduce High; budget misses require optimization.

## Laser intro local-cubemap reference defect (2026-10-01)

User request: "please fix the missing bitch_cubemap error on the laser intro
bake". That name was the material system's internal `env_cubemap` placeholder,
not an authored VTF. Shader initialization now preserves `env_cubemap` as the
placeholder's name; `GetTextureValue` still resolves it through the current
local cube. The core handoff preserves symbolic envmaps for RPRB rather than
downloading their legacy per-view texture. Named texture paths remain unchanged.
No replacement VTF, reflection fallback or new bake was added.

Frozen-path: defect env_cubemap material-load/serialization — this restores the
symbolic reference across the material-system/core handoff, without a new
lighting term or public ABI change.

The native material harness's `-check-local-cubemap-reference` validates actual
shader initialization and `IMaterialVar` serialization. Its
[reference run](../quality-results/laser-cubemap-fix/reference-final/evidence.json)
passes the symbolic reference and ordinary texture-name checks, followed by
the lightmap pixel oracle. Preloading the retained pre-fix material system makes
the symbolic check fail while the ordinary texture check still passes
([negative control](../quality-results/laser-cubemap-fix/negative.json)). Product
and harness builds pass; material binding passes 259 checks. Reproduce with:

```sh
WAFLOCK=.lock-waf-p2 ./waf build --targets=materialsystem,engine,launcher,material_pixel_conformance -j8
python3 tools/quality/material_pixel_conformance.py run --runtime run/runtime --build build-p2 --renderer native-vulkan --hdr none --family lightmap --extra-arg=-check-local-cubemap-reference --out quality-results/laser-cubemap-fix/repro --timeout 120
```

Earlier source2 laser captures with updated binaries pass. The final strict
product attempt is **not certified**: concurrent primitive-handoff work reports
an unrelated `__fontpage` failure on `$phongexponent 0.000000`
([receipt](../quality-results/laser-cubemap-fix/final/evidence.json)). The isolated
reference regression avoids that cohort and rejects the actual old serialization
defect. Full archlint still reports unrelated fstop CreateInterfaceFn expansions.
Changed-file style and whitespace checks report concurrent edits outside this
fix; its edited source regions have no formatting findings in
`quality-results/laser-cubemap-fix/style-complete.log`. No clean full-tree gate or
parent-row closure is claimed.

## R86-LAYOUT: layout, layer contract and runtime wiring (2026-09-26)

State: `partial`, on branch `render-core` (worktree
`../source-engine-render-core`, from `subsystem-refactor` at `20cd956a`). The
user asked for the render family to be built to the RFC's directory layout and
architectural arrow, hooked into the build, the module manifest, the launcher,
the engine, the client, the material system, Hammer, the dedicated server and
static composition. This slice does that. It closes no gate by itself (see
"Gate criteria" below).

### What exists

Every module is a strict C++20 static library that declares its architecture
module, is listed as `cxx20` in `quality/toolchain/policy.json`, and has a row
in `architecture/modules.json`.

| Layer | Module | Target | Contents |
| --- | --- | --- | --- |
| 0 | `render.math` | `render_math` | `float2`–`float4x4`, `Aabb`, `Sphere`, frusta (depth 0–1, Y up) |
| 1 | `render.device` | `render_device` | `render.device.v2`: `IRenderDevice2`, abstract usages, completion tokens with epochs, four bind groups, pipelines from artifacts, `CommandEncoder` with the one-thread diagnostic, the shared description rules |
| adapter | `render.device.null` | `render_device_null` | recording device: work runs when its token completes, real bytes, a real upload ring, per-resource usage validation |
| adapter | `render.device.vulkan` | `render_device_vulkan` | Vulkan 1.3 (timeline semaphores, synchronization2, dynamic rendering required), sync2 barriers from one usage table, upload ring, bind groups, graphics and compute pipelines, Y-up viewport, validation-message counter |
| 2 | `render.graph` | `render_graph` | builder, compiler (culling, transitions, write-after-write, transient lifetimes and usages), serial executor, trace |
| 2 | `render.shader-library` | `render_shaderlib` | artifact keys and store, permutation keys, pipeline recipes resolved per artifact format |
| 2 | `render.resources` | `render_resources` | texture and mesh caches: stage, record, retire behind the token |
| 3 | `render.material` | `render_material` | families, std140 schemas, registry, parameter blocks with revisions |
| 4 | `render.scene` | `render_scene` | scene with atomic change sets, immutable snapshots, visibility port, conservative frustum culling, draw lists |
| 5 | `render.frame` | `render_frame` | `IRenderer`, `IRenderFeature`, `Stage` in the legacy order, `StageTracker`, `IRenderStageHooks` |
| 6 | `render.renderer` | `render_renderer` | builds each frame's graph from its features and runs it |
| 6 | `render.pass.present` | `render_pass_present` | the frame's last pass |
| 6 | `render.legacy-frontend` | `render_legacy` | the material system's provider (forwards to the wrapped backend, keeping its id), `RenderStageMarkers001`, the legacy-stream feature |
| 7 | `render.composition` | `render_composition` | `RenderCore_Create`/`Destroy`/`GetBinding`/`GetLegacyProvider`; the device and feature catalogs |
| 8 | `render.core-tests` | — | `unittests/rendertest/core/` |

Also added: `hammer.adapters.render` (`hammer/adapters/render/`, target
`hammer_render`, tools products), which projects the editor document into a
render scene the editor owns.

### How it hooks in

- **Build.** The root `wscript` adds `render` to the game, tests and tools
  projects, never to dedicated. `render/wscript` recurses into the portable
  modules, `device/vulkan` when the native Vulkan backend is configured, and
  `device/gl` with `--render-core-gl` (which fails configure until K10).
  `--render-core-device` (default `null`) and `--render-core-features`
  (default `legacy-stream,present`) set the launcher's defaults.
- **Manifest.** Module rows, strict target entries, the `layerContracts`
  section CAP011 reads, and `public/render/legacy/stage_markers.h` and
  `public/engine/render_core_binding.h` in `legacyAbi.paths`.
- **Launcher.** `CSourceAppSystemGroup::Create` calls `RenderCore_Create`
  around the selected legacy backend (`-render-device`, `-render-features`,
  `-render-validation`), binds the frontend's provider through
  `MaterialSystem_BindShaderProvider`, calls `Engine_BindRenderCore`, and adds
  `RenderStageMarkers001` as an app system. `-norendercore` composes nothing.
  `Destroy` destroys the core after every system and module.
- **Engine.** `engine/render_core_host.{h,cpp}` (client builds only). It holds
  the binding, begins and ends the core's frame in the `EngineFrameBegin` and
  `EngineFrameEnd` host render steps, marks every 3D view at
  `CRender::Push3DView` and `PopView`, owns the world scene from `R_LevelInit`
  to `R_LevelShutdown`, and prints `r_core_stats`. The engine links no render
  module.
- **Client.** `CHLClient::Init` looks up `RenderStageMarkers001`.
  `viewrender.cpp` marks the skybox, opaque, translucent, view-model,
  post-process and HUD stages (`game/client/render_stage_marks.h`).
- **Material system.** `LegacyShaderProvider` gains `context` and `createFor`,
  so the frontend forwards without a global. The public API is unchanged; the
  side channels stay until K3.
- **Hammer.** `hammer.adapters.render` composes its own scene; the viewport that
  draws it is R17's decision.

### Evidence

| Check | Command | Result |
| --- | --- | --- |
| Layer contract | `python3 -m unittest discover -s tools/archlint/tests` | 157 tests pass; the six RFC fixtures each fail with their rule |
| Architecture | `python3 tools/archlint/archlint.py check --all` | pass, including CAP011 on the real manifest |
| Transitive includes | `archlint.py check --all --compile-deps build-rc-client --compile-deps build-rc-tests` | pass (166 and 57 strict units) |
| Hermetic headers | `archlint.py hermetic` and `--cxx clang++` | 113 portable public headers, 0 errors each |
| Target owners and links | `archlint.py targets --verify build-rc-client build-rc-ded build-rc-tests` | no ownership or link error (the 19 reported are stale entries for trees not passed) |
| Hammer graph | `archlint.py hammer --verify` | pass |
| Device port, null | `render.device.v2.null` | 279 checks (D13 skipped: no rasterizer) |
| Bad adapters | `render.device.v2.sensitivity` | 10 of 10 detected on their clause; control passes |
| Device port, Vulkan | `render.device.v2.vulkan` (`--runner gpu`) | 499 checks on the Radeon 8060S (RADV, Mesa 26.2.3); 0 validation messages with synchronization validation; D7 skipped (loss cannot be forced) |
| Graph | `render.graph.v1` | 22 checks; 1,000 seeded graphs agree with the independent model and the null device accepts every execution |
| Scene | `render.scene.v1` | 14 checks; 0 visible instances culled over 10,000 seeded instances |
| Frame and renderer | `render.frame.v1` | 25 checks |
| Composition and frontend | `render.composition` | 14 checks |
| Shader library, resources, material | `render.shader-library`, `render.resources`, `render.material.v2` | 9, 15, 13 checks |
| Hammer scene | `hammer.adapters.render` | 6 checks |
| Both compilers | the rows above with `--cxx clang++` | pass |
| Style | `python3 tools/stylelint/stylelint.py --changed` | 104 files, 0 failures |
| Dedicated link map | `python3 tools/render/core_links.py dedicated build-rc-ded` | 15 binaries, no render target, no render-core code, no binding export |
| Client link map | `python3 tools/render/core_links.py client build-rc-client` | the launcher defines `RenderCore_Create`; the engine exports `Engine_BindRenderCore` and defines no render-core code; no second copy (8 seeded self-tests: `python3 -m unittest discover -s tools/render/tests`) |
| Static composition | `python3 tools/quality/static_composition.py check --tree build-rc-static --program launcher_main/hl2_launcher --require client --require server --require GameUI --require engine --require launcher --require vphysics --require vphysics_box3d --require filesystem_stdio --require scenefilecache --require soundemittersystem` | PASS, 24 module objects, 0 errors; the program defines `RenderCore_Create` and `Engine_BindRenderCore`, and the tree built no `.so` |
| iOS static product | `./build-ios-app.sh --no-assets` (iPhoneOS 26.5 SDK, pinned toolchain) | unsigned `Portal.app` built; its static-composition check passes (24 module objects, 0 errors); the arm64 Mach-O `hl2_launcher` defines `RenderCore_Create`, `Engine_BindRenderCore` and 266 core functions; no dylib in the tree. Not run on a device |
| Portal boot, core on | `portal_boot.py --renderer native-vulkan --headless --map testchmb_a_00 --console-command r_core_stats` | pass; 208 frames, 410 views, 2,466 stages, 0 order violations, 416 passes, world scene live (identical in two runs) |
| Queued mode | same, `--startup-command "mat_queue_mode 2"` | pass; `mat_queue_mode` 2; 0 order violations |
| Vulkan adapter in the product | same, `--engine-arg=-render-device --engine-arg=vulkan` | pass; 208 frames, 0 failed |
| Unlinked device | same, `-render-device gl` | the launcher stops: "render device 'gl' is not linked in this product" |
| Rollback | same, `--engine-arg=-norendercore` | pass; `r_core_stats`: not bound |
| Pixels unchanged | screenshot differences, core on vs `-norendercore` | at most 3/255 per channel, 0 pixels over 16/255; two core-on runs differ by up to 2/255 |

Trees (private `WAFLOCK`s): `build-rc-client` (Portal, SDL3, native Vulkan),
`build-rc-ded` (dedicated), `build-rc-tests` (`--tests`), `build-rc-tools`
(`--tools`), `build-rc-static` (`--static-composition`). Configure lines are
the first `# using` line of each tree's `config.log`.

### Decisions (agent, under the user's standing instruction)

1. **The frontend keeps the backend's id.** `-renderer` still selects the
   legacy backend; the core wraps it by default, so profile selection, quirks
   and the debug API see the same provider. `-norendercore` is the rollback.
   The RFC's `-renderer core` spelling is dropped.
2. **The engine marks views, the client marks content.** Every 3D view passes
   through `CRender::Push3DView` and `PopView`, so views nest correctly without
   editing the 30 push sites in the client. The client marks what it draws
   inside them.
3. **`render.legacy-provider-contract` is in layer 1**, beside the device port:
   it is the C++11 port the frontend implements. **Test fixtures get their own
   top layer**, since they test composition.
4. **The strict libraries use the new libstdc++ ABI**, and the engine,
   launcher and client keep `_GLIBCXX_USE_CXX11_ABI=0`. Every interface that
   crosses (`renderer.h`, `scene.h`, `render_core.h`, `stage_markers.h`)
   carries no dual-ABI library type. The frontend is compiled with the core's
   ABI and the platform's defines.
5. **The product device defaults to `null`** until K1 moves `CVulkanContext`
   onto the Vulkan adapter; `-render-device vulkan` composes a second device
   today.

### Gate criteria this slice meets

- R86 (K0): the layer contract rule, with its six seeded violations and a
  passing tree.
- R86 (K1): the port suite on the null and Vulkan adapters; 10 of 10 bad
  adapters; port headers reach no Vulkan, GL or SDL header (CAP005, CAP007,
  CAP011).

### Still open

- K0: the `jobs.graph` module, the pinned shader compiler, the material,
  shader-API and studio headers in `legacyAbi.paths` with vtable fixtures, the
  view and per-draw oracles, the render budgets.
- K1: `CVulkanContext` allocating, uploading and retiring through the Vulkan
  adapter (one Vulkan stack); VMA; the idle-wait scan; per-profile feature
  records (Fold7); the D13 bad adapters (flipped Y, −1..1 depth) and a
  capability-lying adapter; per-subresource state.
- Port rules the Vulkan adapter applies that the null adapter does not (draw
  and dispatch state, released resources in bind groups, render-area and copy
  alignment rules). Until they move into `validation.cpp`, the null adapter
  accepts inputs the Vulkan adapter rejects.
- K2: the Python graph model, the pooled executor, transient pooling and
  aliasing, the graph sensitivity suite.
- K3: the inversion (the frontend implementing `IShaderAPI`), removing the
  side channels, renaming `shaderapivulkan` to `rendercore`.
- K4–K10 as in the RFC. The Hammer viewport is R17.
- Evidence gaps: no device run of the iOS build, no Android build, no
  frame-time measurement of the core's per-frame cost, no CI lane.

## K0: prerequisites and frozen oracles (2026-09-26)

State: every K0 check passes on the Linux desktop and the Fold7, with one
declared gap (below). The user asked for K0 after R86-LAYOUT ("K0 is
required").

| K0 check | Evidence | Result |
| --- | --- | --- |
| Layer contract rule exists | `python3 -m unittest discover -s tools/archlint/tests`; `archlint check --all` | the six seeded violations fail with their rule; the tree passes |
| Job system has a module | `jobs.graph` in `architecture/modules.json`; `jobsystem` declares `arch_module = 'jobs.graph'`, builds strict (`-Werror`, keeping the engine's libstdc++ ABI) and has left `engine-and-tiers`; `archlint check --all`, `archlint targets --verify` | pass; the 16 required `jobsystem.*` suites pass |
| Shader compiler pinned | `shader.toolchain-pin` (`tools/render/shader_toolchain.py check`) and `.sensitivity` | shaderc v2026.1, pinned by source archive and sha256 (`quality/toolchain/shader-compiler.json`), built into `dependencies/shader-toolchain/`, rebuilds every committed SPIR-V module byte-identically (30 checks); a foreign compiler and a flipped byte are each detected (8 checks). The host's Fedora glslc is refused |
| Frozen headers listed | CAP010 over `legacyAbi.paths` | the 24 headers of the frozen render surface (material system, studio render, view and model render, shadow manager, mod shader ABI) are listed and CAP010 passes; their seven `CreateInterfaceFn` baseline entries left the loader baseline as preserved ABI |
| vtable fixtures | `legacy.render-abi` (legacy-cxx11), `.sensitivity`, `.table` | 541 methods of the eight interfaces match `quality/fixtures/render-abi/render_abi_v1.h` (574 checks, g++ and clang++); a seeded slot reorder is detected in 8 of 8 interfaces, and an appended virtual in 8 of 8 |
| View oracles captured | `render.view-oracles`, `render.view-oracles.comparator` and its seeded rows (`tools/render/view_oracle.py`) | `testchmb_a_00`, `testchmb_a_08`, the legacy-ports view set (`testchmb_a_01`) and Portal 2's `sp_a1_wakeup`: 62 shots, 411 views, 21,632 draws, with portal recursion at every depth up to 10, water reflection and refraction, a monitor, glass and the pause menu. Four independent captures and a `mat_queue_mode 2` capture match (194 checks). Removing any single draw from any view is detected (21,632 of 21,632), as is removing any view |
| Per-draw fixtures captured | `render.draw-state` (`draw_state_diff.py --exact`) | the same shots' per-draw state; seeded one-field changes in single draws are detected (every state field) |
| Budgets recorded | `render.budgets` (`tools/quality/render_budgets.py check`) | desktop (Radeon 8060S) and Fold7 (Adreno 840) rows with p50, p95, p99, GPU render and main-thread submission limits and k0_records; both inside their limits |

Declared gap: Portal 2's client compiles monitor rendering out
(`USE_MONITORS` is defined only for the HL2 and CS:S clients), so the monitor
view is captured on the two Portal maps only. Retail Portal 2 renders
monitors; that parity gap is outside K0. Closed 2026-09-29: the Portal 2
client defines `USE_MONITORS`, and `corpus.portal2.monitors` checks the
Wheatley monitors of `sp_a4_tb_intro` and `sp_a4_intro`. The K0 captures are
unchanged, because `sp_a1_wakeup` has no `point_camera`, so `DrawMonitors`
returns before drawing.

Determinism: the oracles need `-deterministicrender` (new, off by default,
plus `host_framerate 0.015` and `-nosound`). It seeds particles from their
definition rather than the address and clock, orders world batches and
material combos by stable keys instead of pointers, and implies
`-random_invariant`. With the flag off the frame is unchanged: a default boot
still runs 208 frames with 0 stage-order violations.

Budgets:
- The desktop row is measured through SDL's offscreen driver. A private
  headless mutter Wayland session paces presents at about 18 Hz (55 ms
  intervals with 5 ms of CPU), so it measures the compositor, and the user's
  own Wayland session is not used. Warm pass: p50 4.43, p95 5.12, p99 6.44,
  GPU render p99 1.69, submission p99 3.99 ms, 0 hitches.
- Fold7 (the worktree's APK, `frame_pacing_device.py --platform android`,
  new): p50 16.12, p95 23.79, p99 28.74, GPU render p99 18.10, submission
  p99 14.82 ms, 0 hitches. The first limits (p95 20, p99 25, GPU render p99
  16.7, submission p99 12 ms) were missed; at the user's direction ("You can
  adjust the limits") they are now p95 25, p99 33.3, GPU render p99 20 and
  submission p99 16.7 ms. The first definition is in git history.
- Later gates compare against these records through the frame allowance
  (warm median at most 1.05x, p99 at most 1.10x): `render_budgets.py report`.

Also fixed on the way: the strict environment kept Android's `-llog` in the
compile flags, an unused-argument error under `-Werror`, which the first
strict module configured on Android (now `jobs.graph`) exposed.

Reproduce:

```sh
python3 tools/quality/conformance.py check --suite shader.toolchain-pin \
  --suite shader.toolchain-pin.sensitivity --suite legacy.render-abi \
  --suite legacy.render-abi.sensitivity --suite legacy.render-abi.table
python3 tools/render/view_oracle.py suite --check both --runtime ../source-engine/run/runtime \
  --build build-rc-vo/install --p2-build build-rc-p2/install --p2-runtime build-rc-p2/p2content --out <dir>
python3 tools/render/view_oracle.py selftest
python3 tools/quality/render_budgets.py check
tools/quality/frame_pacing_device.py --platform android --device <serial> \
  --budget-row android-fold7-portal-frame-pacing --out <dir>
```

Not claimed: a hosted CI lane for any of these; Apple rows of the view
oracles; the loader inventory cannot be re-verified inside this worktree
(its submodules are symlinks the scanner skips; the drift is only there).

## K2, K5, K6 and K3/K9 tooling: core slices (2026-09-28)

Scope: the user's goal is "complete K0-K9 of the render-core RFC". These
slices are the gate checks that can be met in the core and on the device
port before the product inversion (K3). None of K2, K5 or K6 is closed.
Each table lists what passes and what remains.

### K2: render graph (`b76e6ea9`)

| K2 check | Evidence | Result |
| --- | --- | --- |
| Graph suite | `render.graph.v1` on the null adapter: G1–G10 (31 checks, g++ and clang++), with a TSan lane `render.graph.v1.tsan`. `render.graph.v1.vulkan` (`b72d7bf9`): the 1,000 random graphs, with real work for their writes, run serially and pooled on RADV (4,911 encoders, 467 pooled transients reused), and the validation layer with synchronization validation reports nothing | pass on null and Vulkan |
| Independent model agrees | G7: the reference model in `graph_fixtures.h` (fixpoint culling, first-fit slots, transitions per state owner) agrees on kept passes, transitions and alias sets on 1,000 seeded graphs; 208 of them share a physical transient | pass |
| Bad graphs caught | `render.graph.v1.sensitivity`: a missing transition (876 graphs), overlapping aliases (599), a culled side effect (787), a reordered dependency (680), and an undefined read (401) are each reported with their kind, every time | pass (5 of 5) |
| Serial equals pooled | G9: `PooledGraphExecutor` (one encoder per pass as `jobs.graph` jobs, `ParallelExecutor(4)`) records the serial executor's stream on the 1,000 graphs; `render.graph.recording` in the RFC's table is this clause | pass |
| Synchronization validated | the graph's compute and copy passes run on the Vulkan adapter under the validation layer in `render.skinning` and `render.opaque` with no message | partial: pixel families and a boot run are open |
| Pixels unchanged | — | open (product passes) |

Also delivered: physical aliasing of same-shape transients with disjoint
lifetimes, `ValidateCompiledGraph`, and `TransientPool`; the renderer keeps
its pool across frames, so later frames create no transients. Port rules
D14 (encoders recorded concurrently, one thread each) and the D12 refinement
(usage state advances at accepted submission) are in the shared device
suite; the Vulkan adapter passes both (agent report, `547b8f07`).

Open for K2: the graph suite's Vulkan lane; present blit, gamma, MSAA
resolve, scene capture and queued compute as graph passes in the product
(they live in `vulkan_device.cpp`, which K1 is refactoring); sync validation
over the pixel families and a boot.

### K5: scene (`2bb556bd`, `d9a1c8a8`)

| K5 check | Evidence | Result |
| --- | --- | --- |
| Scene suite | `render.scene.v1` (17 checks); `render.scene.publication` (6): three readers see only complete snapshots, in order, while the owner commits 2,000 change sets; its TSan lane is clean, and `render.scene.publication.tsan.sensitivity` (publication without the release/acquire edge) fails with a data race, exit 66 | pass |
| Serial equals pooled | C6: `BuildDrawListPooled` (frustum chunks as `jobs.graph` jobs) equals `BuildDrawList` exactly on 1,000 seeded scenes, views and chunk sizes (73,105 draws), with and without a provider | pass |
| Two scenes | `render.opaque` on RADV: two scenes drawn by `render.pass.opaque` into their own targets share no content, and destroying one leaves the other's frame byte-identical | pass (synthetic scenes) |
| Culling matches, Pixels, Submission cost | — | open (need the engine's scene: world groups and props) |

`render.pass.opaque` is the first native drawing feature. It draws a draw
list with a depth test, and the near-wins check fails when the depth test
is disabled. It stands in material colors for K4's families.

### K6: skinning (`861dc553`)

| K6 check | Evidence | Result |
| --- | --- | --- |
| GPU equals CPU skinning | `render.skinning` (GPU): `skin.comp` as a graph compute pass against `SkinReference` (the native emit skinning's portable form: three bones, stereo flex) on 64 seeded meshes. Worst errors: position 1.2e-4 units, normal 3.6e-7, tangent 4.8e-7. `render.skinning.corpus`: studiorender's own software skinning, captured on `testchmb_a_00`, `_a_01`, `_a_08`, `escape_00` and Portal 2's `sp_a1_wakeup`, compacted to 178 meshes and 305,078 vertices (3,993 with two bones, 117 with three). GPU position 1.46e-3 units, 0.75 of tolerance version 2; normal 1.8e-7, tangent 2.4e-7; 0 meshes over | pass. Open: flexed vertices (none reached the software path) and the full Portal 2 character set (one map so far) |
| Defects caught | the seeded bone-index and flex-weight kernels miss by 1.1e3 and 7.7 units on synthetic meshes; on the corpus's 12 blended meshes the bone-index kernel misses by 2.31 units (2,314 times its tolerance) | pass (2 of 2) |
| Oracle | `render.skinning.reference` (headless): the oracle agrees with an independent double-precision matrix-blend reference on 200 meshes | pass |
| Pixels, CPU skinning retired, Cost recorded | — | open (product path) |

### K3 and K9 static scans (`842bb152`)

`tools/render/retirement_scans.py` holds the static halves of four gate
checks:

- `render.side-channels` (K3): 14 lookup sites and the capability queue
  remain. Recorded as an expected failure.
- `render.record-replay` (K3): 3 callers of `CVulkanContext::BeginFrame`.
  Recorded as an expected failure.
- `render.legacy-stream.ratchet` (K9): exact, shrink-only, currently 490
  first-party sites in 145 files. Passes.
- `render.legacy-shaders.deleted` (K9): 46 files. Recorded as an expected
  failure.

The self-test detects 8 of 8 seeded faults.

Build fix on the way: the launcher names `jobsystem` directly. Waf first
reached it through a shared library in the launcher's use list and then
left it out of the static link.

Reproduce:

```sh
python3 tools/quality/conformance.py check --suite render.graph.v1 --suite render.graph.v1.sensitivity \
  --suite render.scene.v1 --suite render.scene.publication --suite render.opaque.null \
  --suite render.skinning.reference --suite render.frame.v1 --suite render.composition
python3 tools/quality/conformance.py check --suite render.skinning --suite render.opaque   # GPU runner
CONFORMANCE_TSAN=1 python3 tools/quality/conformance.py check --cxx clang++ \
  --suite render.graph.v1.tsan --suite render.scene.publication.tsan \
  --suite render.scene.publication.tsan.sensitivity
python3 tools/quality/conformance.py check --suite render.side-channels --suite render.record-replay \
  --suite render.legacy-stream.ratchet --suite render.legacy-shaders.deleted \
  --suite render.retirement-scans.sensitivity
```

K6 corpus (2026-09-28): `studiorender/skin_capture.cpp` records the
software path's input and output only when `SOURCE_SKIN_CAPTURE` names a
file. It is a diagnostic hook with no effect otherwise. The capture uses
private copies of the installed client and Portal 2 builds with only
`libstudiorender.so` replaced:

```sh
python3 tools/render/skin_corpus.py capture --build <install> --runtime ../source-engine/run/runtime --out <dir>
python3 tools/render/skin_corpus.py capture --game portal2 --build <p2 install> --runtime build-rc-p2/p2content --out <dir>
python3 tools/render/skin_corpus.py compact <dir>/*.skcap --out <dir>/k0.skcorpus
RENDER_SKIN_CORPUS=<dir>/k0.skcorpus python3 tools/quality/conformance.py check --suite render.skinning.corpus
```

The first corpus failed a flat 1e-3 bound on fp32 rounding at about 7,000
units, where the legacy path's matrix blend and the kernel's point blend
round differently by up to 3 ulp. The K6 position tolerance is now version 2:
the larger of 1e-3 units and 4 ulp. The RFC's K6 table records the
amendment.

## K4: shader library and materials, groundwork (2026-09-28)

Commits: `9e1e11a8`, `435d1f3a`, `1b9da074` and `f83b7416`. The work was
done by a subagent under the K0–K9 goal. K4 stays open until its family
ports have pixel oracles, which run in the product after K3.

| K4 check | Evidence | Result |
| --- | --- | --- |
| Material suite | `render.material.v2`: 69 checks covering families, the key-mapping oracle, derived-copy revisions, import, patches, PBR and apply. `.sensitivity` has 7 checks and catches a key mapped to the wrong parameter, a key read as the wrong kind, and two kinds of stale GPU copy | pass |
| VMT corpus | `render.material.vmt-corpus` (18) and `.sensitivity` (7) over the retail VPKs. Portal: 6,000 VMTs, of which 2,253 are lightmapped, 1,547 vertexlit, 1,384 unlit, 784 legacy and 32 unsupported (31 unknown debug shaders and 1 malformed file). Portal 2: 3,738 VMTs, of which 829 are lightmapped, 1,211 vertexlit, 1,144 unlit, 541 legacy and 13 unsupported (12 unknown shaders and 1 missing include). No crash; the `.asan` lane passes on clang | pass; counts in `quality/fixtures/render-material/vmt-corpus-v1.json` |
| Families match ports | — | open (family ports, pixel oracles after K3) |
| Proxy corpus | `render.material.proxies.inventory`: Portal registers 66 proxies, Portal 2 registers 69. Legacy captures come from the new `mat_proxy_capture` client command, repeat exactly, and the comparator catches 13 of 13 seeded differences | legacy side recorded; the frontend-side comparison is open (after K3) |
| Bind-group ceiling | `render.shader-artifacts`: reflection is checked against `render/shaders/layouts.json`, and a fifth group fails, as does a seeded layout mismatch in the Waf build | pass for passes; no family is declared yet |
| Artifacts per target | `render.shader-artifacts` (1,259 checks) and `.sensitivity` (13). The pinned SPIRV-Cross `vulkan-sdk-1.4.357.0` turns 204 backend shaders into 403 SPIR-V and GLSL 4.50 artifacts, and the artifact tool reproduces the committed `*_spv.h` headers byte for byte. Five GLSL 4.50 targets are declared exclusions: the ray-query probe trace and the two seven-set passes | partial: the committed headers are not deleted yet (the backend wscript switch comes after K1) |

Port gap found: every backend shader uses combined image-samplers and push
constants, and `render.device.v2` has neither. Decision (agent, under the
user's standing instruction, 2026-09-28):
- Family shaders are written for the port, with separate texture and sampler
  bindings, which GLSL 4.50 and SPIR-V both express. The port does not grow
  combined samplers.
- The port gains per-draw constants as a core clause: a small block of at
  most 128 bytes, the Vulkan minimum for push constants, implemented as push
  constants on Vulkan and a uniform block on OpenGL. A per-draw bind group
  per draw is the alternative, and it would cost a descriptor allocation per
  draw.
- The legacy ports keep their layouts until the frontend (K3) owns them.
- The two seven-set passes, skin/$phong and screenspace post, are regrouped
  under four groups when their families move.

## K7 headless slice: clustered light assignment and shadow atlas (2026-09-28)

State: the headless, oracle-first part of K7 passes. No K7 gate check is
closed: the gate's pixel, flashlight, behavior-decision and budget checks
remain (below). Commit `4b25e176`.

### What exists

- `render.pass.lights` (`public/render/pass/lights/clusters.h`,
  `render/pass/lights/`): a froxel grid per view (pixel tiles, depth slices
  spaced logarithmically between near and far, per-profile capacities;
  `DesktopClusterLimits`, `MobileClusterLimits`) and `AssignLights`, the
  serial path. It takes the point and spot lights of `render.light-set.v1`
  (the only runtime light list; spark lights arrive as its dlights), culls
  conservatively (tile planes, slice slab, the froxel's box, a cone test on
  the froxel's bounding sphere) and writes one offset and count per froxel
  into one index list. Capacity is explicit: lights beyond `maxLights`, per
  froxel and in the index list are kept in light order and counted in
  `ClusterStats`, or `OverflowPolicy::kFail` fails with the counts and
  leaves the output unchanged. Directional and invalid lights are counted,
  never listed; radius 0 is unbounded.
- `render/pass/lights/cluster_assign.comp`: the compute pass, one invocation
  per froxel running the same tests over `PackClusterLights` records into
  `PackClusterGrid`'s layout. It compiles with the pinned glslc (shaderc
  v2026.1). It is left as source: no SPIR-V is committed and the artifact
  task does not build it until the pass is wired into a product, so the
  shader-artifact pipeline is untouched. No device lane compares it with the
  serial path yet.
- `render.pass.shadows` (`atlas.h`, `shadow_views.h`, `render/pass/shadows/`):
  `PlanShadowAtlas`, a deterministic buddy allocator (power-of-two tiles
  aligned to their size, ranked by priority, wanted size and key, reduced
  down to the minimum when space runs short, a caster budget, a status for
  every request); spot and flashlight shadow views; a tile transform applied
  after the perspective divide; `PracticalSplits` and `BuildCascades`
  (orthographic boxes around each split slice's bounding sphere, a radius
  fixed by the projection and splits, moved in whole texels of a lattice
  fixed in the world, extended toward the light by a caster distance).
- Wiring: both modules in `render/wscript`, strict static libraries with
  `arch_module`, rows in `architecture/modules.json` (layer 6 through the
  `render.pass.*` entry; `render.core-tests` may reach them), `cxx20` in
  `quality/toolchain/policy.json`, four manifest rows (R90), contracts
  `render.lights.v1.md` and `render.shadows.v1.md`.

### Evidence

| Suite | Checks | Result |
| --- | --- | --- |
| `render.lights.clusters` | 37 | zero false negatives over 1,000 seeded scenes (805,019 froxels, 125,204 lights, 17.5 million reached pairs at the default seed); false-positive rates points 0.0047 and spots 0.168 (ceilings 0.02 and 0.25; 0.162 to 0.179 over seeds 1 to 5); 64,000 lookup samples with no froxel outside its point and no light missing; exact overflow accounting over 200 scenes under both policies. Clean on 20 further seeds |
| `render.lights.clusters.sensitivity` | 10 | 9 of 9 seeded defects detected: slice boundaries off by one, a range-squared sphere test, rows mirrored, columns off by one, spot half-angle halved (false negatives) and doubled (false-positive ceiling), near-plane culling, unbounded lights dropped, uncounted capacity losses; the real builder clean on the same scenes |
| `render.shadows.atlas` | 25 | 1,000 seeded atlas plans (tiles in bounds, aligned, disjoint; budget and shortages proven; order-independent), 1,000 spot and 1,000 flashlight views, 1,000 cascade cameras (splits, gap-free coverage with casters), 250 snapping cameras. Clean on 150 seeds |
| `render.shadows.atlas.sensitivity` | 15 | 14 of 14 seeded defects detected: overlapping and out-of-bounds tiles, the budget ignored or unreported, priority ignored, a spot frustum from half its cone, reversed depth, a projector without a depth test, a flashlight with clip Y down, shifted, unsnapped and shrunken cascades, the caster distance and lambda ignored |

All four pass through `tools/quality/conformance.py check` with g++ and
clang++ (default and release). `archlint check --all` passes; both targets
build strict under Waf (`build-rc-tests`); `stylelint` is clean on the new
sources.

The reference (`unittests/rendertest/core/pass/lights/cluster_oracle.h`)
shares no code with the module: froxels are rebuilt in double from the
projection, point reach is the exact distance to the froxel's hexahedron,
and spot reach is an exact rejection (a separating plane through the apex,
or all corners outside a cone wider than a hemisphere) followed by a
witness search (points of the froxel in the spot, and rays of the spot
entering the froxel, aimed at its corners, faces, center and nearest point
and over a 145-direction lattice). Only witnessed pairs count as reached, so
a reported false negative is real; spot pairs left without a witness are
counted apart (115,423 at the default seed, inside the false-positive rate).

### Decisions (agent, under the user's standing instruction)

- Light records are transformed to view space in double on the CPU, and the
  compute pass reads view-space records. Transforming in float cost false
  negatives near a camera far from the origin (float error grows with the
  world coordinates, not the view distance); covering it with slack tripled
  the false-positive rate. The float slack left is 4e-6 of the view-space
  magnitudes.
- The atlas tile offset is applied after the perspective divide. Folded into
  a world matrix it multiplied the view's translation and moved points by 5
  percent of a small tile far from the atlas's origin.
- Spots wider than 85 degrees fail with `kConeTooWide`: they need the
  optional point-light cube shadows, which are not in this slice.
- The pooled (per-slice `jobs.graph`) assignment was left out: the serial
  path is the oracle, and the GPU pass is the production path.
- The pure CPU parts live in `clusters.h`, `atlas.h` and `shadow_views.h`;
  A.1's `feature.h` for each module is kept for the `IRenderFeature` that
  wires them into the graph.

### K7 checks still open

- Pixel oracles (`render.shadows`): a caster darkens its receiver, a
  non-caster does not, cascade transitions within tolerance of a
  single-cascade reference.
- The Portal flashlight scene on native and the `SetFlashlightState` census
  (and R32-VIDEO-OPTIONS P7).
- The dlight behavior decision: per-pixel and legacy dlight modes against
  their references, and the switch.
- Product wiring: the two features in the graph, the froxel lists in the
  view bind group, a device lane that runs `cluster_assign.comp` against the
  serial path, the light-set publisher feeding the pass.
- Budgets: `render-v1.json` atlas rows on desktop and the Fold7; the profile
  limits here are provisional until then. No Fold7 run.
- Point-light cube shadows and tile reuse across frames.

Reproduce:

```sh
python3 tools/quality/conformance.py check --cxx g++ --suite render.lights.clusters.gpu \
  --suite render.shadows.atlas \
  --suite render.shadows.atlas.sensitivity
dependencies/shader-toolchain/bin/glslc --target-env=vulkan1.1 -O \
  render/pass/lights/cluster_assign.comp -o /tmp/cluster_assign.spv
```

## Dynamic lights through open portals (2026-09-28, `a8578480`)

The user asked for dynamic lights through portals, at least one kind, as a
reason for this work, and allowed a test map. This is done for dlights on
the lightmapped world and on models, in every renderer. It is part of K7
(R90).

- Contract: `render.portal-lights.v1` (`public/render/portal_lights.h`, in
  render.contracts), header-only and shared by the engine now and by the
  core's clustered lights later.
  - A light in front of an open portal whose radius reaches its opening makes
    an image: the light moved by the pair's transform, from RFC 0011 G10's
    portal set.
  - A receiver on the other side is lit only when the path from the light
    passes through the opening. This is decided in the entry portal's frame.
- Engine (`engine/portal_dlights.cpp`): each frame, before dlights are
  marked, images go into free dlight slots.
  - World lightmaps clip per luxel, flat and bumped, with brush models taken
    through their transform. Models clip at their lighting origin.
  - Displacements and the WMSH PBR per-pixel light set leave images out, so
    light never leaks behind a wall.
  - `r_portal_dlights 0` turns the feature off. `r_portal_dlights_report`
    reports images and slot shortfalls. `r_portal_dlight_test` (a cheat)
    places a test light.
- Evidence:
  - `render.portal-lights` (headless, 7 checks): 0 mismatches against an
    exit-frame reference on 99,968 samples. The seeded forward-transform and
    no-aperture defects fail.
  - `render.portal-lights.lab` (14 checks, on the `portal_dlight_lab` test
    map: two sealed rooms joined only by a portal pair):
    - the floor beyond the exit brightens by 11.7 levels;
    - an in-reach point outside the opening and the wall around the exit
      move by less than 0.5;
    - nothing changes with the feature off or a portal closed;
    - the source lights its own room by 28.7 in every lit state.
  - The seeded no-clip run fails the leak check (+6.7).
  - `testchmb_a_01` still boots.
- Reproduce: `python3 tools/quality/portal_dlight_lab.py build`, then
  `... check --out <dir>` (and `--seed noclip`). The map is published to the
  main tree's `./play portal_dlight_lab`.
- Not yet:
  - elights (model-only lights);
  - displacements and the WMSH PBR per-pixel path, which need a clip in
    their consumers;
  - ~~Portal 2, whose client does not publish the portal set yet~~ (done
    2026-09-29, below);
  - recursive paths (one hop only);
  - shadowing along the path. Lightmap dlights cast no shadows; K7's
    shadow atlas is the path to that.

### Portal 2 publishes its portals (2026-09-29, user request)

The user asked to close the gap above for Portal 2. Lights now pass
through Portal 2's portals as they do in Portal 1.

- Client: `C_Portal_Base2D` publishes its open pairs every frame
  (`game/client/portal2/portal/c_portal_base2d.cpp`), as Portal 1's
  `C_Prop_Portal` does, so the engine's images, RFC 0011 G10's producers
  and the radiosity links see Portal 2's portals.
- Glow (contract clause P6): Portal 2's open portals each light their
  surroundings with a point dlight in the portal's color
  (`r_portal_use_dlights`, on here, off in retail). Imaged through its own
  pair, each glow would tint the other portal's side in its color. The new
  `DLIGHT_NO_PORTAL_IMAGE` flag (`public/dlight.h`) keeps a light out of the
  images; the glow sets it.
- Found on the way: Portal 2 places a portal (origin, angles, link
  transform) only when it is activated by input or shot, as its maps open
  theirs; a `prop_portal` spawned `Activated 1` stays at the world origin
  on client and server. The Portal 2 lab run opens the portals with
  `SetActivatedState 1`. The server gap is recorded, not changed: retail
  maps do not spawn portals open.
  - Closed 2026-09-29: `CPortal_Base2D::Spawn` caches its spawn transform
    in `m_ptOrigin`/`m_qAbsAngle`, so a portal spawned `Activated 1` is
    where the map puts it (`gi_portal_light` in Portal 2; `portal_report`;
    RFC 0011 progress). The lab still opens its portals by input, and
    `render.portal-lights.lab.portal2` passes 16 of 16 on the fixed
    `build-p2`.
- Evidence (`build-p2`, native Vulkan, headless):
  - `render.portal-lights.lab.portal2` (16 checks, `--game portal2`):
    through the portal the floor brightens by 11.57 levels (Portal 1:
    11.67), room A by 27.4; the out-of-opening point and the wall stay at
    0.0 in every state. The engine reports 1 image through 2 open portals
    with the test light, and 0 through 2 with only the glows lit. The glow
    is off and exposure fixed (`mat_force_tonemap_scale 2`) for the pixel
    views: Portal 2's auto exposure was still adapting at the first views
    and moved unlit points by up to 4.6 levels.
  - `render.portal-lights.lab.portal2.seeded-glowimage`: with
    `r_portal_dlights_seed_glowimage` the engine reports 2 images and
    `portal2.glow-not-imaged` fails, as required.
  - `--seed noclip` in Portal 2 fails the leak check (+6.77).
  - Portal 1 unchanged on `build`: 14 of 14, through-portal 11.67, the
    noclip seed +6.70.
- Not changed: the glow is still a point dlight, not an area light (a
  separate step, not scheduled), and area lights are still not imaged.

## K1: device port and the Vulkan and null adapters (2026-09-28)

State: every K1 check passes on the Linux desktop except desktop frame
time, which a loaded host has kept from a clean measurement, and the Fold7
rows, which wait for the device. A subagent did the work under the K0–K9
goal (commits `fe58b0a0`, `614a50e2`, `547b8f07`, `180813c8`, `4d8a7cec`,
`35014b31`). The user stopped it at its last step, and this record was
written from its reports and re-run checks. D16 (`694b0212`) was added
after it.

| K1 check | Evidence | Result |
| --- | --- | --- |
| Port suite | `render.device.v2` on null (headless) and Vulkan (RADV; 652 checks with D16), g++ and clang++. On the Fold7's Adreno 840, cross-compiled with NDK r30 and run from `/data/local/tmp` with no screen, it passed 393 checks before D16 | pass (the Fold7 run predates D16) |
| Bad adapters | `render.device.v2.sensitivity` (null): 11 of 11 with D16's zero-fill adapter. `render.device.v2.vulkan.sensitivity`: flipped Y, a −1..1 depth range, false async compute and false aliasing each fail only their clause | pass |
| Port is backend-neutral | CAP007/CAP005/CAP011, archlint clean on the tree | pass |
| One Vulkan stack | `render.vulkan.allocation-sites` (67 checks) and its seeded self-test. `CVulkanContext` borrows its device, queues, VMA allocator and timeline from the adapter (`host_device.h`), and allocates, uploads and retires through it | pass |
| No idle waits on frame paths | `render.vulkan.idle-waits` (107 checks): every site is in the reviewed list (teardown, mode change, loss recovery); a seeded site fails | pass |
| Pixels unchanged | `material_pixel_conformance.py`: all 17 families in both HDR modes plus the forced cases, byte-identical | pass |
| Boots and resize | `portal_boot.py` on the four maps in both queued modes (8 of 8); `--resize-stress` on Wayland and X11, queued and sync | pass |
| Feature support recorded | `render.device.vulkan-features`: desktop and the Fold7 (timeline, synchronization2, dynamic rendering) | pass (the iPhone and Apple TV are optional and not run) |
| Frame time | `frame_pacing.py` on desktop: the absolute budget failed while host load was 20–40 from other agents' Blender and boot runs. Under the same load, an interleaved A/B against the pre-K1 build had the old build 1.136× slower at the median, so this is not a regression. Fold7: not run, because the device was folded and locked | open: a quiet-host desktop run and the Fold7 run remain |

View oracles also pass (194 of 194), as do static composition and the
Android APK build.

To close K1: the desktop frame pacing on a quiet host
(`render_budgets.py report --row linux-wayland-portal-frame-pacing`), and
the Fold7 frame pacing (`frame_pacing_device.py --platform android`, with
`adb install -r` only, never uninstall), once the device is unlocked.

## K7 GPU slice: cluster assignment on the device and shadow pixel oracles (2026-09-28)

State: K7's "Shadow oracles" check passes on native Vulkan desktop (RADV,
Strix Halo), and the light assignment compute pass has its device lane. K7
is not closed: the flashlight scene, the dlight decision and the budgets
remain (below). Commit `fa4e32be`.

### What exists

- `render.pass.lights` (`public/render/pass/lights/cluster_pass.h`,
  `render/pass/lights/cluster_pass.cpp`): `ClusterKernel` (like
  `SkinningKernel`), `PrepareClusterDispatch`, `CreateClusterResources`,
  `AddClusterUploadPass` and `AddClusterAssignPass`. `cluster_assign.comp`
  now binds one dispatch-local group (the draw role), writes light-set
  indices (a new binding maps packed lights to them), and follows the serial
  path's order of operations with every result `precise`. Its SPIR-V is
  committed in `cluster_assign_spv.h`.
- `render.pass.shadows` (`public/render/pass/shadows/shadow_passes.h`,
  `render/pass/shadows/shadow_passes.cpp`):
  - `ShadowDepthRenderer`: one copy pass of clip matrices, one per (view,
    caster) pair and composed in double; then one depth-only pass (no
    fragment stage, depth test less, no culling) that clears the atlas and
    draws each view in its tile viewport;
  - the receiver helper `shadow_sample.glsl` with its record `ShadowTileGpu`
    (`PackShadowTile`): a 2x2 bilinear percentage-closer filter with taps
    clamped into the tile viewport, the tile transform after the divide, and
    points without shadow information lit;
  - `ShadowReceiverRenderer`: draws receivers lit by a spot or by the sun's
    cascades (selected by view distance) and writes the light per pixel.
    It is the oracles' pass, and it can serve as a shadow mask.
  - The shaders are committed in `shadow_spv.h`. `feature.h` stays reserved
    for the `IRenderFeature`.
- Wiring:
  - `render.graph` edges for both modules and `render.resources` for
    shadows, in `architecture/modules.json`, with the targets' `allowedUse`;
  - two manifest rows (R90, profile `linux-native-vulkan-gpu`);
  - eight `EMBEDDED` rows in `tools/render/shader_toolchain.py`, which now
    has 58 checks;
  - the contracts `render.lights.v1.md` (G1–G5) and `render.shadows.v1.md`
    (P1–P6, C1–C3).

### Evidence

| Suite | Checks | Result |
| --- | --- | --- |
| `render.lights.clusters.gpu` | 24 | G1: over 300 seeded scenes (260,340 froxels, 38,932 lights, 6.3 million serial assignments at the default seed, plus an empty and a 256-light scene), each froxel's list equals `AssignLights`' list exactly: 0 mismatches and 0 boundary pairs. The same over seeds 1 to 5 (1,800 scenes, about 35 million assignments). G2: zero false negatives against the independent reference on 100 scenes (1.9 million reached pairs; false-positive rates: points 0.0042, spots 0.148). G3: per-froxel limits on 60 scenes (43,636 froxels overflowed) give the same prefixes and counters. G4: index capacity on 40 scenes (31,305 overflowed) gives serial prefixes, fills exactly, and the totals match. G5: 3 of 3 seeded kernels rejected (slice off by one: 111,566 mismatches over 40 scenes; cone ignored: 396,630; uncounted overflow: counters wrong in 39 of 40 scenes) |
| `render.shadows.pixels` | 26 | Spot (the spot's tile at (1024, 0), after two other lights' tiles): depth only inside the three views' viewports (0 texels outside); the judged pixels (309 shadowed, 179 shadowed only by the non-caster, 13,956 lit, 46,059 outside the cone) are all correct. Seeded defects: depth reversed (all 14,444 in-cone judged pixels wrong), tile offset by one tile (309 of 309 shadowed pixels lit), a caster left out (259 of 309 lit). Sun: four cascades (splits 4.4, 10.4, 23.5 and 60; texels 0.0067 to 0.091) each shade judged pixels (12,980, 13,151, 5,339 and 2,019); 0 of 32,737 pixels chose the wrong cascade; the cascaded and single-cascade (2048) renders each match the oracle, and agree on 33,489 of 33,489 judged pixels (tolerance 99.8 percent). Snapping: a 2.37-texel camera move shifts the depth image by exactly 2 texels, with 0 texels differing (tolerance 1e-4); unsnapped, 32 percent of compared texels differ |

- Both suites pass through `tools/quality/conformance.py check` with g++ and
  clang++ (default), and with clang++ in release. The headless K7 suites
  still pass. Both runs report no validation-layer message (synchronization
  validation included).
- `shader_toolchain.py check` passes 58 of 58, and `archlint check --all`
  passes.
- `render_pass_lights` and `render_pass_shadows` build strict under Waf
  (`build-rc-tests`), and `stylelint` is clean on the new C++ files.

The shadow oracle is a CPU ray test in double against the caster boxes. A
pixel is judged only when its class (outside the cone, shadowed by a caster,
shadowed only by a non-caster, lit) holds over a disc on the ground. The
disc's radius is two pixel footprints plus three shadow texels projected
along the light. An earlier ring-only test judged a pixel 0.045 units from a
shadow corner, inside a ring of radius 0.32; the disc (four rings of 32
points) replaced it.

### Decisions (agent, under the user's standing instruction)

- The kernel writes light-set indices, not packed indices, so its output
  equals the serial path's and shading indexes the light set directly.
- The GPU and CPU lists are compared exactly. Pairs within 1e-4 of the
  decision (radius and half-angle) are excused as boundary pairs under a
  ceiling of 1e-5 of the assignments. None has occurred on RADV; the
  allowance is for drivers whose square roots round differently.
- Under index-capacity overflow, a froxel that found no room keeps its
  (unspecified) offset past the capacity with count 0. Only the totals are
  defined.
- The shadow depth compare is manual (the port has no comparison sampler),
  and the bias is the receiver's (the port has no depth bias state). The
  PCF is 2x2 bilinear with taps clamped into the tile viewport.
- The seeded "tile offset wrong" and "unsnapped cascade" defects are built
  in the suite (a shifted tile record, a view centered on the sphere), like
  the headless bad providers. Only the reversed compare is a shader variant.

### Port findings (for the device port's owner)

- No comparison sampler (`SamplerDesc` has no compare op). The compare runs
  in GLSL at four point taps where hardware PCF would take one.
- No depth bias in `RasterState` (constant and slope-scaled). This slice
  needs none, because its receivers are not casters, but self-shadowing
  receivers will.
- `SetViewport` state outlives `BeginRendering` on an encoder: a receiver
  pass recorded after the depth pass on the serial executor's encoder drew
  into the last tile's viewport. The receiver pass now sets its viewport.
  Either `BeginRendering` should reset the viewport to the render area, or
  the contract should say that it persists.
- No scissor. The tile viewport is enough for depth (clipping keeps
  fragments inside), and P2 checks it.

### K7 checks still open

- Flashlight: the Portal flashlight scene on native and the
  `SetFlashlightState` census (and R32-VIDEO-OPTIONS P7).
- The dlight behavior decision (per-pixel versus legacy modes, each
  against its reference, and the switch).
- Budgets: `render-v1.json` atlas and clustering rows on desktop and the
  Fold7. The profile limits are provisional, and there is no Fold7 or Apple
  run of either suite.
- Product wiring:
  - both features as `IRenderFeature`s in the frame graph, with the froxel
    lists in the families' view bind group;
  - the light-set publisher feeding the pass;
  - families (K4) including `shadow_sample.glsl`;
  - tile reuse across frames, and point-light cube shadows.
- Runtime-light ownership (agreed with source-engine-70, 2026-09-28): the
  render core owns the split between CPU-baked runtime light and per-pixel
  light, following RFC 0011's indirect policy.
  - `IRenderCoreWorld::RuntimeLight(surface)` returns
    `RenderCoreRuntimeLight` flags (kAreaLights, kProjectedLights,
    kWorldLightOcclusion). It is snapshotted once per frame before
    R_BuildLightMapGuts.
  - It returns 0 until the clustered pass evaluates that light. The same
    change sets the bits, and R_AddAreaLights, R_AddProjectedLights and
    R_ApplyDynamicOcclusion skip those surfaces.
  - Oracle: one area light's radiance on a surface agrees with the bits set
    and unset. A control with both paths forced must fail as doubled light.

Reproduce:

```sh
python3 tools/quality/conformance.py check --cxx g++ --suite render.lights.clusters.gpu \
  --suite render.shadows.pixels
CONFORMANCE_SEED=3 build/quality/render_lights_clusters_gpu.default
python3 tools/render/shader_toolchain.py check
```

## K3: inversion, slice 1: frames recorded into the core's frame graph (2026-09-28, `ded9e2f6`)

State: K3 is open. This slice moves the native Vulkan frame's recording and
submission into the render core.

- **Old path removed:** `CVulkanContext::BeginFrame`/`EndFrame` and the
  context's own frame command buffers are gone.
- **The frame source:** at present the backend hands the legacy frontend's
  executor an `ILegacyFrameSource` (`public/render/legacy/frame_source.h`).
  The launcher binds the executor through
  `NativeVulkanShaderBackend_BindFrameExecutor`.
- **The frame graph:** the executor runs the frame as a render.graph
  execution on the adapter. The frame's commands (compute, uploads, the
  recorded stream, capture, present blit or gamma) are host work on the
  pass's port encoder, translated at `Submit` (`IHostDevice::RecordNative`),
  and the swapchain's acquire and present are binary semaphores on that
  submission.
- **Without an executor:** tests and harnesses use `RenderFrame`, the same
  host work on an encoder of its own.

| K3 check | Evidence | Result |
| --- | --- | --- |
| Pixels and views unchanged | 16 pixel families in both HDR modes: identical measurements before and after (32 of 32). Deterministic `testchmb_a_01` screenshots are byte-identical before and after, in queued modes 0 and 2. View oracles and draw-state not yet rerun | partial |
| Side channels gone | `render.side-channels` passes (slice 2, below) | pass |
| Record replay gone | `render.record-replay` (widened to any `BeginFrame`/`EndFrame` definition or caller): pass, and recorded as a pass in the manifest | pass |
| Products boot | the K1 set (four maps, both queued modes, 8 of 8); `--resize-stress` on isolated Wayland and X11, queued and sync (4 of 4). Portal 2 not yet run | partial |
| Threading | queued TSan lane not run | open |
| Frame time | as K1: host load and the Fold7 | open |

The frame is still one legacy pass. Splitting it by stage, and moving
present, gamma, MSAA resolve and capture onto passes of their own (K2's
product items), are the next slices; so is the side-channel removal.

Known: `render.device.v2.vulkan.sensitivity` does not build under clang
(an unused `kSampledFragment` in the sensitivity build); it passes under g++.

## K3 slice 2: the side channels leave the material system (2026-09-28)

State: K3's "Side channels gone" check passes. K3 stays open (below).

- **What moved:** the engine used to find the world mesh upload, the
  light set sink and the compute service by string (`WorldMeshUpload007`,
  `RenderLightSetConsumer001`, `RenderGpuCompute001`) on
  `IMaterialSystem::QueryInterface`. The material system's
  `render_capability_queue` adapters ordered them.
  - Now the legacy frontend owns them.
    `public/render/legacy/capabilities.h` declares
    `ILegacyCapabilities`, and `render/legacy/queued_capabilities.cpp`
    ports the adapters.
  - The frontend adopts the backend's services when the material system
    creates the backend through it. The engine gets them from
    `RenderCoreBinding::capabilities` (`RenderCoreHost_WorldMeshUpload`,
    `_LightSetConsumer`, `_GpuCompute`).
  - The five engine call sites use them: `gl_rsurf.cpp`,
    `indirect_light_host.cpp` (twice), `light_set_publisher.cpp` and
    `modelloader.cpp`.
- **Ordering unchanged:** the material system exports
  `MaterialSystem_RenderCallQueueHost()`, three C function pointers over
  its render call queue: whether the calling context queues, queue a call
  with a payload destructor, and the bound material. The launcher hands it
  to the core (`RenderCore_BindRenderCallQueue`). Calls still run on the
  owning thread in the order the engine made them, with copies of every
  borrowed byte. A queued `DrawBatch` still learns a rejection one frame
  late.
- **Deleted:** `materialsystem/render_capability_queue.{h,cpp}`, the three
  `QueryInterface` branches and the three interface-name constants.
  `public/render/world_mesh_upload.h` joins `render.contracts`. The Vulkan
  world-mesh bridge now reaches it as an owned edge instead of a legacy
  include.
- **Decision (agent, under the standing instruction):** `-norendercore`
  composes no core, so it has no capabilities. The world draws through the
  legacy brush path, and indirect light uses its CPU producers (gi_door
  selects `radiosity` instead of `sdf`). `-norendercore` rolls back the
  frame composition. It is not a way to keep the core's capabilities, and
  `public/engine/render_core_binding.h` says so.

| Check | Evidence | Result |
| --- | --- | --- |
| Side channels gone | `render.side-channels` (manifest row now `pass`) | pass |
| Adapter contract | `render.legacy-capabilities` (16 checks, g++ and clang++): direct without a queue; queued calls in order among context calls with copied bytes; flushed payloads released unrun; one-frame-late rejection; forgotten at the next upload. Seeded `BORROWED_BYTES` and `UNORDERED` each fail their clauses | pass |
| Capabilities reach the backend | gi_door (BSP2 with WMSH, LMAP, PRBV, RPRB, SDFV, RTRN) in queued modes 0 and 2: WMSH GPU buffers, LMAP, "WMSH draw path active", and the SDF producer on GPU compute | pass |
| Pixels unchanged | gi_door screenshots in modes 0 and 2 are byte-identical to each other and to the pre-K3 install (`build-rc-vo`, sha256 `4d7dc63e...`) | pass |
| Boots | testchmb_a_01 and escape_02 queued, and gi_door with `-norendercore` | pass |
| Builds | client, dedicated, tests, static composition and Portal 2 trees; archlint `check --all` and `baseline --verify` pass | pass |

`archlint inventory --verify` reports the loader inventory stale. The
cause is the symlinked box3d submodule's vendored sokol headers, not this
change; it is left as found.

K3 still open: view oracles and draw-state on the K3 build, the Portal 2
boot, the queued TSan lane, frame time (a quiet host and the Fold7), and
splitting the frame by stage (present, gamma, resolve and capture as
passes).

## K3 slice 3: stage passes and the queued TSan lane (2026-09-28)

State: K3's threading check passes on desktop, and the frame is now a
sequence of stage passes. K3 stays open for frame time (below).

- **Stage passes:** `ILegacyFrameSource` exposes its stages in order
  (`LegacyFrameStage`: compute-and-uploads, scene, resolve, capture,
  present). The executor adds a side-effect pass per stage the frame has:
  resolve only when multisampled, capture only for a back-buffer readback.
  The compiler keeps side-effect passes in declaration order, and the
  serial executor records them on one encoder, so the stages are one
  submission in order.
  - `CVulkanContext::RecordFrameCommands` is split into those stages
    (`AttachFrameStage`, `RecordFrameStage`). Only the present stage adds
    the acquire wait and the present signal. Debug labels the stream opens
    close at the end of their stage, so they nest inside the pass's label.
  - A product frame is 4 passes at 4x MSAA (compute-and-uploads, scene,
    resolve, present) and 5 with a screenshot. The backend reports the
    count once (`[vulkan] frames run in the render core's frame graph
    (RFC 0016 K3): 4 stage passes`).
  - Present, gamma, resolve and capture are passes of the frame graph.
    Their commands are still the backend's own, recorded as host work. The
    graph does not see their resources yet; that comes when the back
    buffer and swapchain image become port textures.
- **Queued TSan lane:** `tools/render/tsan_triage.py` boots a clang
  ThreadSanitizer tree's installed Portal product headless in queue modes
  0 and 2. It parses every report into a signature and fails on any
  signature outside the reviewed families in
  `tools/render/tsan_triage.json`, or on any report with a frame in the
  render core.
  - The families are RFC 0001's R32-QUEUED triage, which this RFC calls
    the K0 triage list.
  - One family was added (agent decision): `shutdown.legacy-threads`.
    These are four teardown races of the texture async loader and reader,
    the async uploader and the server's achievement save thread. The K0
    run never reached shutdown, because it hung at exit and was killed.
    None is render-core code; their owner is R20/R46.
- **A crash the lane found:** the backend's stream report
  (`ReportUnimplementedEntries`, printed at every `ReadPixels`) indexed a
  three-name table with the stream's six record kinds. Queries and scene
  captures read past it. The normal build happened to read something
  printable there; the TSan layout faulted at the first screenshot. The
  table now covers every kind, behind a bounds guard. This existed before
  K3 (`c9d4bfc0`).

| Check | Evidence | Result |
| --- | --- | --- |
| Frame executor | `render.legacy-frame-executor` (13 checks, g++ and clang++, null adapter): stages in order in one submission, each a labelled pass; absent stages have no pass; skip, prepare failure and stage failure behave; `Finish` sees the submission. `.seeded-reversed` fails the order clauses | pass |
| Pixels unchanged | all 16 families in both HDR modes on the stage-split build: measurements identical to the pre-K3 build, 32 of 32. Integer `sky` fails in both builds alike (known). gi_door frames byte-identical to the pre-K3 build in modes 0 and 2 | pass |
| Threading | `render.tsan-queued` (through the runner; `render.tsan-triage.selftest` 6 checks): testchmb_a_01 in modes 0 and 2, 3,827 reports, every one in a triaged family (particles 3,604, bones 156, proxies 28, spew 27, shutdown 10, SDL audio 2), none in the core. gi_door, which exercises the world mesh, light set and compute capabilities, adds 30 reports, all triaged | pass |
| Views and draw state | `view_oracle.py suite --check both`, forced `--queue-mode 0` and `2`, on the final build: 194 of 194 each, Portal 2 `sp_a1_wakeup` included | pass |
| Products boot | the K1 set (testchmb_a_01, testchmb_a_08, escape_00, escape_02) in modes 0 and 2, 8 of 8; Portal 2 through the view oracles in both modes; `--resize-stress` on isolated Wayland and X11, queued and sync, 4 of 4 | pass |
| GPU and headless suites | `--runner gpu`: 24 matched (the skinning corpus row skips without its corpus). `--rfc 0016 --runner headless`: 28 matched (the TSan rows skip without `CONFORMANCE_TSAN`) | pass |
| Frame time | `frame_pacing.py`, 3 interleaved rounds against the pre-K3 install at 1920x1080, mode 0. Host load rose from 5 to 14 (other sessions compiling and running Portal 2). Round 1 (load 5): K3 5.07 ms warm median against 5.46 before. Pooled over the loaded rounds: 6.68 against 5.46 ms, p99 13.7 against 9.2. The noise exceeds the allowance (1.05x median, 1.10x p99 against the K0 record 4.434/6.441 ms). The Fold7 is not attached | open: a quiet-host desktop run and the Fold7 run |

Reproduce the lane (a clang TSan tree first):

```sh
CC=clang CXX=clang++ WAFLOCK=.lock-waf-rc-tsan ./waf configure --platform-provider=sdl3 \
  --render-backend=native-vulkan --build-games=portal --sanitize=thread --disable-warns \
  -T release -o build-rc-tsan --prefix=$PWD/build-rc-tsan/install
WAFLOCK=.lock-waf-rc-tsan ./waf build install
RENDER_TSAN_BUILD=$PWD/build-rc-tsan/install RENDER_TSAN_RUNTIME=<Portal runtime> \
  python3 tools/quality/conformance.py check --suite render.tsan-queued
```

To close K3: the frame allowance on a quiet desktop host
(`render_budgets.py report --row linux-wayland-portal-frame-pacing`) and on
the Fold7 (`frame_pacing_device.py --platform android`, with
`adb install -r` only).

## Upstream to subsystem-refactor and the Hammer viewport decision (2026-09-28)

`render-core@2a8a2af7` is merged into `subsystem-refactor`. The Hammer
editor's viewports render through the core in-process (AGENTS.md, "Editor
viewports": agent decision under the user's standing instruction, agreed
with the Hammer session). The Hammer session owns that integration (R17):
- it retargets `hammer/adapters/render` onto `viewport::RenderSnapshot`;
- it renders offscreen with the Vulkan device, reading back first and then
  exporting a dmabuf;
- it adds a line pass for the 2D views and overlays.

Its device-port needs come back here as RFC 0016 clauses. dmabuf export is
expected to be an optional capability of the Vulkan adapter's host
interop, not a portable clause.

Merge audit: 17 files changed on both sides. Branch-added lines the merge
drops: 3, all intended.
- The probe-table copy grown for the moving occluders' rows (R50-RELIGHT)
  moved with the capability adapter into
  `render/legacy/queued_capabilities.cpp`.
- `WorldMeshUpload008` is a string lookup K3 removed.

Conflicts resolved:
- `keyvalues.h`: both additions kept.
- `viewrender.cpp`: the stage marker kept, with the branch's moved `info`.
- The backend stream report: the branch's identical fix for the record-kind
  table kept.

### What the core shares with the Hammer editor (2026-09-28)

These are agent decisions under the user's standing instruction, from an
interview of the Hammer session the user asked for. Each item names who
owns it.

- **Device access (landed with the upstream):** `RenderCoreBinding::device`
  exposes the core's `render.device.v2` port. It is owned by the core and
  valid until `RenderCore_Destroy`, and nothing may branch on the adapter
  behind it (CAP011 rule 5). Hammer composes `RenderCore_Create` with
  `device="vulkan"`, no features and no legacy backend, and drives its own
  per-view graphs with `SerialGraphExecutor`, without `IRenderer` frames.
  `render.composition` checks it (`P1.the-device-port-is-usable`).
- **Scene:** `viewport::RenderSnapshot` maps onto `render.scene`.
  - Each solid is one mesh instance, with its faces batched by material into
    index ranges. Entity markers are box instances.
  - Selection and hidden state are opaque per-instance flag bits. A
    face-selection change re-stages that solid's mesh.
  - Editor ids stay in the Hammer adapter (object id to instance id).
    `render.scene` holds no editor semantics.
  - Typical change sets are 1 to N solids; open and paste are full rebuilds
    (sp_a2_trust_fling: 764 solids, about 9k triangles).
- **Lines:** a shared `render.pass.lines` over a flat, portable item list:
  - world or screen space, depth-tested or not, a color, 1 px width;
  - filled screen-space quads and discs (size in px) for handles.
  
  Hammer's adapter converts `tools::OverlayList` and the grid (screen-space
  lines hammer.viewport already computes) into it. The game's debug
  overlay is a later consumer. Text is not in this pass (RFC 0010, or GTK
  over the view).
- **Materials:** the textured view resolves through `render.material`
  (families, VMT importer), with Hammer's catalog, or later a
  `render.resources` asset source, supplying VMT text and texels only.
  - Selection tint is a per-instance or per-range highlight parameter
    supplied as data, not material policy.
  - Tool textures draw as ordinary materials.
  - Hammer adds each face's texture axes to its snapshot, so the 3D view's
    UVs match the compiler.
- **Picking:** stays on the CPU (`viewport::picking`, the one ordering
  policy, which tools, MCP and tests use). There is no GPU id buffer
  unless MDL props make CPU picking wrong or slow.
- **Cameras:** `hammer.viewport` owns the editor cameras and their
  conventions. The core may add convention-free builders to `render.math`
  (look-basis view, perspective, ortho from an affine map). Source
  yaw/pitch conventions stay out of `render.math` unless the game's
  `CViewSetup` path adopts the same owner.
- **Threading:**
  - First, the GTK main loop: submit, poll completion from the frame clock,
    read back and present, with no blocking.
  - The target is a render sequence (the `MapBuildQueue` pattern) that owns
    the device, with immutable per-view inputs posted to it and textures
    returned through `GlibTaskRunner`.
  - At most 4 views; only dirty views re-render, and hidden or zero-size
    views skip. The budget to set is edit-to-pixels ≤ 16 ms for
    sp_a2_trust_fling across 4 views.
- **Lighting:** fullbright shaded for now. A lit-preview toggle on the
  clustered lights and shadow atlas comes after R89/R90, and baked-lightmap
  preview belongs to R52.
- **Oracles:** Hammer's viewport checks move to the recording null device
  (exact draws and passes per view, overlay item counts, which views
  re-render per change) and to relational Vulkan pixel checks:
  - selected pixels carry the tint;
  - an overlay box lands at its projection;
  - the object drawn under `WorldToScreen(p)` is the one
    `viewport::picking` hits.

  `viewport_smoke` and `corpus.hammer.ui` stay the product gate. The GL
  renderer and its PPM comparisons go once the core covers cameras, 2D
  wireframe and grid, selection, overlays, displacements and entity
  markers.

### Hammer's viewports on the core (2026-09-28)

State: the GTK shell's viewports render through the core, and the GL renderer
is deleted (its deletion condition above is met). Owner: the Hammer session
(R17). Record: [RFC 0002 progress](0002-progress.md#r17-core-the-gtk-viewports-on-the-render-core-slice-done-2026-09-28).

- **`render.pass.lines`** (layer 6 feature, contract `render.lines.v1`):
  - a flat `LineList` of lines, boxes, polygons, filled triangles with
    per-vertex colors, and filled screen quads and discs, each in world or
    screen space and depth-tested or not, plus resident `MeshBatch`es in the
    same 16-byte vertex layout;
  - one upload pass and one render pass, in a fixed draw order (handles
    last), with the clip matrix and a clip-space depth offset for tested
    lines in 80 bytes of D16 draw constants and no bind groups.
- **`render.math` builders** (`render.math-builders.v1`): `LookBasis` (a view
  from an eye and an explicit basis) and `PixelToClip` (logical pixels, origin
  top-left, to clip). Neither names a camera convention.
- **Tools product:** `--render-core-vulkan=auto|on|off` builds the Vulkan
  adapter for the tools product, which has no engine renderer. It is `auto`
  by default: built when the loader is found. Clients still follow
  `--render-backend`.
- **Hammer side** (`hammer.adapters.render`, contract
  `render_adapter.viewport-geometry.v1`): `ViewportRenderer` over the lines
  pass on `binding->device`, one serial graph per view (grid pass, scene
  pass, readback), with resident scene geometry restaged per scene key.
  `SceneProjection` now follows the editor's `RenderSnapshot` by `ObjectId`.
- **Interim, against the scene decision above:** the solids draw as a
  resident lines-pass triangle batch with the fullbright shading baked into
  vertex colors, not as `render.scene` instances. The opaque pass has no
  shading or per-face color yet.
  - Deletion condition: when a material family draws `render.scene`
    instances with per-range highlight data, the viewport's face batch moves
    to the projection's scene and the resident face batch is deleted.
- **Evidence:**

| Suite | Result |
| --- | --- |
| `render.lines.null` | 13 checks: order, draw constants, pipeline reuse, refusals, empty list |
| `render.lines` (Vulkan) | 17 checks: pixel placement in both spaces, depth and bias, handle order, alpha, resident batches, repeat frames, no validation message |
| `render.math.builders` | 4 checks, including 2 seeded wrong builders |
| `hammer.adapters.render.geometry` / `.viewport.null` / `.viewport` (Vulkan) | 18 / 17 / 15 checks |
| `corpus.hammer.ui` | 14 checks: the live editor's frames show the room it built; with no Vulkan driver both frame checks fail while the editing checks pass |

- **Not done:** textured views (K4 families), dmabuf export (readback into a
  `GdkMemoryTexture` for now), a render sequence off the GTK main loop, and
  the edit-to-pixels budget.

## K4 slice: SPIR-V headers generated by the build (2026-09-28)

State: K4's "Artifacts per target" check passes. The committed `*_spv.h`
headers are deleted and the build reproduces them. K4 stays open for the
family ports with pixel oracles and the frontend side of the proxy corpus.

- **One generator:** `tools/render/shader_artifacts.py headers --out DIR`
  (and `build --headers DIR`) writes all 11 headers
  (`shader_toolchain.GENERATED_NAMES`) into a flat `spv/` directory.
  - The backend's `material_spv.h`, `material_spv_index.h` and
    `legacy_spv.h` come from the artifacts, with the regenerators' writers.
  - The `GENERATED` rows in `tools/render/shader_toolchain.py` produce the
    other eight: demo_triangle, skin, opaque, cluster_assign and shadow, and
    the three test-defect headers.
  - Consumers include `"spv/<name>.h"`. Only the render.device.v2 suite's
    small fixtures stay committed (`EMBEDDED`).
- **Waf:**
  - `render/shaders` is configured in the root environment and recursed
    from the root before every project, and ends a build group:
    `gccdeps` learns header dependencies only after compiling, so it
    cannot order consumers by itself.
  - The task generator `render_spv` exports the generated include root.
    Consumers name it in `use`, which also posts it under `--targets`
    (`build-ios-app.sh` builds `--targets=hl2_launcher`).
  - It depends on every GLSL a header embeds, the render passes' included.
  - Configure fails without the pinned tools in any product that has the
    render core.
- **Regenerators:** `regen_material_spv.py` and `regen_legacy_spv.py`
  never write into the source tree. They write with `--out`, and check a
  build's copies with `--check --compare-dir`; they are the independent
  writers the gates compare with. `--debug-out` takes an optional
  `--shipped` spv directory.
- **Gates:**
  - `shader.toolchain-pin` (52 checks): writes the headers as the build
    does, requires the regenerators and the compiler, row by row, to agree
    with them, rebuilds the committed fixtures, and fails on any committed
    SPIR-V outside `EMBEDDED` and on a committed copy of a generated header.
  - `render.shader-artifacts` (1,269 checks): every header written, the
    backend's agreeing with the regenerators, and each unit's array
    matching its artifact.
  - Both `.sensitivity` rows pass: seeded bytes in generated and committed
    modules, a foreign compiler, a layout mismatch and a fifth group.
- **Runner:** a profile may declare `generated_include_roots`, entries
  with an id and a command that the runner runs once per invocation into
  `build/quality/generated/<id>`. `linux-headless-core` and
  `linux-native-vulkan-gpu` declare the SPIR-V headers, and the iOS device
  harness inherits them through the runner's build functions. Self-tests: a
  generated header reaches the build, and a failed generator fails the
  suite as a compile error (82 runner and iOS harness tests pass).
- **Architecture:** `capabilityModules.generatedHeaders` declares the
  `spv/` prefix as owned by `render.shader-library` (producer
  `render/shaders`). CAP002 resolves such includes to that owner, so a
  consumer needs the edge: the four passes and `render.vulkan.core` now
  declare it. Four new archlint fixtures (owner edge, missing edge,
  undeclared prefix, unknown owner); 157 archlint tests pass.
- **CI:** the Linux, macOS and Android lane scripts, and the Windows steps
  in `build.yml` and `tests.yml`, build the pinned tools before configure.
  Hosted CI has not run.
- **Also:** `RenderCoreBinding::device` for hosts (the Hammer viewports),
  and the portal dlight lab takes its product from
  `PORTAL_DLIGHT_LAB_BUILD` instead of a private tree path.

| K4 check | Evidence | Result |
| --- | --- | --- |
| Artifacts per target | the committed headers are deleted; the client, tests and static trees build them; a `--targets=shaderapivulkan` build from a removed generated directory regenerates them first; both gates and their sensitivity rows pass | pass |
| Suites | headless RFC 0016 (28), the backend consumer suites, and the GPU runner (22; three environment-gated skips) pass on generated headers | pass |

Upstreamed onto subsystem-refactor after the Hammer session had added
`render.pass.lines` with a committed `lines_spv.h`. The port converted it
like the other passes: a `GENERATED` row, `"spv/lines_spv.h"`,
`use=render_spv`, and the `render.shader-library` edge. The branch's own
regenerated `material_spv.h`, `material_spv_index.h` and `legacy_spv.h`
equal what the build now writes from its GLSL, byte for byte.

## K4 family slice: `unlit`, and port clause D17 (2026-09-28)

State: the first material family is on the core and matches its legacy
port. K4 stays open for `lightmapped`, `vertexlit` and `pbr`, and for the
frontend side of the proxy corpus.
- Split with the Hammer session, agreed 2026-09-28 at the user's direction
  ("make sure K4 is completed"):
  - Hammer owns `vertexlit`, `render.pass.opaque` drawing scene instances
    through families, family textures through `render.resources`, and the
    dmabuf capability (clause D18).
  - This session owns `unlit`, `lightmapped`, `pbr`, D17 and the proxy
    corpus.

- **The family:** `render.material`'s `UnlitFamily`.
  - Its program is `render/material/families/unlit.{vert,frag}`, generated
    into `spv/families_spv.h`.
  - `ClaimUnlit` claims UnlitGeneric's base texture, `$color`/`$alpha`,
    vertex color and alpha, alpha test, and translucent or additive
    blending. Any other parameter set away from its default makes it refuse
    the material, naming the parameter, so the material stays on its legacy
    port: a declared narrower capability.
  - The material group (role kMaterial) holds the packed constants, the
    base texture and a sampler; world-to-clip is a D16 draw-constant block.
  - `layouts.json` declares the family (one group of four), so the artifact
    build judges its reflection and the bind-group ceiling (1,291 checks).
- **The oracle:**
  - `quality/fixtures/legacy-shaders/families/unlit.vdf` has six cases.
  - `legacy_shader_conformance.py` draws them through the port and judges
    it against the retail D3D9 bytecode; all six pass.
  - `tools/render/family_port_pixels.py` records the port's pixels as
    `quality/fixtures/render-families/unlit-port-v1.vdf`, with the case
    file's sha256.
  - `render.family.unlit` (59 checks, g++ and clang++) imports each case,
    claims it, draws it with the family on `render.device.vulkan` and
    compares. Every case is within 2 levels, five of them exactly.
  - `.seeded-ignore-vertex-color` is caught.
- **What matching the port took:**
  - Source's gamma rules: `$color` through mathlib's `GammaToLinear` (the
    pow 2.2 table, and 1 from 0.95), vertex color through pow 2.2.
  - The D3D9 half-pixel shift a legacy draw carries: +1/w in x and −1/h in
    y in clip space. The legacy frontend owns it; native cameras such as
    Hammer's don't need it.
  - The port's alpha-write rule: no destination alpha for translucent and
    alpha-tested draws. This needed a new port clause.
- **D17, color write masks:** `PipelineDesc::colorWriteMasks`, one
  `kColorWrite*` mask per color format.
  - Validation rejects a wrong count and masks outside the four channels.
  - Vulkan maps it to `colorWriteMask`.
  - The shared suite checks it on real pixels: a red-and-alpha mask keeps
    green and blue. `render.device.v2.null` 375 checks, `.vulkan` 694.
  - The Vulkan sensitivity knob `ignoreColorWriteMasks` fails D17 alone.
- **Also:** the artifact debt report no longer lists push constants as
  non-port. They are D16 draw constants; only a block over 128 bytes owes.

| K4 check | Evidence | Result |
| --- | --- | --- |
| Families match ports | `unlit` within 2 levels of its port on 6 cases, with a seeded defect caught; `lightmapped`, `vertexlit` and `pbr` open | partial |
| Bind-group ceiling | the `unlit` family is judged by the artifact build (one group); a seeded fifth group still fails | pass |


## K4 family slice: `lightmapped` (2026-09-28)

State: the world's family is on the core and matches its legacy port
exactly. K4 stays open for `vertexlit` (the Hammer session), `pbr` and the
frontend side of the proxy corpus.

- **The family:** `render.material`'s `LightmappedFamily`.
  - Its program is `render/material/families/lightmapped.{vert,frag}`,
    generated into `spv/families_spv.h`.
  - `ClaimLightmapped` claims LightmappedGeneric's and
    WorldVertexTransition's base texture, `$color`/`$alpha`, vertex color,
    vertex alpha blending, alpha test and translucency. It refuses bump
    maps, `$basetexture2`, env maps, `$additive` and every other parameter
    set away from its default, by name.
  - Two groups: the material group holds the constants, the base texture
    and its sampler. The draw group (role kDraw) holds the lightmap page and
    its sampler, because surfaces of one material sit on different pages.
    `layouts.json` declares both, so the artifact build judges the
    reflection and the ceiling.
- **The oracle:** eight cases in
  `quality/fixtures/legacy-shaders/families/lightmapped.vdf`, drawn by the
  port (`-vklegacylightmapped`) and judged against the retail D3D9
  bytecode, all pass. `render.family.lightmapped` (92 checks, g++ and
  clang++) draws each with the family: all eight match the port's pixels
  exactly. `.seeded-gamma-color` is caught.
- **What matching the port took**, read from the port's captured
  constants:
  - The tint is `$color` times the lightmap scale, 2 in gamma space and
    2^2.2 linear. `$color` is not gamma converted, unlike unlit's.
  - Vertex color is used unconverted, also unlike unlit's.
  - The port's vertex fast path (no texture transform, no detail) holds
    for every claimed material. With `$vertexcolor` the vertex alpha
    replaces the modulation alpha; without it, `$alpha` applies twice (VS
    modulation and PS factor). Two added cases pin this:
    `vertexcolor_alpha` and `vertexalpha_only`.
  - `$vertexalpha` without `$vertexcolor` only selects blending: the
    port's vertex format has no color then.
- **Shared code:**
  - `render/material/family_program.{h,cpp}` holds the claim rule
    (`UnclaimedParameter`), parameter reads and Source's gamma table for
    every family. `unlit` now uses it.
  - `unittests/rendertest/core/material/family_pixel_cases.{h,cpp}` is the
    suites' harness: case and fixture parsing, import, the D3D9 half-pixel
    to-clip, drawing with any number of groups, and judging. The unlit
    suite runs on it (65 checks, unchanged verdicts); `vertexlit` can too.
- **Fixed:** `render.core-tests` lacked its edge to
  `content.keyvalues-text`, so archlint reported CAP002 on the unlit suite
  (reported by the Hammer session).

| K4 check | Evidence | Result |
| --- | --- | --- |
| Families match ports | `unlit` within 2 levels on 6 cases; `lightmapped` exact on 8 cases; a seeded defect caught for each; `vertexlit` and `pbr` open | partial |
| Bind-group ceiling | `unlit` (one group) and `lightmapped` (material and draw) judged by the artifact build; a seeded fifth group still fails | pass |

## K4 slice: families draw scene instances, and Hammer's textured viewport (2026-09-28)

State: `render.pass.opaque` draws `render.scene` instances through the
material families, and Hammer's camera view is textured through the `unlit`
family. The lines-pass interim for Hammer's solids is gone. `vertexlit` and
D18 (dmabuf export) are next on the Hammer session's side.

- **Owners of the families' groups** (`public/render/material/material_programs.h`):
  - `GroupResidency` is the one mechanism. Per id, it holds a group of one
    layout with an optional uniform buffer of packed constants and named
    `TextureCache` textures with their samplers.
  - A group exists only while every texture it names is in the cache, and
    is rebuilt when a texture's revision rises. Samplers are shared by
    description. It follows the caches' Set/RecordUploads/Retire protocol.
  - `MaterialPrograms` serves programs (`IDrawPrograms`): pipeline, stride,
    draw-constant size, the draw layout the family reads, and the material
    group.
  - `DrawGroups` serves per-draw groups (`IDrawGroups`), such as
    `lightmapped`'s lightmap page. Families turn a claim into a
    `ProgramRequest`; `UnlitFamily::Request` is the first.
- **Draw-constant convention** (`draw_program.h`): every family's D16 block
  starts with `FamilyDrawConstants { toClip[16]; world[16]; }` (128 bytes).
  The pass writes the first `drawConstantBytes`.
- **`render.pass.opaque`:**
  - `AddOpaquePasses(builder, snapshot, list, view, OpaqueSources{meshes,
    programs, drawGroups}, targets)` draws each item with its program's
    pipeline and material group.
  - It binds the instance's draw group (`MeshInstanceDesc::drawGroup`,
    new) when the program reads one.
  - It imports every mesh buffer, texture and uniform buffer in its
    residency usage and creates no device objects.
  - `OpaqueRenderer`, the flat-color shaders and their GENERATED row are
    deleted.
- **`render.pass.lines`:** colors are display values. On an sRGB target the
  pass decodes them, so the pass can share the families' sRGB target
  (clause P8).
- **Hammer:** described in the
  [RFC 0002 record](RFC/0002-progress.md#r17-core-the-gtk-viewports-on-the-render-core-slice-done-2026-09-28).
  - The viewport draws one mesh and one instance of its own `render.scene`
    per material batch, through the opaque pass and `unlit`.
  - Base textures come from `IMaterialTextures`, the host's
    `MaterialCatalog` on the render sequence.
  - RFC 0008's `hammer_ktx2_preview.py` smoke passes: 89,854 red pixels
    from the packaged KTX2, and 0 for the corrupt control.

| Check | Suite | Result |
| --- | --- | --- |
| Group residency, programs, draw groups | `render.material.programs` (31 checks, null device) | pass |
| Opaque through families | `render.opaque.null` (9), `render.opaque` (14, Vulkan, validation silent): depth test, two scenes, unresolved draws (missing mesh, unknown material, absent texture, wrong stride, missing or wrong-layout draw group) counted | pass |
| Lines on an sRGB target | `render.lines` P8 (18 checks) | pass |
| Hammer textured viewport | `hammer.adapters.render.geometry` (G8, G9), `.viewport.null` (V6), `.viewport` (R4: texel times shading per UV half, within two levels), `.service` (S6, and its TSan row) | pass |
| Positive draw-group draw | waits for `LightmappedFamily::Request` | open |

## K4 family slice: `pbr` (2026-09-28)

State: the RFC 0007 family is on the core for meshes and matches the native
model port within 1 level. K4 stays open for `vertexlit` (the Hammer
session) and the frontend side of the proxy corpus.

- **The family:** `render.material`'s `PbrFamily`.
  - Its program is `render/material/families/pbr.{vert,frag}` and
    `pbr_lighting.glsl`, generated into `spv/families_spv.h`.
  - `ClaimPbr` claims `$basetexture`, `$mraotexture`, `$bumpmap`,
    `$emissiontexture` and `$emissionscale`. It refuses environment maps,
    alpha test, translucency, clear coat and glass by name, so those
    materials stay on the native stages until the family claims them.
  - Three groups: the frame group holds the split-sum table
    (`SplitSumTable()` from RFC 0007's generated table). The view group
    holds Source's model lighting: `PackSourceModelLighting` packs the
    ambient cube and up to four lights as the shader API does (sorted spot,
    point, directional, with `SetLight`'s cone). The material group holds
    the constants and four texture/sampler pairs.
  - The draw constants are object-to-clip and object-to-world: the
    128-byte `FamilyDrawConstants` prefix the Hammer session's opaque pass
    pushes.
  - Model lighting is the legacy frontend's interface until K7's light
    set replaces it.
- **One BRDF:** `pbr_brdf.glsl` moved to `render/shaders/common/`, as the
  RFC's migration table says. The backend's stages include it by a
  relative path, which every compile path (the build, the regenerators,
  glslangValidator debug variants) resolves against the including file.
  - `PbrSplitSumCoordinate` is split out of `PbrSplitSum`, because GLSL
    cannot pass a separately constructed sampler to a function.
  - The native `pbr-model` frames are byte-identical before and after;
    `render.pbr-brdf.glsl` passes.
  - The source guard (`test_pbr_shader_library.py`) resolves includes as
    glslc does, and now also covers the family's stage.
- **The oracle:** the native `pbr-model` material pixel run is judged per
  pixel against an independent BRDF model (`material_pixel_pbr_model.py`).
  - `family_port_pixels.py record-model` records a passing run as
    `quality/fixtures/render-families/pbr-port-v1.vdf`. The fixture holds
    the harness's own inputs (textures, materials, quads, and each case's
    placement, cube and lights) with the port's pixels on an 8-pixel grid
    wherever the oracle judges one (7,132 pixels). The harness stays the
    case table's only owner.
  - `render.family.pbr` (85 checks, g++ and clang++) draws every case with
    the family. Every case is within 1 level of the port, three of them
    exactly. `.seeded-ignore-normal-map` is caught.
  - `pbr_skinned` is drawn with its placement as a rigid transform:
    skinning is `render.pass.skinning`'s (K6).
- **What matching the port took:** a directional light's direction. The
  port's pixel constants place a directional light 10,000 units from the
  lighting origin against its direction (`CommitPixelShaderLighting`),
  while the vertex constants carry the light's own position, which lies
  along its direction. The family shines a directional light along its
  direction.
- **Harness:** `family_pixel_cases` gained model fixtures
  (`LoadModelCases`), any texture format, RGB-only port pixels, per-family
  draw constants, a `$fallbackmaterial` resolver, and
  `RENDER_FAMILY_DUMP_DIR` for frame dumps.

| K4 check | Evidence | Result |
| --- | --- | --- |
| Families match ports | `unlit` within 2 levels on 6 cases; `lightmapped` exact on 8; `pbr` within 1 on 8 (7,132 pixels); a seeded defect caught for each; `vertexlit` open | partial |
| Bind-group ceiling | `unlit`, `lightmapped` and `pbr` (frame, view and material groups) judged by the artifact build (1,342 checks); a seeded fifth group still fails | pass |

## K4 slice: texture transforms in the schemas, and `MapVariables` (2026-09-28)

The first step of the proxy corpus's frontend side: proxies such as
TextureTransform, TextureScroll and MatrixRotate write
`$basetexturetransform`, which no family schema carried. A material that set
a transform was therefore drawn by a family that ignored it.

- `kTransform` parameters hold rows 0 and 1 of the 4x4 (8 floats, 16-byte
  aligned, the identity by default), as Source's shaders read a texture
  transform. The legacy-derived families carry `$basetexturetransform`,
  `$detailtexturetransform` and `$envmapmasktransform`; `lightmapped` and
  `vertexlit` add `$bumptransform`, and `lightmapped` adds
  `$basetexturetransform2` and `$blendmasktransform`.
- The importer reads both of the material system's forms (16 row-major
  numbers, or `center u v scale u v rotate degrees translate u v`, composed as
  `CreateMatrixMaterialVarFromKeyValue` composes it). A malformed value keeps
  the identity with a diagnostic.
- `MapVariables(shader, variables)` is ImportVmt's shader and variable
  mapping without the text. The legacy frontend will pass a bound material's
  variables through it, so a proxy's output reaches the family's block by the
  importer's own rules.
- The families refuse a transform they do not claim, by name.
- Evidence: `render.material.v2` 76 checks (X1–X4: both forms, a malformed
  value, layout and apply, and `MapVariables` equal to `ImportVmt`). The
  key-mapping oracle covers the new rows. The VMT corpus and its `.asan` lane
  pass with unchanged counts, and the unlit, lightmapped and pbr family
  suites pass.

## K4 slice: lightmapped and pbr programs for the opaque pass (2026-09-28)

At the Hammer session's request, the two families now produce the
`ProgramRequest` and `GroupRequest` shapes the opaque pass draws with
(5acb0860, 2875f40e):

- `LightmappedFamily::Request(claim, baseTexture)` fills the material group
  and sets `drawLayout`; `LightmapGroup(page)` is a draw group for a
  lightmap page.
- `PbrFamily::Request(claim, PbrTextures)` fills the material group (a
  placeholder texture fills the unused normal-map and emission slots) and
  sets `frameLayout` and `viewLayout`; `FrameGroup(table)` and
  `ViewGroup(lighting)` are the frame's and the view's groups.
- The `pbr` program follows the pass's draw constants: `toClip` is
  object-to-clip and `world` object-to-world (it had applied `world`
  twice). `render.family.pbr` passes object-to-clip and is unchanged.
- `render.opaque` O6 (19 checks, g++ and clang++): scene C draws a
  lightmapped cube lit by its page and a pbr cube lit by the view's ambient
  cube, with the expected pixels and no unresolved draw. Without the view
  group the pbr draw is counted, not drawn. `render.opaque.null` N4 runs
  the same scene on the null device.

## K4 slice: the proxy corpus's frontend side (2026-09-28)

State: K4's "Proxy corpus" check passes on Portal and Portal 2. K4 stays open
only for `vertexlit` (the Hammer session).

- **`RenderMaterialBlocks001`** (`public/render/legacy/material_blocks.h`,
  a preserved-ABI package with plain C types) is a frontend app system beside
  `RenderStageMarkers001`. The launcher adds it from the core's binding
  (`RenderCoreBinding::materialBlocks`).
  - `FormatBlock` takes a bound material's variables as the material system
    prints them and maps them with `MapVariables` and `ApplyValues`, the
    importer's own rules. It reads the family block back as one JSON line.
  - A legacy-family material gets no block, and an unknown shader is
    refused.
  - The frontend stays strict. It never includes a legacy material header:
    the caller formats the variables.
- **The capture:** `mat_proxy_capture` writes the frontend's block line
  after each material line, when the core is composed
  (`mat_proxy_capture_blocks`, default 1).
- **The check:** `proxy_corpus.py capture --core` requires a block for
  every material pass. Each block parameter must equal the legacy variable
  its key names: textures by name, flags and integers by the importer's
  rules, and floats, vectors and texture transforms (rows 0 and 1) within
  1e-5. The comparator selftest (`render.material.proxies.comparator`,
  25 checks) catches a changed float, texture, transform row, vector
  component and integer, an unapplied block, a key without a variable, an
  unknown family and a missing block.
- **Evidence** (`render.material.proxies.core-portal` and `.core-portal2`):

  | Game | Proxies | Family blocks per capture | Parameters compared | Differences |
  | --- | --- | --- | --- | --- |
  | Portal | 66 | 136 (133 unlit, 3 vertexlit; 2 legacy passes) | 2,492 and 2,491 | 0 |
  | Portal 2 | 69 | 136 (2 legacy passes) | 2,493 and 2,493 | 0 |

  The proxies' outputs reach the blocks: MatrixRotate's and
  TextureScroll's transforms, and Sine's `$alpha`, match their legacy
  values. Both legacy captures still equal the recorded fixture.
  Composition clause P5 covers the interface (18 checks, g++ and clang++).
- **Found:** the first Portal 2 boot after a fresh reconfigure and install
  failed the legacy fixture on the two noise proxies alone. Their values were
  shifted by one draw of the shared random stream. Four later boots all
  match exactly: without the core, with the core and blocks off, and twice
  with blocks. It is a first-boot effect, not the block path. It is not
  fixed.
- **Not claimed:** the product draws no material through a family block
  yet. Proxies still run at the legacy bind point, and routing draws to
  families is K5 and K8 work.

## K4 family slice: `vertexlit` (2026-09-28)

State: VertexLitGeneric's family is on the core and matches its legacy port
exactly on 10 cases. With it every family K4 names matches its port. This
section does not close K4; the product still draws no material through a
family (K5, K8).

- **The family:** `render.material`'s `VertexLitFamily`.
  - Its program is `render/material/families/vertexlit.{vert,frag}`,
    generated into `spv/families_spv.h`.
  - `ClaimVertexLit` claims the base texture, `$color`/`$alpha`, alpha
    test, translucency and `$halflambert`. It refuses bump maps, env maps,
    detail, self-illumination, `$phong`, rim lights, light warps,
    `$additive` and texture transforms by name.
  - `$vertexcolor` and `$vertexalpha` are accepted and their vertex data
    ignored: `vertexlitgeneric_dx9_helper.cpp` ignores both for
    VertexLitGeneric. `$vertexalpha` still selects blending, as the port
    blends (`EvaluateBlendRequirements`).
  - Two groups. The material group holds the constants, the base texture
    and its sampler. The draw group (role kDraw) holds the draw's lighting
    in a uniform buffer, because each model instance has its own ambient
    cube and lights. `LightingGroup` builds it for `DrawGroups`; `Request`
    names the draw layout.
  - The lighting is `PackSourceModelLighting`'s packing, the one owner that
    `pbr` also uses (spot, point, directional; `SetLight`'s cone).
  - The draw constants are the `FamilyDrawConstants` prefix. Positions and
    normals reach the lights' space through object-to-world.
- **The oracle:** ten cases in
  `quality/fixtures/legacy-shaders/families/vertexlit.vdf`: ambient only,
  point, directional with `$color`, spot, four lights of all three types,
  half-Lambert, alpha test, translucency, `$alpha` alone, and ignored
  vertex color. The port (`-vklegacyvertexlit`) draws them and is judged
  against the retail D3D9 bytecode; all pass. `render.family.vertexlit`
  (113 checks, g++ and clang++) matches the port's pixels exactly on every
  case. `.seeded-ignore-half-lambert` is caught (12 levels on the
  half-Lambert case).
- **What matching the port took**, read from its captured constants:
  - Per-vertex `DoLighting` with static control flow: the lights in order,
    then the ambient cube, in the port's expressions (`i0` holds the light
    count, so four lights are drawn).
  - `$alpha` below one blends, as `$translucent` does
    (`IsAlphaModulating`).
  - The alpha-test reference is a byte: 0.5 is 127/255.
  - Source's gamma table is indexed by `RoundFloatToInt`, which rounds
    half to even. `SourceGammaToLinear` used `lround`, so `$color` 0.7
    (178.5 in float) took index 179 instead of 178, one level off. It now
    uses `lrint`. The `unlit` suite's `$color` 0.7 case is also exact now.
- **Opaque pass:** scene C gains a vertexlit cube whose draw group holds
  its lighting. It draws with the expected pixels (O6), and `render.opaque.null`
  N4 draws three families.
- **Harness:** `family_pixel_cases` reads case normals, the ambient cube
  and light blocks, with the material pixel harness's defaults.
- **Decisions** (agent, under the user's standing instruction):
  - The lighting is a draw group, not a view group as in `pbr`. Source
    lights each model instance with its own cube and lights.
  - `PbrModelLighting` and `PackSourceModelLighting` keep their `pbr`
    names; `VertexLitLighting` is an alias. A neutral header is a
    follow-up for the `pbr` owner.
  - Texture transforms are refused, as the other families refuse them.

| K4 check | Evidence | Result |
| --- | --- | --- |
| Families match ports | `unlit` exact on 6 cases; `lightmapped` exact on 8; `pbr` within 1 on 8; `vertexlit` exact on 10 (120 pixels); a seeded defect caught for each | pass for the four families |
| Bind-group ceiling | `render.shader-artifacts` (1,362 checks) judges `vertexlit` (material and draw groups) with the others; a seeded fifth group still fails | pass |

Reproduce:

```sh
python3 tools/quality/legacy_shader_conformance.py --runtime run/runtime \
    --build <installed client tree with material_pixel_conformance> \
    --out <new dir> --cases quality/fixtures/legacy-shaders/families/vertexlit.vdf \
    --hdr none --extra-arg=-vklegacyvertexlit
python3 tools/render/family_port_pixels.py record --run <dir> --family vertexlit \
    --out quality/fixtures/render-families/vertexlit-port-v1.vdf
python3 tools/quality/conformance.py check --suite render.family.vertexlit \
    --suite render.family.vertexlit.seeded-ignore-half-lambert [--cxx clang++]
```

## Port clause D18: external images, and the Hammer viewport's dmabuf frames (2026-09-28)

State: `render.device.v2` exports images, and the Hammer editor's viewports
reach GTK as dmabufs without a copy.

- **The port:**
  - `Capability::kExternalImages` and `ResourceUsage::kExternal` (the hand-over
    to another API) are new, and so is `render/device/external_images.h`.
  - `IRenderDevice2::ExternalImages()` is non-null exactly when the capability
    is claimed.
  - `IExternalImages::CreateExported` makes a 2D, one-mip RGBA8 or BGRA8 texture.
    It returns a handle and one plane's description as opaque integers: a dmabuf
    fd, DRM fourcc and modifier, offset and stride.
  - `CloseHandle` closes a handle, so portable owners name no platform call.
    `CreateTexture` refuses `kExternal`.
- **Vulkan:** with `VK_KHR_external_memory_fd`,
  `VK_EXT_external_memory_dma_buf` and `VK_EXT_image_drm_format_modifier`, an
  exported image is LINEAR (`DRM_FORMAT_MOD_LINEAR`) in dedicated exportable
  memory. `kExternal` is `GENERAL`.
  - The memory is host-visible device-local where a type allows. On RADV a
    device-local-only dmabuf cannot be mapped, and a toolkit's fallback, like the
    clause, maps it.
  - The null device drops the capability from its defaults.
- **Clause D18** (shared suite): the exporter exists exactly when claimed, and
  descriptions outside the rules fail by status. The exported memory, mapped at
  its offset and stride after the hand-over, equals the port's readback.
  - It passes on RADV (Strix Halo) and llvmpipe, g++ and clang++.
  - Bad adapters `stale-export` and `null-exporter` each fail only D18.
- **Hammer:**
  - `ViewportRenderer` renders a view with `ViewRequest::external` into a pool
    of exported images and hands out leases.
  - `ViewportService::ReturnFrame` gives a lease back on the render sequence.
  - The GTK shell dups the handle into a `GdkDmabufTexture` and returns the
    lease when GTK drops the texture. It falls back to read-back pixels if GTK
    refuses an import.
  - `corpus.hammer.ui` passes on both paths and now judges which path the
    live editor used (`frames.path`).

| Check | Suite | Result |
| --- | --- | --- |
| D18 on the adapters | `render.device.v2.vulkan` (RADV; llvmpipe with `RENDER_VK_ADAPTER=1`), `.null`, `.vulkan.sensitivity` (stale-export, null-exporter) | pass |
| Hammer exported frames | `hammer.adapters.render.viewport` R5 (an external frame equals the read-back frame; leases and resize), `.viewport.null` V7 | pass |
| Live editor | `corpus.hammer.ui` (15 checks) with dmabuf frames and with `HAMMER_GTK_READBACK=1` | pass |
| Edit-to-pixels | `hammer-viewport-v1.json` `desktop-trust-fling-4-views-dmabuf` | pass |

## K4 closure: done (2026-09-28)

K4 is met on the Linux desktop. The run at render-core `d041b2c3` (the
tree of subsystem-refactor `fb97c8b2`, g++) passed all 22 K4 suites, 22 of
22 matched. The families, programs and material suites also pass on
clang++ (above and the Hammer session's `vertexlit` record).

| K4 check | Evidence | Result |
| --- | --- | --- |
| Material suite | `render.material.v2` 76 checks (schema, parameter blocks, revisions, import, patches, PBR, texture transforms, `MapVariables`); `.sensitivity` 7: a wrong key mapping, a wrong kind and stale copies are caught | pass |
| VMT corpus | `render.material.vmt-corpus` (18), `.sensitivity` (7) and `.asan` (18) over the Portal and Portal 2 VPKs; every VMT imports or is reported with its reason, no crash, counts recorded in `vmt-corpus-v1.json` | pass |
| Families match ports | `unlit` 6 cases within 2 levels (exact since the Hammer session's gamma-table rounding fix); `lightmapped` 8 cases exact; `vertexlit` 10 cases exact (Hammer session); `pbr` 8 cases within 1 level (7,132 pixels); each family's seeded defect is caught; `legacy` is the K3 stage-pass path, byte-identical in both queued modes (K3 record), and `pbr_brdf.glsl`'s move left the native `pbr-model` frames byte-identical | pass |
| Proxy corpus | `render.material.proxies.core-portal` and `.core-portal2` (cdca445a): every block parameter equals its legacy variable, 0 differences over 2,492 and 2,493 parameters per capture; `.inventory` (8) and `.comparator` (25) | pass |
| Bind-group ceiling | `render.shader-artifacts` judges every family's reflection against `layouts.json` (unlit one group, lightmapped two, vertexlit two, pbr three); its `.sensitivity` (14) keeps a seeded fifth group and a seeded layout mismatch failing | pass |
| Artifacts per target | `render.shader-artifacts` 1,362 checks: SPIR-V and GLSL 4.50 for every family; no committed `*_spv.h` (the build writes them); `shader_toolchain.py check` reproduces them | pass |

Not claimed: hosted CI (not run), the Fold7 and Apple devices (K4 declares
none), and the product drawing materials through families. K5 routes the
world and props through the scene, and K8 does the other cohorts.

## K5 slice: the engine's world scene and "Culling matches" (2026-09-28)

State: K5's "Culling matches" and "Serial equals pooled" checks pass on the
K0 views. K5 stays open for pixels (the world and props drawn from the
scene) and submission cost.

- **The world scene** (`engine/render_core_world.cpp`): at level load the
  engine fills the world render scene it owns.
  - Each non-solid BSP leaf gets one instance, bounded by the box the legacy
    traversal last tests for it: its own, or the parent of its enclosing
    "too small to cull" subtree (`MarkSmallNode`, contents -2).
  - Each static prop gets one instance, bounded by its world render box.
  - The engine links no render module. `render_core_host.cpp` calls the
    core through `SceneFactory`, and the world file, which names no render
    type, passes it plain arrays. The engine's global `render` and the
    core's namespace cannot meet in one file.
- **Port changes:**
  - `ViewDesc::frustum`: a view's owner may pass the planes it culls with
    (scene clause C7).
  - `SceneFactory` gains `makeView`, `buildDrawList`, and
    `buildDrawListPooled` with a context. The composition runs the pooled
    builder on a four-worker `jobsystem::ParallelExecutor` it creates on
    first use.
- **The check:** `r_core_cull_capture` records every world list of a
  frame, with the props the client drew in the same 3D view (views nest).
  For each, the core culls the scene with that view's own frustum planes,
  using the legacy view's visibility as the provider. The provider is the
  BSP traversal's PVS, area bits and area frustums for leaves, and the
  drawn props for props. The core then builds the pooled list beside the
  serial one.
  - `tools/render/culling_capture.py` runs the K0 view workload with the
    capture in place of each screenshot: `render.scene.culling`, and
    `.selftest` with 12 checks and 9 seeded faults.
- **Result** (136 checks, 0 failures):

  | Scenario | Views | Legacy leaves | Outside the view | Props | Leaves the core's frustum culled |
  | --- | --- | --- | --- | --- | --- |
  | testchmb_a_00 | 111 | 1,248 | 255 | 202 | 96,659 |
  | testchmb_a_08 | 141 | 6,237 | 335 | 1,144 | 127,559 |
  | testchmb_a_01 legacy-ports | 17 | 529 | 14 | 56 | 15,550 |
  | Portal 2 sp_a1_wakeup | 82 | 1,294 | 153 | 1,021 | 131,428 |

  - The core's leaves and props equal the legacy ones in every view, with
    one exception: 757 leaves that legacy keeps outside the view. Legacy
    tests a leaf in an area it sees through an area portal only against
    that area's frustum. Every one of the 757 lies outside the view by
    legacy's own box test, and there is no other difference. The RFC check
    is amended to allow exactly these ("Culling amendment" under K5).
  - The pooled draw list equals the serial one item for item in all 351
    views.
- **Not done:** the product still draws the world and props through the
  legacy lists. Pixels, the frame-time budget and the Fold7 remain.

## K3 frame time on desktop (2026-09-28)

The frame allowance is 1.05x the median and 1.10x the p99 against K0. It
was judged by `frame_pacing.py` at 1920x1080, mode 0, interleaved against
the binaries the K0 record was measured with
(`/tmp/claude-1000/rc/desk-1/runtime`, 2026-09-26). The K0 record's
absolute 4.434 ms doesn't reproduce on this host today even with those
binaries (5.3 to 6.7 ms per round), so the allowance is applied to the
same-session reference, as RFC 0005 does on a noisy host.

| Run | Current (A) | K0 binaries (B) | A/B | Allowance |
| --- | --- | --- | --- | --- |
| 5 rounds, host load 4 to 8: median of round medians | 5.939 ms | 5.777 ms | 1.028 | 1.05 pass |
| same: median of round p99s | 10.22 ms | 9.35 ms | 1.093 | 1.10 pass |
| 3 rounds, host load 3 to 4 | 5.436 ms | 5.391 ms | 1.008 | pass |

Also found:

- The hitch counts differ between runs because the host's frame times are
  bimodal: runs sit near 5.5 ms or near 10 to 14 ms. The hitch threshold is
  relative to each run's own median, so the counts don't separate the
  builds. A core-on run with a 5.8 ms median had its p90 at 14 ms.
- The profile of a core-on run puts the core's own frame work (graph build
  and compile, the stage-pass executor, the null device's encoder) under
  0.5% of samples.
- Emit is 13% slower than with the K0 binaries (3.38 against 2.99 ms). It
  sits inside the frame allowance and is recorded for R32-FRAME-PACING.

Frame time passes on the desktop. The Fold7 run remains, and K3 stays open
for it alone. The device isn't attached.

The K1 frame-time check is the same one, and the K0 binaries predate K1,
so the measurement closes K1's desktop frame time too. K1 is open only for
the Fold7.

## K5 plan: drawing the world and props from the scene (2026-09-28)

This is the design for K5's open checks ("Pixels", "Submission cost").
K6–K8's product items rest on the same steps. It was written from the
current code. Step 2 and the adapter half of step 3 are done; see the next
section.

Facts:
- Two graphs run each frame. The legacy frame executor runs the backend's
  stage passes on the backend's own device, the host device's port
  (`ILegacyFrameSource::Device()`), which is a real Vulkan device. The core
  renderer's bookkeeping frame (legacy-stream and present features, stage
  markers) runs on the composed device, which is null in the Portal
  product.
- The backend's `VkDevice` is already an adapter `VulkanDevice`: K1's "one
  Vulkan stack" (`host_binding.h`).
- The backend buffers each frame as an ordered stream of draw records
  (`DynDraw`: draw, clear, copy, query, scene capture). It records them
  into the scene stage at frame end.
- The world must draw inside that stream: after the view's clear and the 3D
  skybox, and before models and translucents. A pass before or after the
  scene stage would be cleared or drawn over.

Steps, each its own slice with its own oracle:
1. **One device for the frame.** The legacy executor's graph already runs
   on the backend's device, so core passes join that graph. Folding the
   renderer's bookkeeping frame into it, so there is one graph per frame,
   follows. Dedicated and test products keep the null device.
2. **The backend's scene targets as port textures.** The adapter imports a
   native image and its current layout as a `TextureId` that the backend
   keeps in step as it transitions. This is private to the Vulkan family
   (CAP007). Oracle: a port pass reads and writes an imported target with
   sync validation silent.
3. **A core pass inside the stream.** A new record kind marks where the
   core draws. The frontend inserts it when the client marks
   `RENDER_STAGE_OPAQUE` for a view. The scene stage records the core
   pass's port commands as a section after its host record, and the
   backend's replay runs the section at the marker, between closing its
   render pass and reopening it (so the replay loop is not split). Oracle:
   an empty core pass leaves every pixel family byte-identical.
4. **World and props content.**
   - World: batches per (material, lightmap page) from the BSP surfaces,
     in `render.resources`' mesh cache.
   - Materials: through `MapVariables` into their families.
   - Textures: the backend's own images, imported (no second decode), in
     the texture cache.
   - Static props: meshes through the model path.
   - The legacy world lists skip what the core draws, by leaf and prop
     from K5's culling.
   - Oracles: K0 views within the pixel-family tolerance, and
     `frame_pacing.py` main-thread submission time against K5's 30% target.

Order: 1, 2, 3, then the world, then static props.

## K2 closure: done (2026-09-28)

| K2 check | Evidence | Result |
| --- | --- | --- |
| Graph suite | `render.graph.v1` null (31, g++/clang++, TSan lane) and `render.graph.v1.vulkan` (the 1,000 random graphs with real work, serial and pooled, on RADV) | pass |
| Independent model agrees | G7 on 1,000 seeded graphs | pass |
| Bad graphs caught | `render.graph.v1.sensitivity`, 5 of 5 | pass |
| Synchronization validated | the validation layer over all 16 pixel families in both HDR modes: 32 runs, 0 validation messages; and a `testchmb_a_01` product boot: 0 messages. Integer `sky` keeps its recorded, unrelated magenta failure (R32-LEGACY-SHADERS) with no message. **Correction (same day):** these runs had the layer without synchronization validation. The adapter enables it on its own instance, but the host instance the backend creates under `-vkvalidate` did not. The host instance now enables it too (next section), and the 32 runs and the boot were repeated with it on: 0 messages | pass |
| Serial equals pooled | G9 (`render.graph.recording`) on the 1,000 graphs | pass |
| Pixels unchanged | the pixel families are byte-identical before and after the graph frames (K1 record, 17 families both HDR modes; K3 record, 32 of 32) | pass |

Present, gamma, MSAA resolve, capture and queued compute run as passes of
the frame graph (K3 slice 3). Their commands are still the backend's own,
and their resources become port textures with the K5 plan's step 2.

## K1 and K3 closure: done under binding rule 7 (2026-09-28)

User decision (2026-09-28): "close perf gates". Binding rule 7 ("Look first,
then optimize") makes every time, cost and budget check a *(perf)* check.
Such a check is measured and recorded, and it never holds a gate open. K1
and K3 each had one open item, the Fold7 frame time, which is a *(perf)*
check. Every other check of both gates passes on its required profiles:

- K1: see the [K1 record](#k1-device-port-and-the-vulkan-and-null-adapters-2026-09-28).
  Port suite, bad adapters, backend-neutral port, one Vulkan stack, no idle
  waits, pixels, boots and resize, and feature support all pass.
- K3: see [K3 slice 3](#k3-slice-3-stage-passes-and-the-queued-tsan-lane-2026-09-28).
  Stage passes, side channels gone, pixels, views and draw state unchanged
  in both queued modes, boots, and the queued TSan lane all pass.
- K0 was met, and K2 [closed](#k2-closure-done-2026-09-28).

So K1 (R86, with K0) and K3 (R87, with K2) are `done`.

| *(perf)* check | Recorded | Optimization item |
| --- | --- | --- |
| K1/K3 frame time, desktop | [interleaved against the K0 binaries](#k3-frame-time-on-desktop-2026-09-28): median 1.028x, p99 1.093x, within the allowance | none |
| K1/K3 frame time, Fold7 | not measured: the device was folded and locked, then not attached | run `frame_pacing_device.py --platform android` (install with `adb install -r` only, never uninstall) and record the result against the Fold7 K0 row |
| Emit on desktop | 13% slower than the K0 binaries (3.38 against 2.99 ms), inside the allowance | R32-FRAME-PACING |

## K5 steps 2 and 3a: imported scene targets and host-run sections (2026-09-28)

Step 2 of the K5 plan, and the adapter half of step 3. Both are host interop,
private to the Vulkan family (`render/device/vulkan/host_device.h`, CAP007).

- **`IHostDevice::ImportImage`.** A host image becomes a port `TextureId`
  with a *home* usage, the one whose layout the host keeps it in. Host work
  in the same encoder must find it at home, and so must the end of the
  submission; `Submit` refuses otherwise (`kInvalidState`). The adapter
  orders the port's first access after host work, and host work after the
  port's writes. It owns only the views: `Release` frees them after the
  token and leaves the image with the host.
- **Sections.** `BeginSection`/`EndSection` bracket port commands recorded
  after a `RecordNative`, and the native record calls `RunSection( cmd,
  index )` at the point the section belongs. Sections run once each and in
  order: asking for one runs the earlier ones, and the ones never asked for
  run when the record returns. Validation treats a section's bounds as host
  work boundaries. The backend's scene replay can then run a core pass at
  a record of its stream without splitting its loop.
- **`Format::kD32FloatS8`.** RADV has no D24S8, so the backend's scene
  depth is D32S8. The port gains the format, and `HasStencil()` replaces the
  D24S8 special cases in the Vulkan adapter.
- **Synchronization validation on the host instance.** The host instance
  now enables it whenever the layer is on, as the adapter's own instance
  does. This corrects the K2 closure's evidence (see its table).

| Check | Evidence | Result |
| --- | --- | --- |
| Import clauses | `render.device.v2.vulkan`: `vulkan.import` and `vulkan.section`. Host work, port commands and host work in one encoder, over an RGBA8 and a D32S8 image; each side reads the other's writes. A section runs mid-record between the host's clear and its readback. Asking for section 1 runs section 0 first, and an unasked section runs after the record. Bad homes, imports away from home at host work or at the end, and 5 of 5 bad sections are refused. Release leaves the image. 745 checks on RADV, g++ and clang++ (was 595) | pass |
| Sync validation silent | the import and section runs report 0 messages. The negative control, a host that skips its own barrier, is reported (`WRITE_AFTER_WRITE`), so validation is on | pass |
| Product unchanged | the backend is unchanged except for the host instance. All 16 pixel families in both HDR modes pass under `-vkvalidate` with synchronization validation: 32 runs, 0 messages (integer `sky` keeps its recorded failure). A `testchmb_a_01` boot: 0 messages | pass |

Also in this slice, from the Hammer session's report: the `unlit` and
`lightmapped` families passed `$alphatestreference` through unchanged, so
`$alphatest` with no reference cut nothing. The legacy shaders call
`AlphaFunc` only for a reference above zero, which leaves the default
state's 0.7, and D3D9 holds the reference as a byte. `detail::AlphaTestReference`
owns that rule for both families. Each suite gains two claim checks (0.7 as
178/255, and 0.5 as 127/255). The port-pixel cases still pass: unlit 68 and
lightmapped 95 checks. `vertexlit` already held the byte, and its 0.7 is
the Hammer session's to adopt.

Next: step 3's backend half (next section).

## K5 step 3: core passes at slots of the legacy stream (2026-09-28)

The backend half of the K5 plan's step 3. A core pass now runs inside the
backend's scene, at a *slot* of its stream
(`public/render/legacy/core_passes.h`):

- **Marking.** The frontend's stage hook queues a slot on the backend's
  `ICorePassSlots` at each stage its recorder asks for. The slot goes
  through the material system's render call queue, as the capability
  adapters queue their calls, so it lands among the stream's draws in frame
  order in both queued modes. Its tag holds the stage and the view depth.
- **Recording.** The backend appends a `kRecordCorePass` record. When the
  scene pass records, it records the recorder's pass for each slot as a
  section after its scene record (`RecordCorePassSections`). The slot's
  target is the back buffer and its depth (or the multisampled pair),
  imported on first use (homes: color attachment, depth write) and released
  with them. Render-target textures are not imported yet.
- **Replay.** At the slot the replay ends its render pass and runs the
  section (`RunSection`). It then rebinds its stream buffers and pipeline;
  the next record reopens a pass that loads what the core drew.
  `FirstPassWantsSrgb` and the view scan treat the slot as a pass break.
  Without a recorder, or with no slot, the stream replays as before.
- **Probe.** Until step 4 the frontend's recorder is a probe chosen by
  `-render-core-passes` (`RenderCoreConfig::corePasses`). `empty` records
  only a label at each view's opaque stage. `seeded-clear` clears the slot's
  color target, as its negative control.
- **Found.** The client marks `RENDER_STAGE_OPAQUE` twice per view: before
  `DrawWorld` and before `DrawOpaqueRenderables`. The seeded clear at the
  second slot wipes the world, so the whole frame turns magenta. Step 4's
  world pass must take the view's first opaque slot.

| Check | Evidence | Result |
| --- | --- | --- |
| Slots in frame order | `render.legacy-capabilities` (18): a slot is marked at once without a queue, and lands among the queued calls (`...; shadow 44; slot 259; context-b; ...`); the seeded unordered adapter fails it | pass |
| Probe composition | `render.composition` P6 (22): an unknown probe fails by name; `empty` queues one slot per view, at its opaque stage with depth 1; no probe queues none | pass |
| Empty probe changes nothing | `testchmb_a_01` headless, `-deterministicrender`, screenshot compared with no probe. Sync (`mat_queue_mode 0`): byte-identical (0 of 786,432 pixels differ; two runs without the probe differ from each other in 9,223 pixels by at most 2 levels), 824 slot sections ran. Queued (`mat_queue_mode 2`): byte-identical, 826 sections ran. Queued, both runs without the probe are identical too | pass |
| Negative control | `seeded-clear`: every pixel differs in both modes (magenta at the center) | detected |
| Sync validation | `empty` under `-vkvalidate` (host instance with synchronization validation): 0 messages, 820 sections | pass |
| No regression | all 16 pixel families in both HDR modes on the new backend: 31 of 32 pass, and integer `sky` keeps its recorded failure. The static-composition product builds, and `static_composition.py check` passes (22 linked entries) | pass |

Next: step 4, the world drawn by the core (next section). The world's slot is
marked by the engine in R_DrawWorldLists, with a forwarded tag, not taken from
the positional opaque stage marker.

## K5 step 4a: block-compressed formats in the port, clause D19 (2026-09-28)

Step 4 draws the world from the scene with the backend's own images. Those
are mostly BC-compressed (D3D9's DXT1/DXT3/DXT5), and `render.device.v2` had
no compressed format. This slice adds them.

- **Formats.** `kBC1Unorm`/`kBC1Srgb`, `kBC2Unorm`/`kBC2Srgb`,
  `kBC3Unorm`/`kBC3Srgb`, `kBC4Unorm` and `kBC5Unorm`, behind
  `Capability::kTextureCompressionBC`. `BlockOf`, `RegionBytes` and
  `CopyRegionAligned` own block sizes and copy alignment. The adapters' copy
  checks and the null adapter's storage use them instead of texel sizes.
  BC1 is Vulkan's `BC1_RGBA`, as the backend uses, so D3D9's one-bit alpha
  survives.
- **Rules (shared validation).** Sampled and copied only, one sample, no
  volume. A copy covers whole blocks or reaches the mip's edge, and a clear
  is refused.
- **Vulkan.** The adapter enables `textureCompressionBC` when the device has
  it, and claims the capability. The null adapter claims it too, and stores
  whole blocks.

| Check | Evidence | Result |
| --- | --- | --- |
| D19, shared suite | null 411 checks, Vulkan 800 (RADV): BC1 and BC3 with a 12x6 mip and a 6x3 mip copy in and back unchanged; attachment and multisampled descriptions fail; a split block and a clear are refused | pass |
| Decode | Vulkan: a BC1 block in three-color mode samples as blue, red and two transparent blacks, D3D9's DXT1 | pass |
| Bad adapter | `render.device.v2.sensitivity`: an adapter that creates a block-compressed attachment fails D19 (13 checks) | detected |

Next (4b): the backend's managed textures and lightmap pages imported as
port textures, which needs `kRGBA16Unorm` for integer-HDR lightmap pages.

## K5 step 4 (first slice): the BSP world drawn by the core (2026-09-28)

The core now draws the BSP world surfaces its material model takes, inside
the legacy scene, and legacy skips exactly those. It is off by default
(`r_core_world 1`, or `./play --core-world`).

- **One material model (user direction, 2026-09-28).** The legacy material
  system's special cases are degenerate cases of one model (RFC 0016
  "Materials"). The world pass names no family: `render.material`'s new
  `ProgramResolver` (program_resolver.h) is the one place that maps a
  material to a program.
  - UnlitGeneric is the lightmapped term with lighting fixed at one
    (`flags.w`).
  - The frame terms (`material::FrameTerms`, the lightmapped frame group)
    are the lightmap scale for the pages' encoding (2^2.2 for LDR gamma
    pages, 16 for integer-HDR pages), the output tone-map scale, and the
    output encoding. The encoding is hardware sRGB, or in-shader onto a unorm
    target when the back buffer has no sRGB view, with one
    `LinearToSrgb` in render/shaders/common/color_encoding.glsl.
  - `ProgramTexture::srgb` means "linear values wanted": gamma-encoded
    images decode through their sRGB view, linear images pass as they are.
  - A surface without a lightmap page samples a neutral white.
  - A material the model can't draw stays legacy, with a named gap in the
    stats.
- **render.pass.world** (new module, layer 6). SetWorld takes the world's
  vertices, fan indices, surfaces and materials (variables and texture
  handles). QueueView records a view's visible taken surfaces, its
  world-to-clip and its viewport, and returns a forwarded tag. Record draws
  at the slot:
  - into the back buffer (or the multisampled pair; the families take a
    sample count);
  - with the backend's imported textures and samplers
    (`ICoreTextures::Import`/`Sampler`, `kRGBA16Unorm` for integer-HDR
    pages);
  - with the frame terms captured when the slot was marked.

  Queued views carry their world generation. Recorded views are kept so a
  re-recorded stream draws the same views: `ReadPixels` re-runs the frame
  for a screenshot. The previous world's objects are released behind the
  submitted token, and `ICorePassRecorder::ReleaseDevice` releases them
  before the backend's device goes.
- **Composition.** `IRenderCoreWorld` (render_core_world.h, engine-facing
  and plain) over the pass. The frontend forwards high-bit tags to it
  (`SetForwardedRecorder`), and texture and lightmap-page handles come
  through the render call queue host (`textureHandle`,
  `lightmapPageHandle`).
- **Engine.** `render_core_world_draw.cpp` extracts the surfaces at the end
  of R_LevelInit. R_DrawWorldLists queues the outermost back-buffer view's
  taken, non-dynamic surfaces when the view has no fog and is not a shadow,
  SSAO, reflection or refraction list; `Shader_DrawChainsStatic` then skips
  them.
  - `r_core_world_stats` prints the counts, the gaps and the claimed
    materials.
  - `r_core_world_isolate 1` makes legacy draw only the taken surfaces (the
    pixel oracle).
  - `r_core_world 3` is the negative control: the surfaces are skipped and
    not drawn.

| Check | Evidence | Result |
| --- | --- | --- |
| Suites | `render.world.null` W1–W5 (11): claims and named gaps, a slot's draw with its pages and the neutral white, re-record, stale-world and unclaimed-surface counting. `render.opaque` O6 and `render.opaque.null` N4 with per-layout frame groups. `render.family.lightmapped` (95, the frame group at LDR defaults) and `render.family.unlit`; composition (22), capabilities (18), device null (411) and Vulkan (800). g++ and clang++ | pass |
| Coverage, testchmb_a_01 | 13 of 133 materials, 1,243 of 5,398 surfaces. Gaps: `$envmap` 98 materials, `$detail` 17, Refract 4, `$bumpmap` 1. Views with fog are not taken yet, so the escape maps draw nothing through the core | recorded |
| Views draw | integer HDR, 4x MSAA, headless: 201–204 views queued and drawn per boot, 0 failed; `mat_queue_mode 2`: 202, 0 failed | pass |
| Sync validation | `-vkvalidate` (synchronization validation on the host instance): 0 messages | pass |
| Pixels, isolated | `r_core_world_isolate 1`, `-deterministicrender`. The negative control (`r_core_world 3`) differs from legacy in 28,686 pixels (9,896 by more than 8 levels). With the core drawing, the core-attributable residual is 538 pixels over 8 levels, max 49, on the alpha-tested `metalgrate018` and one edge line; the lightmapped floor matches at sampled points. The portal's animated particles differ between runs of either mode (2,784 pixels over 8 levels between two legacy runs) | partial |

Open, in order:
- the alpha-tested grate residual (sampling or alpha-to-coverage);
- the fog view term (Black, from source-engine-b7's port, is its first
  consumer);
- the env map term (98 materials);
- detail and bump;
- render-target and nested views (stencil-equal and clip-plane oracle cases);
- the Submission cost row (render_submission, both queued modes);
- static props.

## K5 step 4b: exact claims, no escape hatches, and the review fixes (2026-09-28)

User direction (2026-09-28): "make it fail terribly. Config it with a convar
to prevent escape hatches. Mods and materials that can't be ported get
legacy, but nothing else." RFC 0016 "Materials" records the rule.

- **Failures are fatal.** `r_core_world_strict` (default 1) makes a view or
  a claimed material that the core fails to draw a `Sys_Error` naming the
  reason. It is checked on the main thread at each view and at level
  shutdown, so it fires within a frame or two in queued mode. With 0, each
  failure is a warning and the surfaces stay undrawn. Legacy never draws
  what the core claimed. `IRenderCoreWorld::Failures()` is the cheap
  counter. `r_core_world_seed_failure <material>` is the negative control:
  that material reaches the core without texture handles.
- **Claims are exact.** The engine now sends the material flags as their
  VMT keys (`$translucent`, `$alphatest`, `$allowalphatocoverage`, …); the
  `$flags` words used to hide them. Every variable the model does not read
  must hold legacy's neutral value for its shader. That value comes from a
  material of the same shader with no VMT variables, initialized by the
  shader (`NeutralMaterials` in render_core_world_draw.cpp), falling back to
  the shader's declared default (`MaterialDesc::declaredDefaults`,
  `UnreadVariable` in program_resolver.cpp). A variable with no neutral
  value keeps the material out.
  - Correction to step 4's evidence: the 13 materials it claimed included
    `metal/metalgrate018` (alpha-to-coverage) and `metalgrate018b`
    (translucent), which the core drew opaque because it never saw their
    flags. That was the "alpha-tested grate residual". Honest coverage on
    testchmb_a_01 is 6 of 133 materials, 848 of 5,398 surfaces.
- **Review fixes** (source-engine-60's review of cf7a3615):
  - retired objects are released at a slot of a later frame, behind that
    frame's submitted token (`CorePassTarget::frame`, the backend's
    `m_submitSerial + 1`), not behind the host's `SubmittedValue` inside
    the frame that used them;
  - one set of objects per target format (at most four), so an MSAA toggle
    mid-frame no longer retires the world;
  - a capture that re-records an earlier world's slot fails alone, and the
    next world's queued views stay (recorded views outlive SetWorld, and the
    queue pops only views issued before the slot);
  - the destructor drops handles without calling a device that may be gone.

| Check | Evidence | Result |
| --- | --- | --- |
| Suites | `render.world.null` 20 checks (W6–W9 new). Mutants: releasing in the same frame fails W8; the old queue pop fails W7. The material, family, opaque and composition suites (15) pass on g++ and clang++ (`render.composition` needed `jobsystem/pooled_executor.cpp` in its sources) | pass |
| Coverage, testchmb_a_01 | 6 of 133 materials, 848 of 5,398 surfaces. Gaps: `$envmap` 98, `$detail` 17, Refract 4, `$selfillum` 4, blended 2, `$allowalphatocoverage` 1, `$bumpmap` 1 | recorded |
| Strict boot | `r_core_world 1`: 205 views drawn, 0 failed, no `Sys_Error` | pass |
| Seeded failure | `r_core_world_seed_failure metal/metalwall_bts_005a`, strict: `Sys_Error` naming the material and its texture, and the boot fails. Strict 0: every view warns, and 615 surfaces drawn instead of 4,510; legacy draws none of the claimed surfaces | pass |
| Level change | `mat_queue_mode 2`, `-vkvalidate`, testchmb_a_01, then `map testchmb_a_02`: both worlds taken (848, then 401 surfaces), no failure, 0 validation messages | pass |
| Pixels, isolated | `-deterministicrender`, `r_core_world_isolate 1`: core against legacy 12 pixels over 8 levels (max 31), 78 over 2. Two legacy runs: 1 over 8 (max 10). Negative control (`r_core_world 3`): 861 pixels over 8 (max 111) | pass |
| Queued TSan lane (source-engine-60, on cf7a3615) | build-tsan-queued, testchmb_a_01 in `mat_queue_mode` 0 and 2 with `r_core_world 1`: 3,889 reports, 0 in the render core, all in triaged families; one new family, `driver.mesa-queue-barrier` (RADV's disk-cache thread, a known TSan barrier false positive inside the driver). `tsan_triage.py` now also flags `render::` frames in access stacks (64c3e738) | pass |

Reproduce (headless, from the worktree):

```sh
python3 tools/quality/conformance.py check --cxx g++ --suite render.world.null
python3 tools/quality/portal_boot.py --runtime ../source-engine/run/runtime --build <install> \
  --renderer native-vulkan --headless --map testchmb_a_01 --engine-arg=+sv_cheats \
  --engine-arg=1 --engine-arg=+r_core_world --engine-arg=1 \
  [--engine-arg=+r_core_world_seed_failure --engine-arg=metal/metalwall_bts_005a] \
  --console-command=r_core_world_stats --out <dir>
RENDER_TSAN_BUILD=<tsan install> python3 tools/render/tsan_triage.py run --build <tsan install> \
  --runtime run/runtime --out <dir> --engine-arg=+sv_cheats --engine-arg=1 \
  --engine-arg=+r_core_world --engine-arg=1
```

- **Skipped is not failed** (source-engine-60, 2026-09-28): each queued view
  carries its host frame (`WorldView::hostFrame`, `host_framecount`). A view
  whose slot never recorded is `viewsSkipped` when no slot of its host
  frame recorded: the backend never recorded that frame (a resize, a lost
  surface, a dropped queued frame). If another slot of its frame recorded,
  it is a failure and fatal under strict. `render.world.null` W10 covers
  both. Resize stress, run in a private D-Bus session and headless mutter
  (`--no-mouse`, 1920x1080): queued and sync both pass with `r_core_world
  1` and strict on, as does the sync control with the core world off. No
  failure, no skip turned fatal.
  - Correction: an earlier version of this entry reported a sync failure
    ("presents scaled … at settled sizes") that predated this work. Those
    runs weren't isolated: `portal_boot --resize-stress` makes no
    compositor, and they ran on the user's live session. They are not
    evidence. source-engine-60 found no failure at HEAD either.

Open, in order: the surface-model phases (next section), starting with S1
(the translucent-stage world slot, render state and the fog view term); then
render-target and nested views, the Submission cost row, and static props.

## S1 slice: the fog view term, and UnlitGeneric's vertex color (2026-09-28)

- **Fog is a frame term.** The backend captures the view's fog when a slot
  is marked (`legacy::CorePassFog`: type, linear tone-scaled color,
  parameters, eye z; the same values SetPixelShaderFogParams and
  UpdatePixelFogColorConstant give a pass writing sRGB). The world pass
  hands it to the `lightmapped` frame group, and lightmapped.frag applies
  CalcPixelFogFactor and BlendPixelFog as the port does. Fogged views are
  now the core's, and the engine no longer excludes them.
- **Gamma vertex color is its own term** (source-engine-10's finding).
  UnlitGeneric's port decodes vertex color per vertex (pow 2.2), and
  LightmappedGeneric's doesn't. The resolver's unlit case set only lighting
  one, so unlit materials with `$vertexcolor` drew darker than their port.
  `LightmappedConstants::state.y` now decodes in the vertex stage, and the
  unlit resolution and the editor preview set it.
- The slot's terms move out of the draw record into a per-frame list
  (`CVulkanContext::CorePassTerms`).

| Check | Evidence | Result |
| --- | --- | --- |
| Family | `render.family.lightmapped` 98: the port cases are unchanged (no fog by default), and the range-fog case is within 2 levels of the formula. Mutant: an unsquared factor fails. g++ and clang++ | pass |
| escape_00, isolated | `-deterministicrender`, `r_core_world_isolate 1`: 205 fogged views drawn by the core, 0 failed. With `r_dynamic 0` the core against legacy is 16 pixels over 8 levels (max 17), all at two lamps. The negative control is 39,847 pixels over 8 | pass, outside the lamps |
| escape_00, lamps | with dynamic lighting on, 2,074 pixels over 8 levels (max 31) at the same two lamps, and the core is darker. The ×12 difference is concentric rings: a small lightmap sampling offset on steep gradients, plus a dynamic contribution not found yet. No dynamic lightmap rebuild runs on this map (a counter on R_RenderDynamicLightmaps read 0), and re-importing the pages every slot changes nothing | open |
| Queued, validated | escape_00, `mat_queue_mode 2`, `-vkvalidate`: 205 views, 0 failed, 0 validation messages | pass |

Open: the lamp residual (next), then the rest of S1: blending in the
translucent stage, `$decal`, `$nocull`, `$ignorez`, `$nofog`, and the
per-material fog color (black for additive, grey for mod2x).

## Port clause D20: specialization constants (2026-09-28)

The surface model's permutations are the set of non-neutral terms, so a
neutral term should cost nothing at runtime (the tvOS 60 fps work showed that
specialized combos matter on tile GPUs). The port had no specialization
constants, and they are now clause D20: `PipelineDesc::constants` holds
`{stage, id, 32-bit value}`, validation refuses a duplicate id in a stage, and
the Vulkan adapter passes them as `VkSpecializationInfo`.
`unittests/rendertest/core/device/shaders/specialized.frag` is a new
committed fixture, built byte-identically by the pinned compiler
(`shader_toolchain.py check`: 62 passed, 0 failed).

| Check | Evidence | Result |
| --- | --- | --- |
| Shared suite | D20 on null (437 checks) and Vulkan (850): a duplicate id fails `kInvalidDescription`, an undeclared id is ignored, the default draws red and constant 7 = 1 draws green. g++ and clang++ | pass |
| Mutant | the Vulkan adapter with its specialization info dropped fails "the constant's value reaches the shader" | detected |

## S2–S5 for LightmappedGeneric: the surface term matches its port (2026-09-28)

The `lightmapped` family grew into the lit surface term the plan describes.
It is the port's LightmappedGeneric arithmetic, with each term a
specialization constant (D20):
- bump: RNM with the three bumped pages, ssbump, `$nodiffusebumplighting`;
- env map: cube maps with `$envmapmask`, base-alpha and normal-alpha masks,
  tint, contrast, saturation and fresnel, including the pixel fast path's
  quirk (contrast applies only with saturation, else 0 or 1);
- detail: TextureCombine modes 0–4 and 7–9, and 0 and 1 over a bump map.
  Detail reads through sRGB only in mode 1, as the helper's
  `EnableSRGBRead( SAMPLER12, mode == 1 )` does;
- self-illumination with its tint;
- Portal 2's `$ssbumpmathfix` and `$envmaplightscale`.

The material layout is fixed: base, env map (a cube), mask, bump map and
detail, and a term that is off binds a neutral texture of its dimension.
`GroupResidency` provides those for every program user (the opaque pass,
Hammer's viewport), and the world pass provides its own.

Other parts of the slice:
- A second vertex layout, the surface vertex (72 bytes: normal, tangents,
  the bumped pages' offset). `ProgramResolver::Create` takes a
  `VertexLayout`: flat by default (Hammer), surface for the world pass. A
  flat resolver refuses surface terms by name.
- The engine's extraction builds the surface vertex as
  `BuildMSurfaceVertexArrays` builds the static mesh.
- A detail texture flagged `TEXTUREFLAGS_SSBUMP` (modes 10 and 11) sends
  `$detail_ssbump`, which the model doesn't read, so it is named as a gap.
- Per-view textures (`env_cubemap`, `_rt_*`) are named gaps.
- `ICoreTextures::Import` imports cube maps.
- The frame terms add the eye, ENV_MAP_SCALE, mat_specular and the running
  game's ssbump policy. The backend's `SsbumpBasisNormalized` is its one
  owner: Portal 2's shaders scale every ssbump by 1/√3.
- Fixed: the engine queued brush surface indices where the world pass
  expects its own entries. Portal 1's chambers happened to match, because
  every surface is eligible. Portal 2's maps don't, and
  `./play_p2 --core-world sp_a2_core` failed strictly with "a view named
  surfaces the pass does not draw", as the user found. The engine now
  translates indices through `entryOf`.

| Check | Evidence | Result |
| --- | --- | --- |
| Family against the port | `render.family.lightmapped`: 17 cases, 9 of them new (bump, nodiffusebumplighting, ssbump, detail mod2x with tint, detail additive over a bump, three env map cases, selfillum tint), port pixels recorded (`legacy_shader_conformance.py … -vklegacylightmapped`). Worst difference 0 on every case, g++ and clang++. Mutants: an unnormalized RNM sum fails the bump case; the Vulkan adapter dropping specialization constants fails D20 | pass |
| Suites | family (192 checks), unlit, world (26), composition, opaque (Vulkan and null, with neutral textures), material v2, VMT corpus, material programs; g++ and clang++ | pass |
| testchmb_a_01 | 124 of 133 materials, 5,064 of 5,398 surfaces (was 6 and 848). Isolated against legacy: 2 pixels over 8 levels (max 20). Negative control: 783,959 | pass |
| sp_a2_core (Portal 2) | 38 of 52 materials, 2,494 of 4,623 surfaces, 206 views, 0 failed. Isolated against legacy: max 2 levels, 0 pixels over 2 (before the ssbump policy: 80,705 over 8) | pass |
| Validation | sp_a2_core, `mat_queue_mode 2`, `-vkvalidate`: 12 messages, the same 12 with the core world off (a legacy vertex output at location 5 with no fragment input; a cube view at the legacy port's base texture binding): pre-existing, Portal 2's legacy native path | not the core's |

Open: the retail-map smoke test (every Portal and Portal 2 map with the core
world on, strict; a subagent is building it), two base textures (WVT),
texture transforms, blending in the translucent stage, the models' surface
(S7).

## The surface-model plan (2026-09-28)

User direction: "have a plan for how to modernize most materials with them
being degenerate cases in the new expanded system". The plan was recorded
in RFC 0016's surface-model section, originally titled "legacy materials as
degenerate cases". At this date it gave:
- one `surface` program whose terms all have neutral values;
- legacy shaders as exact points, checked against the native ports;
- modern points as opt-in data (a rule table and sidecars, checked against
  Cycles);
- phases S0–S9 ordered from an inventory of 6,000 Portal and 3,738 Portal 2
  VMTs.

The [2026-10-03 material direction](#material-interpretation-direction-2026-10-03-user-decision)
amends the default and visual oracle while retaining these historical counts.

`tools/render/material_inventory.py --out DIR --phases` reproduces the
coverage per phase:

| Phase | Portal materials / world area / faces | Portal 2 materials / world area / faces |
| --- | --- | --- |
| S0 (now) | 22.7% / 2.2% / 19.1% | 19.8% / 61.1% / 41.6% |
| S1 coverage and state | 45.4% / 2.4% / 20.1% | 55.7% / 61.7% / 42.0% |
| S2 specular image | 53.4% / 7.4% / 36.8% | 57.3% / 61.8% / 42.6% |
| S3 normal and basis light | 58.2% / 57.5% / 77.9% | 62.2% / 84.6% / 77.6% |
| S4 detail | 75.4% / 89.5% / 88.4% | 64.9% / 88.2% / 97.8% |
| S5 emission | 80.7% / 97.6% / 99.4% | 72.6% / 88.3% / 98.1% |
| S6 layers | 81.7% / 97.6% / 99.4% | 73.0% / 97.3% / 98.5% |
| S7 model surfaces | 85.4% | 84.4% |
| S8 unlit points | 92.9% | 88.3% |

The rest is the long tail: SpriteCard, Water, Refract, the eye shaders,
Cable, Portal, SolidEnergy, PaintBlob and engine-internal materials. Each
gets a family on the same terms. Mods' shader DLLs stay on the legacy
profiles.

## K11 and RFC 0014: the lab first, then its instruments (2026-09-28)

User goal (2026-09-28): the render core at Source 2 quality or better, in
stack order. RFC 0014 D0–D1 (the debug view catalog and the lighting-model
controls) come first because they are the lab's instruments; then K11 in
`render_lab`; then K10, K5–K6, K7 and K12 in the product. Owner of every
step: this session (source-engine-43, the render-core owner). R95 and
R95-DEBUG-CONTROLS are `active`.

### K11 slice: `render_lab` composes the core alone

`render_lab` (`render/lab/`, module `render.lab`, a strict C++20 program of
the tools product, built when the core's Vulkan adapter and the texture
readers are configured) reads a BSP2 map's world mesh (`WMSH`, decoded by
the format library's new `mapcontainer/world_mesh_decode.h`) and lightmap
page (`LMAP`, the Total layer, linear RGBA16F), resolves every VMT through
`render.material`'s importer and the one program resolver, stages the
textures through `render.resources`, optionally adds a studio model through
`mdl`, and draws through the Vulkan adapter into a linear RGBA16F target,
written as a PFM. It links no engine, material system, legacy frontend,
composition root or SDL. The VTF reader learned the 24-bit RGB888 and BGR888
formats (expanded to RGBA8 with opaque alpha, with a reader test), which the
gallery's content uses.

| Check | Evidence | Result |
| --- | --- | --- |
| Lab composes the core alone | `render.lab.composition` (`tools/render/lab.py composition`): NEEDED is tier0, the Vulkan loader and the C/C++ runtimes; no defined symbol of the legacy frontend, composition root, engine, material system, studiorender or SDL; the RFC 0011 gallery's Cornell box (4 world batches) with its probe sphere model (5 draws, 496 vertices) under the Khronos validation layer with synchronization validation: 0 messages; a missing layer fails the run | pass (27 checks) |
| Scans catch forbidden modules | `render.lab.composition.selftest`: six seeded symbols, SDL and material-system libraries outside the allow list | pass (10 checks) |

Reproduce: `WAFLOCK=.lock-waf-rc-lab ./waf build --target=render_lab`, then
`python3 tools/quality/conformance.py check --suite render.lab.composition
--suite render.lab.composition.selftest`. The tree is configured with
`./waf configure --tools --disable-warns -T release -o build-rc-lab
--render-core-vulkan=on --ktx-source-root=… --ktx-build-root=…`.

Open for K11: everything after composition. The lab still records its draws
straight on the encoder; it moves onto the frame graph with the surface
program (check "Model assembly").

### RFC 0014 D0 in the lab: the view catalog on the core (2026-09-28)

The debug view catalog is on the core and proven in `render_lab` (binding
rule 3). Product wiring (the engine's `cl_render_debug_*` ConVars, the world
pass, the hatch for legacy stream passes, the post bypass) is the next slice,
and D0 closes only after it passes. Decisions are recorded in
[RFC 0014](0014-native-vulkan-and-bsp2-debug-controls.md#implementation-decisions-d0-2026-09-28).

- `render.shader-library`: `debug_view.h` holds the catalog (views 0–23 and
  32–36; 1–17 installed, the others reserved until their terms reach the
  core), the input and term bits, and the specialization constants 100–108.
- `render.frame`:
  - `DebugControls` is in `FrameDesc`, engine-facing, with no dual-ABI type;
  - `ValidateDebugControls` gives a named status and a console message;
  - `DebugSpecializationFor` maps the controls to one program's
    specialization.
- `render.renderer`: validates `FrameDesc::debug` at `BeginFrame`. It keeps
  the last valid value and counts refusals (`debugRejected`,
  `AppliedDebug()`, `LastDebugRejection()`). `RendererDeps::debugPrograms`
  names the programs the filter may select.
- `render/shaders/common/debug_view.glsl`: every formula, the hatch and the
  filtered-out grey.
- Every family program (`lightmapped`, `pbr`, `vertexlit`, `unlit`) and
  `lines` report their inputs and write the view at their single output
  point. Each family makes debug variants of its shipped pipelines
  (`DebugPipeline`); the resolver serves them by `ResolvedProgram`, which now
  carries its program's name.
- The D1 controls the programs can already honor are in the same shaders:
  - the terms that exist: `baked`, `ibl`, `emission`, `clustered`,
    `probes` and `ao`;
  - the BRDF modes, the furnace, and forced roughness and metalness (`pbr`).

  Their D1 checks follow.

| Check | Evidence | Result |
| --- | --- | --- |
| Views match their formulas (lab) | `render.debug-views` (`render_lab suite debug-views --validate`): 82 checks. Views 1–17 on analytic quads within one 8-bit step: the flat and bumped normal, the RNM baked basis, image specular from the program's constants, NaN/Inf/negative from pages that hold them, the checker, depth. The pbr direct and image-specular views equal the frame minus the frame with the term off. The hatch wherever a program lacks the input. The program filter. Validation of 16 malformed or reserved controls. Neutral controls give the shipped pipeline. 0 validation messages | pass |
| Negative programs | `render.debug-views.sensitivity`: the swapped normal fails `view.2.*`; the tone-mapping view fails 20 radiometric and hatch checks; the NaN miss fails `view.16.nan`; shading instead of the hatch fails every hatch check (5 of 5) | pass |
| Default identity | 24 suites on g++ and clang++: the four family suites and their seeded controls, `render.material.programs`, lines, opaque, world, resources (new R7 clause: cube faces), material v2, scene, frame, composition and the three Hammer viewport suites. Family worst differences are unchanged from the runs before the change (pbr 1 on the lit cases, 0 elsewhere). `render.shader-artifacts` and its sensitivity row pass with the new seeded header | pass |
| Default identity, frame time | Portal interleaved A/B, recorded, not blocking (binding rule 7) | with the product slice |

Open for D0: the product slice on native Vulkan Linux with the render
sequence on and off the main thread; the Fold7 run (required for D0).

### RFC 0014 D0 in the product: the views on native Vulkan (2026-09-28)

The catalog now reaches the product.

- **Engine.** The engine's render core host parses the `cl_render_debug_*`
  ConVars (cheat) once per frame into `FrameDesc::debug`.
  - A refused value prints its reason once, and the frame keeps its
    controls.
  - `cl_render_debug_view_program ?` lists the programs.
  - Term names parse through `IRenderer::ParseDebugTerms`: the engine
    reaches the core only through its ports.
- **World pass.** The composition's world gives each queued view the
  renderer's applied controls (`WorldView::debug`). The render sequence
  therefore draws with the frame's value in either queued mode. Each
  program is drawn with its debug pipeline, and a refused debug pipeline
  fails the view loudly.
- **Frame begin.** Under a pixel view (or `cl_render_debug_legacy 2`), the
  composition marks the frame's first slot with
  `kCorePassForwarded | kCorePassLegacyOff`
  (`render/legacy/core_passes.h`). At that slot it records the
  not-applicable hatch over the target (`render.pass.debug`, a new core
  module with `hatch.frag` over `debug_view.glsl`).
- **Backend (core plumbing on a frozen path).** From such a slot to the
  frame's end, the backend's replay records no legacy draw, copy or scene
  capture and only the depth and stencil parts of clears. It presents
  without the monitor gamma ramp; its slots still run. The legacy post
  chain (bloom, color correction) is legacy stream draws, so it is bypassed
  with them. World view tags keep 30 serial bits, so bit 30 is free for the
  flag.

| Check | Evidence | Result |
| --- | --- | --- |
| Views in the product, render sequence on the main thread | `render.debug-views.product.mode0` (`tools/render/debug_views_product.py run --queue-mode 0`), testchmb_a_01. Checker: 391,471 black, 392,995 white, 1,966 edge blends, no other pixel. Reserved view 18 refused, the console naming why, and the checker kept. The program filter greys all 786,432 pixels (every one is lightmapped world). With the core drawing nothing, 786,432 of 786,432 pixels are the hatch at their own coordinates. Neutral controls give a normal frame, which the judge rejects. `legacy 2` keeps the core's shading | pass (11 checks) |
| Render sequence on its own thread | `render.debug-views.product.mode2`: the same counts, and the console shows `mat_queue_mode` 2 | pass (11 checks) |
| Judges catch what they must | `render.debug-views.product.selftest`: a legacy HUD mark in a checker frame, a hatch one pixel out of phase, the checker as hatch, the hatch as filter grey | pass (11 checks) |
| Default identity, product | `core_world_smoke.py run --game portal` on this build: 26 of 26 retail maps pass with the core drawing the world strictly | pass (27 checks) |

### RFC 0014 D1: the lighting-model controls, in the lab and the product (2026-09-29)

D1's controls are `cl_render_debug_brdf`, `_furnace`, `_term`,
`_force_roughness`, `_force_metalness`, `_legacy` and `_claims`. Their
programs' side landed with D0 and is now checked. The product side adds the
frame-end slot for `cl_render_debug_legacy 1` and the claims report, and
deletes the two frozen `VK_DEBUG_LIGHTMAPPED` diagnostics. The decisions are
recorded in
[RFC 0014](0014-native-vulkan-and-bsp2-debug-controls.md#implementation-decisions-d0-product-and-d1-2026-09-29).
Views 18–20 wait for the indirect terms on the core (K11, step d).

| Check | Evidence | Result |
| --- | --- | --- |
| Each term off is the frame without its input | `render.lighting-controls`, bit for bit, and each term on differs. Lightmapped: baked off = a zero page, emission off = `$selfillumtint 0`, ibl off = no `$envmap`. pbr: clustered off = no lights, probes and ibl off = a black cube, ao off = AO 1, emission off = no emission | pass |
| BRDF modes against `pbr_brdf.h` | Modes 1, 2, 3 and 4 match the CPU terms at every pixel of a lit pbr quad within 3/255. Diffuse plus specular equals the full frame within 1.2e-4 | pass |
| Furnace | A white metal sphere reads 1 within 4.9e-4 (one fp16 step) at roughness 0.05, 0.3, 0.6 and 1.0, and a white dielectric too. With compensation off, a rough white metal reads 0.38 (the single-scatter A+B), below the smooth one's 0.9997. A lightmapped surface reads exactly 1 | pass |
| Overrides | Forced roughness and metalness of 102/255 equal the materials authored with them, worst difference 0 | pass |
| Negative program | `render.lighting-controls.sensitivity`: the program built to ignore `baked` fails the term check | pass |
| Product, both queued modes | `render.debug-views.product.mode0` and `.mode2`, 20 checks each. D0's shots, plus four more. `legacy 1` over four yaws of testchmb_a_01 leaves the core's world untinted and shows a legacy-drawn prop magenta (504 pixels at yaw 180). With the core drawing nothing, all 786,432 pixels are tinted. The furnace turns 783,911 pixels white. `term baked` darkens 681,109 pixels. `cl_render_debug_claims` names each program | pass |

Found while building `legacy 1`: the backend records a frame's scene a second
time for a capture, and screenshots and RenderDoc frames are those second
recordings. Per-frame state on the composition's side must therefore be keyed
by the target's frame serial, not consumed by the first recording (RenderDoc
capture `testchmb_a_01_frame492`: the tint was the frame's last draw, and the
redraw was missing).

Frame time (binding rule 7, recorded, not blocking):
- Setup: `frame_pacing.py` on `portal-frame-pacing-v1`, `mat_queue_mode 2`,
  8 interleaved rounds. A is the pre-D0 build (9d422eff), B the landed D1
  build (4f1ace10, which also carries the upstream HDR-output commits).
- Result: warm median A 5.495 ms, B 6.607 ms (B/A 1.20); p99 1.06.
- Reading: both builds are bimodal, A's rounds spanning 4.4–7.8 ms and B's
  4.5–7.9 ms, and B had more rounds in the slow mode. The backend's emit
  and vertex conversion, which neither D0 nor D1 touches, move with the
  frame time round by round (B/A 1.25). The controls add no measurable
  per-frame work at their defaults. It stays an optimization item for the
  step-10 pass: measure on a quiet host with the `frame_pacing` bimodality
  explained first.

**The Fold7 run (required for D0) is blocked.** The phone is on adb, but its
screen is locked with a secure credential (`deviceLocked=1`), so the app
cannot come to the front. Once it is unlocked:
`adb install -r build-android/portal-0.1.0-arm64-v8a-debug.apk` (built from
4f1ace10), then `python3 tools/render/debug_views_product.py run --platform
android --device <serial> --queue-mode 0 --out <dir>` and `--queue-mode 2`.
D0 and D1 pass every non-perf check on Linux in both queued modes.
R95-DEBUG-CONTROLS stays `active`: D0–D1 are done apart from the Fold7 run,
and D2–D7 are step 7 of the goal.

### K11 step (a), slice a1: one surface program, and `pbr` is its point (2026-09-29)

"Model assembly" asks for one surface program that evaluates every term.
This slice makes the two lit programs one:
- `render/material/families/surface.frag` holds the lightmapped point
  (LightmappedGeneric's arithmetic, unchanged) and the pbr point
  (`kSurfacePbr`: the RFC 0007 BRDF under Source's model lighting, from the
  former `pbr.frag`). One output tail (tone scale, fog, encoding) and one
  debug-view tail serve both.
- Three vertex stages: `surface_flat.vert` and `surface_world.vert` (the
  former lightmapped stages) and `surface_model.vert` (the former
  `pbr.vert`). They share one interface; the model lighting block is
  `surface_lighting.glsl`.
- `pbr.frag`, `pbr.vert`, `pbr_lighting.glsl`, `lightmapped.{vert,frag}`
  and `lightmapped_surface.vert` are gone (rule 4).
- One C++ owner, `render::material::SurfaceProgram`
  (`public/render/material/surface_program.h`): its bind groups, vertex
  layouts, pipelines, debug variants and group requests.
  `LightmappedFamily` and `PbrFamily` keep only their claims and delegate
  the rest. `SurfaceConstants`, `SurfaceFrame`, the vertex structs and the
  `kSurface*` term bits have one definition there. Source's model lighting
  (`ModelLighting`, `PackSourceModelLighting`) moves to `model_lighting.h`,
  which the vertexlit family also reads.
- One set of groups for every point:
  - frame: the terms plus the split-sum table;
  - material: the constants plus base, env map, mask, bump or normal map,
    detail, MRAO and emission;
  - draw: the lightmap page plus the model lighting.

  A point that reads none of an input binds the neutral texture or block.
  So the pbr point's lighting is now a draw group (it was a view group),
  and its view direction comes from the frame's eye. `layouts.json` declares
  the program once as `surface`.

| Check | Evidence | Result |
| --- | --- | --- |
| Legacy points unchanged | `render.family.lightmapped` 192 checks, worst difference 0 on every case; `render.family.pbr` 85 checks, worst 1 on the lit cases, as before the change; g++ and clang++ | pass |
| Seeded families still caught | `.seeded-gamma-color`, `.seeded-ignore-normal-map`, `render.family.vertexlit.seeded-ignore-half-lambert` fail as expected | pass |
| Groups and passes | `render.opaque` 20 (O6: the pbr cube reads its lighting from its draw group; without that group it is counted and not drawn), `render.opaque.null` 10, `render.world.null` 26, `render.composition` 22, `render.family.vertexlit` 113 | pass |
| Debug views and controls on the one program | `render.debug-views` 82, `.sensitivity` 5 (the seeded programs are built from `surface.frag`), `render.lighting-controls` 32, `.sensitivity` 2, `render.lab.composition` 27, `.selftest` 10 | pass |
| Hammer viewports | `hammer.adapters.render.viewport`, `.viewport.null`, `.models`, `.viewport.models`, `.service`, `.service.vulkan` on g++ and clang++. V8 counts one more upload: the frame group's neutral split-sum texture, in the residency that holds frame groups | pass |
| Shader artifacts and layouts | `shader_artifacts.py check` (1,379 checks: reflection against the `surface` layout), `shader_toolchain.py check` (82) | pass |
| Product | `core_world_smoke.py run --game portal` on the installed client: the core draws the world strictly on 26 of 26 retail maps (27 checks) | pass |

Frame time: this slice changes no pixels (every family and debug suite is
unchanged), so it records none. The product's world draw groups each gain
a 432-byte neutral lighting block. The Fold7 is still securely locked
(`mScreenLocked=true`).

Not claimed here: the vertexlit and unlit programs are still their own
(`vertexlit.frag`, `unlit.frag`), and `render.lighting.terms` does not exist
yet. Both are the next slices of step (a). The two families each own a
`SurfaceProgram` instance, which is one definition but two sets of layout
objects; the resolver will share one instance when the pbr point enters the
product (K12). The `rc-tools` and `build-hammer-gtk` trees fail before
building ("Can't find env cache mdl", a configuration older than this
slice); the Hammer suites above cover the viewport code.

### K11 step (a), slice a2: unlit and vertexlit are points too (2026-09-29)

The last two family programs join the surface program. Every legacy family
is now a claim over it:
- **unlit** (`kSurfaceUnlit`): the lighting fixed at one, with
  UnlitGeneric's own vertex rules. `$alpha` applies once, the vertex color
  by `$vertexcolor`, and the vertex alpha by `$vertexalpha`. The product
  resolver and the Hammer preview draw this same point.
- **vertexlit** (`kSurfaceVertexLit`, and `kSurfaceHalfLambert`): the model
  vertex stage evaluates Source's per-vertex `DoLighting` from the draw's
  model lighting. It reuses the stage's per-light attenuation and one
  ambient-cube function (`ModelAmbientCube`, shared with the pbr point).
  The family's vertex is now the model vertex.
- `unlit.{vert,frag}` and `vertexlit.{vert,frag}` are deleted (rule 4), and
  `layouts.json` has only the `surface` family.
- `SurfaceFamily` is the families' one adapter base. It owns a program or
  borrows one, so a root can draw every family through one set of layouts.
  The opaque fixtures now do: one frame group, and a neutral draw group
  for a plain cube.

**Found and fixed: the product's unlit point squared `$alpha`.** The
resolver drew UnlitGeneric as the lightmapped point with the lighting at
one. That point takes LightmappedGeneric's alpha rule, so without
`$vertexcolor` it applied `$alpha` twice, and it ignored `$vertexalpha`.
The unlit family's port suite now runs through the product's point
(`family_unlit_translucent_alpha` and `_vertexcolor_alpha` are worst 0).
Only unlit materials with `$alpha` below one, or with `$vertexalpha`,
change pixels.

| Check | Evidence | Result |
| --- | --- | --- |
| Legacy points unchanged against their ports | `render.family.unlit` 68, `.vertexlit` 113, `.lightmapped` 192, `.pbr` 85: worst difference 0 on every unlit, vertexlit and lightmapped case, and 1 on the lit pbr cases as before; g++ and clang++ | pass |
| Seeded families caught | `unlit.seeded-ignore-vertex-color`, `vertexlit.seeded-ignore-half-lambert`, `lightmapped.seeded-gamma-color`, `pbr.seeded-ignore-normal-map` fail as expected | pass |
| One program for every family in a pass | `render.opaque` 20: scenes A–C draw through one borrowed program, one frame group and neutral draw groups; the unresolved cases (no group, a group of another layout, a missing view group) still count; `render.opaque.null` 10 | pass |
| Lab and debug views | `render.debug-views` 82, `.sensitivity` 5, `render.lighting-controls` 32, `.sensitivity` 2, `render.lab.composition` 27 | pass |
| Hammer, world, composition | the six Hammer render suites, `render.world.null` 26, `render.composition` 22 (its row now lists the output pass's sources, which the peer's `CoreOutput` needs); g++ and clang++ | pass |
| Static | stylelint, archlint (the F-Stop ARCH105 findings are another session's), `shader_artifacts.py check` 1,338, `shader_toolchain.py check` 78 | pass |
| Product | the installed client: `core_world_smoke.py run --game portal` 26 of 26 retail maps with the core drawing the world strictly (27 checks); `debug_views_product.py run --queue-mode 2` 20 checks (the unlit claims, the views, the hatch, `legacy 1`, the furnace) | pass |

Frame time: the pixel change is the unlit alpha fix, on a handful of
translucent unlit materials. No frame-time rows were taken for it. The
Fold7 is securely locked, so it is unavailable.

Open for step (a): `render.lighting.terms`, the suite that runs every term
of the model table through this program with a neutral-is-absent mutant
per term. It grows with steps (b) to (g), which add the terms the program
lacks.

### K11 step (b), slice b1: LTC area lights in the surface program (2026-09-29)

The area-light term of `render.lighting.v1`. RFC 0011 defines the
rectangle, its radiance, its window and one-sidedness
(`render.area-light.v1`, `public/render/area_light.h`). This slice
implements the per-pixel evaluation the model table assigns to the core.
- **One GLSL copy**, `render/shaders/common/ltc.glsl`.
  - Both lobes integrate a clamped cosine over the rectangle: the diffuse
    lobe the cosine itself (the exact Lambert form factor), the GGX lobe the
    cosine transformed by a fitted inverse matrix (Heitz, Dupuy, Hill and
    Neubelt 2016).
  - The rectangle is clipped twice: to the surface's horizon, below which
    the BRDF is zero, and to the transformed cosine's own horizon.
  - The edge integral is exact (atan2 of |a x b| and a . b).
  - The table is filtered in full precision in the shader (four texel
    fetches).
- **The pbr point** reads up to 64 area lights from the frame block
  (`SurfaceFrame::areas`, `PackAreaLight`) and the table at frame-group
  bindings 3/4 (`LtcTable()`).
  - The GGX lobe's magnitude and Fresnel split are the split-sum's A and B,
    with the lobe's energy compensation, as the image light's are.
  - `cl_render_debug_term area` turns it off.
  - No legacy point reads area lights (K12's RuntimeLight decides that per
    surface).
- **The table** is `public/render/pbr_ltc_table.h` (64 x 64), fitted by
  `tools/render/ltc_fit/ltc_fit.cpp` to the RFC 0007 lobe of
  `pbr_brdf.h`; `tools/render/ltc_table.py write` regenerates it and
  `check` verifies it. Two fitter defects were found and fixed:
  - The Nelder-Mead stop test was absolute, and a rough lobe's error sits
    far below it, so 586 of 4,096 fits stopped at step 0. It is relative
    now; every fit converges (median 43 steps, maximum 199).
  - The paper's cubed error favours the peak: at roughness 0.5 and N.V
    0.5 the fitted lobe gave 0.74 of the BRDF straight overhead. The L1
    distance, which bounds an area light's integral error, gives 0.96 there.
  - A four-parameter fit (a second shear) was tried and dropped: it
    overestimates the back tail up to 17-fold.
- **Shared lab code**: the suite driver moved into `render/lab/lab_suite.{h,cpp}`
  (outcomes, seeded programs, the command line); the debug-view suites use
  it too.

| Check | Evidence | Result |
| --- | --- | --- |
| Diffuse lobe exact | `render.lab.area-lights`: against `area_light::IrradianceAt` times `pbr_brdf.h`'s diffuse color, every sampled pixel within 0.4 percent + 3e-4, for a ceiling panel, a tilted one, one crossing the horizon, one whose reach ends on the receiver and 64 at once, from an overhead and a grazing view (worst 0.38 percent) | pass |
| GGX lobe as defined | the same suite: against an independent C++ evaluation of the term (table, both clips, split-sum magnitude and compensation), within 2 percent + 2e-4, three lights, four materials, two views. Pixels where the term moves by more than half that band within 0.1 units are ill-conditioned (a narrow highlight's edge at a grazing view, where the rasterizer's position may differ by that much from the ray's) and are skipped, at most 5 percent of a case (worst case 18 of 576) | pass |
| Neutral and sidedness | the term off and no area lights are the same frame bitwise; a one-sided light seen from behind gives the frame without it; a two-sided one gives its front's light within 0.08 percent; diffuse plus specular is the full frame within 0.09 percent | pass |
| Seeded programs | `render.lab.area-lights.sensitivity`: no horizon clip, the LTC matrix transposed and the GGX magnitude dropped are each caught | pass (4) |
| Table | `render.ltc-table`: the committed table is the fit | pass (5) |
| Legacy points unchanged | the family, opaque, world, composition and Hammer rows (18) on g++ and clang++, with the larger frame block | pass |

**Accuracy against the exact light (a measurement, not a pass).** The
first version of the GGX check judged the lobe against an exact quadrature
of `pbr_brdf.h` over the rectangle (mean error at most 5 percent, 90th
percentile at most 15 percent, fixed before the run). The LTC
approximation failed it: at grazing views and for lights near the
horizon, the transformed cosine is too narrow across the plane of
incidence and has no back-scatter tail. A CPU evaluation of the same table
confirmed that the shader is not at fault: a vertical light near the
horizon gets 0.69 of the exact integral at normal view and roughness 0.8.
The RFC defines the term as LTC, so its "Model assembly" oracle is the
term's own definition. The suite still prints the accuracy per case
(`INFO accuracy.*`): energy-weighted mean error, averaged over the cases,
is 5.1 percent from overhead and 17.7 percent at grazing; the worst are
dielectric grazing cases, 34 to 40 percent, where Fresnel also varies
across the lobe. Ground truth is judged by `render.lab.cycles` on the
area-room and cornell-floors fixtures (slice b2). Improving the fit's
representation at grazing (for example two lobes, or a table per Fresnel
term) is an open quality item.

Frame time: the lab only; no product change draws differently (the frame
block grows by 4 KB, and no product path fills it). The Fold7 is locked.

Open for step (b): b2, render_lab lighting `area-room` and `cornell-floors`
from their `light_rect` entities, judged against their Cycles references
(the 16-spp previews certify nothing; the 2048-spp renders are the
fixture set's to produce).

### K11 step (c), slice c1: clustered runtime lights, both lobes (2026-09-29)

The runtime-lights term of `render.lighting.v1`: render.light-set.v1's point
and spot lights, listed per froxel by render.pass.lights (K7's cluster grid
and assignment), evaluated per pixel in the surface program.
- **The view group** (set 1, role kView) is new in the surface program: the
  view's grid (`SurfaceViewGpu`), the froxels' ranges, the index list and
  the light records (`SurfaceLightGpu`, `PackSurfaceLight`). A variant with
  `kSurfaceClustered` reads it. Every other variant binds the program's
  neutral view group, which `MaterialPrograms` keeps once per layout
  (`ProgramRequest::neutralView`). The opaque pass, the world pass,
  Hammer's viewport and the lab bind it when the frame supplies none.
- **Storage buffers in group requests**: `GroupRequest::storage`, uploaded
  once by `GroupResidency` and left in `kStorageRead`; the opaque pass
  declares their reads, and the world pass builds them the same way.
- **One definition of the spot cone**: `light_set::SpotFactor` in
  `public/render/light_set.h` (RFC 0011's contract). It was only in the
  frozen `world_pbr.frag`, which keeps its copy until K12 deletes it. (c2
  replaced this smoothstep with vrad's rule, light set v3.)
  `render/shaders/common/runtime_light.glsl` is the core's one GLSL copy of
  the two falloffs and the cone.
- **Froxel lookup**: `ClusterFroxel` in `surface.frag` mirrors
  render.pass.lights `FroxelAt` exactly (slice, clamped tiles).
- **The pbr point** adds each listed light's diffuse and GGX lobes, gated by
  `cl_render_debug_term clustered`.
- **Shared lab code**: the receiver scene (views, ray hits, materials,
  plane) moves to `render/lab/lab_receiver.{h,cpp}`, and the area-light
  suite uses it.
- The GL agent's `e7fc9911` (the surface program resolved through the
  artifact store) landed first; this slice's bindings come from the store's
  reflection.

| Check | Evidence | Result |
| --- | --- | --- |
| Clustered lights against every light of the set | `render.lab.clustered-lights`: a mixed set (legacy and inverse-square falloffs, bounded and unbounded, points and spots) and 256 small lights, three materials, an overhead and a grazing view. Every sampled pixel (1,024 or 544 a case) is within 0.5 percent + 3e-4 of an oracle summing every light with `light_set.h` and `pbr_brdf.h`, widened by the oracle's own change within 0.1 units (worst 4.9 percent on metal r0.25 at grazing, inside its widened band). A missing listed light, a wrong falloff or a wrong froxel would fail | pass (16) |
| Neutral bitwise | no lights, and the term off, give the frame of the program without the term | pass |
| Seeded programs | `.sensitivity`: slice off by one (fails `clustered.dense`), each list's first light skipped, the inverse-square window dropped (fails `clustered.mixed`) | pass (4) |
| Legacy points unchanged, every user binding set 1 | family, opaque, world, composition and Hammer rows on g++ and clang++ (18 rows each); debug views, lighting controls and area lights in the lab | passes: 18 of 18 on g++ and 18 of 18 on clang++; `debug-views` 82, `lighting-controls` 32 and `area-lights` 40 with `--validate` |

Tolerance change, recorded: the suite first skipped pixels where the
oracle moves more than half the band within 0.1 units (the area-light
suite's rule), capped at 5 percent of a case. The lights' gradients made
most pixels such, so the cap failed; no judged pixel was outside the band.
Each pixel is now judged against its band widened by that movement, which is
the position uncertainty the rule models.

Not in c1, next in step (c): atlas shadows for spots and projectors, the
projector list, sun cascades, and the lightmapped and vertexlit points'
runtime lights. The lists are the serial path's here; the GPU kernel's
equality to it is `render.lights.clusters.gpu`. Frame time: lab only, no
product draw changes; the Fold7 is locked.

### K11 step (c), slice c2: atlas shadows for clustered spots, and vrad's spot rule (2026-09-29)

The direct-visibility term of `render.lighting.v1` for the clustered lights:
a runtime light with an atlas tile is shadowed by it, per pixel, through the
one receiver helper. The same slice fixes the spot cone's one definition.
- **The shadow record's owner.** `ShadowTileGpu` moves from
  render.pass.shadows to render.contracts (`public/render/shadow_tile.h`).
  Its receivers are material programs and sibling passes, and neither may
  depend on render.pass.shadows under CAP011. render.frame, the earlier
  recommendation, is above the material layer, so it could not hold it.
  `shadow_sample.glsl` moves to `render/shaders/common/`. The shadow pass
  keeps `PackShadowTile` and names the record with a using-declaration. A
  block of `ShadowTile` records is declared `row_major`, as the receiver
  pass's is (the helper's header says so; the first lab run found it).
- **The view group** gains binding 4 (the view's `ShadowTileGpu` records), 5
  (the atlas, a depth texture) and 6 (its point sampler), through
  `SurfaceShadows` on `SurfaceProgram::ViewGroup`. `SurfaceLightGpu.cone.w`
  is the light's tile, or -1. The neutral view group binds a neutral
  texture that no tile indexes. `layouts.json` declares the bindings.
- **Owner-made textures in groups.** `ProgramTexture::external` binds a
  device texture that the group's owner made, such as a pass's output. The
  owner keeps it alive and in `kSampled` where the group is read.
  `GroupResidency` rebuilds the group when the id changes, and the world
  pass binds it with the request's sampler. Product wiring must also declare
  the graph read (K12).
- **vrad's spot rule** (light set v3, agreed with source-engine-5c):
  - `light_set::SpotFactor( cosine, innerCos, outerCos, exponent )` is the
    cosine to the axis times, between the cones, the linear ramp raised to
    the exponent. Exponents 0 and 1 are both linear, as in vrad
    (`utils/vrad/lightmap.cpp` emit_spotlight). Outside the outer cone it
    is 0.
  - `RuntimeLight::spotExponent` is new.
  - It replaces c1's smoothstep, which came from the frozen native
    `world_pbr.frag` (kept there until K12), so runtime spots equal the same
    spots baked and the fixtures' Cycles lamps.
  - `SurfaceLightGpu` grows to 80 bytes (`spot.x`, the exponent).
  - The fog's `MediumLight` carries the exponent (`misc.y`), and
    `lab_media` reads `_exponent` again.
- **Shared oracle.** `RuntimeLightOracle` (one light's falloff, cone and both
  lobes on the receiver) moves to `lab_receiver`. The clustered- and
  shadowed-light suites sum it.

| Check | Evidence | Result |
| --- | --- | --- |
| Shadowed clustered spots against a ray-test oracle | `render.lab.shadowed-lights`: two box casters drawn into two planned 512² tiles (render.pass.shadows `PlanShadowAtlas`, `ShadowDepthRenderer`), two spots with tiles and a point light without one, over the receiver plane, on two materials from an overhead and an oblique view. Oracle: each light's `RuntimeLightOracle` times a ray test to the boxes, sharing no code with the shader or the depth pass. Where visibility is the same over 2.5 atlas texels around the point (seen along the light), every sampled pixel is within 0.5 percent + 3e-4, widened by the oracle's change within 0.1 units; on an edge it lies between that light shadowed and lit. 4,096 and 2,607 judged pixels a case (262 and 135 on edges), worst relative 1.2 percent | pass (4 cases) |
| Coverage | each spot shadows judged pixels (332 and 533), the point light none | pass |
| Neutral bitwise | lights with no tile, with the atlas bound, are the frame without an atlas; the shadowed frame differs | pass |
| Seeded programs | `.sensitivity`: the visibility ignored, each light reading the next tile, the depth compare reversed | pass (4) |
| Spot rule | `render.lab.clustered-lights` judges the mixed set under vrad's rule (one spot at exponent 2); `.sensitivity` adds the cosine to the axis dropped | pass (16); pass (5) |
| Nothing else moves | family, opaque, world, composition and Hammer rows, and the shadow pass's own rows (`render.shadows.atlas`, `.sensitivity`, `.pixels`, `render.lights.clusters`), on g++ and clang++ (22 rows each); every lab suite with `--validate` | pass: 22 of 22 on both compilers after one expectation change. Hammer's `V8` upload count gains the neutral view group's neutral 2D texture (the programs' view residency makes its own for the unread atlas slot), a one-time 1×1 upload. Lab: area-lights 40, debug-views 82, lighting-controls 32, map-terms 21, volumetric 18, probe-volume 38, reflection-probes 30, lightmap-basis 19 |

Post-data change, recorded. The first run failed 12 pixels, all receiver
points inside the pillar's footprint that one spot lit. The cause was the
depth bias, not the tolerance. The tile's receiver bias (2e-4) is in clip
depth, whose world size grows with distance squared over the near plane. At
the spot view's near plane of 1 it came to about 32 units at the receiver,
so points that far behind a caster's face passed the compare. The lab's spot
views now use a near plane of 16, about 2 units of bias, with no change to
the band, the rule or the scene's geometry. A bias in world units (or
slope-scaled) is a render.shadows.v1 open item for K7. Every receiver needs
it, the fog's included.

Not in c2, next in step (c):
- the projector list and its cookies;
- sun cascades in the surface program;
- point-light shadows (RFC 0016 leaves them optional);
- debug view 21 (shadow visibility);
- the lightmapped and vertexlit points' runtime lights;
- the unbounded-light list;
- the shared froxel grid for the fog (still the interim `FroxelLayout`).

The light-set publisher filling `spotExponent` and a runtime-against-baked
spot parity check are source-engine-5c's, after this lands. Frame time: lab
only, no product draw changes; the Fold7 is locked.

## Output and `render_lab`'s presenting host on iPhone and Apple TV (2026-09-28)

User request (2026-09-28): "complete these on tvOS and iOS - in renderlab".
The three items were: HDR output encoding and tone map in the lab, the lab
presenting through the new swapchain, and the tvOS HDR mode switch. The
render-core owner (source-engine-43) agreed that this session owns
`render.pass.output`. Their conditions: one copy of each curve, debug views
bypass the tone map, and the SDR legacy point stays exact. The term is
defined in
[RFC 0016 "Output"](0016-render-core.md#output-renderoutputv1-amended-2026-09-28).
The obligations are in `unittests/rendertest/contracts/render.output.v1.md`.

- **`render.pass.output`**:
  - `public/render/pass/output/output.h`, `render/pass/output/`, a strict
    C++20 library that is Waf-registered.
  - Exposure, then the tone map, which is the BT.2390 EETF on max(R, G, B)
    in the PQ domain. It is the clip alone when the headroom covers the
    scene peak.
  - Then the encoding by target: 8-bit UNORM gets the sRGB curve, an sRGB
    view is linear with hardware encoding, and a half-float target is
    linear extended.
  - The curves: `render/shaders/common/tone_map.glsl` (new) and
    `color_encoding.glsl` (`OutputEncode`, added beside `LinearToSrgb`).
  - A debug view (`toneMap` false) gets the encoding alone.
- **Port change**: `IHostDevice::ImportImage` accepts `kExternal` as a host
  image's home usage, meaning another API reads the image in GENERAL after
  the port's writes. A presentation bridge's back buffer is imported that
  way. Its description is validated like `CreateExported`'s.
- **`render_lab`'s presenting host**:
  - Files: `render/lab/app/lab_app.{h,cpp}`, module `render.lab.app`. The
    headless `render_lab` still links no SDL.
  - An SDL3 window, the R16 SDL3-Vulkan bridge and the `render.backend.v1`
    provider on the adapter's host device. One Vulkan device serves both,
    through `Host().Port()`.
  - Each frame it imports the back buffer into the core and runs
    `render.pass.output` through the frame graph at the presentation's
    current headroom.
  - It asks for the extended-linear range first and falls back to standard
    only when the surface refuses (`kSurfaceIncompatible`). `Range()` says
    which it got.
  - The scene is an HDR chart: a gray ramp from 1/64 to 16 times white,
    gray and warm patches from 0.18 to 16, and saturated patches at 4.
- **Device app**:
  - `tools/quality/ios_conformance.py --app render_lab` builds
    `RenderLab.app` from the device profiles' new `apps` entries, under the
    Portal 2 bundle id `com.panos.sourceengine.portal2`. The user asked for
    no new bundle id. Installing it replaces the Portal 2 app; its data
    container stays.
  - `--env` passes environment variables and `--extra-timeout` extends
    runs, for `RENDER_LAB_SHOW_SECONDS`, which keeps the chart on screen.

| Check | Evidence | Result |
| --- | --- | --- |
| `render.output` (Linux GPU) | 27 checks at `985a4a35` plus the tree; g++ and clang++ release. The SDR legacy point matches clip plus sRGB byte for byte at three exposures. Extended: clip-alone, EETF within 0.3 percent, peak lands on the headroom, monotonic, below-knee unchanged, hue kept. Debug view untouched. 7 refusals. 6 of 6 seeded programs are detected, each by the checks its defect breaks. Khronos validation reports 0 messages | pass |
| `render.output` on devices | the same suite through `--app render_lab`: iPhone 16 Pro 26 checks, Apple TV 4K 26 checks (no Khronos layer there, so validation is not judged) | pass |
| `render.lab.hdr`, iPhone 16 Pro (iOS 27) | extended range granted; layer high in extended linear sRGB; headroom 1.20 rising to 7.23 of 8.00 in 0.35 s; every patch of the presented swapchain image matches the oracle at headroom 7.23 (worst 0.001); brightest 7.23; debug view presents 16.0; the standard control shows the legacy point within one level | 8 of 8 |
| `render.lab.hdr`, Apple TV 4K (tvOS 26.6) | as on the iPhone, plus: HDR10 requested, Match Dynamic Range on, the TV switches in 2.9 s, and the declared HDR10 headroom 4.93 applies; the chart matches the oracle at 4.93; the standard control withdraws the request and the TV switches back. The user watched the chart and confirmed the brighter-than-white patches show brighter on the TV | 12 of 12 |
| Linux, headless mutter | the standard control passes (legacy point within one level) through the real bridge. The extended range is refused (mutter offers no HDR), and the first window stalls on FIFO acquires: the known headless-mutter stall | standard only |

Reproduce:
- `python3 tools/quality/conformance.py check --suite render.output`
- `python3 tools/quality/ios_conformance.py check --app render_lab`
- the same with `--device-profile tvos-arm64-device`
- to watch the chart, add `--env RENDER_LAB_SHOW_SECONDS=120 --extra-timeout 130`

Open:
- The lab host shows the chart, not yet a map scene; the canvas now exposes
  its color target, so that comes next.
- No Waf target for the host yet: it builds through the manifest's source
  list.
- The product (K12 "Game output") is not wired.
- PQ/HLG encodings are out of scope.

## K8 monitors: the legacy baseline and the view-generator proposal (2026-09-29)

Portal 2's Wheatley monitors draw since `f8eb21c2`: the client now defines
`USE_MONITORS` for `PORTAL2`, and `corpus.portal2.monitors` is their product
oracle. They draw through the legacy path, `CViewRender::DrawMonitors`:

- Every active `point_camera` is drawn, in full `ViewDrawScene` (world,
  renderables, particles), serially before the main view.
- A camera is active while a linked monitor is in the player's PVS, whether
  or not its screen is on screen.
- Every camera draws into the one 256x256 `_rt_Camera` (the size Portal 2's
  client passes), which has no mips.

This section measures that path before the K8 work (binding rule 7: look
first, then optimize). It measures; it closes no check.

### Workload and method

- `quality/workloads/portal2-monitors-frame-pacing-v1.json` loads
  `sp_a4_intro` with its five monitors deployed. It measures two phases, 300
  frames each:
  - `facing`: 190 units in front of `wheatley_monitor1`;
  - `away`: turned 180 degrees, so the screen is off screen but still in the
    PVS.
- `tools/quality/frame_pacing.py` takes a scenario `game` key (`portal`, the
  default, or `portal2`), and stages through `portal_boot` for that game.
- The run is an interleaved A/B on one private snapshot of `build-p2`
  (HEAD `7a6af4b3`, plus the shared tree's uncommitted work at 01:56):
  - A: monitors on;
  - B: `+cl_drawmonitors 0`.
- Both sides run with `mat_queue_mode 2` and `./play_p2`'s job arguments,
  `+mat_colorcorrection 1` excepted (the 512-character command limit), on
  Box3D. There are 5 rounds of 3 passes, with the warm pass judged.
- Desktop: Radeon 8060S at 1280x720, under a host load average of 17-23.

### Results (warm pass; medians of 5 rounds; the paired difference is on minus off per round)

| Phase | Metric | Monitors on | Off | Paired difference |
| --- | --- | --- | --- | --- |
| facing | frame interval | 2.94 ms | 2.62 ms | +0.64 ms (4 of 5 rounds +0.63 to +0.74) |
| facing | engine CPU | 2.41 ms | 2.12 ms | +0.29 ms |
| facing | `render_submission` | 2.13 ms | 1.77 ms | +0.36 ms |
| facing | GPU render | 1.03 ms | 0.92 ms | +0.11 ms |
| facing | converted draws per frame | 89.7 | 82.7 | +7 |
| away | frame interval | 11.26 ms | 10.08 ms | +0.77 ms (range -2.84 to +2.20: noise) |
| away | converted draws per frame | 284 | 277 | +7 |

Reading:

- The camera view costs about 7 draws and 0.3-0.6 ms of CPU a frame on
  desktop, most of it in engine CPU and submission. It costs about 0.1 ms
  of GPU, because the camera sees a small hidden room.
- It costs the same with the screen off screen: the legacy path does not
  gate on the screen's visibility.

The Fold7 is unavailable: it is connected but locked (keyguard). It is
unmeasured, as is the iPhone.

### Proposal: monitors as `render.scene` view generators (K8 cohort)

1. **Object.** A `Monitor` view generator in the scene holds:
   - its screen surfaces (mesh instances whose material samples the
     generator's output);
   - the camera: pose, FOV, fog, and aspect policy (`UseScreenAspectRatio`);
   - the active flag, which the game publishes through the change set.

   The game stops drawing into a global target.
2. **Visibility gating.** The generator's view and passes enter the frame
   graph only when one of its screens passes the main view's culling in the
   same frame. The monitor view is culled on the pool like any view (K5
   "Pooled recording").
3. **Resolution and filtering.** The target size follows the screen's
   projected size, about one texel per pixel. It is bucketed in powers of
   two from the graph's transient pool and clamped to 64-1024. The target
   gets a mip chain, so the screen samples it with anisotropy; MSAA follows
   RFC 0012. Each generator gets its own target, not one shared
   `_rt_Camera`.
4. **Lighting.** The view runs the same surface program and
   `render.lighting.v1` terms as the main view, into an HDR target. The
   screen material (`dev/dev_tvmonitor1a`: UnlitTwoTexture with a
   scrolling scanline layer and noise proxies) is a point of the one
   surface model, with the view's output as its base layer. No family names
   "monitor".
5. **Reuse.** The last image is kept while the camera's pose and FOV, and
   the scene revisions of the objects in its view, are unchanged. Wheatley
   animates, so his screens redraw; a static security camera's do not.
6. **Lab first (`render_lab`).** A fixture with a camera on a skinned model
   and a screen quad seen at several distances. Oracles:
   - the screen's pixels match a direct render from that camera at the
     chosen size;
   - the chosen size follows the projected size;
   - an off-screen or occluded screen records no monitor pass (a pass
     census);
   - reuse skips redraws exactly when the inputs are unchanged.

   Negative controls: a stale target, a generator drawn while hidden, a
   wrong camera, and a fixed 256 size.
7. **Product.** When R89 draws the world and models from the scene:
   - `corpus.portal2.monitors` is the unchanged gate;
   - this workload's A/B is recorded, and the `away` phase is expected to
     cost nothing;
   - first-party `DrawMonitors` and `_rt_Camera` are deleted in the same
     change.

   A mod client that draws its own monitors keeps them through the legacy
   frontend (K9 "Mods still render").

Owner of the lab slice and its terms: source-engine-43, by the K11 split.
The product integration waits on R89.

Reproduce (the host must not rebuild `build-p2` during the run; snapshot it):

    rsync -a --prune-empty-dirs --include='*/' --include='*.so' \
      --include='hl2_launcher' --include='c4che/*_cache.py' --exclude='*' build-p2/ <snap>/
    python3 tools/quality/frame_pacing.py --runtime <p2 content runtime> --build <snap> \
      --ab-build <snap> --ab-extra-arg=+cl_drawmonitors --ab-extra-arg=0 --rounds 5 \
      --scenario quality/workloads/portal2-monitors-frame-pacing-v1.json --out <short dir> \
      --physics vphysics_box3d --mat-queue-mode 2 --extra-arg=+cl_render_start_graph \
      --extra-arg=2 --extra-arg=+sv_querycache_job_graph --extra-arg=2 \
      --extra-arg=-vkemitparallel --extra-arg=1

## K11 slice: the lighting fixture set and its Cycles references (2026-09-29)

These are the fixtures that K11's "Ground truth" check compares `render_lab`
against: data plus tools, and no `render/` code. A subagent of
source-engine-5c built them under the split agreed with source-engine-43.
source-engine-43 owns `render_lab` and every lighting term.

- **Fixtures** (`quality/fixtures/lighting/`, schema `lighting-fixtures/v1`):
  1. cornell-floors: rough and polished halves;
  2. area-room: 64 rectangle lights;
  3. projector-cookie: `env_projectedtexture` with a window cookie;
  4. sun-colonnade;
  5. foggy-hall: a medium with 256 lights and a projector;
  6. mirror-corridor: floor roughness from 0.02 to 0.5;
  7. material-sweep: gold, clear coat and dielectric at 8 roughness levels;
  8. portal-chamber: `testchmb_a_00_relit` at the K0 view-oracle poses;
  9. portal2-chamber: `sp_gi_chamber_01`.

  Each fixture has fixed cameras, the lights once as declared and once as
  entity-lump entities, and a BSP2 map built by the one lighting back end.
  All nine maps are built and published (`run/maps/lt_*`).
- **References:** linear half-float EXR (Combined) plus a 16-bit object
  index per view, rendered by `gi_reference_blender.py`, extended in
  `lighting_reference_blender.py` with a projector lamp that follows
  `projected_light.h`. They are **preview** quality (16 spp, reduced bakes),
  and certify no K11 check. The final references (2,048 spp, full bakes)
  are documented in the README and not run, per the user's rule against long
  Cycles builds during pipeline work.
- **Error metric and tolerances:**
  - Per pixel, the largest channel difference divided by the reference's
    mean luminance, skipping background and visible emitters. Each fixture
    has a mean and a p99 bound.
  - `tolerances.json` was fixed at 2026-09-29T08:35Z, before any
    comparison. It is digest-protected, and `check` rejects an edit in
    place, a tolerance fixed after a recorded comparison, and a comparison
    against a changed reference.
- **Tools:** `tools/quality/lighting_fixtures.py` (generate, render, build,
  check, compare). `test_lighting_fixtures.py`: 37 tests, including negative
  cases for missing or changed references, late or edited tolerances, stale
  comparisons, unknown terms, missing cameras, wrong sample labels and
  entity-lump mismatches.
- **Fixed along the way:** `usd_worldmesh_pack.py` read an emitter name as
  `Light(Quad|Disk)` plus exactly two digits, so the 101st light was packed
  as world geometry. It now takes two or more digits.
- **Entities** (decided by source-engine-43 on 2026-09-29 under rule 5,
  with Source 2's names):
  - `light_rect`: `angles` (forward is the emission direction and the
    rectangle's normal), `width` along -right and `height` along up (halfU =
    -right × width/2, since Source's right × up = -forward), `color` (linear 0..255), `brightness`
    (radiance = color / 255 × brightness) and `two_sided`;
  - `env_volumetric_fog_volume`: `box_mins` and `box_maxs` relative to the
    origin, `density` (extinction per unit), `albedo`, `anisotropy` and
    `emission`;
  - one `env_volumetric_fog_controller`.

  Rect lights carry their name as `_fixture_light`, because vbsp turns
  every named `light*` entity into a switchable style (at most 32). All
  seven maps were rebuilt, and `check` passes.
- **Open for source-engine-43:**
  - the two chamber stages are rebuilt from untracked `quality-results/`,
    with the steps recorded in each fixture;
  - `gi_reference.py` writes its environment maps with imageio's lossy
    half-float EXR compression, which zeroed 5% of the texels in a test. The
    RFC 0011 sky fixtures' references may be affected. This is not verified
    on those fixtures.

## K10: OpenGL adapter, slices 1–4 (2026-09-29)

State: `render.device.gl` exists and passes the port suite on the Linux
desktop (radeonsi) and on llvmpipe. Rebased onto K11 a2 (`04756591`): the
counts below are from that base (g++ and clang++; Vulkan device suite 851). K10 stays open: capability negotiation,
pixels, the product boot and the ToGL build each wait on a decision (below).
Owner: a K10 subagent session, on a worktree branch the main session lands.

- **The adapter** (`render/device/gl`, `public/render/device/gl/provider.h`,
  strict C++20, `arch_module = render.device.gl`, links EGL alone):
  - an OpenGL 4.5 core context of its own on an EGL surfaceless display, so
    no window; GL entry points through `eglGetProcAddress`;
  - encoders record CPU command lists on any thread; `Submit` validates them
    as the Vulkan adapter does and replays them in order on the calling
    thread; each device call makes the context current and restores the
    caller's own afterwards (the render sequence moves between the main and
    `MatQueue` threads);
  - tokens are fence sync objects; the upload ring is a persistently mapped
    coherent buffer; programs are cached by stage sources and constants;
  - conventions by `glClipControl( GL_UPPER_LEFT, GL_ZERO_TO_ONE )`: facing
    stays judged in clip space, so the Vulkan multisample clause's culling
    holds unchanged;
  - claims compute, storage buffers and BC (with S3TC and sRGB S3TC); not
    external images, aliasing, parallel recording, async queues or ray query;
  - narrower than Vulkan, by status: `kD24UnormS8` texture copies, sRGB or
    depth storage textures, a binding past slot 15 of its group.
- **Artifacts** (`tools/render/shader_artifacts.py` `cross_compile` owns the
  form; the contract section "OpenGL adapter" in
  `unittests/rendertest/contracts/render.device.v2.md` lists it): flat slots
  `group * 16 + binding`; sampled textures and samplers named after their
  slots, so each combined sampler SPIRV-Cross builds sits on its texture's
  slot and names its sampler; the draw constants the block
  `RenderDrawConstants` at uniform slot 64; a line after `#version` listing
  each specialization constant's type (GLSL has no constant bit cast, so the
  adapter writes typed literals). `GLSL_GENERATED` headers: the suite's
  fixtures and a `<stem>_glsl.h` twin of every core program header.
- **Suite changes:** the Vulkan-local fixtures joined `test_shaders.h`;
  `DeviceDriver::artifact` maps each fixture to the adapter's format; the
  compute, sampling and multisample clauses moved from
  `test_device_vulkan.cpp` into the shared `RunRasterConformance` (same check
  names).

| K10 check | Evidence | Result |
| --- | --- | --- |
| Port suite | `render.device.v2.gl` (profile `linux-native-gl-gpu`): shared suite, raster clauses, small ring, a debug-output pass (0 messages), facts, ring, viewport origin, cross-thread submission, context restoration, masked capability. D7 runs (a simulated context reset, then `Recover`). 822 checks on radeonsi (g++, clang++), 819 on llvmpipe. Unclaimed capabilities are listed (INFO), not failed | pass |
| Bad adapters (K1 clauses on GL) | `render.device.v2.gl.sensitivity`: lower-left origin (D13 y), −1..1 depth (D13 z), false async compute and false aliasing (D15), ignored write masks (D17), dropped specialization (D20), early upload reuse (D10): 7 of 7, each only its clause. D10 and D5 run with the queue held (`gl::HoldSubmissions`, test-only, `44ed025d`), so an early reuse is observable on every driver: 20 of 20 runs catch it on radeonsi and llvmpipe, g++ and clang++ (it had escaped 2 of 6 on radeonsi and was skipped on llvmpipe) | pass (radeonsi, llvmpipe) |
| Artifacts per target | `render.shader-artifacts.gl` (19 checks, g++ and clang++): the core artifact store holds every core program stage in SPIR-V and GLSL 4.50 with one reflection and `Resolve` takes the format asked for; the 11 core programs of K11 a2's set, resolved for GL, link; a truncated artifact fails; the output pass and the skinning and cluster kernels create their programs on GL, and a seeded SPIR-V kernel is refused there. `shader.toolchain-pin` (85) and `.sensitivity` (19), `render.shader-artifacts` (1,347) pass | pass |
| No portable changes needed | archlint `check --all`: no finding in these files (4 findings elsewhere predate this work); CAP011 rule 5 finds no portable comparison of `diagnosticBackend` | pass |
| Capability negotiation | Slice 5: `render.composition.capabilities` (null, headless) and `.gl` (OpenGL, compute and storage buffers masked through the adapter), 14 checks each, g++ and clang++: CPU skinning composed under the declared `skinning=skinning-cpu` and named in `RenderCoreResult`; `RENDER_CORE_UNDECLARED_FALLBACK` without the declaration; an independent oracle passes the good negotiations and catches both seeded bad compositions (an undeclared fallback taken, a silent substitution); a feature with no fallback fails `kMissingCapability` naming it | pass |
| Product boot | blocked on K8/K9 (port owner's decision, 2026-09-29): no legacy path draws through a GL device (the frozen-path rules forbid building one) and there is no `render.bridge.sdl3-gl`; `portal-linux-gl` boots once the cohorts are on the core | blocked |
| Pixels | the pixel families run through the product (`material_pixel_conformance.py`), so blocked with the boot | blocked |
| ToGL untouched | no ToGL or legacy-renderer build input changed; not rebuilt | unverified |

Decisions (port owner, source-engine-43, 2026-09-29, relayed by the main
session), taken on the slice 1-4 report:
1. **Composition fallback (slice 5, done):** `FeatureRequirements::fallback`
   beside `required` (`public/render/frame/feature.h`). Composition
   (`render/composition/negotiation.{h,cpp}`) substitutes a fallback only
   when the product profile declares it (`RenderCoreConfig::fallbacks`, Waf
   `--render-core-fallbacks`, `-render-fallbacks`) and names every
   substitution in `RenderCoreResult::substitutions`; an undeclared one fails
   `RENDER_CORE_UNDECLARED_FALLBACK`. A profile masks capabilities through the
   adapter's options (`RenderCoreConfig::maskedCapabilities`, Waf
   `--render-core-masked-capabilities`); an adapter without a mask option
   (Vulkan today) fails composition. The first user is
   `render.pass.skinning`'s `skinning`/`skinning-cpu` pair
   (`public/render/pass/skinning/feature.h`), which adds no passes until K6's
   product skinning feeds skinned meshes. The launcher reads both settings
   and logs substitutions; no client product was rebuilt for it.
2. **Shader selection (slice 6, done):** one
   artifact store, `CoreArtifacts()` (`public/render/shaderlib/core_artifacts.h`),
   built once from the table the build generates (`spv/core_artifact_table.h`,
   `shader_artifacts.py store_header`: every core program row in both
   formats with its reflected bindings and draw-constant bytes, keyed by
   source, `CoreCompiler()`, format and permutation 0). The lines, debug,
   output, shadow depth and receiver passes and the skinning and cluster
   kernels resolve their programs with `Resolve( CoreRecipe( ... ),
   CoreArtifacts(), device.Facts().artifactFormat )`; none names a format.
   A suite's seeded SPIR-V variant goes through an `ArtifactOverlay`
   (`ReplaceSpirv`), which refuses a device of another format instead of
   running the unseeded program. The surface program
   (`render/material/surface_program.cpp`) moved after K11 b1 (`a7ad7c9c`):
   its three vertex stages and `surface.frag` resolve through the store with
   their reflected bindings (the hand-written reflection is gone), and the
   seeded fragments (`debug_view_defects_spv.h`, `area_light_defects_spv.h`)
   still reach it through `fragmentModule`, as an `ArtifactOverlay`. No core
   consumer names an artifact format now. `render_lab` composes only the
   Vulkan device, so the world pass has not been run on GL; its programs
   link on GL (`render.shader-artifacts.gl`).
3. **Product boot:** blocked on K8/K9 (see the table).

## K10: core pixel families on GL, ToGL and product composition (2026-10-05)

Session source-engine-d7, on HEAD `b06363170` plus this change, Linux
desktop (radeonsi), g++ 16.2.1 and clang++ 22.1.8. The K10 row stays
`partial`: the product boot and product pixel families remain blocked, as
before, and their state is unchanged.

- **The port suite again** (HEAD in a clean worktree, so other sessions' WIP
  is excluded): `render.device.v2.gl` 1,047 checks, `.sensitivity` 19,
  `render.shader-artifacts.gl` 19 and `render.composition.capabilities.gl`
  14 pass on radeonsi and on llvmpipe. `render.device.v2.vulkan.sensitivity`
  (21) and `render.composition.capabilities` (14) pass. The two D21
  sensitivity cases expected the clause's old text (`src + dst * a`) and
  failed the GL row. They now name `the independent color equation`.
- **Core pixel families on GL** (`unittests/rendertest/core/material/family_devices.{h,cpp}`):
  the five K4 family suites choose their device at build time.
  - Without a define they run on Vulkan, as before.
  - `RENDERTEST_FAMILY_GL` runs on `render.device.gl`.
  - `RENDERTEST_FAMILY_CROSS` draws each case on Vulkan, then on GL, in one
    process. Every GL frame must be within the per-case limit of the Vulkan
    frame over all 65,536 pixels, not only the port samples.
  - The limits are a recorded fixture,
    [`cross-backend-v1.vdf`](../quality/fixtures/render-families/cross-backend-v1.vdf):
    2 levels per channel and a reviewed outlier count. Of the 47 cases, 34
    are byte-identical and 10 differ by one level. Three PBR cases differ
    only along one hard shading edge (16–24 pixels; peak 3, 15 and 28
    levels at 117,182), where the two drivers' compilers round a step
    differently. They are allowed 64 or 96 channels.
  - New rows `render.family.<f>.gl` and `render.family.<f>.cross-backend`
    for unlit, water, lightmapped, vertexlit and pbr, plus the sensitivity
    row `render.family.unlit.cross-backend.seeded-gl-lower-left`. That row
    flips the GL adapter's origin, and the cross clause catches it.
  - Result: all 21 family rows pass on g++ and clang++. Each GL row is also
    judged against the legacy port fixture and must report no GL debug
    message.
  - The lightmapped fog check judged every pixel as drawn. On llvmpipe, a
    quad edge on a pixel center leaves pixel 0,0 uncovered, which GL leaves
    to the rasterizer. The check now skips pixels neither draw covered.
  - On llvmpipe, `family_lightmapped_bump_nodiffusebumplighting` and
    `bump_envmap_normalmapalpha` miss the port fixture (83 levels). Vulkan
    on lavapipe misses the same two, so this is the shared Mesa software
    rasterizer, not the adapter. The required runs are GPU runs; the
    cross-backend rows need both adapters on the same GPU.
  - This is the core's own pixel families on GL. The K10 "Pixels" check
    names `material_pixel_conformance.py` on `portal-linux-gl`, which needs
    the product boot.
- **Product composition** (not the K10 boot): a native Vulkan Portal
  client built with `--render-core-gl --render-core-device=gl` logs
  `Render core: device gl`. Its frames run as four stage passes of the
  core's frame graph on the GL adapter, while the legacy stream draws
  through the native Vulkan backend. `portal_boot.py` on `testchmb_a_01`
  passes in `mat_queue_mode` 0 and 2. The GL device does not draw the
  frame, so this is not the "Product boot" check.
- **ToGL** (`--render-backend=legacy --use-togl=1 --build-games=portal`,
  64-bit SDL2): builds at HEAD and installs `libtogl.so`. In a private
  headless mutter (Xwayland), `portal_boot.py` boots `testchmb_a_01` and
  passes.
  - The saved screenshot is noise (stride-like rows over the top third,
    black below). A build at `9e50b9054`, the parent of the first K10
    commit, gives the same, so the defect predates K10.
  - No ToGL, `togl/` or D3D9 input changed in the K10 commits. No ToGL
    check is recorded in `quality/baseline.json`.
  - Over SDL's Wayland driver, the ToGL client crashes in `libtogl.so`
    (`strstr` on a GL string). X11 works.
  - "ToGL untouched" holds for the build and the boot harness. The
    screenshot defect is a pre-existing legacy-profile finding, outside K10
    (frozen path).

| K10 check | Result (2026-10-05) |
| --- | --- |
| Port suite | pass (radeonsi and llvmpipe, g++ and clang++) |
| No portable changes needed | pass (unchanged since slices 1–4) |
| Capability negotiation | pass (at HEAD; in the shared tree, `.gl` does not link against other sessions' uncommitted `CoreWorld` sources) |
| Pixels | the core families pass on GL and within the recorded cross-backend limits; the product families on `portal-linux-gl` remain blocked with the boot |
| Product boot | blocked on K8/K9 (R91), per the port owner's 2026-09-29 decision and the frozen-path rules; the client composes and runs the core on GL in both queued modes |
| ToGL untouched | pass (build and boot); the screenshot noise predates K10 |

## K11: the lab gallery, and where render_lab stands against Cycles (2026-09-29)

User request: "make that easy to replicate, and add it to the rfcs … being
able to produce these comparisons visually for me easily".

`python3 tools/quality/lighting_fixtures.py gallery` renders every view of
every lighting fixture in `render_lab`. It finds render_lab through `--lab`,
`$RENDER_LAB` or a `build-rc-lab` tree, and places the probe model from the
stage. It scores each view with `lighting_fixtures.compare`, so the metric
and tolerance are the ones the K11 "Ground truth" check uses.

It writes one self-contained HTML page to
`quality-results/lighting-gallery/<time>/index.html`. Per view, the page shows:
- render_lab and Cycles at the same exposure;
- the error map (red at twice the p99 tolerance, grey where the metric
  skips);
- mean and p99 against the tolerance.

The command records nothing. `lighting_gallery.py` holds the code, and
`test_lighting_gallery.py` (3 tests) covers the display math and the page.
K11's "Gallery for review" row names the command.

First run (render-core worktree's `build-rc-lab`, preview references,
diagnostic only; 0 of 23 views pass):

| Fixture | mean / tolerance | What the lab lacks against Cycles |
| --- | --- | --- |
| sun-colonnade (yard, along) | 0.094, 0.202 / 0.06 | nearly there; the yard's p99 passes. Mostly reference noise |
| cornell-floors | 0.229, 0.258 / 0.06 | the floor's specular (rough and polished halves) |
| area-room | 0.21–0.27 / 0.08 | specular reflections of the 64 rect lights; the emissive sign draws black |
| foggy-hall | clear 0.40–0.43, fog 0.68–0.78 / 0.10 | specular; no participating media |
| mirror-corridor | 0.63–0.66 / 0.10 | no reflections: the mirror floor draws as its albedo |
| material-sweep | 0.53–0.73 / 0.05 | no specular lobe or image-based light; gold draws flat yellow |
| portal-chamber | 0.70–0.98 / 0.12 | specular, and a black ceiling patch (a material the lab draws black) |
| projector-cookie | 1.03–1.23 / 0.06 | no projected light at all |
| portal2-chamber | 4.5–9.4 / 0.12 | garbage texturing: the lab mis-decodes this map's materials. A lab defect |

So the lab's baked diffuse (lightmap × albedo) is close to Cycles. What's
missing is everything that makes the Source 2 look: specular from lights
and probes, reflections, emission, projected light and media. These are K11
steps b–g, in order, so the table is the expected state.

## K11 fixture: light through a portal pair (`portal-pair`, 2026-09-29, user request)

User request: a `gi_portal_light`-like fixture for `render_lab`, so light
through portals reaches Source 2 quality. The fixture is source-engine-07's.
The lab side (reading the pair, portal views, images and their shadows) is
the render-core owner's (source-engine-43).

**Decisions (render-core owner, 2026-09-29).**
- The term is `portal-transport`, owned by `render.portal-lights.v1` (RFC
  0011). It is how every term is evaluated across an open, linked pair, not
  a row of its own. RFC 0016's model section gains one line when the lab
  side lands.
- The opening is the portal's visible ellipse, inscribed in 64 x 108 units.
  `render.portal-lights.v1` P2's rectangle clip is a known deviation, and
  the fixture exposes it.

**The fixture** (`quality/fixtures/lighting/portal-pair`, generator
`portal_pair` in `lighting_fixtures.py`):
- Room A (5 x 4 m): a 1 m ceiling rectangle, and a spot aimed through
  portal A past a post.
- Room B (4 x 5 m, 6 m away): only a dim 0.5 m lamp of its own, a rough and
  polished floor, a block and the dynamic probe sphere.
- The pair sits on perpendicular walls (A's east, B's south), so the
  transform is a rotation, not a translation.
- `closed` is the portal world, and the map `lt_portal_pair` is built and
  baked from it. Its entity lump carries the pair as `prop_portal`s, which
  `check` now compares like the lights.
- `open` is the reference. Each room is joined through the opening to a copy
  of the other behind its portal wall, placed by Source's
  `MatrixThisToLinked` (forward and right negated). For one pair this is
  exact for every path, any number of crossings included.
- Cameras: `b-portal`, `b-floor` (the polished floor's reflection of the
  opening) and `a-portal` (the view into B through A). Each has a `through`
  region that the lab passes only by drawing the view through the portal.
- References: denoised Cycles at 256 samples. Open room B is 14.6x (`b-portal`)
  and 17.5x (`b-floor`) brighter than closed, so transport is most of its
  light.
- Tolerance fixed before any comparison: mean 0.07, p99 0.8.

**Baseline** (`lighting_fixtures.py gallery --fixture portal-pair`, the lab
at `build-rc-lab`, diagnostic): the lab draws one frame per camera from the
closed bake, so `open` fails everywhere, as the term's negative control must
(mean 0.55–1.42).
- `closed` `a-portal` passes (mean 0.035).
- `closed` room B fails only because the lab draws the probe sphere unlit
  white (dynamic-model lighting is a later K11 step). The world's median
  radiance equals the reference's (0.0048 against 0.0048).

**Fixed on the way.**
- `lighting_fixtures.py render` exited 1 for every denoised render; it now
  accepts every reference status.
- The opening's wall generator skips empty cells when the opening meets the
  wall's edge, and rejects an opening larger than its wall.

**Tests.** `test_lighting_fixtures.py` has 5 new cases (the pair's frames and
transform, the opening's area and winding, the entity, the checked-in
fixture); 46 tests pass. `lighting_fixtures.py check` passes on the whole
set, and `generate --check` matches.

**Next (render-core owner).** The lab reads the pair from the lump and
draws the view through each portal (the K8 view generator, in the lab). It
images lights through the pair with their shadows (atlas depth from the
image). Indirect and specular light cross the pair. A glow state
(source-engine-71, next) adds each portal's own light.

Reproduce:

    python3 tools/quality/lighting_fixtures.py build --fixture portal-pair
    PYTHONPATH=<usd_pythonpath> /usr/bin/python3.12 tools/quality/lighting_fixtures.py \
        render --fixture portal-pair --samples 256 --denoise
    python3 tools/quality/lighting_fixtures.py gallery --fixture portal-pair

### Gallery after defects A and B (2026-09-29)

Both lab defects the first gallery found are fixed, and neither was in a
lighting term.
- **A** (`adae3958`): the Portal 2 chamber's LMAP is a directional 2:1
  page, and the lab now stages its flat half. portal2-chamber drops from
  9.42 to 0.213 (chamber) and from 4.51 to 0.396 (spawn).
- **B** (`6bed4b82`): testchmb_a_00_relit had been baked from a scene
  extracted before `widest_triangulation`, with 1,402 zero-area triangles.
  Blender's split normals turned sideways next to them, and part of a wall
  baked black.
  - The map is rebaked with current tools (preview).
  - The bake now refuses a planar face shaded from behind.
  - portal-chamber's denoised references were re-rendered against the new
    scene.
  - portal-chamber drops from 0.394 to 0.257 (room2) and from 0.595 to 0.327
    (vault).

Gallery against denoised references, `build-rc-lab` at the shared HEAD: 3
of 35 views pass:
- sun-colonnade yard 0.041;
- foggy-hall clear side 0.076;
- portal-pair closed a-portal 0.041.

The remaining gaps are the unbuilt terms:
- specular and image-based light (material-sweep, mirror-corridor);
- projected light (projector-cookie, about 1.0);
- media (foggy-hall fog, 0.42–0.50);
- portals (portal-pair, which K8 draws; room B is dark in the lab).

### K11 step g decisions, from the render-core owner (2026-09-29)

- **Fog froxel grid:** derived from the light grid by
  `SubdivideClusterGrid(grid, tileDivisor, sliceMultiplier)` in
  render.pass.lights, the one owner of the depth split. Each fine froxel
  lies in exactly one light froxel, which a clusters-suite check with a
  seeded mismatch proves. The lab uses 8 px tiles and 96 slices (4× the
  light grid's 24).
- **Layer contract:** CAP011 keeps render.pass.* independent siblings, so
  render.pass.volumetric reads neither lights nor shadows. The interim is a
  plain `FroxelLayout` input that render_lab fills from the grid. Where the
  shared view-level types live (the froxel layout, ShadowTileGpu and
  shadow_sample.glsl) is open with the owner; the recommendation is
  render.frame and render/shaders/common. Fog in-scattering is unshadowed
  until then, a recorded gap.
- **Unbounded lights:** radius-0 lights (inverse-square bulbs) reach every
  froxel and overflow its light list. Step c adds a separate "global lights"
  list, read by the surface program and the fog. Until then, the fog loops
  over the whole light list per froxel (a rule-7 performance item).
- **Legacy fog:** at density zero the composite is bitwise the frame without
  it, legacy range and height fog included (a suite check). The rule that
  turns legacy fog off where volumetric fog is on is the owner's frame-group
  diff, a recorded gap.

### K11 step g, slice g1: volumetric fog, unshadowed (2026-09-29)

This slice builds the participating-media term of `render.lighting.v1` as
`render.pass.volumetric` and proves it in `render_lab`. Nothing is wired
into the product (binding rule 3).

- **The pass** (`public/render/pass/volumetric/volumetric.h`,
  `render/pass/volumetric/`) has two stages.
  - **Inject** (`volumetric_inject.comp`): one invocation per froxel.
    - It averages the medium over 2 x 2 x 4 stratified points: extinction,
      and in-scattered radiance per unit length.
    - At each point, for every light, it takes
      pi E(x) exp(-tau(light, x)) sum_i sigma_s,i HG(g_i, cos). tau is exact
      for boxes, the global density and the height fog.
    - Media add up, each with its own Henyey-Greenstein phase, and emission
      is added.
  - **Composite** (`volumetric_composite.frag`): full screen.
    - It walks front to back along each pixel's own ray through the slices
      up to the scene depth, with Hillaire's energy-conserving step.
    - Each slice's froxel values are filtered bilinearly across columns.
    - It writes (L, T) under the port's transmittance blend (D21), so the
      frame becomes dst T + L on rgb, alpha kept, T at the target's precision.
    - Marching per pixel keeps transmittance exact for a medium uniform
      across a column, the screen's edges included. A column integration
      evaluated at the column's centre misjudged the edge rays' length
      (16 percent error at the edge in the first build). Hillaire's
      integrated volume is left for the translucent application and as an
      optimization.
- **Lights and units.**
  - Lights use `light_set::InverseSquareFalloff`'s rule.
  - Spots used vrad's spot rule in this slice (superseded by the c1 follow-up
    below, which takes light_set::SpotFactor through `runtime_light.glsl`).
  - Projectors use `projected_light::Project` and `Attenuation`, without a
    Lambert term.
- **Neutral value.** Density zero gives T = 1 and L = 0 exactly, and the
  frame is bitwise unchanged. The pass keeps no state between frames: its
  froxel volume is created per frame and released behind the frame's token.
- **Froxels.** The froxels are render.pass.lights' light grid, subdivided
  8 x 4 by `SubdivideClusterGrid` (landed as `8dd98ddc`). render_lab copies
  it into the interim `FroxelLayout` (CAP011 rule 2), so the lab's
  512 x 384 view has 64 x 48 x 96 froxels.
- **render_lab.**
  - It reads the entity lump: `env_volumetric_fog_volume`,
    `env_volumetric_fog_controller`, `light` and `light_spot` as vrad
    compiles them, and `env_projectedtexture` with its cookie in a 2D array.
  - It runs the pass after the opaque draws.
  - New options: `--fog-scale s` (the fixture's density-zero state),
    `--no-volumetric`, `--fog-samples xy,depth`, `--time n` and
    `--inscatter-only`. The last clears the frame before the composite, which
    gives Cycles' Volume Direct pass.
  - The gallery renders a medium-free state of a fixture with media at
    `--fog-scale 0` (`lighting_gallery.py fog_scale`).

| Check | Evidence | Result |
| --- | --- | --- |
| Homogeneous transmittance | `render.lab.volumetric`: exp(-sigma_t d) at every pixel within 0.5 percent + 1e-4 at densities 0, 0.002 and 0.005 per unit (worst relative error 1.0 percent, on small values at the edges, inside the absolute term); see the note below | pass |
| Height fog and emission | exp(-analytic optical depth) within 2 percent + 1e-4 at every pixel (worst 1.2 percent; the composite extrapolates the froxel values beyond the outermost column centres); e (1 - T) / sigma_t within 0.5 percent | pass |
| Single scattering against a numerical integral | a point light, albedo 0.6, g 0.5 and -0.3, against Simpson along each ray with transmittance on both legs. Mean relative error 0.0067 and 0.0042 (at most 0.03), p95 0.018 and 0.015 (at most 0.08), frame mean 0.998 and 1.007 (within 0.02) | pass |
| Density zero bitwise | density zero and an empty medium equal the frame without the pass bitwise, with the surface program's legacy range fog on; the foggy hall's clear views at `--fog-scale 0` equal the lab's frames from before the pass, byte for byte | pass |
| No history after a camera cut | the frame after a cut is bitwise a new renderer's frame | pass |
| Seeded stages | `render.lab.volumetric.sensitivity`: phase ignored and albedo ignored (inject) fail scatter.point; extinction applied twice and the slice off by one (composite) fail transmittance | pass (5) |
| Shadowed projector shaft | not built: shadows wait for the shared view-level types (below) | open |
| Foggy-hall fog views against Cycles (K11 "Volumetric fog" and "Ground truth") | `lighting_fixtures.py gallery`, denoised references, tolerance mean 0.10 / p99 0.9; the fog state rendered from its own map baked with the medium (`e0099728`, `lt_foggy_hall_fog` built in the main checkout) | pass: nave 0.070 / 0.59, side 0.076 / 0.38 |

The suites pass with 0 validation messages (synchronization validation on).
- `render.lab.composition` passes 27 checks with the new libraries linked.
- `render.debug-views`, `render.lighting-controls`, `render.lab.area-lights`
  and `render.lab.lightmap-basis` still pass after the canvas change (the
  depth target is now sampled, and draws may be followed by a post pass).
- `shader_toolchain.py check` passes 105 of 105, and
  `shader_artifacts.py check` passes 1,354.
- The new C++ builds with g++ under Waf, and clang++ `-fsyntax-only` is
  clean.
- archlint finds nothing in these files. The 2 CAP002 findings and 2 ARCH105
  findings it reports are in files this slice does not touch. stylelint is
  clean.

**Tolerance note.** The tolerances (0.5 percent + 1e-4 for transmittance
and emission, 2 percent + 1e-4 for height fog) were fixed before the first
run and are unchanged. The first build failed both, and the causes were
fixed, not the rules (43's review):
- **Transmittance precision.** A premultiplied blend of (L, 1 - T) lost a
  small T to the half-float target: a = 0.95 is stored with a quantum of
  2^-11, which is 1 percent of T = 0.05. The composite now writes (L, T)
  under a new port blend mode, `BlendMode::kTransmittance` (src + dst * a,
  the destination's alpha kept). It is render.device.v2 clause D21, with a
  pixel check at a = 0.05 on a half-float target and a bad adapter that
  draws it as premultiplied, and it is implemented in the Vulkan and OpenGL
  adapters.
- **Screen edges.** Within half a froxel of the edge, the froxel values
  were held at the outermost column's value (the height fog's top corners
  were 15 percent off). They are now extrapolated from the two outermost
  columns, clamped at zero.

`render.device.v2.gl.sensitivity`'s early-upload-reuse case (D10) is racy on
radeonsi: it is not detected in 2 of 6 runs at the base commit
(`8dd98ddc`), and the same holds with D21. This predates the slice, and the
port owner has it.

**Gallery** (`lighting_fixtures.py gallery --fixture foggy-hall`, denoised
references, tolerance mean 0.10, p99 0.9):

| View | Before | After |
| --- | --- | --- |
| fog nave | 0.418 / 1.49 | 0.281 / 0.78 |
| fog side | 0.501 / 1.70 | 0.315 / 0.92 |
| clear nave | 0.098 / 1.08 | 0.098 / 1.08 (bitwise) |
| clear side | 0.076 / 0.37 | 0.076 / 0.37 (bitwise) |

**The term against Cycles.** The reference render keeps Cycles' passes. On
the same views:
- the lab's in-scattered light alone (`--inscatter-only`) has mean luminance
  0.206 against Cycles' Volume Direct + Indirect 0.197 (nave), and 0.213
  against 0.201 (side): 5 to 6 percent high;
- the residue is consistent with the missing pillar shadows (unverified
  until shadows land).

What remains in the fog views is the surfaces. Cycles' surface part (Noisy
Image minus Volume) is 0.49 (nave) and 0.47 (side) of the clear frame; the
lab's is 0.68 and 0.65. The lab applies the camera ray's transmittance
correctly, but the fog state's lightmap was baked without the medium:
`map_lighting` bakes the USD stage, and the medium is only an extra of the
Cycles reference. So the light reaching each surface is missing its
transmittance (about 0.72).

**Performance** (binding rule 7; RADV Strix Halo; `render_lab --time 11`,
the pass alone, submission to idle, median):
- 33 ms at 512 x 384;
- 313 ms at 1920 x 1080, against K11's 1.0 ms target.

The inject stage dominates: 16 samples per froxel over 257 lights. A
sampling sweep on the side view shows that fewer samples leave the metric
nearly unchanged:

| `--fog-samples` | Time | Mean / p99 |
| --- | --- | --- |
| 1,1 | 2.4 ms | 0.321 / 0.97 |
| 1,2 | 4.5 ms | 0.317 / 0.94 |
| 2,2 | 16.7 ms | 0.319 / 0.94 |
| 2,4 (the default) | 33 ms | 0.315 / 0.92 |

Optimization items:
- the cluster lists plus step c's global list;
- fewer samples, jittered;
- a per-column integrated volume.

The Fold7 is unavailable (the keyguard is showing).

**Gaps.**
- Shadows. The atlas record and `shadow_sample.glsl` belong to a sibling pass
  until c2 moves them to render.frame and render/shaders/common. The
  projector shaft check is open, and point-light shadows are not available.
- The sun and rect lights in the medium.
- The translucent application (43's frame-group diff).
- Legacy fog off where volumetric fog is on (43's frame-group diff).
- The fog state's bake without its medium (the fixture's and bake owner's).
- The light set's spot rule differs from vrad's (see the c1 follow-up below).
- The cluster lists (performance).
- Clamping at the screen edge in the half froxel beyond the first and last
  column and row centres.
- The pass has no render.graph form yet (a direct `Record`).

### K11 step g: the fog state's surfaces, baked through the medium (2026-09-29)

The foggy hall's one map was baked without its medium, so the lab's fog-state
surfaces kept 0.65 to 0.68 of the clear frame, against Cycles' 0.47 to 0.49.
The fog state now has its own map, baked on the one lighting back end with the
medium as an explicit input. There are no environment variables and no side
files.

- `tools/quality/participating_medium.py` holds the one Cycles form of a
  homogeneous medium: the world volume the K11 references render.
  `lighting_reference_blender.py` now calls it; before, it carried its own
  copy.
- `pbrt_lightmap_bake.py --medium <json>` is opt-in and recorded in the bake
  receipt. The back-end manifest key `medium` is validated and enters the
  bake step's cache key only when given. `map_lighting.light(medium=...)` and
  `--medium` pass it through.
- The published map carries `lightmap-medium.json`, taken from the bake
  receipt. The pipeline refuses a receipt whose medium differs from the
  manifest's. Only the lightmap bake takes the medium: the probe,
  probe-volume, radiosity and SDF bakes don't, and the build log says so.
- In the fixture, a state may own a map (`lighting.state_maps`,
  `lighting_fixtures.map_for`):
  - `build` lights the same compiled BSP again with the state's medium;
  - the gallery renders each state from its own map;
  - `check` requires a state map to carry exactly its state's medium and the
    base map to carry none.
- foggy-hall's fog state renders from `lt_foggy_hall_fog`, and the clear
  state keeps `lt_foggy_hall`.

| Check | Result |
| --- | --- |
| Outputs unchanged without a medium | `lt_foggy_hall` rebuilt with the new code (bake rerun) is byte-identical to the published map (BSP2 sha256 `047aced6…`) |
| Tests | `test_lighting_fixtures` and `test_lighting_gallery` pass 50 tests, including `StateMaps`: a state map's published bake medium must match its state's medium and the base map must carry none, with three seeded mismatches caught; the manifest names a medium only when given; bad media are refused. `generate --check` and `check` pass (the other fixtures' maps are linked read-only from the shared checkout). `test_legacy_relight`, `test_pbrt_map_build_run`, `test_pbrt_gates`, `test_playable_maps`, `test_vmf_map_build`, `test_lightmap_variants`, `test_pbrt_scene` and `test_lighting_back_end` pass 122 tests |

**Gallery** (foggy-hall, denoised references, tolerance mean 0.10, p99 0.9):

| View | Base map (before) | State map (after) |
| --- | --- | --- |
| fog nave | 0.281 / 0.78 | 0.070 / 0.59 (pass) |
| fog side | 0.315 / 0.92 | 0.076 / 0.38 (pass) |
| clear nave | 0.098 / 1.08 | 0.098 / 1.08 |
| clear side | 0.076 / 0.37 | 0.076 / 0.37 |

On the nave, the lab's surface part is now 0.483 of the clear frame, against
Cycles' 0.490. The clear nave's p99 was already 1.08 before this change.
The likely cause, unverified, is the projector's light on surfaces, which the
lab doesn't draw yet (the projected-light term).

### K11 step g follow-up: the fog's lights through runtime_light.glsl (2026-09-29)

43's c1 (`ad33056a`) made `render/shaders/common/runtime_light.glsl` the one
GLSL copy of light_set.h's falloff and spot rules. The fog's inject stage now
includes it (rule 6):
- `medium_light.glsl`'s own falloff and spot code is deleted;
- `MediumLight` loses its exponent, and `MediumLightFrom` takes the light set
  light alone;
- render_lab no longer reads `_exponent`.

**The light set has no spot exponent, and its spot rule differs from the
fixtures'.** `light_set::SpotFactor` is a smoothstep between the cones, with
no exponent and no axis cosine. vrad compiles, and the fixtures' Cycles lamps
render, `cos x ((cos - outer) / (inner - outer))^exponent`, and the fog
followed that rule before this change. The fog now follows the light set, as
the surface program does. Which rule the one definition should hold (vrad's
for baked-light parity, or the smoothstep the native backend used) is the
light set owner's decision (RFC 0011, render.light-set.v1).

| Check | Result |
| --- | --- |
| `render.lab.volumetric` | 18 checks pass (its point-light oracle has no spot) |
| `render.lab.volumetric.sensitivity` | 5 pass (4 of 4 seeded stages caught) |
| `render.lab.clustered-lights` | 16 pass |

Gallery, foggy-hall (denoised references, tolerance mean 0.10). On the
rebased tree the vrad rule gave fog nave 0.070 / 0.59 and side 0.076 / 0.38.
With SpotFactor, fog nave is 0.075 / 0.59 and side 0.083 / 0.38; both still
pass, slightly worse, which is the spot-rule difference above. The clear views
are unchanged (0.098 / 1.08, 0.076 / 0.37).

## K11 step f: screen-space reflections, `render.pass.ssr` (2026-09-29)

Owned by source-engine-5c's d agent (approved by the render-core owner,
source-engine-43, as scoped: the RFC 0016 owner row and gate). State:
design and CPU reference; the GPU pass, the surface-program inputs and the
mirror-corridor comparison are open.

- **Definition:** `public/render/pass/ssr/ssr.h`, the term's one definition.
  - Inputs per view: depth, an octahedral normal with the roughness the image
    light used, the surface's specular weight (split-sum directional albedo
    times specular occlusion), the image specular it added, and the current
    frame's lit colour. No history.
  - The trace: the reflected ray from the pixel, started one pixel footprint
    off its surface, walked texel by texel through the depth buffer in screen
    space. The GPU pass will skip cells through a min-depth pyramid.
  - Hit and thickness: a texel whose depth the ray reaches while less than
    `thickness` (8 units) behind it.
  - Confidence: the product of the screen-edge fade (10 percent of the
    screen), the thickness fade and the roughness fade (from 0.75 of the 0.4
    cutoff).
  - Inputs from the surface: the image-specular radiance and its weight w,
    not the product (see the composite below).
  - Spatial filter: a trilinear lookup in the lit colour's box pyramid at the
    lobe's footprint, log2(2 r^2 L).
  - Composite: lit + c w (reflected - iblRadiance). Roughness at or above
    the cutoff, the background and c = 0 are written unchanged, bitwise.
- **Surface-program inputs:** proposed to source-engine-43 as diffs after its
  c1 and the step-d wiring land (behind its opt-in `kSsrTargets` guard).
  Until then the lab builds the inputs itself.
- **Reference:** `render/lab/ssr_reference.{h,cpp}`: the definition in double
  precision, walking every texel (Amanatides and Woo), with its own matrix
  inverse and pyramid; no code or acceleration shared with the GPU trace.
  `render/lab/ssr_scene.{h,cpp}` ray-casts analytic rectangle scenes into the
  pass's inputs, with each pixel's true mirror reflection.
- Contract clauses S1–S10: `unittests/rendertest/contracts/render.ssr.v1.md`.
- **Composite (agreed with source-engine-43):** lit + c w (ssr - iblRadiance),
  with the surface's `kSsrTargets` outputs (1) octahedral normal and
  roughness, (2) the image-specular radiance and (3) its weight w, the very
  factors of the pbr point's image-specular expression, so the lit colour
  holds w iblRadiance by construction.
- **The first version's post-hoc bound withdrawn:** it bounded the rays that
  stop at a thin occluder within the thickness at 2 percent, set after the 0.9
  percent was seen. The check is now the exact set comparison above, and the
  classification rule was written into the contract before it ran. Its first
  runs exposed two false premises in the rule (the pixel's own rectangle
  excluded; a step considering only the rectangle at its own point), fixed and
  recorded in the contract with no band changed. A nearer bar was added to
  the scene: at the far bar the ray moves about 5 units of depth per pixel,
  wider than half the thickness, so no pixel there is certainly ambiguous and
  the set's ambiguous side would have been vacuous.

| Check | Evidence | Result |
| --- | --- | --- |
| Reference against analytic truth | `render.lab.ssr` (`render_lab suite ssr`): a mirror floor before a patterned wall with two thin floating bars, 256 x 192. The rays that stop early are exactly the screen-space ambiguity set, computed from the analytic scene by the rule in `render.ssr.v1.md` ("The ambiguity set"): of 13,666 floor pixels whose true reflection the camera sees, 252 are ambiguous and all stop early, and 550 are unclassified (a step within the discretisation band of a surface, or at an edge) and excluded. Every clear pixel hits within 1.5 px of its reflection and reflects its light within 3 percent + 0.01 (0 violations). The seeded reference with the thickness in front of the surface fails the set comparison (10,014 violations). The thickness test passes 594 rays behind the far bar that an unbounded thickness stops. 4,756 off-screen reflections all miss. 3,828 hits in the edge band fade below 1, and below 0.1 at 1 percent from the edge. The roughness fade holds at 0.35; at 0.45, rough surfaces and the background are unchanged bitwise. The octahedral round trip is within 2.4e-7 | pass (10 checks, g++ and clang++) |

- **The GPU pass:** `render/pass/ssr/` (`ssr_pyramid.comp`,
  `ssr_trace.comp`, `ssr_view.glsl`, core artifacts in `ssr_spv.h`),
  `ScreenSpaceReflections::Record` on an encoder: one dispatch per level of
  the min-depth pyramid (rounding up, so a cell covers every texel under it)
  and of the lit pyramid, then the trace and the composite, one invocation
  per pixel. The walk skips a pyramid cell whose least depth the ray stays
  in front of over its span and rises a level, else falls one; at level 0
  it applies the definition's hit test, so its hit is the texel walk's.
  The crossing and `behind` are computed in 1 / w, which is affine along the
  screen segment as depth is but keeps its precision where depth crowds
  towards 1: computed in depth, two pixels' confidences differed from the
  reference by 0.0026 (a 0.05-pixel shift of the crossing), outside the
  check's 1e-3; in 1 / w they agree. This is a numerical change of the
  implementation, not of the definition or the check.
- `maxSteps` is now 8192 texels (it bound some rays at 256 in the reference's
  256 x 192 scene).

| Check | Evidence | Result |
| --- | --- | --- |
| GPU against the reference | `render.lab.ssr` (`--validate`): the pass with its diagnostics trace on three scenes (the mirror floor, glossy at 0.35, rough at 0.45), by the rule in `render.ssr.v1.md` ("GPU against the reference", written before its first run). Mirror and glossy: 23,419 judged pixels each (15,497 hits), 1,215 unstable under the six ray tilts and not judged; the same hit texel, confidence and mip within 1e-3 and the composite within 2e-3 of its magnitude + 1e-3 on every judged pixel; every pixel the reference leaves unchanged is the lit input, bitwise. Rough: nothing traced, every pixel bitwise. 0 validation messages (synchronization validation) | pass (20 checks, g++ and clang++) |
| Seeded traces | `render.lab.ssr.sensitivity`: the thickness ignored (2,020 hits differ), no edge fade (5,075) and the wrong mip (15,497 mips differ) each fail a GPU check | pass (4 checks) |

Performance (rule 7, recorded, not judged): the suite runs in about 50 s,
mostly the CPU reference and its tilted runs; the pass's GPU time is not yet
measured.

Next: the lab's mirror-corridor inputs (its own depth, normal and weight
pass until 43's `kSsrTargets` diff lands), the gallery before and after, and
the fallback-seam walk against the 0.047 gate with its hard-switch control.

### The fallback seam, slice 3 (2026-09-29): continuous fades, thin-bar stress

- **Walk harness:** `render.lab.ssr` walks the analytic mirror scene at
  512 x 384 over 16 stations (camera 48 units up, advancing 150 units while
  its target rises 60), with the metric of `render.ssr.v1.md` S9 (the largest
  |c(p) - c(q)| between adjacent traced floor pixels, except pairs whose hits
  differ in clip w by more than the thickness) and a seeded hard-switch trace
  (`SEEDED_SSR_HARD_SWITCH`, `kSsrTraceHardSwitch`) as its control.
- **First measurement, then a decision after the data:** the rule was fixed
  before the first run, but the owner's instruction to wait for its
  confirmation arrived while the run was going, so the decision is post-data
  (disclosed in the contract). First result, with the first definition's
  fades: largest step 1.0; 24,280 pairs above 0.047, 18,752 of them both hits
  by the thickness fade and 3,740 by the edge fade.
- **Decision (source-engine-43):** the metric stays; the thickness fade is
  a ramp over behind in [0, T] and the edge fade spans edgeFade of the screen
  times the hit's footprint J (a ray differential per screen axis, ssr.h
  step 4); the gating walk is mirror-corridor; the thin-bar walk is a stress
  case, recorded; behind-occluder pairs are measured on mirror-corridor, not
  declared away; the hard-switch control stays at least 0.6.
- **Definition, reference, then pass:** ssr.h step 4 first, then
  `ReferenceSsr` (J in double from the camera rays through the pixel's
  neighbours) and `ssr_trace.comp` (the hit's world position interpolated in
  1 / w along the screen segment, the hit plane's normal read at the hit
  texel).

| Check | Evidence | Result |
| --- | --- | --- |
| Reference and GPU under the new fades | `render.lab.ssr --validate`, g++ and clang++: the 20 checks of slices 1-2 and the hard-switch control | pass (21 checks) |
| Seeded traces | `render.lab.ssr.sensitivity`, g++ and clang++ | pass (4 checks) |
| Hard-switch control | the seeded trace's largest step on counted pairs | 1.0 (at least 0.6: pass) |
| Thin-bar stress walk (not gating) | largest step, at station 0 between (189,183) and (190,183): one hits a bar texel it crosses, the other enters the bar's texels 214.9 units behind and reaches the far plane; the reference's walk agrees | 1.0, recorded |

Of the stress walk's 32,552 pairs above 0.047: 522 a hit beside a miss that
left the screen, 494 behind an occluder, 444 passing behind a visible
surface by the thickness or more, 0 by the edge fade, 30,854 by the thickness
fade, 238 other. 30,852 of the thickness-fade pairs have both hits on one
rectangle, with the larger `behind` at median 4.2 and 90th percentile 6.8
units: the depth buffer's one depth per texel makes a grazing ray enter a
texel already behind by up to its depth span, and the [0, T] ramp reads that
staircase as confidence. Measuring `behind` against the hit texel's plane is
a candidate definition fix, left to the owner and to be judged on
mirror-corridor.

Mirror-corridor (the gate, S8 and S9) is not yet drawable as the pbr point
in the lab: `ProgramResolver::Resolve` claims only `lightmapped` and
`unlit`, the lab maps PBRMetalRough world materials to their LightmappedGeneric
diffuse point (`ResolveMaterial`), and `PbrFamily` draws only on the model
vertex with model lighting. The surface program accepts
kWorld | kSurfacePbr with the lightmap basis and map probes (the step d
wiring), but no family or resolver requests it: that is source-engine-43's
b2. `kSsrTargets` follows 43's c2.

Performance (recorded): the suite runs in 21 s on the clang++ lab tree with the walk
(each station runs the CPU reference for the diagnosis).

### What the gallery shows that the metric missed (2026-09-29, user review)

The user reviewed the gallery and found four gaps. Two of them the K11
metric can't see, because it skips visible emitters and weighs a pixel-wide
edge like any other pixel. Each gap is now a K11 check in RFC 0016:

| Gap | Cause | Owner and next step |
| --- | --- | --- |
| material-sweep's gold renders flat yellow | the lab's resolver serves only the lightmapped and unlit families, so world PBRMetalRough is silently remapped to the lightmapped point: no image specular, no metal | render-core owner, b2 (the resolver serves the world pbr point); K11 "World PBR materials" |
| The bulbs are missing | a fixture light is a `light` point entity with no geometry; Cycles shows the SphereLight's surface | fixtures (source-engine-5c): emissive bulb meshes, camera-only in the reference and excluded from the bake; the lab draws them through the emission term (render-core owner's S5). New rule: "lights have no visible shape; content supplies it". K11 "Visible emitters drawn" |
| Jagged silhouettes (pillars) | the lab draws one sample per pixel; the denoised reference averages 256 | RFC 0012's multisampled targets, pulled forward as a lab-first slice; K11 "Antialiased edges" with an edge metric |
| Smeared bulb halos in fog | a bulb's halo is smaller than a froxel (8 px, 96 slices) | the volumetric pass: an analytic per-light single-scattering term; K11 "Small-light fog halos" |

The antialiasing check pulls RFC 0012's multisampled-target work (R65) ahead
of its rank, for the lab. This is a K11 requirement, not a change to R65's
product scope.

## K11: render_lab draws the whole lighting model (2026-09-29, source-engine-cb)

User goal (2026-09-29): "focus fully on getting the runtime in
rendercore/renderlab at a Source 2 level". It named five gaps a best-quality
bake cannot close: no SSR, no ambient occlusion, unshadowed sun specular
(the sun mask was baked but never packed or read), unoccluded area lights,
and projected lights that do not bounce. source-engine-43 (the render-core
owner) was not running. Its unlanded slice c2 was carried onto the branch
first, crediting it (`7c0cb27ea`). No other peer held any of this scope
(source-engine-04, 5a and b4 were asked).

### What landed

- **The lab's map frame is the model's frame.** Commits `e99275216`,
  `fd9c6a68a`, `3a2dd6c6c`, `85c740cb1`, `3e1b52b73`; the attenuation
  mirror followed.
  - **Materials.** `ProgramResolver::SetWorldPbr` draws PBRMetalRough
    world materials as the pbr point, never remapped (K11 "World PBR
    materials"). The product keeps refusing them until K12.
  - **Dynamic models.** They are the pbr point with `kSurfaceMeshDirect`:
    the probe volume's indirect layer, plus every light's direct light at
    runtime with shadows.
  - **Lights from the entity lump** (`LightsFromEntities`):
    - `light` and `light_spot` are clustered runtime lights;
    - `light_rect` are area lights;
    - `light_environment` is the sun (vrad's normal convention);
    - `env_projectedtexture` are projected lights.
  - **Baked lights.** A baked light's diffuse light is the bake's: the
    total layer, directional where the page is. The core adds its specular
    lobe (`spot.y`, `halfV.w`, `sunColor.w`). With `--core-direct` it draws
    both lobes over the indirect layer.
  - **Shadows (`lab_shadows`).** One 8192² atlas holds:
    - spot tiles;
    - point-light cubes and area-light hemicubes, each face drawn 12° wider
      than 90°;
    - four sun cascades;
    - projector frusta.

    The surface samples them with PCSS sized by the emitter
    (`ShadowVisibilitySoft`, `shadow_faces.glsl`). `ShadowTileGpu::params`
    z and w carry the depth mapping.
  - **Shadow terminator.** Two fixes, standard practice for shadow maps on
    smooth-shaded meshes:
    - receivers offset along the facet's own normal, scaled by the tangent
      of the light's grazing angle;
    - shadows faded as the interpolated n·l approaches 0.
  - **Sun.** On lightmapped surfaces the sun's visibility is the bake's
    mask (total page alpha); elsewhere it is the cascades. The disc widens
    the lobe (Karis 2013).
  - **Projected lights.** Both lobes, the cookie array and a shadow tile.
    Their GLSL and GPU record are now one copy (`projected_light.glsl`,
    `projected_light::LightGpu`), shared with `render.pass.volumetric`.
  - **Frame order.** A depth-and-normal prepass (`kSurfaceDepthNormal`,
    invariant positions), then `render.pass.ao`, then the lit pass with the
    SSR targets (`surface_ssr.frag`, `kSurfaceSsrTargets`), then
    `render.pass.ssr` composited, then the fog.
  - **New options.** `--no-shadows`, `--no-ao`, `--no-ssr`, `--no-bounce`,
    `--rsm-size`. `lighting_fixtures.py gallery --resolution N` renders at N
    times the film size and scores a box-filtered copy.
- **`render.pass.ao` (new module): GTAO** (Jimenez et al. 2016) with the
  paper's multi-bounce fit, applied to indirect light only.
  - Specular occlusion follows Lagarde and de Rousiers 2014.
  - On a lightmapped surface it darkens the bake's indirect layer, never the
    direct light.
  - The per-pixel radius is two lightmap texels' world size on lightmapped
    surfaces, so the bake's coarser occlusion is not counted twice. It is
    the pass's radius (48 units) elsewhere.
- **`render.pass.bounce` (new module): the projected lights' one bounce.**
  - Each projector's reflective shadow map (`kSurfaceRsm`) becomes patch
    lights.
  - The patches are gathered into an atlas of the PRBV layout, with the
    probes' depth-moment visibility.
  - The surface samples it with the volume's weights
    (`kSurfaceProbeBounce`, `probe_volume.glsl`'s pair sampling).
- **Pipeline: the sun mask is packed again** (`1709d3c52`). `pbr_map_build`'s
  ktx2 step passes `--sun-visibility`; the marker texels are written only
  with a probe band.
- **Pipeline: WMSH keeps per-corner normals** (`7c81ca163`).
  `usd_worldmesh_pack.py` averaged each triangle's corner normals into
  one. Every USD-built map was flat-shaded while the bake and Cycles use
  smooth normals: facets showed in specular highlights at 4x.
- **Attenuation.** vrad's attenuation for world lights
  (`LightFalloff::Attenuated`, source-engine-5a's header) is mirrored in the
  clustered lights; `SurfaceLightGpu` is 96 bytes.
- **Layouts.**
  - View group: storage 1–5, then texture/sampler pairs 6–11 (atlas,
    cookies, occlusion).
  - Frame group: the bounce atlas at 11, with its sampler at 12.
  - Draw group: the indirect page at 5/6.
  - The family test harness mirrors these, with two-layer cookie arrays.

### Checks

| Check | Evidence | Result |
| --- | --- | --- |
| Projected-light bounce against an independent oracle | `render.lab.bounce` (new): a projector over a floor, a synthesized RSM, and an area integral over the lit floor (600 × 600 cells); interior probe texels within 3 % + 1e-5, borders equal twins, outside zero, an occluded probe dark, lit texels lit | pass (6); sensitivity pass (4: patch cosine ignored, visibility ignored, flat solid angle) |
| GTAO against a ray-traced reference | `render.lab.gtao` (new): an open plane in [0.99, 1]; depth-free pixels one; a crease's mean error 0.021 and 95th percentile ≤ 0.12; the floor darker near the wall | pass (5); sensitivity pass (4: projected normal ignored, screen-spread slices, snapped samples) |
| Vrad-attenuated world lights | `render.lab.clustered-lights` gains an `attenuated` set judged by `RuntimeLightOracle` | pass (22, was 16); sensitivity pass (5) |
| Nothing else moves | every lab suite with `--validate` (shadowed 10, clustered 22, area 40, map-terms 21, volumetric 18, controls 32, debug views 82, probe volume 38, reflection probes 30, lightmap basis 19, ssr 21); 19 product rows (families, world, opaque, material, lights, shadows, composition, Hammer viewport) on g++ and clang++; the product's engine and launcher units that include the changed headers compile with the product tree's `-Werror` flags | pass |

**Found by the new oracles and fixed.** The first `render.lab.gtao` run
failed: an open plane read 0.22. Three defects:

1. The view matrix's rows were read with GLSL's column index.
2. Samples were rebuilt at texel centres. At grazing angles a sample off
   its slice reads as a horizon. Depth is now read at the sample's own
   position: bilinear where the four texels are one plane (exact for
   planes), else the nearest texel.
3. Slices were spread evenly on the screen, not about the view vector. That
   left a 5 % deficit on a grazing plane (a numerical replica confirmed
   it).

**The crease band was set after the first run** (post-data, disclosed).
GTAO fades a far occluder by pulling its slice's one horizon toward open,
where rays count each blocked direction. So it under-occludes near a crease:
the wall 5–30 units above the floor reads about 0.09 bright. That is the
algorithm, not a defect.

**Not covered by these scenes:** the horizon's falloff (ignoring it matches
the reference no worse) and the blur's surface weights.

### Gallery

`lighting_fixtures.py gallery --resolution 2`, against the final references
(`1febd12b`), with the maps repacked privately (smooth normals, sun mask):
**9 of 35 views pass**, up from 4.

| View | Before (mean) | Now |
| --- | --- | --- |
| mirror-corridor low | 0.64 | 0.077, pass |
| mirror-corridor down | 0.66 | 0.101 |
| sun-colonnade along | 0.196 | 0.074 |
| sun-colonnade yard | 0.068 | 0.051, pass |
| foggy-hall clear nave | 0.40 | 0.083, pass |
| cornell-floors front | 0.23 | 0.044, pass |
| projector-cookie wall | 1.03 | 0.056, pass |

Published for the user's review:
https://claude.ai/artifact/QkqBwB7sM7VDeVCmWGBTdf.

Known differences the gallery shows:

- **projector-cookie's references leave the projector's bounce out by
  design** ("projected lights are never baked"), so the new bounce raises
  that fixture's error. Judging the bounce against Cycles needs a reference
  state with the projector's full light paths. The fixture owner has not
  been asked yet.
- **The Cornell probe model is coarser than its reference.** The lab loads
  the stock `models/props/sphere.mdl` (382 vertices, retargeted), while
  Cycles renders the stage's `ProbeSphereShape` (4,512 points). The
  fixture should compile the stage mesh.
- **Emitters are LDR.** Fixture emitters are one `UnlitGeneric`, so bulbs
  and panels draw near radiance 1. K11 "Visible emitters drawn" needs them
  as emissive pbr materials at their light's radiance.
- **Rough gold is yellower than Cycles** (red about 20 % low): the BRDF's
  metal multiple-scattering tint (RFC 0007).
- **The Portal chambers** need alpha-tested and legacy materials the pbr
  resolver does not claim yet.

### Performance (rule 7, recorded, not judged)

Not measured. The atlas is 8192² D32 (256 MiB), and area-room plans 320
hemicube faces. The PCSS sampler takes 32 taps per light per pixel, and
the bounce gather loops every RSM texel per probe texel. Each is an
optimization item for `render_lab --time`.

### Not done

- The shared `run/maps` still has the old packs. The next `--final`
  fixture build repacks them; the bakes are unchanged.
- `render.pass.volumetric`'s lights do not take vrad's attenuation yet.
- No oracle suite yet for area-light shadows (hemicube PCSS against a
  ray-traced rectangle) or for the sun mask's read. The gallery covers both.
- The mirror-corridor seam walk (S8/S9) was not rerun on the composited
  frame.
- The product (K12) is unchanged.

## K12 decision: moving-light GI is kept (2026-09-29, user decision)

Asked what integrating the lab into the game (K12, R96) needs, the agent
listed the runtime-GI producer as an open decision: the frozen native
backend's PBR path reads RFC 0011's runtime volume, and deleting its copies
would drop the traced producers' response to moved and unbaked lights
unless the core took it over first. The user decided: "keep moving-light
GI when the old path is removed".

- Recorded as a rule of the lighting model and a K12 check, "Moving-light
  GI kept", in [RFC 0016](0016-render-core.md#k12-lighting-model-integrated-in-the-product),
  and in R96's done condition.
- Consequence for the K12 order: the producers' GPU work moves to
  `render/pass/indirect/` (the file map already names it) before the
  backend's `probe_volume.glsl` and producer compute are deleted. RFC
  0011's contract, `r_indirect_producer` and the producers' math are
  unchanged; G9's `swing` is the in-game proof on the core.
- Nothing is implemented by this entry, and no row changes state.

## r_core_world: one world per view (2026-09-29, user request)

User request: "when render world is on do not render the new world and
legacy world at the same time".

- Found: on a map with a resident WMSH (`r_worldmesh_draw 2`, the default),
  the opaque world is drawn by the WMSH batches, not the static chains. Only
  the chains skip the core's surfaces (`RenderCoreWorldDraw_Skips`), so with
  `r_core_world 1` the core drew the BSP faces at its slot and the WMSH
  batches drew the same room again over them. `r_core_world 3` and
  `r_core_world_isolate` did nothing on those maps for the same reason.
- Fixed in two parts (`engine/gl_rsurf.cpp`, `render_core_world_draw.*`):
  - In a view the core draws (or under `r_core_world_isolate`), the opaque
    world goes through the chains, which skip per surface, and the WMSH batches
    only collect their translucent meshlets (`RenderCoreWorldDraw_ChainsOnly`).
  - On a map whose world is its WMSH, the core declines the view by name
    (console line at the first view, and `r_core_world_stats: declined N
    view(s)`). The map's WMSH has its own faces, PBR materials and LMAP
    lighting; the BSP faces the core holds carry the LightmappedGeneric
    fallbacks (`gi_door_fallback/wall`) over legacy lightmap pages the bake
    never lit. Drawing them was a silent remap that showed a black room once
    the WMSH stopped drawing over it. The core draws WMSH with world PBR in
    K12 step 2; the decline goes then.
- Evidence (`tools/render/debug_views_product.py`, new `--content-root`):
  - gi_door, before the fix: `cl_render_debug_legacy 1` frames black, and
    the neutral frame lit only because WMSH drew over the core.
  - gi_door, chains-only without the decline: the core's world black
    (727,516 dark pixels of 786,432); this is what the decline prevents.
  - gi_door, both parts: the neutral frame matches the before frame (2,718
    dark pixels), every tint frame fully legacy, the decline named.
  - testchmb_a_01 (no WMSH): 20 of 20 checks pass.

### `./play` defaults to the core world (2026-09-29, user direction)

User direction: "use the same build that ./play uses and configure it to by
default be the best render core defaults".

- `./play` (tree `build/`) now passes `+sv_cheats 1 +r_core_world 1` by
  default on native Vulkan; `--no-core-world` or `CORE_WORLD=0 ./play` opts
  out. `tools/quality/render_flags.sh` takes the default from the caller
  (`RENDER_CORE_WORLD_DEFAULT`); `./play_p2` sets none, because three Portal
  2 maps still fail fatally under the core (core-world smoke known failures).
  The engine's `r_core_world` default stays 0 until K12's "game matches lab".
  `tools/quality/tests/test_render_flags.py` covers the cases.
- Configure defaults unchanged, on purpose: the core's world pass records
  into the backend's own device (the Vulkan adapter's `Port()`), so the
  configured core device (`null`) only carries the frame graph's features.
  `-render-device vulkan` would compose a second VkDevice that nothing draws
  with, and the `skinning` feature adds no passes until K6's product
  skinning feeds it. The best configure line for `build/` is its current one;
  a host-device choice for the core's own frame is K12 work.

## K12 slice 1: the core draws a BSP2 map's world as a world stage (2026-09-29)

User direction: "have bias for action to start integrating the modern
renderlab render core into the game". This slice replaces the decline added
earlier today (a WMSH map's world was left to the legacy WMSH path) with the
core drawing that world, with world pbr, as `render_lab` does.

What landed:

- `render.pass.world` holds a world stage (`WorldData::stage`): a BSP2
  map's WMSH, its surfaces the meshlets, resolved with world pbr
  (`SetWorldPbr`) and lit by the map's own data: the LMAP pages (linear,
  directional split), the baked PRBV, the RPRB reflection probes, the
  split-sum and LTC tables, and the indirect-light host's change volume as
  the second probe atlas (`kSurfaceProbeBounce`). The lightmap (moving
  objects blocking baked light) and the change volume update in place, so
  moving-light GI reaches the core's world through the same change atlas
  the backend reads (the K12 rule recorded this morning).
- `SplitLightmapLayer` moved from `render_lab` into `render.pass.world`
  (one copy; the lab aliases it). `ClaimForDrawing` claims pbr materials
  for a world stage.
- `render.composition`: `IRenderCoreWorld::SetWorldMesh` (the WMSH lump,
  meshlets, batch materials) and `StageUpload()`, a capture of the
  engine's world mesh uploads.
- Engine: every world mesh upload the backend accepts is teed to the
  stage (`render_core_host.cpp`); at level load a WMSH map sends its
  meshlets and batch materials; in a view the core draws, the WMSH path
  hands the visible meshlets of the batches the core claims to the core at
  the point it would have drawn them, and draws the rest (and every
  translucent meshlet) itself. One world per view holds.

Evidence (`build/`, the tree `./play` boots; headless native Vulkan):

| Map | Claimed | Core against the legacy WMSH frame |
| --- | --- | --- |
| gi_door (mode 0 and 2) | 2 of 2 materials | 3,427 pixels differ, all in the far room, where the legacy frame shows streak and scan-line artifacts the core's does not; every other pixel byte-identical; modes 0 and 2 identical |
| lt_material_sweep | 28 of 28 | 15 pixels over 8 levels (max 26) |
| living_room | 18 of 20 (the two gaps stay legacy's) | 3,170 pixels over 8 levels |
| testchmb_a_00_relit (paused) | 42 of 43 | 57,409 pixels over 8 levels, max 20 (LightmappedGeneric on linear LMAP pages) |

- `render.composition`, `.capabilities` and `render.world.null` pass;
  `debug_views_product.py` passes 20 of 20 on testchmb_a_01 (BSP faces) in
  both queued modes; `render_lab` builds with the shared page split.
- No map failed a view (strict mode on).

Not done in this slice:

- The view's runtime lights, shadows, GTAO, SSR and fog: the stage draws
  the baked and probe terms only (no `kSurfaceClustered` view group yet).
- A moving-light check on the core (G9 `swing`) and the change volume's
  pixels: wired, not yet measured.
- The two living_room gaps and the relit map's 20-level residual are
  unexamined.
- Frame time not recorded.

## K12 slice 2: runtime lights on the world stage (2026-09-29)

- The engine's light set (`light_set_publisher.cpp`, every frame) reaches
  the core too: `IRenderCoreWorld::StageLights()`, teed beside the backend's
  consumer in `render_core_host.cpp`.
- `DrawView` now carries the view's world-to-view and projection. For a
  world stage the composition clusters the frame's runtime lights for each
  view (`render.pass.lights` `CreateClusterGrid`, `AssignLights`, desktop
  limits; near and far from the projection), packs each light with its
  baked flag (a baked light's diffuse is the lightmap's, the core adds its
  specular), and the world pass binds the view group per view
  (`StageViewLights`); the stage's scene terms gain `kSurfaceClustered`.
- `r_core_world_stats` names the stage's runtime lights and lit views.

Evidence (`build/`, headless native Vulkan, paused):

| Map | Lights | Core with and without `cl_render_debug_term clustered` | Core against the legacy WMSH frame |
| --- | --- | --- | --- |
| gi_swing (G9's swinging `light_dynamic`) | 1 | 723,566 pixels over 8 levels: the lamp lights the world | max 2 levels |
| testchmb_a_00_relit | 4 (baked: specular only) | max 6 levels (its pbr surfaces are two light materials) | as slice 1 |
| lt_material_sweep | 0 | none | as slice 1 |

Found: PBRT- and fixture-built maps carry no worldlights lump (no vrad pass
writes one), so the engine's light set is empty there and `render_lab`'s
entity lights have no in-game source yet. That is the content item "lights
reach the game" (goal step 6).

Not done: shadows for the stage's lights (the atlas), projectors, area
lights and the sun in the stage's frame terms; lightmapped materials take no
runtime light yet.

## K12 slice 3: shadows for the world stage's lights (2026-09-29)

- The shadow plan moved out of `render_lab` into `render.pass.shadows`
  (`shadow_plan.h`, `PlanShadows`): which tiles each light kind takes
  (spot one, point six, area light a hemicube, the sun its cascades, a
  projector its frustum), `PlanShadowAtlas` and the tile records. The lab
  draws its atlas from it (one copy; `render.lab.composition` still finds no
  render.composition in the lab).
- For a world stage, the composition plans each view's shadows with its
  lights (points and spots for now) and packs every light with its tiles.
  At the view's slot it draws the plan's depth views of the stage mesh
  (its positions staged once per map) into a 4096-texel atlas as a graph
  submission ahead of the frame's; the atlases are a per-frame pool (one per
  shadowed view), reused frame to frame. The world pass binds the atlas
  (`WorldTarget::shadowAtlas`); a light with tiles and no atlas fails the
  view by name.

Evidence (`build/`, headless native Vulkan):

| Map | Result |
| --- | --- |
| gi_swing (paused) | the swinging lamp's point light is shadowed by the room; against the legacy frame (SDF shadows) 15,167 pixels over 8 levels, max 34: the legacy SDF shadows darken the room's seams where the core's cube shadows do not; Cycles decides (the G9 references, next) |
| testchmb_a_00_relit (fixed camera) | 41,835 pixels over 8 levels, max 16 |
| gi_door, lt_material_sweep, living_room | unchanged from slice 1 |

`render.lab.shadowed-lights`, `.area-lights`, `.composition`,
`render.composition`, `.capabilities`, `render.world.null` pass;
`debug_views_product.py` 20 of 20 (testchmb_a_01, mode 2).

Not done: moving objects as casters (props, R89), static-tile caching, a
mobile atlas budget, area lights, projectors and the sun on the stage.
Frame time not recorded.

## K12 slice 4: GTAO on the world stage, and `./play_p2` on the core (2026-09-29)

GTAO in the product, in render_lab's order:

- For a world stage view the world pass draws the view's depth and normal
  prepass into its own single-sample targets (a second, single-sample
  resolver: the backend's target may be multisampled, and GTAO samples a
  plain depth), then calls the composition's screen passes, which run
  `render.pass.ao` into the stage's occlusion target; the lit view group
  reads it (`kSurfaceAmbientOcclusion`, which darkens the bake's indirect
  layer only). Stage views always bind a view group of their own now, with
  or without runtime lights. The occlusion reconstructs positions with the
  projection the world pass rasterizes with (its D3D9 half-pixel shift) and
  the eye from the view matrix.
- Found on the way: the world pass keyed its draw groups by (layout,
  page), but the pbr point shares its draw layout with the unlit and
  lightmapped points while reading three pages. Whichever program built
  the group first won: on gi_door the unlit emitter did, so the pbr walls
  read the neutral white texture as their indirect page and any occlusion
  blacked them out. Draw groups are now keyed by the program's draw inputs
  too. It affects every stage mixing pbr with unlit or lightmapped
  materials (the relit Portal maps).
- The stage takes its probe volume from the first volume the host
  publishes: a traced producer publishes a change from its first frame, so
  a bake-only volume may never come; a stage set before it arrives is set
  again.

| Check | Result |
| --- | --- |
| gi_door, `cl_render_debug_term ao` on against off | 0 pixels over 8 levels (max 3): the crease darkens from 26 to 24, as render_lab's GTAO does at the same camera (0.94 of the total at the crease) |
| living_room against legacy | 46,443 pixels over 8 levels, of which 45,425 are the ao term (GTAO and the materials' AO, which the legacy WMSH path lacks) |
| gi_portal_light, gi_portal_view, lt_portal_pair (user request: portal light in the testing) | within 4, 14 and 2 levels of the legacy frames |

`./play_p2` (user direction: "make play_p2 use the render core totally as
well") now defaults to the core world, as `./play` does (`--no-core-world`
or `CORE_WORLD=0` opt out). The four known core-world smoke failures are
fixed, and the workload lists none:

- sp_a3_03, sp_a3_speed_ramp, mp_coop_tbeam_polarity2: a frame straddling a
  level change (queued mode records its slots after it) named views of the
  earlier world, which the pass failed. They are skipped now: the queue keeps
  each view's world generation, and an earlier world's view draws nothing
  and counts as skipped (`render.world.null` W4 and W7 changed to this rule).
- sp_a3_crazy_box: its cubemap-patched walls name
  `metal/metalwall_bts_001a_normal`, which no Portal 2 archive holds. A
  texture the content lacks is absent for the core (its neutral value) and
  named once at level load; a texture that exists but no draw has used yet
  is downloaded first.

Sweeps: all 117 retail Portal 2 maps (`core_world_smoke.py --game portal2
--build build-p2`): 115 pass, 2 no-claims, 0 fail; all 26 retail Portal
maps pass (`build/`).

## K12 slice 5: the map's authored lights, area lights and the sun on the stage (2026-09-29)

- `render.pass.lights` owns a map's authored lights now
  (`map_lights.h`: the entity lump parser, the value helpers, and
  `MapLightsFromEntities`: inverse-square `light` and `light_spot`,
  `light_rect` area lights, the `light_environment` sun, projectors),
  moved from `render_lab` (one copy; the lab aliases it).
- The engine hands the stage its entity lump (`SetWorldMesh`). Maps
  compiled without vrad (the PBRT and USD pipelines, the lighting fixtures)
  have no worldlights lump, so their lights reach the game only this way:
  when the frame's light set has no world lights, the stage adds the map's
  lights (each light counts once). The stage's views pack the map's and the
  frame's area lights (with hemicube shadow tiles) and the sun (with
  cascades, or the lightmap's baked mask when the page packs one) into the
  frame terms.
- Level load names the stage's authored lights.

| Map | Term toggled | Pixels over 8 levels |
| --- | --- | --- |
| lt_material_sweep | clustered (the map's light) | 5,316: the glossy spheres' highlights |
| lt_area_room | area | 3,942 |
| lt_sun_colonnade | sun | 0: its pack predates the sun-mask fix (the lab's note), so the mask reads no sun; a repack (goal step 6) decides |

`render.composition`, `render.world.null` and the lab suites (area and
clustered lights, volumetric, map terms, composition, shadowed lights)
pass; `render_lab` builds.

### Frame time with the core world (perf, rule 7: recorded, not judged)

`frame_pacing.py --runtime run/runtime --build build --ab-build build
--ab-extra-arg +sv_cheats --ab-extra-arg 1 --ab-extra-arg +r_core_world
--ab-extra-arg 1 --rounds 3 --mat-queue-mode 2` (portal-frame-pacing-v1,
testchmb_a_01, desktop Linux, `build/` at 3293a047d, host load 5-9 from
other sessions' work): legacy warm median 6.59 ms, core world 7.38 ms (B/A
1.12), p99 43.5 against 46.2 ms (1.06). Noisy host; interleaved rounds
disagree by up to 2 ms. Fold7: unavailable in this session. Optimization
items only: the core world's per-view cost has not been profiled.

### Held back: dynamic lights on the BSP faces

The CPU lightmap path adds dlights into the pages; the core may take that
term for the surfaces it draws only if the CPU path stops adding it for
them (rule 4, "each light counts once"). The core draws the outermost
back-buffer view alone: views through portals and monitors are nested and
drawn by the legacy stream from the same pages, so stopping the CPU dlights
would drop them there. That term waits for the core drawing nested views
(RFC 0016 K8, portal views), and stays the CPU path's meanwhile.

## K12: emitting surfaces lit by the core on retail BSP faces (2026-09-29, source-engine-89)

User request: self-illuminated emitters (Portal 2's arm-panel strips, cores,
panels) light their surroundings through the render core, not the CPU
lightmap path. Agreed with the core world's owner (source-engine-cb).

- **Light set.** The client publishes up to `kMaxFrameAreaLights` (64)
  emitters a frame, most important first (`r_area_lights` 64 on desktop, 4
  on Android). The first 8 keep dlight slots for the CPU paths (models'
  stand-in lights, the lightmaps of surfaces the core does not draw); the
  engine publishes all of them in `Snapshot::areas` (`AreaLights_Frame`).
- **The core.** A view without a stage (the claimed BSP faces of a retail
  map) binds its frame area lights alone (`CoreWorld::AreaViewLights`,
  sharing `PackViewAreaLights` with the stage's packing); the world pass
  hands every view's areas to its frame terms. The `lightmapped` point adds
  `tint x AreaLightIrradiance` (the form factor at the mapped normal times
  radiance and window, unshadowed) to its diffuse light, for the lights
  whose diffuse is not in the bake. `surface_program.glsl` gains
  `AreaLightCorners` (shared with the pbr loop) and `AreaLightIrradiance`.
- **Each light counts once.** Emitting surfaces are packed with their
  diffuse *not* in the bake, on stages too (a stage's LMAP never held them;
  slice 5 packed them as baked, so a stage showed their specular only). A
  map's `light_rect` fixtures stay baked. The engine leaves area light out of
  the lightmaps of surfaces the core draws (`RenderCoreWorldDraw_OwnsLighting`:
  `r_core_world` on and the core takes the surface), and marks such a
  surface only to clear light its page still holds.

### Held back: area light in nested views

As for dlights (slice 5), the core draws the outermost back-buffer view
alone. Portal and monitor views are drawn by the legacy stream from the same
pages, which no longer hold area light for surfaces the core takes, so those
surfaces show no emitter light through a portal or on a monitor until the
core draws nested views (K8). Surfaces the core does not take keep the CPU
path's 8 slotted lights in every view.

Evidence:

| Check | Result |
| --- | --- |
| `render.family.lightmapped` (+4 area checks: an unbaked light brightens, twice the radiance adds twice the light within 2 levels, a baked light and one facing away add nothing) | 198 checks pass; seeded gamma variant still rejected |
| `render.family.unlit`, `.pbr`, `render.lab.area-lights` (+sensitivity), `render.world.null`, `render.composition`, `render.material.programs` | pass |
| sp_a2_intro exit corridor, paused, arms on skin 0, `r_area_lights_scale 50`, core on against off | walls cyan across the view; the legacy path (8 slots) about a third as much |
| Same, physical strength (scale 1) | no pixel changes by more than 2 levels (auto exposure) |

Not done: shadows (the lightmapped point's area term is unshadowed: the core
holds no casters for a retail world, and models are not in the core's
scene); models still take the 8 slotted lights through the CPU stand-in;
`surface_program.glsl`'s term was changed without source-engine-43 (not
running), recorded here for its review. Frame time: not measured (rule 7).

## K12 slices 6 and 7: the SDF producer on the core, and sparse probe-volume publication (2026-09-30, source-engine-5a)

Slice 6 (`273f37c6a`): `render.pass.indirect`'s `PortCompute` implements
`gpu_compute::IGpuCompute` over `render.device.v2`, and the SDF producer
(the desktop default for moving lights and occluders) runs its traces
through it on the render sequence. `sdf_probe_trace.comp` lives in
`render/pass/indirect/` and builds as a core artifact. The backend's
builtin, its SPIR-V row and its layout are deleted. The ray-query producer
keeps the backend's compute until the port has acceleration structures.
`render.indirect-light.sdf` passes 36/36 on the core device, and the
`gi_door`, `gi_swing` and `gi_portal_light` stage frames are unchanged.

Slice 7: publishing and consuming a probe volume costs what changed, not
the whole volume. On `testchmb_a_15_relit`, a SDF producer re-sent the
whole 203 MB volume about every other frame. Moving doors and rotators
also forced a full re-occlusion, copy and re-upload each frame. That was
87% of the main thread, and the map ran at 5.6 fps.

- The producer keeps a pool of three volumes. Each publication writes only
  the probes a pooled volume is stale in plus the ones traced this frame.
  It publishes the changed list (`PublishedVolume::changed`), and the
  switcher forwards it when the publication follows the one consumed
  (`FrameVolume::changedSince`).
- The host follows the consumed volume when the SDF producer sends a
  changed list, or when only proxies moved. The touched probes are the
  changed ones plus the probes the old and new proxy cuts reach
  (`OccludeProbeVisibility`'s cut list). The pooled occluded copy restores
  and re-occludes only those tiles. Brush entities are relit only where
  their luxels read a touched probe (`ProbeVolumeView::SampleProbes`). The
  upload carries only the touched tiles' regions (`ProbeTileRects`, merged
  per section).
- The upload path is regions end to end: the `world_mesh_upload` request's
  partial form, the queued capabilities, the backend's region uploads into
  its current atlas and delta, and the core world stage's patch log.
- The device port gains region copies (`TextureBufferCopy::x, y`, clause
  D22 `RegionCopies`, with the bad adapter `kDropsRegionOrigin`). It
  passes on the null, Vulkan and GL adapters.
- Controls: `r_indirect_sparse` (1; 0 consumes every publication whole).
  `r_indirect_sparse_verify 1` recomputes each sparse update whole and
  counts differences.

Checks:

- `render.indirect-light.sdf`'s `SparsePublication`: 160 publications
  under a moving focus with budget 3, 159 sparse, 0 differ from the
  composed whole, 0 uncovered probes. A control that drops the stale set
  is caught.
- `render.indirect-light`'s `SparseHelpers`: tile rects, change atlas by
  probe against whole, tile copies, cut list against changed visibility,
  restore, and probe sampling isolation. The control is caught.
- In game, verifying: 0 of the sparse updates differ from whole.
- `render.device.v2` (null 471, Vulkan 930, GL 879 and their sensitivity
  suites), the indirect-light, radiosity, policy and switching suites,
  `render.world.null`, composition, legacy capabilities and
  `world.probe-volume` pass on g++. The main ones also pass on clang++.

### Cost (rule 7: recorded, not judged)

Fixture `./play_p2 sp_a2_laser_intro_relit` (user direction), 1920x1080,
offscreen SDL, `mat_vsync 0`, desktop Linux on a host shared with other
sessions. Time to consume a publication (`r_indirect_report 1`, new
`(N ms)` field), over the first 20 s after spawn:

| `r_indirect_sparse` | Consumed whole | Consumed sparse | Total |
| --- | --- | --- | --- |
| 0 | 66, median 187 ms, max 251 ms | — | 12.7 s |
| 1 | 30 (the start fade has no changed list), median 190 ms | 39, median 44 ms, max 76 ms, about 3,200 probes each | 7.9 s |

Once the map settles, publications stop, and frame time over the last
1,200 frames does not separate. Interleaved rounds gave sparse on 15.8
and 17.2 ms median, and sparse off 17.3 and 16.3 ms. Steady-state cost is
elsewhere, most likely the world stage's GPU passes. Those need
per-pass timers (RFC 0014 D4, next). Fold7: unavailable in this session.

Optimization items:

- a sparse consume still takes 44 ms, mostly re-occlusion and brush
  relight on the main thread;
- the start fade publishes without a changed list.

Frozen-path: `materialsystem/shaderapivulkan/vulkan_world_lightmap.cpp`
(region uploads) and `engine/gl_lightmap.cpp` (the sparse brush relight;
committed inside `1666232fb` with source-engine-89's hunks) are defect
fixes for the frame-time failure.

### The core's quality settings in the video options (2026-09-30, user request)

`r_core_ao_quality` (0 off to 4 ultra) and `r_core_shadow_quality` (0 off
to 3 high, the shadow atlas size) are rows of the advanced video options.
Each row's index is the ConVar's value; both take effect from the next
frame. The remaining `RenderCoreWorldQuality` settings now have rows too:
`r_core_depth_prepass`, `r_core_shadow_movers`, and `r_core_runtime_direct`
each use Off/On. The first two take effect from the next frame; runtime
direct light takes effect at the next map load. All five are archived.

- Portal: `gameui/OptionsSubVideo.cpp` makes five combos in three rows below
  High Dynamic Range and Indirect lighting. The retail `.res` predates them;
  the dialog moves later controls and grows when those rows need space.
- Portal 2: `CAdvancedVideo::PreApplyControlSettings` adds five dialog-list
  rows to the retail `advancedvideo.res` data, copied from Model / Texture
  Detail, updates keyboard navigation, and grows the frame to fit. Use
  Defaults restores the ConVars' defaults.
- The retail localization has no such tokens, and its `GameUI_Ultra` reads
  "Very High". Each dialog registers English text for
  the five row labels, `GameUI_QualityOff`, `GameUI_QualityOn` and
  `GameUI_QualityUltra` unless a loaded file defines them.
- A product without the ConVars shows neither row.

Evidence (headless, native Vulkan, private runtimes): the developer
commands `gameui_show_video_advanced [ao shadows]` (Portal) and
`ui_show_video_advanced [ao shadows]` (Portal 2) open the dialog, print
the selected rows and apply a choice as OK/Apply do. Portal: the dialog
showed `High *`/`Medium *` (defaults marked recommended), applying `1 3`
set the ConVars to 1 and 3, and after `r_core_ao_quality 4;
r_core_shadow_quality 0` it reopened on `Ultra`/`Off`. Portal 2: the
dialog opened on `Low`/`High` after the ConVars were set to 1 and 3, and
applying `4 0` set them to 4 and 0. Neither log has a localization or
resource warning from the new rows. Not covered: mouse or keyboard input
on the new rows.

The three additional rows compile in the Portal GameUI and Portal 2 advanced
video translation units, along with the engine ConVar owner. The changed-line
style check passes. A native dialog interaction and reopen check for these
three rows was still needed at that checkpoint; the front-end keyboard
regression below supplies a bounded interaction check. This UI extension
does not close K12.

### Portal 2 front-end graphics option input (2026-10-03)

The reported Runtime Direct Light row changed from On to Off in a loaded
map, but appeared stuck On in the front-end. One native keyboard Left event
reproduced two `KeyCodePressed` deliveries: the row emitted `_coredirect0`,
then `_coredirect1`. `BaseModHybridButton::OnKeyCodePressed` changed the list
selection and forwarded the handled key to `EditablePanel`, whose default
button navigation reposted it to the same row. Binary choices therefore
returned to their original value. This was an input-routing defect, not a
map-only setting restriction.

The three handled list-key branches now return after changing selection.
Unhandled navigation, ordinary buttons and the IgnoreButtonA parent route
used by Apply retain their previous behavior. Runtime direct lighting still
takes effect at the next map load, as specified above.

`vgui.portal2_video_input` compiles exact method slices from the production
button source against a parent-routing fake, with release-active assertions.
Its 133 checks cover all five core option rows with ordinary and reposting
parents, keyboard/controller aliases, wrapping, disabled choices, command
counts, unhandled navigation and Apply routing. The sensitivity suite
`vgui.portal2_video_input.fallthrough` restores the old fallthrough and fails
64 of those checks. Both suites are installed in the shared conformance
manifest; the generated include has an explicit owner in the module manifest.

Native Linux Portal 2 evidence uses an isolated runtime, private HOME and
D-Bus, a headless Mutter Wayland compositor, and compositor keyboard events.
Left changes Runtime Direct Light to Off once; Apply archives
`r_core_runtime_direct 0`; reopening Advanced Video displays Off. The
retained `quality-results/video-options-20261003/` directory contains the
driver, keyboard events, launch command, console logs, screenshots, build
logs, compiler command and positive/negative conformance evidence. This
checks front-end selection and persistence; mouse input and a new native
in-map interaction run are outside this bounded evidence.

Verification: the Portal 2 client/VGUI build, both release conformance suites,
full architecture check, baseline/inventory verification, 166 archlint tests,
38 stylelint tests and the new fixture's style check pass. The whole-workspace
changed-line style check reports an unrelated reflection-probe fixture change;
the graphics-input changes have no style failure. Existing CRLF line endings
are preserved (`git -c core.whitespace=cr-at-eol diff --check` passes).

Reproduction:

```sh
python3 tools/quality/conformance.py check --suite vgui.portal2_video_input --suite vgui.portal2_video_input.fallthrough --config release --out quality-results/portal2-video-input.json
WAFLOCK=.lock-waf-p2 python3 ./waf build --targets=client,vgui2,vguimatsurface -j8
```

## RFC 0014 D4, first part: per-pass GPU timers on the core (2026-09-30, source-engine-5a)

User direction (2026-09-30): make perf diagnosis easy, starting with RFC
0014 D4's GPU timers on each pass, reported per frame. source-engine-43
owned R95-DEBUG-CONTROLS but is not running; this session took D4's
`_gpu_timers` and `_stats` controls. The rest of D4 (`_sync_paranoid`,
`_poison_reuse`, `_graph`, `_rt_list`, `_rt_view`, `_pipeline_miss_log`,
`_pass_merge`) is still open.

- `render.device.v2` gains timestamps: `Capability::kTimestamps`,
  `DeviceFacts::timestampPeriodNs` and `CommandEncoder::WriteTimestamp`
  (clause D23, in the contract with D22). Vulkan uses a query pool per
  submission, reset at its start and copied into the buffer at its end.
  GL uses `glQueryCounter` with a query-buffer-object write. The null
  device keeps a clock of 10 ticks per command.
- `CommandEncoder` takes an `ILabelObserver`. `render.graph`'s
  `GpuPassTimers` (`pass_timers.h`) writes a timestamp after each label
  opens and before it closes. Graph passes are labeled by name, so every
  graph pass is timed. Both executors take the observer
  (`SetLabelObserver`).
- A frame's times are read only after a token covering its submissions
  completes. Each encoder takes its own 64-timestamp chunk buffer, so no
  encoder's transition can discard another's timestamps.
- The core world stage times each view: shadow depth, prepass, GTAO (now
  labeled) and the lit world, plus the view's CPU recording time.
  `cl_render_debug_gpu_timers 1` turns them on (latched per frame), and
  `cl_render_debug_stats 1` prints per-frame means once a second.
  `-vkgputimers` stays until the frozen backend is deleted (the RFC's rule).

Checks:

- D23 on the null, Vulkan (RADV) and GL (radeonsi) adapters:
  - timestamps inside and outside rendering land;
  - they do not decrease;
  - a later submission's are no earlier;
  - device-local memory and unaligned offsets are refused;
  - without the capability, submission fails `kUnsupported`.

  `kDropsTimestamps` is caught (`render.device.v2.sensitivity`).
- `render.graph.v1`'s D4 clause:
  - every label is timed at its depth;
  - nothing is read before completion;
  - the timers are monotonic;
  - pass times sum within the frame's span;
  - a device without timestamps records none.
- Graph, device, composition, world, lab (composition, GTAO, shadowed
  lights), SDF and Hammer viewport suites pass on g++ and clang++.

### First per-pass numbers (rule 7: recorded, not judged)

`./play_p2 sp_a2_laser_intro_relit` (user's perf fixture), 1920x1080,
offscreen, `mat_vsync 0`, desktop RADV (Strix Halo). Per-frame means over
about 67 frames, on a quiet host:

| Section | GPU ms |
| --- | --- |
| core world view (total) | 5.9 |
| GTAO | 3.2 |
| lit world | 2.4 |
| prepass | 0.25 |
| shadow depth (its own submission) | 0.1 |

- The core view's CPU recording is 0.44 ms.
- The whole frame's GPU time (`-vkframestats`) has a median of 6.6 ms,
  and the core sections sum within it.
- The frame interval is 14.8 ms, of which 10.7 ms is backend CPU:
  `mesh_draw` 3.2 ms, `emit` 2.9 ms, `record` 1.5 ms.

So on this fixture the frame is CPU-bound in the legacy draw stream. On
the core's GPU side, GTAO (full resolution) is the largest term. A second
run on a GPU loaded by other sessions scaled every section by about 1.8x.
Fold7: unavailable.

## K8 water, first slice: Portal 2 goo on the core (2026-09-30, user request)

User report: "the water/goo texture looks terrible on portal 2. fix it on
rendercore", then "make sure this is rendercore native". The goo in
`sp_a2_catapult_intro` drew as black water with large orange blotches.

Causes, each found against retail captures (`portal2_material_shots.py`):

- The goo is Portal 2's `Water` with `$flowmap`: flowing normal maps, a
  flowing sludge layer (`$basetexture` with `$color_flow_*`), lightmapped
  water fog and a fresnel reflection. This SDK's water shader has none of
  it; its port drew the base texture at the surface's scale with bumped
  lightmaps.
- Retail's water shader is the CS:GO `water.cpp`/`water_ps2x.fxc` (its
  stdshader_dx9 names `$color_flow_displacebynormalstrength` and
  `$forceenvmap`, and no `$flow_timescale`), not F-Stop's older copy.
- Without `$reflect2dskybox`, retail's reflection view clears to black and
  draws no sky; this client drew the 2D sky box into it, which showed as
  white blobs through the broken ceilings.
- Old Aperture goo (`$forceenvmap`, `sp_a3_jump_intro`) is opaque in
  retail; this SDK's shader blended it as cheap water, so the engine sorted
  it into the translucent world pass (no core view ever saw it) and drew a
  dark blue plane.
- The `water_mist_*` particles over the goo lerped toward raw point
  lighting: this SDK's "Color Random" initializer ignores Portal 2's `tint
  blend mode` (multiply) and `light amplification amount`.

What landed:

- A `water` family (`render/material/water_family.{h,cpp}`, VMT rows in
  `vmt_mapping.cpp`) and a water point of the surface program
  (`kSurfaceWater`, `WaterSurface()` in `surface_program.glsl`): Portal 2's
  `water_ps2x` above water without refraction, with the CS:GO constants
  (reciprocal flow scales, sRGB-curve fog color, Source's gamma table for
  the tint, the tint's x4 in integer HDR as a frame term). The flow map and
  flow noise ride the material group's env map mask and MRAO bindings (a
  group has 16 GL slots); the planar reflection is a view input (view group
  binding 12, `SurfaceScreenInputs::planarReflection`,
  `ResolvedProgram::viewInputs`), imported per view because the backend
  replaces the target's image on resize. `SurfaceFrame` gains the shaders'
  time, the tint scale, the camera's right in the water plane and the
  view's viewport. Refraction, water seen from below, the cheap path,
  bumped-lightmap water, flow debug views and bump transforms are refused
  by name (claim-time gaps).
- The core world takes water surfaces (`SurfaceEligible` no longer drops
  `SURFDRAW_WATERSURFACE`); a view's water-plane offset (the client's
  `waterZAdjust`) reaches the world pass (`DrawView`'s `waterZOffset`).
- Frozen-path plumbing, each named in its commit: the backend imports
  render targets (they rest in the sampled layout between their passes) and
  passes the slot's time and tint scale (`CorePassTarget`); the legacy
  `Water` shader declares Portal 2's parameters with retail's defaults and
  draws `$forceenvmap` water opaque (retail's rule).
- Not render paths: the Portal 2 client's `$reflect2dskybox` rule
  (`viewrender.cpp`), and Portal 2's tint blend modes in "Color Random"
  (`particles/builtin_initializers.cpp`, sharing the existing
  `ComputeLitParticleColor`; mode 0 with amplification 1 is the old rule).

Evidence (worktree `water-core`, build-p2 and a Portal build, `-Werror`):

- `render.family.water` (41 checks): the claim on Portal 2's VMTs and six
  GPU cases judged per pixel against an independent C++ transcription of
  `water_ps2x` (1x1 inputs, a 1x2 reflection target so the vertical flip
  and viewport mapping are judged; `$fogcolor` and `$reflecttint` decoded
  from the VMT, not the packed constants). Its seeded row
  (`RENDER_MATERIAL_WATER_SEEDED_GAMMA_FOG_COLOR`) fails 5 pixel cases and
  the packing check.
- `corpus.portal2.water-retail` (39 checks, 2 of 2 repeats): the new
  `quality/workloads/portal2-water-v1` views of `sp_a2_laser_over_goo` (a
  second-apart series and a grazing view), `sp_a2_catapult_intro` and
  `sp_a3_jump_intro` with `r_core_world 1`, judged against a retail
  reference recorded from retail portal2_linux (numbers only). Its control
  with the core's world off (the legacy port) fails 13 checks.
  `portal2_material_shots.py` now reads a workload's own checks and
  reference.
- Unchanged: `render.family.{unlit,lightmapped,vertexlit,pbr}` and their
  seeded rows, `render.material.programs`, `render.world.null`, the world
  pbr and glass pixel suites, the Hammer render suites, and every
  `render.lab.*` suite and sensitivity row on a fresh `build-rc-lab`.
  `render.material.v2` pins the five families; `vmt-corpus-v1.json` moves
  46 Portal and 59 Portal 2 materials from `legacy` to `water` (no other
  change). A Portal `escape_00` boot with `r_core_world 1` passes.
- Pre-existing, not from this slice: `vmt-corpus`'s `shader-table.current`
  (`black.cpp` was added without regenerating `legacy_shaders.inc`) and
  `render.material.proxies.inventory` (line numbers in untouched files).

Open:

- The reflection image is still drawn by the client's reflection view
  through the legacy stream (core slots draw only into the back buffer);
  the water surface itself is the core's. A core-drawn reflection view
  needs render-target slots.
- `$reflectonlymarkedentities`: this client reflects no entities (no
  `EF_MARKED_FOR_FAST_REFLECTION`), where retail reflects marked ones.
- The mist particles still show orange smoke tiles at some instants: the
  native SpriteCard stage does not implement `$DUALSEQUENCE` or
  `$MAXLUMFRAMEBLEND` (its own unimplemented notes), so a smoke sheet's fire
  frames can show. That is the particle cohort, next.
- Refraction (Portal 2's `*_beneath` materials, seen from under the goo),
  `$pseudotranslucent` blending, the flashlight on water, and the water
  cohort's K8 legacy-stream census. No Fold7 or Apple run; frame time not
  measured (rule 7).

## K12 performance: cached shadow tiles, quality settings, and the SDF read-back (2026-09-30, source-engine-5a)

User direction (2026-09-30): reach a playable frame rate without losing
quality, following the published implementations: Doom Eternal's and
HDRP's cached shadow maps, XeGTAO's presets (cloned to `~/src/refs/XeGTAO`,
MIT), and Activision's GTAO. Each change was located with RFC 0014 D4's
timers or a perf profile.

- **Shadows** (K12 goal step 2), in `render.pass.shadows`, proven in
  `render.shadows.pixels`:
  - `ShadowAtlasTarget::keep` loads the atlas and clears only the given
    views' tiles (a depth-always triangle), so the stage redraws only the
    tiles whose view changed. Its casters are static.
  - C1: an atlas kept across executions, with only the moved light's tile
    redrawn, equals a full redraw texel for texel, and keeping the old
    tile is caught.
  - `ShadowCaster` takes index ranges. C2: casters split into ranges equal
    the whole meshes, and dropping half is caught.
  - The stage chunks its casters into 512-unit cells and draws only the
    chunks inside each tile's frustum.
  - The stage shadows only the lights its clusters list for the view.
- **Quality settings** (`IRenderCoreWorld::SetQuality`), both archived:
  - `r_core_ao_quality`: 0 off (no prepass or GTAO), then XeGTAO's presets
    1–4 (1×2, 2×2, 3×3, 9×3 slices × steps); default 3.
  - `r_core_shadow_quality`: 0 off, then a 2048/4096/8192 atlas; default 2.
  - The Video > Advanced rows in P1 and P2 are `f4c981624`.
- **SDF read-back:** `gpu_compute::IGpuCompute::WrittenRanges` lets a
  producer say which bytes its dispatches write. `PortCompute` copies home
  only those ranges. The SDF producer names its scheduled probes (1,728
  bytes each), and a slot changes only there, so the host copy stays
  exact. Before this, every update copied the whole field back on the
  render thread (57% of that thread). Halving the range fails
  `render.indirect-light.sdf`'s sparse-publication checks.

Frame time (rule 7: recorded), offscreen at the SDL cap of 1024x768,
`mat_vsync 0`, desktop RADV, quiet host:

| Fixture | Before | After | GPU (after) |
| --- | --- | --- | --- |
| `./play_p2 sp_a2_laser_intro_relit` | 15.6 ms median (64 fps) | 8.8 ms (112 fps), p99 10.6 | 4.4 ms |
| `./play testchmb_a_15_relit` | 22.4 ms (45 fps) | 15.2 ms (72 fps), p99 21.9 | 6.0 ms |

On a_15, 255 shadow tiles are kept and 0 drawn per frame, and GTAO is
1.2 ms (it was 3.2).

Checks:

- `render.shadows.pixels` (C1, C2) and the lab suites (shadowed lights,
  GTAO, composition) pass;
- `render.world.null`, `render.indirect-light(.sdf)`, `render.graph.v1`
  and the Hammer viewport suites pass on g++ and clang++;
- `render.composition(.capabilities)` pass on HEAD with this change
  alone. In the shared tree they fail to link on source-engine-e2's
  unfinished panels wiring.

Next: the a_15 frame waits about 7 ms beyond its CPU work (profile
next), and the 30 s GI settle after a load.

## In-world panels: the chamber sign as an emissive surface on the core (2026-09-30, user request)

User request: "make sure the test chamber signage is a first class emissive
material, and is rendered at the proper resolution for the screen space it
takes up. no blurry text here". Follow-ups in the same session:
- its flicker shows a valid frame;
- its dirt is on it in every flicker state;
- it works with the render core and is an area light;
- it casts light "like it would in real life if the screen was actually
  dirty".

Owner: source-engine-e2. 5a (core world) agreed the separate module and the
router in front of `CoreWorld`.

**Found.**
- Portal 2's sign (`vgui_screen info_panel`, `sp_progress_sign`) was a
  400 x 808 VGUI panel drawn into the world through
  `DrawPanelIn3DSpace`. Its "06/19" label comes from a 28-pixel bitmap font,
  magnified about 3 times at a close look.
- It went through the legacy UnlitGeneric path, with no emission term.
- Its flicker advanced in every `Paint`, so each view that painted the
  sign in a frame (portal views) advanced it again. Its area light read the
  previous paint's brightness.
- Retail paints the grime over the lit image as grey. In the dim flicker
  states that adds light: the sign cast 0.0322 with grime against 0.0266
  without.

**Installed.**
- `render.world-panel.v1` (`public/render/world_panel.h`, the one
  definition): the draw list, the resolution policy (`PixelsPerUnit`,
  `ChooseResolution`), coatings (`CompositeAt`, `ScatterField`,
  `EmissionAt`) and the cast light (`Tiles`, `TileRadiance`).
- `render.pass.panels` (layer 6): per panel and frame, it rasterizes the
  emissive quads (gamma, legacy blending) and the coatings (premultiplied
  linear, half float), and a 4-unit scatter grid. A compute kernel builds
  the emission (E (1 - A.a) + field A.rgb) and albedo chains in linear
  light. The panel draws as `PBRMetalRough` through `ResolveMesh`, with the
  albedo image as its base and the emission image as its emission (nearest
  mip, clamped), lit by the ambient cube at the panel.
- Composition: `CorePanels` (`IRenderCorePanels`) and `ForwardedSlots`, the
  router that sends panel tags (`0x90000000 | serial`) to the panels and
  everything else to `CoreWorld`.
- `vguimatsurface`: `VGuiWorldPanelRecorder001`. It records a panel's
  paint as quads and refuses lines, polygons, fades, circles, 3D paint and
  render targets by name. Text draws from twin fonts rasterized at the
  image's density, and layout keeps the font's own metrics. Glyph quads
  carry their coverage from the font pages' backing bits.
- Engine: `VEngineWorldPanels001` (`engine/render_core_panels.cpp`). The
  core takes panels in the views it draws the world in (r_core_world 1,
  the outermost back-buffer view), and `r_core_panels 0` opts out. A failure
  after the core takes a panel is fatal under `r_core_world_strict`. Also
  `r_core_panels_stats`, and `r_area_lights_report` now lists slotless
  lights.
- Client:
  - `C_VGuiScreen` records a lit screen once per frame, at the main view's
    resolution.
  - The core draws it in the core's views; other views (portal, monitor,
    reflection) keep the legacy 2D path, as the world does.
  - It publishes 2 x 4 tile area lights from that same list, each with the
    whole panel's reach.
  - `cl_world_panel_report` prints each frame's list, screen corners and
    tile lights, with and without the grime.
- The sign:
  - opts in (`DrawsAsEmissiveSurface`);
  - declares its dirt overlays coatings;
  - advances its flicker once per frame;
  - takes its light from the image. `EmissiveAreaLights_MaterialRadiance`
    (the board's mean) is deleted.

**Evidence** (2026-09-30, on HEAD `0308156f8` with this change; desktop
RADV; `build-rc-lab` and `build-p2`).

| Suite | Result |
| --- | --- |
| `render.lab.panel` | 44 of 44, 0 validation messages |
| `render.lab.panel.sensitivity` | 4 of 4: gamma-space mips, no scatter, coating ignored |
| `corpus.portal2.sign-panel.selftest` | 18 of 18 (8 seeded judge defects) |
| `corpus.portal2.sign-panel` | 12 of 12 |
| `render.composition`, `.capabilities` | 22, 14 (after adding the panels sources to their rows) |

Measured on the product:
- The close shot holds 1.35 texels per screen pixel. The label's edges span
  1.32 pixels on the core and 2.99 on the legacy 2D path.
- The 39-frame flicker burst spans a light range of 9.8x. Every frame's
  sign pixels equal 1.046 times the light it publishes that frame, plus
  0.0002, with worst excess -0.0013. The neighbour-frame control's excess
  is 0.069.
- The dirt is in all 39 lists at overlay alpha 63, 127 or 255, and it
  dims the cast light in every frame.
- All 8 tile lights are published with the frame's radiances.

In the lab:
- `PixelsPerUnit` is within 1 percent of a ray cast.
- A hard edge spans 0.84 pixels at the chosen resolution, and 2.46 at one
  texel per unit.
- The coated scatter matches `EmissionAt` to 0.2 percent.

**Regressions checked.** `corpus.portal.sign-light.legacy` and
`.core-world` on `sp_a1_intro6`:
- The first run cut the sign's far field. Each tile's own reach (100 to
  130 units) was shorter than the panel's (394), and the near receiver
  rose only 1.24 levels. Each tile now takes the whole panel's reach, and
  every casts-light check passes on both paths.
- Still failing:
  - `sign-on`'s lit fraction. The legacy sign measures luma 85 against a
    threshold of 100 on 30 percent of the pixels, with identical pixels
    whether or not the panel is recorded (`r_area_lights 0` control). It
    measured 90 on 09-28, so the drift predates this change. On the core
    path the sign measures 71: the grime is now a translucent diffuser, not
    grey paint.
  - The recorded `core-draws-receivers` expected failure (R89).
- Portal's `testchmb` hue checks fail on the core-world path. Those signs
  are models this change does not reach.

**Not done.**
- Other `vgui_screen` panels do not opt in yet: elevator video screens,
  indicator panels and the co-op lobby screens.
- No monitor, portal or reflection views draw the core surface (K8 view
  generators).
- The recorder refuses lines and polygons instead of drawing them.
- No Fold7 or Apple run, and no frame-time record (rule 7: recorded when
  measured). The sign's images are 100 MB at 1600 x 3232 at the closest
  view.
- `sign-on`'s thresholds need recalibrating by the sign-light suite's
  owner.

## K12 performance, second round: profile and burn down (2026-09-30, source-engine-5a)

User direction: profile the CPU and GPU, then burn down without losing
quality, using the job system, better algorithms and the GPU where they
fit. Each item was found in a steady-state `perf` profile or with RFC 0014
D4's timers, and each has an oracle:

| Commit | What | Oracle |
| --- | --- | --- |
| `fd4e118e3` | A sparse publication relights only the static props whose ambient cube reads a touched probe | `r_indirect_sparse_verify`: 410 updates, 0 stale props |
| `0308156f8` | Probe visibility occlusion visits only probes within a proxy's reach | scan-all oracle, 40 random proxy sets; a zero reach is caught |
| `a233ee5bb` | The producer switch fades sparsely (only probes that differ); publications compose on the pool | every fade frame names every changed probe; a half list is caught; four threads equal serial |
| `a4c75aa0b` | GTAO at half resolution with a plane-aware upsample; presets set by `render.lab.gtao` against Cycles | `gtao.high-half` passes all 12 lab checks; the earlier 3x3 default failed them and was replaced |
| `b5057bd5e` | `r_core_ao_quality 0` binds a neutral occlusion (one), not black | in game: off against ultra 0.11% of pixels (> 8 levels) |
| `22e658988` | The SDF producer's previous field slot is not read back (84 MB per update on the render thread) | `render.indirect-light.sdf` |
| `7046c0a83` | Occlusion and the change atlas run on the pool | four threads equal serial |
| `edb03f6d2` | Native Vulkan `TexUnlock` uploads the locked rectangle, not the whole texture (Frozen-path, defect) | same view: 0.013% of pixels against a 0.005% noise floor |

In-game quality at the presets on sp_a2_laser_intro_relit: high against
ultra differs in 0.010% of pixels (> 8 levels).

Frame time (rule 7: recorded), offscreen (SDL's 1024x768), `mat_vsync 0`,
desktop RADV:

| Fixture | Morning | After round 1 | Now |
| --- | --- | --- | --- |
| `./play_p2 sp_a2_laser_intro_relit` median | 15.6 ms | 8.8 ms | 5.42 ms (183 fps), p99 6.2 |
| `./play testchmb_a_15_relit` median | 22.4 ms | 15.2 ms | 7.83 ms (128 fps), p95 15.0 |
| GI settle after a load (consume stalls) | 12.7 s | 7.9 s | 0.95 s |

Remaining, largest first:
- a_15's p95: consumes while its doors move;
- the lit pass (step 3: a depth prepass into the target's depth, MSAA
  included);
- the GTAO upsample blur;
- CPU clustering, to move to the GPU (K12 goal step 2).

Fold7: unavailable in this session.

### The plan's four steps, closed (2026-09-30)

The user's performance plan (D4 timers, shadows, the lit pass, GTAO) is
done:

- D4 is `8b0a40d6b`.
- Shadows are `ff2f83881`: lights the clusters reach, casters culled per
  tile, static tiles cached.
- GTAO at half resolution, proven in the lab, is `a4c75aa0b`.
- The lit pass's depth prepass is `e20440f16`. It is drawn into the
  target's own depth at its sample count, which removes the MSAA
  mismatch.

Later CPU items:
- `33010c15e`: view lights are clustered on the render sequence;
- `3016534ea`: light assignment builds each froxel's box once, with no
  sort.

testchmb_a_15_relit now runs at about 8.2 ms median with 3.5 ms of GPU.
sp_a2_laser_intro_relit runs at about 5.5 ms.

### Moving casters over the cached tiles (2026-09-30, source-engine-5a)

`5e9b11b8a` completes step 2 (Doom Eternal's cached shadow maps with
moving casters):
- The frame's moving-object boxes are drawn as cube casters into a
  frame atlas, over tiles restored from the cached static atlas
  (`AddTileCopy`).
- A tile is copied and drawn again only when its movers' signature
  (entity, part, pose version) or its static depth changes.
- `r_core_shadow_movers` turns it off.
- Oracle: `render.shadows.pixels` C3. The restore with movers drawn over
  it equals a full draw texel for texel, and the version without the
  restore is caught.
- Cost at rest on testchmb_a_15_relit: none. A first version that redrew
  every tile a mover reached every frame (234 of 255 tiles, +1.7 ms GPU)
  was replaced.
- No in-game image of a moving shadow yet: the test views have no mover
  under a shadowed light.
## Modern model and decal rendering slice (2026-09-30, active)

The user chose Source 2 quality as the primary path and DXVK for exact legacy
appearance. This slice is owned by `render.material` and `render.lab`: map the
supported `VertexLitGeneric` mesh subset onto the surface program's PBR mesh
point, with probe-volume indirect light, clustered direct light and reflection
probes. Each non-neutral VMT setting must be mapped or refused by name. The
legacy `vertexlit` point remains a pixel oracle, not the new mesh default.
`render.pass.decals` is the next K8 boundary; no product handover is claimed
before a lab pixel oracle and a native pass exist.

The user chose static props as the first in-game model cohort (2026-09-30).
The resolver accepts `SurfaceModelVertex` for object-space meshes: one mesh
shares GPU geometry across instances and studio skins, while each draw has
its own transform and skin material selection. The modern mesh specialization
skips the legacy per-draw light loop; its direct light comes from the view's
clustered set. The supported `VertexLitGeneric` subset includes the
alpha-tested Portal 2 catwalk railing (`railing_bts`, reference 0.65). The
`render_lab` debug-view suite draws an imported `VertexLitGeneric` material
through this layout with a translated instance; `render.family.vertexlit`
passes 122 checks, `render.debug-views` 88, and `render.world.null` 29.

The product now loads static MDL/VVD/VTX meshes into `render.pass.world`,
queues the engine's visible static-prop list in its draw slot, and suppresses
each claimed prop's legacy draw only after the core accepts that slot. On
`sp_a2_laser_intro_relit`, the native Vulkan Portal 2 boot passes with 224
of 294 static placements claimed (245 eligible), 73 of 93 material records
claimed, and 3,344 static instances queued / 3,512 surface draws completed
at the `r_core_world_stats` sample with zero reported failures. The map has
four `hanging_walkway_32b`, two `hanging_stair_128`, and two
`hanging_walkway_end_cap` placements; all eight are in the claimed cohort.
Reproduce with `tools/quality/portal_boot.py --game portal2 --renderer
native-vulkan --headless --require-vulkan --map sp_a2_laser_intro_relit`
using the staged Portal 2 build and
`quality-results/relight/sp_a2_laser_intro_relit/content`. Evidence:
`quality-results/rendercore-model-game/laser-boot05/evidence.json`,
`quality-results/rendercore-model-game/laser-boot05/runtime/engine.log`,
and `quality-results/rendercore-model-game/laser-boot05.png`.

This is an in-game static-prop handoff, not K5 closure. The screenshot still
shows mixed model lighting: 70 static placements and all dynamic/skinned
models remain on the legacy path. The mesh point uses a neutral dielectric
MRAO constant for imported legacy model materials, and at this handoff the
map relight scene did not author model PBR textures or static-prop shadow
casters. Material authoring, model shadow integration, and the remaining
static-prop cohort are follow-up work. No performance comparison is claimed.

### Laser intro static-prop light transport and map-light merge (2026-09-30)

The relight scene now reads the BSP's `sprp` placements and studio skins,
exports LOD 0 through `mdl_mesh_export` (the same `content.studio-model`
parser as rendercore), and uses each opaque mesh's resolved VMT and Source
transform in the Cycles scene. Static props cast into the world lightmap,
probe volume and reflection-probe renders while receiving no world atlas
space and staying out of WMSH. The modern `VertexLitGeneric` default and
Phong roughness used by the baker agree with the runtime mesh point. The
laser intro scene contains 294 placements from 41 models: 296 opaque mesh
draws and 25 materials entered the bake, while 6 translucent/special mesh
draws were omitted. The 2048 preview atlas charted the 5,126 world
triangles and parked 213,728 prop/occluder triangles. Cycles completed its
direct, indirect and directional passes, and the preview package retained
gameplay identity. The preview uses 64 samples and no denoising; its seam
p99 was 0.1627, so its relaxed preview seam limit does not claim the
release-quality lightmap gate.

A same-seed, same-layout 64-sample local control removed only the static-prop
meshes and repeated the separated direct and indirect passes. The world
coverage mask was identical (2,905,659 texels); 41.2% of covered RGB texels
changed, and mean covered luminance fell from 0.01079 without props to
0.00793 with props. This is evidence that the props affect world light
transport, not just scene inventory. The paired EXRs, logs and hashes are
in `quality-results/rendercore-model-game/laser-no-prop-control/`.

The map's relit BSP retains one switchable world light. The previous
all-or-nothing map-light fallback therefore hid its authored baked lamps
from the model point. `render.pass.lights` now merges individual always-on
lamps by shape and position, keeps style-controlled lamps with the engine's
live set, and applies vrad's reference-distance scaling to explicit
constant/linear/quadratic attenuation. Nonzero
`_fifty_percent_distance` remains an explicit unsupported case. The
`render_lab` clustered-light suite passed 25 checks and its seeded run
passed 5 before product integration. On the preview map, native Vulkan
boot passed with 16 runtime lights in the stage view (previously 1), 224
of 294 static props claimed, 740 lit views and zero reported draw failures.
Evidence: `quality-results/rendercore-model-game/laser-prop-preview/`,
`quality-results/rendercore-model-game/laser-prop-preview-game2/evidence.json`,
and `quality-results/rendercore-model-game/laser-prop-preview-game2.png`.

The preview was then rebaked through `content` on a remote RTX 4080 SUPER
with the pinned Blender 5.2.2 profile. The bake receipt records OptiX on
that GPU, 39 charted world meshes and a 70.3-second bake; 48 reflection
probe faces and the probe volume also rendered remotely. The packaged map
passed gameplay identity, and a fresh native Vulkan boot passed with 744
lit views, 224 of 294 static props claimed, 16 runtime lights and no
reported draw failure. The remote host was released after the successful
pipeline. Evidence: `quality-results/rendercore-model-game/laser-prop-remote-run4.log`,
`quality-results/rendercore-model-game/laser-prop-preview/lighting/atlas.exr.json`,
`quality-results/rendercore-model-game/laser-prop-remote-game/evidence.json`,
and `quality-results/rendercore-model-game/laser-prop-remote-game.png`.

This is a quality preview, not K5 or K11 closure. The 70 unclaimed static
placements, model PBR authoring, static-mesh runtime shadow casters,
nonzero `_fifty_percent_distance`, and a denoised high-sample map remain.
The new host tool builds in the configured Portal 2 product; a relight
toolchain's `client_build` must contain it. No performance comparison is
claimed.

### Animated door and movie reference work (2026-09-30, active)

The WorldPass now accepts a queued model pose as owned world-space vertices,
draws it through the shared mesh materials and skin table, and retires its
transient vertex buffer behind the frame token. A model-only view no longer
requires BSP vertex/index buffers or a static instance to initialize the mesh
resolver. `render.world.null` passed 31 checks. The new `render_lab suite
posed-model --validate` passed 5 checks on Vulkan: the current pose moves lit
pixels and turning its area light off removes them. The fixture's lit, moved,
and unlit frames are under
`quality-results/rendercore-model-game/posed-model-lab/`.

The engine now registers `portal_door_combined.mdl` beside the static model
inventory and hands its live Studio bone palette to the core for eligible
top-level views. The core composes each bone-to-world matrix with the model's
pose-to-bone matrix, skins the owned vertices, and draws them at the Studio
slot through the same stage lighting as the static meshes. Other models and
unsupported door draws stay with studiorender. A material claim defect kept
the door out at first: the imported VertexLitGeneric has ordinary
self-illumination and the shader's inactive zero-valued Fresnel parameters.
The mesh claim now ignores those parameters while the Fresnel enable switch
remains unsupported and names a gap.

The native Vulkan Portal 2 close-door and open-door boots both passed on
`sp_a2_laser_intro_relit` with rendercore enabled. The open-door capture
recorded 110 posed models queued and 110 draws, 2,300 static props queued,
548 views drawn, and zero failed views. The door panels visibly move clear of
the doorway while the frame and panels receive stage shading. Evidence:
`quality-results/rendercore-model-game/laser-door-boot11.png`,
`quality-results/rendercore-model-game/laser-door-open-final.png`, and the
open boot's `evidence.json` and `runtime/engine.log`. The material suite passed
123 checks; `render.world.null` passed 31. This is the test chamber door model
cohort, not all animated Studio models or a completed K5 handoff. CPU skinning
cost and quality across other door poses/maps remain to measure.

The map's second instance, the exit door at `(576, 0, -40)`, also appears in a
native Vulkan capture (`laser-exit-door-final.png`); the boot reports 195
posed draws and zero failures. Its surrounding corridor is very dark in this
preview, and the door follows that environment. This check establishes the
handoff for both placed doors, not a final lighting grade for every map.

For the Blender lighting reference, the laser intro's `laser_portal.bik` frame
uses retail `video_splitter.nut` UV mode 12. The extracted USD scene records
each movie screen's world corner and `st` coordinate; the Blender reference
renderer checks those pairs after import and checks that the emission frame
samples `st`. The extracted laser scene passed for all 44 lifted screen quads;
a changed UV and a disconnected UV image node were each rejected. The 41
legacy relight and 16 USD scene tests passed. This is a reference-render
tooling check and has not changed the in-game video path or its on/off state.

### K12: static props cast into the core's runtime shadow atlas (2026-09-30)

The world stage's shadow caster mesh previously contained only WMSH triangles.
The core shaded claimed static props, but those props could not block a runtime
light on the world or on another model. At `SetStaticProps`, composition now
builds world-space caster triangles for the opaque props the world pass claims.
It groups each instance as a frustum-cullable chunk and advances the caster
generation, so cached shadow tiles redraw when the level's props change.
The authored `STATIC_PROP_NO_SHADOW` and Studio
`STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS` flags reach this decision. The depth
caster cannot sample a cutout texture, so alpha-tested surfaces are excluded
from its mesh until a cutout shadow program exists; a prop with opaque and
cutout surfaces contributes only its opaque triangles.

On the staged `sp_a2_laser_intro_relit` preview, 236 of 294 static props
were claimed for drawing. The core shadow mesh contains 226 opaque instances;
two claimed instances have authored no-shadow flags and eight have only
cutout surfaces. A native Vulkan Portal 2 boot with `r_core_world 1` drew
958 core views with zero failed or skipped views. Its build and run evidence
is in `quality-results/rendercore-model-game/static-casters-v1/` (notably
`evidence.json`, `stdout.log`, `runtime/engine.log` and the screenshot).
The earlier staged binary also booted under the same command as a control;
the capture comparison is limited by animated pixels and does not certify
shadow shape or the whole K12 image match.

The Portal 2 product build and `render.shadows.pixels` pass. The standalone
composition manifests were brought up to the model parser, skinning and
vertexlit source set; `render.composition` and `.capabilities` pass with
22 and 14 checks. Stylelint passes. `archlint check --changed` reports its
existing CAP002 occurrences with zero new and zero stale occurrences.

K12 remains active. Cutout shadow casting, unclaimed static props, other
animated models, nested views, the game/lab image comparison, product output
on Apple profiles, and deletion of the native backend's duplicate model
shading remain open. `r_core_world` remains opt-in.

### K12: alpha-tested PBR world surfaces enter the core model (2026-09-30)

`PBRMetalRough` with `$alphatest` and `$alphatestreference` was a named world
material gap even though the shared PBR surface program already discards
pixels below the cutoff. The PBR claim now carries the Source byte reference
and disables target alpha writes for the cutout variant. A GPU pixel check
holds the scene fixed: a base alpha of 160/255 disappears under the default
178/255 cutoff and keeps its lit RGB under a 127/255 cutoff. The other eight
recorded PBR port cases still pass within their existing tolerance.

The world stage's solid depth caster had included every WMSH triangle,
including alpha-tested faces. Until it can sample a cutout texture, it now
omits those triangles as the static-prop caster already does. On
`sp_a2_laser_intro_relit` this omits 14 triangles; cutout-shaped shadows are
still an open K7/K12 item. The core now claims 39 of 39 WMSH batches and 81
of 95 world material records (previously 38/39 and 80/95). A native Vulkan
Portal 2 boot drew 920 of 920 queued views with zero failed or skipped views.
Evidence: `quality-results/rendercore-model-game/pbr-cutout-v1/`.

`render.family.pbr` passed 89 checks, including the new two-threshold GPU
case; `render.shadows.pixels` passed 40 and `render.composition` 22. The Portal
2 product build, stylelint and `git diff --check` passed. `archlint
check --changed` still reports its existing CAP002 occurrences (zero new,
zero stale). This is one material cohort: the remaining 14 world material
records, unclaimed models, nested views, cutout-shaped shadows, game/lab
image comparison and duplicate native shading keep K12 open. The engine's
`r_core_world` default remains off.

### K12: precached Studio models use the core mesh point (2026-09-30)

The product pose bridge previously registered and recognized only
`portal_door_combined.mdl`. For a world stage with `r_core_world 1`, level
composition now registers the client's precached opaque Studio models beside
the static-prop meshes. It keeps the static models first so prop indices stay
stable. The client precache owns the model references; the core copies MDL,
VVD, VTX and materials at level load. The hard-coded door load and identity
check are gone. `r_core_world 0` keeps the legacy path's lazy model loading.

At a Studio draw, the core takes a registered model only in a top-level world
stage view, on LOD 0 and body 0, with no flex, forced material, two-pass,
translucent, wireframe, blink, static-lighting or stats mode and no per-entity
color or alpha modulation. A rejected draw remains with studiorender. The
engine's bone palette still supplies the pose; gameplay animation is not
reimplemented. `r_core_world_stats` now names every model actually claimed,
so merely parsing a precached asset cannot be mistaken for a product handover.

On `sp_a2_laser_intro_relit`, 48 candidate posed models came from 139 client
precache entries; all 89 static and candidate model inputs had MDL, VVD, VTX
and materials. The native Vulkan boot with the final guarded bridge queued
390 posed model views and drew 782 posed surfaces: 194 claims for
`portal_door_combined.mdl` and 196 for `elevator_b.mdl`. All 1,150 queued
core views drew, with zero failed or skipped. Evidence:
`quality-results/rendercore-model-game/posed-registry-final/`. The `r_core_world
0` control in `posed-registry-off-final/` scanned no precache entries, parsed
only the 41 static models and drew no core views; the Portal BSP control on
`testchmb_a_01` booted through the core world path with 200/200 views, but
that map has no world stage or posed-model claims. `render.world.null` passed
31 checks. Both Portal and Portal 2 product builds and stylelint passed;
archlint's unchanged CAP002 debt has zero new and zero stale occurrences.

This moves a second animated model cohort into the render model. Other Studio
models still need material terms, flex, bodygroups, LODs, entity modulation,
and nested-view support. The candidate registry is level-load only: turning
`r_core_world` on mid-level leaves posed models with studiorender until a
new level loads. Model runtime shadow casters and the game/lab image match
also remain open; K12 and the `r_core_world` opt-in state do not close here.

### K12: native model materials and a checked-in claim inventory (2026-09-30)

The elevator tube exposed a material ownership error: its authored glass was
entering the core as an opaque Studio surface, and its `$envmap` could draw a
legacy cubemap. Studio registration and drawing now carry an opaque or blended
surface phase. The tube glass is in the blended phase, samples the core's RPRB
image lighting, and does not import a legacy cubemap for a native mesh. The
small inner elevator platform (`elevator_main_a/b/c`) is opaque and self lit;
the shared PBR program replaces its masked lighting with emission rather than
adding emission over direct lighting. An unlit model mesh also uses this same
surface program in an emissive mode, with native image lighting when its VMT
authors `$envmap`.

`render.material` now maps more VertexLitGeneric model terms into the shared
PBR program: authored Phong and half Lambert, rim, Phong exponent and masks,
self illumination and its separate mask, detail and light warp textures,
base/normal alpha probe masks, probe Fresnel and color controls, and related
blend tints. The `render_lab` posed-model suite covers the blended glass,
platform emission, unlit model and the material terms with native Vulkan
pixels. Its 29 checks pass; the map-terms suite passes 24 checks.

The checked-in [Portal 2 model-material inventory](../quality/materials/portal2-model-claims.json)
contains every `materials/models/**/*.vmt` from the ordered content path, its
resolved input hash, shader, proxies, native-probe claim and gap. The auditor
(`tools/render/material_claim_inventory.py`) feeds each material into
`render_lab claim-batch`, which calls the actual `MapVariables` and
`ClaimForMesh` rules. `--verify` checks the whole report, and the
`render.material.model-claim-inventory` Q-CONTENT suite requires all 1,165
entries. Currently 970/1,165 (83.26%) are statically claimed. This is claim
coverage, not a pixel or reachability result. The largest gaps include 49
proxy materials, 24 `$treesway`, 20 Refract, 11 `$bumpscale`, and 9 `$ssbump`
materials; the inventory retains each specific material and reason.

In a native Vulkan Portal 2 boot on `sp_a2_laser_intro_relit`, the product
claimed 162/177 staged material records (91.5%), including the tube and floor
cohort, and drew 1,012/1,012 queued views with no failures or skips. The 15
unclaimed records are 4 Refract_DX90, 2 WriteZ_DX9, 4 malformed `.2` keys,
2 `/*` keys, and one each of `$envampsaturation`, `$phongwarptexture` and
`$ssbump`. Evidence:
`quality-results/rendercore-model-game/material-inventory-v4/`. The malformed
keys remain explicit gaps because silently accepting them would hide an input
fidelity problem. This cohort does not close K12: the catalog's remaining
material families, native image comparison and other render paths remain open;
`r_core_world` stays opt-in.

### K12: SSBump model normals enter the shared PBR point (2026-09-30)

The PBR mesh point now decodes `$ssbump` as weights in the same three-vector
basis already owned by the core's world surface program. It keeps the normal
texture in the data color space and normalizes the resulting tangent-space
direction before lighting. The model claim selects `kSurfaceSsbump` in place
of the ordinary normal-map variant, requires a bound bump texture, and reads
`$ssbumpmathfix` without applying the legacy diffuse-page correction to PBR
lighting. `render_lab` stages a one-texel normal map and draws both variants;
its Vulkan pixel check detects the changed light response. The 32-check
posed-model suite and the 124-check `render.family.vertexlit` conformance
suite pass.

The VertexLitGeneric shader sources declare no `$bumpscale` control. The
importer now classifies that key as family-specific metadata: the eleven
affected VMTs use their normal texture's authored texels in the shared PBR
point. The checked-in inventory and its 1,165-entry verifier now claim
989/1,165 model VMTs (84.89%), up from 970/1,165. Eight of the nine
`$ssbump` catalog gaps close; the ninth exposes a malformed `0.4` key and
stays explicit. The eleven `$bumpscale` gaps close.

A native Vulkan Portal 2 boot on `sp_a2_laser_intro_relit` claims 163/177
staged materials (previously 162/177), including that scene's SSBump model,
and draws 811/811 queued views with no failures or skips. Evidence:
`quality-results/rendercore-model-game/material-inventory-v5/` and
`quality-results/rendercore-model-game/ssbump-vertexlit-conformance-v2.json`.
The product boot predates the importer-only `$bumpscale` classification; the
subsequent Portal 2 product build passes. The catalog, remaining scene gaps,
view handover, native-backend shader deletion, and game/lab frame comparison
remain open under K12. `r_core_world` stays opt-in.

### K12: unquoted VMT numeric keys stay visible in the model inventory (2026-09-30)

Four staged VertexLitGeneric materials carry unquoted values such as
`$envmaptint .2 .2 .2`. Source KeyValues reads the first scalar for the
parameter and presents the remaining scalar tokens as numeric keys. No shader
can name those keys. The VMT importer now recognizes only complete finite
numeric keys as diagnosed parser metadata; it still refuses an unknown named
render control. `render_lab` checks both cases. The claim-batch audit exports
each numeric key into `numeric_keyvalues_residue` on the affected material's
checked-in record, so the classification is reviewable per VMT. The two
materials whose block comments mispair shader parameters remain named gaps.
The 34-check Vulkan posed-model suite and 79-check `render.material.v2`
conformance suite pass.

The Portal 2 model catalog now claims 1,000/1,165 VMTs (85.84%); the
1,165-entry inventory verification passes. A native Vulkan product boot on
`sp_a2_laser_intro_relit` claims 167/177 staged material records, up from
163/177, and draws 1,336/1,336 queued views without failure or skip.
Evidence: `quality-results/rendercore-model-game/material-inventory-v6/`.
The remaining scene gaps are four Refract_DX90, two WriteZ_DX9, two malformed
block-comment materials, one `$envampsaturation` typo and one authored
`$phongwarptexture`. K12 still needs these surfaces, the other catalog and
view cohorts, one copy of the shader math, and game/lab frame comparison;
`r_core_world` stays opt-in.

### K12: staged material names and Phong warp on the shared PBR point (2026-09-30)

The diagnostic product boot at
`quality-results/rendercore-model-game/material-gap-names/` names every
unclaimed material on `sp_a2_laser_intro_relit`. Three
`glass/glasswindow_refract01*` materials and
`models/props_destruction/glass_fracture_b_normal` need scene-color
transmission with authored normal maps, refraction amount, tint and blur.
The two WriteZ materials are `models/portals/portal_1_anims` and
`portal_2_anims`; they belong to the portal/depth handover.
`models/props/ball_catcher_sheet` and `combine_ball_launcher` have block
comments that KeyValues mispairs, and
`models/anim_wp/arm_interior_192/arm_glasstop` has an undeclared
`$envampsaturation` typo. The temporary diagnostic print was removed
after the capture.

`models/props/reflecto_cube_glass` authored `$phongwarptexture`. The
shared PBR mesh point now colors each direct specular contribution by the
legacy Phong highlight and Fresnel-range lookup while preserving native
IBL. It uses the model's existing data-texture slot, and a material that
authors both Phong warp and light warp is refused because they require
distinct slots. `render_lab` stages a colored warp texture: its 37-check
posed-model suite detects the direct specular color change and the
conflicting-slot rejection. `render.family.vertexlit` passes 124 checks.

A native Vulkan product boot at
`quality-results/rendercore-model-game/phong-warp-v1/` draws 1,347/1,347
queued views without failure. The cube gets past Phong warp but still has
`$cloakfactor 1`; it remains unclaimed until the core owns that
transmission pass. The staged claim remains 167/177. The static model
inventory remains 1,000/1,165 because the authored VMT also has
`$multipass` and other controls that its raw static claim refuses. No
material was counted as handed over merely because the new term rendered
in isolation. The inventory now stores every importer `unmapped_keys` entry
per material, beyond its first claim failure. The cube's record names
`$multipass`, `$refractamount`, `$cloakfactor`, `$cloaktint`,
`$envmapconstrast`, and `$forcephong`; the new Phong warp key is absent from
that list. K12 still needs the transmission cohort, portal depth
cohort, other materials and views, native shader-copy deletion and the
game/lab image match; `r_core_world` stays opt-in.

### K12: full Portal 2 material claims and authored mesh probe masks (2026-09-30)

The [full VMT claim inventory](../quality/materials/portal2-all-claims.json)
now audits all 3,738 resolved Portal 2 materials in content search order. It
calls `render.material`'s `ClaimForDrawing` with world PBR enabled and
`ClaimForMesh` with native RPRB, retaining each material's shader, input hash,
proxies, both claim results, all unmapped keys, and per-shader counts. A model
path requires the mesh claim; another path can take either surface point.
The `render.material.all-claim-inventory` conformance row verifies the entire
checked-in report. At this slice the static union claimed 1,836/3,738 (49.12%).
Among the most numerous unclaimed cohorts are `$ignorez` (302), `$decal`
(273), material proxies (173), SpriteCard (143 materials), and Refract
(37 materials). The report counts non-surface shaders too; its percentage is
a catalog measure, not evidence that all those shaders should become PBR
surfaces or that any individual VMT appears in a tested view.

The mesh point now samples an authored RGB `$envmapmask` as a weight on its
native RPRB image specular. It binds the mask through the shared surface
program's existing data-texture slot and refuses a material that also needs
that slot for `$phongexponenttexture`. It also refuses `$envmapmask` with
`$bumpmap`, where the legacy VertexLitGeneric initialization disables the
envmap instead of combining the two. The lab's RPRB pixel oracle verifies the
three RGB weights; its posed-model suite verifies the claim and both refused
combinations. Both suites pass with Vulkan validation: 25 and 40 checks. The
model inventory rises from 1,000 to 1,006 of 1,165 (86.35%); the full
inventory rises from 1,829 to 1,836. This handover uses no legacy cubemap
texture. The native Vulkan product boot at
`quality-results/rendercore-model-game/probe-mask-v1/` passes with 1,359/1,359
views drawn, zero failed or skipped, and 167/177 staged materials claimed;
this map does not stage one of the six newly claimed model VMTs. The scene's
Refract and cloak materials, portal depth materials,
other catalog gaps, duplicate native shading, and game/lab image comparison
remain open under K12; `r_core_world` stays opt-in.

### K12: authored model `$color2` uses the shared PBR albedo (2026-09-30)

Source's BaseShader multiplies `$color` by `$color2` for the material color.
The native VertexLit mesh claim now packs that product into the shared PBR
program's albedo tint, with finite, nonnegative validation. The posed-model
Vulkan suite compares tinted pixels with the neutral material and passes
42/42 checks. Five more Portal 2 model VMTs claim the mesh point: the model
inventory is 1,011/1,165 (86.78%), and the full catalog is 1,843/3,738
(49.30%). Both checked inventories retain the exact input hashes and gaps.
The native Vulkan product boot at
`quality-results/rendercore-model-game/color2-v1/` passes with 1,358/1,358
views drawn, zero failed or skipped, and 167/177 staged materials claimed;
this map does not stage the five `$color2` model VMTs.
The Refract and cloak cohort remains the next appearance dependency for the
staged Portal 2 scene.

### K12: graph-owned scene-color capture boundary (2026-09-30)

The four staged Refract VMTs require a scene-color transmission input after
opaque shading. `render.graph` now offers `CaptureSceneColor`: it declares a
device-local image-to-buffer copy followed by a buffer-to-image copy into a
sampled transient, with graph transitions and lifetime owned by the frame.
The G11 oracle clears an opaque scene and checks every captured pixel on the
null and Vulkan adapters. Invalid sources are rejected without adding passes.
For 2x, 4x or 8x MSAA, the graph now declares a load-and-resolve render pass
before the copy. The Vulkan lane proves all 16 pixels of a 4x source survive
the resolve and capture; it caught and corrected a multisampled snapshot
allocation. The null lane checks the resolve declaration and single-sample
pixels. The native backend's core target import now declares copy-source only
where the host image was created for it (always for the MSAA color, and only
for a capturable swapchain image). This change is core plumbing in the frozen
backend; no legacy shading logic changed. A native Portal 2 boot at
`quality-results/rendercore-model-game/scene-color-import-v1/` passes with
1,155/1,155 views drawn, zero failed or skipped, and 167/177 staged material
records claimed. The product slot has not yet called the graph capture, and
K12 still needs a separate transmission pass, the shared PBR transmission
term and its authored-parameter oracle before a Refract VMT can claim. No
material count changes from this infrastructure slice; `r_core_world` remains
opt-in.

### K11: visible-emitter gallery diagnostic (2026-09-30)

The receiver metric excluded `LightQuad*` and `LightDisk*` pixels, so a view
could pass while its visible light meshes were dark. The gallery now places a
separate emitter error map beside each receiver error map. It reports mean and
p99 error scaled by the reference emitter luminance, plus the mean error of a
black-emitter negative control. The comparison gate and its fixed receiver
tolerances have not changed; emitter figures are diagnostic until their own
tolerances are fixed. The fixture tests show an exact emitter scores zero and
a black one matches the negative control. The checked-in Cycles references
have visible emitter pixels in 27 views; none has a black emitter reference.

`python3 tools/quality/lighting_fixtures.py gallery --fixture area-room --out
quality-results/lighting-gallery/emitter-diagnostic-20260930` ran against the
local `build-rc-lab` and wrote `index.html` and `summary.json`. Its three
camera results are:

| Camera | Receiver mean / p99 | Receiver gate | Emitter mean / p99 | Black-emitter mean |
| --- | ---: | --- | ---: | ---: |
| grazing | 0.071 / 0.857 | fail | 0.983 / 1.916 | 1.196 |
| overview | 0.056 / 0.798 | fail | 1.018 / 2.112 | 1.253 |
| wall | 0.027 / 0.156 | pass | 0.947 / 1.056 | 1.064 |

The wall camera is a negative example for relying on receiver scores alone.
This run does not raise the prior 9/35 gallery result or complete K11's visible
emitter term. The emitter material/radiance path and its fixed acceptance
tolerances still need work. The fixture and gallery tests pass. A stale
`BakeOverrides.test_final_drops_preview_samples` assertion was updated to
match `final_overrides`' documented omission of disabled preview settings in
favor of the profile defaults. `git diff --check` passes.

A full 2x gallery first rendered 19 of the current 39 fixture views. Eighteen
stopped with `no pbr draw group`, and two Portal views stopped at an alpha-test
material. The PBR failure was a lab call mismatch: model meshes supplied three
empty placeholder texture names, while the resolver's mesh draw layout has
zero texture inputs. The lab now requests the empty draw group for model meshes.
`WAFLOCK=.lock-waf-rc-lab-main ./waf build --target=render_lab` passed in the
existing configured tree. The rerun at
`quality-results/lighting-gallery/emitter-full-fixed-20260930/` renders all
39 views, with 10 receiver passes. This is a changed fixture set, including
the portal pair, so 10/39 is not a like-for-like improvement over the earlier
9/35 record. The new portal-pair views have large receiver and emitter errors,
and the emitter figures still need fixture-specific acceptance tolerances.
The edited C++ line passes stylelint. `archlint check --changed` reports its
existing CAP002 occurrences with zero new and zero stale occurrences.

### K11: authored visible-emitter radiance and cone (2026-09-30)

USD WMSH now gives each `LightQuad*` or `LightDisk*` mesh its own material
identity. The content compiler writes a PBR emitter material from that light's
linear radiance, a black base and the existing white unlit fallback. It rejects
missing sidedness, invalid radiance or cone data, and duplicate names. The
core PBR program now applies authored one-sided emission and the same
inner/outer-cosine ramp and exponent used by the Cycles fixture. An emitter
without radiance omits the emission and cone parameters. The material contract
records these parameters; the legacy shader backend was not changed.

The 2x area-room gallery at
`quality-results/lighting-gallery/emitter-area-cone-final-20261001/` renders
all three views. Emitter mean errors for grazing, overview and wall are
0.032, 0.061 and 0.033, versus 0.983, 1.018 and 0.947 before this slice.
Their p99 errors are 0.379, 0.387 and 0.244. The actual
`--debug-term emission` wall capture has emitter mean error 1.060, near the
black-emitter analytic control's 1.064, compared with 0.033 with emission on.
This control verifies that the new term, rather than another light term,
draws the visible panels. Receiver results remain one of three passes; the
grazing and overview p99 errors are 1.552 and 0.771.

The HDR disk in material-sweep has a narrow authored cone. A no-cone render
at `quality-results/lighting-gallery/emitter-material-hdr-20261001/` makes
the grazing emitter mean error 884,026; the cone-enabled 2x gallery at
`quality-results/lighting-gallery/emitter-material-cone-final-20261001/`
reduces it to 0.377 and the front emitter mean error from 2.115 to 0.091.
Receiver mean errors fall from 0.548 to 0.275 front and 0.798 to 0.083
grazing, but neither view passes its full receiver tolerance. The no-cone
render is a negative control, not a baseline to certify. Emitter metrics
remain diagnostic while fixture-specific acceptance tolerances are unsettled.

The `render.family.pbr` conformance suite passes 93 checks. Its first run
found that the shared pixel-test neutral view lacked the fifth scene-color
binding added for K12; the fixture was corrected, then PBR, lightmapped
(198 checks) and vertexlit (124 checks) pass. The 53 Python fixture/gallery
tests pass, both rebuilt fixture checks report zero problems, and stylelint
and `git diff --check` pass. `archlint check --changed` still exits with its
existing CAP002 occurrences but reports zero new and zero stale. K11 stays
open for the remaining material, probe, shadow and reflection image errors;
these captures do not establish the complete lighting model.

### K11: Cycles normal-map oracle correction and full receiver review (2026-10-01)

The user spotted that the Cycles gallery did not show authored bump/normal
detail. Its K11 wrapper inherited RFC 0011's `normal_maps=False` policy,
which is correct for the smooth-normal indirect oracle but wrong for a
full-appearance comparison. The lighting wrapper now rebinds materials with
authored normal maps, verifies each normal-textured material has a connected
Cycles Principled Normal input, and records both `normal_maps: true` and the
material names. The shared GI renderer is unchanged, preserving the
smooth-normal policy used by its existing references.

Extracted scene receipts show normal textures in only the two Portal chamber
fixtures: 13 of 22 materials in `portal2-chamber`, and 3 of 44 in
`portal-chamber`. `material-sweep` has none, so its rough-gold discrepancy is
unrelated to this omission. All four affected views were rerendered locally
in Blender 5.2.2 at their original 2,048 samples and seed 20260929. Both
`lighting_fixtures.py check --fixture` runs pass with zero problems. The
normal-map-on receipts include the 13 and 3 validated material names. The
K11 full 2x gallery at
`quality-results/lighting-gallery/normals-on-full-20261001/` renders 39/39
views, with 10 receiver passes. `portal2-chamber/chamber` is 0.226 mean,
1.996 p99 (previously 0.224, 1.992); `spawn` is 0.376, 1.550 (previously
0.375, 1.555). `portal-chamber/room2` is 0.243, 1.441 and `vault` is 0.433,
2.062. The wall/floor grid and ceiling differences remain visible. This
corrects the reference material policy, but does not explain the dominant
receiver error in those chambers.

The visual review of every receiver image, including the 39 per-view error
descriptions and the gross Portal-pair/door-room/Portal-2 failures, is in
[the K11 gallery review](0016-gallery-review-2026-10-01.md). The fixed
receiver tolerances were not changed. The 53 fixture/gallery Python tests
and `git diff --check` pass; `lighting_fixtures.py check` passes the full
K11 fixture set. `gi_reference.py check` still fails on pre-existing renderer
digest mismatches and a `room-states` fixture digest mismatch: the shared GI
renderer is byte-for-byte identical to HEAD, while its recorded digest is
already different. This K11 correction did not rerender the separate GI
oracles. K11 remains open.

The follow-up [receiver term isolation](0016-gallery-review-2026-10-01.md#closed-door-term-isolation)
distinguishes the conspicuous `door-room/closed/b-door` failure from its
direct-shadow component. The default full gallery scores 1.007 / 4.839
mean / p99 and shows a bright wedge. Selecting the core direct path removes
that wedge but its total still fails at 0.555 / 0.889. All four door-room
direct-only views pass; the closed B-side scores 0.024 / 0.240, and regional
linear values closely match Cycles direct diffuse. Its excess total is mostly
the static baked indirect layer from the open-room bake, with additional
probe/IBL response on the moving door. A full-gallery `--core-direct`
diagnostic worsened several other fixtures, so it is not a general K11
remedy or a new gate default. On `material-sweep`, the rough gold's red
channel is low by about 30% at roughness 1.0 while glossy gold is too
bright. Turning off multiple-scattering compensation makes the rough gold
darker and worsens p99. Those diagnostics narrow the next lighting work;
neither failure has been marked resolved.
The focused lighting fixture and gallery suites still pass (50 and 3 tests),
as do the full K11 fixture check and `git diff --check`. A wider
`test_lighting_*.py` discovery runs 78 tests and exposes two existing
`test_lighting_back_end` errors: its recording fake does not write the
`legacy-scene/scene-receipt.json` now read by the unchanged
`pbrt_map_build.py` code. Both files match HEAD; this is recorded separately
from the K11 oracle rerender.

### K11: analytic emitter reflection visibility (2026-10-01)

The corrected Cycles full-appearance oracle has a specific emitter-ray policy:
the visible mesh of an analytically replaced USD light is camera-visible,
but `pbrt_blender.add_lamp` hides that mesh from glossy rays and supplies the
light with a Cycles lamp. The RPRB producer rendered each face with a camera,
so its old faces captured the light mesh as reflected radiance. The core then
added the analytic light's direct specular term as well. `map_scene` now owns
the analytic replacement classification; the probe-face renderer hides only
those meshes and records their names. Mesh-only emitters remain visible to
probe cameras. Per-emitter WMSH materials carry `$emissioncameraonly` through
the PBR schema and surface constant, so SSR can leave the probe result in
place when its ray hits a camera-only light pixel. This applies the same
visibility rule to the probe and screen-space reflection paths without
turning off the light or reflection effects.

A probe-only diagnostic improved `material-sweep/front` from 0.275 / 1.285
to 0.261 / 1.020 mean / p99, but regressed `mirror-corridor/low` from
0.076 to 0.209 mean: SSR replaced the corrected probe with a reflection
of the camera-only ceiling strip. The precise SSR hit marker restores the
mirror and improves the final 2x gallery to **16/39 receiver passes** at
`quality-results/lighting-gallery/analytic-emitter-reflections-full-20261001/`.
All three area-room views and both mirror-corridor views now pass; the
Cornell floor, Portal-pair closed A view and sun-colonnade yard also newly
pass. No earlier pass regressed. The mirror's low view is 0.063 / 0.198;
`area-room/grazing` is 0.049 / 0.607. Both material-sweep views still fail
(front 0.261 / 1.020, grazing 0.067 / 0.501), as do the dominant Portal
pair and chamber views. The [39-view receiver review](0016-gallery-review-2026-10-01.md)
records each error and the per-lobe gold and closed-door controls.

The lab's SSR reference now rejects camera-only hits; its Vulkan suite
passes **25 checks** under validation. The seeded shader that ignores the
marker fails on 13,564 hit decisions and 13,354 unchanged-pixel checks.
`render.family.pbr` passes 94 checks, and its ignored-normal-map seed is
rejected (94 checks). `lighting_fixtures.py check` passes with zero
problems; the focused fixture Python suite passes 50 tests;
`shader_toolchain.py check` passes 164, `shader_artifacts.py check` passes
1,400, and stylelint reports 20 files with zero failures. `archlint
check --changed` reports zero new and zero stale occurrences but exits on
the existing CAP002 findings. The independent K12 session's changes in
the same checkout cause `git diff --check` to report CRLF lines in Portal 2
game UI files; the K11 paths have no whitespace errors. The fixture maps
were rebuilt with the new probe/material policy; the reused Portal chamber
maps were not republished by the lighting fixture builder. K11 stays open
for rough metal and the remaining indirect, material, shadow, Portal and
reflection mismatches. The current SSR rule rejects the first hit on a
camera-only emitter and falls back to the probe; a future reflection source
that separates visible overlays can trace behind it.

### K11: full Cycles oracle rerender (2026-10-01)

On the user's request, all **39 K11 Cycles reference views** were rerendered
under the lighting wrapper's authored-normal policy. The recorded seed
20260929 and each fixture's prior quality setting were retained: 35 views
at 2,048 samples without denoising (`final`) and four `door-room` views at
512 samples with OpenImageDenoise (`denoised`). Blender 5.2.2 ran locally on
the CPU for Cornell and area-room and on the Radeon 8060S HIP device for the
remaining fixtures. Every state receipt now has `normal_maps: true`; only
`portal-chamber` and `portal2-chamber` contain normal-textured materials,
with 3 and 13 validated bindings respectively. A new fixture check rejects
a smooth-normal K11 receipt, and its negative test catches that regression.

`lighting_fixtures.py check` passes with zero problems; the focused fixture
and gallery suites pass 51 and 3 tests. To isolate the oracle change, the
[39-view gallery](../quality-results/lighting-gallery/oracles-rerendered-frozen-lab-20261001/index.html)
reuses the full-resolution lab frames from the preceding analytic-emitter
gallery and recomputes every comparison against the new reference hashes.
It remains **16/39 receiver passes**, with no pass/fail changes. The largest
mean-score change is `portal-chamber/room2`, 0.243445 to 0.243520; its p99
changes from 1.441005 to 1.445276. The other differences are smaller.
[The receiver review](0016-gallery-review-2026-10-01.md) now cites this
gallery and records the six rows whose three-decimal display changed.
The visible chamber grid/content mismatches, rough gold, indirect-door and
Portal-pair state errors remain open.

A rough-gold probe-placement control sampled the checked-in material-sweep
RPRB at Gold7's Cycles position and normal pass. Its five captures are all at
x <= 4.875 m while Gold7 averages x = 8.368 m. At roughness 1, the CPU
RPRB oracle reads mean radiance (0.246, 0.239, 0.256). Two single-probe
captures in free space near Gold7 raise that to (0.394, 0.357, 0.360) and
(0.403, 0.349, 0.344), respectively; the reference's estimated indirect
red contribution remains about 0.686. A 512-sample-per-face rerender at the
second position, with the 512-wide prefilter held fixed, gives
(0.404, 0.351, 0.345); the red change from 16 samples is only 0.0008.
Capture sample noise does not explain this remaining gap. Probe placement is part of the
material-sweep error, but changing placement alone does not explain the
remaining rough-metal response. This diagnostic did not alter the map,
shader, receiver tolerances or 16/39 gallery result; the exact regions and
reproduction artifacts are in the [receiver review](0016-gallery-review-2026-10-01.md#rough-metal-and-lobe-controls).

The user removed Portal-pair from the active Cycles receiver set on
2026-10-01: the joined-copy Blender scene cannot certify a runtime portal
view. The fixture remains diagnostic, with `cycles_receiver_oracle: false`,
and default render/gallery selection now excludes it. The [new 27-view
gallery](../quality-results/lighting-gallery/cycles-oracles-no-portal-pair-20261001/index.html)
passes 15/27 receiver views (the previous 16/39 included one Portal-pair
pass); no sweep view changed score. At Gold7, increasing the nearby probe's
GGX prefilter from 256 to 1024 samples lowers red radiance from 0.404 to
0.390, and another 512-sample capture 0.53 m in front of its surface reads
0.362. These controls exclude simple capture-sample noise or an ever-closer
single probe as the missing 0.695 `GlossInd` red response. They do not yet
distinguish the high-roughness material lobe from surface-local transport.

The Portal-pair gallery's 12 saved frames expose a separate state coverage
gap: each camera has one identical lab image hash across `closed`, `open`,
`open-glow` and `glow-only`, while its four Cycles hashes differ. The fixture
points every state at the same closed-state BSP and has no lab mover or state
map; `render_lab` has no portal-state or portal-view input. The 11 failing
Portal-pair rows cannot be reduced to a color or BRDF fix. The lab must
compose the portal view and transport/light state before those views are
like-for-like. The [receiver review](0016-gallery-review-2026-10-01.md#portal-pair)
records this hash evidence and the misleading closed A receiver pass.

### K12: lab and game select the same core scene terms (2026-10-01)

`render.material` now owns the surface program's scene-term selection in
`public/render/material/scene_terms.h`. `render_lab` and the product world pass
adapt their available lightmap layers, probe textures, reflections and view
occlusion to that one policy. Runtime direct light is selected only with the
indirect bake layer; its directional basis comes from that layer, while the
total layer supplies the basis when runtime direct light is off. A probe
bounce requires a probe volume. The game continues to supply a neutral change
atlas when no producer changes the probes; its previous term set is preserved.
No shader math or intended pixels changed in this selection-only slice, so it
adds no new frame-time measurement.

`render.material.v2` passes 82 checks, including the missing-indirect and
probe-bounce negative cases; `render.world.null` passes 31 checks. The native
Portal 2 product build and the standalone `render_lab` build pass. The lab
draws the area-room overview with 70 draws from the same program. A headless
native Vulkan product boot of `sp_a2_laser_intro_relit` passes: 39/39 world
batches, 167/177 staged materials, and 1,054/1,054 queued views drawn with zero
failures or skips. Evidence is in
`quality-results/rendercore-model-game/shared-scene-terms-v2-20261001/`.
The same native Vulkan product also draws `lt_area_room` from the lab fixture:
70/70 WMSH batches, 114/116 materials claimed, 205/205 queued views drawn,
and the stage reads all 64 authored area lights. A private negative control
sets the 64 emitter materials' `$emissionscale` from 5 to 0; both game boots
pass and their captures differ across the visible panels (mean absolute RGB
difference 13.3/255). Evidence is in
`quality-results/rendercore-model-game/area-room-on-clean-20261001/` and
`area-room-off-clean-v2-20261001/`; the same pixel result passes again after
removing diagnostic logging in `area-room-on-final-20261001/` and
`area-room-off-final-20261001/`. The fixture runtime mounts published
`custom/pbrt-lt_area_room` ahead of `--content-root`, which initially hid the
control VMT; these two runs use a private runtime with that custom mount
removed, and the parsed values were checked as 5 and 0 respectively.
Stylelint and `git diff --check` pass. `archlint inventory --verify` passes;
`check --all` and `baseline --verify` report two existing ARCH105 occurrences
in `game/shared/fstop/blob_networkbypass.*`, outside this slice. K12 remains
active: transmission, nested views, unclaimed materials and game/lab frame
comparison are still open. The engine's `r_core_world` default remains opt-in.

### K12: the first same-camera game/lab image check (2026-10-01)

`portal_boot.py --content-root` now stages its private map and materials in
`custom/portal-boot-content` and inserts that path first in the staged game's
`SearchPaths`. The published `custom/pbrt-lt_area_room` previously won over the
test overlay, so the emitter-off control still read the published value 5.
The original runtime and its content are unchanged. The 69 Portal boot tests
pass, including a mount-order fixture; the ordinary mounted Portal 2 runtime
now captures the private on and off materials correctly.

`tools/quality/game_lab_compare.py` checks one declared K12 profile:
`area-room/overview` at 512x384. It renders the lab from the same frozen
content snapshot staged into the game, verifies every staged asset hash, the
successful native Vulkan boot, capture hashes, unit tone-map scale and
top-level view-oracle camera, then converts the lab's linear PFM to display
RGB. This replaced an earlier comparison that read a newer fixture BSP than
the game's hardlink snapshot. The fixed limits are mean absolute error at
most 3/255, p99 at most 25/255 and at most 3% of pixels changing by more
than 8/255. The matching snapshot passes at 1.39/255 mean, 14/255 p99 and
1.75% changed. The negative control with all 64 emitter scales set to zero
fails at 8.79/255 mean, 225/255 p99 and 5.06% changed; its receipt names
exactly the 64 intentionally different VMTs. The game's captured camera
agrees with the fixture within 0.05 Source units and 0.05 degrees. Product,
lab frame and comparison receipt are in
`quality-results/rendercore-model-game/area-room-snapshot-v2-game-on/`;
the control is in `area-room-snapshot-v2-game-off/`. The Portal 2 product
and `render_lab` builds pass; the 73 focused comparator and boot tests pass.
This certifies one same-camera scene, not the other Portal 2 materials,
effects or nested views required to close K12.

The same procedure exposed a wider scene-composition gap on
`portal2-chamber/spawn`. The product camera exactly matches the fixture at
(-1104, 0, 64) Source units, pitch 2.2 degrees and FOV 90; the core takes
21/21 world batches, 94/101 staged materials and draws 2,283/2,283 queued
views with no failure. The normal product frame differs from `render_lab` by
9.89/255 mean display RGB, especially the ceiling panels and central portal
area. With `cl_render_debug_legacy 2`, the ceiling panels match the lab's
dark surfaces, but the portal area and player view still differ and the mean
error is 5.18/255. These are exploratory captures in
`quality-results/rendercore-model-game/portal2-chamber-game-spawn-20261001/`
and `portal2-chamber-core-only-spawn-20261001/`; neither is a K12 parity
pass. The next scene-composition work must identify and hand over those
legacy and nested-view draws without counting the world-batch claim as full
frame ownership.

### K12: GI door and matched game/lab map sweep (2026-10-01)

`tools/quality/game_lab_matrix.py` runs the lighting fixtures from a private
copy of each published map, its materials and models. Portal boot mounts that
copy first; `render_lab` reads the same files. The comparator requires the
staged asset hashes, native Vulkan boot, screenshot and view-oracle hashes,
unit tone-map scale, map, film size and fixture camera before scoring display
RGB. Only `area-room/overview` has declared pixel limits; other scores are
diagnostics. The matching build's Portal 2 client has sha256 `f502e168c9ef`
and its `render_lab` has sha256 `974c632f40cb` (full hashes in receipts).
The 20 captures below booted and had matching content and camera; the largest
camera difference was under 0.25 Source units and 0.25 degrees. The source
and binary hashes are recorded per boot. Matrix evidence is in
`quality-results/rendercore-model-game/lighting-matrix-current-20261001/`;
the corrected portal state and republished door materials are in
`portal-pair-no-live-portals-20261001/` and
`door-room-published-current-20261001/` alongside it. The effective
20-camera selection is `quality-results/rendercore-model-game/matched-matrix-20261001.json`:
20/20 boots, content sets and cameras match; its single declared image gate
passes. Other images retain diagnostic scores. A 22-view, three-column visual
review (game, lab, difference at 4x) is
`quality-results/rendercore-model-game/matched-gallery-20261001/index.html`.
The emitter-off negative control rechecked with the current lab executable
still fails: 64 intentionally changed VMT hashes, mean 8.79/255, p99 225
and 5.06% of pixels over 8; the unchanged on snapshot passes at 1.39/255.

| Baked-state map | Game/lab mean absolute RGB error, /255 by camera | Finding |
| --- | --- | --- |
| `lt_area_room` | grazing 1.80, overview 1.42, wall 0.74 | overview passes its declared 3/25/3% gate |
| `lt_cornell_floors` | floor 0.82, front 2.18 | front's p99 is 72: image edges and model need review |
| `lt_door_room` open | a-door 0.49, b-door 0.52 | both use the republished emitter VMTs |
| `lt_foggy_hall_fog` | nave 18.49, side 24.49 | game has no lab-equivalent fog scatter; Portal 2 logs both `env_volumetric_fog_*` types as unknown and the core does not compose `render.pass.volumetric` |
| `lt_material_sweep` | front 1.94, grazing 1.51 | p99 37 and 33, respectively |
| `lt_mirror_corridor` | down 1.19, low 2.47 | low has 10.35% of pixels differing by over 8; review reflection edges |
| `lt_portal_pair` closed | a-portal 0.81, b-floor 1.24, b-portal 0.63 | the game spawns active portals despite the closed bake; removing `PortalA` and `PortalB` for the capture aligned the scene and removed the false 2.8–8.0 mean differences |
| `lt_projector_cookie` | room 23.29, wall 68.37 | `MapLights` parses its projector, but the product world stage does not feed the projector/cookie to the core lighting pass; the game image is dark |
| `lt_sun_colonnade` | along 1.03, yard 1.49 | yard's p99 is 65; review silhouettes |

The separate GI fixture `gi_door` was captured in both open and closed
states with the same 30-file content snapshot, camera and builds
(`quality-results/rendercore-model-game/gi-door-full-current-20261001/`).
The game core claims all 16 world surfaces and draws 142/142 queued views
without failure. Its open frame matches the lab at mean 0.206/255, p99 1
and 0.19% of pixels over 8. `ent_fire Door Toggle` changes 7,511 game pixels.
The closed frame, with the lab's matching mover bounds, differs at mean
1.397/255, p99 38 and 3.92% over 8: the legacy `func_brush` door's fallback
shading is visibly darker than the lab's PBR mover. Core ownership of moving
brush surfaces remains an R96 caller cohort. Both product boots passed, all
content hashes matched and the camera was within 0.031 units and 0.034
degrees. The earlier GI oracle run forced `r_core_world 1` while requesting
the native backend's `mat_indirect_view`; it was an invalid comparison, not a
core GI failure. `gi_runtime.py` now forces the native path for that oracle
and rejects the incompatible override. Its native Portal 2 run passes the
Cycles door/open world and model regions: walls 0.0199 against 0.0194,
model 0.0163 against 0.0158
(`quality-results/rendercore-model-game/gi-door-indirect-native-control-20261001/`).

The other two gallery maps were booted as exploratory controls at
`quality-results/rendercore-model-game/portal-scenes-exploratory-20261001/`.
`testchmb_a_00_relit` forces tone-map scale 1.5 after the command requests
1, so the exact-image comparator refuses both views. `sp_gi_chamber_01`
has matched map/camera/content and means 20.51 and 10.16/255, but live
portals, player and legacy draws are absent from the lab scene. Neither is
an apples-to-apples image gate. The 122 focused Python tests, stylelint and
both product/lab builds pass. `archlint check --all` still reports two
unrelated `ARCH105` occurrences in `game/shared/fstop/blob_networkbypass.*`;
it is not a green architecture gate. R96 remains active: fog, projected
textures, moving brush surfaces, nested views and the remaining material
and edge differences need core ownership and a new matched capture.

## K9 runtime legacy-stream census on matched maps (2026-10-01)

The native Vulkan frame stats now record `legacy_stream_draws`: actual
`vkCmdDraw`/`vkCmdDrawIndexed` commands issued from its recorded stream after
the core's slots and suppression have been applied. `legacy_program_draws`
counts the subset that used a port in `shaders/legacy/`. This is core plumbing
in the frozen backend, with no shading change. `game_lab_matrix.py` records
both counts for the last 30 presented frames of each matched capture;
`--require-zero-legacy` fails a view unless all 30 stream counts are zero.
The seeded parser check rejects missing counters, and the product control
`cl_render_debug_legacy 2` recorded 0/30 legacy draws on `area-room/overview`
(`quality-results/rendercore-zero-stream-control-20261001/`). Its screenshot
was byte-identical to the normal stationary view: the remaining motion-blur
draw there had neutral motion, not a needed visual contribution in that
particular frame. The control is not a shipped way to remove the stream.

The first census (`quality-results/rendercore-zero-stream-allmaps-20261001/`)
completed 24 cameras on all 11 published lighting maps with exact staged
content and camera checks. Every camera failed the zero-draw condition.
Per-map maximum legacy-stream draws in the settled sample were: area-room 1,
cornell-floors 2, door-room 1, foggy-hall 1, material-sweep 1,
mirror-corridor 1, portal-chamber 48, portal-pair 2, portal2-chamber 34,
projector-cookie 1 and sun-colonnade 2. The Vulkan route log identifies the
common legacy program as `dev/motion_blur`; counts above one also include
native-backend draws. Portal 1's two chamber views still cannot be exact
image comparisons because the product forces tone-map scale 1.5. They do
have valid runtime draw counts. The Portal 2 chamber views are exploratory
because live portal/player composition is absent from the lab camera.

This run exposed a composition timing mistake in the comparator: it enabled
`r_core_world 1` after `+map`, so `LevelInitModels` could not register
precached models. The runner now writes `r_core_world 1` to the startup cfg,
executed before `+map`, and leaves the later camera setup alone. The
early-start sweep at
`quality-results/rendercore-zero-stream-preload-allmaps-20261001/` completed
18 cameras across eight maps. On cornell-floors, the persistent stream count
fell from 2 to 1 and `r_core_world_stats` reports 319 posed sphere draws by
the core, 0 failed. Its floor/front game-lab mean differences increased from
0.82/2.18 to 1.12/2.94 8-bit levels: the core model's lighting still needs
K12 image work. The floor's last-30-frame desktop intervals were median
16.645 ms before and 16.654 ms with early registration (maximum 17.656 and
17.399 ms). Fold7 measurement was unavailable in this session. The
early-start Portal 2 chamber boot did not yield a complete receipt before
the runner's 240-second process bound; projector-cookie and sun-colonnade
were not started in that sweep. A separate retry of those four views at
`quality-results/rendercore-zero-stream-preload-tail-fixed-20261001/`
produced no usable product image: three timed out during private runtime
staging after 600 seconds, and the fourth reported `ENOSPC` while copying
the retail runtime. The retry's matrix records all four failed cameras;
none is counted as a core or image result. `portal_boot.py` now clones writable
runtime files with Btrfs copy-on-write when available, retaining distinct
inodes and a normal-copy fallback; its isolation and fallback tests pass.
With that staging path, the corrected early-start rerun at
`quality-results/rendercore-zero-stream-preload-tail-clone-20261001/`
completed all four missing views: two projector-cookie and two
sun-colonnade cameras, each with matching content/camera and one persistent
legacy motion-blur draw. The former's mean differences remain 23.29 and
68.37/255, locating its projected-light mismatch in K12; the latter's are
1.17 and 1.49/255. The runner now accepts explicit boot/process bounds
and retains a named failed camera instead of aborting the whole matrix.
The Portal 2 chamber completed separately with the cloned runtime at
`quality-results/rendercore-zero-stream-preload-portal2-clone-20261001/`:
its chamber/spawn views have 26 maximum settled legacy-stream draws and
diagnostic game/lab means 21.44/10.71. Together the three early-start
matrices contain runtime censuses for all 11 maps and 24 cameras. The first
two Portal 1 chamber attempts lacked an image comparison: the product's
tone-map scale rose above 1, and a scripted camera controller held the view
away from the authored camera. The other 22 produced comparisons. Every one
of the 24 runtime censuses is nonzero.

The separate `gi_door` open/closed run at
`quality-results/rendercore-zero-stream-gi-door-20261001/` used the same
snapshot and reproduced the earlier screenshots byte for byte. Open has
2 legacy-stream draws per settled frame; closed has 3. The route log names
the common motion-blur port and `gi_door/wall` on the native PBR model path;
closing adds `gi_door_fallback/wall` on the legacy lightmapped brush path.
Thus the door adds one draw without being drawn by both owners. An attempted
early-start GI-door rerun stopped during runtime staging before a product
receipt (`quality-results/rendercore-zero-stream-gi-door-preload-20261001/`).
With private clone staging and core activation before `+map`, the corrected
open/closed run at
`quality-results/rendercore-zero-stream-gi-door-ao-fixed-20261001/report.json`
registers the probe sphere as a posed core model. The settled legacy-stream
census is 1 draw/frame open (motion blur) and 2 closed (motion blur and the
lightmapped brush door). Both states used the same 30 staged map, material
and model files and the fixture camera as the lab. The model first appeared
black: the model view group had omitted screen AO and supplied a 1x1 neutral
texture to a screen-coordinate fetch. Turning AO off restored the sphere,
isolating the occlusion input. The core now
binds the same full-size AO view input to models and world, and clamps a
neutral input fetch to its texture bounds. The open game/lab mean difference
fell from 1.53 to 0.20/255 (p99 1); closed fell from 2.73 to 1.39/255
(p99 38), matching the prior late-opt-in images within capture variance.
The product build and `render.world.null` (33 checks) pass. Independently,
the core's sparse GI upload now forwards the current probe atlas's rectangles
and full replacements, along with the existing change atlas and grid table;
W13 checks an accepted rectangle and rejects an out-of-volume update.

The full rerun at
`quality-results/rendercore-zero-stream-ao-fixed-allmaps-20261001/matrix.json`
used one early-core startup, staged content and camera oracle for all 11 maps
and 24 views. Its 22 non-Portal-1 views scored, while all 24 had valid settled
legacy-draw counts and failed the zero-stream condition. The recorded Portal 1
map has a `point_viewcontrol` that overrides `setpos`/`setang` during its intro.
The comparator now applies the game's recorded linear tone-map scale to the
lab image before display encoding (the shared surface program multiplies by
that same scale); it still refuses a camera mismatch. The fixture runner
disables that controller before placing the camera. A bounded rerun at
`quality-results/rendercore-zero-stream-portal1-controlled-20261001/matrix.json`
verified both cameras exactly and scored them against their lab views. The
room2/vault mean differences are 29.39/52.18 8-bit levels; their settled
legacy-stream counts are 42/34 draws per frame. These are diagnostic images:
the game's live portal and player composition is outside the lab camera.
Together the two matrices compare all 24 authored views to the corresponding
game capture with equal per-view content, camera and recorded output scale.
They do not pass the image-parity or zero-stream gates. The current product
and lab builds, 82 focused Python checks, `render.world.null` (33 checks),
shader artifact conformance (1400 checks) and changed-file stylelint pass.
An open GI-door boot after the final product rebuild at
`quality-results/rendercore-gi-door-final-open-20261001/evidence.json`
passed and reproduced the prior screenshot SHA-256 byte for byte; its last
30 frames still contain exactly one legacy motion-blur draw each.

`render.material.v2`'s `surface` program remains the one core surface model.
The zero-stream gate is open: K8 needs core post motion blur and nested
portal/view cohorts; K12 needs the remaining model lighting and brush mover
ownership, followed by the other K8 cohorts. No legacy effect was disabled
to obtain a zero count. The focused game/lab tests, product build, style
check and legacy-freeze scan passed. The current shared checkout's K9 static
ratchet fails independently: 495 first-party acquisition sites in 147 files
against its recorded 490, with growth in four files. The runtime census does
not certify K9 or a shipped zero-legacy profile.

## Relit Portal 2 lightboard map identity (2026-10-01)

The `sp_a2_laser_intro_relit` lightboard showed `00 / None` with no mapped
icons or dirt. Its `vgui_screen` still uses `sp_progress_sign`, but that panel
looked up the runtime map name literally in `sp_lightboard_icons.txt`; the
script has `sp_a2_laser_intro`, not the published `_relit` name. The relit BSP
preserves the original gameplay entities. The panel now tries the exact map
entry first, then the original map name only for an unmatched `_relit` suffix.

The client build and changed-file style check passed. Native Vulkan product
boots passed for the original map and relit BSP at the same front-facing camera
(`cmd setpos -500 30 -80; cmd setang 0 270 0`), with screenshots and boot
receipts in `quality-results/relit-panel-sp-a2-laser-intro-original-20261001/`
and `quality-results/relit-panel-sp-a2-laser-intro-front-20261001/`. Both
panels show chamber `01`, progress `01/22`, the dirt coating, and the same ten
icon slots. The original capture also contains its transient chapter title;
the map surroundings differ under the relight. This fixes the lightboard's
content lookup, without claiming R96 image parity or closing its render gate.

### K12: Refract model point and product scene-color capture (2026-10-01)

The shared surface program now draws a bounded `Refract_DX90` model point.
Its authored normal alpha scales the screen displacement and cube reflection;
`$refracttint`, `$refractamount`, the integer blur choice, silhouette fade and
named or native cube maps remain distinct from PBR thin transmission. A
`$basetexture` replaces the framebuffer source, as the shipped Refract shader
does for the Portal 2 window variants. Without a cube map, normal alpha also
drives the material's blend. A scene-color source is required for Refract
without `$basetexture`; an `env_cubemap` reference requires the stage's native
RPRB input. Unsupported shader controls remain explicit claim gaps.

The composition now supplies the world pass with a narrow scene-color capture
capability. `render.graph` resolves MSAA if needed, copies the current color
target and keeps the sampled snapshot through the host submission. The world
pass prepares the transmitting view groups between render sections. A Vulkan
posed-model oracle covers the captured background, authored tint and half-alpha
blend, an authored displacement across a scene-color edge, and refusal of a
target without a copy source; the suite passes 52/52.
The checked-in inventories verify at 1,011/1,165 model VMTs and 1,849/3,738
all Portal 2 VMTs. The full-catalog increase from 1,843 is six authored
`$basetexture` Refract window materials. These figures are static claims, not
pixel acceptance.

The native Vulkan `sp_a2_laser_intro_relit` product boot at
`quality-results/rendercore-model-game/refract-product-final/evidence.json`
passes with strict core-world recording: 39/39 WMSH batches, 171/177 staged
materials, 1,343/1,343 views drawn and no failed or skipped views. The three
staged window Refract materials account for the rise from 168 to 171. A
same-camera legacy boot is at
`quality-results/rendercore-model-game/refract-legacy-camera-v3/evidence.json`
(`setpos -1120 0 -170`, `setang 0 180 0`). The elevator assembly is visibly
brighter in the core capture; this does not establish the tube or small inner
platform's appearance parity. Core debug captures at
`quality-results/rendercore-model-game/refract-emission-debug/evidence.json`
and `quality-results/rendercore-model-game/refract-direct-debug/evidence.json`
show the upper tube cover dark in emission and bright in the direct-light
view, locating that mismatch in its direct-light/shadow path. The staged
`glass_fracture_B_normal` material
needs a product camera and draw that actually samples scene color. The six
remaining staged material gaps are two WriteZ, two VMT comment tokens, a cloak
control and an authored env-map saturation spelling. Other material families,
moving and nested views, product refraction across image edges, output-scale
and fog comparisons, and deletion of replaced native shading remain open.
The product and lab builds, inventory verification, and changed-file style
check pass. `archlint check --all` still reports the pre-existing
`CreateInterfaceFn` expansion in `game/shared/fstop/blob_networkbypass.*`;
the world pass no longer includes the graph directly. R96 remains active and
`r_core_world` stays opt-in.

### K12: VertexLitGeneric mask precedence and the arm glass top (2026-10-01)

`VertexLitGeneric::SHADER_INIT_PARAMS` clears `$envmapmask` when `$bumpmap`
is defined, and also clears `$envmap` unless `$normalmapalphaenvmapmask` is set.
The core's VMT block now applies this same precedence before asking for a
native reflection probe. The misspelled `$envampsaturation` on
`models/anim_wp/arm_interior_192/arm_glasstop.vmt` is recorded as inert
metadata: the legacy shader declares `$envmapsaturation`, not that spelling.
The original VMT variables remain in the import record.

The checked-in inventories now verify at 1,012/1,165 model VMTs and
1,850/3,738 complete Portal 2 VMTs. The newly claimed arm glass top is an
alpha model; three already claimed models also no longer ask for a probe that
the legacy shader disables. The `render_lab` posed-model suite passes 54/54,
including controls where `$normalmapalphaenvmapmask` preserves the probe and
`$basealphaenvmapmask` with a bump map removes it. The installed native Vulkan
product boot passes at
`quality-results/rendercore-model-game/envmask-product/evidence.json`: 39/39
WMSH batches, 172/177 staged materials and 1,351/1,351 views drawn, with no
failed or skipped views. The five remaining staged gaps are two WriteZ, two
literal VMT `/*` tokens and a cloak control. The static claim and boot do not
prove the elevator tube or small inner platform's appearance parity.

For the matched elevator camera, disabling only the projected-light term
leaves both regions unchanged. Disabling only the clustered-light term changes
the upper cover's mean RGB from about `(44, 42, 36)` to `(6, 5, 3)` in the
8-bit screenshot; the legacy cover is about `(1, 1, 1)`. The inner platform
changes from about `(86, 87, 81)` to `(68, 67, 62)`; legacy is about
`(71, 75, 73)`. The diagnostic boots are in
`quality-results/rendercore-model-game/elevator-projector-off/` and
`quality-results/rendercore-model-game/elevator-clustered-off/`. These
controls locate most of the difference in the clustered direct-light path;
they do not establish whether its light intensity, model shadowing or material
response is wrong. The next correction needs a light and shadow oracle for
the posed elevator assembly, followed by another matched product capture.

### Published Portal 2 map elevator movies (2026-10-01)

Portal 2's shipped `videos/video_splitter.nut` chooses an elevator clip by
`GetMapName()`. The four published `_relit` maps with movie-table entries had
no matching name, so their original clips were not selected. Portal 2 VScript
now exposes the original name for `_relit` and `_source2` maps; the actual BSP
name remains distinct. The relight reference-scene reader uses the original
entry too, while respecting an explicit published-map entry.

The Portal 2 server build and 44 legacy-relight tests pass. Native Vulkan
product boots pass for `sp_a1_intro4_relit`, `sp_a1_intro5_relit`,
`sp_a2_laser_intro_relit`, `sp_a2_triple_laser_relit`, and
`sp_a2_laser_intro_source2`. Their console logs select, respectively,
`exercises_horiz.bik`, `exercises_vert.bik`, `laser_portal.bik`,
`aperture_appear_vert.bik`, and `laser_portal.bik`; the video provider is
registered and no clip-open failure appears. Evidence is under
`quality-results/elevator-movies/<map>/evidence.json` (the relit laser
confirmation is `sp_a2_laser_intro_relit-confirmed`, and the source2
confirmation is `sp_a2_laser_intro_source2-confirmed`). A separate native
Vulkan boot at `quality-results/elevator-movies/sp_a1_intro4_relit-screen/`
places the camera in front of an arrival panel; its screenshot shows the
`exercises_horiz.bik` image on that panel. The other maps' boot screenshots
face the elevator interior and do not certify their screen pixels. Other
published relights have no authored entry in the shipped movie table.


## Profiling-led Forward+ and core-only shading (2026-10-01)

The user requested the four targets in `quality-results/perf1001/summary.md`,
then narrowed legacy retirement to "When rendercore is enabled, only rendercore
shaders for now" and specified "Forward+". The bounded implementation and its
measurements are in the [Forward+ evidence record](0016-perf-forward-plus-2026-10-01.md).
The legacy lightmap-cache target is superseded for these world-stage fixtures by
bypassing CPU runtime-light integration; retail BSP core surfaces and
compatibility-mode lighting retain their existing lightmaps.
No new retained lightmap cache or second light authority was added.

Frozen-path: core plumbing R96/R91 — the native queue rejects legacy shader
draws before conversion under the core's frame marker, and the engine's queued
world-stage lightmaps omit CPU runtime light in that mode. These are the user's
explicitly requested boundaries, not new shading behavior on the frozen backend.

R95/R96/R91 stay active/partial: the core-only product census names rejected
HUD, particle, glass, sky and model materials. This slice does not close their
migration or claim the old whole-frame appearance.


### R96/R91 intro4 material completion (2026-10-01, active)

User request: “Complete them until the relit sp a1 intro 4 map renders every
material through our Forward+ renderer.” This session owns this map's material
claims and remaining primitive submissions. `render.material` owns parameter
semantics and the shared surface program; `render.pass.world` owns their
geometry and draws; composition supplies the current stage lighting. The
concurrent area-light optimization session owns lighting assignment/LTC and
finishes its evidence; no second lighting implementation is introduced.

The early-core baseline in
`quality-results/intro4-forward-materials/before/evidence.json` passes the boot,
with no claimed-view failures. Its material gaps are seven foliage sway
materials, five UnlitTwoTexture materials, two WriteZ materials, one inactive
cloak control and one flashlight-shadow control. The rejected-draw census
also includes world glass, decals, sky, particles, viewmodel, video and HUD
submissions, plus postprocessing. Completing static claims alone will not
certify this request. New points require lab pixel/failure controls before
product handoff. R96/R91 remain open during this work.

#### Intro4 regression recovery (2026-10-01)

The user reprioritized this slice to restore playability after the dynamic
handoff reported unsupported HUD materials, lost a promised view slot, and
produced a white scene. The original all-material objective remains incomplete.
The dynamic handoff is now behind `r_core_dynamic_draws`, a non-archived cheat
variable whose default is **0**. The composition default is also off, and the
native producer checks that policy before copying mesh data. It must stay
explicitly opt-in until complete cohort, queued rendering, capture replay,
exposure, motion, resize and matched-image acceptance pass. The restored default
keeps the existing core world/model path; it does not certify the omitted cohorts.

The user explicitly permitted legacy HUD/crosshair rendering. Composition now
requests a top-level HUD stage slot for the normal core-only game view. The
native stream permits legacy UI after that boundary; diagnostic pixel views and
legacy-skip views do not request it. This is stage policy, with no material-name
exception. `sprites/hud/portal_crosshairs` therefore never enters the dynamic
world-material claimant in the normal HUD stage.

Two ordering/lifetime defects were reproduced with independent controls:

- World tickets are issued on the main thread, while dynamic tickets are issued
  later on the render sequence. Serial ticket order does not order their frames.
  Recording a dynamic ticket could discard an already queued future world view.
  The recorder now finds that exact ticket without dropping future world views.
- A captured stream can contain more than the old 64 retained view records.
  Replaying it then lost early slots. A CPU stream epoch now owns replay records,
  independently of GPU submission serials. Every accepted slot of the bounded
  stream stays replayable until the backend discards that stream. Composition
  keeps the corresponding view/light snapshot lookup for the same lifetime.

Unsupported dynamic material semantics are checked before publishing a slot;
refused draws enter a bounded reason census. A published draw that later fails
still increments claimed-view failures and remains fatal in strict mode. No
legacy draw takes over a published core claim.

The material-system owner now supplies shader-initialized neutral defaults to
both world and dynamic imports. The engine's duplicate neutral cache was removed.
The lab proves UnlitTwoTexture multiplication and independent texture transforms,
WriteZ depth behavior, inactive cloak controls, and forced Phong behavior, with
texture/unsupported-feature failure controls. These points do not close foliage
sway, postprocessing, full particles, viewmodel lighting or nested-view fidelity.
The additional WriteZ oracle reproduced an empty draw-input access in
`ProgramResolver::DrawGroup`. Its depth point now binds the shared neutral draw
group, and the lightmap branch checks that exactly one input exists before
indexing it. The Vulkan image test puts WriteZ both in front of and behind an
emissive surface: only the former occludes it, and neither writes scene color.
An authored alpha mask is refused rather than claiming a mask shader that this
depth point does not implement. The failed lab run/backtrace are retained in
`recovery-posed-depth.log` and `recovery-depth-debug.log`.

Evidence is under `quality-results/intro4-forward-materials/`:

| Check | Evidence | Result |
| --- | --- | --- |
| Product build | `recovery-product-final-build.log`, existing Portal 2 Waf profile | pass |
| Lab | `recovery-posed-depth-final.log`, posed-model suite, 65 checks | pass |
| Contracts | `recovery-contracts-final`, world 37, frame 28, composition 28 checks | pass, zero skips |
| Queue negative controls | `recovery-mutants-current/evidence.json`: restoring serial-order pruning fails W14; restoring the 64-record limit fails W15 | both detected |
| Stable queued and synchronous products | `recovery-stable-final-queued/evidence.json`, `recovery-stable-final-sync/evidence.json`, strict core, normal exposure, legacy HUD | both pass, no claimed-view failures |
| Experimental queued product | `recovery-experimental-final-queued/evidence.json`, explicit dynamic opt-in | pass, no claimed-view failures; whole-cohort acceptance remains open |
| Native resize and capture | `recovery-resize-final-wayland/boot/evidence.json`, private Wayland compositor, queued mode, 12 settled sizes plus 25 capture offsets | pass, 37 screenshots, no claimed-view failures |
| Runtime install | `recovery-runtime-final-install/manifest.json`, all 30 product hashes equal the tested private runtime; replaced libraries backed up and atomically renamed | coherent tested set installed |
| Installed runtime boot | `recovery-final-installed-queued/evidence.json`, no build overlay, strict queued mode | pass |
| Harness fixtures | `recovery-boot-fixtures.log`, 71 tests | pass |
| Architecture/style fixtures | `recovery-arch-fixtures.log`, 162 tests; `recovery-style-fixtures.log`, 38 tests | pass |
| Changed-line style | `recovery-style-final.log`, pinned clang-format 22.1.8 | pass |
| Architecture/inventory | `recovery-arch-final.log`, `recovery-baseline-final.log`, `recovery-inventory-final.log` | inventory passes; all/baseline retain the pre-existing two ARCH105 occurrences in `game/shared/fstop/blob_networkbypass.*` |

The initial resize attempt is retained as failed evidence: the harness wrote its
script under `portal/cfg` for a Portal 2 run, so the workload did not execute.
`portal_boot.py` now writes every chained script under the selected game's cfg
directory, with a Portal 2 regression fixture. The corrected native test above
executed and verified all 37 sizes. The boot runner leaves auto-exposure enabled;
its usual synchronous mode alone was insufficient evidence for the queued path.
The recovery captures are normally lit. They do not identify the precise cause
of the user's white screenshot or establish full scene fidelity.

Reproduce the restored queued path from the repository root (a new output
directory is required). The synchronous control changes only `mat_queue_mode`
to 0. The experimental check adds `--startup-command 'r_core_dynamic_draws 1'`;
it is additional evidence, not the playable default.

```sh
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite posed-model --validate
python3 tools/quality/conformance.py check --suite render.world.null \
  --suite render.frame.v1 --suite render.composition --out <contracts-evidence>
python3 tools/quality/portal_boot.py --runtime run/runtime-p2 --game portal2 \
  --renderer native-vulkan --headless --require-vulkan --map sp_a1_intro4_relit \
  --startup-command 'mat_queue_mode 2' --startup-command 'r_core_world 1' \
  --startup-command 'r_core_world_strict 1' --startup-command 'r_indirect_producer baked' \
  --startup-command 'r_core_runtime_direct 1' --console-command 'r_core_world_stats' \
  --capture-wait 180 --timeout 120 --out <new-boot-directory>
```

The native resize reproduction argv is in
`recovery-resize-final-wayland/command.json`; it uses the installed
`private_session.dbus_run_session` helper and a private headless mutter display.
Offscreen rendering alone cannot certify native window resize.

Frozen-path: defect fixes and core plumbing for the user's explicit playability
recovery and legacy HUD exception; no new shading is added to the frozen backend.
R95/R96/R91 remain active/partial. Full intro4 material completion, motion/nested
view image acceptance and non-Linux runtime evidence remain unverified.

### R96/R91 automatic model eligibility (2026-10-01, active)

User request: complete family and feature coverage so any model whose materials,
deformation and draw state are supported renders without a model-specific
exception. This chat (`Explain rendercore model allowlist`) owns Studio geometry
selection and deformation, with ownership coordination sent to `Complete
Forward+ material support`; its material/dynamic submission/HUD work is preserved.
That chat confirmed `SetStaticProps`/`PoseModel` and `engine/l_studio.cpp` were free
for this chat after its playability recovery completed.
The full objective includes the remaining built-in families and frame cohorts,
not just this first geometry slice. R96/R91 remain open until their full evidence
passes.

First slice: `content.studio-model` owns body-group arithmetic and imports each
submodel once; composition captures the selected surface indices beside the pose;
`render.pass.world` draws that immutable selection with its existing materials.
The importer, blank groups, inactive unsupported materials, selection changes
between queued draws and pixel coverage are proven before the product bridge
accepts nonzero bodies. No filename selection or new lighting owner is added.

Implemented body-group slice:

- `ParseModelBodyVariants` validates every alternative and retains body/base/count
  identity, including blanks. The existing selected-body reader uses the same
  selection owner. Runtime geometry import needs bones/weights, not first-frame
  animation or ANI files; live animation remains supplied by the engine's palette.
- Static and posed world draws capture an optional increasing set of surface
  indices. An empty set is an authored blank; a failed import stays invalid.
  Eligibility, drawing and static shadow casters use only active surfaces.
- The Studio bridge passes `pInfo.body` and removes the `body == 0` gate after
  lab proof. Remaining deformation, LOD, proxy, draw-state and view restrictions
  retain their existing feature checks. This is a geometry cohort, not R96/R91
  closure or complete shader-family coverage.

Evidence is retained under
`quality-results/automatic-model-eligibility/bodygroups/`:

| Check | Result |
| --- | --- |
| Release conformance (`release-final.json`) | 5 suites, no skips/failures: Studio import 119, first-frame pose 86, world/null 37, native Vulkan model selection 22, composition 40 |
| Real corpus (`corpus-final.json`) | 87 checks; all 199 Portal and 2,033 Portal 2 models import all alternatives without ANI; default geometry preserved bit-for-bit; all 13 nondefault alternatives on 5 Portal 2 models match the selected-body reader. Comparator negative controls cover nonfinite payloads, tangents, UVs, weights, indices and body identity |
| Native Vulkan lab | Independent left/right static and posed pixel footprints, blank bodies, invalid selections, inactive/active unsupported material cases, queued snapshots and capture replay; swapped-body control rejected; sync validation silent |
| Product build/install | Existing Portal 2 SDL3/native-Vulkan release profile: `engine`, `render_composition`, `launcher`, `hl2_launcher` and linked products build/install successfully; no profile reconfiguration |
| Installed queued Portal 2 boot (`portal2-queued-audit/evidence.json`) | Native GPU offscreen capture of `sp_a1_intro4_relit`; all 117 Studio meshes imported, 743 posed models queued and 1,260 posed draws recorded at the stats snapshot; zero core view failures; private sandbox audit unchanged |
| Static checks | Local changed-line style: 41 files, no failures; archlint inventory current; 162 archlint and 38 stylelint fixtures pass; `git diff --check` passes |
| Existing gate failures | Archlint all/baseline still report the two pre-existing ARCH105 `CreateInterfaceFn` occurrences in `game/shared/fstop/blob_networkbypass.cpp/.h`; branch style against `origin/master` encounters existing legacy formatting and invalid UTF-8 in `common/matchmaking/mm_helpers.h`. No baseline/legacy formatting updates made |

The first installed boot (`portal2-queued-installed`) passed render checks but
reported a shared player-config change. Its staged config had an independent
inode and a separate game was running in `run/runtime-p2`; the cause is not proved
by the audit. That attempt is retained, and the repeated boot has no protected
config changes. The initial `--build build-rc-model-p2` boot refused staging
because that tree contains both build and installed launcher copies; the passing
boots select the installed package. An initial product build also selected the
lab's Waf lock and did not build the engine; only the explicit product-lock build
counts as product evidence.

Reproduce the product and contract checks from the repository root:

```sh
WAFLOCK=.lock-waf-rc-model-p2 ./waf build --targets=engine,render_composition,launcher,hl2_launcher -j 6
WAFLOCK=.lock-waf-rc-model-p2 ./waf install --targets=launcher,hl2_launcher -j 6
python3 tools/quality/conformance.py check --suite content.studio-model \
  --suite content.studio-model.pose --suite render.world.null \
  --suite render.composition --suite render.lab.model-selection --config release \
  --build-dir build-model-eligibility-release --out <contracts-evidence>
STUDIO_MODEL_CORPUS_VPKS=/home/john/src/source-engine/run/runtime/portal/portal_pak_dir.vpk,/home/john/src/source-engine/run/runtime-p2/portal2/pak01_dir.vpk \
  python3 tools/quality/conformance.py check --suite content.studio-model.corpus \
  --build-dir build-model-eligibility-conformance --out <corpus-evidence>
python3 tools/quality/portal_boot.py --runtime run/runtime-p2 \
  --build build-rc-model-p2/install --game portal2 --renderer native-vulkan \
  --headless --require-vulkan --map sp_a1_intro4_relit \
  --startup-command 'mat_queue_mode 2' --startup-command 'r_core_world 1' \
  --startup-command 'r_core_world_strict 1' --startup-command 'r_indirect_producer baked' \
  --startup-command 'r_core_runtime_direct 1' --console-command 'r_core_world_stats' \
  --capture-wait 180 --timeout 120 --out <new-boot-directory>
```

Frozen-path: core plumbing in the engine's Studio handoff for body-group
selection; no new legacy shading. Eligible body selection now uses the core
owner; retained legacy cohorts still cover the unimplemented features.

Still open: remaining built-in material families and their variables/proxies,
flex deformation, draw modulation/overrides/static lighting,
nested views, cutout/transmission shadows and the R91 effects/UI/post/sky/glass/
water/portal/monitor cohorts. The installed boot is an integration regression
check, not full scene parity, motion/resize acceptance or non-Linux evidence.
The full automatic-eligibility goal remains active.

### R96 automatic model eligibility: LOD geometry and replacement materials (2026-10-01)

`content.studio-model` now imports every body alternative and VTX LOD, with
slot-specific material replacements. The default selected reader retains LOD 0
behavior, and may select another LOD explicitly. Runtime import retains all
geometry without requiring first-frame animation data. Blank bodies/LODs remain
valid; malformed variants fail the read.

The shared core owns geometry selection and drawing. Composition resolves the
host's LOD-major material descriptors and captures the active surfaces per view.
The world pass snapshots static selections as well as posed selections, so
subsequent changes and capture replay cannot change an earlier draw. Eligibility
depends on the selected geometry and materials, including replacements, with no
model-name exceptions. Only bones used by the selected topology are read from
the host palette.

The lab supplies controlled model bytes and choices to this core path; the game
adapter supplies the real bytes, materials and Studio's existing LOD decision.
There is no second game shader or deformation implementation. Static props have
two existing Studio callers: the array pipeline and `DrawModelExStaticProp`.
Both hand off after their LOD calculation and share modulation/override eligibility
checks. The old pre-LOD static-prop claim is removed. Static shadow casters retain
the highest-detail geometry, independent of a camera's visible LOD.

Evidence under `quality-results/automatic-model-eligibility/lods/`:

| Check | Result |
| --- | --- |
| Release conformance (`release-final.json`) | 4 suites pass without skips: Studio import 151, world/null 37, composition 53, native Vulkan model selection 37 |
| Import corpus (`corpus.json`) | 87 checks pass; all 199 Portal and 2,033 Portal 2 models import all body/LOD variants and replacement lists; default geometry remains unchanged |
| Native Vulkan model selection | Posed and static high/low/blank LOD footprints, replacement colors, body/LOD composition, queued snapshots and replay; wrong-LOD and invalid-selection controls detected; synchronization validation silent |
| Installed product build (`install-product-final.log`) | Existing Portal 2 profile builds/installs launcher, executable, engine and Vulkan backend with their core dependencies; no reconfiguration |
| Queued Portal 2 with forced LOD 1 (`portal2-static-lod1/evidence.json`) | Native GPU capture of `sp_a1_intro4_relit` passes; 117/117 meshes imported, 7,495 static instances queued / 7,847 static draws, 704 posed instances queued / 1,191 posed draws; zero view failures, protected config unchanged |
| Queued Portal 2 with automatic LOD (`portal2-static-auto/evidence.json`) | Native GPU capture passes; 7,449 static instances queued / 7,779 static draws, 705 posed instances queued / 1,187 posed draws; zero view failures, protected config unchanged |
| Static checks | Changed LOD code passes pinned style; 162 archlint and 38 stylelint fixtures pass; inventory current; `git diff --check` passes |

The first static integration boot (`portal2-static-selected`) passed the generic
boot checks but queued zero static props: this profile uses the individual Studio
entry point. That is retained as failed LOD integration evidence, and prompted
the second caller's handoff. A later partial installation
(`portal2-static-final`) mixed an old launcher with the concurrently expanded
`CorePassTarget` layout and crashed in shadow creation. Relinking all product
consumers together fixes that mismatch; the passing LOD-1 run uses that package.
An all-target install (`install-coherent.log`) also encountered an unrelated
concurrent Hammer `DecodeVtf` link failure; the named product targets pass.
Archlint all/baseline retain the two pre-existing ARCH105 findings in
`game/shared/fstop/blob_networkbypass.cpp/.h`. Earlier style snapshots include
concurrent Hammer/VTF and view-state edits; the final changed-line check
(`style-last.log`) passes 35 files. No baselines are weakened.

Reproduce the product slice with the existing product lock:

```sh
WAFLOCK=.lock-waf-rc-model-p2 ./waf install --targets=launcher,hl2_launcher,engine,shaderapivulkan -j 6
python3 tools/quality/portal_boot.py --runtime run/runtime-p2 \
  --build build-rc-model-p2/install --game portal2 --renderer native-vulkan \
  --headless --require-vulkan --map sp_a1_intro4_relit --timeout 180 \
  --startup-command 'mat_queue_mode 2' --startup-command 'r_core_world 1' \
  --startup-command 'r_core_world_strict 1' --startup-command 'r_indirect_producer baked' \
  --startup-command 'r_core_runtime_direct 1' --startup-command 'r_lod 1' \
  --startup-command 'r_staticprop_lod 1' --console-command 'r_core_world_stats' \
  --out <new-LOD-boot-directory>
```

Frozen-path: core plumbing R96 — carry selected Studio LODs and resolved material
slots through the existing model handoffs; no new legacy shading.

This closes the body/LOD geometry selection restriction, not R96/R91. Flex and
eye deformation, remaining material families/variables/proxies, modulation,
overrides/static lighting, cutout/transmission shadows and the remaining frame
cohorts still require their own shared-core implementation and evidence. The
full automatic-eligibility goal remains active.


### R91 portals and viewmodels (2026-10-01, active)

User request: “We need to re-enable portals and viewmodels to the rendercore”.
This slice owns neutral stencil state in render.device, queued per-slot view
state in render.pass.world/composition, and the existing client portal/viewmodel
submission handoff. Material and lighting semantics retain their existing core
owners. Model/LOD work in the shared tree is preserved; ownership coordination
was sent to the active model session. Lab mask, clipping, depth-range, replay
and negative controls precede product integration. R91/R96 remain open.

Frozen-path: core plumbing R91 — capture the client’s portal stencil and view
state and route its depth-mask geometry to the core surface program.

### K12: one probe-volume shader implementation (2026-10-01)

The user requested, “consolidate the PBR and probe lighting shaders”
(2026-10-01). The PBR BRDF and reflection-probe GLSL already have core-owned
shared definitions
(`render/shaders/common/pbr_brdf.glsl` and `reflection_probes.glsl`) consumed
by the native frontend. The remaining duplicated PRBV sampling algorithm is
now owned only by `render/shaders/common/probe_volume.glsl`. The frozen native
`probe_volume.glsl` is a combined-sampler adapter plus its backend-specific
moving-occluder readers; the core keeps separate image and sampler bindings.
No new lighting term or native appearance is introduced. The shared algorithm's
zero result outside the grid also makes the native out parameter defined.

Evidence: the pinned `glslc` compiled world PBR with `DELTA_VOLUME`, model PBR
with `PROBE_VOLUME`, glass PBR, and the core check kernel. `render_lab suite
probe-volume --validate` passed 38/38 and its sensitivity run passed 5/5.
Native world and model PBR pixel suites passed 94/94 and 67/67. The pinned
`shader_toolchain.py check` passed 165/165. Local logs and conformance evidence
are in `quality-results/k12-probe-shader-20261001/`; the exact commands are
the installed conformance and shader-toolchain commands. K11 and K12 remain
open for the Cycles gallery, product image match and deletion of the remaining
native shading once every surface cohort has a core owner.

`archlint check --all` does not pass in the concurrent checkout: it reports
CAP004 for `hammer.formats`, unrelated CAP002 includes, and the two recorded
`game/shared/fstop/blob_networkbypass` ARCH105 findings. None names either
edited GLSL file. The log is retained beside the shader evidence.

Frozen-path: core plumbing K12 — the native probe sampler delegates its math
to the core's shared shader until its remaining surfaces migrate.

### K12: shared directional lightmap math and unused copies (2026-10-01)

The native world PBR shader now calls the core-owned `LightmapDirectional`
function in `render/shaders/common/lightmap_basis.glsl`. Its 2:1 LMAP atlas
sampling remains native because the core represents the two pages separately.
A native GPU pixel case compares a mapped normal with and without a gradient;
the full world PBR suite passes 101/101 checks. The core lightmap basis suite
passes 19/19 and the pinned shader toolchain passes 165/165. Local evidence is
in `quality-results/k12-directional-shader-20261001/`. Changed-file stylelint
passes. Full archlint remains blocked by CAP002 includes and the two fstop
ARCH105 findings already reported above; none points to these edited files.

The unbuilt WebM material files, exact copies of the compiled QuickTime files,
and the unused nested filesystem refcount header, an exact copy of the root
header, were removed. Their source references were absent or commented out.
This removes dormant code; it does not claim a new media or filesystem gate.

Frozen-path: user-requested K12 deduplication — the native world shader shares
the core lightmap formula while its remaining surface cohorts migrate.

### K7: CPU assignment retirement and native game profile (2026-10-01)

The user directed deletion of the test-only CPU light assignment reference as
well as the product CPU path. The headless CPU assignment and sensitivity suites
were removed from the manifest; the GPU suite now owns repeatability, independent
geometry coverage, capacity accounting and seeded defective kernels. The dead
CPU area-mask builder was removed. The installed GPU suite passes 26 checks, or
27 with its GPU-only assignment timings. The 1,000-scene independent geometry
target remains open; the current run covers 100 scenes.

The first complete High `play_p2` route profile on the Radeon 8060S at
1920 × 1080 and 4× MSAA fails the 120 FPS hard floor: 16.484 ms median and
111.222 ms p99 over 845 frames. Stable core pass windows show 15 core world
views per frame on arrival and return, rising to 47 on the reverse view; GPU
core world time rises from about 12 to 54 ms, while GPU BVH assignment stays
under 0.4 ms. CPU core view recording rises from 6.7 to 41.9 ms. The
[GPU BVH record](0016-gpu-light-assignment-2026-10-01.md#gpu-only-retirement-and-game-profile-2026-10-01)
links the frame streams, pass log, suite receipt and limitations. R90/R95/R96
remain partial and the hard performance gate is unverified.

### K11: Intro4 queued-lighting replay lifetime (2026-10-01)

On the published `sp_a1_intro4_probe64` map, a strict native Vulkan product
boot reproduced 192 failed core views. The device refused the world light bind
group with `invalid handle (native 0)`; reducing the shadow atlas did not
change the failure. `PendingView` kept the `StageViewLights` GPU froxel and index
buffers across backend record-stream replays, while `ClusterKernel::Collect`
released those buffers at the next submission frame. The core now retains the
CPU input snapshot but makes the GPU light and shadow work once per submission
frame. The bind-group failure includes the device status for future diagnosis.

The same installed map, renderer, camera, strict mode and screenshot/quit
sequence now pass, both at default exposure and with the forced tone-map scale
set to 4. A turn from 8 to 200 degrees and back to 8 degrees passes in strict
mode without losing the lit bind group. The headless `render.composition`,
`render.world.null` and `render.lights.clusters.gpu` suites pass 53, 37 and 26
checks; changed-file stylelint and archlint pass. The failing and passing
product receipts are under
`quality-results/lighting-diagnosis-intro4-error-code/` and
`quality-results/lighting-diagnosis-intro4-fixed-cache/` (with the default
exposure pass under `lighting-diagnosis-intro4-fixed-normal/` and the turn test
under `lighting-diagnosis-intro4-turn-return/`). This fixes a
GPU resource lifetime defect; the map's default-exposure image remains dark
and its published reflection-probe coverage gates remain failed, so this does
not close K11/R95 or certify visual parity.

A matched-camera comparison also exposes a separate image gap: the earlier
`691ffde` gallery build and current `22f3ea0` build use identical published
map bytes and forced exposure 4, but the 960 × 540 frame's median luminance
falls from 65.4 to 39.9. Albedo, baked-light and image-specular debug views
are similar between those builds; disabling AO or the projected-light term
does not account for the gap. The GPU cluster readback shows 3,240 populated
froxels and 32,866 valid light indices, but the runtime-direct debug image
remains dark even with CPU-generated all-light lists and shadows disabled. The
direct-light image gap's cause and Cycles correctness remain open.
The local comparison receipts are under `quality-results/lighting-diagnosis-intro4-*`.

### K11/K12: Intro4 lighting ownership and bounded shadow work (2026-10-01)

This session owns the reported Intro4 disappearing-light, stencil-crash and
allocation defects in `render.composition` / `render.pass.world`. The lighting
snapshot now belongs to the queued world view, and its GPU lighting and atlas
are built once per submission frame. The former independently capped lighting
deque lost accepted views after 64 cohorts; removing that drop exposed an
atlas allocation per cohort, which exhausted host/GPU memory. A controlled
negative run creates 12 atlases before an 8 GiB diagnostic ceiling; the corrected
native run uses two atlases and peaks below 5.7 GiB. The diagnostic ceiling and
allocation prints are confined to the isolated reproduction build.

The private AO depth/normal prepass has D32 depth and its own raster state;
portal stencil and final-target depth remapping cannot enter that pass.
`render.world.null` fails its new portal case before the fix and passes 41
checks afterward (`quality-results/intro4-private-prepass-{negative,fixed}.json`).
The reported native D32S8 stencil pipeline itself is valid; the incompatible
private prepass state was the defect. Pipeline refusal diagnostics now retain
the device operation, status and native code.

A clean camera reproduction also found reentrant neutral-material creation
from a captured rope draw, exhausting the warning printer's stack. The draw-time
default callback now only looks up a previously prepared neutral; declaration
defaults stay in the captured input when none exists. Native reproduction passes
at 1920 × 1080, 4× MSAA and strict mode, including parallel vertex conversion
(`quality-results/intro4-readonly-default-high-parallel-profile/boot/`).

Frozen-path: defect — the material-system default lookup no longer initializes
a shader inside an active shader draw.

Performance remains failed: the High frame-floor arrival fixture first exceeds
8.333 ms at 12.963 ms (`quality-results/intro4-floor-owned-view/`). The subsequent
High puzzle view's labeled world work is about 55 ms, with diagnostic timestamp
overflow; this is a profiling window, not complete performance acceptance.
The core shadow receiver now gathers the same four bilinear PCF depth texels
in one instruction. The old duplicate hard/soft helper is deleted; sample
counts, filtering and tile bounds are unchanged. Before product integration,
`render_lab shadowed-lights` passes 10 checks and four sensitivity checks;
`render.shadows.pixels` passes 40. Matched full High puzzle-camera runs reduce
arrival median from 66.942 to 64.296 ms and whole-frame GPU p99 from 69.813 to
64.516 ms (`quality-results/intro4-gather-comparison/comparison.json`). Both
complete routes still fail the frame floor. This is an optimization result,
not High image/performance acceptance.

The A* harness now checks swept camera clearance against BSP halfspaces,
including thin walls, and halts on a lighting-layer change while retaining the
first pair. Its five negative/route fixtures pass. The final short puzzle route
halts after 36 layer captures at point 8: specular p95 rises from 0.000953 to
0.005575 as a nearby metal pillar moves further into view. The retained pair
establishes the halt, not another dropout. The longer route also halted on a
material transition. Neither run certifies complete map traversal; static props,
player locomotion and gameplay gates remain outside this camera oracle.
See `quality-results/intro4-astar-near-puzzle-final/route.json`.

At 1080p High, term isolation attributes roughly 38 ms of the 62–63 ms GPU
frame to clustered direct lighting. Disabling terms is diagnostic only
(`quality-results/intro4-high-term-isolation-2/summary.json`); no term is removed
from High. A trial zero-visibility BRDF shortcut passed the lab oracle, but its small
initial timing difference was below observed run-to-run variation. The
subsequent unchanged baseline was faster, so the shortcut was removed
(`quality-results/intro4-shadow-zero-comparison/comparison.json`). The restored
product build and normal `play_p2` runtime include the proven fixes and gather
helper, with every High setting retained.

A second diagnostic brackets High shadows, then disabled shadows, then restored
High: GPU medians are 65.328, 25.158 and 65.647 ms. This identifies about 40 ms
in shadow production/receiver work without counting the disabled phase as
acceptance (`quality-results/intro4-high-shadow-cost/summary.json`). The complete
High route also retains driver-memory samples: peak sampled GPU memory is
4.601 GiB and process RSS is 4.497 GiB, with no diagnostic allocation ceiling
(`quality-results/intro4-shadow-zero-comparison/after-memory-summary.json`).
Samples are observations, not an allocation upper-bound proof. Fold7 hardware
is unavailable; its gate stays unverified. The next performance slice belongs
to `render.pass.shadows` and its shared receiver: eliminate certified redundant
visibility work while preserving the complete filter. The 120 FPS gate remains
failed, not deferred or relaxed.

### K11/K12: Forward+ source comparison and rejected receiver trials (2026-10-02)

The user requested cloning other Forward+ renderers to identify design differences.
Three shallow, filtered source checkouts are retained under
`/tmp/source-forward-plus-references-20261002/`. Exact revisions and checkout
paths are recorded in `quality-results/forward-plus-comparison-20261002/comparison.json`:

- Godot: `e7cfa294a0b81bed7986be04a848cc1832a3f083`.
- Filament: `144ea3160a545aa2a56e4554822f1f426910f045`.
- Wicked Engine: `df44c3db4c4927492bc9c791eac715d98d7ed091`.

This is source inspection; these engines were not built or benchmarked on the
Intro4 scene. Their default shadow algorithms differ from the complete High
image required here. No reference-engine timing advantage is claimed.

| Observed difference | Reference source | Current core source and implication |
| --- | --- | --- |
| Hardware comparison PCF | [Filament shadow sampling](https://github.com/google/filament/blob/144ea3160a545aa2a56e4554822f1f426910f045/shaders/src/surface_shadowing.fs), [Godot lighting](https://github.com/godotengine/godot/blob/e7cfa294a0b81bed7986be04a848cc1832a3f083/servers/rendering/renderer_rd/shaders/scene_forward_lights_inc.glsl), [Wicked shadow sampling](https://github.com/turanszkij/WickedEngine/blob/df44c3db4c4927492bc9c791eac715d98d7ed091/WickedEngine/shaders/shadowHF.hlsli) | `SamplerDesc` has no comparison mode. `ShadowBilinear` gathers depths, performs four comparisons and interpolates manually. The device contract prevents expressing the hardware operation. |
| Direct point-shadow face selection | Filament's `getPointLightFace`; Wicked's `cubemap_to_uv` | `ShadowFacesVisibility` scans each face's matrix. The planner's world-aligned point cube has a known axis order, so this generic search is redundant for that layout. Oriented area-light faces need their own preserved behavior. |
| Coherent light traversal | [Godot clustered surface](https://github.com/godotengine/godot/blob/e7cfa294a0b81bed7986be04a848cc1832a3f083/servers/rendering/renderer_rd/shaders/forward_clustered/scene_forward_clustered.glsl) merges masks with subgroup operations | `surface_program.glsl` walks each fragment's variable index list. A coherent implementation needs equivalent membership and accumulation order, plus a declared subgroup capability; a speedup is still unmeasured. |
| Clustered reflection probes | The same Godot clustered surface iterates reflection masks | `reflection_probes.glsl` scans all ranks to select its two samples. Conservative candidate lists could reduce selection work while preserving global probes, nearest selection and rank-dependent blending. |
| Prefiltered blocker estimation | Filament's current `ShadowSample_EVSSM` reads moment LODs for blocker estimation and penumbra filtering | Our receiver performs sixteen blocker fetches and sixteen bilinear compares. Filament's moment approximation is a different output model. Conservative min/max bounds could instead certify constant visibility and retain our complete filter for uncertain footprints. |

Existing useful mechanisms remain: GPU BVH light assignment, sphere/cone froxel
intersection, cached static shadow tiles with moving-caster composition,
final-target world depth prepass, and material lighting-term specialization.
The comparison does not justify replacing those with another engine's machinery.
Godot gates PCSS by soft-shadow settings and emitter size; Wicked's inspected
source disables its PCSS blocker-search macro; Filament's default is PCF.
Removing our accepted softness to match their defaults would not satisfy rule 7.

The retained map audit reads the published BSP2 entity payload. Its 37 point/spot
lights have no explicit `_distance` and no unsupported fifty-percent curves.
The current producer derives finite bounds for 22 inverse-square lights, roughly
689–7,670 Source units; 15 remain unbounded (14 spots, one point).
`map-light-audit.json` records inputs and the map hash. These broad volumes are
an additional culling pressure point, not permission to truncate nonzero legacy
falloff. Actual per-pixel light/probe distributions still need measurement.

Receiver loop trials are retained in
`quality-results/intro4-shadow-loops-20261002/comparison.json`. Fully rolled and
four-tap grouped loops preserve the lab images but do not establish a native
improvement. On AC, baseline arrival median is 66.411 ms and grouped taps are
72.523 ms. The repeated baseline later rises to 75.876 ms as sampled busy GPU
clocks fall; branch selection (70.067 ms) and hoisted depth mapping (75.016 ms)
therefore do not establish a complete-frame gain. Battery runs are separate.
Each trial passes 18 lab checks with zero validation messages and five
sensitivity checks, including an omitted filter tap. All trial source changes
were removed; the restored product builds and is staged for `play_p2`.
The High frame-floor gate remains failed. Fold7 hardware is unavailable.

The next bounded slices belong to their existing owners: comparison samplers in
`render.device`, explicit cube-layout selection and conservative visibility
bounds in `render.pass.shadows`, and coherent candidate traversal in the light
and probe owners. Each needs its lab oracle and complete-frame measurement
before a performance claim. No roadmap row or budget was advanced by this study.

### K11/K12: reference shadow operations implementation (2026-10-02)

User scope: implement the reviewed Forward+ reference approaches to improve the
complete render time. This session owns the bounded slice: comparison sampling
in `render.device`, receiver operations in `render.pass.shadows`, the existing
surface/view bindings in `render.material`, and integration through the existing
world pass. No additional service, atlas, lighting model or product policy is
introduced. The shadow projection tag now distinguishes a runtime world-axis
cube from a single projection; oriented area-light groups keep their existing
search. The sampler's optional comparison operation owns enable and direction
together. Existing nearest raw-depth sampling remains the blocker-search input.

The first complete soft-image comparison exposed an existing manual-gather
failure: near an integer texel boundary, the texture unit rounds the gather
footprint separately from the shader's floating-point bilinear weights. At one
radius-32 receiver pixel this moved the result from 0.251709 to 0.272949.
A private integer-addressed four-texel oracle removes that ambiguity; the old
unstable gather is retained only as a seeded defect. Hardware comparison
sampling is judged against the existing image band, without removing blocker or
filter taps. Native device contract checks cover comparison direction, equality,
filtering and clamping, with reversed-operation and invalid-description controls.

Evidence is retained under `quality-results/forward-plus-implementation-20261002/`.
Device contracts pass on null, OpenGL (987 checks) and Vulkan (1,038 checks),
with 19/21 GL/Vulkan sensitivity checks; the shadow planner/pixel/world suites
pass. The final lab receiver has 21 checks and five sensitivity checks, with
zero validation messages. Eight complete soft receiver images match the private
integer-addressed oracle within the unchanged band. The 2,205-direction cube
fixture matches the original matrix search exactly; a wrong-face seed produces
1,350 mismatches.

The retained before/after receiver runs on AC show roughly 11–12 percent lower
arrival/reverse frame medians after hardware comparison and direct cube selection.
Those windows were actually **1920×1043**, despite startup reporting 1920×1080;
the relative result is diagnostic evidence only. Their earlier High resolution
receipts cannot certify the profile. The corrected borderless run (`p2`) records
1920×1080 in every measured frame: arrival GPU median 60.574 ms, reverse 23.131 ms,
whole-bracket GPU p99 61.598 ms and max 61.861 ms. The frame-floor gate remains
failed, and full cohort/image acceptance and Fold7 hardware remain unverified.

### K11/K12: GPU attribution and BRDF trials (2026-10-02)

User scope: make profiling reusable and grepable, identify the expensive GPU work,
compare P2:CE, and optimize the BRDF without changing its model. The existing
stats/timer/debug owners remain authoritative. `tools/quality/render_profile.py`
joins delayed GPU queries to their originating frame before selecting phases,
reports frame percentiles and sequential backend segments, and preserves core
nested scopes as inclusive per-window means. It rejects corrupt/incomplete data
instead of reporting missing timing as zero. `--profile` on the existing frame
harness enables the existing timers. `tools/quality/render_profile.md` owns the
working collection/analysis commands and timing limitations.

The existing bounded timer pool used 64-query private chunks for approximately
104–108 short encoder cohorts, exhausting its storage and dropping timestamps.
Chunks now contain 16 queries with the same global bound. The eight-encoder
negative fixture detects the old exhaustion; 84 graph checks pass and native
captures report zero drops. Counts are encoder cohorts, not 104 cameras.

`Frozen-path: defect fix — per-frame drawable extent in native Vulkan statistics.`
The native stats owner now records its actual swapchain extent; the High receipt
requires every measured frame to match the profile. The borderless window fixes
the compositor decoration loss. Startup dimensions alone no longer qualify a run.

The existing lighting-term catalog gains `shadow_visibility`, proven in the lab:
turning it off with an atlas bound is the untiled-light image bitwise. It bypasses
receiver visibility only; producer work, light falloff and BRDF evaluation remain.
The diagnostic GPU median falls from about 61 ms to 23.8 ms in arrival, implicating
visibility work and its shader-resource effects as the dominant cost. With
visibility, IBL and SSR off, full versus diffuse-only BRDF medians are 17.429
versus 14.114 ms. This estimates about 3.3 ms for the direct specular path in this
view; specialization changes resource use, so these differences are not additive
per-pass timers or accepted output-preserving speedups.

Two BRDF trials were removed after measurement:

- A per-fragment GGX/Smith context reused roughness powers, N.V and the view
  square root, and consumed the light loop's existing N.L. Its 5,040 analytical
  GPU cases pass a fixed 2e-5 relative + 1e-6 absolute band against the independent
  CPU definition; a wrong-view seed fails 2,320 cases. The lab passes 28 checks.
  Matched full-image runs (`a5`, `a6`) have arrival GPU medians 65.222/65.827 ms
  at sampled clocks 1,920/1,914 MHz, and reverse 25.977/25.997 ms at 1,784/1,774 MHz.
  That does not establish a useful repeatable improvement.
- Skipping the BRDF after the complete filter returned exactly zero light gives
  four bitwise-identical receiver images, 25 lab checks and five sensitivity
  checks. Full-image runs (`c0`, `c1`, between controls `a6`, `c2`) do not establish
  a gain after clock changes; the repeated trial's reverse median is 27.107 ms
  at 1,640 MHz versus the final control's 26.944 ms at 1,645 MHz. No truncated
  light set, cheaper BRDF approximation or sample reduction was retained.

The production BRDF math is unchanged. `brdf-analysis.json` records diagnostics,
trial results, frame/power hashes and excluded incomplete runs; immutable private
runtime hashes, rejected sources and lab logs are retained beside it. `a1`/`a2`
had missing private QA setup and briefly overlapped; they measured no completed
route and are explicitly invalid, never part of a gain claim.

A separate `RADV_DEBUG=shaderstats` capture (`ds`) exposes large compiled fragment
programs: up to 192 VGPRs, 10,472 instructions and about 56 KiB of code, with no
spills and a compiler residency ceiling of eight subgroups/SIMD. These are static
compiler observations, not measured dynamic occupancy or a shader-to-pass mapping.
`tools/quality/radv_shader_stats.py` exports hashes and counts with explicit units
and rejects malformed/absent/truncated compiler blocks. The evidence supports
investigating full receiver/program complexity rather than assuming GGX arithmetic
alone explains the forward cost. The public Source-PBR ancestor cloned at
`b8c4b76882241ea8cb506e89a61a1f5448d24e71` uses separable Schlick-GGX geometry;
that differs from the core's height-correlated Smith model and does not establish
the installed P2:CE kernel or authorize changing the core's BRDF.

### P2:CE contextual baseline with observed PBR shaders (2026-10-02)

Installed P2:CE build 25033687 runs its D3D11/DXVK-native renderer against a private
`-game` directory. Installed textures/configuration are preserved. Its exported
VBSP retains the original texture-string names; the modern world's generated
material names are separate. The initial capture's inspected generated VMTs did
not prove native PBR usage. A verification run observes `LightmappedGeneric` on
an original tile, so that capture is not labeled a full PBR comparison.

The corrected private comparison maps 39 named native world materials to the
same generated base-color/normal/MRAO texture bytes through Strata's `PBR` shader.
`p2ce-native-material-adapter.json` records each explicit mapping and source/output
hash. The native post-bracket material dump observes those PBR shaders, and a
native screenshot proves 1920×1080. Four map/eye-position checks pass; 4x MSAA,
anisotropy 16, picmip -1 and LOD 0 are queried. The installed content stays read
only; the temporary known Sentry consent file is restored to its prior absent
state after the run, and the private process group is drained.

`tools/quality/p2ce_present.py` starts MangoHud's existing local control socket,
collects localhost netconsole markers and analyzes per-present CSV. It records
input hashes and GPU clock/temperature telemetry, rejects missing/failed checks
and corrupt evidence, and keeps a fixed half-second phase-edge guard. This is
steady-view presentation evidence, not a trimmed hard-floor result. MangoHud
0.8.3 rc1 is extracted privately from its Fedora 44 package; no overlay is drawn.
The corrected arrival/reverse/return presentation medians are 2.8867/2.0572/2.7738 ms.
Native pass/GPU execution timestamps are unavailable through this collector.
Original native model materials, indirect bake, reflection lookup, shadow filter,
portal effects and light-update policy differ; this demonstrates feasibility and
cannot certify equivalent-output gains or core image quality.

The current native Vulkan product and lab build successfully and `play_p2` is
restaged. The debug fixture's obsolete rim-light rejection was replaced with enabled
cloak (an actually unmapped term); invalid debug bits now come from the complement
of the authoritative term mask. The new visibility name has a positive parser/
validation check. Debug views pass 90 checks, lighting controls 32, shadowed lights
21 and shadow sensitivity five. Profiling tools have 55 profile-related, 22
frame-floor, 10 external presentation and four driver-statistics fixture checks;
architecture/style checks pass. Final logs and hashes are retained in
`quality-results/forward-plus-implementation-20261002/brdf-final-validation/`.
The High 120 FPS gate remains failed. No roadmap performance promotion, complete
quality/cohort claim or Fold7 support claim is made.

### K11/K12: shadow receiver microbenchmark slice (2026-10-02, in progress)

User scope: microbenchmark and optimize the measured receiver cost. The existing
`render.pass.shadows` receiver helper retains ownership; `render_lab` measures its
complete operation against a private immutable pre-change control. GPU dispatch
timestamps exclude uploads/readback, include all sixteen blocker and sixteen PCF
taps where the algorithm requires them, and accompany output and lifetime checks.
Perspective, orthographic and hard-filter cases cover lit, shadowed and penumbra
regions. No filter, effect, light set or profile quality is reduced. A candidate
must pass the lab before product integration and receive matched complete-frame
measurements; this slice does not promote the still-failed High floor.

The comparison follow-up tracks bounded experiments here, with raw evidence in
`quality-results/rendercore-opt-20261002/`. No filter, sample count, effect,
resolution, or light cohort was reduced. The High 120 FPS gate remains failed.

| Trial | Observation | Disposition |
| --- | --- | --- |
| RCV-01: clamp comparison coordinates directly to tile texel centres | Paired receiver dispatch ratio 0.9265 against the immutable control; identical-code calibration 0.9999. The earlier full-frame arrival/reverse/return GPU medians were 57.872/22.112/58.110 ms before and 57.324/22.161/57.542 ms after. | Retain the simpler arithmetic, but **no established full-frame gain**; the small differences are insufficient evidence. |
| RCV-02: GPU min/max hierarchy over changed shadow tiles | The spatial query reached a 0.8054 dispatch ratio, with exact output and exact hierarchy checks. Full-frame trials did not establish a gain. A single-query revision and a cheaper whole-tile-root revision also failed to establish a worthwhile benefit. | **Removed from the product and lab source**, including the atlas buffers, pass, bindings and extra benchmark plumbing. Rejected code is retained as `rejected-bounds.patch` and `rejected-bounds-sources.tar.gz` in the evidence directory. |
| RCV-03: oriented area-light face selection | 29 native correctness checks and five sensitivity checks passed. Control/candidate/candidate/control arrival GPU medians were 62.837/60.346/59.499/58.582 ms; the control improved too as clocks rose. | **Rejected and removed**: no attributable frame gain. `rejected-area-face.patch` and `rejected-area-face-sources.tar.gz` retain the experiment. |
| RCV-04: subgroup-coherent area-light traversal | Form a subgroup union of froxel masks, retaining each fragment's own membership and ascending accumulation order. Authored warp materials retain their original traversal. All 16 scalar/candidate lab images match bitwise (68 total checks). | **Rejected and removed**: candidate arrival/reverse/return GPU medians 57.926/22.651/58.482 ms versus fresh scalar control 58.195/22.318/57.999 ms. No consistent gain. Code, private native injection and logs are archived with the evidence. |
| RCV-05: visibility raster resolve followed by material shading | Exact image parity in the bounded lab fixture; roughly 3% faster for large area-light cases, about 31–32% slower for large spot-light cases. | Lab benchmark retained; product remains inline. See the detailed feasibility result below. |
| RCV-06: compiled coherent runtime/area traversal | Separate lab shader merges sorted runtime lists and area masks, then explicitly broadcasts the selected ID. Inspired by id Tech 6 and Godot Forward+. | Lab and one captured frame match exactly when the diffuse light-warp pipeline retains its original compiled shader. All seven replaced pipelines still allocate 192 VGPRs. Production is unchanged; whole-frame speedup is unproven. |
| RCV-07: compiler specialization attribution | Freeze the captured shader's debug-term mask to remove area lighting, shadow receiving, or both; retain every other captured pipeline setting. | Area removal lowers 192 to 144 VGPRs (120 for static models); shadow removal lowers 192 to 144; both lower it to 96 (84 for static models). Diagnostic only: these images intentionally differ and no effect is removed from the product. |
| RCV-08: compact LTC clipping topology | Store original-edge/intersection identity in 18 bits and reconstruct FP32 vertices, preserving clipping and accumulation order. | Retained: all eight pipelines fall from 192 to 144 VGPRs; four captured MSAA samples match the corrected control exactly. Matched ABBA full-frame trials show 3.25–3.77% lower GPU time. The High floor remains failed. |

RCV-09 retains opaque-model prepass coverage and read-only lit depth. The existing final-target
world prepass now also records opaque PBR static and posed models before world
shading, through the same geometry/constants/bindings helper as their lit draws.
Transmitting/blended models and views with ordered stencil mutations or depth
overrides retain their existing stream behavior. Eligible opaque PBR lit draws
then test equal against the prepassed depth, with depth writes disabled. Merely
adding model depth while retaining lit depth writes gave about 50 ms GPU time;
read-only lit depth permits earlier rejection around the shader's clipping and
discard and reduces arrival GPU time to 35.4–36.1 ms against 55.6 ms control.
The posed-model lab passes 71 checks, including exact single-layer, overlapping
posed-layer, overlapping static-instance, clipped-layer and alpha-cutout depth/control image
comparisons; view-state passes 16. Architecture and changed-line style checks
pass. Native product compilation changes only the world-pass object and launcher
library. The private runtime changes only that library; all other staged
libraries remain identical between runs. Two candidate runs have arrival frame
interval medians 50.482/48.069 ms versus control 60.287 ms. This is a substantial
GPU improvement, not 120 FPS acceptance; CPU cost now limits more of the frame.
Control repetition and further image/CPU attribution are in progress. The existing
map-name assertion mismatch and hard-floor failure remain recorded, with no
effect, resolution, sample count, light set or tolerance reduced.
Evidence and immutable variants are in
`quality-results/rendercore-work-amplification-20261002/`. Work continues in a
private checkout with independent build and runtime outputs at the user's request.
That checkout is `/home/john/.codex/worktrees/forward-perf/source-engine`, on
`codex/forward-perf`; Waf uses separate `.lock-waf-forward-perf` and
`.lock-waf-forward-lab` locks and `build-forward-perf`/`build-forward-lab` outputs.
The runtime and raw measurements are `/home/john/.rc-perf-4da389/run` and
`/home/john/.rc-perf-4da389/evidence`. Each run records and verifies its staged
launcher hash. Other checkouts' ordinary builds cannot replace these outputs.

The repeat control remains stable. Four AC-powered runs, in order, use identical
runtime/map/configuration inputs and replace only `bin/liblauncher.so`:

| RCV-09 run | Arrival GPU | Reverse GPU | Return GPU | Arrival frame interval |
| --- | ---: | ---: | ---: | ---: |
| `isolated-control-1` | 55.573 | 21.722 | 55.712 | 60.287 |
| `read-only-2` | 36.125 | 19.701 | 35.906 | 50.482 |
| `read-only-3` | 35.389 | 19.596 | 35.456 | 48.069 |
| `isolated-control-2` | 55.692 | 21.798 | 55.734 | 60.263 |

These are milliseconds and untrimmed phase medians, not hard-floor acceptance.
The arrival GPU reduction is 35.1–36.5%; the frame-interval reduction is
16.3–20.2%. Alpha cutouts reveal an opaque model behind them in the new oracle;
clip-plane coverage has independent covered/discarded checks. All compared image
pixels match exactly. Product/lab builds, 16 view-state checks, 58 composition
checks, architecture baseline/inventory and changed-line style checks pass.
Fold7 measurements remain unavailable: no declared device runner is attached.

RCV-10 is a rejected CPU shadow-preparation experiment after that GPU gain. A separate
`perf record -F 499 --call-graph dwarf,8192` arrival-warmup diagnostic recorded
3150 samples, with zero lost samples. `DrawStageShadows` has 17.56% inclusive
and 10.58% self cycle weight across the sampled process; its `math::Transform`
calls have another 5.97% self weight (included in the first number). These are
sampled CPU cycle shares, not GPU time or independently additive frame timings.
The console reports 876 kept shadow tiles and zero redrawn tiles in settled
arrival windows, yet the CPU still scans held views and tests captured movers.
The experiment first checks a held view's matching plan index, retaining the
full search for changed/reordered plans. It prepares each captured mover's
unchanged bounds, placement and identity once per operation rather than once
per shadow view. Culling arithmetic, tile identity, mover accumulation order,
atlas invalidation and draw commands remain unchanged. No persistent cache or
new resource mechanism was added. After AC power was restored, matched candidate/
read-only-control arrival intervals were 51.388/51.310 ms; CPU medians were
45.314/45.247 ms and GPU medians 37.765/37.847 ms. Reverse intervals were
20.352/20.424 ms and return intervals 50.580/50.860 ms. Both runs retained AC
throughout, approximately 70 W GPU power and 2113/2100 MHz whole-run median GPU
frequencies. There is no worthwhile attributable gain, so the preparation
experiment is removed from supported source. Its code and result remain in the
checkpoint history and `rejected-shadow-prep.cpp`/`rejected-shadow-prep.patch`.
`shadow-prep-1` was interrupted when compilation overlapped startup;
`shadow-prep-2` lost AC power and GPU frequency fell to 976 MHz from the AC
control's 2083 MHz median. Neither establishes a performance comparison.
Battery measurements are tracked separately at the user's direction.
`shadow-prep-battery-1` crossed the return to AC (53 battery and 49 AC samples)
and is also excluded from the matched comparison.

The final settled-arrival CPU profile triggers on `floor_begin`, rather than
warm-up. It records 2960 samples with zero loss and is excluded from performance
comparison because sampling adds overhead. `WorldPass::Record` has 21.06%
inclusive process cycle weight, including bind-group preparation/allocation;
`VulkanDevice::Submit` has 16.96%, legacy mesh emission 14.03%, and
`DrawStageShadows` 5.82%. Nested shares overlap. `LightingInputs` alone has 4.03%
self weight and malloc has 5.27% on the material queue. Thus steady CPU cost is
spread over queued view lookup, per-cohort resource preparation, stream emission
and command translation; the warm-up shadow sample overstated the steady
shadow share. This attribution explains why RCV-10 did not move the frame.
The profiler data and flat/inclusive reports are retained as
`read-only-cpu-steady*` in the private evidence directory. The supported change
remains the measured RCV-09 depth coverage/read-only state, with all shader and
quality settings preserved. The retained product rebuild's launcher SHA256 is
`e5d71f5a76db6f9ed82ee7711ca3ac9a6b415d626516f3159ba233e0690bcc04`,
identical to the measured read-only binary.

Ordinary runs without `--profile` also retain the gain. After restoring AC,
`control-normal-1` versus `retained-normal-1` arrival/reverse/return GPU medians
are 55.784/21.912/56.099 versus 37.603/19.597/37.663 ms; frame intervals are
56.129/22.093/56.409 versus 48.327/19.975/47.927 ms. Arrival CPU time falls
47.395 to 41.281 ms. This pair establishes 32.6% less arrival GPU time and
13.9% less frame interval (16.1% higher frame rate) with detailed profiling off.
Both stay on AC, near 70 W GPU power, at whole-run median frequencies
2081/2054 MHz. Detailed per-pass analysis intentionally reports missing core
scope timers in these ordinary runs; full-frame metrics are present, while
the earlier profiled runs supply pass attribution. The workload's existing
map-name assertion and High-floor failures still prevent acceptance. No trial
is recast as a gate pass, and no statistical outlier is trimmed.

The final native product image run `final-image-3` passes the existing
`portal_boot.py` screenshot/provider checks at 1920x1080, explicitly selecting
`native-vulkan` and retaining 4x MSAA and High. It produces a finite detailed
image and does not change protected login-session files. Its capture seed
removes only broken links to unrelated bootstrap map mounts, recording those
unavailable paths separately; the selected frozen Intro4 mount and its complete
content remain intact. This is product smoke evidence, not a new Cycles golden
or complete-map/cohort acceptance. The initial capture setup/attestation failures
are retained separately rather than hidden.

All writable private runtime files have been detached from shared hard links;
runtime and saved variant libraries are read-only. Staging replaces a library
atomically instead of writing through an existing inode. The final normal-run
receipts hash all 29 staged libraries before and after execution and require
private inodes. Source and build inputs live in the private worktree, with
read-only private dependency copies and independent Waf locks/output trees.
The retained code is committed both there and in the main checkout; unrelated
reflection-probe edits and submodule state are preserved.

RCV-08's image comparison exposed a pre-existing correctness defect before its
optimization could be accepted. At captured event 3066, primitive 245, pixel
(1365, 350), the interpolated receiver has x = 720; the first one-sided emitter
also has center.x = 720 and both half-axis x components are zero. The point is
coplanar with the emitter. `area_light::Faces` requires a strictly positive
front-side distance, but the shader relied only on the final integral's sign,
which can admit spurious coplanar light. `LtcRectangle` now rejects that case
before integration, using the rectangle's geometric normal rather than the
receiver's shading normal. Two-sided behavior is unchanged.

Eight new coplanar image comparisons cover four materials and two camera views;
the original 52 checks remain. The `no-front-test` seeded shader must fail those
checks. All five sensitivity verdicts pass with the unoptimized, corrected
control. This fix is tracked separately from register optimization: both the
old polygon algorithm and each candidate receive the same front-side test.
Evidence is in `quality-results/rendercore-ltc-stream-20261002/`, including the
captured pixel inputs and light constants. No tolerance or golden was relaxed.

RCV-08 retains the compact topology implementation in `render/shaders/common/ltc.glsl`.
The old six-vertex mutable polygon is removed. The area-light suite passes 60
checks and five sensitivity verdicts; shadowed lights pass 153 checks. All four
1920 x 1080 RGBA8 MSAA samples match the corrected control byte for byte in the
captured frame (`replay-topology-fixed.json`). Across 37 lab float images versus
the earlier, uncorrected control, 16 are bit-identical and the maximum absolute
float difference is 3.0517578125e-05; existing oracles and tolerances are unchanged.

The ordinary-game comparison uses a private runtime, one frozen map directory,
and saved control/candidate launcher libraries. Both variants include the same
coplanar correctness fix. Only the launcher library changes; engine/backend
libraries match. GPU medians in milliseconds are:

| ABBA run | Arrival | Reverse | Return |
| --- | ---: | ---: | ---: |
| Control 1 | 58.081 | 22.710 | 58.264 |
| Candidate 1 | 56.082 | 21.974 | 56.265 |
| Candidate 2 | 55.872 | 21.874 | 56.224 |
| Control 2 | 58.234 | 22.610 | 58.630 |

Averaging the run medians gives reductions of 3.75%, 3.25% and 3.77% respectively.
Busy-GPU sampled median clocks are 2137–2187 MHz for controls and 2135–2137 MHz
for candidates; power is approximately 70 W and temperatures 85–89 C. The final
control improves its clock while recovering the slower frame times. Clocks were
not locked. Raw timings, binary hashes, telemetry and staging clarifications are
in `paired-summary.json` and `benchmark-notes.json` beside the replay evidence.
The route still reports the pre-existing map-name assertion mismatch, and every
run fails the 8.333 ms floor, so these are diagnostic performance comparisons.
No effect, resolution, sample count or light set was reduced. Fold7 is unavailable.

Native lab/product builds and all 211 pinned shader generation units (417
artifacts) pass. Architecture check-all, baseline and inventory verification
pass. Three shader-tool fixture failures (two failures and one error) reproduce
against the parent tool source; their logs are retained separately. This modest
gain does not resolve the complete-frame performance deficit.

RCV-07 uses the Radeon 8060S / RADV STRIX_HALO, Mesa 26.2.3, and the existing
1920 x 1080, four-sample High arrival capture. All eight PBR pipelines use the
same original SPIR-V SHA-256
`035a81e12042bc8e4454850adbb429ca65e88aa188cc4f53b7156324b6571fe4`.
The new `tools/renderdoc/shader_register_probe.py` freezes only SpecId 102
(`kDebugTermsOff`, owned by `debug_view.glsl`); masks 0, 4, 4096 and 4100 retain
everything, remove area lighting, remove shadow receiving, and remove both.
Other material specializations remain those of the actual captured pipeline.
This follows [Godot's area-light specialization finding](https://github.com/godotengine/godot/pull/119970),
but the scene contains area lights, so compiling them out is attribution rather
than a product optimization.

| Captured fragment pipeline | Full | No area | No shadows | Neither |
| --- | ---: | ---: | ---: | ---: |
| World 22509 | 192 | 144 | 144 | 96 |
| Static models 22523 | 192 | 120 | 144 | 84 |
| Posed models 22536 | 192 | 144 | 144 | 96 |
| Other five PBR variants | 192 | 144 | 144 | 96 |

These are allocated VGPRs, not timings or an additive register budget. The
world executable shrinks from 56,732 to 40,464 bytes with area lighting removed;
with both effects removed it is 17,044 bytes. Every variant reports zero
register spills and scratch allocation. The zero-mask calibration matches all
four final MSAA sample hashes and all executable statistics other than the
driver pipeline identity hash, including the code hash. Each removal changes
the final image and compiled executable, as expected. Thirteen RenderDoc tool
tests pass, including five tests guarding the specialization rewrite.

Raw evidence is in `quality-results/rendercore-register-ablation-20261002/`.
The capture SHA-256 is
`c12b4bc89c0660fb4bf4f727d226a0bc0291254ac6b258429a0a7b1f960caa10`.
Run each value in a **fresh replay process**: the initial combined replay
returned stale disassembly cached by original pipeline ID despite changed pixels.
Its root `results.json` is excluded; the `original`, `calibration`, `area-off`,
`shadows-off` and `area-and-shadows-off` subdirectories are the valid runs.
The installed command is documented in `tools/renderdoc/README.md`; use events
`3066,30434,2766,4689,6652,2899,30443,2822` for this capture. No shipped source or
binary changes in RCV-07. No frame-time improvement or platform acceptance is
claimed; Fold7 hardware is unavailable for this desktop compiler diagnostic.

RCV-06 sources are checkpointed at the user's request before further experiments.
Its owning surface shader uses a lab-only define; there is no new device
capability or product default. The first trial was invalidated by a shared-file
restore before compilation and is excluded explicitly in
`quality-results/rendercore-coherent-20261002/invalid-first-trial.json`.
The rebuilt candidate SPIR-V contains the subgroup minimum, mask union and
broadcast operations. The shadowed-light suite currently reports 153 checks
and zero failures, including 24 new image comparisons. This is a development
checkpoint, not a complete portability, material-derivative or performance gate.
The pinned reference is
[Godot e7cfa294's traversal](https://github.com/godotengine/godot/blob/e7cfa294a0b81bed7986be04a848cc1832a3f083/servers/rendering/renderer_rd/shaders/forward_clustered/scene_forward_clustered.glsl#L2543)
alongside the retained id Tech 6 speaker notes. Godot is cloned separately under
`/home/john/Downloads/idtech-rendering-research/godot`; it is not an engine dependency.

The source checkpoint is `1d09ff58e`, followed by the lab formatting correction
`dec873246`. Native validation (`render_lab suite shadowed-lights --validate`)
passes 153 checks, including the 24 new comparisons. The candidate SPIR-V SHA-256
is `09aed49e9116ca4f142aba716d443c74eea1bc187c7c2a3f2913f0d22e96d9be`;
the no-define control is
`035a81e12042bc8e4454850adbb429ca65e88aa188cc4f53b7156324b6571fe4` and
matches the captured world shader byte for byte. The local evidence directory is
`quality-results/rendercore-coherent-20261002/`; it is ignored by Git, so these
commits retain the implementation and this result record, not the raw captures.

Replacing all eight PBR pipelines in the High arrival capture initially changed
roughly 450 bytes per MSAA sample, by at most 6/255. Pixel history isolated the
affected pixels to the portal gun's diffuse light-warp material. Its per-light
texture lookup uses implicit gradients; changed lane participation is a plausible
cause, not an independently isolated derivative measurement. Keeping that one
pipeline on its original compiled shader and replacing the other seven produces
**zero changed bytes in every one of the four 1920 x 1080 RGBA8 MSAA samples**
(`replay-no-warp.json`, 8,294,400 bytes compared per sample). This is one captured
frame, not complete material or camera coverage. The replay's shader-ID exclusion
is capture-specific diagnostic code, not an acceptable product selection policy.
A supported implementation must select from material requirements and declared
device capabilities, and must cover both diffuse and specular warp materials.

The replacement fragment executables all still report 192 VGPRs, subgroup size
64 and eight subgroups per SIMD, without spills or scratch allocation. Explicit
broadcast alone has therefore **not reduced the compiler's register allocation**
in these variants. These are compiler resource limits, not measured dynamic
occupancy. No ordinary-game matched frame timings exist yet for RCV-06; neither
the exact image comparison nor the lab timings establish a full-frame speedup.

The latest scalar control (`g1-control`) records arrival/reverse/return frame
interval medians of 62.652/23.098/62.445 ms and GPU render medians of
58.195/22.318/57.999 ms. Together with approximately 99–100% GPU busy during
measured phases and the earlier receiver-visibility diagnostic (about 61 to
23.8 ms), this supports a primarily GPU-bound workload. CPU wall scopes overlap
GPU work and may contain waits; they do not establish an independent CPU critical
path. Engine wall medians remain 18.390/14.160/19.094 ms, so this evidence does
not certify that the CPU could sustain 120 FPS after GPU optimization.

These trials targeted small receiver operations inside the full surface shader.
Standalone improvements did not transfer into a consistent complete-frame gain.
Register pressure, texture latency and lane divergence are still hypotheses;
no hardware-counter capture here isolates their relative contributions. Future
receiver work needs full-shader measurements before promoting a microkernel win.
RCV-04 also fails the pinned OpenGL translator (subgroup operation requires Vulkan
semantics). Its native-only injection was temporary and is removed; no unsupported
subgroup requirement or capability claim remains in the product or lab.

RCV-04 does **not** rule out id Tech 6's scalar light traversal. On re-reading the
downloaded speaker notes, their register reduction depends on removing the
divergent path from the compiled program. RCV-04 retained a runtime divergent
fallback for authored warp materials and did not verify scalar light-record
loads or lower register allocation in the expensive game variants. Its rejection
applies to that prototype only. The engine comparison already called for mapping
compiled variants to actual game draws before changing them; the flat receiver
fixtures below cannot replace that attribution.

RCV-02's exact-depth and partial-update tests passed: removing a blocker rebuilt
only its tile and preserved cached neighbors; guarded, non-origin tiles matched
the original receiver. The final hierarchy lab ran 19 checks plus three sensitivity
checks, and the image suite ran 21 checks. This proved correctness, not a product
speedup. Whole-frame root/control arrival/reverse/return medians were
69.822/27.631/71.765 ms versus 73.298/29.246/75.510 ms, but the root trial ran at
higher clocks and lower temperatures; those numbers do **not** justify promotion.
The initial 70 W runs and later approximately 55 W runs are not interchangeable.
Power/temperature/frequency logs are retained for every product trial.

Every complete product route still reports the pre-existing `map.loaded`
assertion mismatch (`sp_a1_intro4` versus the requested probe64 name), in addition
to the frame-floor failures. Those captures are diagnostic evidence only. One
incomplete run (`c1`) has no valid route frames and is explicitly excluded.
Early binary identity records omitted `liblauncher.so`, which owns linked core
shaders; subsequent records include it, source hashes and the actual commands.
Concurrent source changes and thermal differences further limit early A/B claims.

After removing the rejected experiments, `render_lab` builds with 415 shader
artifacts and zero generation failures; the native shadowed-light suite passes
21 checks with validation enabled. Architecture and changed-line style checks
pass. The final product control build succeeds. No High performance gate is closed.

The bounds experiment exposed a pipeline-recipe defect: reflected draw-constant
ranges were lost by `ResolvedPipeline::Desc()`. The shared recipe owner now takes
the maximum reflected stage range. The shader-library suite passes 14 checks,
including the rejected zero-range negative control. No hierarchy remains as an
unused product mechanism. Rolled/grouped loops and hoisted depth mapping remain
rejected historical trials. Owners remain `render.pass.shadows`,
`render.shader-library`, and `render.lab`; no legacy backend shading was changed.

#### RCV-05: separate visibility evaluation (2026-10-02)

User direction: keep receiving enabled and investigate evaluating visibility
outside the large material shader. The experiment is private to `render_lab`;
`render.pass.shadows` still owns the existing receiver/filter algorithm, and
`render.material` still owns its surface program. No new public pass, material
option, backend requirement or product selection mechanism was added.

Two lab variants compile the same surface source. The first rasterizes the same
receiver geometry and returns per-light visibility, with unused BRDF results
removed by compilation. It preserves geometric-normal derivatives, interpolated
smooth normals, light transforms, source-size calculation, pixel rotation,
sixteen blocker samples and sixteen comparison-filter samples. The second reads
those values with an exact texel fetch before applying the original lighting.
The mask is RGBA32F, with a stable light index per channel. The lab explicitly
rejects more than four lights or mixed runtime/area index spaces; this is a
fixture bound, not a new product light limit. It borrows the unused detail-texture
binding only in its private shader variants; ordinary materials retain that
binding's original meaning.

The native fixture uses two box casters, two shadowed spots plus an untiled point,
and separate two/four-area-light scenes planned and drawn through the existing
shadow owner. Two source sizes, overhead/oblique views, four metal/roughness
materials and 128-square/1024-square targets give **96 bitwise image comparisons**.
Six deliberately swapped-light masks fail comparison. The installed shadowed
suite passes **129 checks with zero validation messages**, and its existing
sensitivity suite passes **5 checks**. These cover a single opaque receiver layer
at one sample; they do not certify 4x MSAA edges, transparency, cutouts, skinned
models, nested views or the complete game frame.

Timing uses one GPU submission per case, 128 warm-up pairs and 64 measured pairs
with alternating fused/split order. The split interval includes both raster
passes, clears, mask writes/reads and barriers. The fused interval includes its
raster pass and corresponding target barriers. Fixture uploads, CPU recording,
submission, atlas production and image readback are outside both intervals.
Correctness renders precede timing. Each timed sequence ends with an image
readback; raw paired samples accompany the summary. This is a GPU draw-scope
comparison, **not** a complete-frame or CPU/GPU-placement acceptance result.

Radeon 8060S / RADV STRIX_HALO, final installed conformance run:

| Scene | Resolution | Fused median | Split median | Median paired split/fused ratio |
| --- | --- | --- | --- | --- |
| Two shadowed spots | 128 x 128 | 0.014628 ms | 0.020719 ms | 1.4198 |
| Two shadowed spots | 1024 x 1024 | 0.289590 ms | 0.379802 ms | 1.3108 |
| Two shadowed area lights | 128 x 128 | 0.020779 ms | 0.027152 ms | 1.3068 |
| Two shadowed area lights | 1024 x 1024 | 0.624386 ms | 0.609518 ms | 0.9741 |
| Four shadowed area lights | 128 x 128 | 0.040597 ms | 0.047069 ms | 1.1453 |
| Four shadowed area lights | 1024 x 1024 | 1.193367 ms | 1.156157 ms | 0.9719 |

Repeated paired runs gave approximately 0.969–0.974 for large area cases and
1.31–1.32 for large spot cases. This is a small area-light benefit and a clear
spot-light regression, not evidence for separating every receiver. Fixed pass
and storage costs outweigh the benefit in the small cases. The experiment does
not isolate dynamic occupancy, texture stalls or register pressure as the cause.

**Disposition:** retain the reproducible lab comparison; keep production shadow
receiving enabled and inline. No game shader selects either lab variant. A
product candidate needs complete-frame evidence and correct per-surface/sample
storage first. A dense float mask costs 16 MiB at 1024-square for four lights;
at 1920x1080 it would cost 31.64 MiB. Naively extending to 64 light channels is
506.25 MiB at one sample, before read/write traffic and any extra MSAA storage.
A many-light design must account for sparse froxel membership and stable light
identity rather than extending that dense fixture directly. Screen-space depth
alone also does not supply the current geometric and smooth normals exactly;
reconstruction, coverage and transparency remain explicit correctness work.
The 120 FPS gate remains failed, and no platform/performance promotion is made.

Evidence: `quality-results/visibility-split-20261002/`, including conformance
identity/input digests, raw timing pairs, shader/build logs and the earlier
CPU-driven timing trial (superseded by sustained paired draws). Reproduction:

```sh
WAFLOCK=.lock-waf-rc-lab-main ./waf build --targets=render_lab -j8
python3 tools/quality/conformance.py check \
  --suite render.lab.shadowed-lights \
  --suite render.lab.shadowed-lights.sensitivity \
  --out quality-results/visibility-split-20261002/conformance.json
```

The existing lab and Portal 2 product configurations build successfully. Full
architecture, baseline/inventory verification and changed-line style checks pass.

#### Actual arrival-frame attribution follow-up (2026-10-02)

The user challenged the repeated small experiments against the existing id Tech
research. The missing step was the comparison report's own requirement: connect
the expensive compiled shaders to actual game draws before choosing another
receiver optimization. RCV-05's flat one-sample planes do not establish that
connection or predict the complete game's gain.

A release-shader RenderDoc capture now reproduces the arrival eye
`(145, -440, 90)`, angles approximately `(8, 20, 0)`, at 1920x1080 on the Radeon
8060S. The existing High cfg keeps receiving enabled. In the 12-frame engine GPU
timing window immediately before the capture, `core world view` is 59.563 ms:
world PBR 20.746 ms, static-model PBR 23.088 ms, posed-model PBR 10.755 ms,
GTAO 2.086 ms and cluster BVH assignment 0.378 ms. These are diagnostic window
means under capture instrumentation, not new ordinary-run medians or independent
CPU timings. The expensive surface cohorts dominate; assignment is not the
primary target. Existing whole-frame baseline and acceptance failures stand.

Source inspection also establishes that the target-depth prepass in
`render.pass.world` draws world surfaces before world lighting, while static and
posed models are shaded later. It does not provide a complete opaque-model
prepass. The cost attributable to that missing coverage remains unmeasured.

Evidence and replay scripts are in
`quality-results/rendercore-frame-attribution-20261002/`. `arrival-high` is the
camera/resolution-matched capture; earlier attempts are explicitly classified in
`identity-and-limitations.json`. Offscreen SDL clamps to 1024x768; RenderDoc on
this installation does not expose Wayland Vulkan surfaces. The valid capture
uses X11 in the existing private compositor with `-noborder`, preventing the
decoration resize to 1920x1043. No login-session window or display was changed.
RenderDoc per-draw duration counters repeat expensive preceding durations on
some zero-invocation draws, so they must not be summed as independent draw costs.
Invocation counts and shader/binding identities are retained separately from
the engine's pass timing windows.

Replay of that capture exposes `KHR_pipeline_executable_properties`, connecting
the compiler statistics to the actual PBR pipelines. The static-model pipeline
22523 (92 draws, 1,941,530 fragment invocations) allocates 192 VGPRs, has 59,576
bytes of machine code and 11,087 static instructions. Major world pipeline
22509 allocates 192 VGPRs, has 56,732 bytes of code and 10,612 static instructions.
Both use 64-lane subgroups, report a maximum eight subgroups per SIMD and no
register spills or scratch allocation. These are driver compiler statistics
from replay, not measured dynamic occupancy or instructions executed per pixel.
See `pipeline-statistics.json`, `disassembly-high.json`, and the corresponding
pipeline executable text for the full mapping. This closes the earlier missing
draw-to-compiler-statistics link; it does not isolate register, instruction-cache,
texture or divergence stalls. The capture confirms a 4x-MSAA main target and
64 area-light records in the lit frame bindings; froxel masks are present, so
64 records must not be presented as 64 evaluated lights at every pixel.

### R91: Portal 2 core portal views (2026-10-02, active)

User request: “Let's get portals rendering on rendercore”. The user confirmed
this session owns portal work. This slice diagnoses and fixes the existing
portal submission, stencil aperture and nested-view handoff; material programs
and world passes retain their existing owners. The existing view-state lab
oracle is extended before product integration as needed. Concurrent shadow and
performance changes are preserved. R91/R96 and hard performance gates remain
open until the required image and timing evidence exists.

### K1/K4: render-core memory and cache audit (2026-10-02)

User request: check rendercore caching and memory use on the 128 GB unified-memory
workstation. This slice owns resource-cache allocation-failure recovery in
`render.resources` and its existing conformance suite. Existing shader, shadow,
material and profiling edits in the shared checkout are outside this slice.
The audit covers VMA placement, uploads, pipeline reuse, transient pooling and
frame graph reuse; no new cache registry or blanket desktop/mobile budget is
introduced. R86/R88 performance acceptance remains open pending measurements.

The user additionally requested fewer copies and general core optimization.
`render.device.v2` D25 now owns initialized, immutable upload-buffer creation;
Vulkan, GL and null implement the same snapshot and failure semantics. The
texture cache consumes it, deleting its encoder buffer-write and intermediate
GPU buffer-copy path in the same change. An allocation failure retains the
pending pixels rather than silently dropping them. Vulkan fills its coherent
mapping before the buffer is published; queue submission publishes those host
writes, and the existing completion-token retirement protects GPU readers.
There is no new mapped-write API for overwriting buffers already in flight.

Observed on this workstation (Radeon 8060S / RADV STRIX_HALO, Mesa 26.2.3):
125 GiB physical RAM, about 62 GiB available at inspection, nearly 8 GiB swap
occupied. Vulkan advertises a separate 83 GiB device-local heap and 41.5 GiB
non-device-local heap, with host-visible device-local memory types. These are
observations, not an engine memory allowance or a claim that all physical RAM
is available to the renderer. The driver budget changes with system activity.

Audit findings and remaining work (R86/R87/R88, no gate closure):

- VMA already suballocates buffers/images; upload ranges retire by completion,
  readback prefers cached memory, material surface pipelines cache by variant
  and debug specialization, and graph transients pool by shape/usages.
- The new texture path removes a full payload-sized GPU buffer transfer and
  its ring range/dedicated spill allocation. It still snapshots/repackages CPU
  mip bytes and retains one initialized staging buffer until completion. It
  does not claim CPU zero-copy or zero-copy optimal-tiled images.
- The adapter's 4 MiB ring remains appropriate to measure for *remaining*
  buffer traffic after this removal. Increasing it without measured spill
  sizes would reserve memory without addressing this redundant transfer.
- `MemoryAllocator` counts allocation bytes, but does not enable
  `VK_EXT_memory_budget` or publish heap usage/budget telemetry. Named texture
  and mesh caches have explicit eviction but no byte-budget/pressure policy.
  Measure these before selecting a larger workstation retention budget; do
  not hardcode a fraction of 128 GB into portable or mobile defaults.
- `Renderer::EndFrame` still builds and compiles the graph each frame. Compiled
  structure reuse requires separating shape from frame-owned callback captures
  and imports; retaining last frame's callbacks would be incorrect.
- Surface pipelines are cached in the material owner. Vulkan pipeline creation
  passes `VK_NULL_HANDLE` for its optional driver pipeline cache; adding a
  device-owned cache is a separate measured startup/variant-creation slice,
  not proof that current draws recompile pipelines each frame.

The direct texture upload is proven in `render_lab` before any new product
plumbing. All existing cache consumers share it automatically. No frozen render
path was edited. Full-frame High performance acceptance, device/mobile timings
and pressure-driven residency remain unverified.

Validation and reproduction:

| Check | Result |
| --- | --- |
| `render.resources`, release | 35 checks pass; old implementation fails the new allocation-failure regression |
| `render.device.v2.null`, `.vulkan`, `.gl`, release | 531 / 1,065 / 1,014 checks pass; positive Vulkan/GL validation reports zero messages (the seeded missing-barrier control intentionally reports a hazard) |
| `render.device.v2.sensitivity`, release | 17 checks pass, including a new zero-filled-upload defect rejected by D25 |
| `render.graph.v1`, `render.material.programs`, release | 84 / 31 checks pass |
| Existing Waf `render_lab` product | Builds in `build-rc-lab`, preserving its configured profile |
| `render_lab suite lightmap-basis --validate`, `suite posed-model --validate` | 19 / 65 checks pass |
| Architecture check/baseline/inventory; style diff | Pass; inventory reports line moves only, no baseline rewritten |
| Architecture/style checker fixtures | 166 / 38 tests pass |

Evidence and benchmark source/binary: `quality-results/rendercore-memory-2026-10-02/`.
The copied conformance JSON retains its original `/tmp` log paths; matching log
directories are mirrored beside the reports. `benchmark-identity.json` records
the revision, dirty-diff digest and binary/source hashes.

The upload microbenchmark alternates old and new paths on the same device, with
four warm-up iterations and 30 measured iterations per path. Each uploads a
2048×2048 RGBA8 texture (16 MiB). GPU timestamps bracket the transfer sequence;
CPU elapsed time includes staging creation, initialization, submission and the
completion wait. Texture allocation and readback verification are outside the
timed region. Every iteration changes the payload and verifies all bytes. A
seeded omitted texture copy fails readback with exit 4.

| Path | Creation-to-completion median | GPU median / p95 | Dedicated ring spills |
| --- | --- | --- | --- |
| Previous ring → staging → image | 3.1511 ms | 0.2918 / 0.3231 ms | 30 |
| Initialized staging → image | 1.4657 ms | 0.1617 / 0.1672 ms | 0 |

This final run saves about 53% of elapsed upload cost and 45% of GPU transfer
time for this workload. An earlier run was faster in absolute terms on both
paths; this shared workstation is not an isolated performance runner. The
structural saving is one 16 MiB GPU transfer and one 16 MiB spill allocation per
benchmark upload. The CPU mip-packing copy remains. No full-frame FPS claim
follows, and no quality setting, effect, resolution or sample count changed.
Native Vulkan device-loss injection remains unavailable (D7); null covers that
clause. Fold7, Apple and Android native runs were not performed in this session; their
measurements remain unavailable here.

```sh
python3 tools/quality/conformance.py check --config release \
  --suite render.resources --suite render.device.v2.null \
  --suite render.device.v2.vulkan --suite render.device.v2.gl \
  --suite render.device.v2.sensitivity --suite render.graph.v1 \
  --suite render.material.programs --out quality-results/rendercore-memory-repeat.json
WAFLOCK=.lock-waf-rc-lab-main ./waf build --target=render_lab -j4
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite lightmap-basis --validate
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite posed-model --validate
g++ -std=c++20 -O2 -DNDEBUG -Ipublic \
  quality-results/rendercore-memory-2026-10-02/upload_bench.cpp \
  build-rc-lab/render/device/vulkan/librender_device_vulkan.a \
  build-rc-lab/render/device/librender_device.a -lSDL3 -lvulkan -pthread \
  -o quality-results/rendercore-memory-2026-10-02/upload_bench
quality-results/rendercore-memory-2026-10-02/upload_bench
quality-results/rendercore-memory-2026-10-02/upload_bench --skip-copy # expected exit 4
```

Design references: Khronos's [memory allocation guide](https://docs.vulkan.org/guide/latest/memory_allocation.html)
explains UMA's host-visible/device-local types; VMA's
[budget guidance](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/staying_within_budget.html)
and [statistics](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/statistics.html)
describe the heap-budget telemetry still needed. The owning code/contracts, not
these references, establish what is installed in this engine.

The user additionally authorized reuse: “you can re-use the legacy shader”,
with the requirement to execute at the correct point in the frame. The existing
PortalRefract stage 0/2 shaders remain one native implementation, retained only
in the normal product frame by `kCorePassCustomEffects`; stage 1 remains the core
aperture. The ordered stream owns the interleave and marks only the most recent
framebuffer copy consumed by each retained refraction draw. Diagnostics keep
legacy suppression. Retirement is replacement of these two stages by the owning
core portal pass, with deletion of the native programs when no caller remains.

Frozen-path: user request 2026-10-02 — reuse the existing custom portal effects
at their original frame positions beside the core stencil/view draws.

### K1: constant-time upload retirement bookkeeping (2026-10-02)

User request: continue optimizing the core, most obvious work first. This slice
owns `render.device.vulkan` upload-ring lookup and its native conformance tests.
The current Submit/Abandon loops visit all live ranges for each upload, although
allocation IDs are consecutive and retirement only removes a prefix. Replace
those scans with an index into the existing deque; keep allocation placement,
fence values, close/abandon behavior and storage unchanged. Material, shadow
and portal work elsewhere in this shared checkout is outside this slice.

Implemented `FindLocked`: unsigned distance from the first live ID indexes the
existing deque, with an empty/range check. Both Submit and Abandon use it under
the existing mutex. No hash table, secondary registry, extra allocation or
additional retained memory was introduced. Batch bookkeeping is O(N), replacing
O(N²); allocation placement, overflow behavior and GPU retirement are unchanged.

Nine native-suite checks exercise out-of-order submission, prefix retirement,
physical wrap, inert old/future IDs, refusal to abandon a submitted range, full
drain, and detach/reattach without ID reuse. Existing D10 concurrency and
held-queue readback tests continue to cover GPU range lifetime; the intentionally
early-reusing Vulkan provider is still detected.

Evidence: `quality-results/rendercore-ring-2026-10-02/`. The CPU benchmark builds
the saved pre-change implementation and current implementation against the same
harness. It uses 64-byte ranges, reverse-order Submit/Abandon calls, four warm-up
iterations and 20 measured iterations per case. It first creates and joins a
thread, so measurements use the mutex path of a multithreaded process. Allocation
and retirement are outside the timed bookkeeping region; full drain is verified
every iteration. These are uncontended CPU microbenchmarks on a shared machine,
not measured frame gains or a controlled performance gate. A seeded lookup that
always chooses the first slot fails the benchmark's drain check (exit 2).

| Live uploads | Submit median before → after | Abandon median before → after |
| --- | --- | --- |
| 64 | 3.035 → 0.762 µs | 2.605 → 0.732 µs |
| 1,024 | 467.960 → 11.823 µs | 345.880 → 11.401 µs |
| 8,192 | 22,319.716 → 94.287 µs | 22,350.885 → 90.971 µs |

Validation:

- `render.device.v2.vulkan`: 1,074 checks pass, including nine new ring checks.
- `render.device.v2.vulkan.sensitivity`: 21 checks pass, including the premature
  upload-reuse negative control. Positive validation reports zero messages;
  the existing intentionally missing host barrier emits its expected hazard.
- Existing configured Waf `render_lab` builds; `suite lightmap-basis --validate`
  passes 19 checks.
- Architecture `check --all` passes. Scoped style checks pass. The committed
  shared-checkout diff against `20c101df3` also contains five unrelated formatting
  failures in ongoing material/debug/view work (`style-committed.log` lists the
  paths). This slice leaves those files alone.
- Native forced Vulkan device loss remains unavailable (D7, covered by null).
  Fold7/mobile and complete-frame timings were not measured; this slice does
  not close R90/R95/R96/R91's full-frame performance acceptance.

The shared checkout was committed as `1177eede9` during validation, including
this slice's code before its final style/evidence updates. That is why the
ordinary HEAD style check initially reported no eligible changes; validation
was repeated on the committed diff and on the slice's source files explicitly.

Reproduce the native checks and lab with:

```sh
python3 tools/quality/conformance.py check --config release \
  --suite render.device.v2.vulkan --suite render.device.v2.vulkan.sensitivity \
  --out quality-results/rendercore-ring-repeat.json
WAFLOCK=.lock-waf-rc-lab-main ./waf build --target=render_lab -j4
LD_LIBRARY_PATH=build-rc-lab/tier0 build-rc-lab/render/lab/render_lab suite lightmap-basis --validate
```

`ring_bench.cpp`, `upload_ring_before.cpp`, both benchmark binaries and their
output live in the evidence directory. Each binary builds with
`g++ -std=c++20 -O2 -DNDEBUG -pthread -Ipublic -Irender/device/vulkan`, the harness,
and either the saved implementation or `render/device/vulkan/upload_ring.cpp`.
`identity.json` records source/binary hashes and the checkout revision.

Next candidate from the audit: `GroupResidency::Refresh` allocates and fills two
temporary vectors before it discovers a material's texture bindings are already
current. Measure allocation-free hit validation there before attempting broader
graph caching or increasing global memory budgets.

### R91: on-wall portal exit visibility correction (2026-10-02)

The user's follow-up image showed the animated rims but wall pixels inside the
apertures. The earlier product proof had moved the exit eight units away from
its wall; it did not cover this case. The core world surface pipeline disabled
face culling, so the virtual exit camera drew the wall's back face within the
client's intentional two-unit clipping tolerance. The material resolver now
exposes authored `$nocull`; the world pass applies back-face culling consistently
to its color, depth-only and depth/normal passes, preserving two-sided materials.
Dynamic portal masks keep their existing ordered stencil and raster behavior.
No portal shader copy or clip-plane tolerance change was added.

The new native-Vulkan lab test failed before the correction (1/16 failures),
then passed 16/16 with validation silent, including a `$nocull` negative control.
GTAO passed 13/13 and panels 44/44, exercising the neighboring world passes.
The complete `build-p2` build passed. The isolated installed game capture at
`quality-results/portal-core-20261002/on-wall/evidence.json` places the exit at
its authored wall (`portal_place 0 1 511.811 0 55.118 0 90 0`): the linked room
and rim are visible together. This replaces the off-wall capture as the proof
for exit-wall visibility. The capture is 640x480, not High acceptance evidence;
R91/R96 and the complete-frame performance/platform gates remain open.

Final receipts: `on-wall-conformance.json` records 41/41 world contract checks
and 16/16 portal lab checks. Native Vulkan bring-up passes 124/124, including
the retained portal effect/copy ordering controls. The on-wall game reports
1,785 views drawn, zero failed views, and 1,170 dynamic draws with zero refusals.
Architecture check, baseline and inventory verification pass. Changed portal
lines pass style; the shared-tree style run still reports unrelated concurrent
edits in `debug_controls.cpp`, `pass_timers.cpp` and `debug_overlays.cpp`.

### R91: retain the fizzler at its ordered core-frame slot (2026-10-02)

User request: “do the same for the fizzler”, following the explicit request to
reuse custom legacy portal shaders at their original frame positions. The
existing SolidEnergy shader remains the sole owner of the fizzler's flow,
opacity and live FizzlerVortex parameters. The composition's product-only
`kCorePassCustomEffects` marker (renamed from the portal-specific marker) now
retains SolidEnergy alongside PortalRefract stages 0/2. The frontend bypasses
core-only rejection for that family, and replay retains the captured draw at
its original slot with its blend, depth, stencil and clipping state. It does
not retain framebuffer copies for SolidEnergy. Pixel/legacy-skip diagnostics
still suppress it; ordinary legacy shaders remain filtered. This also preserves
SolidEnergy's existing bridge/beam consumers without a material-name registry.
Retirement remains replacement by a core-owned effect pass and deletion of the
native shader once its remaining callers migrate. No new shading math was added.

Frozen-path: user request 2026-10-02 — reuse the existing fizzler shader and its
ordered draw state beside the core scene, as with the portal effects.

The native replay regression failed four product/replay checks before the
retention fix. Afterward, native Vulkan bring-up passes 137/137 with required
validation, including independent alpha-blend pixels, a later-slot overwrite
control, product/diagnostic/product reset, depth occlusion and clip-plane tests.
The complete `build-p2` build passes. Product evidence is under
`quality-results/fizzler-core-20261002/{after,suppressed}/evidence.json`: the
stock `sp_a2_fizzler_intro` view submits all three SolidEnergy field surfaces;
the diagnostic control submits none. These 640x480 captures expose existing
unclaimed backdrop materials and models, so they establish the field handoff,
not full retail-image parity or the High performance gate.

This stock-map check also exposed the preceding portal culling fix's legacy
BSP adapter gap: Source's brush fan winding was passed unchanged into the
counter-clockwise core world convention. `RenderCoreWorldDraw_LevelInit` now
reverses those imported triangles once; WMSH import is unchanged. This restores
visible front faces without disabling the exit-wall back-face test. Existing
material-family gaps stay reported rather than hidden by a legacy fallback.

Final fizzler checks: composition 54/54 (`fizzler-core-20261002/composition.json`),
portal view-state 16/16, native replay 137/137 (`native-final.log`), architecture
check/baseline/inventory, changed-line style and `git diff --check` all pass.
High-resolution complete-frame timing and non-Linux acceptance remain unverified.


## RFC 0014 cost scope and resource churn breakdown (2026-10-02)

User scope: split the large world-view cost in the VGUI overlay and add render
resource allocation metrics with a history that exposes churn. This session
extends the existing timer/overlay owners; no new lighting implementation.
World-pass labels separate preparation, world program families, static/posed/
transmitting models and dynamic draws without reordering or removing draws.
Vulkan supplies optional logical resource-handle activity through the device port;
other adapters explicitly report it unavailable. Created handles by kind, release
requests, completed destruction, requested buffer bytes, live handles and pending
retirement feed per-scope rows and two 64-interval histories. These exclude CPU
heap calls, physical VRAM and internal driver/staging allocation. RFC 0014's
[measured overlay](0014-native-vulkan-and-bsp2-debug-controls.md#measured-cost-overlay-installed-2026-10-02)
owns presentation and interpretation. `cl_render_debug_cost_page` exposes rows
that do not fit on one page.

The native rollover fixture first failed with two Vulkan validation messages:
a new timestamp chunk emitted its initial buffer barrier inside dynamic rendering.
The adapter now hoists only copy-destination-only readback-buffer transitions
outside rendering, retaining the previous-use dependency. The fixture now passes
23/23, including allocation failure, duplicate release, release versus destruction,
requested bytes, epoch discontinuity, repeated labels across chunks and bounded
non-destructive churn history. Product capture also exposed timer cleanup being
conditional on a shadow device: a legacy BSP view with timers could retain a dead
device until composition teardown. Releasing timers before that condition fixes
the reproduced shutdown crash; the subsequent isolated product capture exits cleanly.

Verification: `render.graph.v1` 89/89, `render.world.null` 66/66 and
`render.composition` 58/58, recorded in
`quality-results/conformance.20261003T014411Z.json`. Existing `build-rc-lab` and
`build-p2` Waf configurations build without reconfiguration. Native cost suite,
build/style/architecture logs and the timer regression patch are retained under
`quality-results/core-cost-split-verification-20261002/`. The product capture is
`quality-results/core-cost-split-20261002-final/evidence.json`; its PNGs were
visually inspected. Architecture check/baseline/inventory and changed-line style
pass. Fold7 and other native platforms were not run; their acceptance is unverified.

User-selected benchmark: `sp_a1_intro4_probe64`, using the installed
`portal2-intro4-perf-v1` workload at native compositor 1920x1080 and the declared
High settings. The diagnostic run is
`quality-results/core-cost-intro4-probe64-profile-20261002/` (including parsed
`profile.json`/`profile.tsv`). Its final reporting window attributes 23.214 ms to
static PBR models, 19.046 ms to world PBR surfaces and 10.861 ms to posed PBR models;
GTAO is 1.971 ms. These are inclusive window means, not phase percentiles.
The loader confirms the probe64 asset and 64 reflection probes, but the script's
map-name check reports `sp_a1_intro4`; all three camera checks pass. That mismatch
is retained as a failure, not hidden by changing the oracle. Hard render budgets
remain failed; neither the overlay nor this work claims a performance promotion.
The final run without profiling and with no concurrent session builds/tests is
`quality-results/core-cost-intro4-probe64-final-20261002/evidence.json`.
It records 561 measured frames over 20.2 s, 27.8 average FPS, 22.67 ms median,
60.34 ms p99 and 124.35 ms maximum; arrival/reverse/return medians are
58.26/22.35/58.74 ms. All 561 frames miss the 120 FPS floor. The profile/camera
checks are retained separately from the failed map-name assertion. No effects,
resolution or sample count were reduced to obtain these results.


## K5: completion-safe world group resource reuse (2026-10-02)

User scope: reduce render-core resource churn, copies and repeated allocation,
including the game's native core. `render.pass.world` remains the sole owner of
its groups. Its private `GroupResources` retains up to 256 idle/pending group
buffers within 64 MiB and interns up to 64 sampler descriptions. These are cache
bounds, not draw limits: overflow uses ordinary allocation and fenced release.
The cache does not assume the desktop's 128 GiB is available on other profiles.

Retired constants/storage become available only after the actual covering
completion token. Exact size and usage preserve descriptor ranges and shader
array lengths; the prior buffer usage is retained for the next upload barrier.
One completed token certifies earlier submissions only on the same queue and
in the same epoch. This avoids repeated timeline queries for one retired batch.
Borrowed GPU assignment buffers never enter the pool. Same-frame views remain
separate, frame zero waits for teardown, and bind groups still retire behind
their original tokens. Samplers remain owned until drained device teardown;
full descriptor equality includes comparison, filters, address and anisotropy.
Transient group retirement now moves the group's vectors instead of copying them.
Device replacement also drops all retired handles from the old device.

Evidence: `quality-results/rendercore-reuse-2026-10-02/`. The Vulkan microbenchmark
alternates fresh creation and reuse, with ten warm-ups and fifty measured trials
per mode. Each synthetic view requests six buffers (128, 1024, 4096, 65536, 64,
16 bytes) and five samplers, uploads all buffer contents, submits, waits, then
retires/polls. It measures this resource workload, not drawing a complete scene.
`bench-repro.txt`, source, logs and identity are retained for reproduction.

| Workload | Fresh allocation p50/p95 | Reuse acquisition p50/p95 | Fresh total p50/p95 | Reuse total p50/p95 |
| --- | --- | --- | --- | --- |
| 1 view | 1.974 / 2.384 us | 0.441 / 0.541 us | 50.315 / 82.014 us | 45.115 / 113.563 us |
| 32 views | 50.825 / 72.677 us | 1.814 / 3.196 us | 759.417 / 870.877 us | 571.544 / 678.956 us |

The 32-view workload creates zero new buffers after warm-up versus 9,600 across
fifty fresh-allocation trials. Its acquisition median falls about 96%, and total
median about 25%. The one-view total p95 is worse in this run; no tail-latency or
whole-frame improvement is certified. A separate validation-enabled run reports
zero Vulkan validation messages and zero leaked port resources. Complete High
120 FPS, game-route timings, mobile pressure/power and non-Linux evidence remain
open. Descriptor-group recreation and repeated uploads remain follow-up work.

Validation uses W18/W19 in `render.world.null` (66 checks pass in
`world-final.json`): delayed completion, exact shape,
state transitions, later submissions/other queues, bounded count and bytes,
oversized allocation, sampler overflow, failure and drained teardown, two views
in one frame, unknown frame serials and borrowed GPU buffers. A seeded helper
which ignores completion fails three lifetime assertions (`negative.log`). Native
`render_lab suite posed-model --validate` passes 66 checks, including full pixel
equality after reusing a view; `view-state --validate` passes 16 portal/stencil
checks. Native device conformance passes 1,074 checks. Existing lab and game
`shaderapivulkan` Waf profiles build successfully without reconfiguration.
Architecture check, baseline and inventory verification pass. The new private
header and final changed-line check pass pinned style. Earlier broad style
reports include concurrent cost-overlay edits (logs retained). A missing aggregate
initializer in that timer work was supplied so the strict Waf build could proceed.

No render quality, shader, effect, resolution, sample count or frozen-path behavior
was changed. R89/R96 resource efficiency improves; their larger acceptance gates
and RFC 0016's hard render budgets are not certified by this microbenchmark.

## RCV-11: complete-view preparation and opaque batching (2026-10-03, bounded implementation)

**2026-10-03 scope clarification:** the user requested completion of opaque-view
batching in a worktree, then directed: "Do not focus on rendercore too much. The
main gains are the pipeline as a whole in game. That's what matters. Update docs
to say that." The owning [in-game pipeline performance rule](0016-render-core.md#in-game-pipeline-performance-user-decision-2026-10-03)
now makes complete gameplay-frame cost the optimization objective. Continue from
the retained shared-view work below, trace cohort ordering through the game and
legacy/native bridges, and judge the result with matched full-frame product
measurements. Lab checks remain the core correctness prerequisite. This
clarification adds no performance evidence and closes no gate.

The completion work is isolated in
`/home/john/.codex/worktrees/opaque-view-batching/source-engine`, branch
`codex/opaque-view-batching`, based on `d117ea50b`. `render.pass.world` owns opaque
draw preparation/execution, `render.composition` owns integration and ordering,
and the native adapter owns command translation. The existing shared-view branch
and the main checkout's concurrent screen/panel changes are retained separately.

The bounded implementation now batches compatible world/static/posed tickets
through the actual game stream, shares their preparation and early depth, and
preserves color order, target/state boundaries and capture replay. Material policy
stays in the resolver. The [implementation and game evidence](0016-opaque-batching-2026-10-03.md)
records identical-binary off/on/on/off measurements: arrival median frame interval
34.623/36.380 ms off versus 30.199/29.549 ms on, with unchanged High/4x MSAA
settings. Lab pixel oracles and gameplay screenshot pairs cover correctness.
This improves the in-game pipeline; the hard 120 FPS gate and broader R96
content/view acceptance remain open. The following planning record describes the
preceding shared-view work; its pending-batching statements are superseded by
this implementation record.

User direction: checkpoint current progress and plan, work in a separate worktree,
and pursue the next substantial complete-frame improvement. RCV-09/10's retained
depth change is committed on the main branch at `658180d76`; its ordinary matched
arrival control/result is 55.784/37.603 ms GPU and 56.129/48.327 ms frame interval,
with retained CPU time 41.281 ms. These are diagnostic improvements, not High
120 FPS acceptance. The existing resource pool is already implemented; this slice
must eliminate repeated preparation rather than add another allocation pool.

Ownership stays with `render.pass.world` for view/group preparation and opaque
draw execution, `render.composition` for ordered boundaries, and the native
adapter for translating their commands. No new lighting algorithm or quality
selection is planned. Source, outputs and runtime binaries remain private to the
new worktree; the saved RCV-09/10 control and all 29 runtime library identities
remain available for interleaved measurements.

1. Inspect actual cohort inputs and instrument/count repeated group construction,
   uploads and command work. Reuse immutable view lighting and compatible groups
   across cohorts only with explicit input identity and correct submission-token
   retirement. Prove changed lighting, targets, clipping, AO, scene captures and
   nested views cannot borrow stale resources, including delayed completion.
2. Gather compatible opaque world/static/posed cohorts before their depth and
   color draws, within existing ordering boundaries. Preserve alpha coverage,
   clipping, stencil, transparency and scene-color capture semantics. Prove the
   change in `render_lab` before product integration; no later cleanup of a
   replaced implementation is deferred.
3. Measure remaining geometry conversion and command emission, then remove
   repeated copies and bindings where the complete-frame profile justifies it.

Each retained implementation needs relevant null/native correctness and lifetime
checks, architecture/style checks, complete High/4x MSAA product image evidence,
and ordinary matched route timings with binary/source identities and AC/power/
clock records. Report fragment invocations, resource/upload/command reductions
where available. Checkpoint code and evidence at bounded milestones; revert
ineffective trials rather than retaining unmeasured mechanisms. Desktop/Fold7
and the hard render gates remain open until their required evidence passes.

### Shared view bindings: retained result (2026-10-03)

Work is isolated at `/home/john/.codex/worktrees/view-batching/source-engine`,
branch `codex/view-batching`, with private dependency pins, independent Waf locks
and separate product/lab outputs. The benchmark runtime and frozen map are copied
to `/home/john/.rc-view-31885` and this worktree respectively. No shader, light,
shadow filter, resolution, sample count or accepted effect changed.

The composition already shares immutable lighting snapshots. The world pass now
shares their uploaded view groups across cohorts within one frame and CPU stream,
keyed by layout, retained snapshot identity, shadow atlas, AO and reflection input.
Ordered scene-color groups remain transient. Retention is bounded at 256 groups;
unknown frames and overflow keep the existing fenced transient path. Replaced
groups retire behind the actual covering submission token, not a fixed delay.
The snapshot's immutability contract lives in `StageViewLights`' owner header.

Ordinary High/1920x1080/4x MSAA ABBA comparison, milliseconds:

| Run | Arrival GPU | Arrival CPU | Arrival frame interval | Return interval |
| --- | ---: | ---: | ---: | ---: |
| `control-a1` | 37.918 | 41.325 | 48.181 | 47.845 |
| `shared-b1` | 33.296 | 25.409 | 33.648 | 33.304 |
| `shared-b2` | 33.395 | 24.946 | 33.778 | 33.770 |
| `control-a2` | 38.246 | 40.688 | 48.644 | 48.750 |

Mean of arrival phase medians: 30.36% lower interval (43.60% higher frame rate),
38.60% less CPU time and 12.44% less GPU time. All four complete runs stay on AC
near 70 W; whole-run median GPU clocks are 2051/2076.5/2043/2010 MHz. These remain
matched diagnostic comparisons, not acceptance: the pre-existing map-name
assertion and High 120 FPS floor still fail. Ordinary-run analysis explicitly
lacks detailed GPU scope timers; its complete-frame metrics are retained.

The freshly rebuilt control launcher exactly matches the earlier retained SHA256
`e5d71f5a76db6f9ed82ee7711ca3ac9a6b415d626516f3159ba233e0690bcc04`.
The measured shared variant is
`ab89419139a678b7662adbfa21af3352d6090d08ac4dcc415b35472f4aa25c8c`.
All 29 staged libraries are hashed before/after each route and have private
inodes; source identities and power logs accompany every run under
`/home/john/.rc-view-31885/evidence`. Private libraries are read-only and staged
by atomic replacement. No ordinary build can overwrite these outputs.

Native posed-model conformance passes 72 checks, including exact pixels for
separate same-frame cohorts sharing lighting. Null world conformance passes 76
checks including zero repeated storage uploads, changed snapshot/screen/stream
inputs, unknown frames, bounded overflow without dropped draws, delayed GPU completion
and leak-free teardown; composition
passes 58. Architecture check/baseline/inventory and changed-line style pass.
`shared-image/evidence.json` passes native Vulkan/SDL3 screenshot smoke at the full
1920x1080 High image with 4x MSAA; its image was visually inspected. This is not
a new image golden or complete-cohort certification. Fold7 remains unavailable.

Negative control: omitting lighting snapshot and AO from the cache key fails three
world checks (changed snapshot, changed screen input and overflow). Correct source
is restored and the final positive run is recorded in `conformance-retained.json`.
Native view-state conformance also passes 16 checks. These reports, build and
style logs live in the private worktree's `quality-results/view-batching-20261002`.

The byte-identical frame-constant upload experiment remains in checkpoint
`394ea55c1` and is removed from the retained implementation. Its two ordinary
arrival medians were 33.617/33.500 ms, versus 33.648/33.778 ms for shared bindings;
this establishes no meaningful additional complete-frame gain. A later shared
repeat (`shared-b3`) reached 102 C and 1047 MHz and is excluded as a whole run,
with the thermal reason and original records retained in `excluded-runs.json`.
The original four-run AC comparison above remains the measured result.

RCV-11 remains in progress. Complete opaque-view batching is still planned and
follows only across proven ordering boundaries; it must preserve the existing
per-cohort material order, including equal-depth overlaps. After this CPU
reduction, the measured arrival is GPU-limited (about 33 ms GPU versus 25 ms CPU).
The High 120 FPS performance gate and Fold7 evidence remain open.


### R91: fizzler light emission (2026-10-03)

User request: reuse the fizzler shader and make it emit light; implement in the
current checkout with worktrees off. `render.energy-field.v1` in
[`public/render/energy_field.h`](../public/render/energy_field.h) owns the
view-independent source. The retained SolidEnergy surface and `render_lab`
use the core's shared reveal/radiance GLSL. The old inline reveal/color block
was removed in the same change; opacity, clipping, blending, fade and the
original stream slot remain with the retained surface.

Frozen-path: core plumbing R91 — route the retained SolidEnergy surface's
intrinsic reveal/radiance through its core-owned definition; no receiving-light
algorithm is added to the frozen backend or CPU lighting path.

The engine supplies the largest authored rectangular SolidEnergy brush face,
its UVs and tangent frame. The cleanser supplies the live entity transform,
flow textures/settings, intensity, power-up and vortex objects. State advances
once per frame so emission and multiple proxy/view calls share the same pulse.
The source uses the shader's `Plat_FloatTime` clock rather than simulation time.
It integrates a 16x16 sample grid to uniform linear radiance before camera
opacity, fade, exposure or output clamping, then publishes one two-sided area
light through the existing client selection and runtime light set. The core's
LTC, clustering and shadow consumers evaluate it. Missing textures, unsupported
geometry/model-format/vertex-color flow and invalid inputs refuse emission by
name; no collision-bound or point-light substitute is inferred.

`VEngineAreaLights003` adds explicit core-only receiver policy and the geometry
query. `VEngineAreaLights002` remains exposed with its original vtable and
behavior. Core-only sources allocate no CPU stand-in dlight slots. Disabling
follows the surface's power-down reveal until the light reaches zero. Hiding or
destroying the field removes its source immediately; registration and the
retained material reference are released by the entity's destructor.
`cl_fizzler_core_emission 0` is a lighting-only diagnostic control;
`cl_fizzler_core_emission_report 1` prints the authored emitter and live radiance.

Evidence is retained under `quality-results/fizzler-light-20261002/` (the
work began on October 2; final runs October 3):

- `lab-final.json`: intrinsic GPU/CPU oracle 527/527, seeded reveal/intensity
  sensitivity 3/3, diffuse/GGX area receiver pixels including the new fizzler
  source 72/72; shared area/light-set contracts 38/38 and 41/41, existing
  shadowed-light suite 153/153. Native Vulkan validation is silent in lab.
- `native-offscreen-final.log`: native retained-stream replay 137/137 with
  required validation, including ordered effect, clipping, depth and blend
  controls. Both the complete `build-p2` product and `build-rc-lab` lab builds
  pass using their existing Waf configurations (`*-build-final.log`).
- `fixture-final/receivers.json` and `capture/evidence.json`: generated sealed
  room and constant flow texture, 640x480 native Vulkan/SDL3. Emission off/on,
  off/on again and entity Disable/Enable each raise the receiver's mean blue
  by 4.310 byte levels (required >2); source-off returns exactly (required
  mean absolute difference <0.5). This fixture uses intensity 10, the existing
  portal-shot peak, and does not lower its receiver threshold. The source's
  area is 16384 square units, full-power radiance (0.8, 2.5, 8), two-sided;
  the engine reports `0 lit (1 without a slot)`. The nine-check product fixture
  is installed as `render.product.fizzler-light` in the shared manifest and
  passes 9/9 through the shared runner (`product-registered.json`) and after
  the power-down correction (`product-powerdown.json`); run it with
  `python3 tools/quality/conformance.py check --suite
  render.product.fizzler-light --out <fresh-output>/result.json`.
- `stock-settled/evidence.json`: stock `sp_a2_fizzler_intro` boots on the core;
  `effects/fizzler_center` publishes area 49152 at full power, radiance
  (0.003320, 0.010625, 0.013282), reach 230.643, intensity 1. Stock wall-clock
  flow and existing unclaimed backdrop cohorts prevent using this capture as
  an isolated receiver comparator. The generated fixture supplies that oracle.
- Architecture check/baseline/inventory, 166 architecture fixtures, 38 style
  fixtures, 74 conformance-runner fixtures, pinned changed-line style and
  `git diff --check` pass.


The broader compiler-dependency audit is **not passing**:
`python3 tools/archlint/archlint.py check --all --compile-deps build-p2
--compile-deps build-rc-lab` judges 938/438 strict units and reports 58 CAP005
errors, all on existing generated `render/shaders/generated/spv` headers in
cluster, line, output, shadow and shader-library consumers. The installed checker
classifies every `build*` path as external even though the manifest assigns
`spv/` to `render.shader-library`. None names the new energy-field source or
shader. Ordinary architecture/baseline/inventory pass; this existing generated-
header classification gap remains separately open (`compile-deps-final.log`).

This completes the requested retained-shader/direct-light first slice, not R91.
Uniform mean radiance loses spatial variation across the source. The controlled
product view proves receiver response/state removal; it does not certify the
complete field image, moving-occluder shadows or stock-content pixel parity.
The lab covers two-sidedness and the existing shadow receiver mechanism. Full
SolidEnergy surface migration, GI, High complete-frame budgets, CPU/GPU source
integration crossover measurements and non-Linux native evidence remain open.


### R91: visible fizzler light at idle (2026-10-03)

User request: “can we make it emit some light when its on and not being hit with
the portal gun”. The existing source publishes radiance at idle, but the stock
flow texture's mean is very dim (about 0.013 blue before the new strength).
The client-owned `cl_fizzler_core_emission_strength` now defaults to 16 and
multiplies the core light source's output intensity. It leaves the retained
surface shader and ordinary gameplay intensity 1 unchanged. The portal-hit
pulse retains its 10:1 intensity ratio; power-up/down, flow masks, vortex color,
visibility and core-only receiver policy keep their existing owners. Zero
strength gives zero emitted light. This is product radiance tuning on the
existing source, with no new shader, light kind, pass or frozen receiver shading.

Lab proof: `quality-results/fizzler-idle-20261003/lab.json` passes 529/529 and
sensitivity 3/3, including the added idle-strength and portal-hit-ratio checks.
The existing lab configuration and complete `build-p2` product build pass
(`lab-build.log`, `product-build.log`).

The product fixture now uses ordinary idle intensity 1 instead of the preceding
fixture's pulse-peak intensity 10. The source report must explicitly show
`intensity 1.000 powerup 1.000 strength 16.000 shot 0.000`. The shared manifest
raises its minimum to 10 checks; `product.json` passes 10/10. Retained receiver
metrics/capture data are in `quality-results/fizzler-idle-20261003/fixture/`:
three on/off and Disable/Enable comparisons each raise mean receiver blue by
6.1175 byte levels (required >2), and off returns exactly (required mean absolute
change <0.5). The source still takes no CPU stand-in slot. Reproduce with
`python3 tools/quality/conformance.py check --suite render.product.fizzler-light
--build-dir <fresh-output>/commands --out <fresh-output>/product.json`.

The stock `sp_a2_fizzler_intro` field camera also boots successfully
(`stock-field/evidence.json`). `effects/fizzler_center` reports full power,
intensity 1, shot time 0 and radiance (0.052516, 0.168050, 0.210063), confirming
idle emission on original content. The initial spawn-camera boot is retained
separately under `stock/`; that camera has no fizzler source report, so it is
not used as field evidence.

Architecture check, baseline and inventory pass, as does `git diff --check`.
The changed cleanser/lab lines pass pinned style. The complete changed-line
style invocation still reports the pre-existing formatting-only edit in
`public/render/area_light.h`, present before this request and left untouched
(`style.log`). The previously recorded compiler-dependency classification gap
and full-frame performance/platform gates remain open; no new performance or
complete-image claim is made.


### R91: elevator movies and world screens (2026-10-03)

User request: elevator videos and other screens on the render core, using
`sp_a1_intro4_probe64`, with ordinary world/glass interactions. The owner is
K8's [in-world panel contract](0016-render-core.md#in-world-panels-ui-cohort-amended-2026-09-30-user-request),
not a movie renderer. All recordable `CVGuiScreenPanel` consumers now take its
PBRMetalRough surface and tile-light source. Playback, grouping and authored UV
crops stay in the existing movie panel. The fixed movie mean radiance and its
separate registration are deleted; signs also use the base registration. Existing
movie/sign compatibility-receiver policy is retained; new screen sources are
core-only. The core surface owns depth, so its retired legacy WriteZ overlay no
longer draws a second copy. Custom overlay materials and unrecordable primitives
still have no core fidelity claim, and nested legacy views retain their existing
panel path.

Complete opaque CPU procedural RGB/BGR uploads publish a bounded 64x64 sampling
image from the regenerated pixels, before the matching GPU upload, using the
frozen `ITexture::GetLowResColorSample` API. Linear-light stratified means,
channel order, row padding, wrapping/clamping and texture-sheet padding have
explicit handling. Unsupported formats and partial scratch-image uploads clear
sampling instead of keeping stale pixels. The Bink FFmpeg regenerator clears its
unused texture-sheet padding. No GPU readback, movie light registry, receiver
shader or video-specific material family was added.

The PBR family now claims `$translucent`, with ordinary linear alpha blending,
depth testing without writes and destination-alpha preservation. Panel raster
coverage starts clear for transparent surfaces; level zero and mip reduction
apply coverage once. A proven full-face opaque foundation stays opaque despite
the legacy sorting flag. Opaque and alpha panels share a view in original draw
order. The shared family-pixel fixture's neutral shadow slot was corrected to a
valid D32 image; its previous RGBA stand-in triggered Vulkan depth-comparison
validation even in inactive shadow branches. The independent pixel references
are unchanged.

Frozen-path: `materialsystem/ctexture.cpp` only publishes procedural upload samples
through the existing texture ABI; the client drops the obsolete WriteZ draw only
when the core has taken ownership. No native-backend or frozen CPU receiver
shading is introduced.

Evidence is retained under `quality-results/core-screens-20261003/`:

- `render.lab.panel` passes 59 checks with validation, covering live replacements
  under one texture key, grouped UV crops and CPU tiles against GPU pixels,
  alpha 0/64/128/255 over a colored receiver, mixed opaque/alpha surfaces,
  disabled alpha depth writes and coverage mips, alongside the existing
  resolution, frame, coating and light oracles. Its sensitivity passes 4/4;
  the three seeded gamma/scatter/coating defects remain detected.
- `render.procedural-texture-sample` passes 20 release checks: RGB/BGR, padded
  rows, frame replacement, linear means, image padding, malformed extents and
  the full-face coverage proof. `render.family.pbr` passes 97 checks and the
  ignored-normal-map fixture is rejected. The existing `render.lab.posed-model`
  passes 72 validation checks, including the fractured-glass Refract claim and
  its required scene color and native probes. This is a regression check, not
  evidence of an actual shattering event in this map.
- `movie-final/evidence.json` passes on native Vulkan at the arrival elevator
  (`cmd setpos -1552 37 -96; cmd setang 0 -90 0`). Its core report records
  133 lists/rasters/views/panel draws, zero refused and zero failed. The
  console's `cl_world_panel_report` records `media/exercises_horiz` and the
  screen's changing, cropped tile radiance. The capture is
  `movie-final-0.png`.
- `movie-light-control/evidence.json` holds the game/video paused and tone map
  at 1, then captures `r_area_lights 64`, 0 and 64. The left elevator receiver
  region (150,230)-(355,520) rises by mean RGB (2.376,4.361,4.675) byte levels;
  the right region (650,230)-(860,520) rises by (1.359,2.128,2.161). Restoring
  the lights returns both regions exactly. The latter includes the weapon,
  so it is a scene-response control rather than a pure world-light oracle.
  Metrics and three images are retained beside the evidence.

Reproduction uses the existing configurations, without reconfiguring Waf:

```sh
WAFLOCK=.lock-waf-rc-lab-main python3 ./waf build --targets=render_lab -j8
WAFLOCK=.lock-waf-p2 python3 ./waf build --targets=client,engine,materialsystem,vguimatsurface,video_bink -j8
python3 tools/quality/conformance.py check --suite render.procedural-texture-sample --suite render.lab.panel --suite render.lab.panel.sensitivity --suite render.family.pbr --suite render.family.pbr.seeded-ignore-normal-map --suite render.lab.posed-model --config release --out <fresh-output>/suites.json
python3 tools/quality/portal_boot.py --runtime run/runtime-p2 --build build-p2 --game portal2 --renderer native-vulkan --headless --require-vulkan --map sp_a1_intro4_probe64 --startup-command 'r_core_world 1' --console-command 'sv_cheats 1; noclip; cmd setpos -1552 37 -96; cmd setang 0 -90 0' --console-command 'cl_world_panel_report 1; wait 80; r_core_panels_stats' --capture-wait 80 --out <fresh-output>/movie
```

The captures are 1024x768 correctness checks. Complete-frame 1080p High/120 FPS
performance acceptance, moving-fragment interaction, nested core views and
non-Linux platform evidence remain open; R91 is still partial. No shipped quality
or render budget was relaxed.

Architecture check, baseline and loader inventory pass; architecture fixtures
pass 166 tests and style fixtures 38. The owned changed lines pass pinned style.
The shared checkout style run separately reports another session's edited
`render/composition/core_world.cpp:1338`, left untouched (`style-complete.log`).
Whitespace checking uses `git -c core.whitespace=cr-at-eol diff --check` to
preserve the movie source's existing CRLF convention. Product/lab build logs
are retained from the existing profiles; the six-suite release record is
`accepted.json`. The first separate sign capture copied a client library during
a concurrent link and failed loading its ELF image (`sign-final/stdout.log`);
this failed attempt is retained separately and certifies no product coverage.

### R91: fizzler light through animated door apertures (2026-10-03)

User request: light must pass through the portion of a test-chamber door that
has physically opened, rather than appearing immediately when opening starts.
The door opens its visibility portal at `OnOpen`, before its panels have moved.
The old box producer excludes it through `EF_NOSHADOW`. Additionally, imported
BSP surfaces on the core used an area-light-only group without shadow tiles.
The initial controlled product run detects the BSP shadow gap: all six poses and
its removed-blocker control receive the same 4.5216 mean blue rise
(`quality-results/door-light-20261003/fixture/receivers.json`).

The physical caster extension is defined in
[`render/dynamic_occlusion.h`](../public/render/dynamic_occlusion.h) and documented
with its [contract](../unittests/rendertest/contracts/render.dynamic-occlusion.v1.md#core-physical-caster-extension).
The client supplies the authored collision triangles for opaque animated models
using custom bone-follower collision. Named followers and the server's existing
multi-solid fallback retain their authoring semantics; no door-name registry is
introduced. The actual Portal 2 model uses that fallback: a 576-vertex frame and
36-vertex left/right panels. Current client bone poses are gathered after both
legacy and graph render-start animation work, before light-set publication.
`EF_NOSHADOW` still controls legacy blob/CPU box behavior; physical leaves block
core lighting. Door animation, gameplay and visibility-portal timing keep their
existing owners.

The engine copies and versions valid triangle publications, then attaches them
only to the core's light snapshot. Queued views own their geometry. The core
reuses its existing shadow-depth renderer, atlas, caster culling, revision-based
mesh uploads, GPU completion retirement and static/composite tile cache. Meshes
replace the same entity's coarse boxes, and removal/movement invalidates the
corresponding composite tiles. Imported BSP and WMSH inputs now share opaque
world-caster preparation and area-light shadowing. The previous unshadowed BSP
area-only group is removed. The world pass now binds any view's light/shadow inputs,
including BSP receivers, instead of restricting them to WMSH. PBR and lightmapped
receivers use one `AreaLightVisibility` function in the core surface program; the
previous lightmapped irradiance helper ignored shadows. The atlas-only lab check
did not catch these receiver/binding gaps, so the final lab test draws a real
LightmappedGeneric BSP receiver through the production world pass. No second
shading algorithm is introduced.

Frozen-path: core plumbing R91 — the engine's occluder factory and light-set tee
carry physical caster inputs to the core. The original `VEngineOccluders001`
vtable remains exposed, and the client still uses it on engines without v2.
No shading is added to frozen engine CPU lighting or native backend shaders.

Evidence under `quality-results/door-light-20261003/`:

- `publication.json`: all five positive/seeded publication/occlusion suites pass,
  26 checks each. New checks cover owned payloads and retained snapshots,
  unchanged/moving revisions, duplicate keys, nonfinite data, incomplete/oversize
  meshes, null/oversize frames, whole-frame rollback including revisions, removal
  and reset without stale revision reuse.
- `lab-accepted.json`: native Vulkan shadow suite passes 190/190 with zero
  validation messages. Sixteen atlas checks and twenty actual BSP receiver pixel
  checks use a separate ray-plane aperture oracle for closed, partial, open,
  reclosed and removed triangle leaves. Pixel comparisons pair shadowed and
  unshadowed emission with a black bake. Missing leaves fail the closed-door
  oracle. The initial samples at x=24 (atlas) and x=16 (world-pass PCSS) touched
  the filter fringe; the final x=8/12/80/180 points lie inside the promised
  aperture/shadow interiors. Existing soft-filter edge coverage retains its
  original oracle and bands (`lab-shadowed.log`, `lab-world.json`).
- `product-final.json` and `accepted-door/receivers.json`: all 13 native product
  checks pass. Closed, opening-start, partial, open, reclosed and removed-blocker
  blue rises are respectively 0, 0, 38.1967, 94.8879, 0 and 99.9978 on the fixed
  receiver region. The partial physical aperture is 23.861 units versus 105.995
  fully open. Geometry/revisions match across each paused emission-off/on pair;
  closing restores the receiver exactly. Screenshots, console/cfg, generated VMF
  and native boot evidence are under `accepted-door/`. These are receiver/pose
  checks, not full-scene visual fidelity certification.
- `shader-regression.json`: area-light BRDF/reference coverage passes 72 checks;
  the shadow sensitivity suite passes 5 positive/seeded checks. The receiver
  binding defect is retained under `receiver-binding-failure/`: all six poses
  incorrectly had the same 110.7353 blue rise before the world-pass binding fix.
- `idle-regression.json`: the original generated idle fizzler fixture still
  passes all 10 lighting/state checks after BSP shadowing is enabled.
- `product-world-binding-build.log`: complete configured Portal 2 build passes.
  Architecture, baseline/inventory verification and changed-line style checks
  pass; architecture/style fixture suites pass 166/38 respectively. The known
  generated-SPIR-V CAP005 include classification failures remain separately
  open, as recorded in the preceding fizzler evidence.


The installed product fixture is `render.product.fizzler-door-light`; reproduce
through the shared conformance runner with a fresh output directory. It uses
constant generated flow textures, idle strength 16 without a portal hit, fixed
exposure, emission-off/on pairs at frozen poses, and a closed-door control with
moving shadow casters disabled. `setpause`/`unpause` freeze real client poses:
fixed `host_framerate` bypasses timescale, and zero `host_timescale` is not a
supported clock stop in this engine. Timing selects poses only; captured
physical bounds independently require an intermediate aperture and unchanged
geometry in each pair. Initial attempts that sampled a closed or completed pose
are not partial-pose evidence. Native temporary staging used
`/run/user/1000` after host Btrfs metadata exhaustion; only generated content and
capture records are retained, without retail asset bytes or runtime libraries.
The ordinary idle fizzler fixture remains a separate receiver/state check.

This is the bounded physical bone-follower/area-light shadow cohort. It does not
certify full animated visual-mesh shadows, cutout/transmission, the complete
SolidEnergy image, High complete-frame budgets, CPU/GPU placement crossover or
non-Linux native behavior. R91/R96 and those performance/platform gates remain
open.

The ordinary chamber sign is also visually verified on the requested map
(`sign-active/evidence.json`, `capture.png`, retained `console.log`):
`cmd setpos -737 160 16; cmd setang 0 -90 0; ent_fire InstanceAuto63-info_panel SetActive`.
Noclip bypasses its normal activation trigger, so the authored input is required.
It reports 171 lists/rasters/views/draws, zero refused and failed, and a
476x961 surface image; text, icons and grime are present. Scratch staging used
`/dev/shm` and a retained read-only O_NOATIME copy wrapper after the host Btrfs
metadata allocation blocked ordinary staging; renderer, content, profile and
acceptance checks are unchanged. The inactive sign captures certify no screen
coverage and are not used as the sign oracle. Full product budgets remain open.


## RCV-12: material and view specialization (2026-10-03, in progress)

User scope: specialize authored material predicates and absent view lighting,
inspired by [Filament's material feature guards](https://github.com/google/filament/blob/144ea3160a545aa2a56e4554822f1f426910f045/shaders/src/surface_lighting.fs)
and [Godot's view specialization](https://github.com/godotengine/godot/blob/e7cfa294a0b81bed7986be04a848cc1832a3f083/servers/rendering/renderer_rd/shaders/forward_clustered/scene_forward_clustered_inc.glsl).
The baseline is RCV-11's landed `5689e16b9`, including RCV-08's retained
144-VGPR LTC change; the older 192-VGPR capture is not the current baseline.

`render.material` owns predicate classification, shader evaluation and the
existing pipeline cache. `SurfaceProgram::Request` derives material predicates
from the exact immutable constants it uploads; raw `Pipeline` consumers retain
uniform predicates for explicitly mutable fixtures. Numeric values stay uniform.
The core world pass derives view lighting presence from its frame/view inputs
and selects the existing shader through `ViewPipeline`; shadow receiving, filter
samples, resolution, cohorts and effect settings are unchanged. New inputs select
new variants; no second material registry or lighting authority is introduced.

Work is isolated in `/home/john/.codex/worktrees/pbr-specialization/source-engine`
with private dependency/build seeds, separate Waf locks and copied runtime
`/home/john/.rc-pbr-5689`. Prior worktree test edits are preserved.
Native material conformance passes 124 checks, including exact raw/specialized
pixels, each authored predicate under live direct light, sun reappearance and
negative controls that detect omission of each of the five material features
and the sun. Alpha uses an actually masked base texture in this comparison.
The comparison exposed a fixture defect: its neutral shadow image was RGBA and
its comparison sampler ordinary. The shared fixture now binds far D32 depth and
the proper comparison sampler; validation is silent. Native posed-model 72 and
shadowed-light 154 checks pass; null world 76 and composition 58 pass.
Architecture/style checks, actual compiled shader statistics, complete High
images and matched full-frame routes remain required before retaining the trial.
The High 120 FPS gate and mobile evidence remain open.

### RCV-12 compiler attribution and first route comparison

The candidate checkpoint `adaa677e2` launcher is
`363787c7639da9c9da92d907a1c43c2f00b5484aad1c2259ae2ce9df0b3beb97`;
the RCV-11 retained control is `ab89419139a678b7662adbfa21af3352d6090d08ac4dcc415b35472f4aa25c8c`.
The actual 1920x1080 four-sample capture is under
`/home/john/.rc-pbr-5689/evidence/candidate-capture`. The RenderDoc wrapper's
process inspection fails to find the child Vulkan/SDL3 mappings; the capture
itself records the native Radeon renderer and full target. This wrapper receipt
is not a passing product smoke test.

Fresh replay processes measure eight captured PBR pipelines against the same
modules with material SpecId 3 frozen to the uniform sentinel and view SpecId 4
frozen to all lights present. Every pipeline remains 144 VGPRs, 10 subgroups per
SIMD, no scratch or spills. Code drops from 55,432–58,312 to 40,004–43,256 bytes,
instructions from 10,298–10,973 to 7,504–8,172, and branches from 223–235 to
148–165. These are static executable counts, not executed instructions or a
frame-time gain. Results and driver disassembly are in `shader-specialized`
and `shader-generic` under the external evidence directory.

The complete attachment comparison reads all four MSAA samples. Freezing only
view lighting gives byte-identical images. Freezing material predicates changes
500–514 of 8,294,400 channels per sample, maximum six byte levels, localized to
a diagonal edge. Native oracles remain exact; this additional capture comparison
is not exact and is being investigated, with no tolerance change or promotion.

First ordinary High/1920x1080/4x MSAA ABBA comparison, milliseconds:

| Run | Arrival GPU | Arrival CPU | Arrival interval | Reverse interval | Return interval |
| --- | ---: | ---: | ---: | ---: | ---: |
| `control-a1` | 38.794 | 21.296 | 38.852 | 20.115 | 34.755 |
| `candidate-b1` | 33.405 | 24.669 | 33.820 | 20.636 | 35.431 |
| `candidate-b2` | 36.070 | 25.237 | 36.720 | 22.859 | 38.409 |
| `control-a2` | 33.889 | 25.039 | 34.254 | 20.387 | 34.343 |

Arrival-phase median GPU clocks are 1705/2089.5/1978/2099 MHz. They are derived
from the engine's monotonic frame timestamps and the recorded wall-clock power
samples. The apparent first-run improvement is not a matched-clock speedup;
the second candidate also heats to 100 C and loses clocks in the reverse phase.
The nearly matched `candidate-b1` and `control-a2` arrival difference is small,
while reverse and return do not improve. No reliable complete-frame gain is
established. Every binary stays unchanged during each run. The existing map-name
assertion, missing ordinary detailed-scope timers and 120 FPS gate still fail.

### RCV-12 warp compatibility correction

Attribution isolated the six-level edge difference to event 24759 alone: the
portal gun glass material with diffuse warp enabled and its authored nine-mip
light-warp texture. Making only that pipeline's material features uniform
reproduces the entire fully uniform reference attachment, byte for byte in all
four samples. Explicit fine derivatives and blanket no-contraction trials do
not reproduce the old image; neither is retained. The precise numerical cause
inside driver compilation is not established, and no texture filtering changes
are made.

`SurfaceProgram::Request` now keeps authored diffuse/specular-warp materials on
the established uniform material path. It still specializes ordinary immutable
materials and all views independently. Classification and this compatibility
selection remain in the material owner. Native material conformance passes 134
checks: each of the five feature configurations additionally verifies the real
Request's pipeline selection and exact pixels. The shader's raw specialization
fixtures still test both warp features and their omission controls.
The corrected launcher is `2cc30b32a17a2ee31d88f539ca513eb9a4c35ef37fb854e74a2296a5d355f7cd`.

Before correction, additional ordinary runs were `control-a3` (arrival GPU
35.968 ms, interval 36.712 ms) and `candidate-b3` (32.554 ms, 32.913 ms).
Their arrival median clocks were 1887.5 versus 2141 MHz. They do not settle the
matched-clock question. The ordinary candidate native screenshot receipt passes
Vulkan/SDL3, 1920x1080, 4x MSAA, and scene-detail checks with no failures; its
image was inspected. That smoke receipt precedes the compatibility correction.
New power records also include monotonic time directly so frame/telemetry joins
remain reproducible across a host reboot.

The corrected capture `corrected-capture/renderdoc/sp_a1_intro4_probe64_frame219.rdc`
confirms seven ordinary/cutout pipelines use material masks 0/1 and the warp
pipeline uses the uniform sentinel; every captured view uses the actual area
presence mask 4. Freezing material predicates and view families back to their
full uniform reference now yields zero changed bytes in every sample of the
1920x1080 four-sample attachment. Report: `corrected-generic/results.json` under
the external evidence directory. This closes the identified image mismatch;
the RenderDoc-wrapper smoke limitation and full performance gates are separate.

A private paired GPU leaf benchmark reuses the existing lab timestamp runner:
128 warmup pairs, 64 measured pairs, alternated order, one identical receiver
pass for either pipeline. Its patch and full samples are retained under
`quality-results/pbr-specialization-20261003`; the patch is removed from the
supported lab source after measurement. Native image/seeded controls still pass
154 checks. Median paired specialized/control ratios are 0.9901 for two spots,
0.9821 for two areas and 0.9873 for four areas at 1024 square. These are small
(roughly 1–1.8 percent) receiver improvements, not whole-frame or High acceptance.
The actual corrected game pipelines all remain 144 VGPRs; seven have
40,004–43,244 code bytes and 148–165 branches, while the preserved warp pipeline
has 44,120 code bytes and 177 branches. Uniform references have 55,432–58,312
bytes and 223–235 branches. No scratch or spills is introduced.

### RCV-12 retained incremental optimization and final route evidence

The user explicitly selected retaining the small measured receiver improvements
(2026-10-03). Material/view specialization is retained as an incremental core
optimization with exact-image compatibility; it does not certify the complete
High performance gate. The paired 1024-square receiver measurements are:

| Receiver | Uniform median ms | Specialized median ms | Paired median reduction |
| --- | ---: | ---: | ---: |
| Two spots | 0.293979 | 0.291073 | 0.986% |
| Two areas | 0.616571 | 0.606011 | 1.786% |
| Four areas | 1.155235 | 1.138002 | 1.269% |

The reductions use medians of per-pair ratios, rather than the ratio of the two
reported medians. The historical diagnostic fields `fused_ms` and `split_ms`
mean uniform and specialized single receiver pass here; no visibility split or
additional product pass is introduced. The ignored diagnostic patch, sample log,
and unmodified source backup preserve the measurement. The supported lab source
was restored and rebuilt after this diagnostic run.

Final ordinary High/1920x1080/4x MSAA ABBA comparison uses the corrected launcher
against the retained RCV-11 control, with all 29 runtime library hashes verified
before and after every route. GPU and frame interval phase medians in ms:

| Run | Arrival GPU / interval | Reverse GPU / interval | Return GPU / interval |
| --- | ---: | ---: | ---: |
| `corrected-control-a4` | 33.167 / 33.368 | 19.538 / 19.651 | 33.311 / 33.530 |
| `corrected-f1` | 33.726 / 33.950 | 20.065 / 20.194 | 34.879 / 35.234 |
| `corrected-f2` | 32.326 / 32.736 | 19.233 / 19.522 | 32.427 / 32.672 |
| `corrected-control-a5` | 33.387 / 33.625 | 19.473 / 19.647 | 33.304 / 33.527 |

Averaging each pair of phase medians yields GPU changes of -0.75%, +0.74%,
+1.04% and interval changes of -0.46%, +1.06%, +1.27% for arrival, reverse and
return respectively (negative is faster). Dynamic clock/thermal conditions vary:
arrival median clocks are 2155.5/2047.5/2176.5/1970.5 MHz, temperatures
85/90/81/84 C, AC connected throughout. Thus these routes do not establish a
whole-frame speedup or a reliable regression at this small scale. Reports,
identity receipts, original frame streams and monotonic power telemetry are in
`/home/john/.rc-pbr-5689/evidence`. These measurements use source baseline
`5689e16b9`, not subsequent unrelated renderer work on `subsystem-refactor`.

Validation of the corrected implementation: actual launcher/engine/native adapter
and restored render_lab builds pass; native PBR conformance passes 134 checks,
posed-model 72 and shadowed-light 154; null world 76 and composition 58.
Architecture all/baseline/inventory and changed-line style checks pass.
The captured complete four-sample attachment is byte-identical to the uniform
reference. The 120 FPS floor, existing map-name assertion and ordinary report's
missing detailed scope timers remain failures; no complete performance acceptance
is claimed. The executable reduction leaves the current 144-VGPR allocation
unchanged. Further large gains must reduce executed area-light/shadow work and
its live state, while preserving lighting and sample counts.


### R91: screen caching and video preload (2026-10-03)

> **Superseded in part (2026-10-04, user request: video must use barely any
> memory).** `PRELOAD_VIDEO` no longer predecodes. `video_bink` streams: only the
> compressed clip, the decoder and one frame texture are resident, replacing the
> 235-frame elevator cache (about 400 MiB) and the 304-frame menu cache (about
> 1.8 GiB). Playback decodes and uploads one frame per tick again, so the
> upload-free playback and the frame-time gains below attributed to resident
> frames are not retained; the per-frame cost has not been remeasured. Seeks are
> exact by decoding forward from the demuxer's seek point. A borrower of the
> texture no longer keeps the last frame after the movie shuts down.
> `render.video.frame-cache` now checks streamed playback (9/9 checks).

User scope: reduce the impact of elevator movies and other world screens on
`sp_a1_intro4_probe64`. The 2026-10-03 slice predecoded every clip frame for
resident playback; that was superseded on 2026-10-04 because it kept hundreds of
frame textures resident. Each clip now streams (see the note above).
The panel remains the same ordinary core emissive PBR surface and light source;
world and glass receivers, including shattered glass, retain their normal paths.
No panel resolution, light count, shadow sampling or contributing effect is cut.

Ownership and mechanism:

- `render.world-panel.v1` owns actual-field image equality and the derived CPU
  tile cache. Static VTF sample epochs invalidate on their owner reload; unknown
  opaque procedural samples recompute. Unavailable samples, including translucent
  font textures, have an immutable fallback so they do not invalidate unchanged
  signs every frame. Coverage remains part of the authored quad.
- The texture owner publishes content epochs through the internal
  `ICoreTextures::ContentRevision` port. The panel pass retains its GPU image and
  mip chains while authored image, sampled texture identity/epoch/sampler and
  resolution match. Placement, lighting and emission scale remain live. Render
  targets and unknown epochs always rebuild; handle recreation and uploads change
  identity/epoch. No frozen public texture vtable changes.
- `video_bink` honors `PRELOAD_VIDEO` by streaming: only the compressed clip, the
  decoder and one frame texture stay resident, and each tick decodes one frame
  from the demuxer's current position, converts YUV to RGB and uploads it to the
  single procedural texture (seeks decode forward from the demuxer's seek point).
  No per-frame texture, cache array or movie frame selector remains. The normal
  texture owner retains regeneration bits for device restoration. Cache
  ownership ends with the video material/group, including level screen teardown;
  borrowers can retain the currently selected texture until their own release.
  This does not move decoding to a GPU video codec or create a movie shader.
- VGUI binding borrows a procedural texture without replacing its owner's
  regenerator. VGUI creates its own regenerator only for explicit pixel writes,
  and only clears one it owns. The old per-update video regenerator override is
  deleted. Creation references are transferred to the cache, and materials are
  released before owned textures.
- The shared LTC rectangle evaluator rejects only provably noncontributing
  backfaces/below-horizon rectangles before shadow filtering. Visibility debug
  evaluation retains its independent oracle behavior.

Frozen-path: `shaderapivulkan` changes are core texture-epoch plumbing;
`vguimatsurface` corrects borrowed texture-regenerator ownership; `video_bink`
removes the resident frame cache and streams through existing material/texture
APIs. No new legacy shading or CPU receiver lighting is added.

Evidence: `quality-results/core-screens-perf-20261003/` retains original frame
streams, native logs, binary/source receipts, authored quality fixtures, captured
movie images and the no-atime staging wrappers used on the metadata-constrained
Btrfs host. The installed workload is
`quality/workloads/portal2-screen-frame-pacing-v1.json`; all three passes, camera
transitions and their hitches remain in the receipt.

| Native diagnostic, third pass | Before median | After median | Before GPU render median | After GPU render median |
| --- | ---: | ---: | ---: | ---: |
| Elevator movie | 19.534 ms | 15.350 ms | 15.620 ms | 12.040 ms |
| Active chamber sign | 19.817 ms | 17.040 ms | 9.344 ms | 8.379 ms |

These matched drawable captures use actual 1024x768 on Radeon 8060S/RADV, with
MSAA off and HUD/viewmodel hidden to isolate the screens. They are not High
qualification. The shared checkout advanced during the work: source and binary
hashes are retained per run, and other material specialization changes are
included, so the whole-frame difference is not attributed solely to video
caching. `panels-off-control` and `area-lights-off-control` are attribution
controls, not product settings. `core-cache-only` retained the older font-sample
invalidation and therefore does not establish a sign speedup. `preload-initial`
rendered at 1024x720; its faster medians are not a matched-resolution comparison.
The explicit 1080 resize attempt fell back to 640x480 and is retained as
`resize-rejected`, excluded from the table.

Streaming keeps each clip at one frame's memory rather than a full decoded
cache. The superseded predecode measured the elevator clip's 235 frames at
640x400 as 229.5 MiB GPU RGBA plus 172.1 MiB BGR restoration bits, and the
menu background's 304 frames at 1280x720 as 1068.8 MiB GPU plus 801.6 MiB
restoration storage. Those caches are gone in the streamed path, which holds one
YUV frame, one RGB frame and one BGR888 texture per video material. The
streamed path decodes and uploads one frame per tick; its per-frame cost has not
been remeasured after the supersede.

Validation:

- `render.lab.panel`: 77 checks, including actual GPU pixels/raster reuse and
  invalidation plus CPU cached/fresh integration equality. Panel sensitivity
  4, area lights 72, area-light sensitivity 5, shadowed lights 190, posed models
  72 passed with no Vulkan validation messages (see `lab-final.json`).
- Installed `render.video.frame-cache` runs
  `tools/quality/video_frame_cache.py`: the native positive sequence passes
  25 checks for frame count/duration, seek, pause, loop, nonloop end,
  regeneration, borrower lifetime, final texture cleanup and invalid dimensions
  rollback. An authored wrong-green clip fails its two color checks.
  The command runner counts six acceptance checks and rejects missing probes.
  `video-cache/` retains fixtures, logs and the real elevator capture. Manual
  regeneration proves retained bits, not a complete device-loss lifecycle.
- The actual `render_lab` and Portal 2 product consumers build with their
  existing Waf profiles. Architecture all/baseline/inventory, architecture/style
  fixtures and changed-line style pass; final receipts are retained alongside
  the captures.

R91/R96, full device-loss and non-Linux acceptance remain open. The diagnostic
still contains slow frames (movie 61.191 ms and sign 23.492 ms maxima in the last
pass); the full 1920x1080 High 4x MSAA 120 FPS floor is unverified and not met by
these diagnostics. Remaining complete-frame area-light/shadow and submission
costs need further equivalent-output optimization. No performance promotion or
CPU/GPU decode-placement acceptance is claimed.

### K12: profiled static material requirements (2026-10-03)

The existing Portal 2 VMT auditor now runs the core's own world and mesh claims
under six scene-input combinations. Its v3 reports name the declared
`portal2-linux-native-vulkan-high-v1` profile and its VMT conditional settings,
possible world/model geometry, the opaque/blended or special-family pass,
minimal required world stage, native reflection probes and linear scene color,
and all failed claims. The batch protocol refuses drift between C++ VMT
condition defaults and the corpus parser. Proxied materials remain dynamically
unresolved until their bound values are checked. A missing patch include counts as
unsupported rather than silently inflating the supported total.

| Corpus | Statically supported | Supported with requirements | Dynamic | Unsupported |
| --- | ---: | ---: | ---: | ---: |
| All 3,738 Portal 2 VMTs | 1,354 | 1,062 | 146 | 1,176 |
| 1,165 model-path VMTs | 819 | 200 | 44 | 102 |

The full report identifies SpriteCard 143/143 and DecalModulate 101/101 as
unsupported, while Refract has 14/37 conditional candidates that require native
reflection probes, linear scene color or both. This is candidate material
eligibility under the selected profile, not evidence of scene reachability,
pass integration, texture residency,
correct pixels or mobile platform support. K12/R96 and R91 remain open.

Evidence: the isolated Waf `render_lab` target builds; both checked
inventories verify all 3,738 and 1,165 entries; the ten focused fixtures
pass, including malformed protocol, missing include, proxy and conditional
input controls. The native Vulkan `posed-model --validate` suite passes 72
checks with image readbacks, and `lightmap-basis --validate` passes 19. These
GPU suites cover accepted examples. `render.family.water` passes 41 checks,
including its six-case native Vulkan pixel oracle. They cannot certify all accepted
VMTs. The results in `quality/materials/portal2-{all,model}-claims.json` are
the exact per-material evidence for this slice.
The same tool read all 6,000 Portal VMTs using its declared Linux Vulkan
profile (2,296 static, 1,441 conditional, 366 dynamic, 1,897 unsupported);
that profile does not declare core-only High qualification, so this exploratory
run does not add a Portal acceptance claim or a checked-in baseline.

### K12: refusal groups by feature (2026-10-03)

The same claim audit now groups unsupported Portal 2 materials by a feature
named from the core's actual refusal diagnostics. It records each material's
feature keys and a report-level count of affected materials, appearances in
the first refusal, and unmapped-key incidence. The raw claim rows and exact
reason strings remain in the checked inventories. One material counts once per
feature even when six input scenarios repeat its refusal; feature groups
overlap and their counts must not be summed into a number of missing draws.
This is report indexing, not a second material-support rule or evidence that a
material is reached by a game scene.

| Feature in the full Portal 2 corpus | Refused materials |
| --- | ---: |
| `$shadersrgbread360` parameter | 154 |
| SpriteCard shader family | 143 |
| `$spriteorigin` parameter | 102 |
| DecalModulate shader family | 101 |
| `env_cubemap` per-view texture | 63 |
| Lightmapped mesh point | 62 |
| Subrect shader family | 57 |

Both checked inventories retain their prior status totals. The expanded
sensitivity suite passes 12 checks, including repeated claim rows, overlapping
features and family-name case folding. K12/R96 remains open; draw reachability
and pixels are separate evidence.

### Material interpretation direction (2026-10-03, user decision)

The user chose to implement legacy VMT definitions more fully in the Forward+
core, including emissive behavior, rather than requiring improved materials to
match the old shader's pixels. The [surface-model contract](0016-render-core.md#the-surface-model-legacy-definitions-in-the-modern-core-plan-2026-09-28-amended-2026-10-03)
now makes reviewed VMT-to-core interpretation the native default. Legacy-port
pixel oracles remain K4 compatibility controls, while matched game/lab scenes
and visual review judge visual quality. `$selfillum` defines visible
emission inputs but cannot by itself supply physical radiance for an area light;
that needs authored values or a reviewed source rule. The VMT and frozen legacy
renderer continue to serve exact legacy appearance, and unhandled non-neutral
settings remain named claim gaps.

This entry records a documentation decision, not implementation or new image
evidence. K11/R95, K12/R96 and the affected material cohorts remain open.

### K11/K12: render_lab follows the game (2026-10-03, user direction)

The user found that lab-only work was accumulating without appearing in the
game, and removed Cycles comparisons from `render_lab`: some Cycles images
look worse than the product and confuse native visual decisions. RFC 0016 and
the ranked roadmap now pair each term's K11 proof with its K12 game
integration and same-content, same-camera image check. A lab-only result is
work in progress. Cycles remains a map-baking input and historical reference;
its receiver scores no longer close a render_lab or product lighting gate.

The old `lighting_fixtures.py gallery` and `compare` CLI entries and the
Cycles lab gallery implementation were removed. The game's comparison runner
now requires image parity by default (and accepts an explicit
`--diagnostic` collection mode), which rejects diagnostic cameras instead of
reporting a passing gate. The existing matrix still has only one declared
pixel profile (`area-room/overview`); all other camera scores remain
diagnostic. No full game/lab parity, visual quality, performance or K11/K12
completion is claimed by this change.

### S5 native emission, first slice: VertexLitGeneric `$selfillumfresnel` (2026-10-03)

First bounded end-to-end slice of the
[2026-10-03 material interpretation direction](#material-interpretation-direction-2026-10-03-user-decision)
for the [surface model's emission term](0016-render-core.md#one-surface-terms-with-neutral-values).
Cohort: VertexLitGeneric `$selfillum` with its mask, `$selfillumtint` and
`$selfillumfresnel`/`$selfillumfresnelminmaxexp` on the shared PBR mesh point.
The checked-in Portal 2 inventory refused 17 materials only for
`$selfillumfresnel` (`materials/paint/bridge_paint_*.vmt`, the gel on
hard-light bridges, drawn by `C_ProjectedWallEntity` as dynamic meshes that
`CoreWorld::QueueMesh` captures).

Translation (`render.material`, the one owner; rule documented once in
`vertexlit_family.h`):

- `$selfillumfresnel` is mapped for `vertexlit` and read only with
  `$selfillum`, as the shaders' `SELFILLUMFRESNEL` combo requires; alone it is
  inert.
- With it, VertexLitGeneric and its phong (skin) shader agree: the emitting
  region covers `saturate( b + ( 1 - b ) c )` of the surface, `b = min / max`,
  `c = ( N.V )^exp` on the vertex normal, at radiance `max x tint x albedo`.
  A fully covered texel emits `tint x albedo x ( min + ( max - min ) c )`.
  The tint is linear (Source's gamma 2.2 rule), as on the existing mask path.
- Combinations the legacy helper resolves by dropping an authored setting
  are refused by name: `$selfillummask`, `$detail`, `$lightwarptexture` and
  `$normalmapalphaenvmapmask` under the fresnel term, plus negative or
  nonfinite controls. Unknown keys remain named gaps.
- Composition is unchanged from the reviewed mesh point: the covered share
  replaces the surface's lit color (`mix`), so the uncovered share keeps its
  PBR direct, probe and image lighting.
- No area light. `$selfillum` supplies no scene-unit radiance, so the
  translation publishes none. The cohort's real materials are projected-wall
  dynamic meshes, which neither existing RFC 0011 publisher reads (the client
  fits Studio models, `engine/world_emitters.cpp` brush faces and overlays).
  Open: those publishers still infer radiance from base x mask at tint 1 for
  every `$selfillum` model and world face (`selfillum_emission.h`, the
  2026-09-28 user-requested rule), including a model that sets
  `$selfillumfresnel`, whose view-dependent weight they do not model.
  Reconciling them with the 2026-10-03 rule is a separate decision.

Shading lives in the core: `surface_program.glsl`'s PBR self-illumination
block reads a new `SurfaceConstants::selfIllumFresnel` (appended to the
material block, 448 to 464 bytes; no existing offset moved). It is a uniform
branch inside the existing `kSelfIllum` term: no new specialization term, so
the permutation count is unchanged. Nothing in the frozen paths changed.
`WorldPass::SetSurfaceFragmentModule` lets `render_lab` sensitivity runs pass
a seeded fragment module to the pass's color resolvers; products never set it.

| Check | Evidence | Result |
| --- | --- | --- |
| `render.lab.selfillum` | `tools/render/lab.py suite selfillum --validate`: one quad, left half base alpha 1, right half 0, orthographic view, distant eye at 0, 60 and 80 degrees. Analytic emission over a dark scene: 2.000, 0.6499 and 0.2542 against 2.000, 0.6500 and 0.2543 (`[0.2 2 2]`); `[3 1 1]` saturates to 1; max 0 emits 0. Lit at 80 degrees, the region is `( 1 - w )` x the same quad drawn without self-illumination plus the emission (1.8359 against 1.8355); the unmasked half is bitwise that plain surface. `[1 1 3]` is bitwise `$selfillum` alone and `$selfillumfresnel` alone bitwise inert. A captured dynamic mesh (the game's `QueueMesh` form) gives the same 0.6499; a mesh with no vertex normal faces the eye (2.000). Tint `[1 .5 .25]` gives 2.000, 0.4390, 0.0955 (linear). The unmasked half stays 0: the emitter lights nothing. Nine headless claim checks: the cohort claims, four combinations and negative controls are refused by name, unknown keys stay gaps. 23 checks, 0 validation messages | pass |
| `render.lab.selfillum.sensitivity` | Seeded `surface.frag` programs (`selfillum_defects_spv.h`): coverage ignored fails 4 checks (60/80 degrees, lit remainder, dynamic); brightness ignored fails 8 (facing, angles, zero max, tint, dynamic, no normal, lit remainder); control passes | pass (3 checks) |
| Existing lab suites | posed-model 77/77 (the elevator floor's mask path unchanged), lighting-controls 32/32, clustered-lights 26/26, area-lights 72/72, model-selection 37/37, view-state 16/16, panel 77/77, all with validation | pass |
| `debug-views` | 90/91: `view.16.inf.lightmapped` fails identically at `aa4e5e62` with this change reverted, on llvmpipe; pre-existing, not this slice | known fail |
| Static checks | `archlint check --all`, `baseline --verify`, `inventory --verify`, archlint and stylelint unit tests, `stylelint --changed`; `shader_toolchain.py check` (189 modules, 0 failures); `tools/quality` conformance and shader-artifact tests; `tools/render` shader-artifact, claim-inventory (and its 12-check self-test), Vulkan-scan and core-link tests | pass |
| `tools/render/tests/test_shader_toolchain` | 2 failures, 1 error at `aa4e5e62` with this slice's toolchain edit reverted (a stale `check_inventory` keyword, regenerator argument text, the embedded-file count); pre-existing | known fail |

Device and runner: lavapipe (llvmpipe, Mesa 25.2.8, Vulkan 1.4) in a cloud
container with the pinned shader toolchain. The manifest's
`linux-native-vulkan-gpu` profile requires an integrated or discrete GPU, so
`conformance.py plan` reports both new rows unavailable here; the results
above come from `tools/render/lab.py` directly and are not GPU-runner
evidence.

Not done, and why:

- Game/lab image match: not run. This container has no Portal 2 content, so
  no product boot, no claim inventory refresh and no in-game capture were
  possible. Integration is by construction only: the game's static props,
  posed models and `QueueMesh` captures resolve through the same
  `ClaimForMesh`/`ResolveMesh` and `surface.frag` the lab drew. Whether each
  `bridge_paint_*` material passes the new refusals is unknown until its
  variables are read. Reproduce on the content host:
  `python3 tools/render/material_claim_inventory.py --game portal2 --scope all
  --render-lab build-rc-lab/render/lab/render_lab --out
  quality/materials/portal2-all-claims.json` (then review and commit the 17
  changed rows; `render.material.all-claim-inventory` fails its `--verify`
  until then), then capture a Portal 2 view of a painted light bridge with
  `r_core_world 1` and the same draw in the lab for visual review.
- Frame time: desktop and Fold7 unavailable (no GPU, no device). The added
  cost is one uniform branch and a `pow` per self-illuminated fragment.
- K12/R96 stays `active`: this slice closes no K11 or K12 gate.

### S5 `$selfillummask`: the mask texture path, proven and integrated in-game (2026-10-04)

The second bounded slice of the same emission term, completing the
[2026-10-03 direction](#material-interpretation-direction-2026-10-03-user-decision)'s
"its mask, tint and fresnel control the visible emitting region" for the one
part the fresnel slice could not cover. Cohort: VertexLitGeneric `$selfillum`
with a bound `$selfillummask` texture and `$selfillumtint`, on the shared PBR
mesh point. `render.material` remains the only owner of the VMT-to-core
translation; nothing here adds a family, a specialization term or a shaded
pass.

Translation, unchanged owner and unchanged surface terms: the mask texture
occupies the existing emission slot (`program_resolver.cpp`,
`textures.emission` for `selfillummask`), and the shader reads it through the
existing `kSelfIllumMask` specialization inside the existing `kSelfIllum`
block. `$selfillummask` without `$selfillumfresnel` was already claimed; the
fresnel slice refused it by name only in combination with the fresnel term,
which the legacy shader resolves by ignoring the mask. Both paths therefore
follow the one rule.

Oracle (`render.lab.selfillum`, grown from 23 to 26 checks): a second two-texel
fixture texture whose RGB is the *inverse* of the base fixture's alpha (left 0,
right 1). With `$selfillum` + `$selfillummask` + `$selfillumtint [1 .5 .25]`,
the base-lit half emits 0 and the mask-selected half emits the tint's linear
color, which is only possible if the mask texture and not base alpha selects
the region. Ten headless claim checks; `$selfillummask` without the fresnel
term is a claimed cohort, and the fresnel combination stays a named refusal.

Negative control: a third seeded `surface.frag` program
(`-DSEEDED_SELFILLUM_MASK_IGNORED`, `selfillum_defects_spv.h`) drops the mask
texture and falls back to base alpha. It inverts the result exactly — the
masked half reads `1.000 0.219 0.048` where it must be 0, and the
mask-selected half reads 0 where it must be 1 — so the two new checks fail
(25 checks, 2 failed) and the sensitivity run rejects it.

Game integration and matched images, on the content host. `render_lab` cannot
read retail Portal 2 maps (BSP v21 has no WMSH), so the matched pair uses a
published lighting fixture the core already draws: `lt_cornell_floors`, whose
`models/lt_cornell_floors/probesphere.mdl` is posed 376 times by the core
(`r_core_world_stats`: `posed 376 models/lt_cornell_floors/probesphere.mdl`).
A private content root replaces the sphere's default skin with

    "VertexLitGeneric"
    {
        "$basetexture"  "lt_cornell_floors/probegrey/basecolor"
        "$selfillum"    "1"
        "$selfillummask" "lt_cornell_floors/probegrey/basecolor"
        "$selfillumtint" "[1 .5 .25]"
    }

The game boots Portal 2 native Vulkan with `r_core_world 1` and this content
(`portal_boot.py --content-root`, 512x384, tone-map scale 1); `render_lab`
renders the same map, camera and model through the same program
(`--core-direct`, `--model .../probesphere.mdl`). Images retained under
`quality-results/rendercore-selfillum-mask-20261004/` together with both
commands, the boot evidence and the per-suite results.

| Check | Evidence | Result |
| --- | --- | --- |
| `render.lab.selfillum` | mask texture controls the region (base-lit half 0, mask-selected half the linear tint); claim checks unchanged; 26 checks, 0 validation messages | pass |
| `render.lab.selfillum.sensitivity` | mask-ignored inverts both mask checks; fresnel-ignored and brightness-ignored unchanged; control passes; 4 checks | pass |
| Matched game/lab image | both draw the same visible orange emission from the same `$selfillum`/`$selfillummask`/`$selfillumtint` through the core; overall mean 4.28, p99 31, 25.0% of pixels over 8, all in the sphere and its lit surroundings | diagnostic |
| Emission-off control | the fixture's own PBRMetalRough skin leaves game and lab agreeing at mean 0.58, p99 4, 0.4% over 8; the override moves the lab frame by mean 5.19 and the game frame by 0.09, so the term is the only variable | pass |
| Core ownership in-game | `-vkframestats` over the captured frames: `legacy_program_draws` 0 in every settled frame, so the sphere's emission is not a legacy shader port; `r_core_world_stats` reports 376 posed sphere draws | pass |
| Variables reaching the core | the engine's `ReadVariables` forwards `MATERIAL_VAR_SELFILLUM` through `RenderLegacyMaterialFlags::Keys`, and the block carried `$selfillum 1` with `$selfillummask` bound | pass |
| Portal 2 claim inventory | the 17 `materials/paint/bridge_paint_*.vmt` rows no longer refuse for `$selfillumfresnel`; they now gate on `$envmap needs the stage's native reflection probes`, a scene input. `material_claim_inventory.py --verify` passes 3738 checks; its 12-check self-test passes | pass |
| Neighbouring lab suites | posed-model 77/77, lighting-controls 32/32, model-selection 37/37, area-lights 72/72, all with validation | pass |
| Static checks | `archlint check --all`, `baseline --verify`, `inventory --verify`, `stylelint --changed` and the branch diff, `shader_toolchain.py check` (198 checks, 0 failures), conformance runner over both manifest rows (2 matched) | pass |
| `render.legacy-freeze` ratchet | 2 pre-existing failures, neither in this change: `materialsystem/shaderapivulkan/shaders/probe_volume.glsl` and `solidenergy.frag` shrank, and `-vkopaquebatch` is a new frozen-path switch, all from concurrent work merged into the base | known fail, pre-existing |

What the game/lab pair does and does not prove. Both frames show the same
implementation producing visible emission, which is what this slice claims.
The image score is a **diagnostic**, not a parity gate: `lt_cornell_floors`
has no declared image profile, the override is a synthetic skin rather than
retail content, and the sphere's centre differs between the two frames
(game 194/115/80, lab 183/126/100) — same hue family, different level and
saturation. That residual is a lighting and scene-input difference, not a
missing term: it is unchanged when the term is off (0.58 mean), and the
fresnel/mask/tint math is already checked analytically in the lab.

A finding worth keeping: the legacy VertexLitGeneric shader **clears**
`MATERIAL_VAR_SELFILLUM` when the base texture has no alpha channel and the
material supplies neither `$selfillummask` nor `$selfillumfresnel`
(`InitVertexLitGeneric_DX9`). So a bare `$selfillum` on an opaque texture never
reaches the core, and the static claim inventory would otherwise over-report
that cohort. Reaching this cost one instrumented build of
`engine/render_core_world_draw.cpp` and `materialsystem/cmaterial.cpp`; the
temporary diagnostics are removed and the rule is now recorded here rather
than re-derived.

Not done, and why:

- The 17 `bridge_paint_*` materials now need the stage's native reflection
  probes before they claim, so the real light-bridge material is still not
  captured in a matched frame. The published-fixture sphere proves the term
  and its path; the retail bridge remains open.
- Area lights: unchanged and still refused. This translation publishes none.
  `$selfillum` supplies no scene-unit radiance, and the existing RFC 0011
  publishers still infer radiance from base x mask at tint 1, including for
  materials that set `$selfillumfresnel`, whose view-dependent weight they do
  not model. Reconciling them with the 2026-10-03 rule stays a separate
  decision.
- Retail Portal 2 maps cannot be compared against the lab until they are
  published with a WMSH lump.
- Frame time: desktop measured only through boot, not a paced workload; the
  Fold7 is unavailable. The added cost is one uniform branch and one extra
  sampler read per self-illuminated fragment.
- K12/R96 stays `active`: this slice closes no K11 or K12 gate.

## Map completion scope: sp_a1_intro4_relit (2026-10-03, active)

User request: "Keep going until all materials and models that
sp_a1_intro4_relit uses are supported by rendercore - correctly." This session
owns the map-specific material/model completion work in `render.material` and
`render.pass.world`, proven in `render_lab` before product integration. The
initial checkout has no render edits; concurrent game UI edits are unrelated.

The published container is `run/maps/sp_a1_intro4_relit/maps/sp_a1_intro4_relit.bsp`
(BSP2 digest in its existing `published.json`). Exporting its legacy lumps with
the installed independent reader identifies 498 static props using 65 model
names, and 83 distinct model names across static props and authored entity
models. Their material tables resolve to 98 installed material names. This is
content reachability, not proof of selected submeshes, runtime-created models,
proxy values, visibility, shadows or correct pixels. The stage uses its existing
44-material WMSH material list.

Seven referenced model materials fail the current claim: `leaves`,
`leaves_bushes`, `leaves_dead`, `vine_cluster_loop01_dry`, `vines_suspended01`,
`vines_thick_384`, and `vines_thick_384_static`, all under
`materials/models/props_foliage/`. Six request tree sway; all seven specify the
legacy low-quality flashlight hint. The map authors two `env_wind` entities.
The runtime census previously grouped 18 refusals by reason, concealing names;
named per-material diagnostics and linewise console output are the first
boundary fix. The full gate stays open until material terms, geometry,
animation, alpha coverage, cutout/transmission shadows and the map's actual
runtime draw cohorts have evidence. No claim is loosened by this census work.

### RPRB v5 reader and upload repair

The published map carries 150 reflection probes in RPRB v5. The engine's
directory check rejected v4/v5, the shared C++ reader rejected v5, and its
GPU packing copied only one candidate word per cell. The already shared
shader supports four words; the reader and uploader now preserve all four
in their serialized cell order. Versions 1/2 retain the 16-probe limit,
versions 3/4 retain 64, and version 5 accepts 256. Directory and payload
versions must agree. Failed validation leaves the caller's layout unchanged.
`mapcontainer` owns the supported-version check; no shading copy is added.

Evidence under `quality-results/intro4-rendercore-completion/`:

- `rprb-reader.json`: release conformance, 143 checks pass. Covers independent
  64/256-probe fixtures, a conservative 150-probe subset with unchanged
  captured texels, missing ranks across every mask-word boundary, undeclared
  bits in the partial third and unused fourth words, GPU packing bounds and
  complete candidate bytes, plus the original malformed/fuzz/blend/relight
  corpus. The unknown-version mutation uses version 6; version 4 is declared.
- `rprb-candidates-lab.log`: Vulkan lab, 29 checks pass; every grid cell,
  boundaries and outside points, 64/256 probes, four selection modes.
- `rprb-probes-lab.log`: Vulkan lab, 40 checks pass, including relighting and
  the independent blend oracle. Both suites report zero validation failures.
- `rprb-python-format.log`: eight format/relight/capacity Python tests pass.
  The broader 37-test run has five unrelated placement failures, retained in
  `rprb-python.log`; this repair does not change placement behavior.
- `rprb-boot/evidence.json` and `rprb-boot/runtime/engine.log`: Portal 2,
  native Vulkan, queued mode, strict rendercore world, baked indirect and
  runtime direct light. Uploads v5, 150 probes, 8192 x 5019 GPU texture.
  Claimed material slots rise from 270/357 to 336/357; 486/486 world surfaces,
  6599 core views, zero failures. Static props remain 427/498 (452 eligible),
  and 123 model import candidates still include four MDL refusals. The boot
  image proves reachable rendering only; it does not close geometry,
  foliage deformation, shadow, temporal, nested-view or performance gates.
- `style.log` and `arch-all.log`: changed-line style and full architecture
  checks pass. The lab uses isolated `build-intro4-support-lab` with a clean
  checkout of the profile's pinned KTX revision; existing caches/profiles and
  the dirty dependency checkout are preserved. Product evidence is privately
  installed through `--destdir` using `build-p2-fsr`.

Reproduction: `python3 tools/quality/conformance.py check --suite
world.reflection-probes --config release`; `python3 tools/render/lab.py suite
reflection-candidates --tree build-intro4-support-lab --validate`; the same
command with `reflection-probes`; the boot runner's exact options are saved
in `rprb-boot/evidence.json`. R96/K12 and this map's completion remain active.

## Intro4 swipe comparison launch setup (2026-10-03)

User direction: compare all retained Intro4 poses at 3840x2160. The default
pair is `sp_a1_intro4` / `sp_a1_intro4_relit`. Both boots use native Vulkan
from `build-p2-fsr` / `run/runtime-p2-fsr`. A explicitly disables the render
core (`-norendercore`, `r_core_world 0`), temporal scaling and HDR display
output. B enables the core, FSR Native AA (`r_temporal_scale 1`), disables
MSAA and requests HDR output with `mat_hdr_exposure 3`. This supersedes the intermediate DXVK A
selection. The 18 deduplicated comparison/survey cameras exclude the panel
close-up by user direction and include the
64-unit eye offset and activate both chamber panel relays. Each camera,
viewport, PNG extent and B's native-resolution FSR dispatch is verified
before publishing its swipe page. The batch boots A and B concurrently in
one private compositor, then takes every selected pose in those same two
game sessions using the installed view-oracle alias-chain helper. Named
screenshots and numerically ordered view oracles pair each camera. The
closing boot screenshots are verified in the count but excluded from the gallery.

Reproduction from the repository root:

```sh
python3 tools/render/map_swipe_compare.py --all-captures \
  --out quality-results/map-comparisons/intro4-4k-native-legacy-fsr
```

Setup evidence lives in `quality-results/map-swipe-setup/`:
`parallel-host-tests.log` verifies two overlapping mocked hosts, native legacy
A, native core/FSR B at exposure 3 and all 18 gallery links/receipts. Negative
fixtures reject missing captures, wrong names, camera/extent mismatches, boot
failure and missing FSR dispatch. `pose-inventory-native-exposure2.log` checks
the retained camera inventory (the exposure change does not change poses).
Python compilation and scoped whitespace checks pass. The 18 existing
view-oracle tests also pass. Full architecture checking still reports the
pre-existing CAP002 `charconv` include in the graphics-settings header.

Actual run: `quality-results/map-comparisons/intro4-4k-parallel-exposure3-20261003/`.
Both boots pass with identical executable snapshots. A takes 124.94 seconds
and B 162.84 seconds, concurrently; these are capture workload durations,
not gameplay performance acceptance measurements. Each writes 18 named
captures plus the boot runner's closing capture. All 18 requested camera
pairs, 36 3840x2160 PNGs and 18 gallery links pass the checks in
`quality-results/map-swipe-setup/parallel-capture-verification.log`. The
panel pose is absent. B confirms `FSR game: 3840x2160 -> 3840x2160, before
post/HUD`, `mat_hdr_exposure 3` in its startup cfg, and accepted Rec. 2020/PQ
10-bit presentation. A confirms standard-range output without a core pass
recorder. The overview images were visually inspected for camera alignment.
The complete multi-view gallery is `index.html` in that run directory. HDR presentation remains unverified:
the private headless compositor and RGB PNG output cannot establish native
HDR display output. R96/K12 and RFC 0019 qualification gates remain open.


## Intro4 native 8K HDR archive (2026-10-03, captured)

The user selected native 7680x4320 with FSR off because the installed experimental
provider only supports output through 3840x2160. B requests HDR output with
exposure 3 and a 10000-nit display peak, retaining 4x MSAA. A remains native
Vulkan with the render core, temporal scaling and HDR output disabled. The two
private persistent game hosts capture all 19 retained Intro4 poses in parallel.
The added first-fizzler doorway eye is (400.03, 223.97, 66.48), angles
(0, -45.04, 0); the panel close-up remains excluded.

Before camera capture, both hosts trigger `fizzler1_disable_rl`, wait 134
fixed simulation frames (2.01 seconds), query `ent_dump fizzler_brush`, and
wait another 10 frames for its server/client reply. The gallery requires a
bracketed `StartDisabled: 1` reply. `ent_dump` now prints boolean keyfields
instead of silently omitting them. Later fizzlers are left alone.

`screenshot_hdr <name>` exports the existing linear RGBA16F scene through
the preserved ReadPixels ABI as RGB float32 PFM before exposure, grading or
SDR conversion. A JSON sidecar records Rec.709 primaries, 203-nit reference
white, dimensions, finite RGB peak, exposure, requested display peak, active
HDR presentation and whether the required output pass was recorded. Readback
waits for GPU submission completion and respects mapped image row pitch. The
existing PNG comparisons remain SDR previews; each B pose links the raw PFM
and metadata for offline conversion.

Native 8K initially failed the desktop lighting grid's 65536-froxel bound.
The owning desktop limits now allow 262144 froxels and 4194304 light indices,
preserving 64-pixel tiles and 24 depth slices. The GPU cluster suite passes
28 checks, including native 8K grid shape and refusal above bounded capacity:
`quality-results/conformance.20261004T035940Z.json`. The output suite passed
27 checks in `quality-results/conformance.20261004T034556Z.json`.

The initial native 8K trial boots successfully and exports exposure-1 and
exposure-3 scenes with the same raw peak 1.275390625; each archive contains
finite values above scene white. It confirmed `StartDisabled: 1`, RGBA16F
scene storage and accepted Rec.2020/PQ output. That development trial exposed
metadata defects (a trailing NUL and an absent output-pass telemetry variable),
which were corrected before the complete gallery run. Trial evidence is in
`quality-results/map-swipe-setup/hdr-8k-proof-native-capacity/`; it is not the
final archive receipt. The complete gallery is saved under
`quality-results/map-comparisons/intro4-8k-native-hdr-20261003/`.

The completed run passes: 19 paired poses, 38 7680x4320 SDR PNG previews,
19 linear HDR PFMs and their JSON sidecars. Both hosts confirm the first
fizzler disabled before any export. All requested camera origins/angles,
viewports, finite pixels, byte counts and sidecar settings pass. Every B
export confirms active HDR output and the required output pass. The highest
raw RGB channel is 2.734375 scene-white units; the archive preserves values
above SDR white without applying exposure. A and B use identical executable
snapshots and complete in 437.61 and 649.12 seconds, concurrently. These are
capture workload durations, not performance acceptance measurements. The
receipt is `quality-results/map-swipe-setup/hdr-8k-gallery-verification.json`.
The new doorway pair was visually inspected for alignment and an absent
first fizzler. The gallery links each full-resolution comparison, raw PFM
and sidecar.

The exposure control trial preserves 99.08% of raw channel values exactly
between exposure 1 and 3 (the remaining scene animation continues), with an
unchanged raw peak, while the SDR previews become brighter. See the trial's
`pixel-verification.json`. Output exposure therefore does not bake into the
HDR archive. The completed run used the longer closing wait. The subsequent
shortened-wait experiment was disproved by the 2026-10-04 B refresh: exec's
closing commands can run before future alias waits finish. The script again
uses the complete frame budget, including each HDR export, before closing.

The 18 view-oracle fixtures also pass. Architecture baseline and loader
inventory verification are current. Missing physical display measurements
still prevent a physical HDR brightness claim.

The configured product build and swipe fixtures pass (including nine negative
capture cases). Changed-line style is clean for this slice; an unrelated
world-pass change has STYLE001. Architecture reports its existing charconv
include occurrence with zero new/stale occurrences. Physical HDR display
brightness, R96/K12 and temporal-provider qualification remain unverified.


### Original-style offline SDR previews (2026-10-03)

The user requested recreating the PNGs from raw HDR with mapping fitted to the
original game's style. `tools/render/hdr_swipe_tonemap.py` reads the archived
linear Rec.709 PFMs with their 203-nit scene-white convention, correctly
reverses PFM's bottom-first rows, and fits a shared monotonic curve against
A's luminance quantiles. Each pose gets a reference-fitted exposure; the
shared contrast is 1.21. The curve has a smooth shoulder approaching display
white and preserves black. Out-of-gamut chroma is compressed towards mapped
luminance, retaining hue and luminance. Output PNGs carry an sRGB chunk.

```sh
python3 tools/render/hdr_swipe_tonemap.py \
  --gallery quality-results/map-comparisons/intro4-8k-native-hdr-20261003 --preview
python3 tools/render/hdr_swipe_tonemap.py \
  --gallery quality-results/map-comparisons/intro4-8k-native-hdr-20261003
```

All 19 B PNGs, difference images and swipe pages are regenerated at 7680x4320.
Each swipe links the preserved `b-engine.png` and original-style settings.
`comparison-engine.json` keeps the previous engine receipt. Original capture
timestamps, A images, PFM archives and HDR sidecars are preserved. The fitted
curve, per-pose exposure and source/output SHA256s live in the gallery's
`tone-map.json`. The index explicitly labels the offline SDR grade, and the
raw HDR links remain available. This presentation grade does not promote a
native image-fidelity or lighting gate.

Verification: Python compilation and both swipe fixtures pass, including
their negative capture cases. Black, neutral hue, monotonic brightness and
non-finite rejection checks pass. All 19 final PNGs have the correct extent
and sRGB tag, and hashes verify untouched A/HDR/sidecars and preserved engine
PNGs. Mean encoded luminance-quantile RMSE drops from 0.12314 to 0.04247
(65.51%). Eighteen of 19 poses improve this statistic; cool-exit has residual
lighting/detail differences visible in its source capture and its statistic
rises from 0.1065 to 0.1194. No spatial correction is used to conceal them.
Evidence: `quality-results/map-swipe-setup/original-style-verification.json`.
The doorway, overview, both lightboards, cool corridor and warm-room views
were visually inspected. `tone-map-result.png` in the gallery shows A, the
engine B preview and the recreated B preview side by side.


### Intro4 game material defects: model layout, glass and AO receivers (2026-10-03)

The actual `sp_a1_intro4_relit` game capture, including the FSR path, is the
visual oracle for this slice. The private AO prepass's model resolver now uses
the model vertex layout and model view bindings. A world-layout material on a
model surface is excluded from that prepass. This repairs the claimed-view
failure for `models/props_office/office_wallframe`; the material stays claimed.
The world null suite's portal/private-depth fixture includes both the eligible
model surface and the deliberately wrong-layout surface. It passes 110 checks:
`quality-results/intro4-rendercore-completion/world-prepass-conformance.json`.

The dropper's metal showed foliage-shaped dark patches. Matched in-game
`dropper-front-isolation/` captures remove those patches with AO disabled, while
disabling IBL/SSR retains them. The private AO depth includes static geometry
but does not yet gather later posed-model cohorts. Applying that visibility
without a receiver check projects foliage behind the door onto its metal.
`render.pass.ao` now preserves the receiver's view distance alongside visibility,
and the surface rejects another receiver's visibility. Neutral scalar inputs
retain their contract. The corrected strict FSR game run passes and its image
removes the false silhouette: `dropper-ao-receiver-game/evidence.json` under
`quality-results/intro4-rendercore-completion/`. The GTAO suite independently
checks those distances against its ray-cast receivers, including projection
and half-float rounding; 13 checks pass in `ao-receiver-gtao.log`.

Native Refract now combines transmission and dielectric reflection with the
shared Fresnel term. Authored contrast shapes bounded coating reflectance,
rather than squaring HDR probe radiance and adding it over the background.
The posed-model GPU suite checks a uniform white furnace and an eight-unit
reflection source, with the authored contrast enabled: 79 checks pass in
`glass-energy-posed-model.log`. The strict game `glass-fresnel-game/evidence.json`
passes. The launcher containing both this fix and the AO receiver fix was
installed into `run/runtime-p2-fsr`; running processes need a restart.

Matched `portal-core-legacy-isolation/` and `portal-fsr-matched-game/` captures
showed the exit room without FSR and the entry wall with FSR. The native legacy
pipeline treats dynamic triangles as clockwise; the core's surface pipeline
uses counterclockwise front faces. The frozen frontend now normalizes captured
triangles before submitting them to the core. Its cull mode remains intact.
`portal-winding-game/evidence.json` passes with zero claimed-view failures;
visual inspection confirms the exit room inside the aperture on the strict
FSR path. The view-state GPU suite passes 17 checks, including a deliberately
reversed dynamic aperture that back-face culling must reject, in
`portal-winding-view-state.log`. Clean strict counters alone do not certify
these pixels. The clean runtime was installed into `run/runtime-p2-fsr`.

Frozen-path: defect fix in `shaderapivulkan`'s legacy-to-core mesh capture:
normalize Source's dynamic triangle winding to the core convention. The native
legacy shader programs and their winding policy are unchanged. Posed-model AO gathering, full material/cohort coverage,
cutout shadows and complete gameplay-frame budgets remain open; this slice
does not promote K12/R96 or temporal-provider qualification. Architecture
checking reports the existing CAP002 charconv include in
`public/gameui/graphics_settings_service.h`, with zero new or stale occurrences.


### Intro4 cable and world-cutout material support (2026-10-03)

The `cable` family owns CPU-expanded Cable/SplineRope ribbon shading. UV0
samples the linear normal texture, UV1 samples the sRGB base texture, and the
captured linear vertex lighting and alpha modulate it. The normal-map
half-Lambert term simplifies to blue squared. MINLIGHT/MAXLIGHT are declared
but unread by Cable_DX9's programs; the family explicitly preserves that
behavior. Missing required base/normal inputs are refused. The GPU oracle uses
different coordinates for both UV sets, an independently decoded sRGB texel,
linear vertex lighting, and alpha; the posed-model suite passes 84 checks,
including the world-cutout policy controls, in
`quality-results/intro4-rendercore-completion/world-coverage-posed-model.log`.

The actual game exposed an absent normal input on `cable/cable`: Source's
shader parameter declaration describes a texture default but does not assign
it. Cable's initialization now populates its declared default before loading
the bump texture. This removes the texture warning and makes the game's
expanded ropes reach the core. `cable-default-game/evidence.json` passes with
zero claimed-view failures and no cable refusal. The core handoff still uses
its existing explicit `r_core_dynamic_draws 1` policy; full cohort acceptance
and default promotion remain incomplete.

Frozen-path: defect in Cable_DX9 initialization: use its existing declared
normal-texture default before LoadBumpMap. Ribbon geometry and vertex-lighting
production remain the frozen frontend's existing inputs. The native legacy
cable shading remains reachable with the handoff disabled; its retirement
condition is complete cohort/default promotion, not this opt-in slice.

Lightmapped opaque cutouts now carry `$allowalphatocoverage` to the surface
pipeline. Native PBR and legacy surface points share one coverage function.
Single-sample/FSR targets retain the authored alpha test; only multisampled
opaque cutouts request coverage. This accepts the metal-grate setting without
changing translucency into coverage. Native 4x image/performance qualification
remains open.

The installed null world/material contracts pass 110 and 82 checks in
`material-support-contracts.json`. The fixtures' old five-family inventory and
legacy-Refract negative case were stale before this slice: the reviewed
inventory now includes the existing Refract/depth/portal families and the new
cable family, and MotionBlur remains the negative legacy shader. Required
source lists now include the cable family wherever the resolver is linked.

A combined capture without fixed timing passed boot checks but did not settle
all camera poses; it cannot certify the requested multi-pose images. The repeat
uses the harness's existing fixed-time policy (`host_framerate 0.015`) and a
wait after each capture. `material-support-fixed-game/evidence.json` passes;
visual inspection confirms the requested dropper and cracked-glass views.
The portal view is also retained. This is image evidence, not a gameplay-frame
performance measurement. The current game gaps still include particle cards,
depth blending, post effects, shadow-build/query geometry and live proxy-only
materials. No all-material or R96 completion is claimed.


### Intro4 authored model culling and installed game verification (2026-10-03)

The MDL reader normalizes triangles to counterclockwise front faces. Static and
posed model color/depth draws now apply the same authored culling policy as the
world surfaces: back-face culling unless the material requests `$nocull`.
The private model AO prepass uses that policy too. Dynamic mesh snapshots
continue to use their captured raster state. This removes an accidental default
of drawing both model faces; it does not alter authored two-sided foliage.

The independent posed-model GPU control reverses a model triangle: the ordinary
material must disappear, while `$nocull` must retain it. All 85 checks pass in
`quality-results/intro4-rendercore-completion/model-cull-posed-model.log`.
The final null world/material suites pass 110 and 82 checks, with no skips, in
`model-cull-contracts.json` under the same directory. Style checking passes
19 files with zero failures in `material-support-style.log`.

The strict FSR game repeat uses settled dropper, glass and linked-portal poses.
`model-cull-game/evidence.json` passes with zero claimed-view failures; inspected
`view-0.png`, `view-1.png` and `view-2.png` show the dropper without the false
foliage AO, the cracked window and the linked exit room. The final clean product
installation passes in `material-support-installed-build.log`, and its libraries
and launcher are installed into `run/runtime-p2-fsr`. Running game processes need
a restart. Dynamic cable support remains explicitly opt-in with
`r_core_dynamic_draws 1`; full material/cohort coverage and the outstanding
performance/qualification gates in the preceding record remain open.


### Intro4 decal integration slice in progress (2026-10-03)

User direction selects decals before particle cards. `render.material` owns
DecalModulate's dimensionless texture factors and fog toward the neutral factor;
`render.device.v2` owns its multiply blend. Existing material-system projection,
clipping, lightmap coordinates, polygon offset and ordered geometry capture remain
inputs. The concrete consumer is the game's missing surface markings and
`overlays/ratman_diorama01`, plus lightmapped dirt/moss/signage overlays. The
core-owned decal cohort will be captured without opting in the entire unfinished
dynamic-draw cohort. Native game images remain the visual oracle; independent
GPU blend/fog checks precede this wiring. No completion is recorded yet.

### Intro4 decals, indicator frames and FSR glass capture (2026-10-03)

The concrete game defects are now reproduced with fixed cameras, fixed simulation
time, FSR quality and `r_core_dynamic_draws 0`. The default handoff captures
projected decals, UnlitGeneric emission/sky, Refract and VertexLitGeneric model
surfaces. Other dynamic shader families retain their explicit experimental gate.
The captured native lightmap handle and original UVs travel together; a compiled
stage atlas cannot replace the page beneath an already projected decal.

DecalModulate uses raw dimensionless factors, alpha rejection, fog toward 0.5 and
the device's `kModulate2x` blend, preserving destination alpha. Independent native
GL/Vulkan blend conformance passes 1,047/1,107 checks without skips in
`quality-results/intro4-rendercore-completion/decal-device-contracts.json`.

The cracked-glass hatch was a viewport/attachment mismatch: the scene snapshot
copies the full attachment, while FSR draws into a smaller viewport. Refract now
converts view UV to attachment pixels, including the viewport origin, and clamps
distortion to the rendered viewport. The indicator's orange frame was refused as
an unhandled nonzero `$frame`, although its native texture was already selected.
Unlit and modulate decals accept that caller-owned selector. Normal and environment
images use their own frame selectors during capture.

The native Vulkan posed-model suite passes 97 checks with zero validation messages
in `glass-viewport-gpu.log`, including independent reduced/offset viewport pixel
expectations and 0/1/0 selected-frame texture snapshots. World/material conformance
passes 110/82 checks in `glass-viewport-contracts.json`. The strict game capture
`material-pixel-game/evidence.json` passes; its images show transmitted geometry
instead of the unrendered hatch, blue/orange/blue indicator lights and the modulated
Ratman mural. This establishes these behaviors, not complete glass optics or
complete map/material support.

The installed pixel checker owns this fixed scene's command sequence, regions and
tolerances. `python3 tools/quality/intro4_material_check.py --commands` prints the
capture commands for `portal_boot`. The working verification command is:

```sh
python3 tools/quality/intro4_material_check.py \
  --capture quality-results/intro4-rendercore-completion/material-pixel-game \
  --out quality-results/intro4-rendercore-completion/material-pixels.json
```

It passes 11 checks, including seeded missing-transmission, missing-indicator, missing-decal and hatch
negative controls. Missing captures or core statistics fail. Inputs retain image
hashes and the boot's build/revision evidence. Style passes 26 files; full archlint
still reports the pre-existing CAP002 `public/gameui/graphics_settings_service.h`
`charconv` include, with zero new/stale occurrences. Full material coverage,
remaining proxy/effect cohorts, matched optics and performance acceptance remain
open. All implementation is on `subsystem-refactor`.

Frozen-path: shaderapivulkan changes are material/cohort handoff and texture-frame
defect plumbing into rendercore, explicitly requested for game material support;
the material math and blend remain owned by render.material/render.device.


### Intro4 strict desktop material verification (2026-10-03)

The requested actual desktop game test exposed a defect that the offscreen game
run did not: HDR presentation disabled the swapchain screenshot capability, and
that flag incorrectly disabled scene-color capture on the engine-owned RGBA16F
back buffers. They are created with transfer-source usage independently of the
presented swapchain. The frontend now imports their actual copy capability.
The first strict desktop run failed with three claimed views in
`quality-results/intro4-rendercore-completion/strict-visible-materials-game/`;
the repeated run in `strict-visible-materials-fixed-game/` passed with zero
claimed-view failures. Its console explicitly reports `r_core_world_strict = 1`,
`r_core_world = 1` and `r_core_dynamic_draws = 0`.

The native Vulkan Wayland game used the AMD Radeon 8060S, HDR Rec.2020/PQ
presentation and FSR from 1024x768 to the desktop's 1536x1152 pixels. Actual game
captures cover cracked-glass transmission, blue/orange/blue indicator frames,
and the Ratman modulate decal, with draw-disable controls. The pixel checker
now scales its fixed regions to the actual 4:3 capture without resampling the
image, requires strict/default-cohort startup settings, and rejects any logged
strict-mode disable. Both the original 1024x768 and desktop 1536x1152 captures
pass 11 image checks, including four seeded missing-effect controls. A separate
strict-off negative fixture is rejected in `strict-off-negative.json`.
SDR game screenshot conversion does not qualify presented PQ appearance.

The core surface owner also now documents and respects the actual captured
attachment convention: exposure/fog were applied before capture. Refract and
PBR transmission decode manually encoded UNORM inputs when required, compose
with the completed background without exposing/fogging it twice, and encode the
result once. PBR surface fog uses the complementary transmission weight. Native
GPU regression checks pass 101 posed-model and 39 map-terms cases, with zero
validation messages, in `strict-desktop-posed-model.log` and
`strict-desktop-map-terms.log`. They cover quarter/fourfold exposure and active
fog, including an independent constant-fog energy expectation. The product
build succeeds in `strict-desktop-final-product-build.log`; the renderer binary
hashes match those in the successful desktop run.

A separate interactive instance is left at the cracked glass, preserving the
user's existing game. `live-review/launch.json` records its process/command;
`live-review/glass.png` is its actual capture. Its live queried strict mode is
1, with 7,749 views drawn and zero claimed-view failures at capture. The tested
22 products were installed to `run/runtime-p2-fsr` using atomic file replacement
so existing process mappings stay intact (`strict-desktop-runtime-install.json`).

This is bounded material evidence. Eight census proxy materials and the runtime
`particle/particle_noisesphere` depth-blend refusal still remain; cutout shadow
coverage and complete material/performance acceptance are open. R91/R96 are not
qualified by this result. All changes are on `subsystem-refactor`. The pinned
formatter passes the staged edited regions in four eligible files
(`strict-desktop-owned-style.log`). Concurrent world-transition edits are
preserved separately and currently fail the whole-working-tree style check.
Full archlint still reports the pre-existing CAP002 `charconv` include in
`public/gameui/graphics_settings_service.h`, with zero new/stale occurrences.

Frozen-path: shaderapivulkan changes fix the game scene-image import capability
used by rendercore glass; material math remains owned by render.material.

### Optimization resolution sweep documentation (2026-10-03)

User direction: establish optimization baselines by sweeping 1024×768 through
4K, identify CPU/GPU limits, and do not infer that no low-resolution gain means
no high-resolution gain. RFC 0016's
[resolution-sweep policy](0016-render-core.md#optimization-resolution-sweep-user-decision-2026-10-03)
owns the methodology. AGENTS.md, the platform/jobs/quality and rendering domain
RFCs, temporal/provider guidance, profiling and RenderDoc guides now link it.
The existing opaque-batching receipts retain their 1920×1080 scope.

Baseline and candidate sweeps retain matched complete workloads, CPU/GPU and
presented-frame distributions, per-point gains/regressions, repeat variability
and bottleneck/crossover attribution. The policy accounts for aspect/FOV/LOD,
presentation caps, thermal state and temporal input/output extents. Profile
budgets and the FSR timing exception keep their existing acceptance semantics.

The [collector guide](../tools/quality/render_profile.md#resolution-sweep-before-and-after-optimization)
records the installed limitation: `frame_floor.py` rejects dimensions differing
from a workload's budget row, and offscreen buffers cap at 1024×768. This
documentation adds no sweep runner, timings, hardware support or gate completion.
R90/R95/R96/R91 performance acceptance and missing sweep coverage stay unverified.

Validation: added local documentation links and anchors resolve; Markdown diff
whitespace checks pass. Installed archlint baseline and inventory verification
pass, and all 166 architecture checker fixtures pass. `check --all` still fails
on the existing CAP002 `public/gameui/graphics_settings_service.h` `charconv`
include, with zero new and zero stale occurrences. Full logs are retained in
`/tmp/source-engine-resolution-docs-vLTydh/`; reproduction uses the four installed
architecture commands in [AGENTS.md](../AGENTS.md#working-protocol).
This is documentation-only; no build configuration or runtime was changed.


### Intro4 open doorways and indicator boxes (2026-10-03)

The user's doorway image exposed two gaps missed by the floor-strip checks.
`tools/toolsblack_noportal` is an authored opaque UnlitGeneric area-portal cover;
gameplay changes its `$alpha` to zero when the door opens. The core unlit claim
ignored that constant alpha when selecting blending, leaving an opaque black
cover over the room. Unlit and lightmapped claims now include constant alpha
modulation in blend selection, preserving the material's opaque behavior at one.
The same cover becomes transparent at zero without writing destination alpha.

The square indicator panels use `signage/signage_doorstate`, a proxy-driven
LightmappedGeneric material. The game log refused its non-neutral `$frame`, and
the default frontend cohort omitted moving lightmapped brush surfaces. The family
now accepts the caller-owned selected frame; the frontend captures that brush
cohort after proxies run, with its selected texture handle and native lightmap.
No baked material or gameplay/entity behavior was changed.

The native GPU posed-model suite passes 108 checks with zero validation messages
in `quality-results/intro4-rendercore-completion/door-box-gpu.log`. Seven new
checks prove zero/one gameplay fade for both families and the selected emissive
panel's 0/1/0 images against independent texture/emission/lightmap expectations.
World/material conformance passes 110/82 checks in `door-box-contracts.json`.
The product build and edited-source style checks pass. Full archlint retains the
existing CAP002 `charconv` include, with zero new/stale occurrences.

Actual Wayland/HDR/FSR game captures in `door-box-paired-game/` cover both chamber
doors closing/opening/closing and the two boxes receiving different frame indices,
then swapping and returning. Strict mode is queried as 1, dynamic draws as 0;
42,412 views were queued with zero claimed-view failures. The installed oracle
passes 25 checks including six seeded opaque-cover, missing-box, missing-glyph
and stale-shared-frame controls. Its working check is:

```sh
python3 tools/quality/intro4_material_check.py --scene doors \
  --capture quality-results/intro4-rendercore-completion/door-box-paired-game \
  --out quality-results/intro4-rendercore-completion/door-box-pixels.json
```

`--scene doors --commands` emits the complete capture sequence for `portal_boot`;
`door-box-paired-command.json` retains the actual launch arguments and the boot
retains binary hashes. The original material oracle still passes all 11 checks.
The tested 22 products were installed atomically to `run/runtime-p2-fsr`; a new
interactive strict instance is open at the paired doorway (`door-box-live-review/`).
Existing processes were preserved and retain their loaded binaries until restart.

This fixes the observed doorway cover and panel-frame behavior. Remaining effect,
proxy, cutout-shadow, whole-map material coverage and performance gates remain
open. No optimization timing or resolution-sweep claim is made.

Frozen-path: shaderapivulkan only hands moving lightmapped brushes to the core;
constant-alpha blending and selected-frame interpretation stay in render.material.

### Intro4 captured lightmap handles and strict paired loads (2026-10-04)

The reported strict crash occurred while loading Intro4 from the menu. Its
retained native core dump identifies recorded slots 9 and 10 as
`overlays/overlay_moss01` and `decals/moss_wall_decal`. Both captured lightmap
handle 197; their shared draw binding was invalid. The Vulkan frontend stored
its zero-based managed-texture index in `CoreMeshDraw`, while `ICoreTextures`
imports material-system handles by subtracting one. This selected the preceding
image rather than the bound lightmap. Whether that neighboring image happened
to be importable explains why earlier boots could pass without correct sampling.
The bridge now converts the index back to the one-based handle (198 in this
capture), with zero for the neutral page. Coordinates remain the captured page's
coordinates. The corrected field contract is documented at the existing owners.

The world pass now removes failed draw bindings from its cache. Previously the
first import failure had a cause, but subsequent attempts reused the invalid
binding and overwrote the reported cause with the generic claimed-view error.
Failed imports remain strict failures; a subsequent host upload can recover
without replacing the world. Deterministic W6b checks repeated named failures,
the exact captured handle, rejection of the adjacent handle, and recovery.
World/material conformance passes 113/82 checks (`strict-load-contracts.json`).
The native GPU posed-model suite, including the captured overlay lightmap and
decal expectations, passes 108 checks with zero validation messages
(`strict-load-gpu.log`). The installed product build passes
(`strict-load-product-build.log`).

The installed [strict paired game runner](../tools/quality/render_profile.md#strict-intro4-material-captures)
loads `sp_a1_intro4_relit` from the running menu in both modes, waits through
initial spawn, and exercises glass/decals, indicator frames, two doorways and
ropes. FSR-on uses scale 0.5; FSR-off launches without the FSR provider arguments
and requires scale zero. Both query strict mode, default cohorts and temporal
scale; neither mode can substitute for a failed or missing mode. The same built
native Vulkan product passes all 45 pixel and seeded-negative checks in each
mode under `strict-load-fixed-game/`, with zero claimed-view failures. A second
fresh pair in `strict-load-fixed-repeat-game/` also passes 45 checks per mode
with zero failures: four successful strict loads and 180 total pixel/negative
checks. A wrong-scale negative receipt (`strict-dual-wrong-mode-control.json`)
is rejected rather than certifying the wrong FSR mode.
These runs use Box3D, the launcher's job settings and a deterministic fixed
game timestep; requested windowed 1024x768 becomes desktop 1536x1152. They do not
qualify the user's fullscreen timing, presented PQ appearance or performance.
The actual crash dump establishes the faulty binding independently of these
bounded successful captures. Core-dump diagnostics are retained locally under
`quality-results/intro4-rendercore-completion/`; the memory image is not a
checked-in fixture.

The tested 22 installed products were atomically replaced in
`run/runtime-p2-fsr`, with hashes matching both successful mode receipts
(`strict-load-runtime-install.json`). Running processes preserve their existing
mappings until restart. The edited C++ regions pass the pinned style checker
(`strict-load-style.log`); all 73 boot-runner fixtures pass. Architecture baseline
and inventory verification pass. Full archlint still reports the existing CAP002
`public/gameui/graphics_settings_service.h` `charconv` include, with zero new
and zero stale occurrences (`strict-load-arch.log`).

The preceding cable slice also enters the default core frontend cohort:
Cable/Cable_DX9/SplineRope use the existing core interpretation of normal UV0,
base UV1, stored normal-blue squared and captured linear vertex illumination.
The real game `r_drawropes 1/0/1` capture passes nine checks, including three
seeded missing-rope/failed-return/full-region-darkening controls
(`cable-visibility-game/`, `cable-pixels.json`). Wind changes its trajectory, so
the oracle measures the rope footprint without requiring identical on/on pixels.

RFC 0016 owns the new mandatory strict FSR-on/off integration rule; AGENTS.md
links it. The guide records the installed reproduction command. The complete
Intro4 effect/proxy/cutout-shadow and performance gates remain open; R91/R96
remain partial. Eight census proxy refusals and the soft-depth particle gap
remain. These changes are on `subsystem-refactor`.

Frozen-path: shaderapivulkan fixes captured lightmap handle plumbing and hands
rope geometry to the existing core material owner; no legacy shading is added.


### Intro4 indicator emission on core receivers (2026-10-04)

User direction: emissive surfaces should light their surroundings like the
fizzler, including indicator boxes and lines. The previous world producer
sampled frame zero, omitted moving brush geometry and excluded fullbright
indicator overlays because they have no `$selfillum` flag. The
[surface-source policy](0016-render-core.md#surface-emission-sources-installed-indicator-slice-2026-10-04)
owns the new source behavior and radiance scale.

The core-owned mapped-triangle integration samples the selected source image,
includes authored coverage and preserves emitted power when fitting area
rectangles. The geometry bridge preserves the existing area-light interfaces;
world/overlays are enumerated once, excluding the world entity's model alias.
Brush sources follow entity transforms. The client's existing source publisher
reads each overlay's actual proxy owner frame and publishes core-only lights.
It neither reruns proxies nor allocates legacy CPU stand-in slots. Static mask
images retain their independently bound frame zero; invalid base frames are
reported instead of wrapping to another image. Models retain their existing
geometry/skin/bone producer, with the new core light-radiance scale.

Evidence under `quality-results/intro4-rendercore-completion/`:

- `emissive-final-contracts.json`: 45 checks pass; the existing two-sided and
  power-loss seeded defects are detected by their sensitivity suites.
- `emissive-gpu-images.log`: 76 receiver checks pass, zero validation messages.
  The new mapped cyan/orange/transparent/return cases exercise the same source
  integration on real GPU receivers; images are in `emissive-lab-images/`.
- `emissive-product-build-final.log`: installed product build passes using the
  existing FSR profile, without reconfiguring another session's build.
- `emissive-strict-final-game/`: the final product runs Intro4 from the menu in
  strict FSR-on (scale 0.5) and FSR-off (scale 0) modes. Both pass all 75 pixel
  and seeded-negative checks with zero claimed-view failures. The 30 new checks
  judge neighboring wall receivers beside the box and floor line through
  cyan/on, cyan/off, orange/on, orange/off and cyan/on again. Emitting textures
  remain visible during the light-only off control. Eight deliberate missing
  light/stale-frame defects are rejected. Existing glass, decals, door and
  cable checks remain effective. `emissive-strict-game/` retains the first
  passing pair before the final frame-sampling correction.
- `emissive-runtime-install.json`: both final captures identify the same 22
  product files; those tested products are atomically installed into the shared
  `run/runtime-p2-fsr`. Running games retain their existing file mappings.
- `emissive-style-final.log`: edited C++ style passes. Architecture baseline and
  inventory are current. The full architecture check retains the pre-existing
  CAP002 `charconv` occurrence in `public/gameui/graphics_settings_service.h`,
  with zero new/stale occurrences. All 73 boot-runner fixtures pass.

The installed strict runner now includes this emission sequence in `all`,
75 checks per mode; its [guide](../tools/quality/render_profile.md#strict-intro4-material-captures)
records reproduction and controls. This is bounded source/receiver evidence,
not completion of R91/R96, all material/proxy support, indirect-light transport
or the complete-frame performance gate. Advanced UV/emission inputs and animated
studio-source images remain open. Changes are on `subsystem-refactor`.

Frozen-path: engine overlay/world code only supplies authored geometry, facing
and live proxy-owner identity; the versioned area-light bridge and client root
are core plumbing. VTF frame decoding fixes source image selection. Source math
and receiver shading retain their render-core owners; no legacy shader is added.


### Intro4 signage emission on core receivers (2026-10-04)

User direction extends the indicator slice to other signage. Intro4's compiled
material audit identifies seven signage VMTs (`signage-materials.json` under
`quality-results/intro4-rendercore-completion/`): door-state boxes, floor/corner
indicator lines, exit and arrow signs, and the box-hurt/box-dispenser floor
pictograms. The first three retain the preceding indicator coverage. The new
actual-game sequence isolates the other four materials, chamber information
boards and elevator movie screens. The existing
[surface-source policy](0016-render-core.md#surface-emission-sources-installed-indicator-slice-2026-10-04)
owns the shared light-only scale and the new isolation control.

Two defects were observed and corrected:

- World panels published their previous scale-one light and allocated CPU
  compatibility slots. They now apply the shared core source policy after
  cached image integration, and progress/movie panels publish core-only light
  when the core is selected. Their visible image and coatings retain the
  existing panel owner. Retained rendering keeps its previous routing/scale.
- Exit and arrow faces published light facing into the wall. Their compiled
  face plane already points west; `SURFDRAW_PLANEBACK` records the selected
  flipped plane. The new geometry bridge applied that flip again. World and
  overlay ingress now use the already oriented plane. Sources remain one-sided.
  Slotless light diagnostics now report facing, area and sidedness, matching
  the existing slotted-source report.

The first isolated captures showed virtually no exit/arrow receiver light;
corrected captures show positive light on a metal-box receiver in front of each
sign. Floor pictograms are partly covered by authored debris, which provides
receivers for their emitted light. The chamber board lights its metal border;
the elevator movies light the surrounding wall and floor. An exact-material
light filter prevents an indicator or another bright source from masking a
missing contribution. It is reset before the final live queries.

Evidence in `quality-results/intro4-rendercore-completion/`:

- `signage-contracts.json`: 45 area-source checks pass; both existing seeded
  two-sided/power-loss suites detect their defects.
- `signage-area-gpu.log`: 76 checks pass, zero validation messages.
  `signage-panel-gpu.log`: 77 checks pass, zero validation messages.
  `signage-panel-oracle-selftest.log`: 18 panel-judge checks pass.
- `signage-product-build-final.log`: installed product build passes using the
  existing FSR profile.
- `signage-facing-fixed-game/` and `signage-facing-fixed-pixels.json`: the
  isolated strict FSR game passes 52 signage checks after the facing correction.
- `signage-strict-final-game/`: the final product loads Intro4 from the menu
  in strict FSR-on (scale 0.5) and FSR-off (scale 0) modes. Both pass all 127
  checks, with zero claimed-view failures. Its 52 new checks cover isolated
  on/off/on receiver light, restoration, visible exit/arrow patches, source
  facing and core-only routing. Fourteen missing-light/reversed-front/legacy-slot
  seeded defects are rejected. The previous 75 glass/decal/door/rope/indicator
  checks also pass. Requested viewport is 1024x768; desktop HiDPI captures are
  retained at their native pixel dimensions.
- `signage-runtime-install.json`: both mode receipts identify the same 22
  products, atomically installed into `run/runtime-p2-fsr` after the paired
  runs pass. Existing games retain their mapped binaries until restart.
- `signage-style-final.log`: all eight edited C++ files pass. Architecture
  baseline/inventory verification passes. `signage-arch-final.log` retains the
  pre-existing CAP002 `charconv` occurrence with zero new/stale occurrences.

The [installed runner guide](../tools/quality/render_profile.md#strict-intro4-material-captures)
now includes `signage` in `all`, 127 checks per mode. This verifies the named
Intro4 sources and receivers; R91/R96, advanced source inputs, animated studio
source images, full occlusion/indirect transport and complete-frame performance
remain open. These fixes are on `subsystem-refactor`.

Frozen-path: engine changes correct authored geometry facing and expand light
diagnostics; client changes are core source-policy/routing plumbing explicitly
requested for signage. Existing panel image integration and render-core receiver
shading retain their owners; no legacy shading implementation is added.


#### Installed signage screenshot confirmation (2026-10-04)

Follow-up user request: take screenshots to confirm. Fresh actual-game captures
in `quality-results/intro4-rendercore-completion/signage-screenshot-confirmation/`
use the installed `run/runtime-p2-fsr` directly, without a build override, in
strict FSR-on (0.5) and FSR-off (0) modes. Both boots and all 52 signage checks
per mode pass, with zero claimed-view failures. Installed product hashes still
match the preceding tested/deployed products.

Visual inspection of all twelve on/off comparisons confirms brighter cube
upper faces beneath exit/arrow signs, lit debris beneath the floor pictograms,
the chamber board's illuminated metal border and cyan light on elevator walls
and floor. The two floor sources are partly hidden by authored debris; their
receiver contribution is visible. The exit/arrow scenes use a placed receiver
cube. The film animates between captures; its surrounding receivers are judged.

`review/index.html` contains all six source comparisons in both modes.
`review/evidence.json` records source screenshot hashes, lossless native PNGs
and comparison paths: 36 on/off/restored source captures, 1536x1152 pixels each.
Comparisons preserve captured colors and add only an external label band.
These images confirm the named source/receiver behaviors; broader material
and transport completion remains open.


#### Security-camera eye sources (2026-10-04)

User request: security-camera lights must illuminate their surroundings through
rendercore, like the fizzler, with strict FSR/non-FSR game screenshots.
Intro4's three `npc_security_camera` entities use skin 1, whose
`models/props/camera_skin02` material has no self-illumination. Their red eye is
an `env_sprite` using `sprites/glow1.vmt`, attached to `light` on
`models/props/security_camera.mdl`. Model-material emission alone misses it.

The client now publishes a copied core-only area source for this named cohort.
The core's `AttachmentEmitter` builds and validates its one-sided aperture;
client source policy supplies its physical size and linear radiance. Source
geometry follows the current attachment, independently of the halo billboard.
Live RGB, interpolated brightness/scale and HDR scale are read from the existing
sprite owner. Hidden/dormant/removed eyes stop publishing; no sprite pointer is
retained across frames. Missing required attachment data fails by name. The
[radiance/aperture policy](0016-render-core.md#surface-emission-sources-installed-indicator-slice-2026-10-04)
has one authoritative definition, including the `@camera-eyes` isolation control.

The first actual game comparison exposed self-shadowing: head-local attachment
Z 12.9 lies inside its shadow hitbox's maximum 15.751. The reviewed aperture is
placed 3 units forward, just outside that existing proxy. Camera pose, visible
eye and all shadow settings are preserved. This is a bounded accommodation of
the current hitbox caster, not full studio-mesh shadow fidelity. A separate
shadow diagnostic retained in `camera-shadow-diagnostic/` established that the
moving caster blocked the contribution; shipped captures keep quality 3 and
moving shadows enabled. The final receiver shows small red light beneath the
eye. This source policy does not turn every optical glow sprite into a lamp.

The deterministic camera fixture adds 45 checks to `all`: receiver red chroma,
coverage, restoration, hide/show, move/return, turn/restore, removal, the three
source centers/fronts/radiance, core-only routing and enabled shadow settings.
Fifteen seeded missing/stale-light, stale-position, reversed-front, missing-
radiance, legacy-slot, disabled-shadow and missing-report defects must fail.
The earlier door fixture's active fizzler is explicitly verified and preserved
in `all`; the isolated camera fixture has no such extra source. The client also
honors a requested area-light report while the set stays empty; previously its
request could survive until a later nonempty frame and report stale state.

Evidence under `quality-results/intro4-rendercore-completion/` so far:

- `camera-source-policy.json`: model/sprite hashes and measured attachment/
  hitbox facts; red radiance 8.031 at authored brightness 128 and policy 16.
- `camera-contracts-final.json`: 53 checks pass, plus both seeded power-loss/
  two-sided suites match their expected outcomes; zero skipped suites.
- `camera-area-gpu-formatted.log`: 82 GPU checks pass, zero validation messages.
  The new six-state attachment test compares dimmed light against the exact
  irradiance oracle, including the changing reach window.
- `camera-final-product-build.log`: final installed product build passes.
- `camera-intermediate-review.json`: both original full strict captures pass
  the revised 172-check oracle. The initial report-only failure incorrectly
  assumed the prior door fixture's active fizzler was absent; images and core
  drawing passed. A fresh final paired run is retained separately.
- `camera-style-shipping.log`: four edited C++ files pass. Architecture
  baseline/inventory verification passes; `camera-arch.log` retains the
  pre-existing CAP002 `charconv` occurrence with zero new/stale occurrences.

Final evidence:

- `camera-strict-confirmed-game/reviewed-evidence.json`: final-product menu-to-
  Intro4 boots in strict FSR-on (0.5) and FSR-off (0), 172 checks pass per mode,
  zero claimed-view failures. All previous 127 glass/decal/door/rope/indicator/
  signage checks pass alongside the 45 camera checks. Native captures remain
  unchanged. One non-FSR console line joined a frame marker and its report;
  the parser's newline assumption was corrected without changing any geometry,
  radiance, pixel or negative-defect requirement. The original driver/report
  failure remains retained. `camera-report-layout-selftest.json` proves both
  whitespace layouts and all six report-defect controls in both modes.
- `camera-formatted-product-build.log` and `camera-style-final-shipping.log`:
  final formatting and product build pass. `camera-format-binary-equivalence.json`
  proves every allocated client ELF section (code/data/symbols/relocations, at
  the same addresses) matches the full paired capture; only debug/build-id data
  differ after the final expression wrap.
- `camera-runtime-install.json`: 22 products installed atomically, with the
  paired proof and formatting equivalence recorded. Existing process mappings
  remain untouched until restart.
- `camera-installed-final-game/evidence.json`: fresh tests of the installed
  runtime, no build overlay. Both strict FSR-on/off boots pass all 45 camera
  checks, including an actual empty source report during the light-only control.
  Both have zero claimed-view failures and retain shipped shadow settings.
- `camera-installed-final-game/review/index.html` and `review/evidence.json`:
  20 lossless native PNGs (1536x1152), full and cropped on/off comparisons in
  both modes, source screenshot hashes and original-pixel crops. Visual review
  confirms small red light on the cube's upper surface, its disappearance and
  restoration through the tested lifecycle/pose controls. Colors are unchanged;
  only the external comparison labels are added.

The installed runner now allows omitting `--build` to test the deployed runtime
and includes `cameras` in its default `all` sequence (172 checks per mode).
R91/R96, general glow-sprite source policy, full studio shadow fidelity,
indirect transport, full-frame performance and broader material completion
remain open. Changes are on `subsystem-refactor`.

Frozen-path: core plumbing for the user-requested camera source ingress and
empty-set diagnostic correction. Core receiver shading retains its owner;
no legacy shading implementation is added.


### Intro4 soft-particle depth fade (2026-10-04)

The next actual dynamic refusal was `particle/particle_noisesphere`: 230 draw
packets in the retained full-game route needed `$depthblend`. The core unlit
point now implements its positive finite `$depthblendscale`, vertex alpha and
ordered scene-depth input. The shader compares homogeneous clip Z against the
copied depth encoded in alpha with the producer’s captured range (192 in this
product); it retains the encoded far-range fade. Invalid scale/range or a
missing input refuses the view before claiming it. A lost import fails the
claimed view by name. The emissive model point still refuses this particle-only
feature. Depth groups retire behind the frame’s completion token.

Game integration exposed a real failure after the initial lab pass: core-only
replay discarded the depth-alpha framebuffer copy. The sprite rendered and
obeyed opaque depth testing, but scene alpha stayed 1 and intersection fading
never happened. The adapter now retains this ordered core input, treats it as
a depth read when selecting attachment store operations, and merges copies
only when their source, destination, rectangles, depth intent, projection and
range agree. The resampled depth texture uses coordinates normalized by the
full source attachment; FSR’s viewport covers a subregion of that attachment.

The fixture uses the actual noise material in an `env_sprite` at Intro4 BSP
face 1620, an opaque x=256 wall. It explicitly commits each sprite position
through `SetOrigin`; `ent_create` otherwise relocates it to its traced spawn
position. Seven original captures cover background, near/middle/far gaps,
behind the wall, return and removal. Six physical checks plus six seeded
missing/fading/occlusion/stale-state defects run in each strict mode. The full
`all` route now includes these captures (184 checks per mode).

Evidence under `quality-results/intro4-rendercore-completion/`:

- `softparticle-final-control.log`: 59 GPU checks pass, zero validation
  messages, with independently evaluated encoded depth and fade at four
  gaps, three scales, enabled/neutral states and full/half-size viewports.
- `softparticle-final-sensitivity.log`: control plus four seeded broken
  shaders pass their expected outcomes. Missing fade, wrong range, texture
  dimensions and viewport dimensions must each be detected.
- `softparticle-final-copy-decisions.log`: production replay decision code
  passes; five seeded copy-loss/retention/merge defects are rejected.
- `softparticle-transition-regression.log`: all three prior UI capture-alpha
  regression tests pass, preserving orthographic tile opacity.
- `softparticle-copy-replay-game/evidence.json`: both strict FSR-on (0.5)
  and non-FSR (0) targeted game runs pass all 12 checks. Native screenshots
  and lossless crop comparisons are retained in `review/`; visual review
  confirms the near/middle/far transition, wall occlusion and restoration.
- The initial construction, coordinate and discarded-copy failures remain
  in `softparticle-wall-*`, `softparticle-viewport-game/`,
  `softparticle-source-extent-game/` and `softparticle-diagnostic-game/`.
  They are failed evidence, not accepted coverage. Temporary shader colors
  and logging were removed before the corrected paired run.
- `softparticle-final-product-build.log` and `softparticle-final-lab-build.log`
  retain successful final builds. `softparticle-style-final.log` checks 15
  edited C++ files with zero failures. Baseline/inventory verification passes;
  `softparticle-final-arch.log` retains the pre-existing CAP002 `charconv`
  occurrence with zero new/stale occurrences.

This is a bounded material slice. `effects/spark`’s `$brightness` and loaded
materials requiring live proxies remain reported refusals. General material
completion, R91/R96 and whole-frame performance acceptance remain open.

Frozen-path: core plumbing and replay defects for the ordered scene-depth
input; all new particle shading stays in the core surface program.

The final full-route verification also corrected two fixture defects without
relaxing image thresholds. The elevator movie now uses the game’s existing
pause/unpause behavior while comparing on/off/on receiver light; advancing
frames change source radiance, so equal wall lighting was not a valid prior
restoration expectation. The original full route’s movie-return failures in
both modes remain in `softparticle-final-strict-game/`. A signage report can
share a line with its capture marker; its parser now accepts both whitespace
layouts. `softparticle-signage-layout-selftest.json` checks six real captured
log/layout combinations and their seeded bad sources; the installed
`test_signage_report_layout.py` preserves those requirements. New paired runs
save the exact oracle and its hash before booting either mode.
The material claim reports its depth dependency to the pass, keeping family
interpretation under the material owner. `softparticle-owner-*-build.log`,
`softparticle-owner-control.log`, `softparticle-owner-sensitivity.log` and
`softparticle-owner-style.log` verify that final ownership change.


Final private product proof is `softparticle-paused-strict-game/reviewed-evidence.json`:
184 checks pass in each strict mode, with zero claimed-view failures. The
original aggregate failure is retained separately: its frozen older parser
rejected one joined source-report line in non-FSR. `oracle-parser-review.diff`
proves the reread changed only that whitespace assumption; source facing,
counts, radiance, pixel thresholds and seeded defects are identical. Its
`review/` retains 130 lossless native PNGs, original screenshot hashes and
camera/particle/movie comparisons. Visual review confirms receiver light,
wall-intersection fade and movie restoration in both modes.
`softparticle-runtime-install.json` records atomic installation and verified
hashes for all 22 products from both proved builds. A fresh installed-runtime
full paired run, with no build overlay, is being collected separately in
`softparticle-installed-strict-game/`; it is not certified until both modes
complete successfully.


Installed verification completed: `softparticle-installed-strict-game/evidence.json`
passes all 184 checks in each mode with strict enabled and zero claimed-view
failures, using no build overlay. `installed-product-proof.json` independently
checks all 22 staged binaries against the installation receipt in both modes.
Its `review/` retains 130 lossless native PNGs and camera/particle/movie
comparisons with source screenshot hashes. This confirms the deployed soft-
particle fix alongside the prior glass, decals, doors, indicator, signage and
camera checks. It does not close the remaining spark/proxy/material gates.
The code is committed on `subsystem-refactor` as `0f501d2ba`.


### Intro4 additive spark material (2026-10-04)

The remaining seven actual dynamic refusals were `effects/spark`. Its VMT has
`$brightness "effects/spark_brightness"`, but UnlitGeneric, UnlitTwoTexture
and Sprite do not declare or read that key. The importer’s existing family-
scoped metadata table now records that fact for the unlit point. It preserves
the authored additive/base-texture/vertex-color image; it does not invent a
brightness-texture operation or ignore arbitrary unknown settings. No legacy
shader is changed. Reader/content facts are retained in
`remaining-proxy-spark-content.json` and `spark-brightness-reader-audit.json`
under `quality-results/intro4-rendercore-completion/`.

The particle lab now compares the additive point with/without that unused
key against the independent white-quad oracle, and requires an unknown
`$brightness_mystery` to refuse by name. `spark-rejected-control.log` proves
omitting the metadata fix fails three checks (63 total); the initial incomplete
before-mapping run is retained separately and does not certify a negative.
`spark-positive-control.log` passes 64 checks with zero validation messages;
`spark-positive-sensitivity.log` passes the control and four shader defects.
The pre-existing soft-particle math and input/lifetime checks remain included.

The actual game fixture uses `effects/spark.vmt` in an `env_sprite` at the
same Intro4 wall. Seven native captures cover background, visible, hidden,
shown, behind the wall, returned and removed. Six physical checks and six
seeded image defects must pass. `spark-first-game/evidence.json` passes all
12 checks in both strict modes, with zero dynamic refusals; native comparisons
remain in its `review/`. Visual review confirms the authored narrow additive
spark, opaque wall occlusion and lifecycle restoration.
`spark-full-strict-game/evidence.json` passes all 196 full-route checks per
mode, preserving the earlier glass/decal/door/indicator/signage/camera/soft-
particle checks, with zero claimed-view failures and dynamic refusals.
`spark-runtime-install.json` records atomic installation of the 22 exact
proved products. The installed target run `spark-installed-game/evidence.json` passes all
12 checks per mode with no build overlay. Its `installed-product-proof.json`
verifies all 22 staged binaries against the installation receipt; `review/`
retains 14 lossless native PNGs and both seven-state comparisons with source
hashes. Visual review confirms visible, hidden, occluded, returned and removed
states in both modes. The full private run retains 144 native PNGs in its
`review/`. Final lab source passes `spark-final-control.log` (64 checks) and
`spark-final-sensitivity.log` (five outcomes). The core importer/lab edits pass
`spark-final-style.log`; archlint retains the pre-existing CAP002 `charconv`
include with zero new/stale occurrences (`spark-arch.log`).

Loaded catalog materials requiring live proxies are still reported separately.
Intro4 really places two `lab_monitor_pose03` static props, skins 4 and 3, at
(928,-756,328) and (980,-756,332); their live handoff is the next review target.
The four missing precached MDLs are unused HL2/editor defaults (`w_bullet`,
`agibs`, `v_hands`, `axis_helper_thick`), rather than geometry-parser failures;
they remain named missing-content facts, not successful model coverage.
Broader material/model completion and R91/R96 remain open.


### Intro4 live monitor matrix handoff (2026-10-04)

The real map places two `lab_monitor_pose03` props. `monitor-skin-facts.json`
records the MDL hash, skin table and mesh references: only materials 0 and 1
are meshed; skins 4 and 3 select `lab_monitor_blank` and `lab_monitor_off`.
The five monitor proxy materials loaded into the immutable catalogue are not
five missing active screens. Static/posed snapshots refuse live proxies and
send those meshes through the existing frontend, which captures current proxy
results into core draws. Legacy shading remains suppressed on this profile.

A real screenshot oracle found that handoff was incomplete: the scanlines were
visible but native frames never changed. `monitor-strict-game/evidence.json`
retains the failing paired test. `CMaterialVar::GetStringValue()` prints
matrix columns with three decimal places, while VMT input and the core reader
use rows. Translation was dropped and rotations transposed. The shared
`public/render/material/vmt_matrix.h` formatter now preserves row order and
float round-trip precision. Both the initial material/default capture and the
live mesh capture use it. The legacy material variable implementation remains
unchanged; the new header avoids the engine's global `render` name conflict.

`proxy-matrix-final-lab-control.log` passes 108 posed-model checks, including
current dynamic two-texture snapshots serialized by the same formatter, before
product integration. `proxy-matrix-final-roundtrip.log` compiles the actual
formatter and rejects seeded column-major and three-decimal defects, including
rotation, unequal scale, translation and a sub-millipixel offset.
`monitor-fixed-strict-game/evidence.json` passes seven physical checks and four
seeded pixel defects in each strict mode. The live image delta is 0.011741 in
FSR and 0.032257 in native, against the unchanged 0.002 threshold; before the
fix it was 0.000231 and zero. Both screens must disappear with static props
hidden and reappear after restoration. The native `review/` retains eight PNGs,
source hashes and both four-state comparisons. Visual review confirms the
moving band/scanlines and the map's authored blank screen.

`proxy-matrix-runtime-install.json` verifies atomic installation of all 22
products against the paired monitor builds. The full installed regression
`proxy-matrix-installed-full-game/evidence.json` passes all 207 checks in each
strict mode, with zero claimed-view failures and dynamic refusals. It uses no
build overlay. `installed-product-proof.json` verifies all 22 staged binary
hashes in both modes against that installation receipt. Its `review/` retains
152 lossless native PNGs and camera/particle/spark/movie/monitor comparisons
with source hashes; the prior glass/decal/door/indicator/signage/camera checks
remain included. Style passes four
C++ files (`proxy-matrix-final-style.log`); baseline and inventory verification
pass, and archlint retains the existing CAP002 `charconv` include with zero
new/stale occurrences. `intro4-entity-model-usage.json` records all 66 authored
MDL entity references. Portal emitters do occur in the map; their blue/orange
proxy states remain the next explicit review target. The potato bodygroup is
hidden by the game's initial `m_bShowingPotatos = false` state. Broader complete
material/model and cutout-shadow coverage remains open; R91/R96 is not promoted.

Frozen-path: defect fix in `shaderapivulkan` mesh capture: serialize the actual
live matrix into the core's existing VMT input convention. No legacy shader or
material-variable behavior changes.


### Intro4 portal-emitter live material proof (2026-10-04)

The real `portal_emitter_a_lvl3` at (-448,0,56) uses `portal_emitter.mdl`.
Its skin table selects the off/blue/orange material on the same mesh reference.
The map's relay outputs flicker orange skin 2 and off skin 0; blue skin 1 uses
the companion live Sine proxy. `portal-emitter-strict-game/evidence.json`
passes 23 checks per mode on the installed runtime, with no build overlay:
15 physical checks cover authored color, live pulse, floor receiver light,
source-off states, color return and removal of stale glow; eight seeded images
must be rejected. The floor regions contain neither source pixels nor bloom.
The fixture isolates each emitter material with the existing source filter,
then restores the filter. Native `review/` contains twenty lossless PNGs and
both ten-state comparisons with source hashes. Visual review confirms both
colors, pulse, source-only light changes on the floor, and the off skin.
`installed-product-proof.json` verifies all 22 binaries against the existing
matrix-fix installation receipt. No game or shader change was needed here;
the actual core live handoff supports both proxy materials.

The common game oracle now also requires dynamic census reports and zero
refused live draws. A material declined before claiming a view must not hide
behind zero failed claims. `portal-emitter-core-statistics.log` passes four
unit tests, including missing reports, a refused live material and an earlier
failure followed by a clean report. Original immutable oracle/capture receipts
are preserved; `portal-emitter-core-statistics-review.json` independently
validates all four existing full/target logs under the stronger census rule,
recording each log hash. The all-scene command route now declares 230 checks;
that combined route has not yet been run. The prior full 207 checks and these
23 installed target checks are separate evidence, not a claimed combined run.

The loaded eight proxy snapshot refusals now have explicit live-game evidence
for the selected monitor and portal-emitter meshes; the other monitor skins and
hidden potato bodygroup remain loaded catalogue entries. This does not certify
full model/material coverage. The current installed runtime still reports 43
world triangles omitted from its solid shadow caster and 23 cutout-only static
props omitted; classification includes translucent surfaces, so these numbers
must not be mislabeled as exclusively foliage. Alpha-tested foliage requires
its actual cutout shadow program before the overall goal can close. Portal
aperture regression and the remaining model audit are also still open.

### Intro4 portal aperture regression (2026-10-04)

The installed portal-winding fix now has a repeatable actual-game aperture
fixture in `intro4_material_check.py --scene portals`, carried by the paired
strict-game runner. Four fixed-camera shots cover the entry wall, a linked
exit room, deactivation and reopening. The oracle checks the exit-room camera
silhouette and wall replacement inside the aperture, stable pixels outside
it, restoration of the wall on closing and restoration of the exit camera
on reopening. It excludes the animated arrow from the restoration metric;
that source legitimately pulses between captures. Eight physical checks and
six seeded image defects reject opaque/placeholder interiors, stale room
pixels, missing reopened content and writes outside the aperture.

`quality-results/intro4-rendercore-completion/portal-aperture-strict-game/`
passes all 14 checks in each strict FSR-on/off mode. No product overlay is used.
`installed-product-proof.json` verifies all 22 installed products against the
matrix-fix installation receipt. The `review/` retains eight lossless native
PNGs, both four-state comparisons and source hashes. Visual review confirms
the exit room, closing and reopening in both temporal modes. The common
statistics suite passes four unit tests in `portal-aperture-core-statistics.log`;
Python compilation and changed-file style checking pass (no eligible C++ change).

The diagnostic `caster-material-audit.json` reads the installed BSP2's carried
legacy data, model material references and placement skins, with winning pak/
custom/content VMT inputs and hashes. It confirms eligible foliage uses texture
alpha and authored `$treesway`, and perforated railings use alpha test. Some
foliage and glass have explicit no-shadow flags. This diagnostic is a superset
of drawn meshes: it does not parse VTX strip ranges or establish the current
core caster census. Coverage/animation and the material claim belong to the
material owner; shadow depth must consume that coverage, retain required image
resources through GPU completion and use the visible foliage deformation.
Opaque silhouettes or static deformed foliage would not satisfy this boundary.
The existing shared `tree_sway.glsl` owns that deformation math. These remaining
shadow and complete model/material gates stay open. R91/R96 is not promoted.

`portal-installed-full-game/evidence.json` subsequently passes the combined
244 checks in each strict FSR-on/off mode, including the portal-emitter and
aperture fixtures after every earlier material/state change. Both boots
report zero failed claims and zero refused live draws under the common census
gate. `installed-product-proof.json` confirms the same 22 installed binaries
in both modes with no product overlay. `review/` retains 180 native PNGs,
portal/emitter and earlier cohort comparisons, and hashed source captures.
This is a combined installed run, superseding the earlier unrun 230-check
route; it does not close the remaining cutout-shadow or model audit gates.


### Material-owned cutout shadow preparation (2026-10-04, game migration open)

`SurfaceProgram::ShadowPipeline` now prepares a depth-only atlas point from a
resolved surface. The alpha calculation and authored threshold remain in
`surface_program.glsl`; world/model/flat shadow vertex entries include the same
vertex programs, including `tree_sway.glsl`. Color outputs and the lighting-only
lightmap-offset varying are absent from the depth interfaces. The atlas uses
single-sample D32 independently of the visible point's target/sample count.
Blended/transmitting, portal-mask and decal-modulation points refuse this opaque
shadow contract. No second VMT/coverage evaluator was added to the shadow pass.

`ShadowDepthRenderer` accepts prepared `ShadowMaterial` draws, validates required
bindings and mesh stride before adding graph work, declares their bound resources,
and retains the existing position-only caster route. Material/frame/draw resources
are borrowed until the graph's completion token. The architecture manifest and
Waf target now declare the pass's dependency on `render.material`. The owning
[coverage contract](0016-render-core.md#one-surface-terms-with-neutral-values)
also requires captured animation inputs and cache invalidation for moving foliage.

The installed `render_lab suite cutout-shadows --validate --verbose` passes 50
checks with silent Vulkan validation. An 8×8 alpha texture and independently
computed triangle/UV/threshold/depth results cover world vertex alpha, unlit UV
transforms, native model cutouts, upright/hanging foliage, rotated/translated
roots, an opaque control and a 0.65 cutoff resolved for a four-sample color target
but drawn into a single-sample atlas. Missing material/frame/view/draw/image
bindings and stride/layout mismatches refuse before graph pass mutation. Three
blended definitions refuse opaque depth. The alpha-ignored shader fails the
coverage oracle; `--sensitivity` passes both verdicts. The independent double
foliage oracle is shared with the existing tree suite rather than duplicated.
Logs are under `quality-results/intro4-rendercore-completion/cutout-shadow-*`.
Existing tree-sway (85), posed-model (108) and shadowed-lights (191) checks pass.
Style checking passes. Architecture checking reports its pre-existing CAP002
`public/gameui/graphics_settings_service.h`/`charconv`, with zero new/stale
occurrences; baseline and loader inventory verification remain current.

`cutout-shadow-shader-game/evidence.json` passes the full 244 checks in each
strict FSR/native game mode, with zero failed claims and refused live draws.
This staged-product capture checks the shared visible shader changes; it does
not exercise the newly prepared depth point. Native screenshots and comparison
panels are retained in its `review/`. Final shadow-only guard/matrix-copy cleanup
is covered by the final GPU fixture, not by this earlier staged binary receipt.
The installed game runtime remains the matrix-fix product already proved in the
prior combined installed capture. The game still omits the recorded cutout-only
casters; the next integration must provide actual material textures, local
geometry/transforms and current wind/time, invalidate animated atlas tiles, and
fail claimed views when required shadow preparation fails. It must prove the
result through strict FSR/native game captures. This is preparation work in
progress, not delivery of foliage shadows or promotion of R91/R96.

A closer camera-eye diagnostic (`camera-eye-query-game/`) exposes an additional
visible-glow gap in both strict temporal modes: the lens stays dark and both
possible/visible pixel counts are zero. The earlier camera source/receiver
checks do not certify the eye sprite. The replay retains query begin/end but
suppresses their non-writing geometry under core-only policy; restoring that
query input and proving visible/occluded eye states is the next bounded fix.

## Intro4 newest-build B refresh (2026-10-04, captured)

The user requested the newest B build in the existing 8K gallery. The configured
Portal 2 product build completed without changing its profile. `--rerun-b` now
captures all 19 selected poses on one B host, validates them in a private directory,
refits the existing original-style SDR conversion against A, and updates the same
`quality-results/map-comparisons/intro4-8k-native-hdr-20261003/index.html`.
The prior B previews, receipts and gallery are backed up under the versioned
`b-reruns/20261004T170729.096874Z/previous-gallery/`; original raw exports remain
in their original boot directory. New HDR links resolve to the new versioned
boot rather than overwriting historical exports. Every A PNG and left receipt
is unchanged.

The captured product snapshot incorporates game changes through `7b6862975`
(`jp:pbr`); the capture-tool revision is `31bccb376`. All staged build overrides
match the current build hashes. Engine, launcher, native Vulkan shader API,
standard shaders, tier0 and Portal 2 client hashes differ from the original B
snapshot. B uses strict native core rendering, FSR off, 7680×4320, 4× MSAA,
exposure 3 and a 10000-nit HDR peak. The first fizzler's disabled state is checked
after 134 frames and before the first export. All 19 linear PFM exports confirm
active HDR output and a recorded output pass; their raw peak is 7.8671875.
The updated SDR fit has shared contrast 1.315 and preserves the unexposed HDR
files. Native lighting/content differences remain visible; fitting SDR tone
distribution is not a promise of pixel equivalence with the legacy renderer.

The first refresh attempt exposed an incorrect shortened closing wait and was
rejected for missing named captures without changing the gallery. Restoring the
installed view-oracle frame budget, with additional HDR-export waits, produces
20 SDR screenshots and 39 camera records; the successful boot takes 1052.10
seconds. This is a capture duration, not performance acceptance evidence.
Four script tests pass, including B-only reuse and failed-refresh preservation.
The full verification checks all 19 A hashes, left receipts, staged binary
hashes, HDR metadata, raw/PNG hashes and output dimensions. All 171 gallery file
links resolve. Six representative before/after views were visually inspected,
including the disabled-fizzler doorway. Evidence is retained in
`quality-results/map-swipe-setup/latest-b-verification-20261004.json`, with its
reproduction script `verify-latest-b.py`, build and refresh logs alongside it.
`latest-b-rerun.json` identifies the active capture and prior-gallery backup;
`latest-b-review.png` contains the visual review. This capture refresh adds no
render implementation or qualification claim.

## Resource cache residency policy slice (2026-10-04)

`TextureCache::StageMips` now creates a texture with exactly the supplied mip
prefix, so a partial chain consumes only the image levels it can sample. The
entry reports its resident payload bytes. Both texture and mesh entries now
carry explicit integer priority scores and resident-byte counts;
`EvictToBudget` removes the lowest scores first, with name-order tie breaking,
until the caller's byte allowance is met. Entries disappear from lookup when
evicted, and their device objects still retire behind the recorded completion
token. Texture byte sizing uses device format block geometry, so BC images can
be staged and accounted correctly.

The `render.resources` null-device conformance suite passes 43 checks,
including partial-chain allocation/accounting and texture/mesh budget eviction.
The changed-file style check passes. `archlint check --changed` reports two
existing CAP002 violations in `public/gameui/graphics_settings_service.h` and
`public/render/pass/world/world_pass.h`; neither file is part of this change.

This does not yet move the core-world material path off borrowed legacy GPU
images, and the cache budget methods are not wired to a product cache owner.
The current `ICoreTextures` boundary exposes imported GPU IDs, sampler state
and metadata, but no copyable texel source; `TextureCache` stages owned images
from CPU mip bytes. The next implementation slice must provide an owned content
snapshot at that boundary and route core-world textures through its cache.
No render quality gate or support profile is promoted by this work.

## K5/K12: per-level model geometry, residency and CPU-copy ownership (2026-10-04)

Code revision: `0fd1c88c4`. The working-tree change was swept into another
session's `jp: pbr` commit in the shared checkout while this slice was being
recorded, so the diff this entry describes is the one that commit carries, not a
separate commit of its own.

User request: "keep model LODs as separate allocations so far-away models can
drop their LOD0 buffers, and drop the CPU copies once a model is uploaded."
This slice owns `render.pass.world`'s model geometry and
`render.composition`'s publication of it, plus the host declaration that says
which models may be posed. No frozen render path changed, no pixel changed, and
R89/R91/R96 acceptance is unaffected.

**Structure.** `WorldData::StaticMesh` is now a list of
`WorldData::StaticMeshLod` blocks, one per Studio hardware level, each with its
own vertex and index allocation (`MakeLevel` publishes one; `AddLevel` is the
one rule that appends a block's surfaces and per-skin materials and keeps the
counts, so a block's allocation, index range and surface ids cannot disagree).
`WorldSurface::firstIndex` is its level's local index from zero, and
`StaticMesh::LodOfSurface` is the one mapping from a surface to its level. The
composition groups the parsed Studio meshes by level and publishes each block's
staging as one shared allocation (`std::shared_ptr<const std::vector<...>>`),
so the composition and the pass's immutable world snapshot hold one copy
between them rather than a copy each.

**Ownership.** Model buffers moved out of `Resources` (per target format) into
`WorldPass::State`, because model geometry is world data, not target state: a
target format change or a stage republication (a late probe volume re-setting
the stage) no longer drops and re-uploads every model. `WorldData::
modelsRevision` names the geometry revision; a republication with the same
revision keeps the resident buffers, and revision zero means "unversioned", so
a lab scene or test fixture never inherits another world's buffers.

**Residency.** A level no view has selected for `kModelLevelIdleFrames` (120
recorded frames) is released, and its buffers retire behind the frame's
submitted token like any other retired buffer; a view that selects it again
uploads it from the world's staging in that recording. Two levels are pinned
and never released: a model's coarsest level (what every distance selects) and
every level of a model the host may pose (`RenderCoreStaticModel::posed`, which
defaults to true). The depth-and-normal prepass over the world's instances draws
only levels already resident, because it passes over every instance and would
otherwise keep every level resident forever. `WorldStats` reports the levels
resident, their bytes and the levels released, published once per recorded frame
after that frame's uploads.

**CPU copies.** The staging a level is uploaded from is now shared, not copied.
A model the host declares static-only gets no per-frame skinning copy at all
(`ModelPoseSource::vertices` is per level and empty), which is the second CPU
copy every static prop used to hold. The engine sets the declaration from the
one thing that decides it: the client model precache table
(`engine/render_core_world_draw.cpp`), which also fixes a static prop that is
both a prop model and precached - it used to be deduplicated into the static
entry and lost its posable status. The boot line reports the static-only count.
`RenderCoreWorldStats` gained `modelLevels`, `modelStagingBytes`,
`modelPoseSources` and `modelPoseBytes` so the CPU geometry a level set holds is
observable from `r_core_world_stats`.

**Deferred, with the reason.** The staging of a releasable level is still
retained, because it is the only source its re-upload has, and a device change
or a new world needs the same source: dropping it needs a host port that
re-supplies one level's bytes and an asynchronous prefetch so the render
sequence never reads a file. Until that port exists, zero CPU staging is
impossible for any level that can be released or rebuilt, and this slice does
not pretend otherwise. What it removes today is the duplicate copy per owner and
the entire per-frame skinning copy of a static-only model.

Validation and reproduction (Linux desktop, Radeon 8060S/RADV, `build-rc-modelres`):

| Check | Result |
| --- | --- |
| `render.world.null` | 128 checks pass (113 before); new W22 group: one level is one allocation, a second level uploads its own buffers and bytes, an unused level releases them after its idle window and is uploaded again when selected, the coarsest level stays pinned, a level with no staging fails by name, teardown leaks nothing |
| `render.composition` | 65 checks pass (58 before); new P11 group: every hardware level is published, a static-only model keeps no skinning copy, a posed one keeps it, the level geometry is one allocation either way, every level (including the blank one) is a valid selection |
| `render.resources`, `render.opaque.null`, `render.lines.null`, `render.scene.v1`, `render.material.v2`, `render.frame.v1`, `render.shadows.atlas`, `render.skinning.reference`, `render.graph.v1`, `render.material.programs`, `render.lab.composition.selftest`, `render.debug-views.product.selftest`, `render.legacy-frame-executor` | pass |
| `render_lab suite posed-model` (Vulkan, validation) | 108 checks pass, 0 validation messages |
| `render_lab suite model-selection` (Vulkan, validation) | 37 checks pass (36 before) |
| `render_lab suite selfillum`, `tree-sway`, `view-state`, `shadowed-lights`, `sprite`, `softparticle`, `clustered-lights`, `volumetric`, `ssr`, `energy-field`, `cost-overlay`, `reflection-candidates`, `shadow-receiver-perf`, `panel`, `gtao`, `bounce`, `area-lights`, `lightmap-basis`, `probe-volume`, `reflection-probes`, `map-terms`, `cutout-shadows` (Vulkan, validation) | pass |
| `engine/render_core_world_draw.cpp` syntax-only with the Portal 2 product profile's flags | pass |
| archlint `check --all`, `baseline --verify`, `inventory --verify`, `targets --verify --partial build-r03-tests`, 166 archlint and 38 stylelint tests | pass; the two recorded CAP002 findings (graphics_settings_service.h, world_pass.h) are present with this change stashed |
| stylelint `--changed --diff` | my files clean; `public/content/build_graph.h` is another session's file |

```sh
python3 tools/quality/conformance.py check --suite render.world.null --suite render.composition \
  --out quality-results/model-level-residency.json
WAFLOCK=.lock-waf-modelres ./waf configure --tools --disable-warns -T release -o build-rc-modelres \
  --render-core-vulkan=on --ktx-source-root=/tmp/rfc0008-ktx-pin \
  --ktx-build-root=/tmp/rfc0008-ktx-pin/build-rfc0008
WAFLOCK=.lock-waf-modelres ./waf build --targets=render_lab -j8
LD_LIBRARY_PATH=build-rc-modelres/tier0 build-rc-modelres/render/lab/render_lab suite posed-model --validate
LD_LIBRARY_PATH=build-rc-modelres/tier0 build-rc-modelres/render/lab/render_lab suite model-selection --validate
```

Pre-existing failures found while running this slice, none caused by it and none
fixed here:

- `quality/conformance.manifest.json` had drifted from the sources: eleven
  suites that compile `render/pass/world/world_pass.cpp` omitted
  `render/resources/mip_feedback.cpp`, and six that compile
  `render/composition/core_world.cpp` omitted `render/composition/
  core_temporal.cpp`, `render/pass/temporal/temporal.cpp` and
  `render/pass/temporal/input_copy.cpp`. All seventeen failed to link at HEAD
  (`undefined reference to MipFeedbackFrame::AddVisible`, `CoreTemporal::*`,
  `InputCopy::*`), which includes `render.world.null` and `render.composition`.
  This slice adds the missing sources; the repair is a manifest correction, not
  a behaviour change.
- `render_lab suite debug-views` and `suite lighting-controls` fail at HEAD with
  "the canvas frame was refused at submission (device status 9)", reproduced
  with this slice's changes stashed. The Vulkan port's recorded-command
  validation rejects the first draw: the pipeline declares 128 bytes of draw
  constants and the fixture writes fewer, so `DrawConstantCoverage::Ready()`
  (D16) is false. `render/lab/debug_views_suite.cpp` was last changed by
  `20c101df3` ("jp: pbr"), which also changed the draw-constant size the
  fixture has to write. Not repaired here: that file belongs to that slice.
- `render_lab suite temporal` (RFC 0019 FSR) reports "FSR device: unsupported"
  and exits without a checks-v1 record in this tree's configuration.

No performance acceptance, support profile or roadmap gate changes with this
slice. The residency window (120 frames) is a declared constant, not a measured
optimum; the number of levels a real map releases, and the frame cost of a
re-upload at a level switch, are unmeasured because no product tree could be
built in this session without overwriting another session's configure (the
`build` tree's waf lock is stale and reconfiguring it is a shared-profile
change). The census counters exist so that measurement is one boot away.

### sp_a1_intro4_relit optimization captures and proposals (2026-10-05)

[Record: 0016-intro4-relit-rendercore-optimization-2026-10-05.md](0016-intro4-relit-rendercore-optimization-2026-10-05.md).
Optimization evidence only: no engine change and no gate state. The published
P2:CE relit package is now measured against `linux-desktop-high-120` at
1920x1080/High/4x MSAA on native Vulkan (`quality-results/relit-perf-20261005/`):
arrival interval 90.8 ms, CPU 85.5 ms, GPU 58.7 ms, and the frame is CPU-bound
before it is GPU-bound. Three bracket-synced `perf` captures and six controls
attribute the cost:

- `CoreWorld::RecordWorldBatch` is 65% of all process CPU, and the
  depth/normal prepass plus GTAO block inside it is **64% of one core view's
  recording CPU** (47 samples per view per frame, 15.3 ms/frame at two views).
  The prepass draw lists are rebuilt from every `world->surfaces` record and
  every static instance x surface on every call, although they depend only on
  the world generation and material revisions.
- The frame records **two** core world views. The second is the viewmodel scope
  (`Push3DView` in `DrawViewModels`), which repeats cluster assignment, the
  world prepass, GTAO and a share of the 14 per-frame world batch recordings.
  `+r_drawviewmodel 0` measures -18.3 ms interval, -17.8 ms CPU, -8.0 ms GPU.
  The retained RenderDoc audit of the same map shows the same duplication
  (`core world prepass` 499 markers under each scope, two identical 3,568-draw
  static shadow sets).
- Shadows remain the largest GPU item (world surfaces 12.8 -> 4.9 ms with
  `r_core_shadow_quality 0`); the RCV-09 depth prepass is load-bearing (+31 ms
  arrival GPU without it); GTAO costs 2.7-3.5 ms per recording against its own
  0.5 ms component budget; opaque batching is mildly helpful and stays on.
- Two strategic facts: `r_core_world 0` runs the same map and settings at
  10.4 ms interval (cost context only, different image, not a parity
  judgement), and the host/engine portion of the frame is 7.9-8.8 ms in a
  listen-server fixture, already at the row's 8.333 ms CPU limit. A player-client
  capture is needed before renderer work can be said to be able to satisfy the
  row.

Seven ranked proposals (P1 viewmodel scope, P2 prepass list caching, P3
duplicate world geometry between the two depth passes, P4 per-frame VMT
re-import, P5 `WorldView` copy per batch, P6 receiver attribution, P7 GTAO
component) each carry their owner, supported size and required evidence. Two
workloads were added: `portal2-intro4-relit-perf-v1` (budget-linked acceptance
for this map, same cameras as `portal2-intro4-perf-v1`) and
`portal2-intro4-relit-diagnostic-v1` (no budget row, so `frame_floor.py`
accepts switches for attribution; never acceptance evidence). Known gaps: ~6%
run-to-run noise, static cameras only, no resolution sweep (the collector pins
a row's dimensions), and 33 missing environment cubemaps in the published
package (a content gap at load, not a per-frame cost).

### Static-only prop surface selection repair (2026-10-04)

The per-level geometry migration grouped `WorldSurface` records by LOD but
populated `ModelPoseSource::surfaceBodies` only for posed models, in the old
mesh order. A static-only prop therefore selected an empty surface list; the
core accepted it as a valid blank selection, and the engine suppressed the
legacy draw because it considered the prop claimed. The body/LOD selector is
now appended beside each surface as that surface is emitted, for both posed
and static-only models. Skinning vertices remain conditional on `posed`.
This also keeps selector indices aligned with the new LOD-grouped surfaces.

The source/style review covers the regression repair; no product boot or
conformance run has yet verified the catwalk in-game. K5/K12 and map completion
remain open.

### K12: volumetric fog in the game, and the game's output in game/lab comparisons (2026-10-05)

The R96 fog gap from the [matched map sweep](#k12-gi-door-and-matched-gamelab-map-sweep-2026-10-01)
was that Portal 2 ignored `env_volumetric_fog_*` and the product never composed
`render.pass.volumetric`. This slice moves the participating-media term from
`render_lab` into the product world stage, with one owner of the inputs.

- **One owner.** `render.composition`'s `map_media.h` now owns the entity
  parse (`MediaFromEntities`, `FroxelLayoutOf`, `MediumLightsFrom`), which
  `render_lab` used to keep privately. The lab reaches it through
  `lab_media.h` aliases; `render.lab` gains the `render.composition` edge
  (it sits above composition), and `render.composition` gains
  `render.pass.volumetric`. The medium's lights and projectors are
  `render.pass.lights`' `MapLights` through `MediumLightFrom`, the surfaces'
  own set with its windows, not a second light parse. Attenuated (vrad
  c/l/q) lights and non-point/spot shapes count as unsupported, by name.
- **Game.** `CoreWorld` reads the media with the map's lights at
  `SetWorldMesh`. After each stage view's opaque batch, once per view and
  record frame, it composites the medium over the frame on the view's own
  light grid subdivided 8 x 4 (the lab's froxels). It uses the view's
  merged runtime lights (`MediumLightsFrom`), scaled with emission by the
  slot's output scale, and blends through the target's sRGB view. A
  multisampled target, a target without imported color/depth, a viewport
  that does not cover its target, or a refused/unrecorded pass is refused by
  name and counted (`RenderCoreWorldStats::volumetricRefused`, with the
  first reason logged). `volumetricViews` counts composited views. The
  new `r_core_volumetric` (default 1) leaves the term out at 0, as the lab's
  `--no-volumetric`.
- **Not yet in the game's medium:** projectors (their cookies have no
  product upload path; logged at the first composite), shadows (as in the
  lab), the translucent application (as in the lab), and MSAA targets.
- **The comparison's output.** Every core-world game frame is an HDR scene
  that the native backend's `render.pass.output` maps to the SDR back buffer
  with BT.2390 at scene peak 16 (`hdrScene` whenever a core pass recorder
  exists). The lab's PFM was the linear scene, and the comparator only
  clipped it. Binned by lab luminance, the unfogged hall matched the game to
  0.999 below 0.4 linear and fell to 0.50 at 1-2 and 0.10 above 2, so bright
  fixtures were being scored on the output transform, not on lighting.
  `render_lab --output-peak p` now records the same `render.pass.output`
  (exposure 1, headroom 1) before readback, and `game_lab_compare.py` passes
  `GAME_SCENE_PEAK` (16). The new
  `test_lab_frame_takes_the_games_output` check fails when the peak is
  seeded to 0.

Evidence (Linux, AMD Radeon 8060S / RADV, native Vulkan, 512x384, source
`b06363170` plus the dirty tree; the build's other sessions' WIP was
present). `render_lab suite volumetric` 17/17 and `map-terms` 38/38 pass;
`tools/quality/tests/test_game_lab_compare.py` 7/7 and the matrix tests
pass; `archlint check --all` adds nothing new and `baseline --verify` is
current. In-game A/B at the nave camera (`r_core_volumetric 0`/`1`, same
build): the fog adds 0.105, 0.077 and 0.048 linear in the top three row
bands. The diagnostic matrix is in
`quality-results/rendercore-model-game/volumetric-output-matrix-20261005/`:

| Camera | Mean /255 (2026-10-01 → now) | p99 | > 8 | Note |
| --- | --- | --- | --- | --- |
| foggy-hall/nave | 18.49 → 1.52 | 17 | 3.17% | fog and output; 0.17 pt over the 3% limit |
| foggy-hall/side | 24.49 → 1.65 | 18 | 2.02% | within all three first-profile limits |
| area-room/overview | 1.42 → 1.09 | 12 | 1.50% | declared gate passes |
| area-room/grazing, wall | 1.80, 0.74 → 1.38, 0.55 | 18, 7 | 2.8%, 1.0% | |
| cornell-floors/floor, front | 0.82, 2.18 → 0.76, 0.59 | 8, 10 | 1.0%, 1.3% | |
| door-room/a-door, b-door | 0.49, 0.52 → 0.43, 0.46 | 5, 5 | 0.8%, 0.8% | |
| material-sweep/front, grazing | 1.94, 1.51 → 1.63, 1.30 | 30, 27 | 4.3%, 3.2% | p99 over 25 |
| mirror-corridor/down, low | 1.19, 2.47 → 1.17, 2.45 | 17, 41 | 3.3%, 10.4% | reflection edges, unchanged |
| portal-pair/a-portal, b-floor, b-portal | 0.81, 1.24, 0.63 → 0.73, 1.15, 0.42 | 5, 18, 5 | | |
| projector-cookie/room, wall | 23.29, 68.37 → 22.15, 63.40 | 168, 178 | | projector term not fed to the game core (open) |
| sun-colonnade/along, yard | 1.03, 1.49 → 0.88, 1.12 | 24, 55 | 2.9%, 2.4% | along within all limits; yard silhouettes (p99) |

The sun-colonnade lab frames had failed with `render_lab: no draw group`
since the unlit family declared no per-draw inputs: the lab passed the
lightmap page to every non-PBR program. It now passes one page per input
the program declares (`ResolvedProgram::drawInputs`) and names the program
when a draw group is refused; the scores above are from that lab
(`/tmp` run of `--fixture sun-colonnade`, same build).

**Performance: a blocking miss.** `render_lab --time 20` (the pass in its
own submission, same camera): median 57.3 ms at 1024x768, 135.5 ms at
1920x1080, 223.4 ms at 2560x1440 and 488.7 ms at 3840x2160. Every froxel
loops over every light (256 here) at 2 x 2 x 4 samples on a grid that
scales with the screen. Under binding rule 7 this blocks the term's
performance acceptance on `linux-desktop-high-120`. No sample count or
froxel resolution is cut. Shipped Portal 2 maps carry no
`env_volumetric_fog_*`, so their frames do not run the pass. The exact
optimizations named next are per-froxel light lists (the RFC's open
"cluster lists") and skipping froxels beyond each tile's farthest scene
depth, which the composite never reads. No Fold7 run.

R96 stays `active`: projectors, cutout shadows, transmission and
refraction, remaining materials, skinned models, nested and portal views
and moving doors remain; "game matches lab" is not met (projector-cookie,
mirror-corridor/low, material-sweep), and `r_core_world` stays opt-in.

### K12: projected lights on the game's core, and one cookie owner (2026-10-05)

The R96 projector gap (`lt_projector_cookie` room/wall 23.29/68.37 of 255,
"the product world stage does not feed the projector/cookie to the core
lighting pass") is closed for the lighting path. A level difference remains,
from a recorded convention mismatch (below).

- **One cookie owner.** `render.composition`'s `projector_cookies.h` now
  owns the cookie array: `DecodeCookies` (CPU, any thread, the shared VTF
  container reader) and `CookieArray` (device array, upload, and a
  token-based `Release` so a running renderer never waits idle). The lab's
  private copy and `render/lab/lab_media.cpp` are deleted, and
  `lab_media.h` aliases the shared owner. `texturecontainer/wscript` now
  defines `vtf_texture_reader` before its KTX check (the reader needs no
  KTX, and a client tree without KTX silently dropped it from `use`).
  `render.composition` gains the `content.vtf-reader` edge.
- **Game.** The engine installs a `RenderCoreFileSource` (the GAME search
  path, VPKs included) when it binds the world. `CoreWorld`'s light sink
  keeps the light set's `projected` lights. On the main thread, when the
  set of cookie names changes, it decodes them once and gives each queued
  stage view its projectors with cookie layers and the shared images. On
  the render sequence it uploads each new cookie set once (the old array is
  released behind the frames that read it). The projectors join the view's
  shadow plan (`ShadowPlanInput::projectors`, so they get atlas tiles) and
  reach the surface program through `StageViewLights` and
  `SurfaceProjectors`, as in the lab. A cookie that is missing, does not
  decode or differs in size from the first refuses the frame's projectors
  by name (`RenderCoreWorldStats::projectorsRefused`, first reason logged).
  Without a file source they are refused too.
- **The comparator** leaves the lab's projected-light bounce out
  (`LAB_TERMS_OUT_OF_GAME = --no-bounce`). It is moving-light GI, which the
  user's 2026-09-30 decision keeps out of the product, so a lab frame that
  includes it can never match the game. The check
  `test_lab_frame_takes_the_games_output` asserts it.

Evidence (same host, build and dirty-tree caveat as the fog slice):
`render_lab suite volumetric` 17/17, `map-terms` 38/38 and `bounce` 5/5;
comparator and matrix tests 14/14; archlint adds nothing new (its two stale
QuickTime entries are another session's removal). Matrix:
`quality-results/rendercore-model-game/projectors-matrix-20261005/`.

| Camera | Mean /255 (fog slice → now) | p99 | > 8 |
| --- | --- | --- | --- |
| projector-cookie/room | 22.15 → 9.26 | 88 | 12.5% |
| projector-cookie/wall | 63.40 → 30.27 | 89 | 40.7% |
| foggy-hall/nave | 1.52 → 1.43 | 15 | 2.92%: now within all three limits (the hall's projector lights it) |
| foggy-hall/side | 1.65 → 1.56 | 16 | 1.92% |

Every other camera is unchanged from the fog slice's table.

**The remaining projector difference is a convention decision, not
lighting.** In the beam, the game is 0.245x the lab (median per pixel, linear
light), and the cookie, frustum, shadows and range match. The light records
differ only in color: the lab has (2.5, 2.3, 2.0) and the game (0.490, 0.471,
0.442). `render.pass.lights`' `MapLightsFromEntities` (the lab) reads
`lightcolor "255 245.5 230.4 637.5"` as GammaToLinear(rgb) x brightness / 255,
Portal's server convention. Portal 2's server (`game/server/portal2/
env_projectedtexture.cpp`) keeps it in a `color32`, so the brightness 637.5
wraps to 125 in a byte. Its client then folds rgb / 255 x alpha / 255 x
lightstyle x `brightnessscale`, linear, as the retail shaders do:
255/255 x 125/255 = 0.490. Which convention the shared map-light parse
follows per game, and whether this fixture should author a brightness that
Portal 2 can represent, is for the light-set owner and the user. It is not
changed here.

Not in this slice: projectors in the volumetric medium (the game's medium
still logs "projectors not in the medium yet"); `lightworld 0` (models-only)
projectors are lit like any other; and the projector bounce, which is out of
the game's scope by decision. R96 stays `active`.

### K12: no-effect console keys and a current Portal 2 claim inventory (2026-10-05)

The largest refusal group in the [feature census](#k12-refusal-groups-by-feature-2026-10-03),
`$shadersrgbread360` (154 materials), has no effect on PC. No shader in
this tree declares it, and PC textures are read through their sRGB views.
`render.material`'s no-effect key table (`vmt_mapping.cpp`) now records it,
with `$x360appchooser`, with reasons, as it already did for other keys no
shader reads. `material_claim_inventory.py` learned the device's newer
`kModulate2x` and `kAlphaAdditive` blends (5 and 6); without them the
inventory could no longer be regenerated.

The regenerated `quality/materials/portal2-all-claims.json` (3,738 VMTs;
`--verify` 3,738/0) moves from 1,354 statically supported, 1,062 supported
with requirements, 1,176 unsupported and 146 dynamically unresolved to
1,494 / 1,306 / 792 / 146. So 2,800 of 3,738 now claim, against 1,850 in
the R96 row. Of the 384 materials that left `unsupported`, 154 are this
change; the other 230 are earlier sessions' claim work that the checked-in
inventory had not recorded. The largest remaining groups are SpriteCard
(143), `env_cubemap` per-view textures (63), Subrect (57), and
`$ambientocclusion` and `$translucent` (45 each). The claim inventory is
VMT reachability, not scene or pixel evidence.

The composition conformance rows (`render.composition`, `.capabilities`,
`.capabilities.gl`, `.gles` and the product rows that compile
`core_world.cpp`) now list `map_media.cpp`, `projector_cookies.cpp`, the
volumetric pass and the VTF reader. The fog and projector slices had broken
their links (reported by source-engine-d7). Now `render.composition` (65),
`.capabilities` (14) and `.capabilities.gl` (14) pass. The four failures in
`tools/render/tests` are in `shader_toolchain` and `vulkan_scans`, whose
tools have another session's uncommitted edits.

### K12: cutout shadow casters in the game, and block-compressed cookies (2026-10-05)

This continues the [material-owned cutout shadow preparation](#material-owned-cutout-shadow-preparation-2026-10-04-game-migration-open)
into the game. Before this, the product left the stage's alpha-tested world
surfaces out of its position-only casters ("14 alpha-tested world triangles
need cutout shadows; omitted"), so they cast nothing.

- **Ownership.** The composition keeps the caster policy: the meshlets it
  leaves out of the solid casters are its cutout surfaces. It hands each
  frame's shadow atlas, its planned views (with `TileViewport`'s inner
  rectangles) and that surface list to the world pass once per atlas
  (`WorldTarget::cutoutShadows`, `FrameLighting::cutoutsDrawn`). The world
  pass owns the materials and groups, so it draws each surface through the
  surface program's depth point (`SurfaceProgram::ShadowPipeline`). That
  point evaluates the material's own coverage, so there is no second alpha
  evaluator. It binds the material, frame, neutral view and draw groups the
  lit pass uses, with `toClip` set to the shadow view's matrix. It draws
  over whatever the atlas holds (new, kept, shared or mover-composited
  tiles), before any view reads the atlas.
- **Refusals by name.** An animated (`$treesway`) cutout is refused: its
  cached tiles would hold a stale pose, and wind-driven tile invalidation is
  not built. Groups that are not ready are refused too. Blended and
  transmitting points are not opaque casters; `ShadowPipeline` refuses them
  and they cast nothing, as their visible point lets light through.
  `WorldStats` and `RenderCoreWorldStats` count draws and refusals, and
  `r_core_world_stats` prints them with the medium and projector counts.
- **Cookies as shipped.** The first game boot refused Portal 2's own
  projector: `effects/flashlight003` is block-compressed, and the cookie
  array took only RGBA8. `DecodeCookies` now keeps a cookie's own format
  (RGBA8, BGRA8 or BC1-BC5, through a unorm view) and pads with a white
  block of that format. A cookie whose format or size differs from the
  first is refused by name, never converted.

Evidence: `sp_a2_laser_intro_relit`, native Vulkan, `r_core_world 1`,
1024x768 headless (`portal_boot.py` pass, `/tmp` run; same host and
dirty-tree caveat as above). `r_core_world_stats` reports: 1 cutout surface
(14 triangles); 300 cutout shadow draws over 75 lighting builds, 0 refused;
1 projected light lit, 0 refused (it was refused before the cookie change);
no medium on this map. The frame is visually sane.

Not yet: an in-game image oracle of a cutout shadow against the lab's
`cutout-shadows` suite, which proves the coverage math; animated foliage
casters; static props with cutout-only materials (the boot logs "8
cutout-only" props still outside the casters); and the cost. The cutouts are
redrawn every frame into every planned view, not only into dirty tiles, and
no frame-time measurement has been made. R96 stays `active`.

Follow-up the same day: **static props' alpha-tested surfaces cast too.**
`SetStaticCasters` now lists every selected prop surface whose material is
not an opaque shadow material as an (instance, surface) pair, not just the
props with no opaque surface at all. Props that mix opaque and cutout
surfaces had lost their cutout parts as well. The world pass draws each
pair from its model level with the instance's transform and skin
(`StaticMaterial`), through the model material's depth point and the lit
pass's groups. A level that is not resident is skipped and counted
(`cutoutShadowNotResident`). One that is drawn is marked used for the frame,
so residency keeps it for the GPU work. On `sp_a2_laser_intro_relit`,
`r_core_world_stats` reports 2,128 cutout shadow draws, 0 refused and 0 not
resident, with 2,280/2,280 views drawn and 0 failed; the boot passes. The
in-game image oracle and the cost remain open, as above.

### K12: material-sweep's residual is a missing term, SSR (2026-10-05, finding)

Reviewing the [projector matrix](#k12-projected-lights-on-the-games-core-and-one-cookie-owner-2026-10-05)'s
material-sweep/front (mean 1.63, p99 30, 4.3% over 8): the difference
image concentrates on the glossy gold row's reflections, plus a thin band
at the shadow penumbrae. The lab composes `render.pass.ssr` (its log reads
"ao, ssr"), and the game's `CoreWorld` composes no SSR at all. So the
game's glossy reflections are the probes' alone. SSR is a declared term of
`render.lighting.v1` ("relit probes with SSR"), so this is an R96
integration gap, not a tolerance question.

What wiring needs (from `ssr.h`'s contract): the stage's lit pass draws
the surface program's `kSsrTargets` variant, with three more color outputs
(octahedral normal and roughness, IBL radiance, specular weight) at the
view's extent. The pass needs an RGBA16F lit frame and the opaque depth in
`kSampled`. `ScreenSpaceReflections::Record` writes a separate RGBA16F
output, which then replaces the view's color before the translucent stream
continues. In the game the lit pass renders into the backend's frame, so
this changes the world pass's lit rendering (attachments, pipeline
variants, per-view targets). That code is under another session's
uncommitted runtime-direct change, so this slice records the gap and the
design rather than editing around their work. The penumbra band is a
separate, smaller shadow-filter difference, not yet attributed.

Status of "game matches lab" on the matched matrix (diagnostic, current
build): 14 of 22 cameras are within the first-profile limits (mean 3, p99
25, 3% over 8). The misses are projector-cookie room/wall (the
projector-colour convention decision), material-sweep front/grazing (SSR),
mirror-corridor/down and low (3.3% and 10.4% over 8; reflection edges, with
SSR likely), portal-pair/b-floor (5.9% over 8) and sun-colonnade/yard (p99
55, silhouettes). R96 stays `active`.

### K12: LightmappedGeneric `$envmap env_cubemap` reads the stage's reflection probes (2026-10-05)

No LightmappedGeneric material with an `$envmap` claimed. The 49 Portal 2
materials that name the view's `env_cubemap` were refused ("the model does
not bind the per-view texture env_cubemap"), on worlds and meshes. Under
the surface model's native interpretation, a stage's `env_cubemap` is its
reflection probes (RPRB). VertexLitGeneric, UnlitGeneric and Refract
already read it that way; the lightmapped point now does too:

- `BlockFor` admits a lightmapped `env_cubemap` only when the scene carries
  reflection probes. The resolver gives such a claim
  `kSurfaceReflectionProbes` and no cube texture.
  `ClaimForDrawing(..., nativeReflectionProbes)` takes the stage's fact from
  the world pass (a stage with a non-empty RPRB) and from the lab's
  claim-batch world-stage input.
- `surface_program.glsl`'s cubemap term then reads `ReflectionProbesRadiance`
  (linear radiance, so no legacy `ENV_MAP_SCALE`) at the mirror roughness,
  with the ambient cube as the probes' miss, as the world PBR point does.
  The legacy cube path is unchanged for named cube maps.

Evidence: the regenerated full inventory moves from 1,494 statically
supported, 1,306 with requirements and 792 unsupported to 1,494 / 1,353 /
745 (47 materials newly claimed; `supported_with_requirements`,
since they need a probe stage). `render_lab suite reflection-probes` 39/39,
`map-terms` 38/38 and `posed-model` 107/107 pass. `debug-views` fails before
and after this change ("the canvas frame was refused at submission (device
status 9)", reproduced with HEAD's shader). That is a pre-existing lab
failure, recorded here and open. `sp_a2_laser_intro_relit` boots on the
core with 3,230/3,230 views drawn, 0 failed and 106/106 materials claimed.

Open: no image oracle yet. Under binding rule 2 a `render_lab` image check
of a lightmapped `env_cubemap` surface against its probe radiance is owed.
Whether a shipped map draws one of the 47 on the core has not been
captured. R96 stays `active`.

### K12: additive translucent UnlitGeneric and `$nolod` (2026-10-05)

- UnlitGeneric with both `$translucent` and `$additive` blends src-alpha/one,
  as the legacy shaders' additive translucent state does. The device now has
  that blend (`kAlphaAdditive`, already used by sprite glows), so the claim
  takes it instead of refusing "which the port lacks".
- `$nolod` is the material system's texture level-of-detail flag
  (`TEXTUREFLAGS_NOLOD`, mat_picmip), not a shading parameter. It joins the
  no-effect key table with that reason.

The regenerated inventory has 1,494 statically supported, 1,412 with
requirements and 686 unsupported (59 more claimed; `--verify`'s total is
unchanged at 3,738). `render_lab` suites `sprite` (40), `softparticle`
(63), `selfillum` (25), `panel` (76) and `posed-model` (107) pass.
`sp_a2_laser_intro_relit` boots on the core with 3,154/3,154 views drawn and
0 failed. No image capture of a newly claimed additive translucent material
in a game scene has been made yet. R96 stays `active`.

Follow-up: three more keys with no shading effect join the table, each with
its source. `$decalfadeduration` is read by the engine's studio decal system
(`l_studio.cpp`), not by a shader. On DecalModulate, `$translucent`,
`$fogfadestart` and `$fogfadeend` are ignored: `DecalModulate_DX9` declares
no shader parameters and always blends dst x src color. `$flat` stays
refused: it sets the D3D9 flat shade mode, which changes vertex-color
interpolation. The inventory is now 1,494 / 1,425 / 673 unsupported (13
more claimed; most of the materials with these keys also hit another
refusal), and `posed-model` passes 107/107.

Follow-up: UnlitGeneric's `$selfillum` is read and has no effect. The
legacy helper clears `MATERIAL_VAR_SELFILLUM` for UnlitGeneric at init
(`vertexlitgeneric_dx9_helper.cpp`), because an unlit surface already is
its base color. It is a declared unlit schema parameter, so it joins the
unlit family's claimed list rather than the no-effect key table. The
inventory is now 1,525 statically supported, 1,429 with requirements and 638
unsupported (35 more claimed). `selfillum` (25), `sprite` (40), `panel` (76)
and `posed-model` (107) pass.

Follow-up: VertexLitGeneric's `$ambientocclusion` has no effect in this
tree. No shader declares the parameter; `common_vertexlitgeneric_dx9.h`'s
ambient occlusion is a per-pixel screen-space input, which the core supplies
as `render.pass.ao`. It joins the no-effect key table with that reason. The
inventory is now 1,525 / 1,471 / 596 unsupported (42 more claimed), and
`posed-model` passes 107/107.
