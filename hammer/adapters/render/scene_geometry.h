//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The editor's viewport geometry on the RFC 0016 render core (RFC
//			0002 hammer.adapters.render; RFC 0016 "Editor viewports"). Pure
//			functions from what the headless editor presents to what
//			render.pass.lines draws, so every rule here is testable without a
//			device:
//
//			  * BuildSceneGeometry: a viewport::RenderSnapshot as two vertex
//			    sets in the LineVertex layout. Faces: triangles with the
//			    editor's fixed two-light shading baked into the vertex colors
//			    (fullbright preview, RFC 0016 decision "lighting"), per-solid
//			    fill colors, displacement grids tinted by blend alpha, entity
//			    marker boxes. Edges: every face outline, displacement triangle
//			    edges and marker box edges. Selected solids, faces and
//			    entities carry the selection colors.
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
#include "render/pass/lines/lines.h"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace hammer::render_adapter
{

using ::render::pass::lines::LineVertex;

struct SceneGeometry
{
	std::vector<LineVertex> faces; // triangle list
	std::vector<LineVertex> edges; // line list
	std::uint32_t triangles = 0;   // solid and displacement triangles (not markers)
};

SceneGeometry BuildSceneGeometry( const viewport::RenderSnapshot &snapshot );

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
