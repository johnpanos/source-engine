//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/block_tool.h.
//
//=============================================================================//

#include "hammer/tools/block_tool.h"

#include "hammer/app/ops/create_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace hammer::tools
{

using mapgeometry::Component;
using mapgeometry::SetComponent;
using mapgeometry::Vec3d;
using viewport::PlanePoint;
using viewport::PlaneRect;

namespace
{

bool HasVolume( const scene::Box &box )
{
	return box.maxs.x > box.mins.x && box.maxs.y > box.mins.y && box.maxs.z > box.mins.z;
}

int DominantAxis( const Vec3d &v )
{
	const double ax = std::fabs( v.x );
	const double ay = std::fabs( v.y );
	const double az = std::fabs( v.z );
	if ( az >= ax && az >= ay )
		return 2;
	return ax >= ay ? 0 : 1;
}

Vec3d UnitAxis( int axis )
{
	Vec3d e;
	SetComponent( e, axis, 1.0 );
	return e;
}

// The ray's point on the plane x[axis] = coord; nothing when parallel or behind.
std::optional<Vec3d> OnPlane( const viewport::Ray &ray, int axis, double coord )
{
	const double d = Component( ray.direction, axis );
	if ( std::fabs( d ) < 1.0e-9 )
		return std::nullopt;
	const double t = ( coord - Component( ray.origin, axis ) ) / d;
	if ( t <= 0.0 )
		return std::nullopt;
	Vec3d p = ray.origin + ray.direction * t;
	SetComponent( p, axis, coord );
	return p;
}

// The coordinate on 'axis' of the point of the line through 'through' along
// that axis closest to the ray; nothing when the ray runs along the axis.
std::optional<double> AxisParameter( const viewport::Ray &ray, const Vec3d &through, int axis )
{
	const Vec3d e = UnitAxis( axis );
	const Vec3d r = through - ray.origin;
	const double b = mapgeometry::Dot( e, ray.direction );
	const double denom = 1.0 - b * b;
	if ( denom < 1.0e-6 )
		return std::nullopt;
	const double s =
	    ( b * mapgeometry::Dot( ray.direction, r ) - mapgeometry::Dot( e, r ) ) / denom;
	return Component( through, axis ) + s;
}

bool RayHitsBox( const viewport::Ray &ray, const scene::Box &box )
{
	double t0 = 0.0;
	double t1 = 1.0e30;
	for ( int axis = 0; axis < 3; ++axis )
	{
		const double o = Component( ray.origin, axis );
		const double d = Component( ray.direction, axis );
		const double lo = Component( box.mins, axis );
		const double hi = Component( box.maxs, axis );
		if ( std::fabs( d ) < 1.0e-12 )
		{
			if ( o < lo || o > hi )
				return false;
			continue;
		}
		double a = ( lo - o ) / d;
		double c = ( hi - o ) / d;
		if ( a > c )
			std::swap( a, c );
		t0 = std::max( t0, a );
		t1 = std::min( t1, c );
		if ( t0 > t1 )
			return false;
	}
	return true;
}

std::string SizeText( const scene::Box &box )
{
	const Vec3d size = box.Size();
	char buffer[96];
	std::snprintf( buffer, sizeof( buffer ), "%g x %g x %g", size.x, size.y, size.z );
	return buffer;
}

} // namespace

void BlockTool::DepthExtent( const ToolContext &ctx, int axis, double &lo, double &hi ) const
{
	std::optional<scene::Box> source;
	if ( m_pending )
		source = m_pending->box;
	else if ( m_last )
		source = m_last;
	else if ( !ctx.session.CurrentSelection().objects.empty() )
		source = scene::ObjectsBounds( ctx.session.Document(),
		    ctx.session.CurrentSelection().objects, ctx.tool.pointHalfSize );
	if ( source && Component( source->maxs, axis ) > Component( source->mins, axis ) )
	{
		lo = Component( source->mins, axis );
		hi = Component( source->maxs, axis );
		return;
	}
	lo = 0.0;
	hi = static_cast<double>( ctx.grid.Size() );
}

void BlockTool::SetStatusFor( const std::optional<PendingBox> &box )
{
	if ( !box )
		m_status.clear();
	else if ( HasVolume( box->box ) )
		m_status = "Block " + SizeText( box->box );
	else
		m_status = "Block (flat)";
}

ToolResult BlockTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( event.phase == PointerPhase::Down )
	{
		if ( event.button != PointerButton::Left || m_gesture )
			return ToolResult::Ignore();
		Gesture g;
		g.view = event.view;
		g.down = g.current = event.Point();
		g.modifiers = event.modifiers;
		g.before = g.preview = m_pending;
		if ( ctx.view.Is2D() )
		{
			const viewport::Camera2D &camera = *ctx.view.camera2D;
			const PlanePoint at = camera.ScreenToPlane( event.x, event.y );
			std::optional<BoxHandle> handle;
			if ( m_pending )
				handle = HitHandle( camera, ProjectBox( camera, m_pending->box ),
				    BoxHandleMode::Scale, event.x, event.y );
			if ( handle )
			{
				g.kind = Kind::Resize;
				g.handle = *handle;
			}
			else if ( m_pending && RectContains( ProjectBox( camera, m_pending->box ), at ) )
			{
				g.kind = Kind::MoveBox;
			}
			else
			{
				g.kind = Kind::NewBox;
				g.start = SnapPlanePoint( ctx.grid, at, event.modifiers );
			}
		}
		else
		{
			const std::optional<viewport::Ray> ray =
			    ctx.view.camera3D->RayThroughPixel( event.x, event.y );
			if ( !ray )
				return ToolResult::Fail( "no ray under the pointer" );
			std::optional<double> param;
			if ( m_pending && RayHitsBox( *ray, m_pending->box ) )
				param = AxisParameter( *ray, m_pending->box.Center(), m_pending->axis );
			if ( param )
			{
				g.kind = Kind::Height;
				g.lineDown = *param;
			}
			else
			{
				g.kind = Kind::Base;
				PickSettings picking = ctx.Picking();
				picking.entities = false;
				const std::optional<ObjectPick> pick = PickObject3D(
				    ctx.session.Document(), *ctx.view.camera3D, event.x, event.y, picking );
				if ( pick )
				{
					g.planeAxis = DominantAxis( pick->normal );
					g.planeSign = Component( pick->normal, g.planeAxis ) >= 0.0 ? 1.0 : -1.0;
					g.planeCoord = SnapValue(
					    ctx.grid, Component( pick->point, g.planeAxis ), event.modifiers );
				}
				const std::optional<Vec3d> p = OnPlane( *ray, g.planeAxis, g.planeCoord );
				if ( !p )
					return ToolResult::Fail( "no workplane under the pointer" );
				viewport::AxisMask mask;
				( g.planeAxis == 0 ? mask.x : ( g.planeAxis == 1 ? mask.y : mask.z ) ) = false;
				g.baseStart = SnapWorldPoint( ctx.grid, *p, event.modifiers, mask );
			}
		}
		m_gesture = g;
		return ToolResult::Capture();
	}
	if ( !m_gesture )
		return ToolResult::Ignore();
	Gesture &g = *m_gesture;
	if ( event.phase == PointerPhase::Up && event.button != PointerButton::Left )
		return ToolResult::Ignore();
	g.current = event.Point();
	g.modifiers = event.modifiers;
	if ( !g.dragged && ExceedsDragThreshold( g.down, g.current ) )
		g.dragged = true;
	if ( g.dragged )
		g.preview = ctx.view.Is2D() ? Update2D( ctx, g ) : Update3D( ctx, g );
	SetStatusFor( g.preview );
	if ( event.phase == PointerPhase::Move )
		return ToolResult::Handle();

	// Up
	const Gesture done = g;
	m_gesture.reset();
	if ( done.dragged && done.preview && HasVolume( done.preview->box ) )
		m_pending = done.preview;
	SetStatusFor( m_pending );
	return ToolResult::Release();
}

std::optional<BlockTool::PendingBox> BlockTool::Update2D(
    const ToolContext &ctx, const Gesture &g ) const
{
	const viewport::Camera2D &camera = *ctx.view.camera2D;
	const viewport::ViewAxes axes = camera.Axes();
	const PlanePoint down = camera.ScreenToPlane( g.down.x, g.down.y );
	const PlanePoint now = camera.ScreenToPlane( g.current.x, g.current.y );
	const PlanePoint raw{ now.u - down.u, now.v - down.v };
	if ( g.kind == Kind::NewBox )
	{
		const PlanePoint end = SnapPlanePoint( ctx.grid, now, g.modifiers );
		const PlaneRect rect{ { std::min( g.start.u, end.u ), std::min( g.start.v, end.v ) },
		    { std::max( g.start.u, end.u ), std::max( g.start.v, end.v ) } };
		double lo = 0.0;
		double hi = 0.0;
		DepthExtent( ctx, axes.depth, lo, hi );
		PendingBox box;
		SetComponent( box.box.mins, axes.depth, lo );
		SetComponent( box.box.maxs, axes.depth, hi );
		box.box = WithPlaneRect( box.box, axes, rect );
		box.axis = axes.depth;
		box.baseAtMin = true;
		return box;
	}
	if ( !g.before )
		return std::nullopt;
	PendingBox box = *g.before;
	const PlaneRect start = ProjectBox( camera, g.before->box );
	if ( g.kind == Kind::Resize )
	{
		box.box = WithPlaneRect(
		    box.box, axes, ResizeRect( ctx.grid, start, g.handle, raw, g.modifiers ) );
		return box;
	}
	const Vec3d delta =
	    PlaneDeltaToWorld( axes, DragDelta( ctx.grid, start.min, raw, g.modifiers ) );
	box.box.mins += delta;
	box.box.maxs += delta;
	return box;
}

std::optional<BlockTool::PendingBox> BlockTool::Update3D(
    const ToolContext &ctx, const Gesture &g ) const
{
	const std::optional<viewport::Ray> ray =
	    ctx.view.camera3D->RayThroughPixel( g.current.x, g.current.y );
	if ( !ray )
		return g.preview;
	if ( g.kind == Kind::Base )
	{
		const std::optional<Vec3d> p = OnPlane( *ray, g.planeAxis, g.planeCoord );
		if ( !p )
			return g.preview;
		viewport::AxisMask mask;
		( g.planeAxis == 0 ? mask.x : ( g.planeAxis == 1 ? mask.y : mask.z ) ) = false;
		const Vec3d end = SnapWorldPoint( ctx.grid, *p, g.modifiers, mask );
		PendingBox box;
		for ( int axis = 0; axis < 3; ++axis )
		{
			if ( axis == g.planeAxis )
				continue;
			SetComponent( box.box.mins, axis,
			    std::min( Component( g.baseStart, axis ), Component( end, axis ) ) );
			SetComponent( box.box.maxs, axis,
			    std::max( Component( g.baseStart, axis ), Component( end, axis ) ) );
		}
		const double height = static_cast<double>( ctx.grid.Size() );
		SetComponent(
		    box.box.mins, g.planeAxis, g.planeSign > 0 ? g.planeCoord : g.planeCoord - height );
		SetComponent(
		    box.box.maxs, g.planeAxis, g.planeSign > 0 ? g.planeCoord + height : g.planeCoord );
		box.axis = g.planeAxis;
		box.baseAtMin = g.planeSign > 0;
		return box;
	}
	// Height
	if ( !g.before )
		return std::nullopt;
	PendingBox box = *g.before;
	const std::optional<double> param = AxisParameter( *ray, g.before->box.Center(), box.axis );
	if ( !param )
		return g.preview;
	const double base =
	    box.baseAtMin ? Component( box.box.mins, box.axis ) : Component( box.box.maxs, box.axis );
	const double far =
	    box.baseAtMin ? Component( box.box.maxs, box.axis ) : Component( box.box.mins, box.axis );
	const double moved = SnapValue( ctx.grid, far + ( *param - g.lineDown ), g.modifiers );
	SetComponent( box.box.mins, box.axis, std::min( base, moved ) );
	SetComponent( box.box.maxs, box.axis, std::max( base, moved ) );
	box.baseAtMin = moved >= base;
	return box;
}

ToolResult BlockTool::Commit( ToolContext &ctx )
{
	if ( !m_pending )
		return ToolResult::Ignore();
	const PendingBox pending = *m_pending;
	const scene::FaceTexture texture = ctx.editor.faceTexture;
	std::string label;
	app::EditSession::SelectingOperation operation;
	if ( ctx.tool.makeArch )
	{
		const app::ops::ArchSpec arch = ctx.editor.arch;
		label = "Create arch";
		operation = [arch, pending, texture]( scene::DocumentEdit &edit, app::Selection &after )
		{
			scene::ObjectId created;
			const app::EditResult result =
			    app::ops::CreateArch( edit, arch, pending.box, texture, created );
			if ( result )
				after = app::CombineObjects( {}, { created }, app::SelectMode::Replace );
			return result;
		};
	}
	else
	{
		app::ops::PrimitiveSpec spec = ctx.editor.primitive;
		spec.axis = pending.axis;
		label = spec.kind == app::ops::PrimitiveKind::Block ? "Create block" : "Create primitive";
		operation = [spec, pending, texture]( scene::DocumentEdit &edit, app::Selection &after )
		{
			scene::ObjectId created;
			const app::EditResult result =
			    app::ops::CreatePrimitive( edit, spec, pending.box, texture, created );
			if ( result )
				after = app::CombineObjects( {}, { created }, app::SelectMode::Replace );
			return result;
		};
	}
	const auto result = ctx.session.ExecuteSelecting( label, operation );
	if ( !result )
		return ToolResult::Fail( result.Error().message );
	m_last = pending.box;
	m_pending.reset();
	m_status = label;
	return ToolResult::Handle();
}

ToolResult BlockTool::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	if ( !event.IsPress() )
		return ToolResult::Ignore();
	if ( m_gesture )
	{
		if ( event.key != Key::Escape )
			return ToolResult::Ignore();
		Cancel( CancelReason::Escape );
		return ToolResult::Release();
	}
	if ( event.key == Key::Enter )
		return Commit( ctx );
	if ( event.key == Key::Escape && m_pending )
	{
		m_last = m_pending->box;
		m_pending.reset();
		m_status = "Block discarded";
		return ToolResult::Handle();
	}
	return ToolResult::Ignore();
}

void BlockTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
		m_status = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	m_gesture.reset();
}

void BlockTool::Deactivate()
{
	Cancel( CancelReason::ToolSwitched );
	if ( m_pending )
		m_last = m_pending->box;
	m_pending.reset();
}

OverlayList BlockTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	const std::optional<PendingBox> &box = m_gesture ? m_gesture->preview : m_pending;
	if ( !box )
		return out;
	const bool valid = HasVolume( box->box );
	out.Box( box->box.mins, box->box.maxs, valid ? OverlayRole::Pending : OverlayRole::Error );
	if ( ctx.view.Is2D() && valid )
	{
		const viewport::Camera2D &camera = *ctx.view.camera2D;
		const PlaneRect rect = ProjectBox( camera, box->box );
		if ( !m_gesture || m_gesture->kind == Kind::Resize || m_gesture->kind == Kind::MoveBox )
		{
			std::optional<BoxHandle> hot;
			if ( m_gesture && m_gesture->kind == Kind::Resize )
				hot = m_gesture->handle;
			AppendHandles( out, camera, rect, BoxHandleMode::Scale, hot );
		}
		out.Label( camera.PlaneToScreen( { rect.min.u, rect.max.v } ), SizeText( box->box ),
		    OverlayRole::Pending );
	}
	else if ( ctx.view.Is3D() && valid )
	{
		if ( const std::optional<viewport::ScreenPoint> at = ctx.view.Project( box->box.Center() ) )
			out.Label( *at, SizeText( box->box ), OverlayRole::Pending );
	}
	return out;
}

Cursor BlockTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( m_gesture )
	{
		switch ( m_gesture->kind )
		{
		case Kind::Resize:
			return HandleCursor( m_gesture->handle, BoxHandleMode::Scale );
		case Kind::MoveBox:
			return Cursor::Move;
		case Kind::Height:
			return Cursor::ResizeN;
		case Kind::NewBox:
		case Kind::Base:
			break;
		}
		return Cursor::Crosshair;
	}
	if ( m_pending && ctx.view.Is2D() )
	{
		const viewport::Camera2D &camera = *ctx.view.camera2D;
		const PlaneRect rect = ProjectBox( camera, m_pending->box );
		if ( const std::optional<BoxHandle> handle =
		         HitHandle( camera, rect, BoxHandleMode::Scale, x, y ) )
			return HandleCursor( *handle, BoxHandleMode::Scale );
		if ( RectContains( rect, camera.ScreenToPlane( x, y ) ) )
			return Cursor::Move;
	}
	return Cursor::Crosshair;
}

} // namespace hammer::tools
