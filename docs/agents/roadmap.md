# Roadmap

Part of [AGENTS.md](../../AGENTS.md). The ranked work order; `tools/roadmap/roadmap.py` reads the table below. Keep each cell to one line and put evidence in the row's progress record.

## Unified ranked roadmap

Priority, not a schedule. Prerequisites are hard gates; lower-ranked work may
proceed when its own prerequisites are met. States: `planned`, `partial`,
`active`, `blocked`, `done`. A parent closes only when its full scope passes
with RFC 0005 evidence for its profiles. Done conditions are owned by the
cited RFC phase; the cell is a summary.

### Current render priority (user direction, 2026-09-30; amended 2026-10-10)

0. **R89: the game draws from `render.scene`** (user direction, 2026-10-10):
   `render-scene-bypass` (345 sites) to 0 and K5's scene check, ahead of
   every other render slice. Adapters are frozen meanwhile.
1. Source 2 lighting parity at Source 2 cost on High (user direction,
   2026-10-06; [standing](../../RFC/0016-render-core.md#source-2-lighting-defaults-user-direction-2026-10-06)):
   baked static-prop vertex lighting, R66 roughness mips, R65 MSAA with
   alpha to coverage, tube lights, planar reflection view; GTAO/SSR off by
   default; VGPR and resolution-sweep measurements on the 8060S and 3070.
2. K11/R95 and K12/R96 as paired slices: a term proven in `render_lab` is
   delivered only when the game matches it (strict FSR on/off coverage).
3. Finish K12/R96: transmission, cutout shadows, remaining materials,
   skinned models, nested views.
4. Finish K8–K9/R91 cohorts: decals, particles, sprites/beams, post, UI,
   sky, glass, water, portal/monitor views.
5. Remaining RFC 0012 antialiasing; FSR replaces 4x MSAA in High once
   qualified.
6. Modern model, texture, bake and dense-scene paths (RFCs 0007, 0008, 0015).
7. Optimize the complete result on target hardware to the hard budget.

P2:CE PBR content is mounted read-only from the user's installed VPKs
through the asset-source boundary, never copied or redistributed; see the
[archive](../../RFC/agents-archive-2026-10-10.md#p2ce-high-quality-pbr-content-mounts-user-direction-2026-10-01)
for the mount policy until it moves into RFC 0015.

| Rank / ID | Work and RFC scope | Prerequisites | Done looks like | State |
| --- | --- | --- | --- | --- |
| 1 / R01 | Baseline and profile inventory; 0005 Q0 | — | `baseline.py audit` reproduces checks, profiles, captures, budgets | done ([Q0](../../RFC/0005-progress.md#re-audit-2026-09-25)) |
| 2 / R02 | Conformance runner and evidence; 0005 Q1 | R01 | Zero tests, skips, crashes, timeouts, partial output all fail | done ([Q1](../../RFC/0005-progress.md#q1--r02-runner-shared-conformance-runner)) |
| 3 / R03 | Per-target C++20 toolchain; 0006 M0 | R01, R02 | Compile/link/run proof per dialect on Linux and Android | done ([closure](../../RFC/0006-progress.md#r03-closure-2026-09-25)) |
| 4 / R04 | Architecture enforcement; 0001 rank 1, Q-ARCH | R01, R02 | Ownership, includes, link graph, target owners enforced with negatives | done ([closure](../../RFC/0001-phase-a-progress.md#r04-closure-done-2026-09-25)) |
| 5 / R05 | Result/ID/ownership vocabulary; 0006 M1 | R03, R04 | `Expected`, `StrongId`, `ScopedResource` with suites and consumers | done ([closure](../../RFC/0006-progress.md#r05-closure-done-2026-09-25)) |
| 6 / R06 | Composition/lifecycle kernel; 0001 rank 3 | R02, R05 | Typed providers, rollback at every stage, repeat instances | done ([record](../../RFC/0001-conformance-progress.md)) |
| 7 / R07 | Loader containment and telemetry; 0001 rank 4 | R04, R06 | Scoped loader provider, telemetry, frozen-ABI fixtures | done ([closure](../../RFC/0001-phase-a-progress.md#r07-closure-done-2026-09-25)) |
| 8 / R08 | Hammer corpus and migration inventory; 0002 H0 | R02, R03, R04 | Complete caller inventory and migration records; legacy build evidence | active ([record](../../RFC/0002-progress.md#map-building-loop-direction-and-r08-cmd-2026-09-25)) |
| 9 / R09 | Physics feasibility and IVP baseline; 0004 A | R01, R02, R05 | Inventory, measurements, blocker decisions | partial ([record](../../RFC/0004-progress.md)) |
| 10 / R10 | Runners, clocks, serial graph; 0003 A–B | R05, R06 | Virtual time, runner suite, serial host graph equals legacy capture | done ([closure](../../RFC/0003-progress.md#r10-closure-done-2026-09-26)) |
| 11 / R11 | Paths and module resolution; 0001 rank 5 | R05, R07 | Native/virtual paths, resolver equal to legacy search | done ([record](../../RFC/0001-foundation-providers-progress.md#r11-paths-and-module-resolution-2026-10-08)) |
| 12 / R12 | Dedicated-server composition; 0001 rank 6 | R06, R07, R11 | No render/UI linked or loaded; lifecycle and join suites | done ([closure](../../RFC/0001-dedicated-composition-progress.md#r12-closure-no-render-or-desktop-ui-in-the-dedicated-product-done-2026-10-08)) |
| 13 / R13 | Hammer geometry and scene seams; 0002 H1 | R05, R08 | Strict headless targets; legacy callers through shared owner | partial ([state](../../RFC/0002-progress.md#current-state-2026-09-25)) |
| 14 / R14 | Window/input contracts, SDL3 provider; 0001 rank 7 | R06 | Shared suite passes on SDL3 and fake, Wayland and X11 | done ([closure](../../RFC/0001-conformance-progress.md#r14-closure-the-sdl3-windowinput-provider-done-2026-10-08)) |
| 15 / R15 | Render seam and null provider; 0001 rank 8 | R06 | Explicit profile selection; null and legacy suites | done ([record](../../RFC/0001-render-seam-progress.md)) |
| 16 / R16 | Presentation bridges; 0001 rank 9 | R14, R15 | Native handles confined; resize/loss/shutdown; completion-gated reuse | done ([record](../../RFC/0001-presentation-bridge-progress.md)) |
| 17 / R17 | Hammer real renderer; 0002 R1 | R08, R15, R16 | Core viewports on X11/Wayland, scale, capture, teardown | done ([closure](../../RFC/0002-progress.md#r17-closure-declared-profiles-scale-capture-sharing-teardown-and-restoration-2026-09-28)) |
| 18 / R18 | SDL3 provider completion; 0001 rank 10 | R14, R16 | SDL3 suites on every SDL3 profile; SDL private to owners; Android device run | active ([record](../../RFC/0001-conformance-progress.md#r18-sdl3-private-to-its-owners-in-progress-2026-10-08)) |
| 19 / R19 | Box3D one-worker slice; 0004 B | R05, R09 | BSP/PHY, props, traces, impacts, ragdolls, restore | partial ([record](../../RFC/0004-progress.md)) |
| 20 / R20 | Parallel scheduler and legacy bridge; 0003 C | R10 | Bounded queues, no forbidden waits, capacity measured | partial ([record](../../RFC/0003-scheduler-nodes-progress.md)) |
| 21 / R21 | Particle reference migration; 0003 D | R20 | Legacy/serial/parallel outputs agree; budgets pass | partial ([record](../../RFC/0003-batch-migration-progress.md)) |
| 22 / R22 | Hammer persistence; 0002 H2 | R11, R13 | VMF round-trip and compile; loss reporting; save recovery | partial ([state](../../RFC/0002-progress.md#current-state-2026-09-25)) |
| 23 / R23 | Hammer application authority; 0002 H3 | R13, R22 | One selection/mutation/history owner, generated sequences | partial ([state](../../RFC/0002-progress.md#current-state-2026-09-25)) |
| 24 / R24 | Hammer tools and presenters; 0002 H4 | R23 | Shared policies across entry points; no widgets in tools | planned |
| 25 / R25 | GTK editor workflow; 0002 H5 | R17, R22, R24 | Open/edit/undo/save/compile/run; no MFC runtime dependency | planned |
| 26 / R26 | Remaining foundation providers; 0001 rank 12 | R10, R11 | Clock/thread/memory/process/paths/diagnostics suites per profile | done ([record](../../RFC/0001-foundation-providers-progress.md)) |
| 27 / R103 | Tier 0 over the foundation providers; 0001 | R26 | OS calls and platform branches in providers; export fixture passes | partial ([audit](../../RFC/direction-audit-2026-10-08.md)) |
| 28 / R27 | Vulkan compatibility waypoint; 0001 rank 13 | R10, R16, R18 | Measured compatibility experiment, limitations recorded | partial ([record](../../RFC/0001-portal-vulkan-progress.md)) |
| 29 / R28 | Native Vulkan bootstrap; 0001 rank 14 | R10, R16, R18 | Native device and SDL3 bridge present; failure diagnostics | partial ([record](../../RFC/0001-native-vulkan-progress.md)) |
| 30 / R29 | Four-platform architecture proof; 0001 rank 15 | R12, R18, R26, R28 | Each target passes foundation and SDL3/Vulkan lifecycle smoke | partial ([iOS/macOS](../../RFC/0001-static-composition-progress.md)) |
| 31 / R30 | Existing parallel kernels; 0003 E | R21 | Each cohort passes three-mode, ownership, perf, rollback gates | partial ([record](../../RFC/0003-batch-migration-progress.md)) |
| 32 / R31 | Physics core compatibility; 0004 C | R19 | Gameplay corpus passes on client and dedicated | partial ([record](../../RFC/0004-progress.md)) |
| 33 / R32 | Native Vulkan functional MVP; 0001 rank 16 | R10, R28 | Representative map renders; contracts pass; explicit refusals | active ([record](../../RFC/0001-native-vulkan-progress.md)) |
| 34 / R47 | PBR material family; 0007 A/D | R02, R15 | BRDF analytic and furnace tests; negative controls | active ([record](../../RFC/0007-progress.md)) |
| 35 / R86 | Render core prerequisites and device port; 0016 K0–K1 | R02, R05, R10, R16 | Layer contract, pinned compiler, frozen headers, device suite | done ([K1](../../RFC/0016-progress.md#k1-and-k3-closure-done-under-binding-rule-7-2026-09-28)) |
| 36 / R87 | Render graph and inversion; 0016 K2–K3 | R86 | Graph model, sync validation, frontend as stage passes | done ([K2](../../RFC/0016-progress.md#k2-closure-done-2026-09-28)) |
| 37 / R88 | Shader library and materials v2; 0016 K4 | R87 | Families match their ports; proxy corpus | done ([K4](../../RFC/0016-progress.md#k4-closure-done-2026-09-28)) |
| 38 / R89 | GPU scene, views, world, props, skinned models; 0016 K5–K6 | R88 | Game draws from `render.scene`; `render-scene-bypass` 0; culling and skinning match | partial ([audit](../../RFC/direction-audit-2026-10-08.md#ownership-inventory-of-renderpassworlds-inputs)) |
| 39 / R95 | Lighting model proven in `render_lab`; 0016 K11 | R86, R88 | Every `render.lighting.v1` term with oracles and negative controls | active ([record](../../RFC/0016-progress.md#k11-render_lab-draws-the-whole-lighting-model-2026-09-29-source-engine-cb)) |
| 40 / R92 | OpenGL device adapter; 0016 K10 | R88 | Device suite, pixel families within tolerance, GL product boot | partial ([record](../../RFC/0016-progress.md#k10-opengl-adapter-slices-14-2026-09-29)) |
| 41 / R90 | Clustered lights and shadow atlas; 0016 K7 | R89 | GPU assignment, shadow oracles, atlas budgets | partial ([record](../../RFC/0016-progress.md#k7-headless-slice-clustered-light-assignment-and-shadow-atlas-2026-09-28)) |
| 42 / R96 | Lighting model in the product; 0016 K12 | R95, R89, R90 | Game frames match lab frames; native copies deleted; frame allowance holds | active ([record](../../RFC/0016-progress.md#k12-slices-6-and-7-the-sdf-producer-on-the-core-and-sparse-probe-volume-publication-2026-09-30-source-engine-5a)) |
| 43 / R91 | Remaining cohorts and legacy-stream retirement; 0016 K8–K9 | R89, R90, R96 | All cohorts on the core; legacy stream use 0; render off the main thread | partial ([record](../../RFC/0016-progress.md#later-the-same-day-material-draws-without-stdshader-passes-the-m2m3-slices-and-the-core-shader-api-cut-over)) |
| 44 / R97 | In-process device switching; 0016, 0021 V4 | R91, R92 | Switch by name with drain, rebind and rollback; leak suites | planned |
| 45 / R65 | 4x MSAA and alpha to coverage; 0012 A0–A3, A5 | R02, R32, R47 | Edge/alpha oracles; measured per-profile policy | planned ([RFC](../../RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 46 / R48 | Compile tools on Waf and bake seam; 0007 B | R01, R02, R03 | vbsp/vvis/vrad byte-identical; `ILightBaker` suite | partial ([record](../../RFC/0007-progress.md#r48-host-compiler-preparation-2026-09-23)) |
| 47 / R53 | BSP2 container and map reader; 0008 F1 | R02, R04 | Lumps byte-identical; client/server load; fuzzing | active ([record](../../RFC/0008-progress.md)) |
| 48 / R55 | KTX2 textures; 0008 F3 | R15, R53 | Reader, transcode per profile, pixel fixtures | partial ([record](../../RFC/0008-progress.md#f3-ktx2-host-tool-feasibility-2026-09-23)) |
| 49 / R66 | Normal-variance roughness and alpha-coverage mips; 0012 A4 | R55, R65 | KTX2 writer bakes both; shimmer oracles improve | planned ([RFC](../../RFC/0012-antialiasing-msaa-specular-alpha-coverage.md)) |
| 50 / R54 | USD World Stage and lightmap charts; 0008 F2 | R48, R53 | Geometry layer, `usdchecker` clean, semantic comparator | partial ([record](../../RFC/0008-progress.md#f2-compiled-world-geometry-slice-2026-09-23)) |
| 51 / R49 | Cycles light baker; 0007 C/E | R05, R48, R54 | SH/RNM, probe, reflection outputs pass oracles | partial ([record](../../RFC/0007-progress.md#legacy-relight-displacement-and-static-transport-slice-2026-10-04)) |
| 52 / R56 | BSP2 world path on the core; 0008 F4–F5 | R32, R49, R54, R55, R90 | World mesh, lightmaps, probes, clustered lights per cohort | partial ([record](../../RFC/0008-progress.md#f4-world-stage-geometry-and-cycles-light-in-the-playable-wmsh-view-2026-09-23)) |
| 53 / R50 | Image-based lighting; 0007 F | R47, R56 | Baked probes and prefilter pass IBL fixtures | partial ([record](../../RFC/0007-progress.md#r50-parallax-parallax-corrected-blended-reflection-probes-bounded-r50-slice-2026-09-25)) |
| 54 / R51 | Stage reference rendering; 0007 G | R49, R54 | Versioned Cycles references from stages | planned |
| 55 / R52 | Hammer compile/preview and vvis graph; 0007 H | R20, R25, R49 | GTK compile/preview with cancellation; PVS equivalence | planned |
| 56 / R81 | Asset identity, index, content build graph; 0015 C0–C1 | R02, R05, R10 | `AssetRef`, index, compiler suite, incremental equals clean | partial ([record](../../RFC/0015-asset-identity-content-build-graph.md#implementation-progress-2026-10-03)) |
| 57 / R57 | Map builds on the content graph; 0008 F6, 0015 C2 | R52, R54, R81 | Cache traces; atomic publication; private caches removed | planned |
| 58 / R59 | USD-native map schema and compiler; 0009 U0–U2 | R05, R48, R53, R54 | Authored USD room compiles without VMF and runs | active ([record](../../RFC/0009-progress.md#u2-prop-role-cohorts-and-geometric-entities-slice-done-2026-09-25)) |
| 59 / R60 | USD-native editor workflow; 0009 U3–U4 | R13, R25, R57, R59 | USD owns persistence and compile; VMF import with loss reports | planned ([RFC](../../RFC/0009-usd-native-map-authoring.md)) |
| 60 / R83 | Runtime asset index and resolver; 0015 C5 | R81 | Resolver is the only lookup path; ratchet 0 | partial ([record](../../RFC/0015-asset-identity-content-build-graph.md#implementation-progress-2026-10-03)) |
| 61 / R82 | Legacy compiler adoption; 0015 C3 | R03, R81 | studiomdl, captions, scenes, nav, sound cache as graph nodes | planned ([RFC](../../RFC/0015-asset-identity-content-build-graph.md)) |
| 62 / R61 | Modern map spatial/gameplay data; 0008 F8 | R31, R53, R59 | Versioned USD-native payload passes client/server suites | planned ([RFC](../../RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 63 / R62 | Modern model asset path; 0008 F9 | R47, R55, R59, R81 | Model assets for all roles; runtime model replaces `mstudio*` use | planned ([RFC](../../RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 64 / R63 | Modern visual parity and geometry scale; 0008 F10 | R47, R50, R56, R61, R62 | Representative USD maps meet image/time/memory budgets | planned ([RFC](../../RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 65 / R85 | Desktop live reload; 0015 C6 | R81, R83 | Reload equals cold start; none in installed/mobile products | planned ([RFC](../../RFC/0015-asset-identity-content-build-graph.md)) |
| 66 / R64 | Direct USD development runtime; 0008 F11 | R56, R59, R60, R85 | Edit/reload/play with authored identities | planned ([RFC](../../RFC/0008-canonical-world-data-and-runtime-formats.md)) |
| 67 / R84 | Profile packages; 0015 C4 | R81, R83 | Closure packages with variants; installed smoke | planned ([RFC](../../RFC/0015-asset-identity-content-build-graph.md)) |
| 68 / R58 | Mobile map packages; 0008 F7 | R29, R55, R56, R84 | Per-profile packages pass on R29 runners | planned |
| 69 / R33 | Hammer feature families; 0002 H6 | R25 | Each family passes load/edit/save/build gates | planned |
| 70 / R34 | Physics gameplay and tools; 0004 D | R31 | Fluids, vehicles, Portal features pass | partial ([record](../../RFC/0004-progress.md)) |
| 71 / R35 | New audited compute seams; 0003 F | R30 | Animation/render-list/AI/streaming cohorts pass gates | planned |
| 72 / R36 | Four-platform release readiness; 0001 rank 17 | R29, R32 | Per-platform images, loss, budgets, packaging | planned |
| 73 / R37 | Physics parallel rollout; 0004 E | R20, R34 | Worker determinism, bridge, budgets; IVP rollback | planned |
| 74 / R67 | Opt-in Box3D capabilities; 0013 P0–P7 | R19 | One interface and gate per capability | active ([record](../../RFC/0013-progress.md)) |
| 75 / R98 | Box3D beyond IVP; 0026 B0–B9 | R19, R67 | Scoreboard beats IVP; IVP fallback kept | active ([record](../../RFC/0026-progress.md)) |
| 76 / R102 | Product pipeline and `kiln`; 0027 L0, L1, L7 | R02, R04, R10 | L7: each platform packages and runs through `kiln` | active ([record](../../RFC/0027-progress.md)) |
| 77 / R38 | Stateful scheduling migrations; 0003 G | R30, R35, R37 | Snapshot ownership and cohorts preserve order | planned |
| 78 / R39 | First-party module retirement; 0001 rank 18 | R12, R18 | Typed linked factories; no name-based discovery | active ([record](../../RFC/0001-phase-b-progress.md#later-work-not-claimed-here)) |
| 79 / R40 | Tool process cleanup; 0001 rank 19 | R11, R12, R22 | Structured process protocol; wrappers retired | active ([record](../../RFC/0001-phase-e-progress.md)) |
| 80 / R41 | Extension hosts, public loader removal; 0001 rank 20 | R07, R11, R39, R40 | Only approved hosts load; general loaders retired | planned |
| 81 / R42 | Scheduler consolidation; 0003 H | R35, R38 | Redundant queues have zero consumers | planned |
| 82 / R43 | Hammer legacy retirement; 0002 H7 | R33, R60 | Parity gate met; legacy shell removed | planned |
| 83 / R44 | IVP simulation retirement; 0004 F | R37 | No profile depends on IVP; IVP stays selectable | planned |
| 84 / R45 | Independent collision decoding; 0004 F | R40, R44 | Collision corpus without IVP code | planned ([RFC](../../RFC/0004-box3d-primary-physics-backend.md)) |
| 85 / R46 | Tier-global retirement; 0001 rank 21 | R39, R41, R42, R43, R45 | Old globals zero; CAP012 provider ratchets at 0 | planned ([RFC](../../RFC/0001-capability-based-platform-architecture.md#where-platform-and-adapter-code-may-live-user-direction-2026-10-08)) |
| 86 / R93 | LAN discovery and co-op pairing; 0017 | R06, R10 | Two clients pair through real menus; negative controls | planned ([RFC](../../RFC/0017-lan-discovery-and-coop-pairing.md)) |
| 87 / R94 | Frame-wide scheduling, one task API; 0003 I | R20, R21 | Continuous executor; J8 legacy job API 0; budgets on devices | active ([record](../../RFC/0003-progress.md#r94-one-task-system-2026-10-07)) |

`roadmap.py check` fails today on R17 `done` with R08 `active`, a known
failure recorded in `quality/baseline.json`.

## Facts as of 2026-10-10

Short corrections to entries the archive carries; details in the records.

- shaderapivulkan is deleted (`154155845`); desktop clients draw through
  `materialsystem/shaderapicore`, the core shader API, which shares one file
  with the 3DS client (`PLATFORM_3DS` blocks) and still carries its 3DS
  header comment and the `CShaderAPIEmpty` class name.
- `r_core_world` defaults to 1 (`engine/render_core_world_draw.cpp`).
- CAP012 totals (target zero): platform branches 2,493; console code 1,778;
  format shapes 1,601; OS calls 1,340; adapter code outside adapters 525;
  scene bypass 345. Jobs: legacy job API 297 sites, thread creation 36.
- Unranked children and side programs (R70–R80, R91-PANELS, R50-CUBE,
  Portal 2 split-screen, TVOS-PROFILE and the rest) keep their state in
  their records; the archive lists them.
