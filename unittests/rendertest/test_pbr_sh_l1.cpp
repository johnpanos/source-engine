//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent affine and directional-light oracles for the RFC 0007
//          four-sample SH L1 irradiance fit.
//
//===========================================================================//

#include "render/pbr_sh_l1.h"
#include "testing/conformance_result.h"

#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

bool Near( float actual, float expected, float tolerance = 0.00001f )
{
	return std::isfinite( actual ) && std::fabs( actual - expected ) <= tolerance;
}

std::array<render::pbr::IrradianceDirection, 4> SourceSampleDirections()
{
	// The flat normal and three Source RNM directions are copied as immutable
	// test data from public/mathlib/bumpvects.h. The fitter takes caller data;
	// the strict headless profile must not include that legacy platform header.
	std::array<render::pbr::IrradianceDirection, 4> directions = { {
	    { 0.0f, 0.0f, 1.0f },
	    { 0.81649661064147949f, 0.0f, 0.57735025882720947f },
	    { -0.40824821591377258f, 0.70710676908493042f, 0.57735025882720947f },
	    { -0.40824821591377258f, -0.70710676908493042f, 0.57735025882720947f },
	} };
	return directions;
}
} // namespace

int main()
{
	using namespace render::pbr;
	auto directions = SourceSampleDirections();
	std::array<float, 4> samples = {};
	for ( int i = 0; i < 4; ++i )
	{
		const IrradianceDirection &n = directions[i];
		samples[i] = 0.2f + 0.5f * n.x - 0.25f * n.y + 0.3f * n.z;
	}
#ifdef PBR_SEEDED_SWAPPED_BASIS
	std::swap( samples[1], samples[2] );
#endif
	ShL1Irradiance fit = {};
	Check( FitShL1( directions, samples, fit ), "Source flat and RNM directions are independent" );
	const bool coefficientRecovered = Near( fit.constant, 0.2f ) && Near( fit.x, 0.5f ) &&
	                                  Near( fit.y, -0.25f ) && Near( fit.z, 0.3f );
	Check(
	    coefficientRecovered, "four known affine samples recover their independent coefficients" );
	const IrradianceDirection probe{ 0.6f, 0.0f, 0.8f };
	Check( Near( EvaluateShL1( fit, probe ), 0.74f ),
	    "SH L1 evaluates a direction that was not used in the fit" );
	for ( int i = 0; i < 4; ++i )
		Check( Near( EvaluateShL1( fit, directions[i] ), samples[i] ),
		    "fit reproduces a source lighting sample" );

	const std::array<float, 4> uniform = { 0.7f, 0.7f, 0.7f, 0.7f };
	ShL1Irradiance uniformFit = {};
	Check( FitShL1( directions, uniform, uniformFit ) && Near( uniformFit.constant, 0.7f ) &&
	           Near( uniformFit.x, 0.0f ) && Near( uniformFit.y, 0.0f ) &&
	           Near( uniformFit.z, 0.0f ),
	    "uniform irradiance has no spurious directional term" );

	ShL1Irradiance untouched{ 7.0f, 8.0f, 9.0f, 10.0f };
	auto degenerate = directions;
	degenerate[3] = degenerate[2];
	Check( !FitShL1( degenerate, samples, untouched ) && Near( untouched.constant, 7.0f ) &&
	           Near( untouched.x, 8.0f ),
	    "coplanar sample directions fail without publishing a result" );
	auto nonfinite = samples;
	nonfinite[0] = std::numeric_limits<float>::infinity();
	Check( !FitShL1( directions, nonfinite, untouched ) && Near( untouched.z, 10.0f ),
	    "nonfinite irradiance fails without publishing a result" );

	// A grazing point light is not band-limited to L1. The four-sample fit
	// greatly overestimates a perpendicular horizon normal, even though it
	// reproduces every baked sample. This is feasibility evidence, not a
	// quality acceptance threshold; the baker needs further sampling/policy.
	std::array<float, 4> grazing = {};
	for ( int i = 0; i < 4; ++i )
		grazing[i] = std::fmax( 0.0f, directions[i].x );
	ShL1Irradiance grazingFit = {};
	const bool fittedGrazing = FitShL1( directions, grazing, grazingFit );
	const float perpendicularError =
	    fittedGrazing ? EvaluateShL1( grazingFit, { 0.0f, -1.0f, 0.0f } ) : 0.0f;
	Check( fittedGrazing && perpendicularError > 0.5f,
	    "grazing-light fixture exposes the four-sample L1 approximation error" );
	std::printf( "GRAZING_L1_PERPENDICULAR_ERROR %.6f\n", perpendicularError );

#ifdef PBR_SEEDED_SWAPPED_BASIS
	return testing::ReportConformance( 1, coefficientRecovered ? 1 : 0 );
#else
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
