# Contract: `render.lights.v1`

Module: `render.pass.lights` (RFC 0016 K7). `clusters.cpp` owns grid creation,
packing and admission; `cluster_build.comp` constructs the Morton BVH;
`cluster_assign.comp` assigns lights and area masks; `cluster_pass.cpp` records
the GPU work. `test_clusters_vulkan.cpp` is the installed device suite.
`cluster_oracle.h` checks output against independent double-precision froxel
geometry and light reach. It has no light-list assignment implementation.

The frame's light set (`render.light-set.v1`, RFC 0011) is the only runtime
light list. Point and spot lights are clustered; directional lights are not.
Radius zero means unbounded. There is no CPU assignment or runtime fallback.

| Clause | Obligation |
| --- | --- |
| L1 | `CreateClusterGrid` rejects invalid viewport, depth, projection and limits. Its tile planes, corner rays and logarithmic slice depths match the projection; `FroxelAt` and `SliceOfDepth` name the containing froxel or clamp at a boundary. |
| L2 | Assignment is conservative: no light that reaches a froxel is absent. The independent geometry check uses exact point-to-froxel distance for points and exact rejection plus witness search for spots. Point and spot false-positive rates stay under 0.02 and 0.25. The full gate requires over 1,000 seeded GPU scenes; the installed 100-scene independent check is partial evidence. |
| L3 | At sampled points inside a view, the froxel lookup contains the point and every light lighting it by a margin. |
| L4 | Each froxel has one disjoint range in the index list. Indices are ascending light-set indices of admitted point or spot lights. |
| L5 | `maxLights`, `maxLightsPerFroxel` and `maxLightIndices` are explicit capacities. Per-froxel and global limits keep prefixes of the unlimited GPU lists. The GPU header counts requested pairs, dropped pairs and froxels that lost a pair. Unsupported product capacity fails explicitly. |
| L6 | Directional and invalid lights are counted and never listed. Lights wholly behind the eye or beyond far reach nothing; unbounded points reach every froxel; unbounded spots retain cone culling. |
| L7 | `PackClusterLights` packs admitted lights in light-set order in view space, using a finite sentinel for unbounded range; `PackClusterGrid` matches the compute pass layout. |
| L8 | Repeated GPU dispatches on identical immutable inputs return identical lists and counters. Resources stay alive through the last GPU completion token. |

The installed `render.lights.clusters.gpu` suite runs the compute pass through
`render.graph` on `render.device.vulkan`. It checks 300 seeded scenes for list
shape and deterministic output, 100 scenes against independent geometry,
per-froxel and global capacity prefixes and counters, direct consumer buffers,
1024 lights with equal Morton keys, 64 area-light masks and three seeded bad
kernels. The seeded slice and cone kernels must differ from the production GPU
kernel; the overflow kernel must fail the independent capacity accounting.
Where available, Vulkan synchronization validation reports no message.

`CONFORMANCE_CLUSTER_BENCH=1` adds diagnostic GPU upload/build/assignment and
host prepare/submit/wait timings. It contains no CPU assignment benchmark and
cannot certify the complete frame or High budget.
