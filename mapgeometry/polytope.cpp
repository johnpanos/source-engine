//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/mapgeometry/polytope.h.
//
//=============================================================================//

#include "mapgeometry/polytope.h"

#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace mapgeometry
{

PlaneSide ClassifyPoints( const std::vector<Vec3d> &points, const Plane &plane, double epsilon )
{
	bool front = false;
	bool back = false;
	for ( const Vec3d &p : points )
	{
		const double d = PlaneDistance( plane, p );
		if ( d > epsilon )
		{
			front = true;
		}
		else if ( d < -epsilon )
		{
			back = true;
		}
	}
	if ( front && back )
	{
		return PlaneSide::Spanning;
	}
	if ( front )
	{
		return PlaneSide::Front;
	}
	if ( back )
	{
		return PlaneSide::Back;
	}
	return PlaneSide::On;
}

std::vector<Vec3d> SolidVertices( const BrushSolid &solid )
{
	std::vector<Vec3d> out;
	for ( const BrushFace &face : solid.faces )
	{
		for ( const Vec3d &v : face.vertices )
		{
			bool seen = false;
			for ( const Vec3d &o : out )
			{
				if ( NearlyEqual( o, v, 1.0e-6 ) )
				{
					seen = true;
					break;
				}
			}
			if ( !seen )
			{
				out.push_back( v );
			}
		}
	}
	return out;
}

Plane Flipped( const Plane &plane )
{
	Plane out;
	out.normal = -plane.normal;
	out.dist = -plane.dist;
	return out;
}

Plane PlaneThrough( const Vec3d &point, const Vec3d &normal )
{
	Plane out;
	out.normal = Normalize( normal );
	out.dist = Dot( out.normal, point );
	return out;
}

bool InsideAll( const std::vector<Plane> &planes, const Vec3d &p, double epsilon )
{
	for ( const Plane &plane : planes )
	{
		if ( PlaneDistance( plane, p ) > epsilon )
		{
			return false;
		}
	}
	return true;
}

bool IsClosedSolid( const std::vector<Plane> &planes )
{
	// BuildSolidFromPlanes clips each face from a finite quad, so an open plane
	// set still yields faces; they reach out to that quad's extent. A closed
	// solid stays inside the map coordinate range.
	constexpr double kMapLimit = 65536.0;
	const BrushSolid solid = BuildSolidFromPlanes( planes );
	if ( solid.faces.size() < 4 || !solid.bounded )
	{
		return false;
	}
	const Vec3d &a = solid.mins;
	const Vec3d &b = solid.maxs;
	return a.x > -kMapLimit && a.y > -kMapLimit && a.z > -kMapLimit && b.x < kMapLimit &&
	       b.y < kMapLimit && b.z < kMapLimit;
}

std::optional<std::vector<Plane>> ConvexHullPlanes(
    const std::vector<Vec3d> &points, double epsilon )
{
	if ( points.size() < 4 )
	{
		return std::nullopt;
	}

	// Brute force over triples: fine for brush-sized point sets (tens of points).
	// A triple spans a hull face when every point lies on or behind its plane.
	std::vector<Plane> planes;
	const std::size_t n = points.size();
	for ( std::size_t i = 0; i < n; ++i )
	{
		for ( std::size_t j = i + 1; j < n; ++j )
		{
			for ( std::size_t k = j + 1; k < n; ++k )
			{
				const Vec3d normalRaw = Cross( points[j] - points[i], points[k] - points[i] );
				if ( Length( normalRaw ) < 1.0e-9 )
				{
					continue;
				}
				for ( int orientation = 0; orientation < 2; ++orientation )
				{
					const Vec3d normal =
					    Normalize( orientation == 0 ? normalRaw : -normalRaw );
					const Plane plane = PlaneThrough( points[i], normal );
					bool allBehind = true;
					bool anyStrictlyBehind = false;
					for ( const Vec3d &p : points )
					{
						const double d = PlaneDistance( plane, p );
						if ( d > epsilon )
						{
							allBehind = false;
							break;
						}
						if ( d < -epsilon )
						{
							anyStrictlyBehind = true;
						}
					}
					if ( !allBehind || !anyStrictlyBehind )
					{
						continue;
					}
					bool duplicate = false;
					for ( const Plane &existing : planes )
					{
						if ( Dot( existing.normal, plane.normal ) > 1.0 - 1.0e-9 &&
						     std::fabs( existing.dist - plane.dist ) < epsilon )
						{
							duplicate = true;
							break;
						}
					}
					if ( !duplicate )
					{
						planes.push_back( plane );
					}
				}
			}
		}
	}
	if ( planes.size() < 4 )
	{
		return std::nullopt;
	}
	return planes;
}

void SnapNearIntegers( BrushSolid &solid, double epsilon )
{
	auto snap = [epsilon]( double v )
	{
		const double r = std::round( v );
		return std::fabs( v - r ) <= epsilon ? r : v;
	};
	bool first = true;
	for ( BrushFace &face : solid.faces )
	{
		for ( Vec3d &v : face.vertices )
		{
			v = Vec3d( snap( v.x ), snap( v.y ), snap( v.z ) );
			if ( first )
			{
				solid.mins = v;
				solid.maxs = v;
				first = false;
			}
			solid.mins = Vec3d( std::min( solid.mins.x, v.x ), std::min( solid.mins.y, v.y ),
			    std::min( solid.mins.z, v.z ) );
			solid.maxs = Vec3d( std::max( solid.maxs.x, v.x ), std::max( solid.maxs.y, v.y ),
			    std::max( solid.maxs.z, v.z ) );
		}
	}
	solid.bounded = !first;
}

namespace
{

// Projection interval of a vertex set on an axis.
void Project( const std::vector<Vec3d> &points, const Vec3d &axis, double &lo, double &hi )
{
	lo = 1.0e300;
	hi = -1.0e300;
	for ( const Vec3d &p : points )
	{
		const double d = Dot( p, axis );
		lo = d < lo ? d : lo;
		hi = d > hi ? d : hi;
	}
}

bool Separates( const std::vector<Vec3d> &a, const std::vector<Vec3d> &b, const Vec3d &axisRaw,
    double epsilon )
{
	const double len = Length( axisRaw );
	if ( len < 1.0e-9 )
	{
		return false;
	}
	const Vec3d axis = axisRaw / len;
	double aLo = 0.0;
	double aHi = 0.0;
	double bLo = 0.0;
	double bHi = 0.0;
	Project( a, axis, aLo, aHi );
	Project( b, axis, bLo, bHi );
	return aHi <= bLo + epsilon || bHi <= aLo + epsilon;
}

} // namespace

bool SolidsOverlap( const BrushSolid &a, const BrushSolid &b, double epsilon )
{
	if ( a.faces.empty() || b.faces.empty() )
	{
		return false;
	}
	const std::vector<Vec3d> va = SolidVertices( a );
	const std::vector<Vec3d> vb = SolidVertices( b );
	for ( const BrushFace &f : a.faces )
	{
		if ( Separates( va, vb, f.plane.normal, epsilon ) )
		{
			return false;
		}
	}
	for ( const BrushFace &f : b.faces )
	{
		if ( Separates( va, vb, f.plane.normal, epsilon ) )
		{
			return false;
		}
	}
	// Edge-edge axes.
	auto edges = []( const BrushSolid &s )
	{
		std::vector<Vec3d> out;
		for ( const BrushFace &f : s.faces )
		{
			for ( std::size_t i = 0; i < f.vertices.size(); ++i )
			{
				out.push_back( f.vertices[( i + 1 ) % f.vertices.size()] - f.vertices[i] );
			}
		}
		return out;
	};
	const std::vector<Vec3d> ea = edges( a );
	const std::vector<Vec3d> eb = edges( b );
	for ( const Vec3d &x : ea )
	{
		for ( const Vec3d &y : eb )
		{
			if ( Separates( va, vb, Cross( x, y ), epsilon ) )
			{
				return false;
			}
		}
	}
	return true;
}

} // namespace mapgeometry
