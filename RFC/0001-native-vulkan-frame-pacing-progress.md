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

## Emit conversion on the engine pool (R32-EMIT-PARALLEL, 2026-09-25)

Question: how much of `emit` is the per-vertex conversion, how much of that
sits in draws large enough to split, and does splitting them across the
engine's compute pool (RFC 0003's synchronous batch) pay.

### Measurement

`-vkframestats` now has an `emit_convert` cost kind (inside `emit`: the
conversion of a draw's unique vertices, timed in nanoseconds) and a
`convert` record per frame: skinned vertices, pooled draws, and per
draw-size bucket `[draws, unique vertices, us]` (`vulkan_frame_stats.h`
`kEmitConvertBucketMin`). `frame_pacing.py` summarizes them
(`emit_convert_median_ms`, `summary.convert`), and `--ab-extra-arg` gives
the B runs of an A/B an engine argument, so one build can be compared with
and without a switch.

To time the conversion alone, `EmitToNativeQueue`'s `convertDraw` is split:
a serial pass assigns each unique vertex its slot in first-use order and
writes the indices (as before); a second pass converts the slots. A record
depends only on its vertex and the draw's state, so this changes no output:
all 14 `material_pixel_conformance` families (hdr none) give byte-identical
captures with the HEAD backend and the split one, and an interleaved A/B
(4 rounds, mode 0) puts the split build at 0.97x HEAD's median (noise).

Portal scenario, warm pass, RADV Strix Halo, load 5 to 9:

| | mode 0 | mode 2 |
| --- | --- | --- |
| emit median | 3.05 ms | 3.61 ms |
| emit_convert median | 2.47 ms (81 % of emit) | 2.96 ms (82 %) |
| converted draws / unique vertices per frame | 161 / 54,400 | 168 / 62,400 |
| skinned share of vertices | 56 % | 57 % |

Conversion time by unique vertices per draw (mode 0; mode 2 within 3
points):

| Unique vertices | Draws/frame | Vertices/frame | Share of convert time | ns/vertex |
| --- | --- | --- | --- | --- |
| 1-63 | 122.5 | 1,207 | 1.0 % | 18 |
| 64-255 | 15.3 | 1,985 | 1.6 % | 18 |
| 256-1023 | 7.3 | 5,671 | 14.8 % | 58 |
| 1024-2047 | 11.3 | 15,118 | 15.8 % | 23 |
| 2048-4095 | 1.7 | 3,847 | 7.5 % | 43 |
| 4096-8191 | 2.5 | 14,779 | 31.9 % | 48 |
| 8192+ | 0.75 | 11,789 | 27.4 % | 51 |

Draws of 2048+ unique vertices are 5 per frame but 67 % of the conversion
time (69 % in mode 2); 1024+ is 83 %.

### Design

- `vulkan_emit_convert.h` (`CEmitConvertBatch`): the slots split into chunks
  of 128; chunk c converts only its slots' records and writes only its own
  bone maximum; the caller reduces the maxima after the batch returns. The
  batch's fork publishes the inputs and its join the records
  (`parallel_batch.h`); no chunk reads another's output.
- `convertPooled` runs it through `RunThreadPoolJobBatch` on `g_pThreadPool`
  (no new threads) for stream draws with at least
  `mat_vk_emit_parallel_min_vertices` (1024) unique vertices, when
  `mat_vk_emit_parallel` is 1. Default 0: the serial pass is the oracle and
  the rollback. Slot assignment, indices, the reuse cache and the stream
  reserve stay serial, in their old order.
- Legality under R20's wait rules: the bridge queues only this call's
  runners, the caller claims chunks itself, and the join cancels unstarted
  runners and waits only for running ones; it never runs unrelated pool work
  and never `YieldWait`s. The caller is the main thread (mode 0) or
  `MatQueue` (mode 2), neither a `CmpJob` worker, so no nested pool wait
  arises; from inside a batch the call runs serially (`BatchDepth`). No
  forbidden-wait or starvation warning appeared in any run.
- `-vkemitparallel`, `-vkemitparallelmin` override the ConVars from the
  first frame (the material system applies console sets queued, which the
  pixel harness never does); `-vkemitparallelchunk n` shrinks chunks for
  oracles; `-vkemitparallelfault offset|sharedbone` seeds defects;
  `material_pixel_conformance -threadpool N` starts the pool in the harness.

### Oracles

| Check | Result |
| --- | --- |
| `-vkemitparallelverify` (every pooled draw also converted serially and privately; records and bone maximum byte-compared), portal scenario, 3 passes | mode 0: 11,000+ checked, 0 mismatched; mode 2: 10,000+ checked, 0 mismatched, 0 cross-thread calls, 1,933 render-thread presents |
| same with `-vkemitparallelfault offset` | mode 0: 3,311 of 3,311 mismatched; mode 2: 3,372 of 3,372 |
| 14 pixel families, hdr none: pooled (`-threadpool 3 -vkemitparallel 1 -vkemitparallelmin 1 -vkemitparallelchunk 1`, so every draw is split into one-vertex chunks) vs serial | all 14 captures byte-identical (75 listed cases; exposure and pbr-fallback compared whole, less `poll_frames`); every pooled draw verified, 0 mismatched |
| same with the offset fault (lightmap, skinning, modellight, portal) | all 4 captures differ, the oracle fails all 4, verify mismatches every draw |
| `render.vulkan.emit-convert-batch` (new, CPU only): chunked vs serial conversion over 13 draw sizes, 5 chunk sizes, serial and 1/2/32 real worker threads | 269 checks pass on g++ and clang++, default and `-O2 -DNDEBUG`; the comparison catches a chunk offset (5 chunk sizes) and a wrong bone maximum |
| `.tsan` (clang, `-fsanitize=thread`) | pass, no report |
| `.tsan.sensitivity` (`-DEMIT_CONVERT_SEED_RACE`: every chunk read-modify-writes one shared bone maximum) | fails as required (exit 66, race reported at the header's shared write). A first version that wrote the shared value only when raising it was not reported in 8 runs: once the palette maximum is reached every chunk only reads |

The engine-pool bridge itself is not run under TSan here; its fixtures
(`jobsystemthreadpooltest`, the engine-bridge tests) are the existing
evidence. The product has not run under TSan with the switch on.

### Performance

Interleaved A/B on one build, B with `-vkemitparallel 1`, warm pass medians
per round (ms):

| Run | A median | B median | B/A median, emit, emit_convert, p99 |
| --- | --- | --- | --- |
| mode 0, min 2048, 5 rounds | 5.54 6.59 5.23 6.30 6.49 | 4.18 4.14 4.52 4.45 5.47 | 0.71, 0.57, 0.48, 0.66 |
| mode 2, min 2048, 5 rounds | 5.04 4.40 4.57 4.58 4.80 | 3.08 3.22 3.22 3.30 3.20 | 0.70, 0.63, 0.55, 0.68 |
| mode 0, min 1024 (default), 4 rounds | 7.76 7.88 8.29 8.29 | 5.94 6.12 6.53 6.23 | 0.78, 0.59, 0.49, 0.81 |
| mode 2, min 1024, 4 rounds | 5.35 4.98 4.97 4.55 | 4.43 3.47 3.49 3.12 | 0.70, 0.60, 0.51, 0.73 |

B is faster in every round of every run; per-round emit falls from 3.3-4.0
to 2.0-3.0 ms. The host's speed drifted between runs (A at 5.5 then 7.8 ms
in mode 0), so compare within a run. Threshold (mode 2, both pooled): 1024
against 2048 is 0.93x median and 0.88x emit, better in all 4 rounds; 512
against 1024 is 0.97x, better in 2 of 4 (not adopted).

With pooling on, the 256-1023 bucket (skinned models at about 100 ns a
vertex, still serial) is the largest remaining share of conversion (31 %).

### Not done

- Default stays 0. The gain is clear on this desktop; making it the default
  is the user's decision.
- Android (Fold7) and Apple are unmeasured: pool size, big/little cores,
  power and thermal cost of three more busy workers, and whether the
  compute pool is contended there (the Android main pins
  `mat_queue_mode 0`). Both paths call the same `convertSlots`, but the
  compiler may inline it at each call site; on AArch64 (FMA) a run with
  `-vkemitparallelverify` is the check that floating-point contraction does
  not differ between the copies. x86-64 here has no FMA in these builds.
- Total CPU (all workers) was not measured; only the presenting thread's.
- No low-core profile run (the pool had 3 `CmpJob` workers).
- The 256-1023 skinned draws: GPU skinning (`skin.vert`) remains the
  structural fix for skinned conversion cost, and would shrink what the pool
  has to do.

**Desktop default and merge (2026-09-25).**

- The worktree branch (`30068dd2`) was merged into the shared tree without a
  commit. There, `render.vulkan.emit-convert-batch` passes (269 checks, g++
  and clang++), the TSan lane is clean, its race control fails as required,
  and `build.portal-native`/`-clang`, `arch.check` and `arch.compile-deps`
  pass.
- Taking the recommended option under the user's standing instruction, the
  desktop launchers now pass `-vkemitparallel 1`: `run.conf`'s `JOB_ARGS`,
  and `play_p2`. That is the exact switch the in-game A/B measured. The
  engine ConVar default stays 0, which is the rollback (`JOB_ARGS=` or
  dropping the flag). The Android launcher is unchanged until Fold7
  measurements exist.

