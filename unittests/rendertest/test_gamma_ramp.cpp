//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.gamma-ramp.v1 — the monitor gamma ramp against an analytic
//          double-precision oracle of the documented curve.
//
//===========================================================================//

#include "render/render_gamma_ramp.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cmath>
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

double Clamp01( double value )
{
	return std::min( 1.0, std::max( 0.0, value ) );
}

// The curve as the contract states it, in double precision.
double ExpectedEntry( const render::GammaRampParams &params, int index )
{
	double value = Clamp01( std::pow( index / 255.0, double( params.gamma ) / 2.2 ) );
	if ( params.tvEnabled )
	{
		value = Clamp01( std::pow( value, 2.2 / double( params.tvExponent ) ) );
		value = Clamp01(
		    value * ( params.tvRangeMax - params.tvRangeMin ) / 255.0 + params.tvRangeMin / 255.0 );
	}
	return value * 65535.0;
}

// Every entry within 2 of the analytic value (float evaluation and truncation).
bool MatchesOracle( const render::GammaRampParams &params, const render::GammaRamp16 &ramp )
{
	for ( int i = 0; i < 256; ++i )
	{
		if ( std::fabs( ramp[i] - ExpectedEntry( params, i ) ) > 2.0 )
			return false;
	}
	return true;
}

[[maybe_unused]] bool Monotonic( const render::GammaRamp16 &ramp )
{
	for ( int i = 1; i < 256; ++i )
	{
		if ( ramp[i] < ramp[i - 1] )
			return false;
	}
	return true;
}

render::GammaRampParams Gamma( float gamma )
{
	render::GammaRampParams params;
	params.gamma = gamma;
	return params;
}
} // namespace

int main()
{
	render::GammaRamp16 ramp = {};
#ifdef GAMMA_SEEDED_INVERTED_EXPONENT
	// Seeded defect: the reciprocal exponent (2.2 / gamma instead of gamma / 2.2).
	const render::GammaRampParams requested = Gamma( 1.8f );
	render::BuildGammaRamp16( Gamma( 2.2f * 2.2f / 1.8f ), ramp );
	return testing::ReportConformance( 1, MatchesOracle( requested, ramp ) ? 1 : 0 );
#else
	for ( float gamma : { 1.6f, 1.8f, 2.2f, 2.4f, 2.6f } )
	{
		const render::GammaRampParams params = Gamma( gamma );
		render::BuildGammaRamp16( params, ramp );
		Check( MatchesOracle( params, ramp ), "ramp matches the analytic curve" );
		Check( Monotonic( ramp ), "ramp is non-decreasing" );
		Check( ramp[0] == 0 && ramp[255] == 65535, "black stays black and white stays white" );
	}

	render::BuildGammaRamp16( Gamma( 2.2f ), ramp );
	Check( render::IsIdentityAt8Bit( ramp ), "the default gamma 2.2 is an identity at 8 bits" );
	bool exactTimes257 = true;
	for ( int i = 0; i < 256; ++i )
		exactTimes257 = exactTimes257 && ramp[i] == i * 257;
	std::printf( "GAMMA_2_2_EXACT_I_TIMES_257 %d\n", exactTimes257 ? 1 : 0 );

	render::BuildGammaRamp16( Gamma( 1.6f ), ramp );
	Check( !render::IsIdentityAt8Bit( ramp ) && ramp[128] > 128 * 257,
	    "a lower gamma brightens midtones and is not an identity" );
	render::BuildGammaRamp16( Gamma( 2.6f ), ramp );
	Check( !render::IsIdentityAt8Bit( ramp ) && ramp[128] < 128 * 257,
	    "a higher gamma darkens midtones and is not an identity" );

	render::GammaRampParams tv = Gamma( 2.2f );
	tv.tvEnabled = true;
	render::BuildGammaRamp16( tv, ramp );
	Check( MatchesOracle( tv, ramp ), "TV ramp matches the analytic curve" );
	Check( ramp[0] >= 4110 && ramp[0] <= 4112 && ramp[255] == 65535,
	    "TV ramp maps black to the 16/255 floor and keeps white at 255" );
	Check( !render::IsIdentityAt8Bit( ramp ), "the TV ramp is never an identity" );

	Check( render::GammaRampEntryTo8Bit( 0 ) == 0 && render::GammaRampEntryTo8Bit( 65535 ) == 255 &&
	           render::GammaRampEntryTo8Bit( 128 * 257 ) == 128 &&
	           render::GammaRampEntryTo8Bit( 128 * 257 - 128 ) == 128 &&
	           render::GammaRampEntryTo8Bit( 128 * 257 - 129 ) == 127,
	    "8-bit conversion rounds to nearest" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
