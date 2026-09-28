//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/clip_tool.h.
//
//=============================================================================//

#include "hammer/tools/clip_tool.h"

#include "hammer/scene/map_queries.h"
#include "hammer/tools/preview.h"
#include "mapgeometry/vec3.h"

namespace hammer::tools
{

using mapgeometry::Vec3d;
using viewport::PlanePoint;

namespace
{

const char *KeepName( app::ops::ClipKeep keep )
{
	switch ( keep )
	{
	case app::ops::ClipKeep::Front:
		return "front";
	case app::ops::ClipKeep::Back:
		return "back";
	case app::ops::ClipKeep::Both:
		return "both";
	}
	return "?";
}

Vec3d ToWorld( viewport::ViewKind view, PlanePoint p )
{
	return PlaneDeltaToWorld( viewport::AxesOf( view ), p );
}

std::vector<scene::ObjectId> SelectedSolids( const ToolContext &ctx )
{
	std::vector<scene::ObjectId> solids;
	const scene::MapDocument &doc = ctx.session.Document();
	for ( scene::ObjectId id :
	    scene::ExpandToLeaves( doc, ctx.session.CurrentSelection().objects ) )
		if ( doc.FindSolid( id ) )
			solids.push_back( id );
	return solids;
}

} // namespace

std::optional<mapgeometry::Plane> ClipTool::PlaneOf( const ClipLine &line )
{
	const Vec3d a = ToWorld( line.view, line.a );
	const Vec3d b = ToWorld( line.view, line.b );
	const Vec3d normal =
	    mapgeometry::Normalize( mapgeometry::Cross( b - a, viewport::ViewDirection( line.view ) ) );
	if ( normal == Vec3d() )
		return std::nullopt;
	return mapgeometry::Plane{ normal, mapgeometry::Dot( normal, a ) };
}

std::optional<int> ClipTool::PointUnder( const ToolContext &ctx, double x, double y ) const
{
	if ( !m_line || !ctx.view.Is2D() || m_line->view != ctx.view.kind )
		return std::nullopt;
	const viewport::Camera2D &camera = *ctx.view.camera2D;
	const double da = HandleDistance( camera.PlaneToScreen( m_line->a ), { x, y } );
	const double db = HandleDistance( camera.PlaneToScreen( m_line->b ), { x, y } );
	if ( da <= kHandleRadiusPixels && da <= db )
		return 0;
	if ( db <= kHandleRadiusPixels )
		return 1;
	return std::nullopt;
}

ToolResult ClipTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( !ctx.view.Is2D() )
		return ToolResult::Ignore();
	const viewport::Camera2D &camera = *ctx.view.camera2D;
	if ( event.phase == PointerPhase::Down )
	{
		if ( event.button != PointerButton::Left || m_gesture )
			return ToolResult::Ignore();
		Gesture g;
		g.view = event.view;
		g.down = event.Point();
		g.before = m_line;
		if ( const std::optional<int> point = PointUnder( ctx, event.x, event.y ) )
		{
			g.point = *point;
			g.line = *m_line;
		}
		else
		{
			const PlanePoint start = SnapPlanePoint(
			    ctx.grid, camera.ScreenToPlane( event.x, event.y ), event.modifiers );
			g.line = { event.view, start, start };
		}
		m_gesture = g;
		return ToolResult::Capture();
	}
	if ( !m_gesture || ( event.phase == PointerPhase::Up && event.button != PointerButton::Left ) )
		return ToolResult::Ignore();
	Gesture &g = *m_gesture;
	if ( !g.dragged && ExceedsDragThreshold( g.down, event.Point() ) )
		g.dragged = true;
	if ( g.dragged )
	{
		const PlanePoint p =
		    SnapPlanePoint( ctx.grid, camera.ScreenToPlane( event.x, event.y ), event.modifiers );
		( g.point == 0 ? g.line.a : g.line.b ) = p;
	}
	if ( event.phase == PointerPhase::Move )
		return ToolResult::Handle();
	const Gesture done = g;
	m_gesture.reset();
	if ( done.dragged && !( done.line.a == done.line.b ) )
		m_line = done.line;
	m_message.clear();
	return ToolResult::Release();
}

ToolResult ClipTool::Apply( ToolContext &ctx )
{
	if ( !m_line )
		return ToolResult::Ignore();
	const std::optional<mapgeometry::Plane> plane = PlaneOf( *m_line );
	if ( !plane )
		return ToolResult::Fail( "the clip line has no length" );
	const std::vector<scene::ObjectId> solids = SelectedSolids( ctx );
	if ( solids.empty() )
		return ToolResult::Fail( "no solids selected" );
	const scene::FaceTexture cap = ctx.editor.faceTexture;
	bool crosses = false;
	for ( scene::ObjectId id : solids )
	{
		const app::ops::SplitResult split =
		    app::ops::SplitSolid( *ctx.session.Document().FindSolid( id ), *plane, cap );
		crosses = crosses || ( split.back && split.front );
	}
	if ( !crosses )
		return ToolResult::Fail( "the clip plane crosses no selected solid" );
	const std::vector<scene::ObjectId> ids = ctx.session.CurrentSelection().objects;
	const app::Selection current = ctx.session.CurrentSelection();
	const app::ops::ClipKeep keep = m_keep;
	const mapgeometry::Plane clip = *plane;
	const auto result = ctx.session.ExecuteSelecting( "Clip",
	    [&]( scene::DocumentEdit &edit, app::Selection &after )
	    {
		    std::vector<scene::ObjectId> created;
		    const app::EditResult clipped =
		        app::ops::ClipSolids( edit, ids, clip, keep, cap, &created );
		    if ( clipped )
			    after = app::CombineObjects( current, created, app::SelectMode::Add );
		    return clipped;
	    } );
	if ( !result )
		return ToolResult::Fail( result.Error().message );
	m_line.reset();
	m_message = "Clipped";
	return ToolResult::Handle();
}

ToolResult ClipTool::OnKey( ToolContext &ctx, const KeyEvent &event )
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
	if ( event.IsChar( 'x' ) && event.modifiers.Shift() )
	{
		m_keep = m_keep == app::ops::ClipKeep::Front
		             ? app::ops::ClipKeep::Back
		             : ( m_keep == app::ops::ClipKeep::Back ? app::ops::ClipKeep::Both
		                                                    : app::ops::ClipKeep::Front );
		return ToolResult::Handle();
	}
	if ( event.key == Key::Enter )
		return Apply( ctx );
	if ( event.key == Key::Escape && m_line )
	{
		m_line.reset();
		return ToolResult::Handle();
	}
	return ToolResult::Ignore();
}

void ClipTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
		m_message = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	m_gesture.reset();
}

void ClipTool::Deactivate()
{
	Cancel( CancelReason::ToolSwitched );
	m_line.reset();
}

OverlayList ClipTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	const std::optional<ClipLine> line =
	    m_gesture ? std::optional<ClipLine>( m_gesture->line ) : m_line;
	if ( !line )
		return out;
	const std::optional<mapgeometry::Plane> plane = PlaneOf( *line );
	if ( plane && ( !m_gesture || m_gesture->dragged ) )
	{
		const scene::MapDocument &doc = ctx.session.Document();
		for ( scene::ObjectId id : SelectedSolids( ctx ) )
		{
			const app::ops::SplitResult split =
			    app::ops::SplitSolid( *doc.FindSolid( id ), *plane, ctx.editor.faceTexture );
			if ( !split.back || !split.front )
				continue;
			if ( m_keep != app::ops::ClipKeep::Back )
				AppendSolidEdges( out, *split.front, OverlayRole::Clip );
			if ( m_keep != app::ops::ClipKeep::Front )
				AppendSolidEdges( out, *split.back, OverlayRole::Clip );
		}
	}
	if ( ctx.view.Is2D() && ctx.view.kind == line->view )
	{
		const viewport::Camera2D &camera = *ctx.view.camera2D;
		out.Line(
		    ToWorld( line->view, line->a ), ToWorld( line->view, line->b ), OverlayRole::Clip );
		for ( int i = 0; i < 2; ++i )
		{
			const bool hot = m_gesture && m_gesture->point == i;
			out.Handle( camera.PlaneToScreen( i == 0 ? line->a : line->b ), kHandleRadiusPixels,
			    HandleShape::Square, hot ? OverlayRole::HandleHot : OverlayRole::Handle );
		}
	}
	return out;
}

Cursor ClipTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( !ctx.view.Is2D() )
		return Cursor::Forbidden;
	if ( ( m_gesture && m_gesture->point >= 0 ) || PointUnder( ctx, x, y ) )
		return Cursor::Move;
	return Cursor::Crosshair;
}

std::string ClipTool::Status() const
{
	std::string status = std::string( "Clip: keep " ) + KeepName( m_keep );
	if ( !m_message.empty() )
		status += " -- " + m_message;
	return status;
}

} // namespace hammer::tools
