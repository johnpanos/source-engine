//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The light a burst of sparks emits (RFC 0011 runtime light set):
//          old-style trail sparks (game/client/fx_sparks.cpp) and particle
//          systems (PCF) drawn with a spark material
//          (game/client/particles_new.cpp). Tier0-free C++11 so the
//          conformance suite (unittests/rendertest/test_spark_light.cpp) runs
//          the same policy the client instantiates.
//
//          A burst is one light, not one per spark: the renderer takes at most
//          a few unbaked lights per frame. The light sits at the emission-
//          weighted centroid of the live sparks near its source (within its
//          reach), in their emission-weighted color. Its strength is the frame's total emission over the burst's
//          peak (or over a full burst's emission, when the burst has one), so
//          it starts at full strength and fades as the sparks fade and die.
//          A trail spark's emission is its drawn ramp times the share of its
//          trail still drawn; a system particle's is its alpha times its
//          tint's brightest channel.
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

// A system particle's emission: its alpha (clamped to 0..1) times its tint's
// brightest channel, as the particle draws.
inline float SystemEmission( float flAlpha, const float tint[3] )
{
	float flBrightest = tint[0] > tint[1] ? tint[0] : tint[1];
	flBrightest = tint[2] > flBrightest ? tint[2] : flBrightest;
	const float flClamped = flAlpha < 0.0f ? 0.0f : ( flAlpha > 1.0f ? 1.0f : flAlpha );
	return flBrightest > 0.0f ? flClamped * flBrightest : 0.0f;
}

// A particle system's burst is at full strength from this much emission (eight
// fully drawn sparks); fewer, fainter sparks light less.
const float kSystemFullEmission = 8.0f;

// Whether a particle material draws sparks: its name contains "spark", in any
// case (effects/spark, particle/sparks/sparks, particle/particle_spark,
// particle/glow_spark_01, ...).
inline bool IsSparkMaterial( const char *pName )
{
	if ( !pName )
		return false;
	static const char kSpark[] = "spark";
	for ( const char *p = pName; *p; ++p )
	{
		int k = 0;
		for ( ; kSpark[k]; ++k )
		{
			char c = p[k];
			if ( c >= 'A' && c <= 'Z' )
				c = char( c - 'A' + 'a' );
			if ( c != kSpark[k] )
				break;
		}
		if ( !kSpark[k] )
			return true;
	}
	return false;
}

struct Light_t
{
	bool m_bLit;
	float m_Origin[3];
	float m_Color[3]; // emission-weighted, brightest channel 1
	float m_flScale;  // 0..1 of the burst's full light
};

// One burst's light. Each frame: BeginFrame, then Add every live spark (in one
// or more batches), then Current. Current may be read after each batch; it
// covers every spark added this frame so far.
class CBurst
{
public:
	// flFullEmission > 0: a burst's full strength needs at least this much
	// emission (a system of a few faint glints lights less than a shower);
	// 0: every burst's peak is its full strength.
	explicit CBurst( float flFullEmission = 0.0f ) : m_flPeak( flFullEmission ), m_flReach( 0.0f )
	{
		m_Anchor[0] = m_Anchor[1] = m_Anchor[2] = 0.0f;
		BeginFrame();
	}

	// Only sparks within flReach of the anchor (the burst's source) light it; 0
	// takes every spark. A spark that has flown far off, or fallen out of the
	// world, lights nothing near the burst and must not drag its light away.
	void SetReach( const float anchor[3], float flReach )
	{
		for ( int k = 0; k < 3; ++k )
			m_Anchor[k] = anchor[k];
		m_flReach = flReach;
	}

	void BeginFrame()
	{
		for ( int k = 0; k < 3; ++k )
			m_Sum[k] = m_ColorSum[k] = 0.0f;
		m_flTotal = 0.0f;
	}

	void Add( const float pos[3], float flEmission )
	{
		static const float kWhite[3] = { 1.0f, 1.0f, 1.0f };
		Add( pos, flEmission, kWhite );
	}

	void Add( const float pos[3], float flEmission, const float color[3] )
	{
		if ( !( flEmission > 0.0f ) )
			return;
		if ( m_flReach > 0.0f )
		{
			float flDistSq = 0.0f;
			for ( int k = 0; k < 3; ++k )
				flDistSq += ( pos[k] - m_Anchor[k] ) * ( pos[k] - m_Anchor[k] );
			if ( !( flDistSq <= m_flReach * m_flReach ) )
				return;
		}
		for ( int k = 0; k < 3; ++k )
		{
			m_Sum[k] += pos[k] * flEmission;
			m_ColorSum[k] += color[k] * flEmission;
		}
		m_flTotal += flEmission;
		if ( m_flTotal > m_flPeak )
			m_flPeak = m_flTotal;
	}

	Light_t Current() const
	{
		Light_t light;
		light.m_bLit = m_flTotal > 0.0f;
		float flBrightest = 0.0f;
		for ( int k = 0; k < 3; ++k )
			flBrightest = m_ColorSum[k] > flBrightest ? m_ColorSum[k] : flBrightest;
		for ( int k = 0; k < 3; ++k )
		{
			light.m_Origin[k] = light.m_bLit ? m_Sum[k] / m_flTotal : 0.0f;
			light.m_Color[k] = flBrightest > 0.0f ? m_ColorSum[k] / flBrightest : 0.0f;
		}
		light.m_flScale = light.m_bLit ? m_flTotal / m_flPeak : 0.0f;
		return light;
	}

private:
	float m_Sum[3];
	float m_ColorSum[3];
	float m_flTotal;
	float m_flPeak;
	float m_Anchor[3];
	float m_flReach;
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
