# RFC 0030: Factoring `world_pass.cpp` and `core_world.cpp`

- Status: Implemented (2026-10-08); see Progress. Splitting `WorldPass::State`
  into sub-structs is not done.
- Date: 2026-10-08
- User direction (2026-10-08): split `render/pass/world/world_pass.cpp` and
  `render/composition/core_world.cpp` along the responsibilities they
  already hold, editing the code in place, with tests that pass the whole
  time.
- Render architecture: [RFC 0016](0016-render-core.md) owns the layers
  (archlint CAP011), the passes and the binding rules. This RFC changes no
  contract, pass order, shader or pixel.
- Tracking: no ranked row. A maintainability child of R89/R91; ranking is
  the user's decision.

## Why

At 2026-10-08 the two files are the render core's largest:

- `world_pass.cpp` is 6,336 lines. `WorldPass::RecordBatch` alone runs from
  line 2009 to 6284: one function of about 4,275 lines holding about 31
  lambdas (group building, material preparation, stage textures, residency,
  GPU culling, indirect draws, screen passes, dynamic draws) that share
  captured locals. `WorldPass::State` holds about 150 members.
- `core_world.cpp` is 4,529 lines in `render/composition/`, but it records
  passes itself: the world batch (`RecordWorldBatch`), stage shadows
  (`DrawStageShadows`), SSR, volumetric fog and temporal reconstruction, plus
  stage-capture decode and upload.

Both are drifting toward the monoliths the core replaced
(`shaderapivulkan.cpp`, 9,021 lines).

## Rule: edit in place, tests green throughout

The refactor edits the existing code in place, in small steps on the
working branch. No parallel copy, no second implementation kept beside the
old one, no long-lived branch: each step changes the code where it lives and
the old form is gone in the same commit.

Each step is behavior-preserving: no logic, ordering, allocation or
algorithm change, and no "while I'm here" fixes. A fix found during the work
is its own commit.

### Tests

- **Tests pass the whole time.** Every commit builds (g++ and clang++) and
  passes the render suites that cover the code it touches; none is ever left
  red, skipped or disabled to land a step.
- **Move with confidence, test whole systems.** A piece that is plainly a
  move (a helper, a lambda lifted to a function, a struct split) is moved in
  place without a test of its own. Tests sit at the system level: the world
  pass, the composition and each pass module are exercised whole by their
  suites, and a system whose coverage is thin gets a system-level test
  before its pieces move.
- **The relevant suites per step**: `render.device.v2` (null and Vulkan),
  the graph suites, the world-pass and composition suites, and the
  `render.lab.*` suites that drive the moved code. Steps that move pass
  recording out of `core_world.cpp` also run a native Portal 2 boot with
  sync validation on.
- Every commit also passes `python3 tools/archlint/archlint.py check --all`
  and `stylelint --changed --diff` on the edited files.

## Target layout

### `render/pass/world/`

| File | Takes |
| --- | --- |
| `world_materials.cpp` | `MapWorldMaterial`, `MaterialSnapshotKey`, `StaticMaterial`, `DynamicClaim`, `State::Mapped`, material preparation and texture slots |
| `world_lightmap.cpp` | half-float helpers, `SplitLightmapLayer`, `BlockLightmapLayer`, footprint bounds and mip feedback |
| `world_stage_resources.cpp` | stage textures, RPRB cube array, probe volume and regions, grid table, neutral inputs, split-sum table; the `SetStage*` setters |
| `world_residency.cpp` | `UploadModelLevel`, `SweepModelResidency`, `PublishModelResidency`, released staging, pipeline prewarm and keys |
| `world_groups.cpp` | frame, view and draw group building and their failure tracking, lit view groups, area-light packing |
| `world_gpu_submit.cpp` | occlusion pyramid, S3/S4 cull dispatches, indirect buckets |
| `world_screen_passes.cpp` | depth prepass, AO hookup, planar reflection and water refraction imports |
| `world_draw.cpp` | `drawSurfaces`, range merging, model-draw binding and constants, dynamic draws, sprite-card expansion, water-plane draws |
| `world_pass.cpp` | public API, `QueueView`, and `RecordBatch` as the ordered sequence of the above |

The enabling step: `RecordBatch`'s captured locals become one explicit
`BatchContext` (encoder, target, view, slot failure, borrowed `State&`);
each lambda becomes a function taking it. `State` splits into sub-structs
(`MaterialCache`, `StageResources`, `Residency`, `GpuSubmit`) owned by the
matching files.

### `core_world.cpp`

Pass recording leaves the composition layer for its pass module:

| Code | Destination |
| --- | --- |
| `DrawStageShadows`, `ReleaseShadows`, caster setup | `render/pass/shadow/` (beside the K7 atlas owner, not a second owner) |
| `EnsureSsr`, `RecordSsr`, `ReleaseSsr` | `render/pass/ssr/` |
| `RecordVolumetric` | `render/pass/fog/` |
| temporal capture, reconstruction, history | `render/pass/temporal/` (RFC 0019's owner) |
| `StageCapture` decode and upload | `render/map_media/` or `world_stage_resources.cpp` |
| view lights, area lights, cookies | `core_world_lights.cpp` |
| `QueueMesh`, UI list, UI materials | `core_world_handoff.cpp` |
| stats, costs, GPU timers, quality | `core_world_diagnostics.cpp` |
| `CoreWorld` | setters, `BeginFrame`/`EndFrame`, `RecordSlot` and the order it calls passes |

A destination that would need an upward edge stays in composition until a
separate change resolves it; CAP011 decides.

## Order

1. Self-contained pieces: lightmap helpers, materials, diagnostics.
2. `BatchContext`, then lift lambdas one group per commit.
3. Split `State` into sub-structs.
4. Move pass recording out of `core_world.cpp`, one pass per commit.

New files enter the wscript and `architecture/modules.json` in the same
commit, so the shared tree never breaks.

## Done

No file under `render/pass/world/` or `render/composition/` exceeds about
1,500 lines, no function exceeds about 400, `core_world.cpp` records no pass
itself, and each moved system is covered by a system-level suite, and every commit along the way built and passed its suites.

## Progress (2026-10-08)

State: implemented on `subsystem-refactor` (source-engine-6e). Every
function in `render/pass/world/` and the `core_world*` files is under 400
lines; the largest file is `world_draw.cpp` at 988.

Baseline first: `render.world.null` W9 and `render.composition` P7 were stale
against deliberate resolver and HUD-slot changes, and the composition suites'
source lists missed `depth_alpha.cpp` and the VTF decompressor. Fixed in
`787fc27d3`, so all five headless suites passed before any move.

### `render.pass.world`

- Step 1 (`c92f1949e`): `world_pass_internal.h` holds the pass's private
  types and `WorldPass::State`; members split into `world_materials.cpp`,
  `world_lightmap.cpp`, `world_stage.cpp`, `world_residency.cpp` and
  `world_record.cpp`.
- Step 2: `RecordBatch` (4,275 lines) keeps its queue-taking and validation,
  then builds `WorldPass::Batch` (`world_batch.h`), an aggregate made with
  designated initializers. Its steps run in the original order:
  `PrepareResources`, `PrepareFrameTerms`, `PrepareViewGroups`,
  `PrepareSurfacesAndModels`, `PrepareDrawHelpers`, `RecordCutoutShadows`,
  `PrepareGpuSubmission`, `RecordScreenPasses`, `PrepareDynamicDraws`,
  `RecordView`; a step returns false where `RecordBatch` returned.
  - Locals used by more than one step, and lambdas called from more than
    one step, became members (36 functions); the rest stayed local,
    unchanged.
  - Bodies moved verbatim. The edits are only those the move forced:
    explicit return types, declarations turned into assignments,
    `pass.` before the three `WorldPass` members, and `{}` initializers.
  - One capture changed form: the pipeline-key sink outlives the batch,
    so it binds `State` (`[&s = s, tag]`), as `[&s]` did before.
  - Files: `world_resources.cpp`, `world_groups.cpp`, `world_view.cpp`,
    `world_draw.cpp` (draw helpers and GPU-driven submission; the template
    `submitSurfaceFootprints` and all its callers are in it),
    `world_cutout_shadows.cpp`, `world_screen_passes.cpp`,
    `world_dynamic.cpp`, `world_record.cpp`.

### `render.composition`

- `core_world.cpp` keeps the frame lifecycle, `DrawView` and posed models.
  The rest moved to `core_world_scene.cpp`, `core_world_stage_capture.cpp`,
  `core_world_lights.cpp`, `core_world_temporal.cpp`,
  `core_world_diagnostics.cpp`, `core_world_handoff.cpp`,
  `core_world_record.cpp`, `core_world_volumetric.cpp`,
  `core_world_ssr.cpp` and `core_world_shadows.cpp`.
- `RecordWorldBatch` lost its target setup to `PrepareWorldTarget`.
  `DrawStageShadows` lost its moving casters to `CollectMovingCasters`, and
  its plane lambdas became `PlanesOf` and `ChunkInside`.
- Pass recording stays in composition. Moving it into `render.pass.*`
  would be a redesign, not a move: `RecordVolumetric` uses
  `render.map-media` (a layer above the passes), and the SSR, volumetric
  and shadow recorders own `CoreWorld` state (renderers, targets, atlases,
  `BindStageDevice`). That takes a separate change that gives the passes
  that state.

### Not done

- Splitting `WorldPass::State` into sub-structs (order item 3). Every
  access would be renamed for no size or ownership gain over the split
  files; left for when an owner needs it.

### Evidence

- Headless, g++ and clang++, at every commit: `render.world.null` (159
  checks), `render.composition` (65), `render.composition.capabilities`
  and its GL and GLES variants (14 each).
- `render_lab` (`build-rc-lab`, 59 suites): identical per-check results
  across the pre-split binary, step 1 and step 2. These fail the same way
  at HEAD before any move:
  - `view-state.aperture-does-not-claim-stage-0`;
  - posed-model's `refract-refuses-missing-scene-color` and
    `cable-refuses-missing-required-normal-texture`;
  - `shadow-receiver-perf` (and its sensitivity);
  - two seeded portal-lights controls.
- Unavailable here: the FSR tree, Cornell and Portal 2 Steam content,
  and the Apple device.
- archlint shows no finding in these paths; stylelint is clean on every
  edited file.
- Lab run 3 (final layout): identical per-check results again. Portal 2
  (`portal2-linux-native-vulkan`, warnings on) builds; a headless
  `portal_boot.py` run on `sp_a1_intro4` passes.
