# P2:CE clustered lighting: feasibility and our performance work

- Date: 2026-10-01
- Scope: Research from P2:CE/Strata developer publications and official wiki
  sources, compared with this checkout and retained native measurements
- Requirement owner: [RFC 0016](0016-render-core.md#high-performance-clustered-lighting-user-decision-2026-10-01)
- Tracking: R90 (assignment/shadows), R95 (lab lighting), R96 (integration) and
  R91 (remaining frame cohorts); this note closes no implementation gate

## User direction

The user asked to record that high-performance clustered lighting is possible
and update every RFC: "We NEED high performance clustered lighting."
The subsequent assessment was "Our current performance is embarrasing".
The current performance is therefore rejected as the product target. This is a
requirement to resolve performance debt while preserving the complete intended
image; it does not change RFC 0016's prohibition on cutting effects to meet a
budget. The linked owner defines the implementation and evidence obligations.

The user then directed: "make a rule that if its faster on the GPU, we do it
on the GPU". RFC 0003 owns the binding
[CPU/GPU execution placement rule](0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01);
RFC 0016 rule 8 applies it to rendering. A proven faster GPU path must be used
for its supported workload, with preparation, transfers and synchronization
included in the comparison and the CPU implementation retained as an oracle.

The final direction is "Add render budgets too. And stick to them. We need a
minimum of 120FPS, high, on this machine." The user confirmed 1920×1080.
RFC 0016's [hard budget policy](0016-render-core.md#hard-render-budgets-user-decision-2026-10-01)
and its referenced budget row now govern acceptance. This explicitly supersedes
the earlier rule that all performance misses were nonblocking. Current source
inspection and retained measurements do not establish a pass.

## What P2:CE establishes

P2:CE's [May 2025 clustered announcement](https://store.steampowered.com/news/app/440000/view/541105873081074765)
describes GPU-compute lighting integrated into Source's forward shaders and the
replacement of CPU dlight lightmap updates/uploads with GPU evaluation. Its
[feature showcase](https://portal2communityedition.com/clustered) demonstrates
dynamic highlights and shadows on brushes, props, water, paint and gel.
This is concrete feasibility evidence for improving Source lighting and
removing an expensive legacy CPU path, not a matched numerical benchmark of
this engine.

The official [clustered introduction](https://github.com/StrataSource/Wiki/blob/main/docs/graphics/clustered/what_is.md)
documents a shared shadow atlas, cached shadow updates, point and spot lights,
and independent direct/indirect/specular choices. Indirect light is baked or
disabled; clustered assignment does not supply dynamic bounce. The current
[PBR documentation](https://github.com/StrataSource/Wiki/blob/main/docs/material/pbr/pbrshader.md)
also documents metal/roughness/AO textures, normal maps and cubemap reflections.
P2:CE supports PBR; comparing it as a renderer without PBR would be incorrect.

Its [volumetric documentation](https://github.com/StrataSource/Wiki/blob/main/docs/graphics/volumetrics/intro.md)
describes a froxel volume, shadowed lights, scattering and reprojection;
[cookies](https://github.com/StrataSource/Wiki/blob/main/docs/graphics/clustered/cookies.md)
work with both surface and volumetric lighting. The
[March 2026 developer changelog](https://steamcommunity.com/app/440000/allnews/)
reports increasing the shadow-face update budget from six to twelve; the wiki's
older six-face default is not a reliable current default. Its qualitative
volumetric-performance claim has no matched hardware/workload timings here.

The [known issues](https://github.com/StrataSource/Wiki/blob/main/docs/graphics/clustered/troubleshooting.md)
include atlas exhaustion, delayed/flickering shadow updates and incomplete
caster invalidation. Those tradeoffs are observations, not policies adopted
for our renderer. Our optimization must preserve accepted shadow behavior.

[Strata](https://stratasource.org/) provides no public engine source/SDK; its
[Engine repository](https://github.com/StrataSource/Engine) is an issue tracker.
The exact assignment kernel, grid, capacities, frame graph and comparative
costs are not established by those publications. The subsequent
[local RenderDoc investigation](0016-p2ce-renderdoc-investigation-2026-10-01.md)
verifies GPU BVH assignment, grid/list layouts, and native Canyon captures;
comparative costs remain unmeasured. The [DXVK Native project](https://github.com/StrataSource/dxvk-native)
lists P2:CE's Linux use; a graphics API or translation layer alone establishes
neither a performance winner nor parity with our native Vulkan core.

## Our observed implementation and measurements

- [Core composition](../render/composition/core_world.cpp) currently calls
  `AssignLights` and `AssignAreaLights` on the CPU and uploads their lists.
  [The GPU assignment pass](../render/pass/lights/cluster_pass.cpp) exists and
  has conformance coverage, but its existence does not prove product use.
- The core already shares lighting results between identical-view cohorts,
  conservatively masks area lights, and caches static shadow depth. Its moving
  shadow casters currently include occluder boxes; complete animated geometry,
  material coverage and nested-view acceptance remain open. P2:CE's caching is
  not a feature wholly absent from our implementation.
- Our shared surface program includes LTC rectangle lighting, GGX compensation,
  probes/reflections and volumetrics. P2:CE's public material does not document
  an equivalent LTC rectangle path. Richer lighting requires its own matched
  quality and cost evidence; it is not permission for poor performance.
- The [2026-10-01 native profiling record](0016-perf-forward-plus-2026-10-01.md#measurements)
  uses Radeon 8060S/RADV, an actual 1024x768 drawable and 4x MSAA on a shared
  host. Intro4 reverse records approximately 4.9–5.4 ms render-thread CPU and
  19.8–19.9 ms GPU render; the laser chamber records 5.219 ms CPU and 5.271 ms
  GPU render. Intro remains GPU bound. Missing core-only cohorts contribute to
  the CPU savings, so these are not accepted same-image whole-frame gains.
- The [active Cycles gallery review](0016-gallery-review-2026-10-01.md) still
  records 15/27 passing receiver views. R95, R96 and R91 remain open. Neither
  this note nor the external showcase certifies our images or performance.

## Work carried into the RFCs

All 18 RFCs link to the single
[performance requirement](0016-render-core.md#high-performance-clustered-lighting-user-decision-2026-10-01)
and identify their domain's responsibility. The immediate recorded work is
product CPU/GPU assignment comparison, view-result reuse, correct shadow-cache
invalidation, removal of duplicate runtime work for migrated surfaces, and
optimization of the full surface/area/volumetric cost. Full-scene image coverage
and paired timings are required for a performance claim. Numeric targets stay
in the existing profile/workload budget authority; no P2:CE FPS, universal
speedup, new benchmark command or platform acceptance result is invented.
