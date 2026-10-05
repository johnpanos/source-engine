# sp_a1_intro4_relit: render core optimization captures and proposals

Date: 2026-10-05. Optimization evidence and proposals under
[RFC 0016](0016-render-core.md) for R90/R95/R96/R91 and the
`linux-desktop-high-120` budget row. **No engine change is proposed as delivered
here and no roadmap gate changes.** Every proposal below is a proposal with its
required evidence.

Source revision `51a15923e` plus the working tree (dirty: the uncommitted
`render/material/program_resolver.cpp` neutral-default change, 4 files). Host:
AMD Ryzen AI Max+ PRO 395, Radeon 8060S (RADV STRIX_HALO), Mesa 26.2.3, Vulkan
1.4.354, 32 cores. Every run is the real Portal 2 native Vulkan product through
`./play_p2`, 1920x1080 back buffer, the pinned High settings, 4x MSAA, vsync
off, in a private headless mutter at 120 Hz. All timings are medians of the
retained `frames.jsonl`; CPU and GPU are the backend's own per-frame measures.

The published map is the P2:CE-mounted relit package
(`run/maps/sp_a1_intro4_relit/published.json`, `status: pass`, 534 files,
content root `quality-results/relight/sp_a1_intro4_source2_aperturevr`).

## Result 1: the budget row fails on this map by a wide margin

`quality-results/relit-perf-20261005/base/` is a run of the new budget-linked
workload `quality/workloads/portal2-intro4-relit-perf-v1` against
`linux-desktop-high-120`. It pins every High setting through
`configure_budget` and verifies them in the console receipt.

| Phase | Interval p50 | CPU p50 | GPU render p50 |
| --- | ---: | ---: | ---: |
| arrival | 90.8 ms | 85.5 ms | 58.7 ms |
| reverse | 35.7 ms | 31.2 ms | 24.8 ms |
| return | 84.6 ms | 79.4 ms | 53.7 ms |

Whole route: 360 frames over 21.6 s, median 37.9 ms, p99 167.5 ms, 1% low 5.8
fps. Failures: `cpu_p99_ms` 139.1, `cpu_max_ms` 140.9, `gpu_render_p99_ms`
125.8, `gpu_render_max_ms` 130.7, every frame below the 120 fps floor. The row's
own coverage text already names this map ("sp_a1_intro4_probe64, relit").

**The frame is CPU-bound before it is GPU-bound.** At arrival, CPU 85.5 ms
against GPU 58.7 ms; in the settled reverse view, CPU 31.3 ms against GPU
23.8 ms. `backend` CPU (the material system's own frame) is 77.9 ms of the
arrival frame's 85.5 ms, while the host/engine portion is 7.9 ms.

A matched capture of the existing `sp_a1_intro4_probe64` workload
(`quality-results/rp/probe64`) is *worse*, not better: arrival interval 111.9
ms, CPU 104.8 ms, GPU 53.5 ms. The relit map is not the pathological package;
the core path is generally slow on Intro4. This matters because the retained
RCV tuning (below) was measured on `probe64`.

## Result 2: the cost is the core's per-frame CPU recording

Three `perf record -g` captures of 12 s each, started only after the measured
bracket's `floor_begin` mark appears in `frames.jsonl`, so no capture contains
map load. Samples are normalized per frame (a faster configuration renders more
frames in the same wall-clock window, so raw sample counts are not comparable).

| Symbol (inclusive) | base | AO off | viewmodel off |
| --- | ---: | ---: | ---: |
| `CoreWorld::RecordWorldBatch` | **65.2%** | 42.3% | 61.4% |
| `WorldPass::RecordBatch` per-draw lambda | 58.8% (57.3% self) | 33.7% (33.3% self) | 55.6% (54.5% self) |
| `VulkanDevice::Submit` / `Translator::Encoder` | 5.0% | 6.9% | 5.7% |
| `Translator::RunSectionsThrough` | 4.2% | 5.4% | 4.6% |
| `CEmptyMesh::EmitToCoreQueue` | 3.9% | 8.4% | 3.7% |
| `VulkanDevice::Validate` | 2.2% | 1.4% | 1.7% |
| `MapWorldMaterial` -> `MapVariables` -> `MapValues` | 2.7% | 6.0% | 2.6% |
| `WorldPass::QueueMesh` | 2.7% | 6.0% | 2.4% |
| `__memmove_avx512` | 1.4% | 2.0% | 1.3% |

Per-frame sample counts (223 base / 124 AO off / 152 viewmodel off per frame):

- `RecordWorldBatch` = **146 samples/frame** base, 52 with AO off, 94 with the
  viewmodel off.
- That decomposes exactly: 2 views x 47 samples of ambient-occlusion screen-pass
  recording + 52 samples of everything else (26 per view).
- **The AO screen-pass block is 64% of one core view's recording CPU.**

`render/pass/world/world_pass.cpp` records that block per view (the
`target.screenPasses` branch at ~3288, guarded by `screenReusable`). Inside it,
the prepass draw lists are rebuilt from scratch on every call: a loop over every
`world->surfaces` record, and a nested loop over **every static instance x every
surface of its mesh** (~448 static props and 65 static models on this map), each
iteration calling `materialReadyIn`, `drawGroupReady`, `frameGroupReady` and
`viewGroupReady` before pushing to a vector. The lists depend on the world
generation and the prepass material revisions, not on the view.

## Result 3: the second core view per frame is the viewmodel scope

`+r_drawviewmodel 0` (`quality-results/rp/novm`) is the largest single
diagnostic win found, and it is not a rendering-cost result at all:

| Phase | Interval p50 | CPU p50 | GPU p50 | Core views/frame | GTAO recordings | Core CPU recording |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| base | 85.6 ms | 80.1 ms | 45.2 ms | 2 | x2 | 18.2 ms |
| viewmodel off | **67.3 ms** | **62.3 ms** | **37.2 ms** | 1 | x1 | 11.4 ms |

`render->Push3DView(viewModelSetup, ...)` at
`game/client/viewrender.cpp:1137` reaches `CRender::Push3DView`, which calls
`RenderCoreHost_MarkViewBegin()` and `RenderCoreWorld_ViewBegin()`
(`engine/gl_rmain.cpp:714`), so the viewmodel's 3D view becomes a second full
core world view: its own cluster light assignment, its own
world depth prepass, its own GTAO, its own share of the 14 per-frame world batch
recordings. The retained RenderDoc audit of this map
(`quality-results/renderdoc-forward-core-20261004/sp_a1_intro4_relit/`) shows
the same duplication in the action tree: `core world prepass` 499 markers under
`CSimpleWorldView::Draw` **and** 499 under `DrawViewModels`, `core world gtao` 2
and 2, and 7,482 `shadow-depth` draws in two identical sets of 3,568 indexed
static caster draws plus 90 tile clears each.

## Result 4: control matrix

All rows: pinned High settings, `sp_a1_intro4_relit`, the same cameras and
marks, arrival-view medians unless stated. `diag-base` is the same
configuration as the budget run reached through the diagnostic workload; the
6% difference between the two base rows is the run-to-run noise floor and every
delta below is read against it.

| Configuration | Interval | CPU | GPU | Evidence |
| --- | ---: | ---: | ---: | --- |
| base (High, core) | 85.6 ms | 80.1 ms | 45.2 ms | `rp/d`, `rp/pf-base` |
| budget row, same settings | 90.8 ms | 85.5 ms | 58.7 ms | `relit-perf-20261005/base` |
| `+r_drawviewmodel 0` | 67.3 ms | 62.3 ms | 37.2 ms | `rp/novm`, `rp/pf-novm` |
| `+r_core_ao_quality 0` | 51.8 ms | 46.8 ms | 31.6 ms | `rp/ao0p` |
| `+r_core_shadow_quality 0` | 73.7 ms | 68.2 ms | 34.6 ms | `rp/s0` |
| `+r_core_depth_prepass 0` | 117.3 ms | 111.4 ms | 76.3 ms | `rp/pre0` |
| `-vkopaquebatch 0` | 92.4 ms | 86.8 ms | 51.9 ms | `rp/nobatch` |
| `+r_core_world 0` (legacy world) | 10.4 ms | 9.8 ms | 9.2 ms | `rp/legacy` |

Core GPU pass table for the settled reverse view (per frame, from
`cl_render_debug_stats`; `x` is executions per frame):

| Pass | base | AO off | viewmodel off | shadows off | depth prepass off |
| --- | ---: | ---: | ---: | ---: | ---: |
| core world view (all scopes) | 22.3 ms x14 | 15.5 ms x14 | 19.2 ms x8 | 13.3 ms x14 | 33.0 ms x19 |
| world surfaces (lit) | 12.8 ms x14 | 13.0 ms x14 | 15.1 ms x8 | **4.9 ms** x14 | 19.6 ms x19 |
| core world gtao | 5.43 ms x2 | - | 2.88 ms x1 | 5.26 ms x2 | 6.94 ms x2 |
| core world prepass | 0.51 ms x2 | - | 0.27 ms x1 | 0.91 ms x2 | 0.51 ms x2 |
| core world depth / model depth | 0.06 / 0.03 ms | 0.06 / 0.03 ms | 0.07 / 0.02 ms | 0.07 / 0.04 ms | - |
| models posed / pbr | 2.20 ms x2 | 1.27 ms x2 | 0.15 ms x1 | 0.96 ms x2 | 4.22 ms x2 |
| cluster BVH assignment | 0.61 ms x2 | 0.43 ms x2 | 0.24 ms x1 | 0.61 ms x2 | 0.53 ms x2 |
| core world view CPU recording | 18.2 ms | 5.9 ms | 11.4 ms | 15.7 ms | 16.0 ms |

Readings:

- **Shadows cost ~8-10 ms of GPU per frame** (world surfaces 12.8 -> 4.9 ms),
  the largest GPU item, consistent with the earlier Forward+ diagnosis that
  receiver visibility dominates.
- **The depth prepass (RCV-09) is load-bearing**: removing it costs +31 ms of
  arrival GPU and +6.8 ms of the settled view's lit surfaces.
- **GTAO costs 2.7-3.5 ms per recording**, against the row's own
  `lighting_components.gtao_ms` of 0.5 ms — 5-7x over its component budget.
- Opaque batching is mildly helpful and stays on (turning it off is 8% worse).
- Shadow tiles are cached correctly in the warm steady state: the counters read
  "32 kept, 0 shared, 0 with movers" per frame, and the 7,482 shadow draws in
  the RenderDoc capture are a first-frame/newly-dirty-plan frame, not the
  steady state. The *duplicated* second set is the part that is real.

## Result 5: cost context against the legacy world path

`+r_core_world 0` on the same map with the same High settings runs the arrival
view at 10.4 ms interval, 9.8 ms CPU and 9.2 ms GPU (135 fps median), against
the core path's 85.6 / 80.1 / 45.2.

This is **cost context, not a parity judgement and not an acceptance
comparison**: the legacy path draws the frozen legacy lighting (no core GTAO,
clustered runtime direct light, probe volume, reflection probes or SSR), so the
two images differ and the legacy path is not a candidate product. What it does
establish is that on this hardware and content the core's own CPU work, not
GPU shading and not the base engine, is what currently makes the frame slow.

A second strategic fact: the host/engine portion of the frame is 7.9-8.8 ms
even on the legacy path, and this fixture runs a **listen server** in the same
process (`CEngineAPI::RunListenServer` in the profile) with the QA driver
active. The row's limits are 8.333 ms for CPU p50/p95/p99/max. A player-client
measurement is therefore needed before any renderer work can be said to be
able to satisfy this row at all.

## Ranked proposals

Each item names its existing owner, the mechanism, the size the captures
support, and the evidence required to keep it. Nothing here reduces an effect,
a sample count, a cohort or a resolution to pass a budget.

### P1 — Do not record world screen passes for the viewmodel scope (largest)

Owner: `render.pass.world` (`WorldPass::RecordBatch`'s screen-pass branch) with
`render.composition.core_world` and the view marking in
`game/client/viewrender.cpp`. Mechanism: a core view that carries only viewmodel
models must not trigger the world's depth/normal prepass, GTAO, cluster
assignment or shadow-atlas build; it should bind the lighting inputs the main
view already produced, or the neutral occlusion, and be drawn with its own
projection and depth range. Supported size: **-18.3 ms interval, -17.8 ms CPU,
-8.0 ms GPU per frame** (measured as the whole duplicate scope; the screen
passes are the majority of it).
Evidence: exact image comparison of the viewmodel and held-object pixels
against the current frame in the strict game runner's `cameras` and
`materials` scenes; the core's `render.pass.world` lab suites unchanged; the
core still reports the viewmodel scope as drawn; per-frame core view count 1.
Risk: the viewmodel currently receives a *world-projection* GTAO. Any change to
which occlusion it reads is a visible image decision and must be reviewed, not
assumed.

### P2 — Build the AO prepass draw lists once per world revision, not once per view

Owner: `render.pass.world`. Mechanism: cache the prepass list (the
`world->surfaces` opaque subset and the static instance x surface subset) keyed
by world generation plus the prepass material revisions, and record the screen
passes once per frame when `screenInputs` match, instead of rebuilding the lists
inside every `RecordBatch` call. Supported size: **~47 CPU samples per view per
frame, 15.3 ms/frame at two views (72% of the core's recording CPU)**; the
remaining GPU cost is the 2.7-3.5 ms duplicate GTAO.
Evidence: unchanged prepass images and identical draw order; a cache-hit/miss
counter; the lab's exact depth/normal image comparisons for the prepass
targets; a negative fixture where a world or material revision must invalidate.

### P3 — Eliminate the duplicated world geometry between the two depth passes

Owner: `render.pass.world`. Mechanism: with GTAO enabled the world is drawn
three times - the private AO prepass (D32F + RGBA16F normals), the final-target
world depth prepass, and the final-target model depth prepass - plus GTAO plus
the lit pass. The private prepass's depth is the same coverage as the
final-target prepass. One prepass whose depth is shared (or copied) with GTAO
reading it would remove a full geometry pass.
Evidence: measured before/after draw counts per pass from the RenderDoc action
tree; unchanged lit and GTAO images; correct behavior when the final target's
depth range is remapped or stencil is mutated (the existing
`depthPrepassSafe` conditions).

### P4 — Stop re-importing VMT variables per frame (material cache)

Owner: `render.material` (`MapVariables`/`MapValues`) with
`render.pass.world`'s `MapWorldMaterial`. Mechanism: `MapWorldMaterial` runs a
full VMT import for every surface, instance and dynamic draw, every frame:
2.7% of process CPU here, dominated by `MapValues`' string compares (2.2%, with
`__memcmp` under it). Memoize the imported `MaterialDesc` per material id and
world generation, invalidated through the owner's committed change. This is the
same class of fix as the uncommitted `program_resolver.cpp` neutral-default
work, and belongs beside it rather than in a second cache.
Evidence: byte-identical program and parameter-block selection; a revision
invalidation test; unchanged material pixel families.

### P5 — Stop copying `WorldView` per batch

Owner: `render.pass.world`. Mechanism: `RecordBatch` copies a `WorldView` (with
its `DynamicDraw` vector) while merging cohort slots - 1.4% of process CPU in
`__memmove` alone. Take the slot by reference and merge only what is needed.
Evidence: unchanged recorded draw order; the existing opaque-batch oracle.

### P6 — Shadow receiver cost (largest remaining GPU item)

Owner: `render.pass.shadows`, `render.material`. The captures re-confirm
8-10 ms/frame, which the earlier diagnosis and the RCV-01..RCV-10 trials
already own; those trials are recorded in
[the progress record](0016-progress.md#k11k12-shadow-receiver-microbenchmark-slice-2026-10-02-in-progress)
and this note adds no new receiver microbenchmark. The untried option with the
strongest prior is the one the read-only lit depth already enables: verify that
a fragment rejected by the prepass depth never reaches the shadow taps, and
that clipping/discard happens before them, then attribute the remaining cost to
actual compiled surface variants (the technique RCV-07 used diagnostically).
Evidence: exact filter/visibility semantics within the existing oracle,
compiled-variant attribution, repeated full-image timings at matched clocks.

### P7 — GTAO component cost

Owner: `render.pass` / composition, against `lighting_components.gtao_ms: 0.5`.
At 2.7-3.5 ms per recording the pass is 5-7x its own component budget, and P1/P2
remove one of the two recordings. After P1 and P2, decide the remaining gap with
a measurement: half-resolution AO with a bilateral upsample, or a measured step
count at `r_core_ao_quality 3`. Any such change needs an AO image oracle and a
negative control; a quality reduction is not a proposal here.

## Threats to validity and measurement gaps

- **Noise floor ~6%**: the two base configurations differed by 6% at arrival
  (85.6 vs 90.8 ms) with identical settings. Deltas above ~10% are real; smaller
  ones are not yet established. Every accepted change needs interleaved ABBA
  repeats.
- **Static cameras.** The fixture holds three fixed views. P1 and P2 are
  per-frame costs and should survive camera motion, but that must be measured
  with a motion route, not assumed from these captures.
- **Listen-server fixture.** The host/engine CPU floor (7.9-8.8 ms) includes a
  co-resident server and the QA driver. A player-client capture is needed before
  the row's CPU limits can be attributed to the renderer.
- **No resolution sweep.** `frame_floor.py` pins a budget row's exact
  dimensions and has no sweep command, so all points here are 1920x1080 only.
  RFC 0016's resolution-sweep requirement cannot be met with the current
  collector; that gap is recorded, not worked around.
- **Two profiling methods disagree on absolutes and agree on structure**:
  `cl_render_debug_stats` (GPU pass and CPU recording totals, in
  milliseconds) and `perf` (whole-process CPU shares). This note uses the first
  for GPU and the second for CPU, and does not add them together.
- **Content gap, not performance**: the published package logs 33
  `couldn't find materials/maps/sp_a1_intro4_relit/*.vtf` messages at load,
  including `cubemapdefault.vtf` and every `c-*.vtf` environment cubemap. They
  occur once at load, not per frame. The map's image is missing those cubemaps;
  that is a package/authoring question for the user, and any performance work
  that changes reflection results must not be judged against the fallback.

## Reproduce

```sh
# the budget row on this map (fails; retains the whole run with --no-stop)
python3 tools/quality/frame_floor.py --profile --no-stop \
  --workload quality/workloads/portal2-intro4-relit-perf-v1/workload.json \
  --out quality-results/relit-perf-20261005-NEW

# the same settings through the diagnostic workload, which accepts switches
W=quality/workloads/portal2-intro4-relit-diagnostic-v1/workload.json
python3 tools/quality/frame_floor.py --profile --no-stop --workload $W \
  --out quality-results/rp/NAME --runtime run/rt --extra-arg "+exec render_budget_high"
# then repeat with, for example:
#   --extra-arg "+r_drawviewmodel 0"
#   --extra-arg "+r_core_ao_quality 0"
#   --extra-arg "+r_core_shadow_quality 0"
#   --extra-arg "+r_core_depth_prepass 0"
#   --extra-arg "+r_core_world 0"
#   --opaque-batching off
#   --preview            (retains matched screenshots)

# per-frame and per-pass analysis
python3 tools/quality/render_profile.py \
  quality-results/rp/NAME/r/frames.jsonl \
  --console quality-results/rp/NAME/r/console.log --json /tmp/profile.json \
  > /tmp/profile.tsv
rg '^metric|^gpu_segment|^cpu_inclusive' /tmp/profile.tsv

# whole-process CPU attribution, started only after the bracket begins
perf record -p "$(pgrep -x hl2_launcher)" -g --call-graph dwarf -o /tmp/perf.data -- sleep 12
perf report -i /tmp/perf.data --children --percent-limit 1 --stdio --sort symbol
```

Two workloads were added for this map. `portal2-intro4-relit-perf-v1` is the
budget-linked acceptance workload (identical cameras, marks and warming to
`portal2-intro4-perf-v1`, so the two packages differ only in content) and is
what should own acceptance for this map. `portal2-intro4-relit-diagnostic-v1`
is the same route without the budget row, because `frame_floor.py` refuses
render switches on a hard row; it must be given
`+exec render_budget_high` to hold the pinned settings and none of its results
are acceptance evidence. Its scenario name is the short `r` because the Linux
launcher refuses a command line over `MAX_LINUX_CMDLINE` (512,
`tier0/vcrmode_posix.cpp`) once the diagnostic arguments are added.

Retained local evidence: `quality-results/relit-perf-20261005/` (budget run),
`quality-results/rp/` (diagnostic base, five controls, three bracket-synced
`perf` runs, screenshots, `summary.json`, `core-passes.json`).
`quality-results/` is untracked; these receipts travel with the change only as
documented numbers.
