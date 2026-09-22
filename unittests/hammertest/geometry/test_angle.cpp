//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for the angle numeric policies (RFC 0002). Pins the
//			exact legacy fixang/lineangle behavior, including fixang's quirks (a
//			single wrap step, so 720 -> 360, not 0) so extraction cannot silently
//			change tool rotation behavior.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/angle.h"

#include <cmath>
#include <cstdio>

using hammer::geometry::LineAngleDegrees;
using hammer::geometry::NormalizeAngleDegrees;

namespace
{

int g_failures = 0;

bool Near( float a, float b )
{
	return std::fabs( a - b ) <= 1e-3f;
}

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_angle.cpp", line, expression );
		++g_failures;
	}
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

void TestNormalize()
{
	CHECK( Near( NormalizeAngleDegrees( 0.0f ), 0.0f ) );
	CHECK( Near( NormalizeAngleDegrees( 180.0f ), 180.0f ) );
	CHECK( Near( NormalizeAngleDegrees( -90.0f ), 270.0f ) );
	CHECK( Near( NormalizeAngleDegrees( 359.0f ), 359.0f ) );
	CHECK( Near( NormalizeAngleDegrees( 400.0f ), 40.0f ) );
	// Legacy quirk: a single wrap step only. 720 -> 360 (NOT 0). Pinning this
	// prevents a "helpful" full-modulo change from slipping in unreviewed.
	CHECK( Near( NormalizeAngleDegrees( 720.0f ), 360.0f ) );
}

void TestLineAngle()
{
	CHECK( Near( LineAngleDegrees( 0.0f, 0.0f, 1.0f, 0.0f ), 0.0f ) );    // +x
	CHECK( Near( LineAngleDegrees( 0.0f, 0.0f, 0.0f, 1.0f ), 90.0f ) );   // +y
	CHECK( Near( LineAngleDegrees( 0.0f, 0.0f, -1.0f, 0.0f ), 180.0f ) ); // -x
	CHECK( Near( LineAngleDegrees( 0.0f, 0.0f, 0.0f, -1.0f ), -90.0f ) ); // -y
	CHECK( Near( LineAngleDegrees( 1.0f, 1.0f, 2.0f, 2.0f ), 45.0f ) );
	// Zero-length segment returns 0.
	CHECK( Near( LineAngleDegrees( 5.0f, 5.0f, 5.0f, 5.0f ), 0.0f ) );
}

int main()
{
	TestNormalize();
	TestLineAngle();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.geometry angle: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.geometry angle: all checks passed\n" );
	return 0;
}
