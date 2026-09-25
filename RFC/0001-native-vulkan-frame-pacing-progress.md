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

- Superseded (2026-09-23): the portal scenario has since run on a Galaxy Z
  Fold7 (see [device result](#device-result-galaxy-z-fold7-adreno-840-2026-09-23)),
  through an investigation script, not installed tooling. Originally:
  Nothing measured on Android. The pipeline and queue-idle stalls are the
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
Superseded (2026-09-23): imported maps now get per-leaf meshlet references and
meshlet frustum culling (leaf references 5.4 M -> 227 K); see
[RFC 0008 F4 WMSH spatial visibility](0008-progress.md#f4-wmsh-spatial-visibility-for-imported-maps-2026-09-23).

## Mobile GPU cost: render-pass breaks (2026-09-23)

Question: where the native backend spends GPU cost that a tiled mobile GPU
pays and a desktop GPU hides, and which of it can go without changing pixels.
Desktop RADV reports 0.3-0.9 ms of GPU time for the portal scenario, so the
cost cannot be read from GPU timestamps here.

Method: an investigation-only `LD_PRELOAD` counter around the Vulkan loader
(render passes with their load/store ops, clears, blits and copies, and each
pipeline's color-write mask and blend state) on the portal scenario at
1024x720. It uses a tiled-GPU traffic model: every attachment `LOAD` reads the
target and every `STORE` writes it (4 B/px color, 3+1 B/px depth/stencil). The
model is an upper bound, since a driver may render a pass directly to memory,
and it is not a device measurement. A read-only feature probe ran on the
attached Galaxy Z Fold7 (Adreno 840, Vulkan 1.4.295): `textureCompressionBC`,
`ETC2` and `ASTC_LDR` are supported, so DXT textures already stay compressed
there. It also exposes `VK_KHR_load_store_op_none`,
`VK_QCOM_render_pass_transform`, `VK_KHR_dynamic_rendering_local_read` and
`VK_EXT_rasterization_order_attachment_access`.

Finding: once the portals are linked, a frame used 31-33 render passes, each
of which loaded and stored color, depth and stencil. The sequence showed no
render-target-texture passes. Most breaks were the back buffer switching
between its sRGB and UNORM views. Every UNORM-view pass held only color-masked
draws (the portal stencil masks) or depth/stencil-only clears, whose results
do not depend on the view. Other breaks were empty passes reopened between
back-to-back copies, one copy that repeated the previous one exactly, and a
UNORM pass opened only to end the frame.

Change (`vulkan_device.cpp` stream replay):

- queries, depth/stencil-only clears and color-masked draws keep the open
  view; opening a pass takes the view of the next record that needs one;
- a copy closes the pass and the next record reopens it;
- a copy identical to the previous one, with nothing drawn since, is skipped;
- the frame may end in the sRGB view.

`render_pass` and `target_copy` are now count-only kinds in `-vkframestats`.

| Phase (median frame) | Passes before | After | Modeled MB/frame at 1024x720, before | After |
| --- | --- | --- | --- | --- |
| idle | 10 | 5 | 126 | 60 |
| fire_orange | 33 | 8 | 431 | 100 |
| walk_blue | 25 | 5 | 325 | 60 |
| walk_orange | 32 | 7 | 418 | 86 |

Scaled to the Fold7 inner back buffer (2448x1848), the walk_orange model goes
from about 2.6 GB to about 0.53 GB per frame (154 to 32 GB/s at 60 FPS).

Evidence (A = backend from `d7fc2598`, B = the same source plus this change;
all other libraries identical):

- Pixels: all 10 `material_pixel_conformance` families in both HDR modes give
  byte-identical `pixels.json` for A and B. The exposure family differs only
  in `poll_frames`, which also varies between runs of A. `sky/integer` and
  `pbr-fallback` fail identically on A and B (pre-existing).
- Backend suites pass: bring-up 79 checks, backend 24, material-facing 20,
  equivalence 31. The new bring-up case gives 3 passes and 1 copy. Built
  against the old policy, it reports 7 passes and 2 copies, and its occlusion
  query fails because the old policy split it across a view change (3
  failures).
- The in-game screenshots were not an identity oracle: two runs of A already
  differ across the whole frame (particles, weapon sway, timing).

Not verified / next:

- Nothing has run on Android yet. The traffic figures are modeled, not
  measured. The app was not launched on the device, and the harness still has
  no Android runner. (Superseded the same day by the device result below. The
  harness still has no Android runner: `frame_pacing.py` has no device path.)
- Pixel-identical follow-ups, largest first:
  1. open the frame's first pass through the view its first color draw needs
     (an sRGB `CLEAR` variant; 1 break per frame);
  2. present by rendering straight into the swapchain image when the sizes
     match and gamma is identity, dropping the full-frame blit (8 B/px);
  3. store depth/stencil `DONT_CARE`/`NONE` in the frame's last pass;
  4. pre-rotate for `currentTransform` (`VK_QCOM_render_pass_transform`)
     after confirming on the device whether the compositor rotates the frame.
- The remaining breaks are the engine's framebuffer copies for refraction
  (5-7 per linked-portal frame). Framebuffer fetch could replace them, but
  that is a larger design decision.
- Excluded because they change pixels: a lower render scale, reduced
  precision, and lower anisotropy.

### Device result: Galaxy Z Fold7, Adreno 840 (2026-09-23)

The same APK was run with `-vkpassmerge 1` and `0` (arm64 debug build of the
working tree, which also contained other sessions' uncommitted backend edits;
both modes use the same binary). Settings: native 2448x1848, `fps_max 1000`,
`host_framerate 60`, portal scenario with 2 passes, the warm pass measured, and
3 runs per mode in ABBA order. The runner was an investigation script, not
installed tooling.

| Warm pass (median) | Merge on | Merge off |
| --- | --- | --- |
| Render passes per frame | 5 | 31 |
| Copies per frame | 5 | 6 |
| GPU ms (per run) | 7.62, 7.21, 7.61 | 7.30, 7.04, 7.65 |
| Frame interval ms | 8.58 | 8.47 |
| GPU busy (kgsl) | 85-88 % | 84-87 % |

The pass count falls as designed, but GPU time does not: the difference is
within run-to-run spread. The tiled-traffic model above does not describe this
driver. Adreno evidently does not store and reload the target at every pass
boundary (direct rendering and driver-side pass merging are both plausible;
neither was measured). One phase goes the other way: during the portal opening
(`fire_orange`) all three merged runs (8.3-10.2 ms) were slower than all three
unmerged runs (7.4-7.7 ms). The sample is small and the device was thermally
warm (status 2), so this is unconfirmed.

Consequences:

- This change is not a GPU saving on the Adreno 840. Pixels are unchanged, and
  it fixes occlusion queries that were split and failed around color-masked
  draws.
- GPU cost attribution on the device needs a GPU profiler (Android GPU
  Inspector or Snapdragon Profiler: per-pass time, binning or direct mode,
  bandwidth counters) before more backend changes are aimed at it.
- The working tree at the time started the game at 640x480 and then about
  931x703, not the panel size. `-w 2448 -h 1848` was passed for these runs.

## R32-PARALLEL-EMIT: CPU vertex conversion on the engine pool (active, 2026-09-25)

Scope: the job-based follow-up to this record's profile. "Emit"
(`kCostEmit`) is converting each pass's mesh into the frame vertex stream,
mostly CPU skinning. It is several times the command-recording cost, so it
is the main-thread (or MatQueue) cost worth splitting. Slices:

- **(A) Measure.** Add an `emit_convert` sub-scope and a unique-vertex count,
  and stop if conversion is a minor share.
- **(B) Parallelize.** Convert large draws' vertices as a synchronous batch on
  the engine pool (`RunThreadPoolJobBatch`). Slot assignment, the reuse
  cache, the stream reserve and submission order stay serial. It is off by
  default, and the serial path remains the oracle and the rollback.

Oracle:

- a byte-comparing `-vkemitparallelverify` mode;
- the 14 material pixel families byte-identical between serial and parallel;
- TSan on the split conversion;
- seeded chunk-offset and shared-`maxBone` negative controls;
- an interleaved `frame_pacing.py` A/B in queue modes 0 and 2.

Implemented in an isolated worktree; results are recorded here when merged.
