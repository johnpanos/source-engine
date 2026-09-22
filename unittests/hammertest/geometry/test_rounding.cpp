//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::geometry::RoundHalfAwayFromZero
//			(RFC 0002). Pins the legacy V_rint rounding rule, especially the exact
//			.5 boundaries where round-half-away-from-zero differs from the
//			round-half-to-even that std::rint would give. A substitution that used
//			std::rint would fail here.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/rounding.h"

#include <cstdio>

using hammer::geometry::RoundHalfAwayFromZero;

namespace
{

int g_failures = 0;

void ExpectEqual( float actual, float expected, const char *label, int line )
{
	if ( actual != expected )
	{
		std::printf( "FAIL %s:%d: %s -> %.6f, expected %.6f\n", "test_rounding.cpp", line, label,
		    actual, expected );
		++g_failures;
	}
}

} // namespace

#define EXPECT_EQ( actual, expected ) ExpectEqual( ( actual ), ( expected ), #actual, __LINE__ )

int main()
{
	// Exact halves round away from zero (this is the whole point of the policy).
	EXPECT_EQ( RoundHalfAwayFromZero( 0.5f ), 1.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( 1.5f ), 2.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( 2.5f ), 3.0f ); // std::rint would give 2.0f
	EXPECT_EQ( RoundHalfAwayFromZero( -0.5f ), -1.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( -2.5f ), -3.0f );

	// Ordinary rounding and exact integers.
	EXPECT_EQ( RoundHalfAwayFromZero( 0.0f ), 0.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( 0.4f ), 0.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( 0.6f ), 1.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( -0.4f ), 0.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( -0.6f ), -1.0f );
	EXPECT_EQ( RoundHalfAwayFromZero( 7.0f ), 7.0f );

	if ( g_failures != 0 )
	{
		std::printf( "hammer.geometry rounding: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.geometry rounding: all checks passed\n" );
	return 0;
}
