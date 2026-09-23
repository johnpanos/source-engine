//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent analytic and numerical oracles for the RFC 0007
//          metal/roughness specular BRDF. The integral tests evaluate the
//          public BRDF over a hemisphere rather than repeating its equations.
//
//===========================================================================//

#include "render/pbr_brdf.h"

#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace
{
int g_checks = 0;
int g_failures = 0;
std::vector<std::string> g_failureDescriptions;

using DistributionFunction = float ( * )( float, float );
using SpecularFunction = render::pbr::Color ( * )(
    render::pbr::Color, float, float, float, float, float );

struct Model
{
	DistributionFunction distribution;
	SpecularFunction specular;
};

#ifdef PBR_SEEDED_BAD_GGX
float UnnormalizedGgx( float normalDotHalf, float roughness )
{
	return 0.5f * render::pbr::GgxDistribution( normalDotHalf, roughness );
}
#endif

#ifdef PBR_SEEDED_NO_FRESNEL
render::pbr::Color NoFresnelSpecular( render::pbr::Color, float normalDotView, float normalDotLight,
    float normalDotHalf, float, float roughness )
{
	if ( normalDotView <= 0.0f || normalDotLight <= 0.0f )
	{
		return { 0.0f, 0.0f, 0.0f };
	}
	const float value = render::pbr::GgxDistribution( normalDotHalf, roughness ) *
	                    render::pbr::SmithVisibility( normalDotView, normalDotLight, roughness );
	return { value, value, value };
}
#endif

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		g_failureDescriptions.emplace_back( description );
#if !defined( PBR_SEEDED_BAD_GGX ) && !defined( PBR_SEEDED_NO_FRESNEL ) &&                         \
    !defined( PBR_SEEDED_BAD_SPLIT_SUM )
		std::fprintf( stderr, "FAIL: %s\n", description );
#endif
	}
}

bool Near( double actual, double expected, double tolerance )
{
	return std::isfinite( actual ) && std::abs( actual - expected ) <= tolerance;
}

// Integrate D(H) * (N.H) over the hemisphere. The normal distribution must
// integrate to one independent of the roughness parameter.
double DistributionIntegral( float roughness, DistributionFunction distribution )
{
	constexpr int kSteps = 100000;
	double integral = 0.0;
	for ( int step = 0; step < kSteps; ++step )
	{
		const double normalDotHalf = ( step + 0.5 ) / kSteps;
		integral += distribution( static_cast<float>( normalDotHalf ), roughness ) * normalDotHalf;
	}
	return integral * 2.0 * render::pbr::kPi / kSteps;
}

// A white environment supplies unit radiance in every direction. The
// reflected radiance is the BRDF's cosine-weighted hemisphere integral.
double WhiteFurnace( float roughness, float normalDotView, SpecularFunction specular,
    float reflectanceAtNormal = 1.0f )
{
	constexpr int kPolarSteps = 160;
	constexpr int kAzimuthSteps = 512;
	const double viewX = std::sqrt( 1.0 - normalDotView * normalDotView );
	const double viewZ = normalDotView;
	double integral = 0.0;
	for ( int polar = 0; polar < kPolarSteps; ++polar )
	{
		const double lightZ = ( polar + 0.5 ) / kPolarSteps;
		const double lightRadius = std::sqrt( 1.0 - lightZ * lightZ );
		for ( int azimuth = 0; azimuth < kAzimuthSteps; ++azimuth )
		{
			const double angle = 2.0 * render::pbr::kPi * ( azimuth + 0.5 ) / kAzimuthSteps;
			const double lightX = lightRadius * std::cos( angle );
			const double lightY = lightRadius * std::sin( angle );
			const double halfX = viewX + lightX;
			const double halfY = lightY;
			const double halfZ = viewZ + lightZ;
			const double inverseHalfLength =
			    1.0 / std::sqrt( halfX * halfX + halfY * halfY + halfZ * halfZ );
			const float normalDotHalf = static_cast<float>( halfZ * inverseHalfLength );
			const float viewDotHalf =
			    static_cast<float>( ( viewX * halfX + viewZ * halfZ ) * inverseHalfLength );
			const render::pbr::Color reflected = specular(
			    { reflectanceAtNormal, reflectanceAtNormal, reflectanceAtNormal }, normalDotView,
			    static_cast<float>( lightZ ), normalDotHalf, viewDotHalf, roughness );
			integral += reflected.red * lightZ;
		}
	}
	return integral * 2.0 * render::pbr::kPi / ( kPolarSteps * kAzimuthSteps );
}
} // namespace

int main()
{
	using namespace render::pbr;
	Model model{ GgxDistribution, EvaluateSpecular };
#ifdef PBR_SEEDED_BAD_GGX
	model.distribution = UnnormalizedGgx;
#endif
#ifdef PBR_SEEDED_NO_FRESNEL
	model.specular = NoFresnelSpecular;
#endif

	Check( Near( model.distribution( 1.0f, 1.0f ), 1.0 / kPi, 1e-6 ),
	    "GGX at roughness one is uniform over half-vector solid angle" );
	Check( Near( model.distribution( 1.0f, 0.5f ), 16.0 / kPi, 1e-5 ),
	    "GGX uses alpha = roughness squared" );
	Check( Near( SmithVisibility( 1.0f, 1.0f, 1.0f ), 0.25, 1e-7 ),
	    "Smith normal-incidence visibility" );
	Check( Near( SmithVisibility( 0.5f, 0.5f, 1.0f ), 0.5, 1e-7 ),
	    "Smith correlated visibility at roughness one" );
	Check( Near( FresnelSchlick( 0.04f, 1.0f ), 0.04, 1e-6 ), "Schlick normal-incidence boundary" );
	Check( Near( FresnelSchlick( 0.04f, 0.0f ), 1.0, 1e-6 ), "Schlick grazing-incidence boundary" );
	Check( Near( FresnelSchlick( 0.04f, 0.5f ), 0.07, 1e-6 ),
	    "Schlick midpoint detects omitted Fresnel" );

	for ( float roughness : { 0.25f, 0.5f, 1.0f } )
	{
		Check( Near( DistributionIntegral( roughness, model.distribution ), 1.0, 0.003 ),
		    "GGX distribution integrates to one" );
		for ( float normalDotView : { 0.3f, 0.65f, 1.0f } )
		{
			const double energy = WhiteFurnace( roughness, normalDotView, model.specular );
			Check( std::isfinite( energy ) && energy >= 0.0 && energy <= 1.01,
			    "single-scatter BRDF does not create energy in a white furnace" );
		}
	}
	Check( Near( WhiteFurnace( 1.0f, 1.0f, model.specular ), 1.0 - std::log( 2.0 ), 0.004 ),
	    "roughness-one normal-incidence furnace matches analytic 1 - ln(2)" );
	for ( float roughness : { 0.5f, 1.0f } )
	{
		for ( float normalDotView : { 0.3f, 0.65f, 1.0f } )
		{
			SplitSumCoefficients table = SampleSplitSum( normalDotView, roughness );
#ifdef PBR_SEEDED_BAD_SPLIT_SUM
			std::swap( table.a, table.b );
#endif
			const double grazing = WhiteFurnace( roughness, normalDotView, model.specular, 0.0f );
			const double white = WhiteFurnace( roughness, normalDotView, model.specular );
			Check( Near( table.a, white - grazing, 0.03 ) && Near( table.b, grazing, 0.03 ),
			    "split-sum table matches an independent hemisphere BRDF integral" );
			const double dielectricSpecular =
			    WhiteFurnace( roughness, normalDotView, model.specular, 0.04f );
			const double layeredWhite = dielectricSpecular + 1.0 - ( 0.04 * table.a + table.b );
			Check( Near( layeredWhite, 1.0, 0.03 ),
			    "layered white dielectric closes energy in a white furnace" );
		}
	}
	const Color whiteDielectric =
	    EvaluateLayeredDirect( { 1.0f, 1.0f, 1.0f }, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f );
	const Color whiteMetal =
	    EvaluateLayeredDirect( { 1.0f, 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f );
	Check( whiteDielectric.red > whiteMetal.red && whiteMetal.red > 0.0f,
	    "layered dielectric includes diffuse and pure metal does not" );

	const Color dielectric =
	    model.specular( { 0.04f, 0.04f, 0.04f }, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f );
	Check( Near( dielectric.red, 0.04 / ( 4.0 * kPi ), 1e-7 ),
	    "dielectric normal-incidence BRDF anchor" );
	const Color metallic = model.specular( { 0.8f, 0.4f, 0.1f }, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f );
	Check( metallic.red > metallic.green && metallic.green > metallic.blue,
	    "metallic F0 preserves the color channel order" );
	// For V=(sqrt(3)/2, 0, 1/2) and L=(0, 3/5, 4/5), H is
	// (sqrt(3)/2, 3/5, 13/10)/sqrt(14/5). Swapping V and L must
	// preserve the BRDF, including its correlated visibility term.
	const float normalDotHalf = static_cast<float>( 1.3 / std::sqrt( 2.8 ) );
	const float viewDotHalf = static_cast<float>( 1.4 / std::sqrt( 2.8 ) );
	const Color forward =
	    model.specular( { 0.04f, 0.2f, 0.8f }, 0.5f, 0.8f, normalDotHalf, viewDotHalf, 0.45f );
	const Color reverse =
	    model.specular( { 0.04f, 0.2f, 0.8f }, 0.8f, 0.5f, normalDotHalf, viewDotHalf, 0.45f );
	Check( Near( forward.red, reverse.red, 1e-7 ) && Near( forward.green, reverse.green, 1e-7 ) &&
	           Near( forward.blue, reverse.blue, 1e-7 ),
	    "BRDF is reciprocal when view and light are exchanged" );
	const Color belowHorizon = model.specular( { 1.0f, 1.0f, 1.0f }, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f );
	Check( belowHorizon.red == 0.0f && belowHorizon.green == 0.0f && belowHorizon.blue == 0.0f,
	    "light below the horizon contributes nothing" );

#if defined( PBR_SEEDED_BAD_GGX ) || defined( PBR_SEEDED_NO_FRESNEL ) ||                           \
    defined( PBR_SEEDED_BAD_SPLIT_SUM )
#ifdef PBR_SEEDED_BAD_GGX
	const char *expectedFailure = "GGX distribution integrates to one";
#elif defined( PBR_SEEDED_BAD_SPLIT_SUM )
	const char *expectedFailure = "split-sum table matches an independent hemisphere BRDF integral";
#else
	const char *expectedFailure = "dielectric normal-incidence BRDF anchor";
#endif
	bool rejected = false;
	for ( const std::string &description : g_failureDescriptions )
	{
		if ( description == expectedFailure )
		{
			rejected = true;
			break;
		}
	}
	if ( !rejected )
	{
		std::fprintf( stderr, "FAIL: oracle accepted seeded defect: %s\n", expectedFailure );
	}
	std::printf( "CONFORMANCE 1 %d\n", rejected ? 0 : 1 );
	return rejected ? 0 : 1;
#else
	std::printf( "CONFORMANCE %d %d\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
#endif
}
