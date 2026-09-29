# RFC 0016 progress: render core

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
python3 tools/quality/conformance.py check --cxx g++ --suite render.lights.clusters \
  --suite render.lights.clusters.sensitivity --suite render.shadows.atlas \
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
being degenerate cases in the new expanded system". The plan is RFC 0016's
section "The surface model: legacy materials as degenerate cases". It gives:
- one `surface` program whose terms all have neutral values;
- legacy shaders as exact points, checked against the native ports;
- modern points as opt-in data (a rule table and sidecars, checked against
  Cycles);
- phases S0–S9 ordered from an inventory of 6,000 Portal and 3,738 Portal 2
  VMTs.

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
| Bad adapters (K1 clauses on GL) | `render.device.v2.gl.sensitivity`: lower-left origin (D13 y), −1..1 depth (D13 z), false async compute and false aliasing (D15), ignored write masks (D17), dropped specialization (D20), early upload reuse (D10): 7 of 7, each only its clause. On llvmpipe the D10 case cannot show (copies run at submission) and is skipped, so the row fails its minimum there | pass on radeonsi |
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
