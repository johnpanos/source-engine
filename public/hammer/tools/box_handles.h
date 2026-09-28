//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The 2D box handles shared by the Selection tool (selection
//			bounds) and the Block tool (the pending box) (RFC 0002,
//			hammer.tools), after legacy Box3D:
//
//			  * a handle is a (su, sv) pair in {-1, 0, 1} naming the box edge
//			    or corner it sits on in the view plane (su: -1 left, +1 right;
//			    sv: -1 bottom, +1 top); (0, 0) is not a handle;
//			  * Scale mode shows 8 handles (corners and edge midpoints), Rotate
//			    mode the 4 corners;
//			  * handles sit kHandleOffsetPixels outside the projected box and are
//			    hit within kHandleRadiusPixels (square test); corners win ties;
//			  * resizing moves the handle's edges by the drag and snaps each
//			    moved edge to the grid (interaction_policy.h); edges the handle
//			    does not touch stay put.
//
//=============================================================================//

#ifndef HAMMER_TOOLS_BOX_HANDLES_H
#define HAMMER_TOOLS_BOX_HANDLES_H

#include "hammer/scene/solid_geometry.h"
#include "hammer/tools/input.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/grid.h"

#include <optional>
#include <vector>

namespace hammer::tools
{

enum class BoxHandleMode
{
	Scale,
	Rotate,
};

struct BoxHandle
{
	int su = 0;
	int sv = 0;
	friend bool operator==( const BoxHandle &, const BoxHandle & ) = default;
};

std::vector<BoxHandle> VisibleHandles( BoxHandleMode mode );

// The box's rectangle on the view axes of 'camera'.
viewport::PlaneRect ProjectBox( const viewport::Camera2D &camera, const scene::Box &box );
// 'box' with its view-axis extents replaced by 'rect' (depth kept).
scene::Box WithPlaneRect(
    const scene::Box &box, viewport::ViewAxes axes, const viewport::PlaneRect &rect );

viewport::ScreenPoint HandlePosition(
    const viewport::Camera2D &camera, const viewport::PlaneRect &rect, BoxHandle handle );

std::optional<BoxHandle> HitHandle( const viewport::Camera2D &camera,
    const viewport::PlaneRect &rect, BoxHandleMode mode, double x, double y );

Cursor HandleCursor( BoxHandle handle, BoxHandleMode mode );

// The rectangle after dragging 'handle' by 'rawDelta' (plane units), each
// moved edge snapped. May be inverted or flat; callers validate.
viewport::PlaneRect ResizeRect( const viewport::GridPolicy &grid, const viewport::PlaneRect &start,
    BoxHandle handle, viewport::PlanePoint rawDelta, Modifiers modifiers );

bool RectHasArea( const viewport::PlaneRect &rect );
bool RectContains( const viewport::PlaneRect &rect, viewport::PlanePoint point );

// Appends the handles of 'rect' ('hot' drawn as HandleHot).
void AppendHandles( OverlayList &out, const viewport::Camera2D &camera,
    const viewport::PlaneRect &rect, BoxHandleMode mode, std::optional<BoxHandle> hot );

} // namespace hammer::tools

#endif // HAMMER_TOOLS_BOX_HANDLES_H
