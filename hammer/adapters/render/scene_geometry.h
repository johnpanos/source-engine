//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewport geometry on the RFC 0016 render core (RFC
//			0002 hammer.adapters.render; RFC 0016 "Editor viewports"). Pure
//			functions from what the headless editor presents to what
//			render.pass.lines draws, so every rule here is testable without a
//			device:
//
//			  * BuildSceneGeometry: a viewport::RenderSnapshot as face batches
//			    for the material families (render.material's unlit vertex:
//			    position, uv, color) and an edge set in the LineVertex layout.
//			    Faces: triangles with the editor's fixed two-light shading
//			    baked into the vertex colors (fullbright preview, RFC 0016
//			    decision "lighting"). A face whose material's base texture
//			    size is known (the textured preview) goes into that
//			    material's batch, its uv from the side's texture axes
//			    (texels = dot(p, axis) / scale + shift, over the texture's
//			    size) and its color the shading alone; every other face, the
//			    displacement grids' blend tint and the entity marker boxes go
//			    into the untextured batch (material ""), colored by per-solid
//			    fill (a hash of the solid's id, so a solid keeps its color
//			    whatever else is added or removed). Edges: every face
//			    outline, displacement triangle edges
//			    and marker box edges. Selected solids, faces and entities
//			    carry the selection colors (a tint over a texture).
//			    Displacements take uv from their displaced positions, exact
//			    when the texture axes lie in the face plane (the usual case).
//			    The geometry of a snapshot is the concatenation of its
//			    chunks' (ChunkOf), which ViewportRenderer stages separately.
//			    With GeometryOptions: entities drawn as models (modelBox
//			    answers their world box) get no marker faces; their box
//			    edges go to edges2D (drawn in 2D views only) when unselected
//			    and to edges in the selection color when selected (legacy
//			    CMapStudioModel: the bounds in 2D, a wireframe box in 3D
//			    only when selected). Instance content (tint) multiplies its
//			    face colors by the tint and draws unselected edges in
//			    edgeColor.
//			  * BuildModelBatches: a model's meshes for one skin as
//			    per-material indexed batches in model space, uv as the model
//			    stores it, shaded by the model-space normal (the shading
//			    turns with the model: a preview approximation), times a
//			    tint.
//			  * ModelWorld / ModelWorldBox: a model entity's placement
//			    (origin, Source angles through mapgeometry::AngleMatrix,
//			    uniform model scale) and its world box.
//			  * ChunkOf: the chunk an object's geometry is resident in.
//			  * AppendOverlay: a tools::OverlayList as line items (world
//			    lines, boxes and polygons; screen rects; filled screen
//			    handles). Labels are not drawn (no text pass yet).
//			  * AppendGrid: viewport::GridLine lists as screen-space lines.
//			  * ViewFor: a Camera2D or Camera3D as the pass's view, with the
//			    3D depth range fitted to the scene.
//
//			Colors and shading are the GTK shell's former GL renderer's, so the
//			migration changes no pixel meaning. They are data, not render
//			policy: selection tint lives here, not in render.material.
//
//=============================================================================//

#ifndef HAMMER_ADAPTERS_RENDER_SCENE_GEOMETRY_H
#define HAMMER_ADAPTERS_RENDER_SCENE_GEOMETRY_H

#include "hammer/scene/solid_geometry.h"
#include "mdl/studio_model.h"
#include "hammer/tools/input.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/extraction.h"
#include "hammer/viewport/grid.h"
#include "render/material/unlit_family.h"
#include "render/pass/lines/lines.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace hammer::render_adapter
{

using ::render::pass::lines::LineVertex;

using ::render::material::UnlitVertex;

struct TextureSize
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

// A material's base texture size when the textured preview has it.
using TextureSizes = std::function<std::optional<TextureSize>( const std::string &material )>;

struct FaceBatch
{
	std::string material;              // as authored; "" for the untextured batch
	std::vector<UnlitVertex> vertices; // triangle list
};

struct SceneGeometry
{
	std::vector<FaceBatch> faces;    // by material, the untextured batch first
	std::vector<LineVertex> edges;   // line list
	std::vector<LineVertex> edges2D; // line list drawn in 2D views only
	std::uint32_t triangles = 0;     // solid and displacement triangles (not markers)
};

struct GeometryOptions
{
	TextureSizes sizes; // empty: every face is untextured (the flat preview)
	// The world box of an entity the renderer draws as a model; nothing for
	// an entity drawn as its marker. Empty: every entity is a marker.
	std::function<std::optional<scene::Box>( const viewport::EntityDraw & )> modelBox;
	// Instance content: face colors times 'tint', unselected edges in 'edgeColor'.
	std::optional<scene::Rgb> tint;
	std::optional<scene::Rgb> edgeColor;
};

// A model file as the preview draws it: the parsed model and the material
// each texture resolves to (mdl::ResolveMaterials).
struct ModelAsset
{
	mdl::Model model;
	std::vector<mdl::ResolvedMaterial> materials;
};

struct ModelBatch
{
	std::string material; // as a VMF names it; "" for the untextured batch
	std::vector<UnlitVertex> vertices;
	std::vector<std::uint32_t> indices; // triangle list
};

// The chunk an object's geometry is staged in (ViewportRenderer): its id's
// value over 64. Ids are stable across edits, so an edit restages only the
// chunks of the objects it touched, and the chunk size bounds the draws (one
// per material per chunk).
constexpr unsigned kChunkShift = 6;
constexpr std::uint64_t ChunkOf( scene::ObjectId id )
{
	return id.value >> kChunkShift;
}

// 'sizes' empty: every face is untextured (the flat preview).
SceneGeometry BuildSceneGeometry(
    const viewport::RenderSnapshot &snapshot, const TextureSizes &sizes = {} );
SceneGeometry BuildSceneGeometry(
    const viewport::RenderSnapshot &snapshot, const GeometryOptions &options );

// The model's meshes drawn with 'skin' (mdl::TextureIndex), one batch per
// material (the untextured batch first, then by name): a mesh whose material
// resolved and has a texture size in 'sizes' is textured, colored by the
// shading times 'tint'; any other mesh is untextured, colored by the shading
// times 'fill' times 'tint'. Empty for a model without triangles.
std::vector<ModelBatch> BuildModelBatches( const ModelAsset &asset, std::int32_t skin,
    const TextureSizes &sizes, const scene::Rgb &tint, const scene::Rgb &fill );

// The tint a model entity's meshes are drawn with: the selection fill when
// selected; else its render color (white when none), times kInstanceTint for
// instance content.
scene::Rgb ModelTint( const viewport::EntityDraw &entity, bool instanceContent );

// The entity's model-to-world transform: translate(origin) * rotate(angles) *
// scale(modelKeys.scale).
::render::math::float4x4 ModelWorld( const viewport::EntityDraw &entity );
// The world box of the model's bounds (its eight corners) under ModelWorld.
scene::Box ModelWorldBox( const mdl::Model &model, const viewport::EntityDraw &entity );

// Tool feedback is drawn over the scene in every view (no depth test), as the
// editor has always drawn it: a pending box inside a wall stays visible.
void AppendOverlay( const tools::OverlayList &overlay, ::render::pass::lines::LineList &out );

// Grid lines span the view's logical width or height.
void AppendGrid( std::span<const viewport::GridLine> grid, double width, double height,
    ::render::pass::lines::LineList &out );

::render::pass::lines::LinesView ViewFor( const viewport::Camera2D &camera );
// The depth range reaches past the farthest corner of 'bounds' seen from the
// eye (at least 16384 units).
::render::pass::lines::LinesView ViewFor(
    const viewport::Camera3D &camera, const std::optional<scene::Box> &bounds );

} // namespace hammer::render_adapter

#endif // HAMMER_ADAPTERS_RENDER_SCENE_GEOMETRY_H
