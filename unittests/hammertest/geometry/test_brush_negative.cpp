//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Negative-provider check for the geometry.brush.v1 contract (RFC 0002).
//			RFC 0002/0005 require a deliberately-broken provider to prove the
//			conformance suite actually detects violations and is not vacuous.
//
//			The shared predicate below expresses the contract's core clause -- a
//			brush's faces are the CONVEX INTERSECTION of its side planes, so every
//			face vertex lies within the brush bounds. It is run against the real
//			BuildSolidFromPlanes (must pass) and against a BuggyBuilder that skips
//			the inter-plane clipping and therefore emits the unbounded seed quads
//			(must be detected).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/brush.h"

#include <cmath>
#include <cstdio>
#include <vector>

using hammer::geometry::BrushFace;
using hammer::geometry::BrushSolid;
using hammer::geometry::BuildSolidFromPlanes;
using hammer::geometry::Plane;
using hammer::geometry::Vec3d;

namespace
{

// The outward side planes of the cube [0,64]^3.
std::vector<Plane> CubePlanes()
{
	return {
	    { { 1, 0, 0 }, 64 }, { { -1, 0, 0 }, 0 }, { { 0, 1, 0 }, 64 },
	    { { 0, -1, 0 }, 0 }, { { 0, 0, 1 }, 64 }, { { 0, 0, -1 }, 0 },
	};
}

// A deliberately non-conforming provider: it emits one large seed quad per plane
// but NEVER clips it against the other planes, so the "faces" span +/-1e5 instead
// of being the convex intersection. It reuses the real builder for orientation so
// that ONLY the intersection clause is violated.
BrushSolid BuggyBuild( const std::vector<Plane> &planes )
{
	constexpr double kExtent = 1.0e5;
	BrushSolid solid;
	for ( const Plane &p : planes )
	{
		Vec3d up = ( std::fabs( p.normal.z ) < 0.9 ) ? Vec3d( 0, 0, 1 ) : Vec3d( 1, 0, 0 );
		// u = up x n, v = n x u (not normalised; magnitude irrelevant for the test).
		Vec3d u( up.y * p.normal.z - up.z * p.normal.y, up.z * p.normal.x - up.x * p.normal.z,
		         up.x * p.normal.y - up.y * p.normal.x );
		const double ul = std::sqrt( u.x * u.x + u.y * u.y + u.z * u.z );
		u = Vec3d( u.x / ul, u.y / ul, u.z / ul );
		Vec3d v( p.normal.y * u.z - p.normal.z * u.y, p.normal.z * u.x - p.normal.x * u.z,
		         p.normal.x * u.y - p.normal.y * u.x );
		const Vec3d c( p.normal.x * p.dist, p.normal.y * p.dist, p.normal.z * p.dist );

		BrushFace f;
		f.plane = p;
		f.vertices = {
		    { c.x - u.x * kExtent - v.x * kExtent, c.y - u.y * kExtent - v.y * kExtent,
		      c.z - u.z * kExtent - v.z * kExtent },
		    { c.x + u.x * kExtent - v.x * kExtent, c.y + u.y * kExtent - v.y * kExtent,
		      c.z + u.z * kExtent - v.z * kExtent },
		    { c.x + u.x * kExtent + v.x * kExtent, c.y + u.y * kExtent + v.y * kExtent,
		      c.z + u.z * kExtent + v.z * kExtent },
		};
		solid.faces.push_back( f );
	}
	return solid;
}

// The convex-intersection clause of geometry.brush.v1 as a provider-parameterized
// predicate: given the cube planes, every produced face vertex must lie within the
// cube bounds [0,64]^3 (with tolerance). Returns true when the provider conforms.
template <typename Builder> bool ConformsConvexIntersection( Builder build )
{
	const BrushSolid s = build( CubePlanes() );
	if ( s.faces.empty() )
	{
		return false;
	}
	for ( const BrushFace &f : s.faces )
	{
		for ( const Vec3d &vtx : f.vertices )
		{
			if ( vtx.x < -1.0 || vtx.x > 65.0 || vtx.y < -1.0 || vtx.y > 65.0 || vtx.z < -1.0
			     || vtx.z > 65.0 )
			{
				return false;
			}
		}
	}
	return true;
}

} // namespace

int main()
{
	const bool realConforms =
	    ConformsConvexIntersection( []( const std::vector<Plane> &p ) { return BuildSolidFromPlanes( p ); } );
	const bool buggyConforms = ConformsConvexIntersection( BuggyBuild );

	int failures = 0;

	if ( !realConforms )
	{
		std::printf( "FAIL: real BuildSolidFromPlanes violated the convex-intersection clause\n" );
		++failures;
	}
	if ( buggyConforms )
	{
		std::printf( "FAIL: conformance predicate did NOT detect the unclipped provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.geometry.brush negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf(
	    "hammer.geometry.brush negative: oracle detects violations (real passes, buggy caught)\n" );
	return 0;
}
