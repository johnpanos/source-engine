//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the headless editor interaction authority declared
//			in public/hammer/app/editor_controller.h (RFC 0002, hammer.app). See
//			that header for the contract. Grid snapping routes through the shared
//			hammer::geometry::RoundHalfAwayFromZero owner; brush geometry and VMF
//			import reuse the hammer.geometry brush bridge and hammer.formats codec.
//
//=============================================================================//

#include "hammer/app/editor_controller.h"

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/rounding.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace hammer::app
{

namespace
{

using geometry::Vec3d;

// The six outward face planes of an axis-aligned box.
std::vector<geometry::Plane> AabbToPlanes( const Vec3d &mins, const Vec3d &maxs )
{
	return {
	    { { 1, 0, 0 }, maxs.x },  { { -1, 0, 0 }, -mins.x }, { { 0, 1, 0 }, maxs.y },
	    { { 0, -1, 0 }, -mins.y }, { { 0, 0, 1 }, maxs.z },  { { 0, 0, -1 }, -mins.z },
	};
}

// One VMF "plane" value (three points) for a face of the box, wound outward.
std::string FacePlaneText( int face, const Vec3d &mn, const Vec3d &mx )
{
	// Three points per face, matching Source's outward winding convention. The
	// importer reorients regardless, but conventional text keeps the VMF portable.
	std::array<Vec3d, 3> p;
	switch ( face )
	{
	case 0: // +Z (top)
		p = { Vec3d( mn.x, mx.y, mx.z ), Vec3d( mx.x, mx.y, mx.z ), Vec3d( mx.x, mn.y, mx.z ) };
		break;
	case 1: // -Z (bottom)
		p = { Vec3d( mn.x, mn.y, mn.z ), Vec3d( mx.x, mn.y, mn.z ), Vec3d( mx.x, mx.y, mn.z ) };
		break;
	case 2: // +X
		p = { Vec3d( mx.x, mn.y, mn.z ), Vec3d( mx.x, mn.y, mx.z ), Vec3d( mx.x, mx.y, mx.z ) };
		break;
	case 3: // -X
		p = { Vec3d( mn.x, mx.y, mn.z ), Vec3d( mn.x, mx.y, mx.z ), Vec3d( mn.x, mn.y, mx.z ) };
		break;
	case 4: // +Y
		p = { Vec3d( mx.x, mx.y, mn.z ), Vec3d( mx.x, mx.y, mx.z ), Vec3d( mn.x, mx.y, mx.z ) };
		break;
	default: // -Y
		p = { Vec3d( mn.x, mn.y, mn.z ), Vec3d( mn.x, mn.y, mx.z ), Vec3d( mx.x, mn.y, mx.z ) };
		break;
	}
	char buf[256];
	std::snprintf( buf, sizeof( buf ), "(%g %g %g) (%g %g %g) (%g %g %g)", p[0].x, p[0].y, p[0].z,
	               p[1].x, p[1].y, p[1].z, p[2].x, p[2].y, p[2].z );
	return buf;
}

void AddPair( formats::KeyValueNode &node, const char *key, const std::string &value )
{
	node.pairs.push_back( { key, value } );
}

} // namespace

EditorController::EditorController()
{
	ResetHistory();
}

void EditorController::ViewAxes( ViewId view, int &uAxis, int &vAxis, int &freeAxis )
{
	switch ( view )
	{
	case ViewId::Top: // X / Y
		uAxis = 0;
		vAxis = 1;
		freeAxis = 2;
		break;
	case ViewId::Front: // X / Z
		uAxis = 0;
		vAxis = 2;
		freeAxis = 1;
		break;
	case ViewId::Side: // Y / Z
		uAxis = 1;
		vAxis = 2;
		freeAxis = 0;
		break;
	default: // Camera (navigation only)
		uAxis = 0;
		vAxis = 1;
		freeAxis = 2;
		break;
	}
}

double EditorController::Snap( double value ) const
{
	if ( m_grid <= 0 )
	{
		return value;
	}
	const float snappedUnits =
	    geometry::RoundHalfAwayFromZero( static_cast<float>( value / m_grid ) );
	return static_cast<double>( snappedUnits ) * m_grid;
}

void EditorController::ResetHistory()
{
	m_history = DocumentHistory{};
	m_snapshots.clear();
	m_snapshots.push_back( DocState{ m_brushes, m_nextId } );
}

void EditorController::PushSnapshot()
{
	// Drop any redo tail, then record the current state as a new history unit.
	m_snapshots.resize( m_history.Position() + 1 );
	m_snapshots.push_back( DocState{ m_brushes, m_nextId } );
	m_history.Commit( true );
}

void EditorController::LoadSnapshotAtPosition()
{
	const DocState &s = m_snapshots[m_history.Position()];
	m_brushes = s.brushes;
	m_nextId = s.nextId;
}

MapBrush *EditorController::FindBrush( int id )
{
	for ( MapBrush &b : m_brushes )
	{
		if ( b.id == id )
		{
			return &b;
		}
	}
	return nullptr;
}

const MapBrush *EditorController::FindBrush( int id ) const
{
	for ( const MapBrush &b : m_brushes )
	{
		if ( b.id == id )
		{
			return &b;
		}
	}
	return nullptr;
}

void EditorController::NewMap()
{
	m_brushes.clear();
	m_nextId = 1;
	m_selection.reset();
	m_pending = Pending{};
	m_dragging = false;
	ResetHistory();
}

void EditorController::SetTool( Tool tool )
{
	m_tool = tool;
	m_pending = Pending{};
	m_dragging = false;
}

void EditorController::SetGridSize( int grid )
{
	if ( grid > 0 )
	{
		m_grid = grid;
	}
}

void EditorController::PendingToAabb( Vec3d &mins, Vec3d &maxs ) const
{
	int uAxis = 0;
	int vAxis = 1;
	int freeAxis = 2;
	ViewAxes( m_pending.view, uAxis, vAxis, freeAxis );

	const double uMin = std::min( m_pending.u0, m_pending.u1 );
	const double uMax = std::max( m_pending.u0, m_pending.u1 );
	const double vMin = std::min( m_pending.v0, m_pending.v1 );
	const double vMax = std::max( m_pending.v0, m_pending.v1 );

	double lo[3] = { 0, 0, 0 };
	double hi[3] = { 0, 0, 0 };
	lo[uAxis] = uMin;
	hi[uAxis] = uMax;
	lo[vAxis] = vMin;
	hi[vAxis] = vMax;
	lo[freeAxis] = 0.0;
	hi[freeAxis] = m_blockDepth;

	mins = Vec3d( lo[0], lo[1], lo[2] );
	maxs = Vec3d( hi[0], hi[1], hi[2] );
}

bool EditorController::PendingBox( Vec3d &mins, Vec3d &maxs ) const
{
	if ( !m_pending.active )
	{
		return false;
	}
	PendingToAabb( mins, maxs );
	return true;
}

void EditorController::PointerDown( ViewId view, double u, double v )
{
	if ( view == ViewId::Camera )
	{
		return;
	}
	const double su = Snap( u );
	const double sv = Snap( v );

	if ( m_tool == Tool::Block )
	{
		m_pending.active = true;
		m_pending.view = view;
		m_pending.u0 = su;
		m_pending.v0 = sv;
		m_pending.u1 = su;
		m_pending.v1 = sv;
		return;
	}

	// Selection: hit-test the topmost brush whose projection contains (u,v).
	int uAxis = 0;
	int vAxis = 1;
	int freeAxis = 2;
	ViewAxes( view, uAxis, vAxis, freeAxis );

	m_selection.reset();
	m_dragging = false;
	for ( auto it = m_brushes.rbegin(); it != m_brushes.rend(); ++it )
	{
		const double lo[3] = { it->mins.x, it->mins.y, it->mins.z };
		const double hi[3] = { it->maxs.x, it->maxs.y, it->maxs.z };
		if ( u >= lo[uAxis] && u <= hi[uAxis] && v >= lo[vAxis] && v <= hi[vAxis] )
		{
			m_selection = it->id;
			m_dragging = true;
			m_dragStartU = su;
			m_dragStartV = sv;
			m_dragOrig = *it;
			m_dragMoved = false;
			break;
		}
	}
}

void EditorController::PointerDrag( ViewId view, double u, double v )
{
	if ( view == ViewId::Camera )
	{
		return;
	}
	const double su = Snap( u );
	const double sv = Snap( v );

	if ( m_tool == Tool::Block )
	{
		if ( m_pending.active && m_pending.view == view )
		{
			m_pending.u1 = su;
			m_pending.v1 = sv;
		}
		return;
	}

	if ( m_dragging && m_selection )
	{
		int uAxis = 0;
		int vAxis = 1;
		int freeAxis = 2;
		ViewAxes( view, uAxis, vAxis, freeAxis );
		const double du = su - m_dragStartU;
		const double dv = sv - m_dragStartV;
		if ( du != 0.0 || dv != 0.0 )
		{
			m_dragMoved = true;
		}
		MapBrush *brush = FindBrush( *m_selection );
		if ( brush )
		{
			double lo[3] = { m_dragOrig.mins.x, m_dragOrig.mins.y, m_dragOrig.mins.z };
			double hi[3] = { m_dragOrig.maxs.x, m_dragOrig.maxs.y, m_dragOrig.maxs.z };
			lo[uAxis] += du;
			hi[uAxis] += du;
			lo[vAxis] += dv;
			hi[vAxis] += dv;
			brush->mins = Vec3d( lo[0], lo[1], lo[2] );
			brush->maxs = Vec3d( hi[0], hi[1], hi[2] );
		}
	}
}

void EditorController::PointerUp( ViewId view, double u, double v )
{
	if ( view == ViewId::Camera )
	{
		return;
	}
	if ( m_tool == Tool::Block )
	{
		if ( m_pending.active && m_pending.view == view )
		{
			m_pending.u1 = Snap( u );
			m_pending.v1 = Snap( v );
		}
		return;
	}

	if ( m_dragging )
	{
		if ( m_dragMoved )
		{
			PushSnapshot(); // the move is one undo unit
		}
		m_dragging = false;
	}
}

bool EditorController::Commit()
{
	if ( !m_pending.active )
	{
		return false;
	}
	Vec3d mins;
	Vec3d maxs;
	PendingToAabb( mins, maxs );
	// Reject a degenerate (zero-area) rectangle.
	if ( mins.x >= maxs.x || mins.y >= maxs.y || mins.z >= maxs.z )
	{
		return false;
	}

	MapBrush brush;
	brush.id = m_nextId++;
	brush.mins = mins;
	brush.maxs = maxs;
	brush.material = m_defaultMaterial;
	m_brushes.push_back( brush );
	m_selection = brush.id;
	m_pending = Pending{};
	PushSnapshot();
	return true;
}

bool EditorController::DeleteSelection()
{
	if ( !m_selection )
	{
		return false;
	}
	const int id = *m_selection;
	const auto before = m_brushes.size();
	m_brushes.erase(
	    std::remove_if( m_brushes.begin(), m_brushes.end(), [id]( const MapBrush &b ) { return b.id == id; } ),
	    m_brushes.end() );
	if ( m_brushes.size() == before )
	{
		return false;
	}
	m_selection.reset();
	PushSnapshot();
	return true;
}

bool EditorController::Undo()
{
	if ( !m_history.Undo() )
	{
		return false;
	}
	LoadSnapshotAtPosition();
	m_selection.reset();
	m_pending = Pending{};
	m_dragging = false;
	return true;
}

bool EditorController::Redo()
{
	if ( !m_history.Redo() )
	{
		return false;
	}
	LoadSnapshotAtPosition();
	m_selection.reset();
	m_pending = Pending{};
	m_dragging = false;
	return true;
}

geometry::WorldScene EditorController::BuildScene() const
{
	geometry::WorldScene scene;

	auto addSolid = [&]( const Vec3d &mins, const Vec3d &maxs, int id )
	{
		geometry::BrushSolid solid =
		    geometry::BuildSolidFromPlanes( AabbToPlanes( mins, maxs ), {}, id );
		if ( solid.faces.empty() )
		{
			return;
		}
		if ( solid.bounded )
		{
			if ( !scene.bounded )
			{
				scene.mins = solid.mins;
				scene.maxs = solid.maxs;
				scene.bounded = true;
			}
			else
			{
				scene.mins = Vec3d( std::min( scene.mins.x, solid.mins.x ),
				                    std::min( scene.mins.y, solid.mins.y ),
				                    std::min( scene.mins.z, solid.mins.z ) );
				scene.maxs = Vec3d( std::max( scene.maxs.x, solid.maxs.x ),
				                    std::max( scene.maxs.y, solid.maxs.y ),
				                    std::max( scene.maxs.z, solid.maxs.z ) );
			}
		}
		scene.solids.push_back( std::move( solid ) );
	};

	for ( const MapBrush &b : m_brushes )
	{
		addSolid( b.mins, b.maxs, b.id );
	}
	if ( m_pending.active )
	{
		Vec3d mins;
		Vec3d maxs;
		PendingToAabb( mins, maxs );
		if ( mins.x < maxs.x && mins.y < maxs.y && mins.z < maxs.z )
		{
			addSolid( mins, maxs, kPendingSolidId );
		}
	}
	return scene;
}

std::string EditorController::ToVmf() const
{
	formats::KeyValueNode root;

	formats::KeyValueNode version;
	version.name = "versioninfo";
	AddPair( version, "editorversion", "400" );
	AddPair( version, "editorbuild", "8000" );
	AddPair( version, "mapversion", "1" );
	AddPair( version, "formatversion", "100" );
	AddPair( version, "prefab", "0" );
	root.children.push_back( version );

	int id = 1;
	formats::KeyValueNode world;
	world.name = "world";
	AddPair( world, "id", std::to_string( id++ ) );
	AddPair( world, "mapversion", "1" );
	AddPair( world, "classname", "worldspawn" );

	for ( const MapBrush &b : m_brushes )
	{
		formats::KeyValueNode solid;
		solid.name = "solid";
		AddPair( solid, "id", std::to_string( id++ ) );
		for ( int face = 0; face < 6; ++face )
		{
			formats::KeyValueNode side;
			side.name = "side";
			AddPair( side, "id", std::to_string( id++ ) );
			AddPair( side, "plane", FacePlaneText( face, b.mins, b.maxs ) );
			AddPair( side, "material", b.material );
			AddPair( side, "uaxis", "[1 0 0 0] 0.25" );
			AddPair( side, "vaxis", "[0 -1 0 0] 0.25" );
			AddPair( side, "rotation", "0" );
			AddPair( side, "lightmapscale", "16" );
			AddPair( side, "smoothing_groups", "0" );
			solid.children.push_back( std::move( side ) );
		}
		world.children.push_back( std::move( solid ) );
	}
	root.children.push_back( std::move( world ) );

	return formats::WriteKeyValues( root );
}

bool EditorController::LoadVmf( const std::string &vmfText, std::string &error )
{
	formats::ParseResult pr = formats::ParseKeyValues( vmfText );
	if ( !pr.ok )
	{
		error = pr.error;
		return false;
	}

	const geometry::WorldScene scene = geometry::BuildSceneFromDocument( pr.root );

	m_brushes.clear();
	m_nextId = 1;
	m_selection.reset();
	m_pending = Pending{};
	m_dragging = false;

	for ( const geometry::BrushSolid &solid : scene.solids )
	{
		if ( !solid.bounded )
		{
			continue;
		}
		MapBrush b;
		b.id = m_nextId++;
		b.mins = solid.mins;
		b.maxs = solid.maxs;
		b.material = solid.faces.empty() ? m_defaultMaterial : solid.faces.front().material;
		if ( b.material.empty() )
		{
			b.material = m_defaultMaterial;
		}
		m_brushes.push_back( std::move( b ) );
	}

	ResetHistory(); // the loaded map is the unmodified base state
	return true;
}

} // namespace hammer::app
