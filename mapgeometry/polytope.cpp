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
	// Incremental hull: seed a tetrahedron, then add each point outside the
	// current hull by removing the faces it sees and stitching the horizon to
	// it. O(n^2) for n points; triangles are merged into planes at the end.
	const std::size_t n = points.size();
	if ( n < 4 )
	{
		return std::nullopt;
	}

	// Seed: two far-apart points, the point farthest from their line, and the
	// point farthest from that plane.
	std::size_t i0 = 0;
	std::size_t i1 = 0;
	double best = -1.0;
	for ( std::size_t i = 1; i < n; ++i )
	{
		const double d = Length( points[i] - points[i0] );
		if ( d > best )
		{
			best = d;
			i1 = i;
		}
	}
	if ( best <= epsilon )
	{
		return std::nullopt;
	}
	const Vec3d lineDir = Normalize( points[i1] - points[i0] );
	std::size_t i2 = 0;
	best = -1.0;
	for ( std::size_t i = 0; i < n; ++i )
	{
		const Vec3d rel = points[i] - points[i0];
		const double d = Length( rel - lineDir * Dot( rel, lineDir ) );
		if ( d > best )
		{
			best = d;
			i2 = i;
		}
	}
	if ( best <= epsilon )
	{
		return std::nullopt;
	}
	const Vec3d seedNormal = Normalize( Cross( points[i1] - points[i0], points[i2] - points[i0] ) );
	std::size_t i3 = 0;
	best = -1.0;
	for ( std::size_t i = 0; i < n; ++i )
	{
		const double d = std::fabs( Dot( points[i] - points[i0], seedNormal ) );
		if ( d > best )
		{
			best = d;
			i3 = i;
		}
	}
	if ( best <= epsilon )
	{
		return std::nullopt; // coplanar
	}

	struct Face
	{
		std::size_t a, b, c;
		Plane plane;
		bool alive = true;
	};
	const Vec3d inside = ( points[i0] + points[i1] + points[i2] + points[i3] ) / 4.0;
	std::vector<Face> faces;
	auto addFace = [&]( std::size_t a, std::size_t b, std::size_t c )
	{
		Face f{ a, b, c, {}, true };
		Vec3d normal = Normalize( Cross( points[b] - points[a], points[c] - points[a] ) );
		if ( Dot( normal, points[a] - inside ) < 0.0 )
		{
			std::swap( f.b, f.c );
			normal = -normal;
		}
		f.plane.normal = normal;
		f.plane.dist = Dot( normal, points[f.a] );
		faces.push_back( f );
	};
	addFace( i0, i1, i2 );
	addFace( i0, i1, i3 );
	addFace( i0, i2, i3 );
	addFace( i1, i2, i3 );

	for ( std::size_t p = 0; p < n; ++p )
	{
		if ( p == i0 || p == i1 || p == i2 || p == i3 )
		{
			continue;
		}
		std::vector<std::size_t> visible;
		for ( std::size_t f = 0; f < faces.size(); ++f )
		{
			if ( faces[f].alive && PlaneDistance( faces[f].plane, points[p] ) > epsilon )
			{
				visible.push_back( f );
			}
		}
		if ( visible.empty() )
		{
			continue; // inside or on the hull
		}
		// Horizon: directed edges of visible faces whose reverse edge is not on
		// another visible face.
		std::vector<std::pair<std::size_t, std::size_t>> edges;
		for ( std::size_t f : visible )
		{
			const Face &face = faces[f];
			edges.push_back( { face.a, face.b } );
			edges.push_back( { face.b, face.c } );
			edges.push_back( { face.c, face.a } );
		}
		for ( std::size_t f : visible )
		{
			faces[f].alive = false;
		}
		for ( const auto &e : edges )
		{
			bool shared = false;
			for ( const auto &o : edges )
			{
				if ( o.first == e.second && o.second == e.first )
				{
					shared = true;
					break;
				}
			}
			if ( !shared )
			{
				addFace( e.first, e.second, p );
			}
		}
	}

	std::vector<Plane> planes;
	for ( const Face &face : faces )
	{
		if ( !face.alive )
		{
			continue;
		}
		bool duplicate = false;
		for ( const Plane &existing : planes )
		{
			if ( Dot( existing.normal, face.plane.normal ) > 1.0 - 1.0e-9 &&
			     std::fabs( existing.dist - face.plane.dist ) < epsilon )
			{
				duplicate = true;
				break;
			}
		}
		if ( !duplicate )
		{
			planes.push_back( face.plane );
		}
	}
	if ( planes.size() < 4 )
	{
		return std::nullopt;
	}
	return planes;
}

std::optional<RayEntry> RayEnterConvex(
    const std::vector<Plane> &planes, const Vec3d &origin, const Vec3d &direction )
{
	constexpr double kParallelEpsilon = 1.0e-12;
	double tEnter = -1.0e300;
	double tExit = 1.0e300;
	int enterPlane = -1;
	for ( std::size_t i = 0; i < planes.size(); ++i )
	{
		const double denom = Dot( planes[i].normal, direction );
		const double side = PlaneDistance( planes[i], origin );
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
				enterPlane = static_cast<int>( i );
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
	return RayEntry{ tEnter, enterPlane };
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

bool Separates(
    const std::vector<Vec3d> &a, const std::vector<Vec3d> &b, const Vec3d &axisRaw, double epsilon )
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
