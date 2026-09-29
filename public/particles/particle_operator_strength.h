//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: An operator's strength in one particle system instance: its fade
//          window and the per-instance random modulation that Portal 2 and
//          CS:GO author on every operator ("operator time offset", "operator
//          time scale" and "operator strength scale" seeds with their ranges).
//          CParticleCollection::CheckIfOperatorShouldRun calls this rule, and
//          the conformance suite particles.operator-strength
//          (unittests/particlestest/test_operator_strength.cpp) runs it
//          directly. Tier0-free C++11.
//
//=============================================================================//
#ifndef PARTICLE_OPERATOR_STRENGTH_H
#define PARTICLE_OPERATOR_STRENGTH_H

#include <math.h>

//-----------------------------------------------------------------------------
// The modulation fields every operator unpacks (BEGIN_PARTICLE_OPERATOR_UNPACK
// in particles.h). The unpack defaults make each modulation term a no-op: a
// seed of 0 draws nothing, and the ranges default to an offset of 0 and scales
// of 1.
//-----------------------------------------------------------------------------
struct ParticleOperatorModulation_t
{
	float m_flOpStartFadeInTime;
	float m_flOpEndFadeInTime;
	float m_flOpStartFadeOutTime;
	float m_flOpEndFadeOutTime;
	float m_flOpFadeOscillatePeriod;

	// Per-instance random phase: the operator's clock is shifted by a draw in
	// [min, max] from the system's random stream at sample id seed.
	int m_nOpTimeOffsetSeed;
	float m_flOpTimeOffsetMin;
	float m_flOpTimeOffsetMax;

	// Per-instance random rate: past the fade-in start, the clock runs at
	// 1 / draw in [min, max].
	int m_nOpTimeScaleSeed;
	float m_flOpTimeScaleMin;
	float m_flOpTimeScaleMax;

	// Per-instance random strength multiplier in [min, max].
	int m_nOpStrengthScaleSeed;
	float m_flOpStrengthMinScale;
	float m_flOpStrengthMaxScale;

	// Portal 2 port (CS:GO): -1 runs always, 0 only while the system plays,
	// 1 only in its end cap (after StopEmission( ..., bPlayEndCap )).
	int m_nOpEndCapState;
};

namespace ParticleOperatorStrength
{

// The operator's fade window at flCurTime: 0 before the fade-in start or after
// a positive fade-out end, ramping to 1 across the fade-in and back to 0 across
// the fade-out. The legacy particles.cpp FadeInOut, unchanged.
inline float FadeInOut( float flFadeInStart, float flFadeInEnd, float flFadeOutStart,
    float flFadeOutEnd, float flCurTime )
{
	if ( flFadeInStart > flCurTime ) // started yet?
		return 0.0;

	if ( ( flFadeOutEnd > 0. ) && ( flFadeOutEnd < flCurTime ) ) // timed out?
		return 0.;

	// handle out of order cases
	flFadeInEnd = ( flFadeInEnd > flFadeInStart ) ? flFadeInEnd : flFadeInStart;
	flFadeOutStart = ( flFadeOutStart > flFadeInEnd ) ? flFadeOutStart : flFadeInEnd;
	flFadeOutEnd = ( flFadeOutEnd > flFadeOutStart ) ? flFadeOutEnd : flFadeOutStart;

	// FLerp( 0, 1, i1, i2, x ) = 0 + ( 1 - 0 ) * ( x - i1 ) / ( i2 - i1 ), as mathlib.
	float flStrength = 1.0;
	if ( ( flFadeInEnd > flCurTime ) && ( flFadeInEnd > flFadeInStart ) )
	{
		float flRamp = 0.0f + ( 1.0f - 0.0f ) * ( flCurTime - flFadeInStart ) /
		                          ( flFadeInEnd - flFadeInStart );
		flStrength = ( flStrength < flRamp ) ? flStrength : flRamp;
	}

	if ( ( flCurTime > flFadeOutStart ) && ( flFadeOutEnd > flFadeOutStart ) )
	{
		float flRamp = 0.0f + ( 1.0f - 0.0f ) * ( flCurTime - flFadeOutEnd ) /
		                          ( flFadeOutStart - flFadeOutEnd );
		flStrength = ( flStrength < flRamp ) ? flStrength : flRamp;
	}

	return flStrength;
}

// The operator's strength at the system's time flCurTime. The operator runs
// when the result is above 0. fnRandomFloat( nSampleId, flMin, flMax ) is the
// system instance's own random stream (CParticleCollection::RandomFloat( int,
// float, float ): the shared constant table at its seed plus nSampleId), so the
// result depends only on the instance and never on thread or call order.
//
// As Portal 2's retail CParticleCollection::CheckIfOperatorShouldRun (Mac
// client.dylib build 841) and CS:GO's particles.cpp: the time offset applies
// first and clamps at 0; the time scale then stretches time past the fade-in
// start by 1 / max( 0.0001, draw ); a fade oscillation replaces the time with
// the phase of the unmodulated clock; and the strength draw multiplies the
// fade strength squared (flStrength *= max( 0, flStrength * draw )), which is
// the retail arithmetic, kept on purpose.
template <typename RandomFloatFn>
inline float Compute( const ParticleOperatorModulation_t &op, bool bInEndCap, float flCurTime,
    RandomFloatFn fnRandomFloat )
{
	if ( op.m_nOpEndCapState != -1 )
	{
		if ( bInEndCap != ( op.m_nOpEndCapState == 1 ) )
			return 0.0f;
	}

	float flTime = flCurTime;
	if ( op.m_nOpTimeOffsetSeed )
	{
		// allow per-instance-of-particle-system random phase control for operator strength.
		float flOffset =
		    fnRandomFloat( op.m_nOpTimeOffsetSeed, op.m_flOpTimeOffsetMin, op.m_flOpTimeOffsetMax );
		flTime += flOffset;
		flTime = ( 0.0 > flTime ) ? 0.0 : flTime;
	}
	if ( op.m_nOpTimeScaleSeed && ( flTime > op.m_flOpStartFadeInTime ) )
	{
		float flDraw =
		    fnRandomFloat( op.m_nOpTimeScaleSeed, op.m_flOpTimeScaleMin, op.m_flOpTimeScaleMax );
		float flTimeScalar = 1.0 / ( ( .0001 > flDraw ) ? .0001 : flDraw );
		flTime = op.m_flOpStartFadeInTime + flTimeScalar * ( flTime - op.m_flOpStartFadeInTime );
	}
	if ( op.m_flOpFadeOscillatePeriod > 0.0 )
	{
		flTime = fmod( flCurTime * ( 1.0 / op.m_flOpFadeOscillatePeriod ), 1.0 );
	}

	float flStrength = FadeInOut( op.m_flOpStartFadeInTime, op.m_flOpEndFadeInTime,
	    op.m_flOpStartFadeOutTime, op.m_flOpEndFadeOutTime, flTime );
	if ( op.m_nOpStrengthScaleSeed )
	{
		float flStrengthMultiplier = fnRandomFloat(
		    op.m_nOpStrengthScaleSeed, op.m_flOpStrengthMinScale, op.m_flOpStrengthMaxScale );
		float flScaled = flStrength * flStrengthMultiplier;
		flStrength *= ( 0. > flScaled ) ? 0. : flScaled;
	}
	return flStrength;
}

} // namespace ParticleOperatorStrength

#endif // PARTICLE_OPERATOR_STRENGTH_H
