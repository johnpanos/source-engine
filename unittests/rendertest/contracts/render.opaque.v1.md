# Contract: `render.opaque.v1`

Module: `render.pass.opaque` (RFC 0016 layer 6 feature)
Header: `public/render/pass/opaque/opaque.h`; the pass has no shaders of its
own (the families' programs draw)
Suites: `unittests/rendertest/core/pass/opaque/test_opaque.cpp` (`render.opaque.null`,
headless) and `test_opaque_vulkan.cpp` (`render.opaque`, `linux-native-vulkan-gpu`)
Rows: R89 (RFC 0016 K5), drawing through R88's families (K4)

| Clause | Obligation |
| --- | --- |
| O1 | A scene view's draw list is drawn in its order into the color and depth targets in one graph pass, each draw through its material's family program (`render.material` `IDrawPrograms`): the program's pipeline, its material group (kMaterial) and the shared draw-constant prefix (`FamilyDrawConstants`: world-to-clip, then world) cut to the program's size. With the family's depth test a near object stays visible even when a far one draws after it; the rest of the target is the clear color |
| O2 | Two scenes draw independently: neither frame contains the other's content |
| O3 | Destroying one scene leaves another's frames byte-identical |
| O4 | A draw whose mesh or program does not resolve (unknown material, or a texture not resident), whose mesh stride is not the program's, or whose program reads a draw group (`drawLayout`) the instance lacks or has of another layout, or a frame or view group (`frameLayout`, `viewLayout`) the sources lack or have of another layout, is counted in `OpaqueStats::unresolved` and not drawn; the frame's owner checks the count |
| O5 | Invalid targets fail with `kInvalidTargets` before any pass is added. Mesh buffers, the groups' textures and uniform buffers are imported in their residency usages, so the graph (and the null device) sees every access. The pass creates no device objects: frames leave the device's live count unchanged |
| O6 | Every family's groups: a lightmapped draw reads its lightmap page from its instance's draw group (`LightmappedFamily::Request` and `LightmapGroup`), a pbr draw reads the frame's split-sum table and the view's model lighting (`PbrFamily::Request`, `FrameGroup`, `ViewGroup`), and a vertexlit draw reads its lighting from its instance's draw group (`VertexLitFamily::Request` and `LightingGroup`); all three draw with no unresolved draw and the expected pixels (the page times the lightmap scale; a white rough dielectric under a uniform ambient cube returns the cube; white under an ambient cube lit on +z shows that face's light on the face toward the camera). Without the view group the pbr draw is counted and not drawn. Suites: `render.opaque` (O6.*) and `render.opaque.null` (N4) |

A program that reads a draw group gets the instance's
(`MeshInstanceDesc::drawGroup` through `IDrawGroups`), bound at kDraw; one
that reads a frame or view group gets `OpaqueSources::frame` or `view`,
bound at kFrame or kView. The positive cases wait for the first families
that read them (`lightmapped`'s draw group, `pbr`'s frame and view groups).

Open for K5: world mesh groups, static props and brush models drawn from the
engine's scene (the legacy culling comparison on the K0 views), GPU-resident
instance storage, and the submission-cost target.
