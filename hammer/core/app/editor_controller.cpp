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

#include "hammer/formats/vmf_geometry.h" // also the keyvalues codec
#include "hammer/geometry/rounding.h"
#include "hammer/geometry/texture_axes.h"

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
	    { { 1, 0, 0 }, maxs.x },
	    { { -1, 0, 0 }, -mins.x },
	    { { 0, 1, 0 }, maxs.y },
	    { { 0, -1, 0 }, -mins.y },
	    { { 0, 0, 1 }, maxs.z },
	    { { 0, 0, -1 }, -mins.z },
	};
}

Vec3d Cross( const Vec3d &a, const Vec3d &b )
{
	return Vec3d( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

Vec3d Normalized( const Vec3d &a )
{
	const double len = std::sqrt( a.x * a.x + a.y * a.y + a.z * a.z );
	if ( len <= 1.0e-12 )
	{
		return Vec3d( 0, 0, 1 );
	}
	return Vec3d( a.x / len, a.y / len, a.z / len );
}

// A VMF "plane" value (three points) for an arbitrary plane, wound so that
// (p1-p0) x (p2-p0) points along the plane normal (outward preserved). Works for
// any brush face, not just axis-aligned boxes.
std::string PlaneToText( const geometry::Plane &plane )
{
	const Vec3d n = plane.normal;
	const Vec3d up = ( std::fabs( n.z ) < 0.9 ) ? Vec3d( 0, 0, 1 ) : Vec3d( 1, 0, 0 );
	const Vec3d u = Normalized( Cross( up, n ) );
	const Vec3d v = Cross( n, u ); // unit, since u,n orthonormal
	const double s = 64.0;
	const Vec3d c( n.x * plane.dist, n.y * plane.dist, n.z * plane.dist );
	const Vec3d p0 = c;
	const Vec3d p1( c.x + u.x * s, c.y + u.y * s, c.z + u.z * s );
	const Vec3d p2( c.x + v.x * s, c.y + v.y * s, c.z + v.z * s );
	// Negative zero (from a normal rebuilt out of loaded plane points) prints as
	// "-0"; writing it as 0 keeps a reloaded map's save byte-identical.
	auto z = []( double value ) { return value == 0.0 ? 0.0 : value; };
	char buf[256];
	std::snprintf( buf, sizeof( buf ), "(%g %g %g) (%g %g %g) (%g %g %g)", z( p0.x ), z( p0.y ),
	    z( p0.z ), z( p1.x ), z( p1.y ), z( p1.z ), z( p2.x ), z( p2.y ), z( p2.z ) );
	return buf;
}

// A VMF texture axis at the default scale: "[x y z offset] scale".
std::string AxisToText( const Vec3d &axis )
{
	char buf[128];
	auto z = []( double value ) { return value == 0.0 ? 0.0 : value; };
	std::snprintf(
	    buf, sizeof( buf ), "[%g %g %g 0] 0.25", z( axis.x ), z( axis.y ), z( axis.z ) );
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
	m_snapshots.push_back( DocState{ m_brushes, m_entities, m_worldProperties, m_nextId } );
}

void EditorController::PushSnapshot()
{
	// Drop any redo tail, then record the current state as a new history unit.
	m_snapshots.resize( m_history.Position() + 1 );
	m_snapshots.push_back( DocState{ m_brushes, m_entities, m_worldProperties, m_nextId } );
	m_history.Commit( true );
}

void EditorController::LoadSnapshotAtPosition()
{
	const DocState &s = m_snapshots[m_history.Position()];
	m_brushes = s.brushes;
	m_entities = s.entities;
	m_worldProperties = s.worldProperties;
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

MapEntity *EditorController::FindEntity( int id )
{
	for ( MapEntity &e : m_entities )
	{
		if ( e.id == id )
		{
			return &e;
		}
	}
	return nullptr;
}

bool EditorController::RetextureBrush( int id )
{
	MapBrush *brush = FindBrush( id );
	if ( !brush )
	{
		return false;
	}
	bool changed = false;
	for ( std::string &material : brush->materials )
	{
		if ( material != m_defaultMaterial )
		{
			material = m_defaultMaterial;
			changed = true;
		}
	}
	// A brush loaded with fewer materials than planes still gets fully covered.
	if ( brush->materials.size() < brush->planes.size() )
	{
		brush->materials.resize( brush->planes.size(), m_defaultMaterial );
		changed = true;
	}
	return changed;
}

std::vector<int> EditorController::Selections() const
{
	std::vector<int> all = m_extraSelected;
	if ( m_selection )
	{
		all.push_back( *m_selection );
	}
	return all;
}

bool EditorController::IsSelected( int id ) const
{
	if ( m_selection && *m_selection == id )
	{
		return true;
	}
	for ( int e : m_extraSelected )
	{
		if ( e == id )
		{
			return true;
		}
	}
	return false;
}

void EditorController::ClearBrushSelection()
{
	m_selection.reset();
	m_extraSelected.clear();
}

void EditorController::ApplyBrushPick( int id, bool additive )
{
	m_entitySelection.reset();
	if ( !additive )
	{
		// Single-select: replace the whole selection with just this brush.
		m_extraSelected.clear();
		m_selection = id;
		return;
	}
	// Additive (Ctrl-click): toggle 'id'. Removing the primary promotes the most
	// recent extra to primary; adding a new one makes it the primary.
	if ( m_selection && *m_selection == id )
	{
		if ( m_extraSelected.empty() )
		{
			m_selection.reset();
		}
		else
		{
			m_selection = m_extraSelected.back();
			m_extraSelected.pop_back();
		}
		return;
	}
	for ( auto it = m_extraSelected.begin(); it != m_extraSelected.end(); ++it )
	{
		if ( *it == id )
		{
			m_extraSelected.erase( it ); // toggle an extra off
			return;
		}
	}
	if ( m_selection )
	{
		m_extraSelected.push_back( *m_selection ); // demote the old primary
	}
	m_selection = id;
}

void EditorController::NewMap()
{
	m_brushes.clear();
	m_entities.clear();
	m_worldProperties = { { "skyname", "sky_day01_01" } };
	m_displacements.clear();
	m_nextId = 1;
	ClearBrushSelection();
	m_entitySelection.reset();
	m_pending = Pending{};
	m_dragging = false;
	ResetHistory();
}

void EditorController::SetEntityClass( const std::string &classname )
{
	if ( !classname.empty() )
	{
		m_entityClass = classname;
	}
}

void EditorController::SetActiveMaterial( const std::string &material )
{
	if ( !material.empty() )
	{
		m_defaultMaterial = material;
	}
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

void EditorController::PointerDown( ViewId view, double u, double v, bool additive )
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

	int uAxis = 0;
	int vAxis = 1;
	int freeAxis = 2;
	ViewAxes( view, uAxis, vAxis, freeAxis );

	if ( m_tool == Tool::Entity )
	{
		// A single click places a point entity of the active class at the snapped
		// location; the in-view axes take (su,sv) and the free axis sits at 0.
		double o[3] = { 0, 0, 0 };
		o[uAxis] = su;
		o[vAxis] = sv;
		o[freeAxis] = 0.0;
		PlaceEntity( m_entityClass, Vec3d( o[0], o[1], o[2] ) );
		return;
	}

	if ( m_tool == Tool::Material )
	{
		// Clicking a brush applies the active material to all its faces (one undo
		// unit iff it changed), routed through the shared retexture path.
		for ( auto it = m_brushes.rbegin(); it != m_brushes.rend(); ++it )
		{
			const double lo[3] = { it->mins.x, it->mins.y, it->mins.z };
			const double hi[3] = { it->maxs.x, it->maxs.y, it->maxs.z };
			if ( u >= lo[uAxis] && u <= hi[uAxis] && v >= lo[vAxis] && v <= hi[vAxis] )
			{
				ApplyBrushPick( it->id, false );
				if ( RetextureBrush( it->id ) )
				{
					PushSnapshot();
				}
				break;
			}
		}
		return;
	}

	// Selection: hit-test the topmost brush whose projection contains (u,v).
	m_entitySelection.reset();
	m_dragging = false;
	int hitId = 0;
	bool hit = false;
	for ( auto it = m_brushes.rbegin(); it != m_brushes.rend(); ++it )
	{
		const double lo[3] = { it->mins.x, it->mins.y, it->mins.z };
		const double hi[3] = { it->maxs.x, it->maxs.y, it->maxs.z };
		if ( u >= lo[uAxis] && u <= hi[uAxis] && v >= lo[vAxis] && v <= hi[vAxis] )
		{
			hitId = it->id;
			hit = true;
			break;
		}
	}

	if ( hit )
	{
		if ( additive )
		{
			ApplyBrushPick( hitId, true ); // Ctrl-click toggles; no drag
			return;
		}
		// Clicking a member of a multi-selection keeps the whole set (so it drags
		// together); clicking elsewhere selects just this brush.
		if ( !IsSelected( hitId ) )
		{
			ApplyBrushPick( hitId, false );
		}
		// Begin dragging every selected brush from its current position.
		m_dragging = true;
		m_dragStartU = su;
		m_dragStartV = sv;
		m_dragMoved = false;
		m_dragOrigs.clear();
		for ( int id : Selections() )
		{
			if ( const MapBrush *b = FindBrush( id ) )
			{
				m_dragOrigs.push_back( *b );
			}
		}
		return;
	}

	// Nothing hit: a plain click clears the brush selection; Ctrl keeps it.
	if ( !additive )
	{
		ClearBrushSelection();
	}

	// No brush under the pointer: pick the nearest point entity within a half-grid
	// tolerance in the view plane, so placed entities can be selected and deleted.
	if ( !m_selection )
	{
		const double tol = ( m_grid > 0 ? m_grid : 64 ) * 0.5;
		double best = tol;
		for ( auto it = m_entities.rbegin(); it != m_entities.rend(); ++it )
		{
			const double c[3] = { it->origin.x, it->origin.y, it->origin.z };
			const double du = u - c[uAxis];
			const double dv = v - c[vAxis];
			const double d = std::sqrt( du * du + dv * dv );
			if ( d <= best )
			{
				best = d;
				m_entitySelection = it->id;
			}
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

	if ( m_dragging && !m_dragOrigs.empty() )
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
		double d[3] = { 0, 0, 0 };
		d[uAxis] = du;
		d[vAxis] = dv;
		const Vec3d delta( d[0], d[1], d[2] );

		// Translate every dragged brush from its recorded original: shift each face
		// plane and the cached bound by the same delta, so a non-box shape moves
		// rigidly and the whole multi-selection moves together.
		for ( const MapBrush &orig : m_dragOrigs )
		{
			MapBrush *brush = FindBrush( orig.id );
			if ( !brush )
			{
				continue;
			}
			brush->planes = orig.planes;
			for ( geometry::Plane &p : brush->planes )
			{
				// dot(n, x') = dist + dot(n, delta) keeps the plane through x+delta.
				p.dist += p.normal.x * delta.x + p.normal.y * delta.y + p.normal.z * delta.z;
			}
			brush->mins =
			    Vec3d( orig.mins.x + delta.x, orig.mins.y + delta.y, orig.mins.z + delta.z );
			brush->maxs =
			    Vec3d( orig.maxs.x + delta.x, orig.maxs.y + delta.y, orig.maxs.z + delta.z );
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

namespace
{

// Clips a ray against a brush's cached AABB (broad phase). Returns the entry t
// (>= 0) if the ray meets the box ahead of its origin, else false.
bool RayHitsAabb(
    const double o[3], const double d[3], const Vec3d &mins, const Vec3d &maxs, double &outEnter )
{
	const double lo[3] = { mins.x, mins.y, mins.z };
	const double hi[3] = { maxs.x, maxs.y, maxs.z };
	double tNear = 0.0;
	double tFar = HUGE_VAL;
	for ( int i = 0; i < 3; ++i )
	{
		if ( std::fabs( d[i] ) < 1.0e-12 )
		{
			if ( o[i] < lo[i] || o[i] > hi[i] )
			{
				return false;
			}
			continue;
		}
		double t1 = ( lo[i] - o[i] ) / d[i];
		double t2 = ( hi[i] - o[i] ) / d[i];
		if ( t1 > t2 )
		{
			const double tmp = t1;
			t1 = t2;
			t2 = tmp;
		}
		tNear = t1 > tNear ? t1 : tNear;
		tFar = t2 < tFar ? t2 : tFar;
		if ( tNear > tFar )
		{
			return false;
		}
	}
	outEnter = tNear;
	return tFar >= 0.0;
}

// Clips a ray against the convex intersection of a brush's outward half-spaces
// (dot(n,x) <= dist inside), the brush's TRUE shape. Returns the entry t (>= 0)
// if the ray actually enters the solid ahead of its origin, else false. This is
// the precise face pick: a ray through a non-box brush's AABB corner correctly
// misses the solid. Falls back to false for a brush with no planes.
bool RayHitsConvex( const double o[3], const double d[3],
    const std::vector<geometry::Plane> &planes, double &outEnter )
{
	if ( planes.empty() )
	{
		return false;
	}
	double tEnter = 0.0; // clamp at the origin: ignore the solid behind the eye
	double tExit = HUGE_VAL;
	for ( const geometry::Plane &p : planes )
	{
		const double n[3] = { p.normal.x, p.normal.y, p.normal.z };
		const double denom = n[0] * d[0] + n[1] * d[1] + n[2] * d[2];
		const double distO = n[0] * o[0] + n[1] * o[1] + n[2] * o[2] - p.dist; // >0 outside
		if ( std::fabs( denom ) < 1.0e-12 )
		{
			// Parallel to this face: outside it means the ray never enters the solid.
			if ( distO > 0.0 )
			{
				return false;
			}
			continue;
		}
		const double t = -distO / denom;
		if ( denom < 0.0 )
		{
			tEnter = t > tEnter ? t : tEnter; // crossing inward
		}
		else
		{
			tExit = t < tExit ? t : tExit; // crossing outward
		}
		if ( tEnter > tExit )
		{
			return false;
		}
	}
	outEnter = tEnter;
	return tExit >= 0.0;
}

} // namespace

bool EditorController::PickByRay(
    const geometry::Vec3d &origin, const geometry::Vec3d &dir, bool additive )
{
	// Broad phase against the cached AABB, then a precise ray/convex-polytope test
	// against the brush's real face planes; keep the nearest brush the ray enters
	// ahead of the origin. The AABB is shared with the 2D click pick, so both stay
	// one selection policy; the plane test makes 3D picking face-accurate for
	// arbitrary convex brushes, not just their bounding box.
	const double o[3] = { origin.x, origin.y, origin.z };
	const double d[3] = { dir.x, dir.y, dir.z };

	bool haveHit = false;
	double bestT = HUGE_VAL;
	int hitId = 0;
	for ( const MapBrush &b : m_brushes )
	{
		double aabbT = 0.0;
		if ( !RayHitsAabb( o, d, b.mins, b.maxs, aabbT ) || aabbT >= bestT )
		{
			continue;
		}
		double solidT = 0.0;
		if ( !RayHitsConvex( o, d, b.planes, solidT ) )
		{
			continue;
		}
		if ( solidT < bestT )
		{
			bestT = solidT;
			hitId = b.id;
			haveHit = true;
		}
	}

	// Selection-only mutation, exactly like a 2D click: no history push.
	m_dragging = false;
	if ( haveHit )
	{
		ApplyBrushPick( hitId, additive ); // additive (Ctrl) toggles into a set
	}
	else if ( !additive )
	{
		ClearBrushSelection(); // an empty non-additive click deselects
		m_entitySelection.reset();
	}
	return haveHit;
}

bool EditorController::MoveSelectionBy( double dx, double dy, double dz )
{
	const std::vector<int> ids = Selections();
	if ( ids.empty() )
	{
		return false;
	}
	// Shift every selected brush rigidly (every face plane and the cached bound by
	// the same delta), so a non-box brush and the whole multi-selection move as one.
	bool moved = false;
	for ( int id : ids )
	{
		MapBrush *brush = FindBrush( id );
		if ( !brush )
		{
			continue;
		}
		for ( geometry::Plane &p : brush->planes )
		{
			p.dist += p.normal.x * dx + p.normal.y * dy + p.normal.z * dz;
		}
		brush->mins = Vec3d( brush->mins.x + dx, brush->mins.y + dy, brush->mins.z + dz );
		brush->maxs = Vec3d( brush->maxs.x + dx, brush->maxs.y + dy, brush->maxs.z + dz );
		moved = true;
	}
	if ( moved )
	{
		PushSnapshot(); // one undo unit for the whole nudge, like a drag-move
	}
	return moved;
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
	// A degenerate (zero-area) rectangle creates nothing and stays pending.
	if ( !CreateBlock( mins, maxs ) )
	{
		return false;
	}
	m_pending = Pending{};
	return true;
}

std::optional<int> EditorController::CreateBlock( const Vec3d &mins, const Vec3d &maxs )
{
	if ( mins.x >= maxs.x || mins.y >= maxs.y || mins.z >= maxs.z )
	{
		return std::nullopt;
	}
	MapBrush brush;
	brush.id = m_nextId++;
	brush.planes = AabbToPlanes( mins, maxs );
	brush.materials.assign( brush.planes.size(), m_defaultMaterial );
	brush.mins = mins;
	brush.maxs = maxs;
	m_brushes.push_back( brush );
	ClearBrushSelection();
	m_entitySelection.reset();
	m_selection = brush.id;
	PushSnapshot();
	return brush.id;
}

std::optional<int> EditorController::PlaceEntity(
    const std::string &classname, const Vec3d &origin )
{
	if ( classname.empty() )
	{
		return std::nullopt;
	}
	MapEntity e;
	e.id = m_nextId++;
	e.classname = classname;
	e.origin = origin;
	m_entities.push_back( e );
	m_entitySelection = e.id;
	ClearBrushSelection();
	PushSnapshot();
	return e.id;
}

bool EditorController::SetEntityOrigin( int entityId, const Vec3d &origin )
{
	MapEntity *entity = FindEntity( entityId );
	if ( !entity )
	{
		return false;
	}
	if ( entity->origin.x == origin.x && entity->origin.y == origin.y &&
	     entity->origin.z == origin.z )
	{
		return false; // no-op edit records no history
	}
	entity->origin = origin;
	PushSnapshot();
	return true;
}

bool EditorController::SetWorldProperty( const std::string &key, const std::string &value )
{
	if ( key.empty() || key == "classname" || key == "mapversion" || key == "id" )
	{
		return false;
	}
	for ( EntityProperty &p : m_worldProperties )
	{
		if ( p.key == key )
		{
			if ( p.value == value )
			{
				return false;
			}
			p.value = value;
			PushSnapshot();
			return true;
		}
	}
	m_worldProperties.push_back( { key, value } );
	PushSnapshot();
	return true;
}

bool EditorController::DeleteSelection()
{
	if ( m_selection )
	{
		// Delete every selected brush (the primary plus any Ctrl-added extras).
		const std::vector<int> ids = Selections();
		const auto before = m_brushes.size();
		m_brushes.erase( std::remove_if( m_brushes.begin(), m_brushes.end(),
		                     [&ids]( const MapBrush &b )
		                     {
			                     return std::find( ids.begin(), ids.end(), b.id ) != ids.end();
		                     } ),
		    m_brushes.end() );
		if ( m_brushes.size() == before )
		{
			return false;
		}
		ClearBrushSelection();
		PushSnapshot();
		return true;
	}

	if ( m_entitySelection )
	{
		const int id = *m_entitySelection;
		const auto before = m_entities.size();
		m_entities.erase( std::remove_if( m_entities.begin(), m_entities.end(),
		                      [id]( const MapEntity &e )
		                      {
			                      return e.id == id;
		                      } ),
		    m_entities.end() );
		if ( m_entities.size() == before )
		{
			return false;
		}
		m_entitySelection.reset();
		PushSnapshot();
		return true;
	}

	return false;
}

bool EditorController::ApplyActiveMaterialToSelection()
{
	if ( !m_selection )
	{
		return false;
	}
	if ( !RetextureBrush( *m_selection ) )
	{
		return false;
	}
	PushSnapshot();
	return true;
}

bool EditorController::SetEntityProperty(
    int entityId, const std::string &key, const std::string &value )
{
	if ( key.empty() || key == "classname" || key == "origin" || key == "id" )
	{
		return false;
	}
	MapEntity *entity = FindEntity( entityId );
	if ( !entity )
	{
		return false;
	}
	for ( EntityProperty &p : entity->properties )
	{
		if ( p.key == key )
		{
			if ( p.value == value )
			{
				return false; // no-op edit records no history
			}
			p.value = value;
			PushSnapshot();
			return true;
		}
	}
	entity->properties.push_back( { key, value } );
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
	ClearBrushSelection();
	m_entitySelection.reset();
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
	ClearBrushSelection();
	m_entitySelection.reset();
	m_pending = Pending{};
	m_dragging = false;
	return true;
}

geometry::WorldScene EditorController::BuildScene() const
{
	geometry::WorldScene scene;

	auto addSolid = [&]( const std::vector<geometry::Plane> &planes,
	                    const std::vector<std::string> &materials, int id )
	{
		geometry::BrushSolid solid = geometry::BuildSolidFromPlanes( planes, materials, id );
		if ( solid.faces.empty() )
		{
			return;
		}
		scene.AddSolid( std::move( solid ) );
	};

	// Render each brush's true shape from its own planes -- not a bounding box.
	for ( const MapBrush &b : m_brushes )
	{
		addSolid( b.planes, b.materials, b.id );
	}
	if ( m_pending.active )
	{
		Vec3d mins;
		Vec3d maxs;
		PendingToAabb( mins, maxs );
		if ( mins.x < maxs.x && mins.y < maxs.y && mins.z < maxs.z )
		{
			addSolid( AabbToPlanes( mins, maxs ), {}, kPendingSolidId );
		}
	}

	// Summarise placed point entities so a presenter can draw their markers. This is
	// identification only; point entities contribute no solids.
	for ( const MapEntity &e : m_entities )
	{
		geometry::SceneEntity se;
		se.classname = e.classname;
		se.origin = e.origin;
		for ( const EntityProperty &p : e.properties )
		{
			if ( p.key == "targetname" )
			{
				se.targetname = p.value;
				break;
			}
		}
		scene.entities.push_back( std::move( se ) );
	}

	// Carry any loaded displacement (dispinfo terrain) surfaces so the interactive
	// viewport renders them alongside the brushes. They are display-only in this
	// slice (not editable), and their vertices extend the scene bounds.
	for ( const geometry::DisplacementMesh &d : m_displacements )
	{
		scene.AddDisplacement( d );
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
	for ( const EntityProperty &p : m_worldProperties )
	{
		AddPair( world, p.key.c_str(), p.value );
	}

	for ( const MapBrush &b : m_brushes )
	{
		formats::KeyValueNode solid;
		solid.name = "solid";
		AddPair( solid, "id", std::to_string( id++ ) );
		// One side per real plane, preserving the brush's actual shape and (when
		// known) each face's material -- not a six-sided box approximation.
		for ( std::size_t f = 0; f < b.planes.size(); ++f )
		{
			formats::KeyValueNode side;
			side.name = "side";
			AddPair( side, "id", std::to_string( id++ ) );
			AddPair( side, "plane", PlaneToText( b.planes[f] ) );
			AddPair( side, "material",
			    f < b.materials.size() && !b.materials[f].empty() ? b.materials[f]
			                                                      : m_defaultMaterial );
			// World-aligned axes from the face normal (legacy Hammer's rule), so
			// walls are not smeared by the floor's projection.
			const geometry::TextureAxes axes =
			    geometry::WorldAlignedTextureAxes( b.planes[f].normal );
			AddPair( side, "uaxis", AxisToText( axes.u ) );
			AddPair( side, "vaxis", AxisToText( axes.v ) );
			AddPair( side, "rotation", "0" );
			AddPair( side, "lightmapscale", "16" );
			AddPair( side, "smoothing_groups", "0" );
			solid.children.push_back( std::move( side ) );
		}
		world.children.push_back( std::move( solid ) );
	}
	root.children.push_back( std::move( world ) );

	// Each placed point entity is a top-level "entity" block (sibling of "world").
	for ( const MapEntity &e : m_entities )
	{
		formats::KeyValueNode entity;
		entity.name = "entity";
		AddPair( entity, "id", std::to_string( id++ ) );
		AddPair( entity, "classname", e.classname );
		for ( const EntityProperty &p : e.properties )
		{
			AddPair( entity, p.key.c_str(), p.value );
		}
		char origin[128];
		std::snprintf( origin, sizeof( origin ), "%g %g %g", e.origin.x, e.origin.y, e.origin.z );
		AddPair( entity, "origin", origin );
		root.children.push_back( std::move( entity ) );
	}

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

	const geometry::WorldScene scene = formats::BuildSceneFromDocument( pr.root );

	m_brushes.clear();
	m_entities.clear();
	m_nextId = 1;
	ClearBrushSelection();
	m_entitySelection.reset();
	m_pending = Pending{};
	m_dragging = false;

	// Carry displacement (dispinfo terrain) surfaces for display; they render but
	// are not yet editable, and survive save via the original brush faces.
	m_displacements = scene.displacements;

	// Worldspawn keyvalues (skyname and the like) are document state.
	m_worldProperties.clear();
	for ( const formats::KeyValueNode &block : pr.root.children )
	{
		if ( block.name != "world" )
		{
			continue;
		}
		for ( const auto &pair : block.pairs )
		{
			if ( pair.key != "id" && pair.key != "mapversion" && pair.key != "classname" )
			{
				m_worldProperties.push_back( { pair.key, pair.value } );
			}
		}
		break;
	}

	for ( const geometry::BrushSolid &solid : scene.solids )
	{
		if ( !solid.bounded )
		{
			continue;
		}
		// Preserve the brush's real shape: keep every side plane and material, not
		// just the bounding box. The cached AABB is only for picking/grid math.
		MapBrush b;
		b.id = m_nextId++;
		b.mins = solid.mins;
		b.maxs = solid.maxs;
		for ( const geometry::BrushFace &face : solid.faces )
		{
			b.planes.push_back( face.plane );
			b.materials.push_back( face.material.empty() ? m_defaultMaterial : face.material );
		}
		m_brushes.push_back( std::move( b ) );
	}

	// Reconstruct placed point entities from the top-level "entity" blocks. Brush
	// entities (blocks with a "solid" child) already contributed their geometry to
	// the brush list above and are a later migration, so they are skipped here.
	for ( const formats::KeyValueNode &block : pr.root.children )
	{
		if ( block.name != "entity" )
		{
			continue;
		}
		bool hasSolid = false;
		for ( const formats::KeyValueNode &child : block.children )
		{
			if ( child.name == "solid" )
			{
				hasSolid = true;
				break;
			}
		}
		if ( hasSolid )
		{
			continue;
		}

		MapEntity e;
		e.id = m_nextId++;
		if ( const std::string *cls = block.Find( "classname" ) )
		{
			e.classname = *cls;
		}
		if ( const std::string *origin = block.Find( "origin" ) )
		{
			double x = 0;
			double y = 0;
			double z = 0;
			std::sscanf( origin->c_str(), "%lf %lf %lf", &x, &y, &z );
			e.origin = Vec3d( x, y, z );
		}
		// Preserve every other authored key verbatim so the entity round-trips.
		for ( const formats::KeyValue &kv : block.pairs )
		{
			if ( kv.key == "id" || kv.key == "classname" || kv.key == "origin" )
			{
				continue;
			}
			e.properties.push_back( { kv.key, kv.value } );
		}
		m_entities.push_back( std::move( e ) );
	}

	ResetHistory(); // the loaded map is the unmodified base state
	return true;
}

} // namespace hammer::app
