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
| O4 | A draw whose mesh or program does not resolve (unknown material, or a texture not resident), whose mesh stride is not the program's, or whose program reads a draw group (`drawLayout`) the instance lacks or has of another layout, is counted in `OpaqueStats::unresolved` and not drawn; the frame's owner checks the count |
| O5 | Invalid targets fail with `kInvalidTargets` before any pass is added. Mesh buffers, the groups' textures and uniform buffers are imported in their residency usages, so the graph (and the null device) sees every access. The pass creates no device objects: frames leave the device's live count unchanged |

A program that reads a draw group gets the instance's
(`MeshInstanceDesc::drawGroup` through `IDrawGroups`), bound at kDraw. The
positive draw-group case waits for the first family that reads one
(`lightmapped`).

Open for K5: world mesh groups, static props and brush models drawn from the
engine's scene (the legacy culling comparison on the K0 views), GPU-resident
instance storage, and the submission-cost target.
