//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.sample-count.v1 — MSAA mode support and clamping, checked
//          exhaustively against the contract's definition.
//
//===========================================================================//

#include "render/render_sample_count.h"
#include "testing/conformance_result.h"

#include <cstdint>
#include <cstdio>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

[[maybe_unused]] void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

bool IsPowerOfTwo( int value )
{
	return value > 0 && ( value & ( value - 1 ) ) == 0;
}

// Definition: 1, or the largest supported power of two in [2, requested].
bool ClampIsCorrect( int requested, uint32_t mask, int result )
{
	int best = 1;
	for ( int samples = 2; samples <= 64; samples *= 2 )
	{
		if ( samples <= requested && ( mask & static_cast<uint32_t>( samples ) ) )
			best = samples;
	}
	return result == best && IsPowerOfTwo( result );
}

// The clamp a suite must reject: rounds a request up to the next supported count.
[[maybe_unused]] int ClampRoundingUp( int requested, uint32_t mask )
{
	for ( int samples = 2; samples <= 64; samples *= 2 )
	{
		if ( samples >= requested && ( mask & static_cast<uint32_t>( samples ) ) )
			return samples;
	}
	return 1;
}

template <typename Clamp> bool ClampCorrectEverywhere( Clamp clamp )
{
	for ( uint32_t mask = 0; mask < 128; ++mask )
	{
		for ( int requested = -1; requested <= 70; ++requested )
		{
			if ( !ClampIsCorrect( requested, mask, clamp( requested, mask ) ) )
				return false;
		}
	}
	return true;
}
} // namespace

int main()
{
#ifdef SAMPLE_COUNT_SEEDED_ROUND_UP
	return testing::ReportConformance( 1, ClampCorrectEverywhere( ClampRoundingUp ) ? 1 : 0 );
#else
	Check( ClampCorrectEverywhere(
	           []( int requested, uint32_t mask )
	           {
		           return render::ClampSampleCount( requested, mask );
	           } ),
	    "clamp matches the definition for every mask and request" );

	bool supportCorrect = true;
	for ( uint32_t mask = 0; mask < 128; ++mask )
	{
		for ( int samples = -1; samples <= 70; ++samples )
		{
			const bool expected = samples >= 2 && IsPowerOfTwo( samples ) &&
			                      ( mask & static_cast<uint32_t>( samples ) ) != 0;
			supportCorrect =
			    supportCorrect && render::IsMsaaModeSupported( samples, mask ) == expected;
		}
	}
	Check( supportCorrect, "mode support matches the definition for every mask and count" );

	const uint32_t typical = 1 | 2 | 4 | 8;
	Check(
	    !render::IsMsaaModeSupported( 6, typical ) && render::ClampSampleCount( 6, typical ) == 4,
	    "6x is never supported and clamps down to 4x" );
	Check(
	    render::ClampSampleCount( 16, typical ) == 8, "a request above the maximum clamps to it" );
	Check( render::ClampSampleCount( 4, 1 ) == 1 && render::ClampSampleCount( 0, typical ) == 1,
	    "single-sample-only devices and AA off resolve to one sample" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
