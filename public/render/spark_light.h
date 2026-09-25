//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The light a burst of trail sparks emits (RFC 0011 runtime light
//          set). Tier0-free C++11 so the conformance suite
//          (unittests/rendertest/test_spark_light.cpp) runs the same policy
//          the client instantiates (game/client/fx_sparks.cpp).
//
//          A burst is one light, not one per spark: the renderer takes at most
//          a few unbaked lights per frame. The light sits at the emission-
//          weighted centroid of the live sparks. Its strength is the frame's
//          total emission over the burst's peak, so it starts at 1 and fades
//          as the sparks' trails shrink and die. A spark's emission is its
//          drawn brightness ramp times the share of its trail still drawn.
//
//          The client carries the light as a dlight, which the engine
//          publishes in the frame's light set. A budget caps how many bursts
//          hold a light at once.
//
//=============================================================================//
#ifndef SPARK_LIGHT_H
#define SPARK_LIGHT_H

namespace SparkLight
{

// A trail spark's brightness ramp, as CTrailParticles draws it: it fades in
// over its first 0.3 seconds, or fades out over its life, when asked to. (The
// 0.3 is a double, as the original comparison was.)
inline float TrailRamp( float flLifetime, float flDieTime, bool bFadeIn, bool bFade )
{
	if ( flLifetime <= 0.3 && bFadeIn )
		return flLifetime;
	if ( bFade )
		return 1.0f - ( flLifetime / flDieTime );
	return 1.0f;
}

// A trail spark's emission: its ramp times the share of its trail still drawn
// (the trail's length shrinks with its remaining life).
inline float TrailEmission( float flLifetime, float flDieTime, bool bFadeIn, bool bFade )
{
	if ( !( flDieTime > 0.0f ) || flLifetime >= flDieTime )
		return 0.0f;
	const float flRemaining = 1.0f - flLifetime / flDieTime;
	const float flEmission = TrailRamp( flLifetime, flDieTime, bFadeIn, bFade ) * flRemaining;
	return flEmission > 0.0f ? flEmission : 0.0f;
}

struct Light_t
{
	bool m_bLit;
	float m_Origin[3];
	float m_flScale; // 0..1 of the burst's full light
};

// One burst's light. Each frame: BeginFrame, then Add every live spark (in one
// or more batches), then Current. Current may be read after each batch; it
// covers every spark added this frame so far.
class CBurst
{
public:
	CBurst()
	{
		BeginFrame();
		m_flPeak = 0.0f;
	}

	void BeginFrame()
	{
		m_Sum[0] = m_Sum[1] = m_Sum[2] = 0.0f;
		m_flTotal = 0.0f;
	}

	void Add( const float pos[3], float flEmission )
	{
		if ( !( flEmission > 0.0f ) )
			return;
		for ( int k = 0; k < 3; ++k )
			m_Sum[k] += pos[k] * flEmission;
		m_flTotal += flEmission;
		if ( m_flTotal > m_flPeak )
			m_flPeak = m_flTotal;
	}

	Light_t Current() const
	{
		Light_t light;
		light.m_bLit = m_flTotal > 0.0f;
		for ( int k = 0; k < 3; ++k )
			light.m_Origin[k] = light.m_bLit ? m_Sum[k] / m_flTotal : 0.0f;
		light.m_flScale = light.m_bLit ? m_flTotal / m_flPeak : 0.0f;
		return light;
	}

private:
	float m_Sum[3];
	float m_flTotal;
	float m_flPeak;
};

// How many bursts hold a light at once. The owner acquires before it first
// lights a burst and releases once when the burst goes dark or is destroyed.
class CBudget
{
public:
	CBudget() : m_nHeld( 0 ) {}

	// nMax <= 0 lights no burst.
	bool Acquire( int nMax )
	{
		if ( m_nHeld >= nMax )
			return false;
		++m_nHeld;
		return true;
	}

	void Release()
	{
		if ( m_nHeld > 0 )
			--m_nHeld;
	}

	int Held() const { return m_nHeld; }

private:
	int m_nHeld;
};

} // namespace SparkLight

#endif // SPARK_LIGHT_H
