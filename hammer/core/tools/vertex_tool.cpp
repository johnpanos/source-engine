//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/tools/vertex_tool.h.
//
//=============================================================================//

#include "hammer/tools/vertex_tool.h"

#include "hammer/app/ops/vertex_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/tools/preview.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <map>

namespace hammer::tools
{

using mapgeometry::Vec3d;

namespace
{

constexpr double kStackPixels = 0.5;     // handles this close on screen are one spot
constexpr double kSamePosition = 1.0e-6; // world units, remapping after a move

void SortUnique( std::vector<VertexTool::HandleRef> &handles )
{
	std::sort( handles.begin(), handles.end() );
	handles.erase( std::unique( handles.begin(), handles.end() ), handles.end() );
}

bool Contains( const std::vector<VertexTool::HandleRef> &set, const VertexTool::HandleRef &h )
{
	return std::binary_search( set.begin(), set.end(), h );
}

std::vector<VertexTool::HandleInfo> HandlesOf( scene::ObjectId id, const scene::Solid &solid )
{
	std::vector<VertexTool::HandleInfo> out;
	const std::vector<Vec3d> vertices = app::ops::SolidVertexList( solid );
	for ( std::size_t i = 0; i < vertices.size(); ++i )
		out.push_back( { { id, false, static_cast<int>( i ) }, vertices[i] } );
	const std::vector<std::pair<int, int>> edges = app::ops::SolidEdgeList( solid );
	for ( std::size_t i = 0; i < edges.size(); ++i )
		out.push_back( { { id, true, static_cast<int>( i ) },
		    ( vertices[edges[i].first] + vertices[edges[i].second] ) * 0.5 } );
	return out;
}

} // namespace

std::vector<VertexTool::HandleInfo> VertexTool::Handles( const ToolContext &ctx )
{
	std::vector<HandleInfo> out;
	const scene::MapDocument &doc = ctx.session.Document();
	for ( scene::ObjectId id :
	    scene::ExpandToLeaves( doc, ctx.session.CurrentSelection().objects ) )
	{
		if ( const scene::Solid *solid = doc.FindSolid( id ) )
		{
			const std::vector<HandleInfo> handles = HandlesOf( id, *solid );
			out.insert( out.end(), handles.begin(), handles.end() );
		}
	}
	return out;
}

std::vector<VertexTool::HandleRef> VertexTool::SelectedHandles( const ToolContext &ctx ) const
{
	if ( m_revision != ctx.session.Revision() || m_serial != ctx.session.Document().Serial() )
		return {};
	std::vector<HandleRef> live;
	for ( const HandleInfo &info : Handles( ctx ) )
		if ( Contains( m_selected, info.ref ) )
			live.push_back( info.ref );
	SortUnique( live );
	return live;
}

void VertexTool::Select( const ToolContext &ctx, std::vector<HandleRef> handles )
{
	SortUnique( handles );
	m_selected = std::move( handles );
	m_revision = ctx.session.Revision();
	m_serial = ctx.session.Document().Serial();
}

std::vector<VertexTool::HandleRef> VertexTool::HitHandles(
    const ToolContext &ctx, double x, double y ) const
{
	struct Projected
	{
		HandleRef ref;
		viewport::ScreenPoint at;
	};
	std::vector<Projected> projected;
	for ( const HandleInfo &info : Handles( ctx ) )
		if ( const std::optional<viewport::ScreenPoint> at = ctx.view.Project( info.position ) )
			projected.push_back( { info.ref, *at } );
	const Projected *best = nullptr;
	double bestDistance = 0.0;
	for ( const Projected &p : projected )
	{
		const double distance = HandleDistance( p.at, { x, y } );
		if ( distance > kHandleRadiusPixels )
			continue;
		// Nearest wins; vertices win ties over edge midpoints.
		if ( !best || distance < bestDistance ||
		     ( distance == bestDistance && best->ref.edge && !p.ref.edge ) )
		{
			best = &p;
			bestDistance = distance;
		}
	}
	std::vector<HandleRef> hit;
	if ( !best )
		return hit;
	for ( const Projected &p : projected )
		if ( p.ref.edge == best->ref.edge && HandleDistance( p.at, best->at ) <= kStackPixels )
			hit.push_back( p.ref );
	SortUnique( hit );
	return hit;
}

Vec3d VertexTool::DragDeltaWorld( const ToolContext &ctx, const Gesture &g ) const
{
	if ( !ctx.view.Is2D() || !g.grabbed )
		return Vec3d();
	const viewport::Camera2D &camera = *ctx.view.camera2D;
	const viewport::PlanePoint down = camera.ScreenToPlane( g.down.x, g.down.y );
	const viewport::PlanePoint now = camera.ScreenToPlane( g.current.x, g.current.y );
	const viewport::PlanePoint delta = DragDelta(
	    ctx.grid, camera.ToPlane( *g.grabbed ), { now.u - down.u, now.v - down.v }, g.modifiers );
	return PlaneDeltaToWorld( camera.Axes(), delta );
}

std::vector<std::pair<scene::ObjectId, std::vector<int>>> VertexTool::MovingVertices(
    const ToolContext &ctx ) const
{
	std::map<scene::ObjectId, std::vector<int>> bySolid;
	const scene::MapDocument &doc = ctx.session.Document();
	for ( const HandleRef &h : SelectedHandles( ctx ) )
	{
		const scene::Solid *solid = doc.FindSolid( h.solid );
		if ( !solid )
			continue;
		std::vector<int> &indices = bySolid[h.solid];
		if ( !h.edge )
		{
			indices.push_back( h.index );
			continue;
		}
		const std::vector<std::pair<int, int>> edges = app::ops::SolidEdgeList( *solid );
		indices.push_back( edges[h.index].first );
		indices.push_back( edges[h.index].second );
	}
	std::vector<std::pair<scene::ObjectId, std::vector<int>>> out;
	for ( auto &[id, indices] : bySolid )
	{
		std::sort( indices.begin(), indices.end() );
		indices.erase( std::unique( indices.begin(), indices.end() ), indices.end() );
		out.emplace_back( id, indices );
	}
	return out;
}

ToolResult VertexTool::OnPointer( ToolContext &ctx, const PointerEvent &event )
{
	if ( event.phase == PointerPhase::Down )
	{
		if ( event.button != PointerButton::Left || m_gesture )
			return ToolResult::Ignore();
		Gesture g;
		g.view = event.view;
		g.down = g.current = event.Point();
		g.pressModifiers = g.modifiers = event.modifiers;
		g.before = SelectedHandles( ctx );
		g.hit = HitHandles( ctx, event.x, event.y );
		if ( !g.hit.empty() )
		{
			const bool allSelected = std::all_of( g.hit.begin(), g.hit.end(),
			    [&]( const HandleRef &h )
			    {
				    return Contains( g.before, h );
			    } );
			if ( !allSelected )
			{
				std::vector<HandleRef> next =
				    event.modifiers.Ctrl() ? g.before : std::vector<HandleRef>();
				next.insert( next.end(), g.hit.begin(), g.hit.end() );
				Select( ctx, next );
			}
			for ( const HandleInfo &info : Handles( ctx ) )
				if ( info.ref == g.hit.front() )
					g.grabbed = info.position;
		}
		m_gesture = g;
		return ToolResult::Capture();
	}
	if ( !m_gesture || ( event.phase == PointerPhase::Up && event.button != PointerButton::Left ) )
		return ToolResult::Ignore();
	Gesture &g = *m_gesture;
	g.current = event.Point();
	g.modifiers = event.modifiers;
	if ( !g.dragged && ExceedsDragThreshold( g.down, g.current ) )
		g.dragged = true;
	if ( g.dragged && !g.hit.empty() )
		g.delta = DragDeltaWorld( ctx, g );
	if ( event.phase == PointerPhase::Move )
	{
		if ( g.dragged && g.delta != Vec3d() )
			m_status = "Move vertices " + scene::FormatVec3( g.delta );
		return ToolResult::Handle();
	}

	// Up
	const Gesture done = g;
	m_gesture.reset();
	if ( !done.dragged )
	{
		if ( done.hit.empty() )
		{
			if ( !done.pressModifiers.Ctrl() )
				Select( ctx, {} );
			return ToolResult::Release();
		}
		const bool allSelected = std::all_of( done.hit.begin(), done.hit.end(),
		    [&]( const HandleRef &h )
		    {
			    return Contains( done.before, h );
		    } );
		if ( allSelected )
		{
			if ( done.pressModifiers.Ctrl() )
			{
				std::vector<HandleRef> next;
				for ( const HandleRef &h : done.before )
					if ( !Contains( done.hit, h ) )
						next.push_back( h );
				Select( ctx, next );
			}
			else
			{
				Select( ctx, done.hit );
			}
		}
		return ToolResult::Release();
	}
	if ( done.hit.empty() || !ctx.view.Is2D() || done.delta == Vec3d() )
		return ToolResult::Release();

	const Vec3d delta = done.delta;
	const std::vector<std::pair<scene::ObjectId, std::vector<int>>> moving = MovingVertices( ctx );
	// Where each selected handle ends up, to find it again after the rebuild.
	std::vector<std::pair<HandleRef, Vec3d>> targets;
	for ( const HandleInfo &info : Handles( ctx ) )
		if ( Contains( m_selected, info.ref ) )
			targets.emplace_back( info.ref, info.position + delta );
	const auto result = ctx.session.Execute( "Move vertices",
	    [&moving, delta]( scene::DocumentEdit &edit ) -> app::EditResult
	    {
		    for ( const auto &[solid, indices] : moving )
		    {
			    const app::EditResult moved = app::ops::MoveVertices( edit, solid, indices, delta );
			    if ( !moved )
				    return moved;
		    }
		    return {};
	    } );
	if ( !result )
		return ToolResult::Fail( result.Error().message );
	std::vector<HandleRef> remapped;
	for ( const HandleInfo &info : Handles( ctx ) )
		for ( const auto &[ref, position] : targets )
			if ( ref.solid == info.ref.solid && ref.edge == info.ref.edge &&
			     mapgeometry::NearlyEqual( info.position, position, kSamePosition ) )
				remapped.push_back( info.ref );
	Select( ctx, remapped );
	m_status = "Moved vertices " + scene::FormatVec3( delta );
	return ToolResult::Release();
}

ToolResult VertexTool::OnKey( ToolContext &ctx, const KeyEvent &event )
{
	if ( !event.IsPress() || event.key != Key::Escape )
		return ToolResult::Ignore();
	if ( m_gesture )
	{
		Cancel( CancelReason::Escape );
		return ToolResult::Release();
	}
	if ( SelectedHandles( ctx ).empty() )
		return ToolResult::Ignore();
	Select( ctx, {} );
	return ToolResult::Handle();
}

void VertexTool::Cancel( CancelReason reason )
{
	if ( m_gesture )
	{
		m_selected = m_gesture->before;
		m_status = std::string( "Cancelled (" ) + CancelReasonName( reason ) + ")";
	}
	m_gesture.reset();
}

void VertexTool::Deactivate()
{
	Cancel( CancelReason::ToolSwitched );
	m_selected.clear();
}

OverlayList VertexTool::Overlay( const ToolContext &ctx ) const
{
	OverlayList out;
	const std::vector<HandleRef> selected = SelectedHandles( ctx );
	for ( const HandleInfo &info : Handles( ctx ) )
	{
		const std::optional<viewport::ScreenPoint> at = ctx.view.Project( info.position );
		if ( !at )
			continue;
		out.Handle( *at, kHandleRadiusPixels * 0.75,
		    info.ref.edge ? HandleShape::Circle : HandleShape::Square,
		    Contains( selected, info.ref ) ? OverlayRole::Selection : OverlayRole::Handle );
	}
	if ( !m_gesture || !m_gesture->dragged || m_gesture->hit.empty() ||
	     m_gesture->delta == Vec3d() )
		return out;
	const scene::MapDocument &doc = ctx.session.Document();
	for ( const auto &[id, indices] : MovingVertices( ctx ) )
	{
		const scene::Solid &solid = *doc.FindSolid( id );
		std::vector<Vec3d> points = app::ops::SolidVertexList( solid );
		for ( int index : indices )
			points[index] += m_gesture->delta;
		if ( const std::optional<scene::Solid> rebuilt =
		         app::ops::RebuildFromVertices( solid, points ) )
		{
			AppendSolidEdges( out, *rebuilt, OverlayRole::Pending );
		}
		else
		{
			AppendSolidEdges( out, solid, OverlayRole::Error );
			for ( int index : indices )
				out.Box( points[index], points[index], OverlayRole::Error );
		}
	}
	return out;
}

Cursor VertexTool::CursorFor( const ToolContext &ctx, double x, double y ) const
{
	if ( m_gesture )
		return m_gesture->dragged && !m_gesture->hit.empty() ? Cursor::Move : Cursor::Default;
	return HitHandles( ctx, x, y ).empty() ? Cursor::Default : Cursor::Move;
}

} // namespace hammer::tools
