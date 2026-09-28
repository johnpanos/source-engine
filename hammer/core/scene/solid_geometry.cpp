//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/solid_geometry.h.
//
//=============================================================================//

#include "hammer/scene/solid_geometry.h"

#include "mapgeometry/polytope.h"
#include "mapgeometry/texture_axes.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>

namespace hammer::scene
{

using mapgeometry::Vec3d;

Vec3d Box::Center() const
{
	return ( mins + maxs ) * 0.5;
}

Vec3d Box::Size() const
{
	return maxs - mins;
}

bool Box::Contains( const Vec3d &p, double epsilon ) const
{
	return p.x >= mins.x - epsilon && p.x <= maxs.x + epsilon && p.y >= mins.y - epsilon &&
	       p.y <= maxs.y + epsilon && p.z >= mins.z - epsilon && p.z <= maxs.z + epsilon;
}

bool Box::Intersects( const Box &o, double epsilon ) const
{
	return mins.x <= o.maxs.x + epsilon && maxs.x >= o.mins.x - epsilon &&
	       mins.y <= o.maxs.y + epsilon && maxs.y >= o.mins.y - epsilon &&
	       mins.z <= o.maxs.z + epsilon && maxs.z >= o.mins.z - epsilon;
}

bool Box::Encloses( const Box &o, double epsilon ) const
{
	return Contains( o.mins, epsilon ) && Contains( o.maxs, epsilon );
}

void Box::Extend( const Vec3d &p )
{
	mins = Vec3d( std::min( mins.x, p.x ), std::min( mins.y, p.y ), std::min( mins.z, p.z ) );
	maxs = Vec3d( std::max( maxs.x, p.x ), std::max( maxs.y, p.y ), std::max( maxs.z, p.z ) );
}

void Box::Extend( const Box &other )
{
	Extend( other.mins );
	Extend( other.maxs );
}

Box PointBox( const Vec3d &p )
{
	return Box{ p, p };
}

mapgeometry::BrushSolid BuildGeometry( const Solid &solid )
{
	std::vector<mapgeometry::Plane> planes;
	std::vector<std::string> materials;
	planes.reserve( solid.sides.size() );
	materials.reserve( solid.sides.size() );
	for ( const Side &side : solid.sides )
	{
		planes.push_back( side.Plane() );
		materials.push_back( side.texture.material );
	}
	mapgeometry::BrushSolid out =
	    mapgeometry::BuildSolidFromPlanes( planes, materials, static_cast<int>( solid.vmfId ) );
	mapgeometry::SnapNearIntegers( out );
	return out;
}

std::optional<Box> SolidBounds( const Solid &solid )
{
	const mapgeometry::BrushSolid geometry = BuildGeometry( solid );
	if ( !geometry.bounded )
	{
		return std::nullopt;
	}
	return Box{ geometry.mins, geometry.maxs };
}

std::optional<SolidRayHit> RayEnterSolid(
    const Solid &solid, const Vec3d &origin, const Vec3d &direction )
{
	const mapgeometry::BrushSolid geometry = BuildGeometry( solid );
	std::vector<mapgeometry::Plane> planes;
	for ( const mapgeometry::BrushFace &face : geometry.faces )
	{
		planes.push_back( face.plane );
	}
	const std::optional<mapgeometry::RayEntry> entry =
	    mapgeometry::RayEnterConvex( planes, origin, direction );
	if ( !entry )
	{
		return std::nullopt;
	}
	const mapgeometry::BrushFace &face = geometry.faces[static_cast<std::size_t>( entry->plane )];
	SolidRayHit hit;
	hit.t = entry->t;
	hit.side = static_cast<std::size_t>( face.sourcePlane );
	hit.point = origin + direction * entry->t;
	hit.normal = face.plane.normal;
	return hit;
}

std::optional<std::array<Vec3d, 4>> QuadCorners( const Solid &solid, std::size_t sideIndex )
{
	for ( const mapgeometry::BrushFace &face : BuildGeometry( solid ).faces )
	{
		if ( static_cast<std::size_t>( face.sourcePlane ) != sideIndex )
		{
			continue;
		}
		if ( face.vertices.size() != 4 )
		{
			return std::nullopt;
		}
		return std::array<Vec3d, 4>{
		    face.vertices[0], face.vertices[1], face.vertices[2], face.vertices[3] };
	}
	return std::nullopt;
}

std::array<Vec3d, 3> PointsFromPolygon( const std::vector<Vec3d> &ccw )
{
	// Side::Plane() takes (p0 - p1) x (p2 - p1); for a counter-clockwise polygon
	// that is outward when the points run backwards: v2, v1, v0.
	return { ccw[2], ccw[1], ccw[0] };
}

std::array<Vec3d, 3> PointsFromPlane( const mapgeometry::Plane &plane )
{
	const Vec3d n = mapgeometry::Normalize( plane.normal );
	const Vec3d helper = std::fabs( n.z ) < 0.9 ? Vec3d( 0, 0, 1 ) : Vec3d( 1, 0, 0 );
	const Vec3d a = mapgeometry::Normalize( mapgeometry::Cross( helper, n ) );
	const Vec3d b = mapgeometry::Cross( n, a );
	const Vec3d origin = n * plane.dist;
	constexpr double kSpan = 128.0;
	// a, b, n is right-handed, so origin, +a, +b is counter-clockwise from outside.
	return PointsFromPolygon( { origin, origin + a * kSpan, origin + b * kSpan } );
}

std::optional<Solid> NormalizeSides( const Solid &solid )
{
	std::vector<mapgeometry::Plane> planes;
	for ( const Side &side : solid.sides )
	{
		planes.push_back( side.Plane() );
	}
	if ( !mapgeometry::IsClosedSolid( planes ) )
	{
		return std::nullopt;
	}
	const mapgeometry::BrushSolid geometry = BuildGeometry( solid );
	Solid out = solid;
	out.sides.clear();
	for ( const mapgeometry::BrushFace &face : geometry.faces )
	{
		Side side = solid.sides[static_cast<std::size_t>( face.sourcePlane )];
		side.points = PointsFromPolygon( face.vertices );
		out.sides.push_back( std::move( side ) );
	}
	return out;
}

FaceTexture WorldAlignedTexture( const FaceTexture &texture, const Vec3d &normal )
{
	FaceTexture out = texture;
	const mapgeometry::TextureAxes axes = mapgeometry::WorldAlignedTextureAxes( normal );
	out.u.axis = axes.u;
	out.v.axis = axes.v;
	out.u.shift = 0.0;
	out.v.shift = 0.0;
	out.rotation = 0.0;
	return out;
}

Solid MakeBoxSolid( const Box &box, const FaceTexture &texture )
{
	const Vec3d &a = box.mins;
	const Vec3d &b = box.maxs;
	struct FaceSpec
	{
		Vec3d normal;
		std::vector<Vec3d> ccw;
	};
	// Counter-clockwise from outside for each of the six faces.
	const FaceSpec faces[6] = {
	    { Vec3d( 0, 0, 1 ), { Vec3d( a.x, a.y, b.z ), Vec3d( b.x, a.y, b.z ),
	                            Vec3d( b.x, b.y, b.z ), Vec3d( a.x, b.y, b.z ) } },
	    { Vec3d( 0, 0, -1 ), { Vec3d( a.x, b.y, a.z ), Vec3d( b.x, b.y, a.z ),
	                             Vec3d( b.x, a.y, a.z ), Vec3d( a.x, a.y, a.z ) } },
	    { Vec3d( -1, 0, 0 ), { Vec3d( a.x, a.y, a.z ), Vec3d( a.x, a.y, b.z ),
	                             Vec3d( a.x, b.y, b.z ), Vec3d( a.x, b.y, a.z ) } },
	    { Vec3d( 1, 0, 0 ), { Vec3d( b.x, b.y, a.z ), Vec3d( b.x, b.y, b.z ),
	                            Vec3d( b.x, a.y, b.z ), Vec3d( b.x, a.y, a.z ) } },
	    { Vec3d( 0, -1, 0 ), { Vec3d( b.x, a.y, a.z ), Vec3d( b.x, a.y, b.z ),
	                             Vec3d( a.x, a.y, b.z ), Vec3d( a.x, a.y, a.z ) } },
	    { Vec3d( 0, 1, 0 ), { Vec3d( a.x, b.y, a.z ), Vec3d( a.x, b.y, b.z ),
	                            Vec3d( b.x, b.y, b.z ), Vec3d( b.x, b.y, a.z ) } },
	};
	Solid solid;
	for ( const FaceSpec &f : faces )
	{
		Side side;
		side.points = PointsFromPolygon( f.ccw );
		side.texture = WorldAlignedTexture( texture, f.normal );
		solid.sides.push_back( std::move( side ) );
	}
	return solid;
}

} // namespace hammer::scene
