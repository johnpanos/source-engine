# P2:CE clustered rendering: local RenderDoc investigation

Date: 2026-10-01. Research evidence for R90/R95/R96/R91 under
[RFC 0016](0016-render-core.md), not an implementation or performance gate pass.
This extends the [public-source assessment](0016-p2ce-clustered-lighting-2026-10-01.md).
No engine implementation was changed by this investigation.

## Findings

P2:CE implements GPU light assignment using a Morton-sorted, 32-way light
bounding-volume hierarchy (BVH), followed by one 32-thread workgroup per
cluster. Surface shaders consume cluster offset/count records and loop over
the selected light indices. The captured grid is 16 × 8 × 24, with logarithmic
depth slices. It is a forward renderer, with separate shadow and volumetric
work, rather than a deferred full-screen light accumulation pass.

The user's native P2:CE example, **[Canyon](https://steamcommunity.com/sharedfiles/filedetails/?id=3804485837)**,
is a useful complete-scene reference, but **all five user captures have zero
local clustered-light assignments**. Clustered lighting is enabled, and the
assignment shader executes, but every cluster's count is zero. Its appearance
in these views cannot be used as evidence for the cost of shading many local
clustered lights. Its baked HDR lightmaps, cubemap reflections, cascaded
sunlight, and volumetric fog are separate, real rendering workloads.

A separate synthetic light test on `sp_a2_laser_intro` verifies that the same
assignment path produces nonempty lists: 43 active light indices, two view
contexts, and up to 43 lights in a cluster. This is a mechanism test, not the
native-map quality reference requested by the user.

## Build and evidence provenance

- Installed Steam app 440000, unstable branch, Steam build **25033687**.
- Engine reports **0.0.8.2**, build **Aug 31 2026 09:26:37 (10159)**.
- Native Linux executable and D3D11 backend, translated by **DXVK Native
  v2.6.2-12-g6dc71aad** to Vulkan.
- AMD Radeon **8060S**, Mesa **RADV 26.2.3**; captured drawable **1280 × 720**.
- RenderDoc **1.45**, using the installed `renderdoccmd` and qrenderdoc Python
  API. Shader interpretation uses the captured SPIR-V, RenderDoc disassembly,
  and locally built SPIRV-Cross revision
  `aa217aeb6c9f0ace7a0ab233b28807edf45eb165` after `spirv-opt -O`.
- There is no access to P2:CE's private engine source. Shader-stage names below
  are semantic identifications from their instructions/resource flow and the
  shader names present in the installed material-system binary. They are not
  source-level debug symbols from the capture.

Local evidence is under
[`quality-results/p2ce-clustered-20261001/`](../quality-results/p2ce-clustered-20261001/).
`manifest.json` records capture hashes and provenance. Captures/disassembly
remain local, outside tracked source. The five user-created Canyon `.rdc`
files remain at their original installation paths. Their summaries and
thumbnails are in `canyon/user-captures/`.

The checkout was dirty and other sessions were editing it during this work.
Statements about this engine are observations of the inspected source, not a
certification of a fixed clean revision or another session's changes.

## Native-map examination: Canyon

The installed Workshop manifest is **5140758830552706151**. Its package contains
`maps/canyon_final.bsp`, BSP version **25**. The addon describes itself as made
on the unstable branch. The conflicting local Dynamic Light Bridge Lights
addon was already disabled. No synthetic lights or global fog override were
added to Canyon. Runtime queries recorded:

```text
r_clustered_lighting_enable = 1
r_volumetrics_enable = 1
r_volumetrics_default_density = 0
sv_enable_dynamic_legacy_lights = 0
```

The BSP entity lump contains 268 entities, including 37 `light`, eight
`light_spot`, two `light_dynamic`, three `obb_volumefog`, one
`light_environment`, and one `env_cascade_light`. All 45 `light`/`light_spot`
records explicitly set `_directmode=1` and `_indirectmode=1`. The installed
FGD defines both values as **Static Only**. Entity-lump counts alone are not
runtime light counts; the GPU lists provide the decisive runtime evidence.

The HDR lighting lump is 2,239,772 bytes. The BSP embeds material patches that
reference PBR material paths and map-specific environment cubemaps (76 patches
match the PBR-path inspection heuristic; that is not a census of all materials
rendered). The authored fog volumes have densities 0.007, 0.002, and 0.007.

Each user capture was opened and replayed, and the output grid was read back
immediately after its assignment dispatch:

| User frame | Draw calls | Compute dispatches | Cull event | Clusters | Nonempty clusters |
| --- | ---: | ---: | ---: | ---: | ---: |
| 3657 | 918 | 8 | 2188 | 3072 | 0 |
| 4652 | 922 | 8 | 3099 | 3072 | 0 |
| 5049 | 851 | 8 | 3061 | 3072 | 0 |
| 5553 | 871 | 8 | 2255 | 3072 | 0 |
| 6456 | 1371 | 20 | 3221 | 3072 | 0 |

All five cull constant blocks decode as unsigned words `[0, 0, 1, 0]`:
zero light/view records, zero BVH levels, one view context, context index zero.
The actual grid readbacks independently confirm all 3072 counts are zero.
Consequently, absence of BVH-build dispatches in these Canyon frames is **not
proof of a static BVH cache**: there are no active local lights to build it for.

An additional assistant capture, frame **6489** (`canyon/frame.rdc`), verifies
surface-resource use in the bridge/water view:

- E4529 builds cluster AABBs; E4533 assigns empty local-light lists.
- E4537/E4541/E4545 execute the volumetric sequence.
- Surface draw E4577 binds the same cluster grid/list buffers, a **128²
  BC6_UFLOAT cubemap**, an **RGBA16F 2048-wide texture**, the integrated fog
  volume, the local-light shadow atlas, and a separate sun shadow texture.
  The cubemap binding agrees with the map-specific `$envmap` patches.
- The **2048² D32S8** sun-shadow resource (#938) is cleared and used as a
  depth/stencil target by **622 draws**, and is read by surface and compute
  shaders. The **8192² D32S8** local-light shadow atlas (#934) has shader
  reads but **no depth-target draws** in this frame.

This distinguishes real cascaded sunlight/shadow work from empty local-light
assignment. It does not establish that every bound texture contributes to
every pixel. The captured camera positions differ; these are not paired image
or performance comparisons.

## GPU assignment path

The synthetic 43-light frame (`multilight/frame.rdc`, frame 1015) exercises the
stages that the zero-light Canyon views omit:

| Event | Identified work | Dispatch groups | Threads per group |
| --- | --- | --- | --- |
| 47 | Transform light data into view contexts | 1 × 1 × 1 | 64 |
| 51, 55 | Reduce light bounds, two passes | 1 × 1 × 1 each | 512 |
| 59 | Compute Morton codes | 1 × 1 × 1 | 1024 |
| 63 | Radix sort codes and light indices | 1 × 1 × 1 | 256 |
| 67, 71 | Build lower and upper light-BVH levels | 1 × 1 × 1 each | 512 |
| 74, 76 | Build each view's cluster AABBs | 6 × 1 × 1 each | 512 |
| 80, 82 | Traverse BVH and assign each view's lights | 3072 × 1 × 1 each | 32 |

The view-data pass receives 43 lights and two contexts; the subsequent BVH
input contains **86 light/view records**. Assignment uniforms are
`[86, 2, 2, 0]` and `[86, 2, 2, 1]`. The shader selects the current context
using record-index modulo context count and recovers the original light index
by division. The capture proves two contexts, but this investigation does not
identify which engine view owns each context.

Morton generation interleaves ten bits per spatial coordinate into a 30-bit
code. The BVH shader reduces groups of 32 leaf bounds. Its level-offset tables
contain 1, 33, 1057, 33825, etc.; traversal assigns the 32 lanes to a node's
children. Culling includes distinct point/sphere and spotlight-cone tests,
with a bounding sphere derived from the cluster AABB. It does not simply
scan the complete light table independently in every cluster.

The grid kernel establishes `x = id / 192`, `y = (id % 192) / 24`, and
`z = id % 24`: **16 columns × 8 rows × 24 slices**. The fragment shader uses
the corresponding `x*192 + y*24 + z` index and a logarithmic depth mapping.
The initial captured depth endpoints are approximately 5.916 and 9481.24;
these are captured view parameters, not claimed universal engine constants.

Each cluster has a 24-byte AABB and an 8-byte `(offset,count)` record.
The culling workgroup has a 1024-word shared traversal array and a 2048-word
shared accepted-light array. Its output count is clamped to 2048, with fixed
offset `clusterIndex << 11`. The live output buffers verify:

- AABBs: **73,728 bytes** per view.
- Offset/count grid: **24,576 bytes** per view.
- Light indices: **25,165,824 bytes (24 MiB)** per view, even for sparse lists.
- Captured light-table descriptor: **917,504 bytes**, with a 112-byte shader
  stride (storage for 8192 records; not a proof of the product's accepted
  scene-light limit).

The 43-light test produced **43,522 assignments** for context 0 and **22,546**
for context 1. Both lists include indices 0 through 42 and have a maximum
cluster count of 43. Every one of the 3072 offsets in each view equals
`clusterIndex*2048`. This verifies the layout with actual data, rather than
only allocating a large buffer and assuming its meaning.

The fixed ranges avoid a global variable-length output allocator but reserve
substantial space per view. The kernel's capacity clamp is visible; its full
overflow behavior, any engine diagnostics, and worst-case ordering have not
been exercised. These limits and policies are observations, not a proposed
replacement for this engine's explicit overflow contract.

## Surface, shadow, and volumetric work

In the initial legacy-map capture, surface draws E531 and E643 read the exact
buffers written by assignment E371. A surface shader computes its cluster,
loads the count and offset, fetches a 32-bit light index, and addresses that
light's 28-word record. Its local-light loop contains attenuation, light-mode
branches, shadow/cookie sampling, and material response. Different material
shaders consume this common data rather than requiring a per-light world draw.

The installed runtime reports `r_clustered_shadowframebudget=12`, depth bias
0.000025, slope scale 2.0, and `r_clustered_static_lightcull=1`. These are
observed session values, not all asserted factory defaults. The local-light
atlas is **8192² D32S8**. The official documentation separately describes
cached shadow updates and the six faces needed by a shadowed point light;
cache invalidation correctness was not tested here.

The captured fog resources are **128 × 128 × 384 RGBA16F**: 48 MiB of texels
per volume before driver overhead. The synthetic frame exposes voxelization,
temporal reprojection, and depth propagation kernels, with 8 × 8 × 4 threads
for volume work and 16 × 16 × 1 for column propagation. The uniform block
contains the observed history weight **0.95**. These resources are distinct
from the 16 × 8 × 24 surface-light grid. Do not substitute the older wiki's
quality-level dimensions for the actual captured allocation.

As a negative control, setting `r_clustered_lighting_enable=0` in the synthetic
scene removed the BVH/cluster/volumetric dispatches: **22 compute dispatches
became three**, while both captures retained **268 draw calls**. The
remaining three are 8 × 8 image operations, not cluster assignment. The
camera moved between captures, so this control validates pass removal only.
The setting was restored to 1 afterwards. No such effect-disable control was
applied to the user's Canyon captures.

## Implications for this engine

At inspection time, `render/composition/core_world.cpp` calls CPU
`AssignLights` and `AssignAreaLights`. The installed GPU assignment kernel in
`render/pass/lights/cluster_assign.comp` scans every packed light per cluster.
The concrete P2:CE techniques worth evaluating are GPU view preparation,
spatial sorting/BVH traversal, cooperative cluster assignment, and avoiding
duplicated work between consumers of the same view. A sparse packed output
may be preferable to P2:CE's 24 MiB-per-view fixed allocation; that is a
measurement and contract decision, not established by this capture.

Keep two reference workloads separate: Canyon for a representative complete
PBR/fog/water/foliage/sun-shadow scene, and a map with verified nonempty native
clustered-light lists for local-light scaling. Canyon's sampled empty lists
do not diminish its image quality, but they cannot justify a many-light
performance comparison. Baked diffuse/GI and cubemap reflections must also be
distinguished from runtime indirect-light producers and area-light integration.

No FPS, GPU-time speedup, 1080p/high budget pass, dense-light scaling curve,
LTC area-light equivalence, or platform parity is claimed. The launch used
`fps_max 30` for investigation, not benchmarking. RenderDoc replay and a
1280×720 interactive session are not substitutes for an uncaptured, matched
1920×1080/high product benchmark under the current RFC 0016 budget policy.

## Reproduce and inspect

The existing `tools/renderdoc/rdc.py` replay tool was used; no new installed
runner is claimed. Example from the repository root:

```sh
python3 tools/renderdoc/rdc.py script \
  '/home/john/.local/share/Steam/steamapps/common/Portal 2 Community Edition/chaos_frame6456.rdc' \
  quality-results/p2ce-clustered-20261001/scripts/summarize-canyon.py --json
```

`canyon/user-captures/*.json` contains the five independent grid readbacks.
`canyon/surface.json` records surface bindings and shadow-resource usage.
`canyon/authored-lighting.json` records parsed authored entities;
`canyon/embedded-pbr-materials.json` records the inspected material references.
`multilight/checks.json` records actual populated-list counts and indices.
The `scripts/` directory retains the capture/control/query scripts, and
`inspect.json` files retain events, dispatch dimensions, resources, and
constant blocks. The JSON field `storage` includes both read-only and writable
Vulkan storage-buffer descriptors; it must not be interpreted as a list of
writes. DXVK pools large buffers, so descriptor offsets/ranges, not total
Vulkan buffer allocation sizes, identify logical resources.

The game logs contain existing missing-particle/dev-material and other
warnings. Captures nevertheless replayed with visible native-map content;
that is not certification of P2:CE's installation or all map behaviors.
This documentation-only investigation was checked through capture replay,
buffer invariants, the negative control, and local reference validation; no
unrelated engine build or architecture baseline was changed.
