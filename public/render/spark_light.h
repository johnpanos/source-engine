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
//          weighted centroid of every live spark of the burst, in their
//          emission-weighted color, and its radius grows with how far the
//          sparks have spread, so its reach covers them wherever they have
//          flown or fallen. Its strength is the frame's total emission over
//          the burst's peak (or over a full burst's emission, when the burst
//          has one), so it starts at full strength and fades as the sparks
//          fade and die. A trail spark's emission is its drawn ramp times the
//          share of its trail still drawn; a system particle's is its alpha
//          times its tint's brightest channel.
//
//          The client carries the light as a dlight, which the engine
//          publishes in the frame's light set. Each frame the client lights
//          the bursts most important at the viewer, at most a budget of them
//          (SelectLit), and keeps a lit burst lit against a slightly stronger
//          newcomer, so two close bursts do not trade the light every frame.
//
//=============================================================================//
#ifndef SPARK_LIGHT_H
#define SPARK_LIGHT_H

#include <cmath>

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
	float m_flSpread; // emission-weighted RMS distance of the sparks from m_Origin
};

// How far a burst's light reaches: its base radius plus 1.5 times its sparks'
// spread (beyond the spread lie a few sparks, not most), at most 2.5 times the
// base radius. A burst gathered at its source lights its base radius.
const float kSpreadReach = 1.5f;
const float kMaxRadiusScale = 2.5f;

inline float LightRadius( float flBaseRadius, float flSpread )
{
	const float flRadius = flBaseRadius + kSpreadReach * ( flSpread > 0.0f ? flSpread : 0.0f );
	const float flMax = kMaxRadiusScale * flBaseRadius;
	return flRadius < flMax ? flRadius : flMax;
}

// One burst's light. Each frame: BeginFrame, then Add every live spark (in one
// or more batches), then Current. Current may be read after each batch; it
// covers every spark added this frame so far.
class CBurst
{
public:
	// flFullEmission > 0: a burst's full strength needs at least this much
	// emission (a system of a few faint glints lights less than a shower);
	// 0: every burst's peak is its full strength.
	explicit CBurst( float flFullEmission = 0.0f ) : m_flPeak( flFullEmission ) { BeginFrame(); }

	void BeginFrame()
	{
		for ( int k = 0; k < 3; ++k )
			m_Ref[k] = m_Sum[k] = m_ColorSum[k] = 0.0f;
		m_flSquareSum = 0.0f;
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
		// Positions are summed relative to the frame's first spark, so the
		// spread keeps its precision far from the world origin.
		if ( m_flTotal == 0.0f )
		{
			for ( int k = 0; k < 3; ++k )
				m_Ref[k] = pos[k];
		}
		for ( int k = 0; k < 3; ++k )
		{
			const float d = pos[k] - m_Ref[k];
			m_Sum[k] += d * flEmission;
			m_flSquareSum += d * d * flEmission;
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
		float flMeanSquare = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			const float flMean = light.m_bLit ? m_Sum[k] / m_flTotal : 0.0f;
			light.m_Origin[k] = light.m_bLit ? m_Ref[k] + flMean : 0.0f;
			light.m_Color[k] = flBrightest > 0.0f ? m_ColorSum[k] / flBrightest : 0.0f;
			flMeanSquare += flMean * flMean;
		}
		// E|p - c|^2 = E|p|^2 - |E p|^2, both relative to the reference.
		const float flVariance = light.m_bLit ? m_flSquareSum / m_flTotal - flMeanSquare : 0.0f;
		light.m_flSpread = flVariance > 0.0f ? std::sqrt( flVariance ) : 0.0f;
		light.m_flScale = light.m_bLit ? m_flTotal / m_flPeak : 0.0f;
		return light;
	}

private:
	float m_Ref[3];
	float m_Sum[3];
	float m_ColorSum[3];
	float m_flSquareSum;
	float m_flTotal;
	float m_flPeak;
};

// A burst that asks for light this frame.
struct Candidate_t
{
	float m_Origin[3];
	float m_flRadius;   // LightRadius
	float m_flStrength; // the light's brightest linear channel (scale included)
	bool m_bWasLit;     // it held a light last frame
};

// A lit burst keeps its light against a newcomer less than this much more
// important, so two bursts of about the same importance do not trade it.
const float kKeepLitBias = 1.25f;

// A burst's importance at the viewer: its strength times how much of the view
// its reach covers, r^2 / ( r^2 + d^2 ), d the distance from the view.
inline float Importance( const Candidate_t &candidate, const float view[3] )
{
	if ( !( candidate.m_flStrength > 0.0f ) || !( candidate.m_flRadius > 0.0f ) )
		return 0.0f;
	float flDistSq = 0.0f;
	for ( int k = 0; k < 3; ++k )
	{
		const float d = candidate.m_Origin[k] - view[k];
		flDistSq += d * d;
	}
	const float flReachSq = candidate.m_flRadius * candidate.m_flRadius;
	return candidate.m_flStrength * flReachSq / ( flReachSq + flDistSq );
}

// Which of nCandidates bursts are lit this frame: the nMaxLit most important
// at the viewer (a lit burst counted kKeepLitBias times), never one of no
// importance. Ties go to the lower index. pLit[i] receives each verdict;
// returns how many are lit. nMaxLit <= 0 lights none.
inline int SelectLit(
    const Candidate_t *pCandidates, int nCandidates, int nMaxLit, const float view[3], bool *pLit )
{
	for ( int i = 0; i < nCandidates; ++i )
		pLit[i] = false;
	int nLit = 0;
	while ( nLit < nMaxLit )
	{
		int iBest = -1;
		float flBest = 0.0f;
		for ( int i = 0; i < nCandidates; ++i )
		{
			if ( pLit[i] )
				continue;
			float flRank = Importance( pCandidates[i], view );
			if ( pCandidates[i].m_bWasLit )
				flRank *= kKeepLitBias;
			if ( flRank > flBest )
			{
				flBest = flRank;
				iBest = i;
			}
		}
		if ( iBest < 0 )
			break;
		pLit[iBest] = true;
		++nLit;
	}
	return nLit;
}

} // namespace SparkLight

#endif // SPARK_LIGHT_H
