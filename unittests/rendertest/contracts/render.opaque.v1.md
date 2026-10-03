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
| O6 | Every family's groups: a lightmapped draw reads its lightmap page from its instance's draw group (`LightmappedFamily::Request` and `LightmapGroup`), a pbr draw reads the frame's split-sum table and its model lighting from its instance's draw group (`PbrFamily::Request`, `FrameGroup`, `LightingGroup`; both families are points of the one surface program, `surface_program.h`), and a vertexlit draw reads its lighting from its instance's draw group (`VertexLitFamily::Request` and `LightingGroup`); all three draw with no unresolved draw and the expected pixels (the page times the lightmap scale; a white rough dielectric under a uniform ambient cube returns the cube; white under an ambient cube lit on +z shows that face's light on the face toward the camera). Without its lighting group the pbr draw is counted and not drawn. Suites: `render.opaque` (O6.*) and `render.opaque.null` (N4) |

A program that reads a draw group gets the instance's
(`MeshInstanceDesc::drawGroup` through `IDrawGroups`), bound at kDraw; one
that reads a frame or view group gets `OpaqueSources::frame` or `view`,
bound at kFrame or kView. The positive cases wait for the first families
that read them (`lightmapped`'s draw group, `pbr`'s frame and view groups).

Open for K5: world mesh groups, static props and brush models drawn from the
engine's scene (the legacy culling comparison on the K0 views), GPU-resident
instance storage, and the submission-cost target.

## render.pass.world (RFC 0016 K5)

`render.world.null` (`unittests/rendertest/core/pass/world/test_world_pass.cpp`):

| Clause | Obligation |
| --- | --- |
| W1 | SetWorld claims each material through `material::ProgramResolver` (the one place that names families): LightmappedGeneric, and UnlitGeneric as the lightmapped term with lighting fixed at one; a material outside the model or blended stays legacy, with its gap named |
| W2 | A queued view records at its slot with its materials' textures and lightmap pages imported through `IWorldTextures`; a surface without a page samples a neutral white |
| W3 | A slot recorded again (the backend re-records a frame's stream for a capture) draws the same view |
| W4 | A slot whose view was queued against an earlier world draws nothing, and a view naming a surface the pass does not draw is counted; neither is silent |
| W5 | The stats name the claimed materials and their surface counts |
| W6 | A claimed material the render sequence fails (a texture that does not import) is a counted failure naming the material and why; the pass still claims it and never hands it back to legacy (RFC 0016 "No escape hatches") |
| W7 | A slot of an earlier world recorded again (a capture across a level change) fails alone: the next world's queued views stay queued and draw |
| W8 | Objects a frame's slots used outlive that frame: a level change or a second target format between two slots of one submission leaves the first slot's objects alive; they are released at a later frame's slot, behind its submitted token (`WorldTarget::frame`) |
| W10 | A view whose slot never recorded is skipped (counted, not a failure) only when no slot of its host frame recorded; a view lost from a host frame that recorded is a failure |
| W9 | A variable the model does not read keeps its material out unless it holds its shader's neutral value (`MaterialDesc::declaredDefaults`); one with no neutral value keeps it out too; the gap names the variable |
| W13 | A sparse update to the current probe atlas records on the next world view; a rectangle outside the atlas fails that view by name instead of silently sampling stale light |

Model geometry selection (R96): static instances and posed models may carry a
value-owned subset of surface indices. Null selects all surfaces; an explicit
empty list is a valid blank body. Subsets contain increasing, unique, in-range
indices. Eligibility and recording examine only selected surfaces, so an inactive
unsupported material cannot reject an otherwise supported body or LOD. Static
views may override the instance's selection; absence inherits it at queue time.
Invalid selections are refused before publishing a slot. Queued selections survive
later body-group and LOD changes. `render.lab.model-selection` proves the
posed/static pixel footprints, blank bodies and LODs, LOD replacement materials,
capture replay and swapped-selection negative controls on native Vulkan.
Studio body arithmetic remains owned by `mdl::BodyPart::SelectedModel`.

Portal/view state (R91): `render.lab.view-state` records nested stencil masks,
exit clipping, depth reset and viewmodel depth range with independent pixel
controls and capture replay. The dynamic stage-1 PortalRefract aperture accepts
the shader's `$model` and `$translucent` classification flags; the latter
does not enable color blending on the depth/stencil-only program. Full and
partially open ellipses preserve the parent outside the opening. Stages 0 and 2
remain outside this aperture contract.

World surface draws use counter-clockwise front faces and cull back faces unless
the resolved material authors `$nocull`. The color, depth-only and normal
prepasses share this policy. Portal exit clipping retains its existing tolerance;
a back-facing exit wall inside that tolerance must not hide the linked room.
The view-state suite includes an on-wall case and a two-sided negative control.

The legacy BSP world adapter normalizes Source brush fans to that core winding;
WMSH surfaces already carry the core convention. The product's ordered custom
effect exception retains the existing SolidEnergy shader (fizzler flow and live
proxy state) beside the core scene. Native replay checks blending, depth,
clipping, slot order, capture replay and diagnostic suppression. Only portal
refraction retains a framebuffer snapshot; SolidEnergy adds no copy exception.
