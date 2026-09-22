//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the angle normalization policy (RFC 0002/0005).
//			A "helpful" full-modulo normalizer diverges from the legacy single-step
//			fixang for out-of-range input (720 -> 0 instead of 360). The oracle
//			must detect that divergence; otherwise it does not actually pin the
//			legacy behavior.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/geometry/angle.h"

#include <cmath>
#include <cstdio>

using hammer::geometry::NormalizeAngleDegrees;

namespace
{

// Broken provider: full-range modulo normalization into [0, 360).
float FullModuloNormalize( float degrees )
{
	float wrapped = std::fmod( degrees, 360.0f );
	if ( wrapped < 0.0f )
	{
		wrapped += 360.0f;
	}
	return wrapped;
}

// The legacy-quirk clause: NormalizeAngleDegrees( 720 ) must equal 360 (single
// wrap step), NOT 0. Returns true when the provider matches legacy behavior.
template <typename Fn> bool ConformsLegacyQuirk( Fn normalize )
{
	return std::fabs( normalize( 720.0f ) - 360.0f ) <= 1e-3f;
}

} // namespace

int main()
{
	const bool realConforms = ConformsLegacyQuirk( NormalizeAngleDegrees );
	const bool brokenConforms = ConformsLegacyQuirk( FullModuloNormalize );

	int failures = 0;

	if ( !realConforms )
	{
		std::printf(
		    "FAIL: real NormalizeAngleDegrees did not preserve the legacy single-wrap quirk\n" );
		++failures;
	}
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the full-modulo divergence\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.geometry angle negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf( "hammer.geometry angle negative: oracle detects violations (real passes, modulo "
	             "caught)\n" );
	return 0;
}
