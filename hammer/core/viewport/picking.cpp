//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hit testing for the Hammer viewports (RFC 0002, hammer.viewport).
//			See public/hammer/viewport/picking.h.
//
//=============================================================================//

#include "hammer/viewport/picking.h"

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace hammer::viewport
{

using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

constexpr double kParallelEpsilon = 1.0e-12;

bool Finite( const Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

double NonNegative( double value )
{
	return std::isfinite( value ) && value > 0.0 ? value : 0.0;
}

// An axis-aligned screen rectangle (min <= max).
struct Rect
{
	double x0 = 0.0;
	double y0 = 0.0;
	double x1 = 0.0;
	double y1 = 0.0;

	double Area() const { return ( x1 - x0 ) * ( y1 - y0 ); }
};

Rect Normalized( ScreenPoint a, ScreenPoint b )
{
	return {
	    std::min( a.x, b.x ), std::min( a.y, b.y ), std::max( a.x, b.x ), std::max( a.y, b.y ) };
}

Rect ProjectBox( const Camera2D &camera, const scene::Box &box )
{
	return Normalized( camera.WorldToScreen( box.mins ), camera.WorldToScreen( box.maxs ) );
}

double DistanceToRect( const Rect &rect, double px, double py )
{
	const double dx = std::max( { rect.x0 - px, 0.0, px - rect.x1 } );
	const double dy = std::max( { rect.y0 - py, 0.0, py - rect.y1 } );
	return std::sqrt( dx * dx + dy * dy );
}

double DistanceToSegment( ScreenPoint a, ScreenPoint b, double px, double py )
{
	const double ex = b.x - a.x;
	const double ey = b.y - a.y;
	const double lengthSq = ex * ex + ey * ey;
	double t = 0.0;
	if ( lengthSq > 0.0 )
	{
		t = std::clamp( ( ( px - a.x ) * ex + ( py - a.y ) * ey ) / lengthSq, 0.0, 1.0 );
	}
	const double dx = a.x + ex * t - px;
	const double dy = a.y + ey * t - py;
	return std::sqrt( dx * dx + dy * dy );
}

// Entities that own at least one solid (brush entities) in the document.
std::set<ObjectId> BrushEntities( const scene::DocumentReader &doc )
{
	std::set<ObjectId> owners;
	for ( ObjectId id : doc.SolidIds() )
	{
		const scene::Solid *solid = doc.FindSolid( id );
		if ( solid && solid->owner.IsValid() )
		{
			owners.insert( solid->owner );
		}
	}
	return owners;
}

bool Before2D( const Hit2D &a, const Hit2D &b )
{
	if ( a.distance != b.distance )
	{
		return a.distance < b.distance;
	}
	if ( a.area != b.area )
	{
		return a.area < b.area;
	}
	return a.object < b.object;
}

bool BeforeRay( const RayHit &a, const RayHit &b )
{
	if ( a.t != b.t )
	{
		return a.t < b.t;
	}
	return a.object < b.object;
}

// Ray against a convex volume given by outward planes: the entering t and
// the entering plane index, or nothing (miss, or the origin is inside).
struct Entry
{
	double t = 0.0;
	int plane = -1;
};

template <typename PlaneAt>
std::optional<Entry> EnterConvex(
    int planeCount, PlaneAt planeAt, const Vec3d &origin, const Vec3d &direction )
{
	double tEnter = -std::numeric_limits<double>::infinity();
	double tExit = std::numeric_limits<double>::infinity();
	int enterPlane = -1;
	for ( int i = 0; i < planeCount; ++i )
	{
		const mapgeometry::Plane plane = planeAt( i );
		const double denom = mapgeometry::Dot( plane.normal, direction );
		const double side = mapgeometry::PlaneDistance( plane, origin );
		if ( std::fabs( denom ) < kParallelEpsilon )
		{
			if ( side > 0.0 )
			{
				return std::nullopt; // parallel and outside
			}
			continue;
		}
		const double t = -side / denom;
		if ( denom < 0.0 )
		{
			if ( t > tEnter )
			{
				tEnter = t;
				enterPlane = i;
			}
		}
		else
		{
			tExit = std::min( tExit, t );
		}
	}
	if ( enterPlane < 0 || tEnter > tExit || tEnter < 0.0 )
	{
		return std::nullopt;
	}
	return Entry{ tEnter, enterPlane };
}

mapgeometry::Plane BoxPlane( const scene::Box &box, int index )
{
	// 0..2: +X +Y +Z sides (maxs), 3..5: -X -Y -Z sides (mins).
	const int axis = index % 3;
	Vec3d normal;
	if ( index < 3 )
	{
		mapgeometry::SetComponent( normal, axis, 1.0 );
		return { normal, mapgeometry::Component( box.maxs, axis ) };
	}
	mapgeometry::SetComponent( normal, axis, -1.0 );
	return { normal, -mapgeometry::Component( box.mins, axis ) };
}

} // namespace

ObjectId SelectionTarget( const Hit2D &hit )
{
	return hit.owner.IsValid() ? hit.owner : hit.object;
}

ObjectId SelectionTarget( const RayHit &hit )
{
	return hit.owner.IsValid() ? hit.owner : hit.object;
}

std::vector<Hit2D> Pick2D( const scene::DocumentReader &doc, const Camera2D &camera, double px,
    double py, const PickOptions &options )
{
	std::vector<Hit2D> hits;
	if ( !std::isfinite( px ) || !std::isfinite( py ) || !camera.HasArea() )
	{
		return hits;
	}
	const double edgeTolerance = NonNegative( options.edgeTolerancePixels );
	const double handleRadius = NonNegative( options.centerHandlePixels );

	if ( options.solids )
	{
		for ( ObjectId id : doc.SolidIds() )
		{
			const scene::Solid *solid = doc.FindSolid( id );
			if ( !solid || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const mapgeometry::BrushSolid geometry = scene::BuildGeometry( *solid );
			if ( geometry.faces.empty() )
			{
				continue;
			}
			double edgeDistance = std::numeric_limits<double>::infinity();
			for ( const WorldEdge &edge : UniqueEdges( geometry ) )
			{
				edgeDistance =
				    std::min( edgeDistance, DistanceToSegment( camera.WorldToScreen( edge.a ),
				                                camera.WorldToScreen( edge.b ), px, py ) );
			}
			const scene::Box bounds{ geometry.mins, geometry.maxs };
			const Rect rect = ProjectBox( camera, bounds );
			const ScreenPoint center = camera.WorldToScreen( bounds.Center() );
			const double centerDistance = std::hypot( center.x - px, center.y - py );

			const bool onEdge = edgeDistance <= edgeTolerance;
			const bool onCenter = centerDistance <= handleRadius;
			if ( !onEdge && !onCenter )
			{
				continue;
			}
			Hit2D hit;
			hit.object = id;
			hit.owner = solid->owner;
			hit.area = rect.Area();
			if ( onEdge && ( !onCenter || edgeDistance <= centerDistance ) )
			{
				hit.kind = HitKind::SolidEdge;
				hit.distance = edgeDistance;
			}
			else
			{
				hit.kind = HitKind::SolidCenter;
				hit.distance = centerDistance;
			}
			hits.push_back( hit );
		}
	}

	if ( options.entities )
	{
		const std::set<ObjectId> brushEntities = BrushEntities( doc );
		for ( ObjectId id : doc.EntityIds() )
		{
			const scene::Entity *entity = doc.FindEntity( id );
			if ( !entity || brushEntities.count( id ) || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const std::optional<scene::Box> marker =
			    EntityMarkerBox( *entity, options.catalog, options.pointHalfSize );
			if ( !marker )
			{
				continue;
			}
			const Rect rect = ProjectBox( camera, *marker );
			const double distance = DistanceToRect( rect, px, py );
			if ( distance > edgeTolerance )
			{
				continue;
			}
			hits.push_back( { id, ObjectId{}, HitKind::EntityMarker, distance, rect.Area() } );
		}
	}

	std::sort( hits.begin(), hits.end(), Before2D );
	return hits;
}

std::vector<RayHit> PickRay( const scene::DocumentReader &doc, const Vec3d &origin,
    const Vec3d &direction, const PickOptions &options )
{
	std::vector<RayHit> hits;
	const Vec3d dir = mapgeometry::Normalize( direction );
	if ( !Finite( origin ) || !Finite( direction ) || dir == Vec3d() )
	{
		return hits;
	}
	const double maxDistance = std::isnan( options.maxDistance ) ? 0.0 : options.maxDistance;

	if ( options.solids )
	{
		for ( ObjectId id : doc.SolidIds() )
		{
			const scene::Solid *solid = doc.FindSolid( id );
			if ( !solid || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const mapgeometry::BrushSolid geometry = scene::BuildGeometry( *solid );
			if ( geometry.faces.empty() )
			{
				continue;
			}
			const std::optional<Entry> entry = EnterConvex(
			    static_cast<int>( geometry.faces.size() ),
			    [&]( int i )
			    {
				    return geometry.faces[static_cast<std::size_t>( i )].plane;
			    },
			    origin, dir );
			if ( !entry || entry->t > maxDistance )
			{
				continue;
			}
			const mapgeometry::BrushFace &face =
			    geometry.faces[static_cast<std::size_t>( entry->plane )];
			RayHit hit;
			hit.object = id;
			hit.owner = solid->owner;
			hit.kind = HitKind::SolidFace;
			hit.t = entry->t;
			hit.point = origin + dir * entry->t;
			hit.normal = face.plane.normal;
			hit.face.solid = id;
			if ( face.sourcePlane >= 0 &&
			     static_cast<std::size_t>( face.sourcePlane ) < solid->sides.size() )
			{
				hit.face.side = solid->sides[static_cast<std::size_t>( face.sourcePlane )].vmfId;
			}
			hits.push_back( hit );
		}
	}

	if ( options.entities )
	{
		const std::set<ObjectId> brushEntities = BrushEntities( doc );
		for ( ObjectId id : doc.EntityIds() )
		{
			const scene::Entity *entity = doc.FindEntity( id );
			if ( !entity || brushEntities.count( id ) || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const std::optional<scene::Box> marker =
			    EntityMarkerBox( *entity, options.catalog, options.pointHalfSize );
			if ( !marker )
			{
				continue;
			}
			const scene::Box box = *marker;
			const std::optional<Entry> entry = EnterConvex(
			    6,
			    [&]( int i )
			    {
				    return BoxPlane( box, i );
			    },
			    origin, dir );
			if ( !entry || entry->t > maxDistance )
			{
				continue;
			}
			RayHit hit;
			hit.object = id;
			hit.kind = HitKind::EntityMarker;
			hit.t = entry->t;
			hit.point = origin + dir * entry->t;
			hit.normal = BoxPlane( box, entry->plane ).normal;
			hits.push_back( hit );
		}
	}

	std::sort( hits.begin(), hits.end(), BeforeRay );
	return hits;
}

std::vector<ObjectId> MarqueeSelect( const scene::DocumentReader &doc, const Camera2D &camera,
    const ScreenRect &screenRect, MarqueeMode mode, const MarqueeOptions &options )
{
	std::vector<ObjectId> ids;
	if ( !std::isfinite( screenRect.a.x ) || !std::isfinite( screenRect.a.y ) ||
	     !std::isfinite( screenRect.b.x ) || !std::isfinite( screenRect.b.y ) )
	{
		return ids;
	}
	const Rect area = Normalized( screenRect.a, screenRect.b );
	const auto qualifies = [&]( const Rect &r )
	{
		if ( mode == MarqueeMode::Inside )
		{
			return r.x0 >= area.x0 && r.y0 >= area.y0 && r.x1 <= area.x1 && r.y1 <= area.y1;
		}
		return r.x0 <= area.x1 && r.x1 >= area.x0 && r.y0 <= area.y1 && r.y1 >= area.y0;
	};

	// Per brush entity: whether any / every shown solid qualified.
	struct OwnerTally
	{
		bool any = false;
		bool all = true;
	};
	std::map<ObjectId, OwnerTally> owners;

	if ( options.solids )
	{
		for ( ObjectId id : doc.SolidIds() )
		{
			const scene::Solid *solid = doc.FindSolid( id );
			if ( !solid || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const std::optional<scene::Box> bounds = scene::SolidBounds( *solid );
			if ( !bounds )
			{
				continue;
			}
			const bool ok = qualifies( ProjectBox( camera, *bounds ) );
			if ( options.ownersForBrushEntities && solid->owner.IsValid() )
			{
				OwnerTally &tally = owners[solid->owner];
				tally.any = tally.any || ok;
				tally.all = tally.all && ok;
			}
			else if ( ok )
			{
				ids.push_back( id );
			}
		}
		for ( const auto &[owner, tally] : owners )
		{
			if ( mode == MarqueeMode::Inside ? tally.all : tally.any )
			{
				ids.push_back( owner );
			}
		}
	}

	if ( options.entities )
	{
		const std::set<ObjectId> brushEntities = BrushEntities( doc );
		for ( ObjectId id : doc.EntityIds() )
		{
			const scene::Entity *entity = doc.FindEntity( id );
			if ( !entity || brushEntities.count( id ) || !IsShown( options.visible, doc, id ) )
			{
				continue;
			}
			const std::optional<scene::Box> marker =
			    EntityMarkerBox( *entity, options.catalog, options.pointHalfSize );
			if ( marker && qualifies( ProjectBox( camera, *marker ) ) )
			{
				ids.push_back( id );
			}
		}
	}

	std::sort( ids.begin(), ids.end() );
	ids.erase( std::unique( ids.begin(), ids.end() ), ids.end() );
	return ids;
}

} // namespace hammer::viewport
