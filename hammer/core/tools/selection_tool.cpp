//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/selection_tool.h.
//
//=============================================================================//

#include "hammer/tools/selection_tool.h"

#include "hammer/app/ops/structure_ops.h"
#include "hammer/app/ops/transform_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/tools/preview.h"
#include "mapgeometry/vec3.h"

#include <cmath>
#include <cstdio>
#include <numbers>

namespace hammer::tools
{

using mapgeometry::Vec3d;
using viewport::PlanePoint;
using viewport::ScreenPoint;

namespace
{

std::string Format( const char *format, double a, double b = 0.0, double c = 0.0 )
{
	char buffer[128];
	std::snprintf( buffer, sizeof( buffer ), format, a, b, c );
	return buffer;
}

// Degrees of the counter-clockwise angle from 'from' to 'to' about 'center'
// in view-plane coordinates (v up), in (-180, 180].
double PlaneAngle( PlanePoint center, PlanePoint from, PlanePoint to )
{
	const double a0 = std::atan2( from.v - center.v, from.u - center.u );
	const double a1 = std::atan2( to.v - center.v, to.u - center.u );
	double degrees = ( a1 - a0 ) * 180.0 / std::numbers::pi;
	while ( degrees > 180.0 )
		degrees -= 360.0;
	while ( degrees <= -180.0 )
		degrees += 360.0;
	return degrees;
}

// +1 when the view's u x v is its +depth axis, -1 otherwise.
double DepthSign( viewport::ViewAxes axes )
{
	Vec3d u;
	Vec3d v;
	mapgeometry::SetComponent( u, axes.u, 1.0 );
	mapgeometry::SetComponent( v, axes.v, 1.0 );
	return mapgeometry::Component( mapgeometry::Cross( u, v ), axes.depth ) >= 0.0 ? 1.0 : -1.0;
}

} // namespace

BoxHandleMode SelectionTool::HandleMode( const app::Selection &selection ) const
{
	if ( selection.objects.empty() || selection.objects != m_modeObjects )
		return BoxHandleMode::Scale;
	return m_mode;
}

void SelectionTool::SetMode( BoxHandleMode mode, const app::Selection &selection )
{
	m_mode = mode;
	m_modeObjects = selection.objects;
}

std::optional<BoxHandle> SelectionTool::HandleUnder(
    const ToolContext &ctx, double x, double y ) const
{
	const app::Selection &selection = ctx.session.CurrentSelection();
	if ( !ctx.view.Is2D() || selection.objects.empty() )
		return std::nullopt;
	const std::optional<scene::Box> bounds =
	    scene::ObjectsBounds( ctx.session.Document(), selection.objects, ctx.tool.pointHalfSize );
	if ( !bounds )
		return std::nullopt;
	return HitHandle( *ctx.view.camera2D, ProjectBox( *ctx.view.camera2D, *bounds ),
	    HandleMode( selection ), x, y );
}

ToolResult SelectionTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( event.phase == PointerPhase::Down )
	{
		if ( event.button != PointerButton::Left || m_gesture )
			return ToolResult::Ignore();
		return OnDown( ctx, event );
	}
	if ( event.phase == PointerPhase::Move )
	{
		if ( !m_gesture )
		{
			// Hover: track the hot handle.
			const std::optional<BoxHandle> hot = HandleUnder( ctx, event.x, event.y );
			const bool changed = hot != m_hot || ( hot && m_hotView != event.view );
			m_hot = hot;
			m_hotView = event.view;
			return changed ? ToolResult::Handle() : ToolResult::Ignore();
		}
		Gesture &g = *m_gesture;
		g.current = event.Point();
		g.modifiers = event.modifiers;
		if ( !g.dragged && ExceedsDragThreshold( g.down, g.current ) )
		{
			g.dragged = true;
			if ( g.view != viewport::ViewKind::Camera3D )
			{
				if ( g.press == Press::Handle )
					g.drag = g.handleMode == BoxHandleMode::Rotate ? Drag::Rotate : Drag::Scale;
				else if ( g.press == Press::Object && !g.pressModifiers.Ctrl() )
					g.drag = Drag::Move;
				else
					g.drag = Drag::Marquee;
			}
		}
		g.edit = EditFor( ctx, g );
		m_status = g.edit ? g.edit->label : std::string();
		return ToolResult::Handle();
	}
	// Up
	if ( !m_gesture || event.button != PointerButton::Left )
		return ToolResult::Ignore();
	m_gesture->current = event.Point();
	m_gesture->modifiers = event.modifiers;
	return OnUp( ctx );
}

ToolResult SelectionTool::OnDown( ToolContext &ctx, const PointerEvent &event )
{
	const app::Selection &selection = ctx.session.CurrentSelection();
	const scene::MapDocument &doc = ctx.session.Document();
	Gesture g;
	g.view = event.view;
	g.down = g.current = event.Point();
	g.pressModifiers = g.modifiers = event.modifiers;
	g.handleMode = HandleMode( selection );

	if ( const std::optional<BoxHandle> handle = HandleUnder( ctx, event.x, event.y ) )
	{
		g.press = Press::Handle;
		g.handle = *handle;
		g.moving = selection.objects;
	}
	else
	{
		std::optional<ObjectPick> pick;
		if ( ctx.view.Is2D() )
			pick = PickObject2D( doc, *ctx.view.camera2D, event.x, event.y, ctx.Picking() );
		else
			pick = PickObject3D( doc, *ctx.view.camera3D, event.x, event.y, ctx.Picking() );
		if ( pick )
		{
			g.press = Press::Object;
			g.target = pick->target;
			g.targetSelected = selection.Contains( pick->target );
			g.moving =
			    g.targetSelected ? selection.objects : std::vector<scene::ObjectId>{ pick->target };
		}
	}
	if ( !g.moving.empty() )
	{
		if ( const std::optional<scene::Box> bounds =
		         scene::ObjectsBounds( doc, g.moving, ctx.tool.pointHalfSize ) )
		{
			g.bounds = *bounds;
			g.hasBounds = true;
		}
	}
	m_gesture = g;
	m_status.clear();
	return ToolResult::Capture();
}

ToolResult SelectionTool::Click( ToolContext &ctx, const Gesture &g )
{
	const app::Selection &selection = ctx.session.CurrentSelection();
	if ( g.press == Press::Object )
	{
		if ( g.pressModifiers.Ctrl() )
			return ResultOf( ctx.session.SelectObjects( { g.target }, app::SelectMode::Toggle ),
			    ToolResult::Release() );
		if ( g.targetSelected )
		{
			// Legacy: clicking the selection again cycles the handle mode.
			const BoxHandleMode next = HandleMode( selection ) == BoxHandleMode::Scale
			                               ? BoxHandleMode::Rotate
			                               : BoxHandleMode::Scale;
			SetMode( next, selection );
			m_status = next == BoxHandleMode::Rotate ? "Rotate handles" : "Scale handles";
			return ToolResult::Release();
		}
		return ResultOf( ctx.session.SelectObjects( { g.target }, app::SelectMode::Replace ),
		    ToolResult::Release() );
	}
	if ( g.press == Press::Empty && !g.pressModifiers.Ctrl() && !selection.Empty() )
		return ResultOf( ctx.session.ClearSelection(), ToolResult::Release() );
	return ToolResult::Release();
}

ToolResult SelectionTool::OnUp( ToolContext &ctx )
{
	const Gesture g = *m_gesture;
	m_gesture.reset();
	if ( !g.dragged )
		return Click( ctx, g );
	if ( g.drag == Drag::None )
		return ToolResult::Release(); // a 3D drag: no gesture
	if ( g.drag == Drag::Marquee )
	{
		if ( !ctx.view.Is2D() )
			return ToolResult::Release();
		const std::vector<scene::ObjectId> ids = MarqueeTargets( ctx.session.Document(),
		    *ctx.view.camera2D, { g.down, g.current }, ctx.tool.marquee, ctx.Picking() );
		const app::SelectMode mode =
		    g.pressModifiers.Ctrl() ? app::SelectMode::Add : app::SelectMode::Replace;
		m_status = Format( "Selected %.0f", static_cast<double>( ids.size() ) );
		return ResultOf( ctx.session.SelectObjects( ids, mode ), ToolResult::Release() );
	}
	const std::optional<DragEdit> edit = EditFor( ctx, g );
	if ( !edit )
		return ToolResult::Release();
	const auto result = ctx.session.Execute( edit->label, edit->operation, edit->selectionAfter );
	if ( !result )
		return ToolResult::Fail( result.Error().message );
	// Transforming the selection keeps its handle mode (legacy).
	if ( g.press == Press::Handle || g.targetSelected )
		SetMode( g.handleMode, ctx.session.CurrentSelection() );
	m_status = edit->label;
	return ToolResult::Release();
}

std::optional<SelectionTool::DragEdit> SelectionTool::EditFor(
    const ToolContext &ctx, const Gesture &g ) const
{
	if ( !g.hasBounds || g.moving.empty() || !ctx.view.Is2D() || ctx.view.kind != g.view )
		return std::nullopt;
	const viewport::Camera2D &camera = *ctx.view.camera2D;
	const viewport::ViewAxes axes = camera.Axes();
	const PlanePoint down = camera.ScreenToPlane( g.down.x, g.down.y );
	const PlanePoint now = camera.ScreenToPlane( g.current.x, g.current.y );
	const PlanePoint raw{ now.u - down.u, now.v - down.v };
	const app::ops::TransformOptions options{ ctx.editor.textureLock };
	const std::vector<scene::ObjectId> ids = g.moving;

	DragEdit edit;
	if ( g.drag == Drag::Move )
	{
		const PlanePoint delta =
		    DragDelta( ctx.grid, ProjectBox( camera, g.bounds ).min, raw, g.modifiers );
		const Vec3d world = PlaneDeltaToWorld( axes, delta );
		if ( world == Vec3d() )
			return std::nullopt;
		edit.label = "Move";
		edit.operation = [ids, world, options]( scene::DocumentEdit &e )
		{
			return app::ops::Translate( e, ids, world, options );
		};
		if ( !g.targetSelected )
			edit.selectionAfter = app::CombineObjects( {}, { g.target }, app::SelectMode::Replace );
		return edit;
	}
	if ( g.drag == Drag::Scale )
	{
		const viewport::PlaneRect start = ProjectBox( camera, g.bounds );
		const viewport::PlaneRect rect = ResizeRect( ctx.grid, start, g.handle, raw, g.modifiers );
		if ( rect.min == start.min && rect.max == start.max )
			return std::nullopt;
		const scene::Box from = g.bounds;
		const scene::Box to = WithPlaneRect( g.bounds, axes, rect );
		edit.label = "Scale";
		edit.operation = [ids, from, to, options]( scene::DocumentEdit &e )
		{
			return app::ops::ScaleToBox( e, ids, from, to, options );
		};
		return edit;
	}
	if ( g.drag == Drag::Rotate )
	{
		const Vec3d center = g.bounds.Center();
		const double degrees =
		    SnapAngle( PlaneAngle( camera.ToPlane( center ), down, now ), g.modifiers );
		if ( degrees == 0.0 )
			return std::nullopt;
		const int axis = axes.depth;
		const double worldDegrees = DepthSign( axes ) * degrees;
		edit.label = "Rotate";
		edit.operation = [ids, axis, worldDegrees, center, options]( scene::DocumentEdit &e )
		{
			return app::ops::Rotate( e, ids, axis, worldDegrees, center, options );
		};
		return edit;
	}
	return std::nullopt;
}

ToolResult SelectionTool::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	if ( !event.IsPress() )
		return ToolResult::Ignore();
	if ( m_gesture )
	{
		if ( event.key == Key::Escape )
		{
			Cancel( CancelReason::Escape );
			return ToolResult::Release();
		}
		return ToolResult::Ignore();
	}
	const app::Selection &selection = ctx.session.CurrentSelection();
	if ( event.key == Key::Escape )
	{
		if ( selection.Empty() )
			return ToolResult::Ignore();
		return ResultOf( ctx.session.ClearSelection(), ToolResult::Handle() );
	}
	if ( selection.objects.empty() )
		return ToolResult::Ignore();
	const std::vector<scene::ObjectId> ids = selection.objects;
	if ( event.key == Key::Delete )
	{
		m_status = "Delete";
		return ResultOf( ctx.session.Execute( "Delete",
		                     [ids]( scene::DocumentEdit &e )
		                     {
			                     return app::ops::DeleteObjects( e, ids );
		                     } ),
		    ToolResult::Handle() );
	}
	const std::optional<Vec3d> nudge =
	    NudgeDelta( ctx.view.kind, event.key, event.modifiers, ctx.grid, ctx.tool.nudge );
	if ( !nudge )
		return ToolResult::Ignore();
	const Vec3d delta = *nudge;
	const app::ops::TransformOptions options{ ctx.editor.textureLock };
	m_status = Format( "Nudge %g %g %g", delta.x, delta.y, delta.z );
	return ResultOf( ctx.session.Execute( "Nudge",
	                     [ids, delta, options]( scene::DocumentEdit &e )
	                     {
		                     return app::ops::Translate( e, ids, delta, options );
	                     } ),
	    ToolResult::Handle() );
}

void SelectionTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
		m_status = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	m_gesture.reset();
}

OverlayList SelectionTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	const scene::MapDocument &doc = ctx.session.Document();
	const app::Selection &selection = ctx.session.CurrentSelection();
	const std::optional<scene::Box> bounds =
	    selection.objects.empty()
	        ? std::nullopt
	        : scene::ObjectsBounds( doc, selection.objects, ctx.tool.pointHalfSize );
	if ( bounds )
		out.Box( bounds->mins, bounds->maxs, OverlayRole::Selection );

	const bool dragging = m_gesture && m_gesture->drag != Drag::None;
	if ( bounds && ctx.view.Is2D() &&
	     ( !dragging || m_gesture->drag == Drag::Scale || m_gesture->drag == Drag::Rotate ) )
	{
		std::optional<BoxHandle> hot = m_hotView == ctx.view.kind ? m_hot : std::nullopt;
		if ( m_gesture && m_gesture->press == Press::Handle )
			hot = m_gesture->handle;
		AppendHandles( out, *ctx.view.camera2D, ProjectBox( *ctx.view.camera2D, *bounds ),
		    HandleMode( selection ), hot );
	}
	if ( !m_gesture )
		return out;

	const Gesture &g = *m_gesture;
	if ( g.press == Press::Object && !g.dragged && !g.targetSelected )
		AppendObject( out, doc, g.target, OverlayRole::Hover, ctx.catalog, ctx.tool.pointHalfSize );
	if ( g.drag == Drag::Marquee )
	{
		if ( ctx.view.kind == g.view )
			out.Rect( g.down, g.current, OverlayRole::Pending );
		return out;
	}
	// The drag's edit was computed in the gesture's view on the last move;
	// every view shows its preview.
	if ( const std::optional<DragEdit> &edit = g.edit )
	{
		const PreviewOutcome outcome = AppendStagedPreview(
		    out, doc, edit->operation, OverlayRole::Pending, ctx.catalog, ctx.tool.pointHalfSize );
		if ( !outcome.ok && g.hasBounds )
		{
			out.Box( g.bounds.mins, g.bounds.maxs, OverlayRole::Error );
			if ( ctx.view.kind == g.view )
				out.Label( g.current, outcome.error, OverlayRole::Error );
		}
		else if ( ctx.view.kind == g.view )
		{
			out.Label( g.current, edit->label, OverlayRole::Pending );
		}
	}
	return out;
}

Cursor SelectionTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( m_gesture )
	{
		switch ( m_gesture->drag )
		{
		case Drag::Move:
			return Cursor::Move;
		case Drag::Scale:
			return HandleCursor( m_gesture->handle, BoxHandleMode::Scale );
		case Drag::Rotate:
			return Cursor::Rotate;
		case Drag::Marquee:
			return Cursor::Crosshair;
		case Drag::None:
			break;
		}
		return Cursor::Default;
	}
	if ( const std::optional<BoxHandle> handle = HandleUnder( ctx, x, y ) )
		return HandleCursor( *handle, HandleMode( ctx.session.CurrentSelection() ) );
	if ( ctx.view.Is2D() )
	{
		const std::optional<ObjectPick> pick =
		    PickObject2D( ctx.session.Document(), *ctx.view.camera2D, x, y, ctx.Picking() );
		if ( pick && ctx.session.CurrentSelection().Contains( pick->target ) )
			return Cursor::Move;
	}
	return Cursor::Default;
}

} // namespace hammer::tools
