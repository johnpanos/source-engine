# Contract: `world.meshlet_cull` v1

Owner: [RFC 0008](../../../RFC/0008-canonical-world-data-and-runtime-formats.md), phase F4.
Interface: [`engine/worldmesh_cull.h`](../../../engine/worldmesh_cull.h).
Shared suite: [`test_worldmesh_cull.cpp`](../test_worldmesh_cull.cpp) (`world.meshlet-cull`).

The engine skips WMSH meshlets the GPU would draw no pixels of. Both tests are
conservative: they may keep a meshlet a view could skip, never the reverse.

## `ConeFacesAway`

Inputs are a WMSH v2 meshlet's bounding sphere and front-face cone (every
unit `cross(b - a, c - a)` of its non-degenerate triangles has
`dot(n, axis) >= cutoff`) and the eye.

1. It returns true only if, for every such normal and every point `p` of the
   sphere, `dot(n, p - eye) > margin * |center - eye|`: every triangle faces
   away from the eye, and none is near edge-on.
2. A cone of `cutoff <= 0` or an eye inside the sphere never culls.

The caller uses it only for materials with back-face culling, in perspective,
unmirrored views with the default cull mode.

## `OcclusionBuffer`

1. `AddOccluder` receives only triangles the GPU draws opaque, depth-tested
   and depth-writing in the view. One-sided triangles occlude only while
   `dot(cross(b - a, c - a), eye - a)` is positive (with a sine margin).
   Only the part with `zNear <= w <= zFar` occludes.
2. A cell records a depth only when the clipped projection covers the whole
   cell with a margin, and records the farthest depth of the occluder over it.
3. `IsBoxOccluded` returns true only if the box's nearest depth exceeds the
   farthest recorded depth over every cell its projection can reach, plus a
   margin for float error and 24-bit depth quantization. A box reaching the
   near plane, an uncovered cell, or a box entirely off screen is never
   occluded.

The caller uses occlusion only in the outermost perspective scene view, with no
height clip, and only for occludees whose materials depth-test.

## Suite

Seeded random meshlets, eyes, walls and boxes are checked against exhaustive
references: a culled meshlet may have no triangle facing the eye at any
corner, and a culled box may have no on-screen sample point whose eye ray
reaches it before a drawn occluder. The cullers must reject a share of the
cases. Two deliberately unsound cullers, the cone test without the sphere
radius and an occlusion test of the box center, are run against the same
references, and each must be caught. Fixed cases cover walls seen from the
front and behind, boxes in front of a wall, across it or reaching the near
plane, and one- and two-sided occluders.
