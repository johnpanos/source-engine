# Contract: `render_adapter.viewport-geometry.v1`

Module: `hammer.adapters.render` (RFC 0002; RFC 0016 "Editor viewports")
Headers: `hammer/adapters/render/scene_geometry.h`, `hammer/adapters/render/viewport_renderer.h`,
`hammer/adapters/render/viewport_service.h`, `hammer/adapters/render/material_textures.h`
Suites: `unittests/hammertest/render/test_viewport_geometry.cpp`
(`hammer.adapters.render.geometry`, headless), `test_viewport_renderer.cpp`
(`hammer.adapters.render.viewport.null`, headless), `test_viewport_renderer_vulkan.cpp`
(`hammer.adapters.render.viewport`, `linux-native-vulkan-gpu`) and `test_viewport_service.cpp`
(`hammer.adapters.render.service`, headless, with a TSan row) and `test_viewport_service_vulkan.cpp`
(`hammer.adapters.render.service.vulkan`, `linux-native-vulkan-gpu`)
Migration: R08-GTK-WORKSPACE follow-up (RFC 0016 decision "Editor viewports"; R17)

## Purpose

The editor's viewports on the RFC 0016 render core. The pure half turns what the headless editor
presents (`viewport::RenderSnapshot`, the workspace's cameras, grid lines and the active tool's
`tools::OverlayList`) into per-material face batches for the material families and line items for
`render.pass.lines`; `ViewportRenderer` draws one view per call offscreen on the device the
composition root passes in and hands back sRGB RGBA8 pixels: solids through `render.pass.opaque`
and the render core's editor preview, `render::material::ProgramResolver::ResolvePreview` (RFC
0016 K5; `render.material.v2` F-PREVIEW), edges, grid and overlay through `render.pass.lines`.
`ViewportService` runs the renderer on a render sequence and replies on the host's.

## Obligations

| Clause | Obligation |
| --- | --- |
| G1 | A snapshot's solids become two triangles per quad face (a fan per polygon) and one edge per face side; displacements their grid; point entities shaded marker boxes with edges. Without texture sizes every face is in one untextured batch |
| G2 | Selection is data: the selected solid's faces carry the selection fill and its edges the selection edge color; brush-entity solids their entity color; others the plain edge color |
| G3 | Faces carry the fixed two-light shading of their outward normal (fullbright preview) |
| G4 | Every overlay kind maps to its items: world lines, boxes and polygons untested in every view; screen rects as outlines; handles filled squares or discs; labels not drawn |
| G5 | Grid lines become full-width or full-height screen lines on pixel centers |
| G6 | `ViewFor(Camera2D)` projects exactly like `Camera2D::WorldToScreen` for every 2D kind; `ViewFor(Camera3D)` like `Camera3D::WorldToScreen`, with depth in [0, 1] growing with distance |
| G7 | Each projection check rejects a seeded wrong view |
| G8 | Textured preview: a face whose material has a texture size goes into that material's batch, colored by the shading alone (the selection tint over a selected solid or face), with uv = (dot(p, axis) / scale + shift) / size from the side's texture axes (`FaceDraw::uAxis`, `vAxis`); marker boxes stay untextured; a wrong size is rejected |
| G9 | A material without a size, or with a zero size, stays in the untextured batch |
| G14 | `ImportSourceMaterial` (2026-09-28, "Viewports on ResolvePreview"): a material is its VMT imported by `render::material::ImportVmt` (the one VMT reader: patches through the source's own files, conditions, fallback blocks) and the decoded image of its base texture (the last `$basetexture`, as `ResolvePreview` reads it) keyed by the importer's normalized name (`materials/a/b`). A patch imports through its include with its replacement; a missing VMT, a missing include and an unknown shader are refused with the reason; a texture that does not decode is absent, not an error. `SourceMaterialFromVariables` maps a shader's variables (`MapVariables`) for sources without VMT text |
| G10 | `BuildMipChain` (2026-09-28): an image has floor(log2(max(w, h))) + 1 levels down to 1x1, level m `max(1, size >> m)` per axis; each texel is the equally weighted box of its 2x2 source texels (the last box of an odd axis 3 wide, a one-texel axis 1 wide), color averaged in linear light (sRGB decode, mean, encode to the nearest byte), alpha as stored. A black and white 2x2 gives 188 (an encoded-byte mean, 128, is rejected); an image whose bytes do not match its size has no chain |
| G11 | A solid's untextured fill is a function of its id, so it keeps its color when other solids are added or removed (a chunk restaged alone). `ChunkOf(id)` is the id's value over 64 |
| G12 | Models and instances (R17 follow-up, 2026-09-28; `hammer.adapters.render.models`): with `GeometryOptions::modelBox`, an entity drawn as a model has no marker faces; its model box goes to `edges2D` (drawn in 2D views only) in its color when unselected and to `edges` in the selection color when selected (legacy `CMapStudioModel`: the bounds in 2D, a box in 3D only when selected). An entity without a model box keeps its marker. Instance content (`tint`) multiplies every face and marker color by `kInstanceTint` and draws unselected edges in `kInstanceEdgeColor` |
| G13 | `BuildModelBatches`: one batch per material, the untextured batch first; a mesh whose texture resolved (`mdl::ResolvedMaterial::found`) and has a texture size is textured with the model's uv, any other is untextured (uv 0) in the fill; colors are the model-space shading times the tint; the skin family picks the texture; indices name the batch's vertices. `ModelWorld` is translate(origin) * `mapgeometry::AngleMatrix`(angles) * scale; `ModelWorldBox` bounds the transformed corners of the model's box; `ModelTint` is the selection fill when selected, else the render color (white) times `kInstanceTint` for instance content |
| V1 | The scene is staged once per caller key |
| V2 | A 2D view runs the grid pass (clearing) and the scene pass (loading): edges then overlay. The 3D view runs the opaque pass (clearing; every batch resolves) and then the lines pass: edges, overlay |
| V3 | Nothing blocks: `Take` returns nothing until the device completes the frame |
| V4 | A view without its camera, a zero size and an unknown ticket are refused |
| V5 | Everything the renderer made is released once it is gone |
| V6 | With a material source, the source is asked once per material (not per restage); a material with a texture gets its own batch and program; one without stays untextured; the camera view draws both batches, then the edges; everything is released |
| V11 | Preview (2026-09-28): every material resolves through `ResolvePreview`; the renderer stages each texture its program samples by the program's name (shared between materials that name it) with its mip chain, gives its draw group a neutral lightmap page (the 1x1 white; lighting is one) and its frame group the LDR terms without shader encoding (the target has an sRGB view), and draws through `render.pass.opaque` with `OpaqueSources::drawGroups` and `frames`. A program whose vertex stride is not `PreviewVertex`'s, or that reads another draw input or a view group, is a named failure. A material setting a variable the preview does not read still draws textured and is counted: `SceneStats::approximatedMaterials` and `ignored` (variable -> materials). A material the source refuses or the resolver refuses is named in `SceneStats::failures` (and `failedMaterials`) and draws untextured; nothing is silent. Untextured faces, markers and the flat preview draw through the same path: a neutral `UnlitGeneric` material whose base texture is the white. Faces carry display colors (`FaceVertex`); the renderer converts them to the program's vertex, re-encoding each display byte as gamma 2.2 because the preview program decodes vertex colors as gamma 2.2 (`ResolvePreview`'s gamma term, render-core 8e671774), so every byte comes back within one level |
| V7 | On a device that exports no images, `CanExport()` is false and an external frame is refused with `kUnsupported` |
| V8 | Surfaces (2026-09-28; since V11 by `ResolvePreview`'s rules): a blended batch (`$translucent` or `$vertexalpha` alpha, `$additive` additive) is drawn in a second opaque-pass instance that loads the targets and writes no depth, after every opaque and alpha-tested batch, back to front by instance depth (stable); `ViewStats::blended` counts it. `$alphatest` with no reference uses 0.7, the legacy default (`detail::AlphaTestReference`). `$translucent` with `$additive` draws additive (the preview's rule; before V11 it drew translucent). Every texture is staged with its full mip chain (`TextureCache::StageMips`), sampled trilinearly |
| V9 | Per-chunk restaging (2026-09-28): an object's geometry is resident in chunk `ChunkOf(id)`, one face mesh per material and one edge mesh per chunk. A new key rebuilds only the chunks whose objects differ from the staged ones: moving one solid uploads exactly its chunk's face and edge meshes (the bytes equal that chunk's geometry, restated), the same content under a new key uploads nothing, and a chunk that empties is dropped and its meshes released. `SceneStats::stagedChunks` and `stagedMeshes` count the last restage; the resident totals equal the whole scene's geometry |
| V10 | Models (`hammer.adapters.render.models`, render.device.null): with an `IModelSource` a studio-model entity is read once per canonical path, staged once per (skin, tint) as indexed batches in model space, and drawn as one scene instance (and one indexed draw) per batch with `ModelWorld`; a missing, unparsable or empty model draws its marker (`SceneStats::missingModels`); moving a model entity restages its chunk without reading or staging the model again; each `InstanceDraw` is one chunk keyed apart from the id chunks (`SceneStats::instanceChunks`); everything is released with the renderer |
| R1 | On a real device, the pixel under a top face's projected center has that face's built color within one level (the preview program decodes vertex colors as gamma 2.2; the renderer re-encodes the display colors for it), a dark fill (display below 49) included |
| R2 | Edge, grid and overlay pixels land where the camera projects them, in their colors |
| R3 | A restage follows the selection; the same inputs give byte-identical frames; the validation layer reports nothing |
| R4 | Textured: where the side's axes put u in a texture's left (red) half the top face shows red, in its right (blue) half blue, each the texel times the shading in linear light within two levels; a source with no texture leaves the R1 colors |
| R6 | Mipmaps: a one-texel black and white checker seen from afar shows the linear-light mid gray (188) times the shading at 49 points of the face within four levels (0 measured; mip 0 alone deviates by 59); from close the same face shows over 96 levels of contrast |
| R7 | Blending: a translucent pane (blue, alpha 128) over an opaque wall (red) shows their blend in linear light within three levels, an opaque pane of the same texture only itself within two; an alpha-tested grate shows the wall through its transparent half and itself on its opaque half |
| R8 | After one solid of a three-chunk scene moves, the restaged renderer (one chunk rebuilt) draws the 3D and top views byte-identical to a fresh renderer of the moved scene, and both frames differ from those before the move |
| R5 | Exported frames (clause D18): an external frame's memory, mapped through its description, equals the read-back frame of the same view; a leased image is not drawn into (a second frame takes a new image), a returned one is, and a resize replaces the free images |
| M1 | Models on Vulkan (`hammer.adapters.render.viewport.models`; a scene extracted with an `InstancePreview` over real VMF text and a synthetic MDL/VVD/VTX box): a prop at yaw 90 shows the texel times its top-face shading within two levels where its rotated footprint projects, and the background where only its unrotated footprint would lie |
| M2 | A missing model draws its marker and is counted; one model file is read once for two entities |
| M3 | The instance's box shows its fill times the instance tint where its placement (yaw 90, translation) puts its top, and the tint changes the fill; the instance's prop shows the texel times the tinted shading |
| M4 | 2D: the prop's rotated world box is drawn in its entity color (and not the unrotated box); instance edges are drawn in the instance edge color |
| M5 | Selecting the prop tints its model with the selection fill; the validation layer reports nothing. Seeded faults: a model world matrix without its rotation, without its translation, and instance content without its tint each fail the suite |
| S1 | A job's pixels arrive only through the reply runner, never on the render thread, at the requested size |
| S2 | Jobs reply in submission order |
| S3 | A view without its camera and a missing scene reply with `kInvalidView` |
| S4 | Destroying the service with jobs in flight: no reply runs afterwards, and once the destructor has returned and the device has polled, it holds nothing the renderer made |
| S5 | A shut-down render runner refuses jobs, and destroying the service then does not wait |
| S6 | A material source set on the service is asked only on the render thread and is destroyed there with the service |
| SV1 | On a real device, two documents with their own services share one device and its one render sequence (the device port allows concurrent encoder recording only): their views, submitted interleaved, equal what a lone renderer draws of each view (`hammer.adapters.render.service.vulkan`) |
| SV2 | Restoration after a remount: a replaced material source with the same textures gives the frame drawn before |
| SV3 | Restoration after a resize: a view drawn smaller and then at its size again equals its first frame |
| SV4 | Teardown: a service destroyed with jobs in flight, exported frames still leased and a source swap queued leaves the device holding what it held before the service existed |

## Inputs and outputs

A `RenderSnapshot` and a caller key (the snapshot revision and selection); per view a
`ViewRequest` (kind, the workspace's camera, grid lines, overlay, framebuffer size); optionally an
`IMaterialTextures` source of materials (`SourceMaterial`: the imported `MaterialDesc` and its
textures, sRGB RGBA8, by the importer's names) and an `IModelSource` of studio models
(`ModelAsset`: a `content.studio-model` model and its resolved materials). Output: sRGB RGBA8 rows, top first, or with `ViewRequest::external` an exported image
(`ExternalFrame`: a dmabuf handle and plane, and a lease). The image is not drawn into again until
the host returns the lease (`ReturnFrame`); the handle stays the renderer's.

## Failure behavior

Refusals are `ViewportStatus` values; a refused view submits nothing and leaks nothing. A missing
or undecodable texture draws its faces untextured (counted in `SceneStats::missingTextures`); a
material that does not import or resolve too, and is named with its reason in
`SceneStats::failures`; variables the preview ignores are counted in `SceneStats::ignored`; a
missing, malformed or empty model draws its entity's marker (counted in `SceneStats::missingModels`); a
draw the opaque pass cannot resolve is counted in `ViewStats::unresolved`.

## Lifetime and threading

The renderer borrows the device and the material source and is destroyed before them; its
destructor waits for its own frames. It is used from one sequence. `ViewportService` owns the
material source and the renderer on its render sequence (RFC 0016 decision "threading").

## Evidence and oracles

Pure checks against independently restated rules (G), the null device's recorded command stream
(V), relational pixel checks on Vulkan (R) and thread and lifetime checks on a real render thread
(S), not golden images. Declared limits: the preview approximates by contract (base texture times
`$color` and `$alpha`, blend and alpha test; bump maps, detail, env maps and the rest are counted,
not drawn); the mip filter is a box (no coverage-preserving alpha
mips, RFC 0012 R66, so alpha-tested texels thin with distance); blended batches sort per chunk
batch, not per face; sampling is trilinear without anisotropy. Models draw in their bind pose (no
sequence), so a model whose reference skeleton differs from its idle sequence (Portal 2's turret
and faith plate) draws as rigged, not as placed in game; the shading turns with the model (it is
baked in model space); models draw at every distance (legacy fades them past 400 units).
