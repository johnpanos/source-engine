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
  both. `portal_boot --resize-stress --resize-mode queued` passes with
  `r_core_world 1` and strict on. The sync resize-stress fails with the
  core world off as well ("2 presents scaled the back buffer … at settled
  sizes", 3 of 3 runs), so that failure predates this work; R32-RESIZE
  owns it.

Open, in order: the surface-model phases (next section), starting with S1
(the translucent-stage world slot, render state and the fog view term); then
render-target and nested views, the Submission cost row, and static props.

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
