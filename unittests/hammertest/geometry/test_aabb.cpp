//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless conformance test for hammer::geometry::AxisAlignedBox
//			(RFC 0002, HAM-GEOMETRY-001). Runs on the Linux headless core profile
//			with no MFC, tier0, GPU, or display. This is the first executable
//			oracle for the geometry seam; it encodes the characterized BoundBox
//			behavior so a substitution that changes it fails.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/aabb.h"

#include <cmath>
#include <cstdio>

using hammer::geometry::AxisAlignedBox;
using hammer::geometry::kAxisZ;
using hammer::geometry::kUninitialized;
using hammer::geometry::Vec3;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_aabb.cpp", line, expression );
		++g_failures;
	}
}

bool NearlyEqual( float a, float b )
{
	return std::fabs( a - b ) <= 1e-4f;
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

//-----------------------------------------------------------------------------
// A freshly reset box is deliberately invalid until the first UpdateBounds, and
// seeds correctly from a single point.
//-----------------------------------------------------------------------------
void TestResetAndSeed()
{
	AxisAlignedBox box;
	CHECK( !box.IsValidBox() );
	CHECK( box.mins[0] == kUninitialized );
	CHECK( box.maxs[0] == -kUninitialized );

	box.UpdateBounds( Vec3( 1.0f, 2.0f, 3.0f ) );
	CHECK( box.IsValidBox() );

	Vec3 mins;
	Vec3 maxs;
	box.GetBounds( mins, maxs );
	CHECK( mins[0] == 1.0f && mins[1] == 2.0f && mins[2] == 3.0f );
	CHECK( maxs[0] == 1.0f && maxs[1] == 2.0f && maxs[2] == 3.0f );

	box.UpdateBounds( Vec3( -4.0f, 5.0f, -6.0f ) );
	box.GetBounds( mins, maxs );
	CHECK( mins[0] == -4.0f && mins[1] == 2.0f && mins[2] == -6.0f );
	CHECK( maxs[0] == 1.0f && maxs[1] == 5.0f && maxs[2] == 3.0f );
}

//-----------------------------------------------------------------------------
// Center, size, and containment.
//-----------------------------------------------------------------------------
void TestCenterSizeContains()
{
	AxisAlignedBox box( Vec3( -2.0f, -2.0f, -2.0f ), Vec3( 4.0f, 6.0f, 8.0f ) );

	Vec3 center;
	box.GetBoundsCenter( center );
	CHECK( NearlyEqual( center[0], 1.0f ) );
	CHECK( NearlyEqual( center[1], 2.0f ) );
	CHECK( NearlyEqual( center[2], 3.0f ) );

	Vec3 size;
	box.GetBoundsSize( size );
	CHECK( NearlyEqual( size[0], 6.0f ) );
	CHECK( NearlyEqual( size[1], 8.0f ) );
	CHECK( NearlyEqual( size[2], 10.0f ) );

	CHECK( box.ContainsPoint( Vec3( 0.0f, 0.0f, 0.0f ) ) );
	CHECK( box.ContainsPoint( Vec3( -2.0f, -2.0f, -2.0f ) ) ); // inclusive faces
	CHECK( !box.ContainsPoint( Vec3( 5.0f, 0.0f, 0.0f ) ) );
}

//-----------------------------------------------------------------------------
// Intersection uses open faces (touching boxes do not intersect); containment
// uses closed faces.
//-----------------------------------------------------------------------------
void TestIntersectAndInside()
{
	AxisAlignedBox box( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 10.0f, 10.0f, 10.0f ) );

	CHECK( box.IsIntersectingBox( Vec3( 5.0f, 5.0f, 5.0f ), Vec3( 15.0f, 15.0f, 15.0f ) ) );
	// Faces that only touch at x==10 must NOT count as intersecting.
	CHECK( !box.IsIntersectingBox( Vec3( 10.0f, 0.0f, 0.0f ), Vec3( 20.0f, 10.0f, 10.0f ) ) );
	CHECK( !box.IsIntersectingBox( Vec3( 20.0f, 20.0f, 20.0f ), Vec3( 30.0f, 30.0f, 30.0f ) ) );

	CHECK( box.IsInsideBox( Vec3( -1.0f, -1.0f, -1.0f ), Vec3( 11.0f, 11.0f, 11.0f ) ) );
	CHECK( box.IsInsideBox( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 10.0f, 10.0f, 10.0f ) ) ); // inclusive
	CHECK( !box.IsInsideBox( Vec3( 1.0f, 1.0f, 1.0f ), Vec3( 9.0f, 9.0f, 9.0f ) ) );
}

//-----------------------------------------------------------------------------
// SnapToGrid preserves the box size and snaps the minimum corner. A non-positive
// grid is a rejected degenerate input (declared refinement over legacy divide).
//-----------------------------------------------------------------------------
void TestSnapToGrid()
{
	AxisAlignedBox box( Vec3( 3.0f, 11.0f, -7.0f ), Vec3( 19.0f, 27.0f, 9.0f ) );
	Vec3 sizeBefore;
	box.GetBoundsSize( sizeBefore );

	box.SnapToGrid( 16 );

	Vec3 mins;
	Vec3 maxs;
	box.GetBounds( mins, maxs );
	// rint(3/16)=0, rint(11/16)=1, rint(-7/16)=0 -> 0,16,0
	CHECK( NearlyEqual( mins[0], 0.0f ) );
	CHECK( NearlyEqual( mins[1], 16.0f ) );
	CHECK( NearlyEqual( mins[2], 0.0f ) );

	Vec3 sizeAfter;
	box.GetBoundsSize( sizeAfter );
	CHECK( NearlyEqual( sizeBefore[0], sizeAfter[0] ) );
	CHECK( NearlyEqual( sizeBefore[1], sizeAfter[1] ) );
	CHECK( NearlyEqual( sizeBefore[2], sizeAfter[2] ) );

	// Exact half boundary: snapping must round halves AWAY from zero, matching
	// legacy V_rint. 8/16 == 0.5 -> 16 (not 0), -8/16 == -0.5 -> -16. std::rint
	// would round to even and give 0 here, so this pins the fidelity.
	AxisAlignedBox halfBox( Vec3( 8.0f, -8.0f, 0.0f ), Vec3( 12.0f, -4.0f, 4.0f ) );
	halfBox.SnapToGrid( 16 );
	CHECK( NearlyEqual( halfBox.mins[0], 16.0f ) );
	CHECK( NearlyEqual( halfBox.mins[1], -16.0f ) );

	// Degenerate grid leaves the box unchanged.
	AxisAlignedBox unchanged( Vec3( 3.0f, 11.0f, -7.0f ), Vec3( 19.0f, 27.0f, 9.0f ) );
	AxisAlignedBox copy = unchanged;
	unchanged.SnapToGrid( 0 );
	CHECK( unchanged.mins[0] == copy.mins[0] && unchanged.maxs[2] == copy.maxs[2] );
}

//-----------------------------------------------------------------------------
// Rotate90 about Z swaps the X/Y extents around the box center and preserves the
// footprint area for a symmetric box.
//-----------------------------------------------------------------------------
void TestRotate90()
{
	AxisAlignedBox box( Vec3( 0.0f, 0.0f, 0.0f ), Vec3( 8.0f, 2.0f, 4.0f ) );
	box.Rotate90( kAxisZ );

	Vec3 size;
	box.GetBoundsSize( size );
	// A 8x2 footprint becomes 2x8 after a 90-degree turn; Z is untouched.
	CHECK( NearlyEqual( size[0], 2.0f ) );
	CHECK( NearlyEqual( size[1], 8.0f ) );
	CHECK( NearlyEqual( size[2], 4.0f ) );

	Vec3 center;
	box.GetBoundsCenter( center );
	CHECK( NearlyEqual( center[0], 4.0f ) );
	CHECK( NearlyEqual( center[1], 1.0f ) );
}

int main()
{
	TestResetAndSeed();
	TestCenterSizeContains();
	TestIntersectAndInside();
	TestSnapToGrid();
	TestRotate90();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.geometry AABB: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.geometry AABB: all checks passed\n" );
	return 0;
}
