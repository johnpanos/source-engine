//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: world.map-geometry polytope conformance (RFC 0002, R08 domain
//			logic): plane-side classification, containment, closed-solid
//			detection, convex hulls (the vertex tool's rebuild) and the
//			separating-axis overlap test, plus BrushFace::sourcePlane mapping.
//			Negative checks: degenerate hull input and open plane sets.
//
//=============================================================================//

#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>

using namespace mapgeometry;

namespace
{

std::vector<Plane> BoxPlanes( const Vec3d &a, const Vec3d &b )
{
	return {
	    PlaneThrough( b, Vec3d( 1, 0, 0 ) ),
	    PlaneThrough( a, Vec3d( -1, 0, 0 ) ),
	    PlaneThrough( b, Vec3d( 0, 1, 0 ) ),
	    PlaneThrough( a, Vec3d( 0, -1, 0 ) ),
	    PlaneThrough( b, Vec3d( 0, 0, 1 ) ),
	    PlaneThrough( a, Vec3d( 0, 0, -1 ) ),
	};
}

} // namespace

int main()
{
	testing::Checks checks;

	const Plane floor = PlaneThrough( Vec3d( 0, 0, 0 ), Vec3d( 0, 0, 1 ) );
	checks.That(
	    ClassifyPoints( { Vec3d( 0, 0, 1 ), Vec3d( 5, 5, 2 ) }, floor ) == PlaneSide::Front,
	    "points above are in front" );
	checks.That(
	    ClassifyPoints( { Vec3d( 0, 0, -1 ), Vec3d( 0, 0, 0 ) }, floor ) == PlaneSide::Back,
	    "points below or on are behind" );
	checks.That( ClassifyPoints( { Vec3d( 0, 0, 0.001 ) }, floor ) == PlaneSide::On,
	    "within epsilon is on the plane" );
	checks.That(
	    ClassifyPoints( { Vec3d( 0, 0, -1 ), Vec3d( 0, 0, 1 ) }, floor ) == PlaneSide::Spanning,
	    "both sides span" );
	checks.That( Flipped( floor ).normal == Vec3d( 0, 0, -1 ) && Flipped( floor ).dist == 0,
	    "flip reverses the plane" );

	const std::vector<Plane> box = BoxPlanes( Vec3d( 0, 0, 0 ), Vec3d( 64, 32, 16 ) );
	checks.That( InsideAll( box, Vec3d( 10, 10, 10 ) ), "interior point is inside" );
	checks.That( InsideAll( box, Vec3d( 64, 32, 16 ) ), "corner is on the boundary" );
	checks.That( !InsideAll( box, Vec3d( 65, 0, 0 ) ), "outside point is rejected" );
	checks.That( IsClosedSolid( box ), "six box planes close a solid" );
	std::vector<Plane> open( box.begin(), box.end() - 1 );
	checks.That( !IsClosedSolid( open ), "five box planes are open (negative)" );

	// sourcePlane maps built faces back to their input plane; a redundant plane
	// produces no face.
	{
		std::vector<Plane> planes = box;
		planes.push_back( PlaneThrough( Vec3d( 100, 0, 0 ), Vec3d( 1, 0, 0 ) ) ); // redundant
		const BrushSolid solid = BuildSolidFromPlanes( planes );
		checks.Equal( solid.faces.size(), std::size_t( 6 ), "redundant plane gives no face" );
		bool mapped = true;
		for ( const BrushFace &f : solid.faces )
		{
			mapped = mapped && f.sourcePlane >= 0 && f.sourcePlane < 6 &&
			         Dot( f.plane.normal, planes[f.sourcePlane].normal ) > 0.999;
		}
		checks.That( mapped, "each face names its input plane" );
		checks.Equal(
		    SolidVertices( solid ).size(), std::size_t( 8 ), "a box has eight unique vertices" );
	}

	// Convex hull: a cube's corners plus an interior point give six planes.
	{
		std::vector<Vec3d> pts;
		for ( int i = 0; i < 8; ++i )
			pts.push_back( Vec3d( i & 1 ? 16 : 0, i & 2 ? 16 : 0, i & 4 ? 16 : 0 ) );
		pts.push_back( Vec3d( 8, 8, 8 ) );
		const std::optional<std::vector<Plane>> hull = ConvexHullPlanes( pts );
		checks.That( hull.has_value() && hull->size() == 6, "cube hull has six planes" );
		if ( hull )
		{
			const BrushSolid solid = BuildSolidFromPlanes( *hull );
			checks.Equal( solid.faces.size(), std::size_t( 6 ), "hull planes rebuild the cube" );
			checks.That( solid.bounded && solid.maxs == Vec3d( 16, 16, 16 ), "hull bounds" );
		}
		// A pyramid: four base corners and an apex.
		const std::optional<std::vector<Plane>> pyr = ConvexHullPlanes( { Vec3d( 0, 0, 0 ),
		    Vec3d( 16, 0, 0 ), Vec3d( 16, 16, 0 ), Vec3d( 0, 16, 0 ), Vec3d( 8, 8, 16 ) } );
		checks.That( pyr.has_value() && pyr->size() == 5, "pyramid hull has five planes" );
		// Negative: coplanar and too few points.
		checks.That( !ConvexHullPlanes( { Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 0 ),
		                 Vec3d( 1, 1, 0 ) } ),
		    "coplanar points have no hull" );
		checks.That( !ConvexHullPlanes( { Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 1 ) } ),
		    "three points have no hull" );
	}

	// Overlap: separated, touching and interpenetrating boxes.
	{
		const BrushSolid a =
		    BuildSolidFromPlanes( BoxPlanes( Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) ) );
		const BrushSolid b =
		    BuildSolidFromPlanes( BoxPlanes( Vec3d( 8, 8, 8 ), Vec3d( 24, 24, 24 ) ) );
		const BrushSolid c =
		    BuildSolidFromPlanes( BoxPlanes( Vec3d( 16, 0, 0 ), Vec3d( 32, 16, 16 ) ) );
		const BrushSolid d =
		    BuildSolidFromPlanes( BoxPlanes( Vec3d( 40, 0, 0 ), Vec3d( 48, 8, 8 ) ) );
		checks.That( SolidsOverlap( a, b ), "interpenetrating boxes overlap" );
		checks.That( !SolidsOverlap( a, c ), "face-touching boxes do not overlap" );
		checks.That( !SolidsOverlap( a, d ), "separated boxes do not overlap" );
		// A diagonal wedge beside a box: only an edge axis separates them.
		std::vector<Plane> wedge = BoxPlanes( Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) );
		wedge.push_back( PlaneThrough( Vec3d( 16, 0, 0 ), Normalize( Vec3d( 1, 1, 0 ) ) ) );
		const BrushSolid w = BuildSolidFromPlanes( wedge );
		const BrushSolid e =
		    BuildSolidFromPlanes( BoxPlanes( Vec3d( 12, 12, 0 ), Vec3d( 20, 20, 16 ) ) );
		checks.That( !SolidsOverlap( w, e ), "a cut corner clears a box beside the cut" );
	}

	// Ray entry into a convex volume.
	{
		const std::vector<Plane> cube = BoxPlanes( Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) );
		const std::optional<RayEntry> e =
		    RayEnterConvex( cube, Vec3d( -10, 8, 8 ), Vec3d( 1, 0, 0 ) );
		checks.That(
		    e && std::fabs( e->t - 10 ) < 1e-12 && cube[e->plane].normal == Vec3d( -1, 0, 0 ),
		    "a ray enters through the -X face at t=10" );
		const std::optional<RayEntry> scaled =
		    RayEnterConvex( cube, Vec3d( -10, 8, 8 ), Vec3d( 2, 0, 0 ) );
		checks.That( scaled && std::fabs( scaled->t - 5 ) < 1e-12, "t is in direction units" );
		checks.That( !RayEnterConvex( cube, Vec3d( -10, 8, 8 ), Vec3d( -1, 0, 0 ) ),
		    "pointing away misses (negative)" );
		checks.That( !RayEnterConvex( cube, Vec3d( 8, 8, 8 ), Vec3d( 1, 0, 0 ) ),
		    "starting inside is no entry (negative)" );
		checks.That( !RayEnterConvex( cube, Vec3d( -10, 20, 8 ), Vec3d( 1, 0, 0 ) ),
		    "parallel outside misses (negative)" );
	}

	return checks.Report();
}
