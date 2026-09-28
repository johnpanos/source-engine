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
monitors; that parity gap is outside K0.

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
  - Portal 2, whose client does not publish the portal set yet;
  - recursive paths (one hop only);
  - shadowing along the path. Lightmap dlights cast no shadows; K7's
    shadow atlas is the path to that.

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

