//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the VMF -> convex brush geometry bridge (RFC
//			0002, HAM-GEOMETRY-001 / hammer.geometry). Pins plane-value parsing,
//			plane derivation, the convex intersection of a brush's side planes
//			(vertex/face counts, exact bounds, outward orientation, convexity,
//			CCW winding), single-solid block import, and whole-document scene
//			import (world solids, brush-entity solids, entity summary).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/brush.h"

#include <cmath>
#include <cstdio>
#include <string>

using hammer::formats::ParseKeyValues;
using hammer::formats::ParseResult;
using hammer::geometry::BrushFace;
using hammer::geometry::BrushSolid;
using hammer::geometry::BuildSceneFromDocument;
using hammer::geometry::BuildSolidFromBlock;
using hammer::geometry::BuildSolidFromPlanes;
using hammer::geometry::ParsePlanePoints;
using hammer::geometry::Plane;
using hammer::geometry::PlaneFromPoints;
using hammer::geometry::Vec3d;
using hammer::geometry::WorldScene;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_brush.cpp", line, expression );
		++g_failures;
	}
}

#define CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

bool Near( double a, double b, double eps = 1.0e-6 )
{
	return std::fabs( a - b ) <= eps;
}

double Dot( const Vec3d &a, const Vec3d &b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

double Len( const Vec3d &a )
{
	return std::sqrt( Dot( a, a ) );
}

// Six axis-aligned side planes (each written as three points) of the cube
// [0,64]^3, in the VMF "plane" text form. Winding is intentionally mixed to prove
// BuildSolid reorients each plane against the interior rather than trusting it.
const char *kCubeVmfSolid = "solid\n"
                            "{\n"
                            "\t\"id\" \"42\"\n"
                            "\tside { \"plane\" \"(64 0 0) (64 1 0) (64 0 1)\" \"material\" \"DEV/A\" }\n"
                            "\tside { \"plane\" \"(0 0 0) (0 0 1) (0 1 0)\" \"material\" \"DEV/B\" }\n"
                            "\tside { \"plane\" \"(0 64 0) (0 64 1) (1 64 0)\" \"material\" \"DEV/C\" }\n"
                            "\tside { \"plane\" \"(0 0 0) (1 0 0) (0 0 1)\" \"material\" \"DEV/D\" }\n"
                            "\tside { \"plane\" \"(0 0 64) (1 0 64) (0 1 64)\" \"material\" \"DEV/E\" }\n"
                            "\tside { \"plane\" \"(0 0 0) (0 1 0) (1 0 0)\" \"material\" \"DEV/F\" }\n"
                            "}\n";

// Asserts 's' is the cube [0,64]^3: six quads, exact bounds, unit outward normals,
// every vertex behind every face plane (convex), CCW winding about each normal.
void CheckIsUnitCube( const BrushSolid &s )
{
	CHECK( s.faces.size() == 6 );
	CHECK( s.bounded );
	CHECK( Near( s.mins.x, 0.0 ) && Near( s.mins.y, 0.0 ) && Near( s.mins.z, 0.0 ) );
	CHECK( Near( s.maxs.x, 64.0 ) && Near( s.maxs.y, 64.0 ) && Near( s.maxs.z, 64.0 ) );

	for ( const BrushFace &f : s.faces )
	{
		CHECK( f.vertices.size() == 4 );      // each cube face is a quad
		CHECK( Near( Len( f.plane.normal ), 1.0 ) ); // unit normal

		// Every vertex lies on its own plane.
		for ( const Vec3d &v : f.vertices )
		{
			CHECK( Near( Dot( f.plane.normal, v ), f.plane.dist, 1.0e-3 ) );
		}

		// Convexity + outward orientation: every vertex of the whole solid is on
		// or behind this face plane (dot(n,x) <= dist).
		for ( const BrushFace &g : s.faces )
		{
			for ( const Vec3d &v : g.vertices )
			{
				CHECK( Dot( f.plane.normal, v ) - f.plane.dist <= 1.0e-3 );
			}
		}

		// CCW winding about the outward normal: the polygon's signed area along
		// the normal is positive.
		Vec3d area;
		for ( std::size_t i = 0; i < f.vertices.size(); ++i )
		{
			const Vec3d &a = f.vertices[i];
			const Vec3d &b = f.vertices[( i + 1 ) % f.vertices.size()];
			area.x += a.y * b.z - a.z * b.y;
			area.y += a.z * b.x - a.x * b.z;
			area.z += a.x * b.y - a.y * b.x;
		}
		CHECK( Dot( area, f.plane.normal ) > 0.0 );
	}
}

void TestPlaneParsing()
{
	const auto ok = ParsePlanePoints( "(0 0 0) (1 0 0) (0 1 0)" );
	CHECK( ok.has_value() );
	if ( ok )
	{
		CHECK( Near( ( *ok )[1].x, 1.0 ) && Near( ( *ok )[2].y, 1.0 ) );
	}
	// Extra whitespace / negative and fractional components.
	const auto ok2 = ParsePlanePoints( "  ( -1.5 0 0 )   (2 0 0)\t(0 3.25 0) " );
	CHECK( ok2.has_value() );
	if ( ok2 )
	{
		CHECK( Near( ( *ok2 )[0].x, -1.5 ) && Near( ( *ok2 )[2].y, 3.25 ) );
	}

	CHECK( !ParsePlanePoints( "(0 0 0) (1 0 0)" ).has_value() );             // too few points
	CHECK( !ParsePlanePoints( "(0 0 0) (1 0 0) (0 1 0) (0 0 1)" ).has_value() ); // too many
	CHECK( !ParsePlanePoints( "(0 0 0) (x 0 0) (0 1 0)" ).has_value() );     // non-numeric
	CHECK( !ParsePlanePoints( "(0 0) (1 0 0) (0 1 0)" ).has_value() );       // short point
	CHECK( !ParsePlanePoints( "(0 0 0 (1 0 0) (0 1 0)" ).has_value() );      // unbalanced
}

void TestPlaneFromPoints()
{
	const auto p = PlaneFromPoints( Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 0, 1, 0 ) );
	CHECK( p.has_value() );
	if ( p )
	{
		CHECK( Near( Len( p->normal ), 1.0 ) );
		CHECK( Near( std::fabs( p->normal.z ), 1.0 ) ); // XY plane -> normal along Z
		CHECK( Near( p->dist, 0.0 ) );
	}
	// Collinear points have no plane.
	CHECK( !PlaneFromPoints( Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ), Vec3d( 2, 0, 0 ) ).has_value() );
}

void TestBuildFromPlanes()
{
	// Cube via explicit (already-outward) planes.
	std::vector<Plane> planes = {
	    { { 1, 0, 0 }, 64 }, { { -1, 0, 0 }, 0 }, { { 0, 1, 0 }, 64 },
	    { { 0, -1, 0 }, 0 }, { { 0, 0, 1 }, 64 }, { { 0, 0, -1 }, 0 },
	};
	CheckIsUnitCube( BuildSolidFromPlanes( planes, {}, 7 ) );

	// Inward-facing planes must produce the SAME cube (reorientation policy).
	std::vector<Plane> inward = {
	    { { -1, 0, 0 }, -64 }, { { 1, 0, 0 }, 0 }, { { 0, -1, 0 }, -64 },
	    { { 0, 1, 0 }, 0 },    { { 0, 0, -1 }, -64 }, { { 0, 0, 1 }, 0 },
	};
	CheckIsUnitCube( BuildSolidFromPlanes( inward, {}, 8 ) );

	// Fewer than four planes cannot bound a finite solid.
	CHECK( BuildSolidFromPlanes( { planes[0], planes[1], planes[2] } ).faces.empty() );

	// Off-origin box: the interior point must be derived from the polytope, not
	// assumed near the origin (regression guard for a brush far from 0,0,0).
	std::vector<Plane> offset = {
	    { { 1, 0, 0 }, 164 }, { { -1, 0, 0 }, -100 }, { { 0, 1, 0 }, 264 },
	    { { 0, -1, 0 }, -200 }, { { 0, 0, 1 }, 364 }, { { 0, 0, -1 }, -300 },
	};
	const BrushSolid box = BuildSolidFromPlanes( offset, {}, 9 );
	CHECK( box.faces.size() == 6 );
	CHECK( box.bounded );
	CHECK( Near( box.mins.x, 100.0 ) && Near( box.mins.y, 200.0 ) && Near( box.mins.z, 300.0 ) );
	CHECK( Near( box.maxs.x, 164.0 ) && Near( box.maxs.y, 264.0 ) && Near( box.maxs.z, 364.0 ) );
	for ( const BrushFace &f : box.faces )
	{
		for ( const Vec3d &v : f.vertices )
		{
			CHECK( v.x >= 99.0 && v.x <= 165.0 && v.z >= 299.0 && v.z <= 365.0 );
		}
	}
}

void TestBuildFromBlock()
{
	ParseResult pr = ParseKeyValues( kCubeVmfSolid );
	CHECK( pr.ok );
	CHECK( pr.root.children.size() == 1 );
	if ( !pr.ok || pr.root.children.empty() )
	{
		return;
	}
	const BrushSolid s = BuildSolidFromBlock( pr.root.children[0] );
	CHECK( s.id == 42 );
	CheckIsUnitCube( s );

	// Every face carries a material, and materials are non-empty for this input.
	int withMaterial = 0;
	for ( const BrushFace &f : s.faces )
	{
		if ( !f.material.empty() )
		{
			++withMaterial;
		}
	}
	CHECK( withMaterial == 6 );
}

void TestBuildScene()
{
	std::string vmf;
	vmf += "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n";
	vmf += kCubeVmfSolid; // one world solid
	vmf += "}\n";
	// A brush entity contributing a second solid.
	vmf += "entity\n{\n\t\"classname\" \"func_detail\"\n";
	vmf += kCubeVmfSolid;
	vmf += "}\n";
	// A point entity with an origin and no solids.
	vmf += "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"origin\" \"16 32 48\"\n}\n";

	ParseResult pr = ParseKeyValues( vmf );
	CHECK( pr.ok );
	if ( !pr.ok )
	{
		return;
	}

	const WorldScene scene = BuildSceneFromDocument( pr.root );
	CHECK( scene.solids.size() == 2 );   // world solid + brush-entity solid
	CHECK( scene.TotalFaces() == 12 );   // two cubes
	CHECK( scene.bounded );
	CHECK( Near( scene.mins.x, 0.0 ) && Near( scene.maxs.z, 64.0 ) );

	CHECK( scene.entities.size() == 2 );
	if ( scene.entities.size() == 2 )
	{
		CHECK( scene.entities[0].classname == "func_detail" );
		CHECK( scene.entities[0].solidCount == 1 );
		CHECK( !scene.entities[0].origin.has_value() );

		CHECK( scene.entities[1].classname == "info_player_start" );
		CHECK( scene.entities[1].solidCount == 0 );
		CHECK( scene.entities[1].origin.has_value() );
		if ( scene.entities[1].origin )
		{
			CHECK( Near( scene.entities[1].origin->x, 16.0 )
			       && Near( scene.entities[1].origin->y, 32.0 )
			       && Near( scene.entities[1].origin->z, 48.0 ) );
		}
	}
}

} // namespace

int main()
{
	TestPlaneParsing();
	TestPlaneFromPoints();
	TestBuildFromPlanes();
	TestBuildFromBlock();
	TestBuildScene();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.geometry.brush: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.geometry.brush: all checks passed\n" );
	return 0;
}
