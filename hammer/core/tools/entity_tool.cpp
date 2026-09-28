//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/entity_tool.h.
//
//=============================================================================//

#include "hammer/tools/entity_tool.h"

#include "hammer/app/ops/create_ops.h"
#include "hammer/viewport/view_policy.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>

namespace hammer::tools
{

using mapgeometry::Vec3d;

namespace
{

int DominantAxis( const Vec3d &v )
{
	const double ax = std::fabs( v.x );
	const double ay = std::fabs( v.y );
	const double az = std::fabs( v.z );
	if ( az >= ax && az >= ay )
		return 2;
	return ax >= ay ? 0 : 1;
}

std::optional<scene::Box> MarkerAt( const ToolContext &ctx, const Vec3d &origin )
{
	scene::Entity entity;
	entity.classname = ctx.editor.entityClass;
	entity.SetOrigin( origin );
	return viewport::EntityMarkerBox( entity, ctx.catalog, ctx.tool.pointHalfSize );
}

} // namespace

std::optional<Vec3d> EntityTool::PlacementAt(
    const ToolContext &ctx, double x, double y, Modifiers modifiers, std::string *why ) const
{
	const auto fail = [why]( const char *reason ) -> std::optional<Vec3d>
	{
		if ( why )
			*why = reason;
		return std::nullopt;
	};
	if ( ctx.view.Is2D() )
	{
		const viewport::Camera2D &camera = *ctx.view.camera2D;
		const viewport::PlanePoint plane =
		    SnapPlanePoint( ctx.grid, camera.ScreenToPlane( x, y ), modifiers );
		const double depth =
		    m_last3D ? mapgeometry::Component( *m_last3D, camera.Axes().depth ) : 0.0;
		return camera.PlaneToWorld( plane, depth );
	}
	if ( !ctx.view.Is3D() )
		return fail( "no view" );
	PickSettings picking = ctx.Picking();
	picking.entities = false;
	const std::optional<ObjectPick> pick =
	    PickObject3D( ctx.session.Document(), *ctx.view.camera3D, x, y, picking );
	if ( !pick || !pick->hasPoint )
		return fail( "no surface under the pointer" );
	const int axis = DominantAxis( pick->normal );
	viewport::AxisMask mask;
	( axis == 0 ? mask.x : ( axis == 1 ? mask.y : mask.z ) ) = false;
	const Vec3d point = SnapWorldPoint( ctx.grid, pick->point, modifiers, mask );
	// Push the marker out along the normal until its box touches the surface.
	const std::optional<scene::Box> box = MarkerAt( ctx, Vec3d() );
	double offset = 0.0;
	if ( box )
	{
		for ( int corner = 0; corner < 8; ++corner )
		{
			const Vec3d c( corner & 1 ? box->maxs.x : box->mins.x,
			    corner & 2 ? box->maxs.y : box->mins.y, corner & 4 ? box->maxs.z : box->mins.z );
			offset = std::max( offset, -mapgeometry::Dot( pick->normal, c ) );
		}
	}
	return point + pick->normal * offset;
}

ToolResult EntityTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( event.phase == PointerPhase::Down )
	{
		if ( event.button != PointerButton::Left || m_gesture )
			return ToolResult::Ignore();
		Gesture g;
		g.view = event.view;
		g.down = g.current = event.Point();
		g.modifiers = event.modifiers;
		g.preview = PlacementAt( ctx, event.x, event.y, event.modifiers );
		m_gesture = g;
		return ToolResult::Capture();
	}
	if ( event.phase == PointerPhase::Move )
	{
		if ( !m_gesture )
		{
			Hover hover{ event.view, PlacementAt( ctx, event.x, event.y, event.modifiers ) };
			const bool changed = !m_hover || m_hover->view != hover.view || m_hover->at != hover.at;
			m_hover = hover;
			return changed ? ToolResult::Handle() : ToolResult::Ignore();
		}
		Gesture &g = *m_gesture;
		g.current = event.Point();
		g.modifiers = event.modifiers;
		if ( !g.dragged && ExceedsDragThreshold( g.down, g.current ) )
			g.dragged = true;
		if ( g.dragged )
			g.preview = PlacementAt( ctx, g.current.x, g.current.y, g.modifiers );
		return ToolResult::Handle();
	}
	if ( !m_gesture || event.button != PointerButton::Left )
		return ToolResult::Ignore();
	Gesture g = *m_gesture;
	m_gesture.reset();
	g.current = event.Point();
	if ( !g.dragged && ExceedsDragThreshold( g.down, g.current ) )
		g.dragged = true;
	const viewport::ScreenPoint at = g.dragged ? g.current : g.down;
	const std::string classname = ctx.editor.entityClass;
	if ( classname.empty() )
		return ToolResult::Fail( "no entity class selected" );
	std::string why;
	const std::optional<Vec3d> origin = PlacementAt( ctx, at.x, at.y, event.modifiers, &why );
	if ( !origin )
		return ToolResult::Fail( why );
	const Vec3d place = *origin;
	const ports::IEntityCatalog *catalog = ctx.catalog;
	const auto result = ctx.session.ExecuteSelecting( "Place entity",
	    [&classname, place, catalog]( scene::DocumentEdit &edit, app::Selection &after )
	    {
		    scene::ObjectId created;
		    const app::EditResult placed =
		        app::ops::PlaceEntity( edit, classname, place, catalog, created );
		    if ( placed )
			    after = app::CombineObjects( {}, { created }, app::SelectMode::Replace );
		    return placed;
	    } );
	if ( !result )
		return ToolResult::Fail( result.Error().message );
	if ( ctx.view.Is3D() )
		m_last3D = place;
	m_status = "Placed " + classname;
	return ToolResult::Release();
}

ToolResult EntityTool::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	(void)ctx;
	if ( !event.IsPress() || event.key != Key::Escape || !m_gesture )
		return ToolResult::Ignore();
	Cancel( CancelReason::Escape );
	return ToolResult::Release();
}

void EntityTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
		m_status = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	m_gesture.reset();
}

void EntityTool::AppendMarker(
    OverlayList &out, const ToolContext &ctx, const Vec3d &origin, OverlayRole role ) const
{
	if ( const std::optional<scene::Box> box = MarkerAt( ctx, origin ) )
		out.Box( box->mins, box->maxs, role );
}

OverlayList EntityTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	if ( m_gesture )
	{
		if ( m_gesture->preview )
			AppendMarker( out, ctx, *m_gesture->preview, OverlayRole::Pending );
	}
	else if ( m_hover && m_hover->at && m_hover->view == ctx.view.kind )
	{
		AppendMarker( out, ctx, *m_hover->at, OverlayRole::Hover );
	}
	return out;
}

Cursor EntityTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( ctx.view.Is3D() && !m_gesture && !PlacementAt( ctx, x, y, {} ) )
		return Cursor::Forbidden;
	return Cursor::Crosshair;
}

} // namespace hammer::tools
