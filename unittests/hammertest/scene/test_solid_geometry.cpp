//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.scene solid geometry conformance (RFC 0002, R08 domain
//			logic): box solids with outward planes and world-aligned texture
//			axes, face polygons mapped back to their sides, side points from
//			polygons and planes, side normalization (redundant sides dropped,
//			texture data kept), and box arithmetic. Negative checks: open side
//			sets are refused and leave the solid untouched.
//
//=============================================================================//

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

using namespace hammer::scene;
using mapgeometry::Vec3d;

int main()
{
	testing::Checks checks;

	FaceTexture tex;
	tex.material = "BRICK/BRICKWALL001";
	tex.u.scale = 0.5;
	const Solid box = MakeBoxSolid( { Vec3d( -32, 0, 0 ), Vec3d( 32, 64, 128 ) }, tex );
	checks.Equal( box.sides.size(), std::size_t( 6 ), "a box has six sides" );

	// Every side's plane points out through its face.
	const Vec3d center( 0, 32, 64 );
	bool outward = true;
	for ( const Side &s : box.sides )
	{
		const mapgeometry::Plane p = s.Plane();
		outward = outward && mapgeometry::PlaneDistance( p, center ) < 0 &&
		          std::fabs( mapgeometry::Length( p.normal ) - 1 ) < 1e-12;
	}
	checks.That( outward, "box side planes face outward" );

	// World-aligned axes: the floor gets (1 0 0)/(0 -1 0); walls are not smeared.
	const Side &top = box.sides[0];
	checks.That( top.texture.u.axis == Vec3d( 1, 0, 0 ) && top.texture.v.axis == Vec3d( 0, -1, 0 ),
	    "top face uses floor axes" );
	checks.That( box.sides[2].texture.u.axis == Vec3d( 0, 1, 0 ) &&
	                 box.sides[2].texture.v.axis == Vec3d( 0, 0, -1 ),
	    "west wall uses wall axes" );
	checks.That( top.texture.material == tex.material && top.texture.u.scale == 0.5,
	    "material and scale carried" );

	// Geometry: faces map back to sides; bounds exact.
	const mapgeometry::BrushSolid g = BuildGeometry( box );
	checks.Equal( g.faces.size(), std::size_t( 6 ), "six faces" );
	bool mapped = true;
	for ( const mapgeometry::BrushFace &f : g.faces )
		mapped = mapped && f.material == box.sides[f.sourcePlane].texture.material;
	checks.That( mapped, "faces carry their side's material" );
	const std::optional<Box> b = SolidBounds( box );
	checks.That(
	    b && b->mins == Vec3d( -32, 0, 0 ) && b->maxs == Vec3d( 32, 64, 128 ), "exact bounds" );

	// Points from a plane reproduce the plane.
	{
		mapgeometry::Plane p;
		p.normal = mapgeometry::Normalize( Vec3d( 1, 2, 3 ) );
		p.dist = 17;
		Side s;
		s.points = PointsFromPlane( p );
		const mapgeometry::Plane q = s.Plane();
		checks.That( mapgeometry::NearlyEqual( q.normal, p.normal, 1e-12 ) &&
		                 std::fabs( q.dist - 17 ) < 1e-9,
		    "PointsFromPlane round-trips" );
		mapgeometry::Plane down;
		down.normal = Vec3d( 0, 0, -1 );
		down.dist = 4;
		s.points = PointsFromPlane( down );
		checks.That( s.Plane().normal == Vec3d( 0, 0, -1 ), "vertical normals keep their sign" );
	}

	// Normalization: a redundant side is dropped, the others keep their data.
	{
		Solid extra = box;
		Side redundant = box.sides[0];
		redundant.vmfId = 99;
		mapgeometry::Plane far;
		far.normal = Vec3d( 0, 0, 1 );
		far.dist = 500;
		redundant.points = PointsFromPlane( far );
		extra.sides.push_back( redundant );
		extra.sides[3].texture.u.shift = 7;
		const std::optional<Solid> n = NormalizeSides( extra );
		checks.That( n && n->sides.size() == 6, "redundant side dropped" );
		checks.That( n && !n->FindSide( 99 ), "the dropped side is the redundant one" );
		bool shifted = false;
		if ( n )
			for ( const Side &s : n->sides )
				shifted = shifted || s.texture.u.shift == 7;
		checks.That( shifted, "texture data survives normalization" );

		// Negative: an open solid is refused.
		Solid open = box;
		open.sides.pop_back();
		checks.That( !NormalizeSides( open ), "open side set refused" );
	}

	// Ray entry maps back to the entered side.
	{
		const std::optional<SolidRayHit> hit =
		    RayEnterSolid( box, Vec3d( 0, 32, 500 ), Vec3d( 0, 0, -1 ) );
		checks.That( hit && hit->t == 372 && hit->point == Vec3d( 0, 32, 128 ) &&
		                 hit->normal == Vec3d( 0, 0, 1 ),
		    "a downward ray enters the top" );
		checks.That( hit && box.sides[hit->side].Plane().normal == Vec3d( 0, 0, 1 ),
		    "the hit names the top side" );
		checks.That( !RayEnterSolid( box, Vec3d( 0, 32, 500 ), Vec3d( 0, 0, 1 ) ),
		    "a ray pointing away misses (negative)" );
	}

	// Box arithmetic.
	{
		Box a{ Vec3d( 0, 0, 0 ), Vec3d( 10, 10, 10 ) };
		const Box c{ Vec3d( 10, 0, 0 ), Vec3d( 20, 10, 10 ) };
		checks.That( a.Intersects( c ) && !a.Intersects( c, -0.5 ),
		    "touching boxes intersect only with slack" );
		checks.That( a.Encloses( { Vec3d( 1, 1, 1 ), Vec3d( 9, 9, 9 ) } ), "enclosure" );
		checks.That( !a.Encloses( c ), "partial overlap is not enclosure" );
		a.Extend( c );
		checks.That(
		    a.maxs == Vec3d( 20, 10, 10 ) && a.Center() == Vec3d( 10, 5, 5 ), "extend and center" );
		checks.That( PointBox( Vec3d( 1, 2, 3 ) ).Size() == Vec3d(), "point box has no size" );
	}

	return checks.Report();
}
