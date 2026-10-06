# Contract: `render.cull.v1`

Module: `render.culling` (RFC 0016 layer 5 mechanism, below the passes that consume it)
Header: `public/render/culling/cull.h`
Kernels: `render/culling/cull.comp` (frustum mask), `compact.comp` (bucketed
indirect commands), `hiz.comp` (depth pyramid) and `occlusion.comp`
Suites: `unittests/rendertest/core/culling/test_cull_vulkan.cpp`
(`render.cull`, `linux-native-vulkan-gpu`) and `render/lab/gpu_submission_suite.cpp`
(`render.lab.gpu-submission`)
Rows: R89 (RFC 0016
[GPU-driven submission](../../../RFC/0016-render-core.md#gpu-driven-submission-plan-2026-10-05-user-direction)
phases S3 and S4)

The oracle is `render::scene`'s CPU culler: `BuildDrawList` without a
visibility provider (`CullRange` and `math::Intersects( Frustum, Aabb )`).
RFC 0003's
[CPU/GPU placement rule](../../../RFC/0003-dependency-aware-job-system.md#cpugpu-execution-placement-user-decision-2026-10-01)
decides which of the two the product runs; the CPU culler stays the oracle
either way.

| Clause | Obligation |
| --- | --- |
| C1 | An instance is kept exactly when the view's bit is in its view mask (a view bit of 32 or more matches every instance), its world bounds are not empty, and for every frustum plane the box corner furthest along the plane's normal is not behind it (distance = ((n.x·x + n.y·y) + n.z·z) + d, unfused, kept at distance ≥ 0) |
| C2 | The output is one bit per instance in snapshot order: bit i % 32 of word i / 32, unused high bits zero |
| C3 | The GPU mask equals the oracle's bit for bit on seeded scenes (no tolerance: the kernel follows the oracle's order of operations and forbids contraction) |
| C4 | A seeded near-corner test, an ignored view mask and a kept empty box each disagree with the oracle |
| C5 | The pass is a compute pass that asks for the async compute queue; compiled with `CompileOptions::asyncCompute` it runs there and a later reader on graphics waits for it; compiled without, it runs on graphics with no wait |
| C6 | A device without `Capability::kCompute` fails kernel creation with `kNoCompute`; a dispatch the kernel cannot record counts in `RecordFailures` |
| C7 | Bind groups of recorded dispatches are released behind the token passed to `Collect` |
| C8 | Compaction (`compact.comp`, `CompactKernel`, `AddCompactPass`): instances are ordered by bucket (`DrawBucket { first, count }`, one per pipeline and its bindings) and each `DrawTemplate` names its bucket; word b of the output is bucket b's kept count and from `CommandsOffset( bucketCount )` bucket b's region at its instances' indices holds one `DrawIndexedIndirectCommand` per kept instance i, `{ indexCount, 1, firstIndex, vertexOffset, i }`, in instance order, equal to `CompactReference` exactly (1 to 64 buckets, some empty); zero instances write zero counts; at most `kMaxCompactInstances` instances |
| C9 | A seeded kernel that drops the last command from the count, and one that writes the wrong first instance, each disagree with the reference |
| C10 | On a device claiming `kDrawIndirectCount`, one `DrawIndexedIndirectCount` per bucket over the compacted buffer draws an image equal, byte for byte, to direct `DrawIndexed` calls for the CPU culler's kept instances in instance order |
| C11 | Occlusion (`hiz.comp`, `occlusion.comp`, `OcclusionKernels`, `AddOcclusionPass`): the depth pyramid's levels equal `DepthPyramidReference` of its level 0 exactly (max of up to 2x2, sizes rounding up); an instance kept by the frustum mask is removed exactly when all eight corners of its box are in front of the camera and past the near plane and its nearest depth is farther than the pyramid's farthest over its screen rectangle at the first level spanning at most 2x2 texels; the GPU mask agrees with `OcclusionReference` on the read-back pyramid (at least 99.5 %) |
| C12 | Conservative: drawing an occluder and only the occlusion-kept boxes gives the image drawing every frustum-kept box gives, byte for byte, while a real share is removed; a kernel testing the farthest corner, or one texel of level 0, changes an image |
| C13 | The world pass (`WorldTarget::gpuSubmission`, `r_core_world_gpu_submit`) culls a view's surfaces per surface, compacts them per (material, lightmap page) bucket and draws each bucket with one `DrawIndexedIndirectCount` from the world's shared buffers, giving the per-surface path's image pixel for pixel (`render.lab.gpu-submission`) |

The placement comparison (CPU serial and pooled against the GPU round trip
and resident instances) is printed, never judged here; its decision and
numbers live in RFC 0003's progress record.
