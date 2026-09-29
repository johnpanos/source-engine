//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: particles.visibility-inputs.v1 - how a sprite renderer's visibility
//          inputs scale its particles' alpha and radius
//          (public/particles/particle_visibility_inputs.h, the rule
//          SetupParticleVisibility applies). Hand-computed cases from retail
//          Portal 2 renderers for the distance, view-dot, pixel-visibility and
//          FOV inputs; bitwise equality with the previous rule for renderers
//          that set none of the new inputs. The sensitivity build restores the
//          previous rule, which the retail cases must reject.
//
//===========================================================================//

#include "particles/particle_visibility_inputs.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;
#ifdef PARTICLE_VISIBILITY_SEEDED_LEGACY
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
unsigned long g_rejected = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		if ( kSeeded )
		{
			++g_rejected;
			std::fprintf( stderr, "seeded defect detected: %s\n", description );
			return;
		}
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

bool SameBits( float a, float b )
{
	return std::memcmp( &a, &b, sizeof( a ) ) == 0;
}

// The renderer unpack defaults (BEGIN_PARTICLE_RENDER_OPERATOR_UNPACK).
CParticleVisibilityInputs Defaults()
{
	CParticleVisibilityInputs inputs;
	std::memset( &inputs, 0, sizeof( inputs ) );
	inputs.m_nCPin = -1;
	inputs.m_flProxyRadius = 1.0f;
	inputs.m_flInputMin = 0.0f;
	inputs.m_flInputMax = 1.0f;
	inputs.m_flDotInputMin = 0.0f;
	inputs.m_flDotInputMax = 0.0f;
	inputs.m_flDistanceInputMin = 0.0f;
	inputs.m_flDistanceInputMax = 0.0f;
	inputs.m_flAlphaScaleMin = 0.0f;
	inputs.m_flAlphaScaleMax = 1.0f;
	inputs.m_flRadiusScaleMin = 1.0f;
	inputs.m_flRadiusScaleMax = 1.0f;
	inputs.m_flRadiusScaleFOVBase = 0.0f;
	inputs.m_flCameraBias = 0.0f;
	return inputs;
}

// The rule before this change, verbatim from builtin_particle_render_ops.cpp at
// 71d986fa (mathlib's RemapValClamped and clamp spelled out): the pixel
// visibility remapped through the input range onto the alpha and radius
// ranges; no distance, dot or FOV input.
float LegacyRemapValClamped( float val, float A, float B, float C, float D )
{
	if ( A == B )
		return val >= B ? D : C;
	float cVal = ( val - A ) / ( B - A );
	const float t = cVal < 0.0f ? 0.0f : cVal;
	cVal = t > 1.0f ? 1.0f : t;
	return C + ( D - C ) * cVal;
}

struct Scales
{
	float m_flAlpha;
	float m_flRadius;
};

// The renderer's scales for a pixel visibility query result, a view dot and a
// camera distance at the visibility control point, and the view's projection
// [0][0], as SetupParticleVisibility combines them.
Scales Evaluate( const CParticleVisibilityInputs &inputs, float flPixelVisibility, float flDot,
    float flDistance, float flProjectionXX )
{
	Scales scales;
#ifdef PARTICLE_VISIBILITY_SEEDED_LEGACY
	(void)flDot;
	(void)flDistance;
	(void)flProjectionXX;
	scales.m_flAlpha = LegacyRemapValClamped( flPixelVisibility, inputs.m_flInputMin,
	    inputs.m_flInputMax, inputs.m_flAlphaScaleMin, inputs.m_flAlphaScaleMax );
	scales.m_flRadius = LegacyRemapValClamped( flPixelVisibility, inputs.m_flInputMin,
	    inputs.m_flInputMax, inputs.m_flRadiusScaleMin, inputs.m_flRadiusScaleMax );
#else
	float flVisibility =
	    ParticleVisibility::Compute( inputs, flPixelVisibility, flDot, flDistance );
	scales.m_flAlpha = ParticleVisibility::AlphaScale( inputs, flVisibility );
	scales.m_flRadius = ParticleVisibility::RadiusScale( inputs, flVisibility );
	if ( ParticleVisibility::UsesFOV( inputs ) )
		scales.m_flRadius *= ParticleVisibility::FOVRadiusScale( inputs, flProjectionXX );
#endif
	return scales;
}

void CheckDistance()
{
	// paint_spray_fx.pcf paint_blob_splat2: control point 0, input 1..1 (no pixel
	// term), distance 512..768, alpha 1..0: fades out with distance.
	CParticleVisibilityInputs inputs = Defaults();
	inputs.m_nCPin = 0;
	inputs.m_flInputMin = 1.0f;
	inputs.m_flInputMax = 1.0f;
	inputs.m_flDistanceInputMin = 512.0f;
	inputs.m_flDistanceInputMax = 768.0f;
	inputs.m_flAlphaScaleMin = 1.0f;
	inputs.m_flAlphaScaleMax = 0.0f;
	Check( Evaluate( inputs, 1.0f, 0.0f, 640.0f, 1.0f ).m_flAlpha == 0.5f,
	    "distance input: half way through 512..768 the splat is at half alpha" );
	Check( Evaluate( inputs, 1.0f, 0.0f, 100.0f, 1.0f ).m_flAlpha == 1.0f &&
	           Evaluate( inputs, 1.0f, 0.0f, 900.0f, 1.0f ).m_flAlpha == 0.0f,
	    "distance input: opaque nearer than 512, gone beyond 768" );

	// rain_fx_unused.pcf rain_sheet: an inverted range (1200..800).
	inputs = Defaults();
	inputs.m_nCPin = 0;
	inputs.m_flDistanceInputMin = 1200.0f;
	inputs.m_flDistanceInputMax = 800.0f;
	Check( Evaluate( inputs, 1.0f, 0.0f, 1000.0f, 1.0f ).m_flAlpha == 0.5f,
	    "distance input: an inverted range remaps the other way" );
}

void CheckDot()
{
	// br_train_effects.pcf br_window_lightbeam shape: dot -1..1, alpha 0.25..1.
	CParticleVisibilityInputs inputs = Defaults();
	inputs.m_nCPin = 0;
	inputs.m_flDotInputMin = -1.0f;
	inputs.m_flDotInputMax = 1.0f;
	inputs.m_flAlphaScaleMin = 0.25f;
	Check( Evaluate( inputs, 1.0f, 0.5f, 0.0f, 1.0f ).m_flAlpha == 0.8125f,
	    "dot input: a beam seen at dot 0.5 of -1..1 has alpha 0.25 + 0.75 * 0.75" );
	Check( Evaluate( inputs, 1.0f, -1.0f, 0.0f, 1.0f ).m_flAlpha == 0.25f,
	    "dot input: a beam seen from behind is at its minimum alpha" );
}

void CheckPixelVisibility()
{
	// spark_fx.pcf env_sparks_d: proxy radius 2, input 0..0.25, alpha 0..4, radius 1..2.
	// Retail remaps the proxy radius (2, past the range: 1), not the query.
	CParticleVisibilityInputs inputs = Defaults();
	inputs.m_nCPin = 0;
	inputs.m_flProxyRadius = 2.0f;
	inputs.m_flInputMax = 0.25f;
	inputs.m_flAlphaScaleMax = 4.0f;
	inputs.m_flRadiusScaleMax = 2.0f;
	Scales scales = Evaluate( inputs, 0.125f, 0.0f, 0.0f, 1.0f );
	Check( scales.m_flAlpha == 0.5f && scales.m_flRadius == 1.125f,
	    "pixel input: a spark 1/8 visible has alpha 0.5 and radius 1.125, as retail" );

	// impact_fx.pcf impact_metal_child_glow: proxy radius 0.5 halves the visibility.
	inputs = Defaults();
	inputs.m_nCPin = 0;
	inputs.m_flProxyRadius = 0.5f;
	inputs.m_flAlphaScaleMax = 10.0f;
	Check( Evaluate( inputs, 1.0f, 0.0f, 0.0f, 1.0f ).m_flAlpha == 5.0f,
	    "pixel input: a proxy radius of 0.5 in 0..1 halves the visibility (alpha 5)" );
}

void CheckFOV()
{
	CParticleVisibilityInputs inputs = Defaults();
	Check( !ParticleVisibility::IsUsed( inputs ),
	    "no control point and no FOV base: visibility is off" );
	inputs.m_flRadiusScaleFOVBase = 90.0f;
	Check( ParticleVisibility::IsUsed( inputs ),
	    "a FOV base turns visibility on without a control point" );
	// At a 90 degree base, a projection with [0][0] = 2 (a narrower view) halves the radius.
	float flRadius = Evaluate( inputs, 1.0f, 0.0f, 0.0f, 2.0f ).m_flRadius;
	Check( std::fabs( flRadius - 0.5f ) < 1e-6f,
	    "FOV base 90: a zoomed-in view (projection 2) halves the radius" );
	Check( Evaluate( inputs, 1.0f, 0.0f, 0.0f, 1.0f ).m_flAlpha == 1.0f,
	    "FOV base: without a control point the alpha stays at its maximum" );
}

// Renderers that set none of the new inputs (every Portal 1 and HL2 PCF): the
// previous rule's exact bits over the authored ranges retail uses.
void CheckAbsentFieldsAreNoOps()
{
	static const float kProxy[] = { 1.0f, 1.5f, 2.0f, 2.25f, 3.0f, 4.0f, 8.0f };
	static const float kScale[] = {
	    0.0f, 0.1f, 0.25f, 0.35f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 4.0f, 10.0f };
	unsigned long compared = 0;
	unsigned long mismatches = 0;
	for ( float proxy : kProxy )
		for ( float amin : kScale )
			for ( float amax : kScale )
				for ( float rmin : kScale )
					for ( float rmax : kScale )
					{
						CParticleVisibilityInputs inputs = Defaults();
						inputs.m_nCPin = 0;
						inputs.m_flProxyRadius = proxy;
						inputs.m_flAlphaScaleMin = amin;
						inputs.m_flAlphaScaleMax = amax;
						inputs.m_flRadiusScaleMin = rmin;
						inputs.m_flRadiusScaleMax = rmax;
						for ( int step = 0; step <= 64; ++step )
						{
							float flPixel = float( step ) / 64.0f;
							if ( step == 7 )
								flPixel = 0.1f; // and a value that is not a binary fraction
							Scales now = Evaluate( inputs, flPixel, 0.0f, 0.0f, 1.0f );
							float flAlpha =
							    LegacyRemapValClamped( flPixel, 0.0f, 1.0f, amin, amax );
							float flRadius =
							    LegacyRemapValClamped( flPixel, 0.0f, 1.0f, rmin, rmax );
							++compared;
							if ( !SameBits( now.m_flAlpha, flAlpha ) ||
							     !SameBits( now.m_flRadius, flRadius ) )
								++mismatches;
						}
					}
	std::printf( "absent inputs: %lu cases, %lu mismatches\n", compared, mismatches );
	Check( mismatches == 0,
	    "renderers without the new inputs keep the previous alpha and radius bits" );

	CParticleVisibilityInputs inputs = Defaults();
	inputs.m_nCPin = 0;
	Check( !ParticleVisibility::UsesDot( inputs ) && !ParticleVisibility::UsesDistance( inputs ) &&
	           !ParticleVisibility::UsesFOV( inputs ) &&
	           ParticleVisibility::UsesPixelVisibility( inputs ),
	    "defaults gather only the pixel visibility query, as before" );
}

} // namespace

int main()
{
	CheckDistance();
	CheckDot();
	CheckPixelVisibility();
	CheckFOV();
	CheckAbsentFieldsAreNoOps();

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
