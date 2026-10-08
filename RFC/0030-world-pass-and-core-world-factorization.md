# RFC 0030: Factoring `world_pass.cpp` and `core_world.cpp`

- Status: Proposed (2026-10-08); nothing implemented.
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
