# Contract: `render.shadows.v1`

Module: `render.pass.shadows` (RFC 0016 layer 6, K7)
Headers: `public/render/pass/shadows/` (`atlas.h`, `shadow_views.h`,
`shadow_passes.h`)
Sources: `render/pass/shadows/atlas.cpp`, `shadow_views.cpp`,
`shadow_passes.cpp` (`ShadowDepthRenderer`, `ShadowReceiverRenderer`), the
shaders `shadow_depth.vert`, `shadow_receiver.vert`, `shadow_receiver.frag`
and the receiver helper `shadow_sample.glsl` (SPIR-V the build generates
with the pinned glslc into `spv/shadow_spv.h`)
Suites: `unittests/rendertest/core/pass/shadows/test_shadows.cpp` with the
oracles in `shadow_oracle.h`, the bad providers in
`test_shadows_negative.cpp`, and the pixel oracles in
`test_shadows_vulkan.cpp` with the seeded receiver in `shadow_defects_spv.h`
Rows: R90 (RFC 0016 K7)

Conventions follow `render.math` and the device port: clip depth 0 to 1,
clip Y up, texture row 0 at the top.

| Clause | Obligation |
| --- | --- |
| S1 | Invalid limits fail with `kInvalidLimits` (sizes not powers of two, minimum above maximum, maximum above the atlas, no caster budget, a guard band that fills the smallest tile). Invalid views fail with their error: a spot wider than 85 degrees (`kConeTooWide`: it needs a cube shadow), no direction or up along forward (`kInvalidLight`), a range inside the near plane (`kInvalidDepthRange`), cascades with a bad count, lambda or distance. Mobile limits are no larger than desktop |
| S2 | Over 1,000 seeded request sets: every request gets a status, in request order. Tiles are powers of two between the minimum and the size wanted, aligned to their size, inside the atlas and pairwise disjoint. Requests ranked (priority, then wanted size, then key) beyond the caster budget are `kOverBudget`, and no more tiles than the budget are handed out. A request is `kReduced` or `kAtlasFull` only when no free aligned block of the size it wanted (or of the minimum, for `kAtlasFull`) remains and no lower-ranked request got a block that large. Repeated keys and non-finite values are `kInvalidRequest`. The plan's counters match its statuses |
| S3 | Planning is deterministic and does not depend on request order (for unique keys) |
| S4 | Spot lights: a caster inside the cone (by 5 percent of its angle) and between near and range projects into its tile's viewport (the tile less its guard band) with depth 0 to 1, and a deeper caster on the same ray deeper; a point behind the light, beyond its range, or outside the square pyramid around its cone does not project; the axis lands on the viewport's center |
| S5 | Flashlights: points inside both fields of view project, points outside either, behind or beyond far do not; the flashlight's up runs up the tile (toward row 0) and forward x up to the right; a tilted up hint gives the same frustum |
| S6 | Cascades: split distances follow the practical scheme (lambda blends logarithmic and uniform; lambda 0 is uniform, 1 logarithmic); over 1,000 seeded cameras every sampled point of the view frustum up to the shadow distance (each split slice's corners included) lies in the orthographic box of the cascade whose split holds its depth, and stays there moved toward the light by up to the caster distance |
| S7 | Texel snapping: moving the camera by fractions of a texel or by several texels moves each cascade's projection of the world origin by whole texels (within float resolution of the world coordinates; cascades below it are counted and left out), and turning the camera keeps each cascade's texel size |

The tile transform applies after the perspective divide
(`ProjectToShadowTile`, `TileTransform`): folding a tile's atlas offset into
a world matrix multiplies it into the view's translation and loses precision
far from the atlas's origin (seen at 5 percent of a small tile).

Sensitivity (`render.shadows.atlas.sensitivity`): with the real providers
clean on the same cases, each seeded defect is detected: a tile handed out
twice, a tile past the atlas's edge, the budget ignored, over-budget requests
reported as a full atlas, priority ignored, a spot frustum from half its
cone, depth reversed, no depth test in the projector, a flashlight with clip
Y down, cascades given the next one's box, unsnapped cascades, boxes cut to
80 percent, the caster distance ignored and lambda ignored (14 of 14).

Pixel oracles (`render.shadows.pixels`, profile `linux-native-vulkan-gpu`):
`ShadowDepthRenderer` draws casters into D32 atlas tiles (one copy pass of
per-(view, caster) clip matrices composed in double, one depth-only render
pass that clears the atlas and sets each view's tile viewport; depth test
less, no culling), and `ShadowReceiverRenderer` draws receivers through
`shadow_sample.glsl` (the atlas and a point sampler in the frame group, the
camera and light in the view group, receiver worlds in the draw group). The
port has no comparison sampler and no depth bias state, so the helper
compares in the shader: a 2x2 bilinear percentage-closer filter with taps
clamped into the tile viewport, a point lit when its depth less the
receiver bias is at most the stored depth, and points without shadow
information lit. The oracle is a CPU ray test in double against the caster
boxes; only pixels whose class holds over a disc of two pixel footprints
plus three projected shadow texels are judged.

| Clause | Obligation |
| --- | --- |
| P1 | The planned spot tile is away from the atlas's origin (after two other lights' tiles), and the oracle's camera reprojects to pixel centers within 0.01 pixels |
| P2 | Depth lands only inside the rendered views' tile viewports: every other texel of the atlas keeps the clear depth 1 |
| P3 | A caster darkens its receiver: every judged pixel in a caster's shadow is dark (red at most 70 of 255, ambient 0.2) |
| P4 | A non-caster does not: every judged pixel shadowed only by a box left out of the depth views is lit (red at least 240) |
| P5 | Judged lit pixels stay lit, and judged pixels outside the spot's cone or range stay dark |
| P6 | Seeded defects are detected by P3-P5: the depth compare reversed, the receiver reading a tile offset by one tile, and a caster left out of the depth pass |
| C1 | Four cascades from `BuildCascades` in four planned 1024 tiles each shade judged pixels, and each pixel away from a split uses the cascade whose split holds its view distance |
| C2 | The cascaded render and a single-cascade reference (one 2048 tile over the same shadow distance) each match the oracle at every judged pixel and agree on at least 99.8 percent of them (measured 100 percent, 2026-09-28) |
| C3 | Moving the camera 2.37 texels along the light's right axis moves a snapped cascade's box by a whole number of texels, and the depth images agree after that shift (at most 1e-4 of the compared texels differ; measured 0); a cascade centered on its sphere instead (unsnapped) differs on 32 percent and is detected |

Open for K7: the Portal flashlight scene on native and the
`SetFlashlightState` census; point-light cube shadows; tile reuse across
frames; the passes as an `IRenderFeature` in the frame graph (`feature.h`);
receiver-side families (K4) including `shadow_sample.glsl`; atlas budget rows
on desktop and the Fold7; a Fold7 and Apple device run.
