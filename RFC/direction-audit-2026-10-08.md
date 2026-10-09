# Direction audit, 2026-10-08

User request (2026-10-08): "What happened to the platform work, and why are
there still so many ifdefs around for different platforms in tier0? Same with
rendercore, why is it so complicated?", then: "rendercore itself, which is
supposed to be the cleanest layer, is not", "we also need to ratchet on LoC
and an overall general direction audit", and "we also need a systemic way to
enforce this: nothing checks that the game draws from the scene, that
composition stays thin, or that the core's API is free of legacy types."

This record states what was measured, what it shows, what is now enforced,
and the decisions that remain the user's. Numbers are from the working tree at
`133402437` plus the uncommitted work present on 2026-10-08.

## Summary

1. **Gates measured proxies, not the goal.** R103 closed with "every OS call
   cohort at zero", but Tier 0 still has 438 platform conditionals, because
   its ratchet counted calls by name and the calls moved behind a helper
   inside the same `#ifdef` branches. CAP011 checks that render includes point
   down, and nothing checked what the layers contain. K11/K12 slices are judged
   by "game matches lab" one term at a time, which the quickest route passes
   by adding the term to whatever already draws the game.
2. **The render core has two renderers.** The architecture RFC 0016 describes
   (scene, views, culling, passes, material families) is what Hammer's
   viewport uses: `render.scene` is 255 lines and `render.pass.opaque` 239.
   The game draws through `render.pass.world`, 7k lines with a 945-line
   public header, which never includes `render.scene` and holds the world,
   model meshes and their residency, legacy material key/value pairs and
   texture handles, lightmaps, probes, dynamic and UI draws, sprite cards,
   water, fog and foliage terms, GPU culling and five screen-pass hooks.
   `render.composition` (7.7k lines, 14 of 29 files recording passes) drives
   it. The inventory below assigns each of its inputs an owner.
3. **Scope grew faster than the north star closed.** 30 numbered RFCs in about
   three weeks; 87 roadmap rows with 19 done. Seven device adapters (Vulkan,
   GL/GLES, D3D12, Metal, WebGPU, PICA, null) and a proposed eighth (D3D9),
   plus 3DS, WebAssembly, tvOS, F-Stop, TF2 and the Portal 2 reconstruction.
   Of the four north-star platforms, Linux is the only one with routine
   native runs; macOS has never run, and the Android and iOS lifecycle gates
   are open.
4. **Process weight.** `AGENTS.md`, loaded by every session, is 2,430 lines
   (216 KB), most of it tracking narrative. Since 2026-09-20, about 800k of
   the 1.6M added lines (excluding vendored trees and fixtures) are harness,
   evidence and record files (`quality/`, `unittests/`, `tools/quality/`,
   `RFC/`).
5. **Enforcement is now structural** (below). The user named the target
   architecture (2026-10-08): ports and adapters, with an anti-corruption
   layer at the legacy boundary, so legacy types never leak into the core.
   Archlint CAP011 rules 8–9 enforce that boundary from the module manifest;
   CAP012 and CAP013 check what the layers contain and how large each area
   is. All run on every `archlint check --all`, and the RFCs that planned
   otherwise are amended or deleted.

## Measurements

| Measure | Value |
| --- | --- |
| First-party code lines (C/C++ and shaders, blank and comment-only lines excluded) | 3,080,572 in 495 areas |
| Added / removed since 2026-09-20 (excluding thirdparty, games, external, fixtures) | +1,612,434 / −729,990 lines, 1,153 commits |
| Largest added areas since 2026-09-20 | `quality/` 364k, `unittests/` 247k, `game/client` 180k, `tools/quality` 122k, `game/server` 94k, `RFC/` 64k |
| Roadmap rows | 87: 19 done, 14 active, 25 partial, 29 planned |
| Render core (`render/`, `public/render/`) | ~146k lines: lab 31.6k, device 29.7k (seven adapters), pass 19.9k (world 7.0k), material 12.2k, composition 7.7k, scene 0.6k |
| Legacy render code still present | `stdshaders` 12.2k, `shaderapicore` 8.3k, `shaderapiempty` 2.9k |
| Tier 0 platform conditionals (`tier0/`, `public/tier0/`) | 438 in 54 files (`threadtools.cpp` 77, `public/tier0/platform.h` 55, `threadtools.h` 45) |
| PS3/Xbox 360 references tree-wide (code, not comments) | 1,790 in 278 files; Tier 0's own removed here (24) |
| Platform conditionals outside providers and bridges | 2,552 in 639 files |

Reproduce: `python3 tools/archlint/archlint.py structure --report --top 60`.

## Finding 1: the gates measured proxies

| Gate | What it measured | What the goal was |
| --- | --- | --- |
| R103 `tier0_ratchet.py` | Native OS calls by name in Tier 0 | Tier 0 answers through providers, without compile-time platform selection |
| CAP011 layer contract | Include and link edges point down | Each layer holds only its own concept |
| K11/K12 "game matches lab" | A term's pixels in matched game/lab frames | The game renders through the core's architecture |
| Roadmap row states | Evidence for the slice that was worked | Progress toward the north star |

`threadtools.cpp` shows the pattern: `CreateSimpleThread` still branches on
`_WIN32` and POSIX (and, until this change, PS3), and on POSIX calls
`StartPosixThread`, which goes through `tier0_facade::Threads()` and converts
the result back to a `pthread_t`. The OS call is gone from the cohort count,
and the platform branch is still there. The facade keeps `HANDLE` and
`pthread_t` as the frozen handle types, so every caller still branches.

## Finding 2: the render core

### What the game actually runs

```
engine (CRender, client view)        Hammer viewport, render_lab scenes
        │ SetWorld/SetStaticProps/DrawView        │
        ▼                                         ▼
render.composition (core_world*, 11 files)   render.scene ChangeSet → Snapshot
        │ QueueView / RecordBatch               │ makeView / buildDrawList
        ▼                                         ▼
render.pass.world  (the renderer)            render.pass.opaque
```

`render.pass.world`'s header says it itself: it is "the BSP world's surfaces
drawn by the core, at a core-pass slot of the legacy stream". The frame is
still the legacy stream's (R91 open); the core's passes run at marked slots
inside it, so everything a view needs at its slot is captured into `WorldView`
and `WorldTarget` when the engine queues it. Each K12 term added fields there.

### Ownership inventory of `render.pass.world`'s inputs

Owner names are existing modules unless marked *new*. "Frontend" is
`render.legacy-frontend`, the one place legacy types and conventions are
converted.

| Input (public/render/pass/world/world_pass.h) | What it is | Owner |
| --- | --- | --- |
| `WorldData::vertices`, `indices`, `surfaces` | BSP world geometry | `render.scene` (world mesh instances); GPU buffers in `render.resources` |
| `WorldData::staticMeshes` (LODs, skins, `posed`), `IModelLevelSource`, residency sweep | Model assets and level residency | `render.scene` (mesh assets, LOD), `render.resources` (residency) |
| `WorldData::staticInstances` | Static props | `render.scene` instances |
| `WorldMaterial` (shader name, `$key` string pairs, legacy texture handles, `defaults`, `hasProxy`) | A legacy VMT as data | Frontend converts to `render.material` parameter blocks; the core sees material ids |
| `WorldData::reflection`, `stage` (lightmap pages, indirect layer, shadow mask, probe volume) | The map's baked lighting environment | `render.map-media` loads; a scene-owned lighting environment publishes (*new* concept, one owner) |
| `SetStageLightmap*`, `SetStageChange`, `SetStageProbe*` | Runtime updates of that environment | Same owner as above |
| `WorldView::surfaces`, `staticInstances` | Visibility results computed by the engine | `render.culling` over the scene view (draw list) |
| `WorldView::posedModels` | Skinned models at a captured pose | `render.scene` instances with `render.pass.skinning` |
| `WorldView::dynamicDraws` (`Streams`, `bonePalette`, `indices16`, `lighting`) | Immediate legacy mesh draws | Frontend publishes per-frame transient instances to the scene |
| `DynamicDraw::cards`, `cardModel`, `cardView` | SpriteCard particles in D3D9 conventions | A particles pass (*new*, K8), fed by the frontend |
| `UiListView` | The UI draw list as dynamic draws | A UI pass (*new*, K8) |
| `toClip`, `motionToClip`, `previousToClip`, `temporalView`, `viewport`, `hostFrame`, `debug` | The view and frame | `render.scene` `SceneView`, `render.frame` |
| `lights`, `stageLighting`, `StageViewLights` | Clustered lights per view | `render.pass.lights` output, bound through the graph |
| `viewRight`, `waterZOffset`, `WorldTarget::waterReflectTintScale`, `time`, `foliage` | Water and foliage terms | The water and vertex-animation material families' view terms |
| `depthAlphaHandle`, `depthAlphaRange` | Soft-particle depth input | A graph resource declared by the particles pass |
| `WorldTarget::device`, `color`, `depth`, formats, size, `samples`, `submitted`, `frame`, `streamEpoch` | Targets and frame bookkeeping | `render.graph` imported resources and `render.frame`; never pass inputs |
| `drawState`, `clipPlanes`, depth range | View state | `SceneView` |
| `runtimeDirect`, `ambientOcclusionTerm`, `softShadows`, `probeBounce`, `depthPrepass`, `gpuSubmission`, `gpuOcclusion` | Profile and feature switches | `render.frame` feature set (`frame/feature.h`) |
| `lightmapScale`, `outputScale`, `eye`, `envmapScale`, `specular`, `ssbumpNormalized`, fog | Frame terms | `material::FrameTerms`, owned by `render.frame` |
| `ssr*`, `shadowAtlas`, `ambientOcclusion`, `Prepass`, `screenPasses` callback, `sceneColorCapture`, `cutoutShadows`, `motion*`, `temporalViewport` callback | Other passes' resources and hooks | Graph resources declared by `ssr`, `shadows`, `ao`, `temporal`; no callbacks into the world pass |
| `IWorldTextures` (legacy handles), `mipFeedback` | Texture residency | `render.resources`; legacy handles stay in the frontend |

What remains of `render.pass.world` is a pass: it draws the opaque and
cutout world surfaces of a draw list with their programs. The other rows
leave its header.

### Legacy concepts in the core

User decision (2026-10-08): "Legacy types shouldn't leak into the core." The
architecture is ports and adapters (hexagonal): the core owns its ports in
its own terms, dependencies point inward, and translators form the
anti-corruption layer that turns formats and the legacy engine's model into
core types ([RFC 0016](0016-render-core.md#the-anti-corruption-boundary-user-decision-2026-10-08)).

Measured against that:

- **Legacy types** reach core declarations in 5 places: `ITexture` and
  `IMaterial` forward-declared in the composition's public API, and the PICA
  adapter naming the legacy pass recorder.
- **Formats** are read inside the core: `render.material` parses VMT text
  (its VMT importer lives in the core), and `render.composition` decodes VTF,
  studio models and BSP2 lumps.
- **Legacy data shapes** cross ports with no type name to catch: BSP
  vertices (`RenderCoreWorldVertex`), WMSH bytes, legacy lightmap page
  handles as `int`, D3D9 row-vector matrices in sprite cards, and `$key`
  string pairs with their defaults reach the world pass. These ports are the
  engine's model with the type names removed. The ports themselves have to
  change, as the inventory above describes; a translator then produces them.
- **A plan to widen the leak**: RFC 0028 decision 9 would have bound mod
  D3D9 bytecode and its register model directly on the core. It is withdrawn.

## Finding 3: scope

Thirty RFCs since 2026-09-20; most are user-directed, and AGENTS.md records
each decision. The cost of each is ongoing: every adapter or profile shares
the device suite, shader artifact formats, capability refusals, the kiln
profiles and CI lanes, and every row adds tracking text. Meanwhile the
north-star gates that need hardware time (Android lifecycle, iOS lifecycle,
macOS run, R36) have not moved. This audit records the measurement; which
scope to freeze or delete is the user's decision (below).

## Finding 4: process weight

`AGENTS.md` is meant to own the work order and a concise tracking summary
("Keep the table concise and link details below or from the domain progress
file"). It holds 2,430 lines, mostly per-slice narrative that duplicates the
progress records. Every session loads it. The harness and record volume is
about half of what was added since 2026-09-20.

## What is enforced now

**The anti-corruption boundary: archlint CAP011 rules 8 and 9**, declared
in the render layer contract (`layerContracts.render.translation` in
[`architecture/modules.json`](../architecture/modules.json)). The contract
names the translators (`render.legacy-frontend`,
`render.legacy-provider-contract`, `render.legacy-pass-contract`,
`render.map-media`), the format libraries and the fixtures.

- **Rule 8:** only translators and fixtures depend on a format library, and
  only translators, the composition root and fixtures depend on a
  translator. This is structural: it reads the module edges, so a new format
  or a new core module is covered without editing a name list.
- **Rule 9:** a core module forward-declares no type that the core does not
  define. Includes are already bounded by the edges, so forward declaration
  is how a foreign type reaches a core declaration. Types a translator
  defines may be named only by the composition root, which wires them.
  Opaque tag types that are defined nowhere are allowed.
- **Existing violations** are listed in `pending`, each with its row and
  reason: 5 edges (the composition's VTF, studio and map-container reads;
  `render.material`'s VMT text) and 3 types (`ITexture`, `IMaterial`,
  `ICorePassRecorder`). An entry that no longer matches a violation fails as
  stale, so the list only shrinks. RFC 0016's K5 now requires it to be
  empty.
- `tools/archlint/tests/test_capabilities.py` (`TranslationContractTest`)
  seeds each case: a core edge to a format, a core edge to a translator, a
  foreign forward declaration in a pass, a stale pending edge and type, a
  malformed pending entry, and the allowed cases (translators reading
  formats, the composition naming a translator type, comments).

This replaces the name-list ratchet first installed for this
(`render-core-legacy-types`, which counted `IMaterial`, `ITexture` and
similar names): a list of names repeats the tier0 ratchet's mistake of
counting tokens instead of checking the structure.

**Structure ratchets:** `tools/archlint/structure.py`, run by `archlint
check --all` and recorded in [`architecture/structure.json`](../architecture/structure.json):

- **CAP012 identifier and branch ratchets.** Exact per-file counts in code
  (comments and literals stripped). A new file or a higher count fails; a
  lower count fails until recorded with `--write`, so the record stays
  current and only shrinks. A new rule is recorded once with `--adopt`.
- **CAP013 line ceilings.** Every area (the capability module that owns a
  file, or its legacy directory) has a ceiling of code lines. Above it fails.
  `--write` lowers ceilings and never raises one. Growth is a reviewed
  decision: `--raise AREA --reason TEXT` records the date, old and new
  ceilings and the reason in the record, where review sees it.

| Rule | Scope | Baseline | Target | Owner |
| --- | --- | --- | --- | --- |
| `render-composition-thin` | `render.composition`: graph passes, resources, commands | 40 in 4 files | 0 | R96 |
| `render-pass-content-import` | Passes, scene, frame, culling, renderer: VMT import types | 2 in 2 files | 0 | R96 |
| `render-scene-bypass` | Render core, engine, client, material system: the non-scene geometry channel (`SetWorld`, `SetStaticProps`, `WorldData`, `PosedModel`, `DynamicDraw`, ...) | 345 in 36 files | 0 | R89 |
| `tier0-platform-branches` | `tier0/`, `public/tier0/` | 438 in 54 files | Frozen ABI declarations only | R103 |
| `console-platform-code` | Whole tree | 1,790 in 278 files | 0 | R46 |
| `platform-branches-outside-providers` | Whole tree except providers, bridges, Tier 0, tests, tools | 2,552 in 639 files | 0 outside providers | R46 |
| Line ceilings | All 495 areas | Current size, 2026-10-08 | Falling | Each area's row |

`tools/archlint/tests/test_structure.py` seeds each violation (an identifier
in a new file, a higher count, a new platform branch, an unrecorded
decrease, a line ceiling exceeded, a raise without a reason, a malformed rule)
and checks that comments, strings and the `#ifdef _WIN32 / #pragma once`
header idiom do not count.

### "The game draws from the scene"

`render-scene-bypass` is the static half: the channel the game uses instead of
the scene shrinks to zero. The runtime half is not installed. It needs
`WorldStats` (or `FrameStats`) to count draws by origin (scene instance vs
other) and a conformance suite on a Portal 2 boot that fails when a non-scene
draw count rises above its record. It belongs to the first slice that moves
geometry into the scene, so that the counter has a consumer.

## Changes in this audit

- Tier 0's PS3 and Xbox 360 code is removed (`threadtools.cpp`,
  `threadtools.h`, `stacktools.cpp`, by `unifdef` with those macros
  undefined, then the `!defined(PS3)`/`!defined(_X360)` terms). No build
  defines them; only the separate `external/vpc` copy does. Both files
  compile with `-fsyntax-only` using the `portal2-linux-native-vulkan` tree's
  flags; `tier0_ratchet.py check` passes. The `IsConsole()`/`IsX360()` shim
  macros in `platform.h` stay; they are tree-wide (R46).
- R103's row is reopened as `partial`: its call cohorts are zero, and its
  platform-branch cohort is now measured.

### RFC changes (user direction 2026-10-08: "update RFCs to match. even delete them outright if they are planning the wrong thing")

| RFC | Change | Why |
| --- | --- | --- |
| [0016](0016-render-core.md) | New binding section, [the anti-corruption boundary](0016-render-core.md#the-anti-corruption-boundary-user-decision-2026-10-08); the VMT translation moves from `render.material` to the translator `render.vmt-translation`; mod bytecode never enters the core; K5 gains "game draws from the scene" and "ports in core terms" checks | It placed the VMT translation inside the core, and K5's gates never checked that the game uses the scene |
| 0030 | **Deleted**; its record of completed work moved to [RFC 0016's progress file](0016-progress.md#rfc-0030-world-pass-factoring-record-2026-10-08) | Its remaining plan refined `WorldPass` in place (splitting `State` into sub-structs), which entrenches the renderer-in-a-pass the boundary dissolves. Its one live item, moving pass recording out of composition, is carried by `render-composition-thin` |
| [0028](0028-direct3d9-device-adapter.md) | Decision 9 (mod `ShaderDLL004` bytecode bound with its D3D9 registers on the core) withdrawn; gate D9-5 amended | It put a legacy format and register model inside a core port. The adapter itself, compiling the core's own programs to SM3, is an ordinary adapter and stays |
| [0027](0027-product-pipeline-lowering-streaming-kiln.md) | The VMT mapping and the material IR belong to `render.vmt-translation`; `VmtProfile` moves to `content.vmt` with the reader | Followed RFC 0016's old owner |
| [0001](0001-capability-based-platform-architecture.md) | Tier 0 facade rule 6: platform selection lives in the providers; R103 closes at the branch ratchet's floor | Its gate counted calls, so every branch survived |

The device-adapter RFCs (0022, 0024, 0025, 0026's PICA adapter, 0029) are
adapters on the device port and consistent with the boundary; whether to keep
building them is the scope decision below. RFC 0026's number is shared by
two RFCs (`0026-box3d-beyond-ivp.md` and `0026-pica200-device-adapter.md`);
renumbering is left to whoever next edits either.

## Recommended order

1. **Tier 0 platform branches** (R103): move each branch into the provider
   it belongs to, keeping a branch only where a frozen ABI declaration needs a
   native type. `threadtools.cpp`, `platform.h` and `threadtools.h` hold 177
   of 438.
2. **Render: one renderer.** In inventory order, the world, static props and
   posed models become scene instances, culling runs over the scene view,
   and the world pass draws a draw list. Add the runtime draw-origin census
   with the first slice. Move pass recording out of composition (RFC 0030's
   open step), then remove legacy types from the composition API.
3. **AGENTS.md**: cut the tracking narrative to one line per row with a link,
   as the file's own rule says, and move the narrative into the progress
   records.

## Decisions for the user

- **Scope.** Which adapters and profiles to keep building: the north star is
  Vulkan plus null; GL serves mods. D3D12, Metal, WebGPU, PICA, the proposed
  D3D9 adapter, 3DS, WebAssembly and tvOS were each user-directed. Freezing a
  scope means its suites stop running in the default gates; deleting it
  removes its code and suites.
- **Line-ceiling policy.** Ceilings are at today's size, so any growth now
  needs a recorded raise. If that is too strict for active feature areas, the
  alternative is a fixed headroom per area (for example 5 %) with raises
  above it.
- **Row order.** Whether "one renderer" (item 2) moves ahead of the current
  render-quality priority 0 (Source 2 lighting parity), since every lighting
  slice now adds to the world pass.
