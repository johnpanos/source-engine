# Native Vulkan frame pacing (portal stutter) — progress

Scope: the user-reported microstutter on Android when shooting portals and
walking up to them on the native Vulkan renderer (`shaderapivulkan`). The work
here reproduces the condition on Linux, installs a measuring harness, and burns
down the backend costs it finds. Roadmap context: R32 (native Vulkan MVP) and
the R29/R36 Android performance obligations. No gate is closed by this record.

## Harness

- `tools/quality/frame_pacing.py` runs a versioned scenario in a private staged
  runtime (staging via `portal_boot`), headless on the real GPU
  (`SDL_VIDEODRIVER=offscreen`), with a fixed simulation step
  (`host_framerate`) so a faster build does the same game work per frame.
- Scenario `quality/workloads/portal-frame-pacing-v1.json`: testchmb_a_02 first
  room at (-448 150 0); fizzle, fire blue (yaw 90), fire orange (yaw 270), look,
  walk through the linked pair repeatedly, walk back. Three passes in one
  process: pass 1 shows first-use costs, the last (warm) pass is gated.
- Backend telemetry (`-vkframestats <file>`, `vulkan_frame_stats.h`): one JSON
  line per presented frame with wall interval, presenting-thread CPU time,
  engine/backend split, GPU timestamp duration, vertex/index/upload bytes, and
  count + time of every blocking operation (pipeline_create, single_submit,
  device_wait_idle, texture_create/upload, buffer_grow, query_wait,
  fence_wait, acquire, record, submit, present, mesh_draw, emit, emit_reuse).
  `vk_frame_mark <label>` names scenario phases in the stream.
- Options: `--ab-build` + `--rounds` (interleaved A/B, medians over rounds —
  required on this shared host, where load moved even CPU time by ~35%),
  `--cold-shader-cache` (disables Mesa/NVIDIA disk caches, so pipeline builds
  cost what a first run or a mobile driver pays), `--pipeline-store DIR`
  (share the backend pipeline store between runs).
- Self-tests: `python3 -m unittest tools/quality/tests/test_frame_pacing.py`
  (21 tests: invalid scenarios, cfg chaining under the 512-char console limit,
  truncated/foreign stats, missing or collapsed marks, a seeded hitch with
  attribution, budgets, A/B summary).

Gotchas found while building it: `fps_max 0` makes this engine stop
presenting (default `--fps-max 1000`); frame-counted `wait`s before signon
finish never render without `host_framerate`; one console line over 512
characters is silently split and run out of order (hence chained cfgs); the
engine refuses a command line over 512 characters (keep `--out` short, the
harness checks); the spawn room walls are no-portal material.

## Findings (Linux, RADV Strix Halo, 1280x720)

Baseline (source `2b2ee370`, instrumentation only): once the orange portal
links the pair, frames go from ~4.8 ms to 19-24 ms and stay there, with 50-150
ms spikes on first use. ~80% of the frame is `emit`: the CPU conversion of each
draw's mesh into the frame's vertex stream, fully de-indexed (17 MB/frame),
with several heap vectors per draw. 74% of those vertices are skin-shader
draws (Chell and props seen through the portals, re-skinned per view). GPU time
is ~2 ms. Opening the orange portal also builds 18 pipelines lazily in that
frame (0.2-1.6 ms each on RADV cold; mobile compilers are far slower), and HUD
glyph uploads each submit and `vkQueueWaitIdle`.

## Changes

1. Emit writes straight into the stream (`BeginDynamicDraw`/`EndDynamicDraw`),
   no per-draw heap vectors, geometric stream growth, no zero-fill.
2. Indexed draws: each unique vertex converted once per draw, triangles as
   32-bit indices, `vkCmdDrawIndexed` (vertex bytes halved).
3. Per-frame-slot vertex/index stream buffers. Fixes a real race: one buffer
   was rewritten (and on growth destroyed) while the previous frame could
   still read it.
4. Within-frame reuse of converted geometry for repeat draws (portal views,
   multipass) keyed on mesh write revisions, range, flags, lights and the bone
   matrices actually referenced. `-vkemitreuseverify` converts every reused
   draw again privately and compares: 0 mismatches in 6635 reuses; with bones
   dropped from the key (negative control) it reports 575 mismatches.
5. Pipeline store: a `VkPipelineCache` plus a key list of every material
   pipeline variant used, in the mod directory (`-vkpipelinecache <dir|none>`);
   variants are prewarmed at device bring-up and saved at shutdown.
6. Texel uploads up to 256 KB are deferred and recorded at the start of the
   next frame's command buffer (barriers from the fragment stage order them
   after in-flight reads) instead of one submit + queue-idle each; larger ones
   stay synchronous after the pending ones.
7. `powf(x, 0)` skipped for point/directional light attenuation (bit-exact by
   C Annex F; no measurable effect here, kept as harmless).

## Evidence

- Interleaved A/B, 3 rounds, cold driver shader cache, warm pass
  (baseline `libshaderapivulkan.so` from `2b2ee370` vs current, all other
  libraries identical): median 18.4 -> 6.1 ms (0.33x), p99 25.9 -> 10.5 ms,
  max 26.7 -> 13.5 ms, emit 14.9 -> 4.1 ms.
- Pipeline store, cold driver cache, two runs sharing a store: run 1 builds 24
  pipelines mid-game in pass 1 (7.7 ms); run 2 prewarms 44 at bring-up and
  builds 0 in pass 1.
- Correctness: all nine material-pixel families (lightmap, exposure, skinning,
  portal, modellight x HDR modes) pass against their D3D9 fixtures and their
  captured pixels are byte-identical across every step above; backend suites
  pass headless (bring-up 25, backend 24, material-facing 13, equivalence 31
  checks); in-game screenshots of the scenario show the linked recursive
  portal views and glyph text.

## Not verified / next

- Nothing measured on Android. The pipeline and queue-idle stalls are the
  likeliest device stutter sources, but their size there is unmeasured; the
  harness has no Android runner yet.
- Remaining sustained cost is CPU skinning (~90 ns/vertex) of unique skinned
  draws; GPU skinning in `skin.vert` (bones + light attenuation in a UBO, as
  D3D9 does) is the next structural step and would also make skinned static
  meshes cacheable across frames.
- The first run still compiles pipelines on first use; a shipped seed key list
  and/or extended dynamic state (fewer depth/stencil/cull variants) would
  remove that.
- `SetManagedTextureSamplerState` still calls `vkDeviceWaitIdle` (not hit in
  this scenario).
- The relative hitch threshold penalises a faster build; per-profile absolute
  budgets (`max_frame_ms`) are the gate to set once device budgets exist.

## WMSH world draw submission (2026-09-23)

User report: the PBRT bedroom map (`quality/fixtures/pbrt-maps/bedroom.json`,
1.5 M triangles, 71,600 meshlets in 32 material batches) ran at ~15 FPS on
native Vulkan. Scenario `quality/workloads/bedroom-frame-pacing-v1.json`, run
with the new `frame_pacing.py --content-root <out>/content`.

Root causes (warm frame, 61 ms, CPU-bound; Radeon 8060S / RADV, 1280x720):

1. Per-meshlet submission: `Shader_DrawWorldMeshBatches` issued one material
   pass and draw per visible meshlet (71,629 per frame): engine `mesh_draw`
   38.2 ms, backend `record` 13.5 ms, GPU 18.2 ms of per-draw overhead.
   Visible meshlets whose index ranges are adjacent now go out as one range in
   the same primitive order: 32 draws.
2. Leaf visibility marking (found with `perf`: 72 % of samples after fix 1):
   every leaf references every meshlet (the packer's all-visible policy), so
   76 leaves wrote 5.4 M flags per frame, reloading the vector base each
   store. The loop now stops once every meshlet is marked, through a local
   pointer.

Interleaved A/B, 3 rounds each (`/tmp/claude-1000/fpr/bed-ab1`, `bed-ab2`):
HEAD 61-129 ms median -> coalescing 3.7-8.4 ms -> both fixes 2.1-2.4 ms
(CPU 1.2-1.7 ms; now GPU-bound, GPU median 2.0 ms). Oracle: camera-matched
1920x1080 frames from HEAD and the fixed engine, overlays off, are
byte-identical (sha256 `fcdcf2fc...`). Open: the packer still writes a full
reference list per leaf (~31 MB of the 272 MB bedroom BSP2); identical lists
should share one range, and real spatial visibility is still absent.
