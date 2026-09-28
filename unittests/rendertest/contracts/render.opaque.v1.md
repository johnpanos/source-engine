# Contract: `render.opaque.v1`

Module: `render.pass.opaque` (RFC 0016 layer 6 feature)
Header: `public/render/pass/opaque/opaque.h`; shaders `render/pass/opaque/opaque.vert`, `opaque.frag`
Suites: `unittests/rendertest/core/pass/opaque/test_opaque.cpp` (`render.opaque.null`,
headless) and `test_opaque_vulkan.cpp` (`render.opaque`, `linux-native-vulkan-gpu`)
Rows: R89 (RFC 0016 K5)

| Clause | Obligation |
| --- | --- |
| O1 | A scene view's draw list is drawn in its order into the color and depth targets through graph passes (an upload pass, then the draw pass) with a depth test: a near object stays visible even when a far one draws after it; the rest of the target is the clear color |
| O2 | Two scenes draw independently: neither frame contains the other's content |
| O3 | Destroying one scene leaves another's frames byte-identical |
| O4 | A draw whose mesh or material does not resolve is counted in `OpaqueStats::unresolved` and not drawn; the frame's owner checks the count |
| O5 | Invalid targets fail with `kInvalidTargets` before any pass is added; bind groups made while recording are released behind the token given to `Collect`, and a failure to make them counts in `RecordFailures` |

The per-material color stands in for material families until K4, and the
pass culls nothing until families carry raster state.

Open for K5: world mesh groups, static props and brush models drawn from the
engine's scene (the legacy culling comparison on the K0 views), GPU-resident
instance storage, and the submission-cost target.
