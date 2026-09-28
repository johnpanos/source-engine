# Contract: `render.lights.v1`

Module: `render.pass.lights` (RFC 0016 layer 6, K7)
Headers: `public/render/pass/lights/` (`clusters.h`, `cluster_pass.h`)
Sources: `render/pass/lights/clusters.cpp` (the serial path),
`cluster_assign.comp` (the compute pass; the build generates its SPIR-V with
the pinned glslc into `spv/cluster_assign_spv.h`) and `cluster_pass.cpp` (`ClusterKernel`,
the upload and assign graph passes)
Suites: `unittests/rendertest/core/pass/lights/test_clusters.cpp` against the
independent reference in `cluster_oracle.h`, the bad builders in
`test_clusters_negative.cpp`, and the device lane `test_clusters_vulkan.cpp`
with the seeded kernels in `cluster_defects_spv.h`
Rows: R90 (RFC 0016 K7)

The frame's light set (`render.light-set.v1`, RFC 0011) is the only runtime
light list. Point and spot lights are clustered; directional lights are not
(shading applies them everywhere). A radius of 0 means unbounded.

| Clause | Obligation |
| --- | --- |
| L1 | `CreateClusterGrid` fails with its error for a zero viewport, a depth range not above 0 or with far not above near, a projection whose clip w is not the view distance or that offsets x or y, zero limits, and a grid beyond `maxFroxels`. A grid has `ceil(width / tile)` by `ceil(height / tile)` tiles and the profile's slices; slice k starts at `near * (far / near)^(k / slices)`; its corner rays and tile planes match those solved from the projection (pixel row 0 at clip +Y); `SliceOfDepth` maps each slice's depths to it. Mobile limits are no larger than desktop |
| L2 | Conservative assignment: over 1,000 seeded scenes (camera anywhere within 16,000 units, field of view 20 to 130 degrees, aspect 0.4 to 2.5, a quarter off-center, near 0.05 to 32, far up to 5,000 times near, 0 to 256 lights mixing points and spots with cones up to 175 degrees, placed in, straddling the near plane, behind and around the view, with ranges from 0.01 to four times far and unbounded), no light that reaches a froxel is missing from it. The reference decides reach exactly for points (distance to the froxel's hexahedron) and by an exact rejection plus a witness search for spots. The false-positive rates are recorded and stay under 0.02 (points) and 0.25 (spots) |
| L3 | At random points of each view, `FroxelAt` names a froxel that holds the point, and every light lighting the point (by a margin) is in that froxel's list |
| L4 | Lists: one range per froxel, contiguous in froxel order, covering the index list exactly; indices ascending within a froxel, each a point or spot light of the span |
| L5 | Capacity: beyond `maxLights`, point and spot lights leave the grid in light-set order; each froxel keeps the prefix of its list that fits under `maxLightsPerFroxel` and then the index list's remaining room in froxel order. `ClusterStats` counts exactly the lights over capacity, the froxels that lost a light and the pairs lost; `Overflowed()` says whether anything was. Under `OverflowPolicy::kFail` the build fails with `kOverflow` and the same counts and leaves the output unchanged |
| L6 | Directional lights and invalid lights (non-finite values, negative radius, a spot without a direction) are counted, never listed. A light wholly behind the eye or beyond far reaches nothing; an unbounded point reaches every froxel; an unbounded spot reaches its cone |
| L7 | `PackClusterLights` packs the lights `AssignLights` clusters, in its order, in view space (transformed in double), with -1 as a point light's cone cosine and a large finite radius for unbounded; `PackClusterGrid` lays the grid out as the compute pass reads it |
| L8 | Assignment is deterministic |

Sensitivity (`render.lights.clusters.sensitivity`): with the real builder
clean on the same scenes, each seeded defect is detected: slice boundaries
off by one, a range-squared sphere test, rows mirrored, columns off by one,
spot half-angles halved (false negatives) and doubled (false-positive
ceiling), lights nearer than the near plane culled, unbounded lights read as
reaching nothing, and capacity losses not counted (9 of 9).

The compute pass (`render.lights.clusters.gpu`, profile
`linux-native-vulkan-gpu`) runs `cluster_assign.comp` through `render.graph`
on `render.device.vulkan`. Its bindings are one dispatch-local group (the
draw role): parameters (uniform), packed lights, their light-set indices and
the packed grid (read-only storage), then the froxel ranges and the index
list with its counters (written storage). It writes light-set indices.

| Clause | Obligation |
| --- | --- |
| G1 | Over 300 seeded scenes (L2's generator, an empty scene and a 256-light scene), each froxel's list equals `AssignLights`' list, the same light-set indices ascending; only the offsets differ (scheduling order). A pair listed by one path only fails unless the serial path lists it for the light grown by 1e-4 of its radius and half-angle and not for it shrunk by 1e-4 (a boundary pair); boundary pairs stay under 1e-5 of the assignments. The ranges are disjoint and cover exactly the kept indices; an empty range's offset is unspecified. The requested count equals the serial assignments |
| G2 | The device's lists, compacted to froxel order, have zero false negatives against the independent reference (L2) and pass its lookup (L3) on 100 scenes |
| G3 | With a per-froxel limit of 1 to 8 lights, each list is the serial prefix, and `froxelsOverflowed` and `assignmentsDropped` equal `ClusterStats`' |
| G4 | With index capacity at a half to a fifth of the assignments, each list is a prefix of its per-froxel-limited serial list, the lists fill the capacity exactly, and the requested and dropped totals equal the serial path's (which froxels lose room depends on scheduling) |
| G5 | Seeded kernels are rejected: slice boundaries off by one and the spot cone ignored (G1 mismatches off the boundary), and capacity losses not counted (G3 counters) |

The kernel's arithmetic follows the serial path's order with every result
`precise` (no fused multiply-add the CPU does not do); square roots are the
only operations a device may round differently. On RADV (Strix Halo) the
lists were identical, with no boundary pair, over six seeds (1,800 scenes,
35 million assignments; 2026-09-28).

Open for K7: the pass as an `IRenderFeature` in the frame graph
(`feature.h`) with the lists in the families' view bind group; the dlight
behavior decision; product wiring (the light-set publisher feeding the
pass); a Fold7 and Apple device run; profile limits from desktop and Fold7
budgets.
