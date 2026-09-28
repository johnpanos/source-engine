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
//			    fill. Edges: every face outline, displacement triangle edges
//			    and marker box edges. Selected solids, faces and entities
//			    carry the selection colors (a tint over a texture).
//			    Displacements take uv from their displaced positions, exact
//			    when the texture axes lie in the face plane (the usual case).
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
	std::vector<FaceBatch> faces;  // by material, the untextured batch first
	std::vector<LineVertex> edges; // line list
	std::uint32_t triangles = 0;   // solid and displacement triangles (not markers)
};

// 'sizes' empty: every face is untextured (the flat preview).
SceneGeometry BuildSceneGeometry(
    const viewport::RenderSnapshot &snapshot, const TextureSizes &sizes = {} );

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
