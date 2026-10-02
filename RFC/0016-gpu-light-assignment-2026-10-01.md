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
