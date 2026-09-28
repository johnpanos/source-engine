//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/box_handles.h.
//
//=============================================================================//

#include "hammer/tools/box_handles.h"

#include "hammer/tools/interaction_policy.h"
#include "mapgeometry/vec3.h"

#include <algorithm>

namespace hammer::tools
{

using viewport::PlanePoint;
using viewport::PlaneRect;
using viewport::ScreenPoint;

std::vector<BoxHandle> VisibleHandles( BoxHandleMode mode )
{
	// Corners first so they win ties in HitHandle.
	std::vector<BoxHandle> handles = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 } };
	if ( mode == BoxHandleMode::Scale )
	{
		handles.push_back( { 0, -1 } );
		handles.push_back( { 1, 0 } );
		handles.push_back( { 0, 1 } );
		handles.push_back( { -1, 0 } );
	}
	return handles;
}

PlaneRect ProjectBox( const viewport::Camera2D &camera, const scene::Box &box )
{
	const PlanePoint a = camera.ToPlane( box.mins );
	const PlanePoint b = camera.ToPlane( box.maxs );
	return { { std::min( a.u, b.u ), std::min( a.v, b.v ) },
	    { std::max( a.u, b.u ), std::max( a.v, b.v ) } };
}

scene::Box WithPlaneRect( const scene::Box &box, viewport::ViewAxes axes, const PlaneRect &rect )
{
	scene::Box out = box;
	mapgeometry::SetComponent( out.mins, axes.u, rect.min.u );
	mapgeometry::SetComponent( out.maxs, axes.u, rect.max.u );
	mapgeometry::SetComponent( out.mins, axes.v, rect.min.v );
	mapgeometry::SetComponent( out.maxs, axes.v, rect.max.v );
	return out;
}

namespace
{

double Pick( int s, double lo, double hi )
{
	return s < 0 ? lo : ( s > 0 ? hi : ( lo + hi ) * 0.5 );
}

} // namespace

ScreenPoint HandlePosition(
    const viewport::Camera2D &camera, const PlaneRect &rect, BoxHandle handle )
{
	const ScreenPoint lo = camera.PlaneToScreen( rect.min ); // bottom-left on screen
	const ScreenPoint hi = camera.PlaneToScreen( rect.max ); // top-right on screen
	ScreenPoint p;
	p.x = Pick( handle.su, lo.x - kHandleOffsetPixels, hi.x + kHandleOffsetPixels );
	// v up = screen y down: sv +1 is the top (smaller y).
	p.y = Pick( handle.sv, lo.y + kHandleOffsetPixels, hi.y - kHandleOffsetPixels );
	return p;
}

std::optional<BoxHandle> HitHandle( const viewport::Camera2D &camera, const PlaneRect &rect,
    BoxHandleMode mode, double x, double y )
{
	std::optional<BoxHandle> best;
	double bestDistance = 0.0;
	for ( BoxHandle handle : VisibleHandles( mode ) )
	{
		const double distance = HandleDistance( HandlePosition( camera, rect, handle ), { x, y } );
		if ( distance <= kHandleRadiusPixels && ( !best || distance < bestDistance ) )
		{
			best = handle;
			bestDistance = distance;
		}
	}
	return best;
}

Cursor HandleCursor( BoxHandle handle, BoxHandleMode mode )
{
	if ( mode == BoxHandleMode::Rotate )
		return Cursor::Rotate;
	if ( handle.su == 0 )
		return handle.sv > 0 ? Cursor::ResizeN : Cursor::ResizeS;
	if ( handle.sv == 0 )
		return handle.su > 0 ? Cursor::ResizeE : Cursor::ResizeW;
	if ( handle.sv > 0 )
		return handle.su > 0 ? Cursor::ResizeNE : Cursor::ResizeNW;
	return handle.su > 0 ? Cursor::ResizeSE : Cursor::ResizeSW;
}

PlaneRect ResizeRect( const viewport::GridPolicy &grid, const PlaneRect &start, BoxHandle handle,
    PlanePoint rawDelta, Modifiers modifiers )
{
	PlaneRect out = start;
	if ( handle.su < 0 )
		out.min.u = SnapValue( grid, start.min.u + rawDelta.u, modifiers );
	else if ( handle.su > 0 )
		out.max.u = SnapValue( grid, start.max.u + rawDelta.u, modifiers );
	if ( handle.sv < 0 )
		out.min.v = SnapValue( grid, start.min.v + rawDelta.v, modifiers );
	else if ( handle.sv > 0 )
		out.max.v = SnapValue( grid, start.max.v + rawDelta.v, modifiers );
	return out;
}

bool RectHasArea( const PlaneRect &rect )
{
	return rect.max.u > rect.min.u && rect.max.v > rect.min.v;
}

bool RectContains( const PlaneRect &rect, PlanePoint point )
{
	return point.u >= rect.min.u && point.u <= rect.max.u && point.v >= rect.min.v &&
	       point.v <= rect.max.v;
}

void AppendHandles( OverlayList &out, const viewport::Camera2D &camera, const PlaneRect &rect,
    BoxHandleMode mode, std::optional<BoxHandle> hot )
{
	const HandleShape shape =
	    mode == BoxHandleMode::Rotate ? HandleShape::Circle : HandleShape::Square;
	for ( BoxHandle handle : VisibleHandles( mode ) )
	{
		const OverlayRole role =
		    hot && *hot == handle ? OverlayRole::HandleHot : OverlayRole::Handle;
		out.Handle( HandlePosition( camera, rect, handle ), kHandleRadiusPixels, shape, role );
	}
}

} // namespace hammer::tools
