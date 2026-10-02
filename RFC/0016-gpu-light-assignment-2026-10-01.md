# GPU light assignment restoration, 2026-10-01

R90/R95/R96/R91 remain partial. The user selected GPU-only runtime assignment
and an 8192-square default shadow atlas. The atlas default is quality 3 in both
the engine control and core composition; an archived explicit setting can still
override the default.

## Implementation

`render/pass/lights/cluster_build.comp` builds Morton keys, sorts them on the GPU
and builds a 32-way light BVH. The bounded 1024-light implementation uses bitonic
sorting, rather than claiming to reproduce P2:CE's sorter. Assignment uses one
32-thread workgroup per froxel, traverses the BVH, preserves ascending light-set
indices and generates conservative area-light masks on the GPU. The existing
view-dependent grid policy is retained; this is not a copy of P2:CE's fixed
16 × 8 × 24 grid.

Core composition and the clustered, shadowed and area-light lab consumers bind
these GPU lists directly. Surface lists reserve enough space for every admitted
light, avoiding the previous per-cluster truncation. Unsupported capacity fails
explicitly. Same-view cohorts share the resulting buffers; their lifetime ends
behind the last consumer's completion token. Runtime CPU assignment and area-mask
construction are removed. The test-only CPU assignment reference was later
deleted by user direction; independent geometry checks still verify GPU output.

Shadow planning uses conservative whole-view light visibility without a GPU
readback. This can admit more shadow candidates than fine cluster coverage, so
assignment timings alone do not establish a complete-frame improvement.

## Verification and reproduction

The isolated native Vulkan suite passed **28 checks** on AMD Radeon 8060S
(RADV STRIX_HALO), including 300 randomized equivalence scenes, 100 independent
coverage scenes, bounded-overflow cases, seeded fault controls, direct consumer
buffers, 1024 lights with duplicate Morton keys and 64 area lights. CPU suites
passed 249 oracle checks and 10 sensitivity checks. Style fixtures passed 38 tests.

Evidence: `quality-results/conformance.20261001T235302Z.json` and its sibling
`.logs/render.lights.clusters.gpu/run.0.log`. The run was made from isolated base
`45ae4289b896c16bca9baa656695d0c3bffa724f` with the restored implementation;
the evidence records its dirty digest. Results were copied into the main
checkout's ignored `quality-results` directory. Reproduce with the installed
`tools/quality/conformance.py check` runner selecting `render.lights.clusters.gpu`
and setting `CONFORMANCE_CLUSTER_BENCH=1` for the diagnostic samples.

Eight measured samples per case after warm-up, at 4590 froxels (milliseconds):

| Lights | CPU oracle | GPU upload + build + assign | Host prepare + submit + wait |
| --- | ---: | ---: | ---: |
| 0 | 0.012 | 0.097 | 0.440 |
| 43 | 0.323 | 0.160 | 0.536 |
| 256 | 0.623 | 0.256 | 0.571 |
| 1024 | 1.903 | 0.540 | 1.046 |

These are diagnostic assignment costs, not a matched P2:CE comparison or the
complete product frame. Small scenes do not show a full-cost GPU win. Native
product/lab source compilation progressed, but final product links were blocked
by concurrent VTF ABI changes; shared shader regeneration also encountered
concurrent probe-shader changes. Full architecture checking reports two unrelated
ARCH105 failures in `game/shared/fstop/blob_networkbypass.{cpp,h}`. Complete-image,
product integration and hard render-budget acceptance remain unverified.

## Recovery

The GPU kernels and consumer integration survived the shared-checkout overwrite.
The observed loss was in `clusters.cpp`: removed CPU area helpers reappeared and
packed-light admission statistics were discarded. Those changes were restored
from the isolated validated copy. Other active chats were notified before commit
to prevent a broad restore from reinstating the obsolete runtime assignment.

## Current-checkout performance rerun (2026-10-01)

After the Portal 2 launcher build repair, the documented
`CONFORMANCE_CLUSTER_BENCH=1` run passed all 28 GPU checks on the Radeon 8060S.
The headless oracle and sensitivity suites passed 249 and 10 checks. The timed
GPU run had no `hl2_launcher` process active. Its evidence and full sample log
are in [`play-p2-gpu-bvh-bench-clean.json`](../quality-results/play-p2-gpu-bvh-bench-clean.json).
Eight measured samples per case followed warm-up at 4590 froxels (milliseconds):

| Lights | CPU oracle | GPU upload + build + assign | Host prepare + submit + wait |
| --- | ---: | ---: | ---: |
| 0 | 0.008 | 0.051 | 0.360 |
| 43 | 0.278 | 0.074 | 0.309 |
| 256 | 0.571 | 0.094 | 0.352 |
| 1024 | 1.698 | 0.203 | 0.552 |

These remain assignment diagnostics, not a complete-frame or hard-budget result.

## GPU-only retirement and game profile (2026-10-01)

By user direction, the test-only CPU `AssignLights` reference and its two
conformance suites were deleted. The dead CPU area-mask builder was also
removed. The installed `render.lights.clusters.gpu` suite compares repeated GPU
dispatches, uses independent double-precision geometric reach checks, checks
capacity against the actual discarded pairs, and rejects three faulty kernels.
`CONFORMANCE_CLUSTER_BENCH=1` now times only GPU upload/build/assignment and host
prepare/submit/wait. The CPU timings above are historical observations from
before this retirement; their implementation and run commands no longer exist.
The current GPU suite passed 26 checks (27 with timing enabled): 300 repeatability
scenes had zero mismatches, 100 independent geometry scenes had zero false
negatives over 1,934,795 reached pairs, and all three seeded defects were
detected. The four GPU upload/build/assignment medians for 0, 43, 256 and 1024
lights were 0.041, 0.063, 0.087 and 0.160 ms at 4590 froxels, with eight samples
per case. The [GPU-only conformance evidence](../quality-results/p2-gpu-only-clusters-bench-final.json)
and its [sample log](../quality-results/p2-gpu-only-clusters-bench-final.logs/render.lights.clusters.gpu/run.0.log)
record the run.

The `portal2-frame-floor-v1` route was run through `./play_p2` on the Radeon
8060S/RADV at the actual 1920 × 1080 back buffer, with the High profile's 16
settings verified, including 4× MSAA. The uncapped baseline measured 845 frames:
16.484 ms median, 111.222 ms p99, 40.7 average FPS, 8.5 FPS 1% low. Arrival
and return medians were 16.19 and 16.56 ms; the reverse view's median was
55.75 ms. The hard 120 FPS frame floor (8.333 ms) therefore fails. The
[baseline evidence](../quality-results/p2-gpu-bvh-game-profile/high-baseline-evidence.json)
and [frame stream](../quality-results/p2-gpu-bvh-game-profile/high-baseline-frames.jsonl)
retain the exact settings and timings.

`-vkgputimers` labeled 854 route frames. The three back-buffer draw segments
averaged 15.14 ms per arrival frame, 51.02 ms per reverse frame and 15.37 ms
per return frame, over 99% of the summed labeled GPU time. Upload/compute start,
MSAA resolve and present were each below 0.05 ms on average. The named backend
CPU costs averaged 3.71 ms `mesh_draw`, 1.27 ms command recording and 0.32 ms
texture upload. The [GPU-segment frame stream](../quality-results/p2-gpu-bvh-game-profile/gpu-segment-frames.jsonl)
records these measurements.

The render core's debug timers initially dropped labels in the reverse view;
its diagnostic capacity was raised from 512 to 4096 timestamps per frame and
the route was rerun with `cl_render_debug_gpu_timers` and
`cl_render_debug_stats`. In stable one-second windows, weighted by frames:

| Cost per frame | Arrival | Reverse | Return |
| --- | ---: | ---: | ---: |
| Core world view GPU (all views) | 15.13 ms, 15 views | 55.38 ms, 47 views | 15.83 ms, 15 views |
| Core world GPU (within those views) | 11.65 ms | 53.91 ms | 12.21 ms |
| Cluster BVH assignment GPU | 0.37 ms, 2 uses | 0.40 ms, 2 uses | 0.39 ms, 2 uses |
| Core world CPU recording | 6.70 ms | 41.92 ms | 7.51 ms |
| Shadow-depth GPU | 0.08 ms | 1.13 ms | 0.10 ms |

The reverse view also reported about 155 shadow tiles drawn per frame versus
13 in arrival. The large cost increase tracks repeated core world views and
their recording, with more shadow work. GPU BVH assignment is a small fraction
of the frame. The diagnostic timers add overhead, so the untimed baseline owns
the performance result; the timed run identifies where it is spent. See the
[complete pass log](../quality-results/p2-gpu-bvh-game-profile/core-pass-stdout.log)
and [diagnostic evidence](../quality-results/p2-gpu-bvh-game-profile/core-pass-evidence.json).

The scripted view checks and `QA_DONE checks=4 failures=0` appear in the captured
stdout, while the frame-floor runner's separate `console.log` reader reported
those route records missing. The performance miss is independently visible in
the frame stream. The runner's formal route acceptance remains failed pending
that log-source fix; this profile does not promote the hard budget.
