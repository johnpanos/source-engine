//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A renderer's visibility inputs ("Visibility ..." fields of
//          render_animated_sprites, render_sprite_trail and the other sprite
//          renderers) and how they scale its particles' alpha and radius.
//          SetupParticleVisibility (builtin_particle_render_ops.cpp) gathers
//          the pixel visibility, control point and camera terms and calls
//          this rule; the conformance suite particles.visibility-inputs
//          (unittests/particlestest/test_visibility_inputs.cpp) runs it
//          directly. Tier0-free C++11.
//
//=============================================================================//
#ifndef PARTICLE_VISIBILITY_INPUTS_H
#define PARTICLE_VISIBILITY_INPUTS_H

#include <math.h>

struct CParticleVisibilityInputs
{
	float m_flCameraBias;
	float m_flInputMin;
	float m_flInputMax;
	float m_flAlphaScaleMin;
	float m_flAlphaScaleMax;
	float m_flRadiusScaleMin;
	float m_flRadiusScaleMax;
	float m_flProxyRadius;
	float m_flBBoxScale;
	bool m_bUseBBox;
	int m_nCPin;

	// Portal 2 port (CS:GO): distance and view-dot inputs, and the FOV that
	// keeps the particles' on-screen size (0 for none).
	float m_flDistanceInputMin;
	float m_flDistanceInputMax;
	float m_flDotInputMin;
	float m_flDotInputMax;
	float m_flRadiusScaleFOVBase;
};

namespace ParticleVisibility
{

// mathlib's RemapValClamped( flVal, flA, flB, 0, 1 ), unchanged.
inline float RemapClamped01( float flVal, float flA, float flB )
{
	if ( flA == flB )
		return flVal >= flB ? 1.0f : 0.0f;
	float flT = ( flVal - flA ) / ( flB - flA );
	flT = flT < 0.0f ? 0.0f : flT;
	flT = flT > 1.0f ? 1.0f : flT;
	return 0.0f + ( 1.0f - 0.0f ) * flT;
}

// Whether the renderer computes visibility at all (its context's
// m_bUseVisibility): a visibility control point or a FOV base.
inline bool IsUsed( const CParticleVisibilityInputs &inputs )
{
	return ( inputs.m_nCPin >= 0 ) || ( inputs.m_flRadiusScaleFOVBase > 0 );
}

// Which terms need gathering; each is skipped when its range is empty.
inline bool UsesPixelVisibility( const CParticleVisibilityInputs &inputs )
{
	return inputs.m_nCPin >= 0 && inputs.m_flInputMin != inputs.m_flInputMax;
}

inline bool UsesDot( const CParticleVisibilityInputs &inputs )
{
	return inputs.m_nCPin >= 0 && inputs.m_flDotInputMin != inputs.m_flDotInputMax;
}

inline bool UsesDistance( const CParticleVisibilityInputs &inputs )
{
	return inputs.m_nCPin >= 0 && inputs.m_flDistanceInputMin != inputs.m_flDistanceInputMax;
}

// The visibility in [0, 1], as Portal 2's retail SetupParticleVisibility (Mac
// client.dylib build 841) and CS:GO's. Terms the Uses* tests skip are ignored.
//   flPixelVisibility: the occlusion query at the control point, of proxy radius size;
//   flDot: dot( control point forward, normalize( control point - view origin ) );
//   flDistance: | control point - camera |.
// The pixel term is the query times the proxy radius remapped through the
// input range (retail's arithmetic: the radius, not the query, is remapped).
inline float Compute( const CParticleVisibilityInputs &inputs, float flPixelVisibility, float flDot,
    float flDistance )
{
	float flVisibility = 1.0f;
	if ( UsesPixelVisibility( inputs ) )
	{
		flVisibility = flPixelVisibility;
		flVisibility *=
		    RemapClamped01( inputs.m_flProxyRadius, inputs.m_flInputMin, inputs.m_flInputMax );
	}
	if ( UsesDot( inputs ) )
		flVisibility *= RemapClamped01( flDot, inputs.m_flDotInputMin, inputs.m_flDotInputMax );
	if ( UsesDistance( inputs ) )
		flVisibility *=
		    RemapClamped01( flDistance, inputs.m_flDistanceInputMin, inputs.m_flDistanceInputMax );
	return flVisibility;
}

// Lerp( flVisibility, min, max ) of the alpha and radius scales.
inline float AlphaScale( const CParticleVisibilityInputs &inputs, float flVisibility )
{
	return inputs.m_flAlphaScaleMin +
	       ( inputs.m_flAlphaScaleMax - inputs.m_flAlphaScaleMin ) * flVisibility;
}

inline float RadiusScale( const CParticleVisibilityInputs &inputs, float flVisibility )
{
	return inputs.m_flRadiusScaleMin +
	       ( inputs.m_flRadiusScaleMax - inputs.m_flRadiusScaleMin ) * flVisibility;
}

// The radius factor that keeps the particles' on-screen width when the view's
// FOV differs from m_flRadiusScaleFOVBase (1 when the base is 0), from the
// projection matrix's [0][0] = 1 / tan( fov / 2 ).
inline bool UsesFOV( const CParticleVisibilityInputs &inputs )
{
	return inputs.m_flRadiusScaleFOVBase != 0.0f;
}

inline float FOVRadiusScale( const CParticleVisibilityInputs &inputs, float flProjectionXX )
{
	if ( !UsesFOV( inputs ) )
		return 1.0f;
	const float DEGREES_TO_RADIANS = 0.01745329f;
	float flNeutralMatrixX =
	    1.0f / tanf( 0.5f * inputs.m_flRadiusScaleFOVBase * DEGREES_TO_RADIANS );
	return flNeutralMatrixX / flProjectionXX;
}

} // namespace ParticleVisibility

#endif // PARTICLE_VISIBILITY_INPUTS_H
