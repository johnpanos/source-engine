//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.spark-light.v2 - the light a burst of sparks emits
//          (render/spark_light.h, RFC 0011 light set): the drawn ramp
//          against the original CTrailParticles expressions, the emission-
//          weighted centroid over every live spark, the radius that grows
//          with their spread, the fade against the burst's peak, batching
//          and the per-frame selection of the lit bursts at the viewer. The
//          sensitivity builds substitute defective policies the oracle must
//          reject.
//
//===========================================================================//

#include "render/spark_light.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
using namespace SparkLight;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( SPARK_LIGHT_SEEDED_UNWEIGHTED ) || defined( SPARK_LIGHT_SEEDED_NO_FADE ) ||           \
    defined( SPARK_LIGHT_SEEDED_SOURCE_REACH ) || defined( SPARK_LIGHT_SEEDED_FIXED_RADIUS ) ||      \
    defined( SPARK_LIGHT_SEEDED_FIRST_COME ) || defined( SPARK_LIGHT_SEEDED_NO_KEEP )
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
// A seeded build passes when the oracle rejects its defect.
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

bool Near( float a, float b, float tolerance = 1.0e-5f )
{
	return std::fabs( a - b ) <= tolerance;
}

// The policy under test: the real one, or a seeded defective one.
class BurstUnderTest
{
public:
	explicit BurstUnderTest( float fullEmission = 0.0f ) : m_burst( fullEmission ) {}
	// The burst's source and base radius. The real policy has no use for
	// them; the source-reach defect (the first policy) counts only the sparks
	// within the base radius of the source.
	void SetSource( const float source[3], float radius )
	{
		for ( int k = 0; k < 3; ++k )
			m_source[k] = source[k];
		m_reach = radius;
	}
	void BeginFrame() { m_burst.BeginFrame(); }
	void Add( const float pos[3], float emission )
	{
		static const float kWhite[3] = { 1.0f, 1.0f, 1.0f };
		Add( pos, emission, kWhite );
	}
	void Add( const float pos[3], float emission, const float color[3] )
	{
#ifdef SPARK_LIGHT_SEEDED_SOURCE_REACH
		if ( m_reach > 0.0f )
		{
			float distSq = 0.0f;
			for ( int k = 0; k < 3; ++k )
				distSq += ( pos[k] - m_source[k] ) * ( pos[k] - m_source[k] );
			if ( distSq > m_reach * m_reach )
				return;
		}
#endif
#ifdef SPARK_LIGHT_SEEDED_UNWEIGHTED
		// Every live spark counts the same, however much of it is drawn.
		m_burst.Add( pos, emission > 0.0f ? 1.0f : 0.0f, color );
#else
		m_burst.Add( pos, emission, color );
#endif
	}
	Light_t Current() const
	{
		Light_t light = m_burst.Current();
#ifdef SPARK_LIGHT_SEEDED_NO_FADE
		if ( light.m_bLit )
			light.m_flScale = 1.0f;
#endif
		return light;
	}

private:
	CBurst m_burst;
	float m_source[3] = { 0.0f, 0.0f, 0.0f };
	float m_reach = 0.0f;
};

float RadiusUnderTest( float base, float spread )
{
#ifdef SPARK_LIGHT_SEEDED_FIXED_RADIUS
	(void)spread;
	return base;
#else
	return LightRadius( base, spread );
#endif
}

int SelectUnderTest(
    std::vector<Candidate_t> candidates, int maxLit, const float view[3], std::vector<bool> *lit )
{
	const int n = int( candidates.size() );
	bool verdicts[64] = {};
#if defined( SPARK_LIGHT_SEEDED_FIRST_COME )
	// The first policy: the first bursts to ask hold the light.
	(void)view;
	int count = 0;
	for ( int i = 0; i < n && count < maxLit; ++i )
	{
		if ( candidates[i].m_flStrength > 0.0f )
		{
			verdicts[i] = true;
			++count;
		}
	}
#else
#if defined( SPARK_LIGHT_SEEDED_NO_KEEP )
	for ( Candidate_t &candidate : candidates )
		candidate.m_bWasLit = false;
#endif
	const int count = SelectLit( candidates.data(), n, maxLit, view, verdicts );
#endif
	lit->assign( verdicts, verdicts + n );
	return count;
}

Candidate_t MakeCandidate( float x, float strength, bool wasLit = false )
{
	Candidate_t candidate = { { x, 0.0f, 0.0f }, 160.0f, strength, wasLit };
	return candidate;
}

// CTrailParticles::RenderParticles' ramp before it moved to spark_light.h.
float OriginalRamp( float lifetime, float dieTime, bool fadeIn, bool fade )
{
	float ramp = 1.0;
	if ( lifetime <= 0.3 && fadeIn )
		ramp = lifetime;
	else if ( fade )
		ramp = ( 1.0f - ( lifetime / dieTime ) );
	return ramp;
}

struct Spark
{
	float pos[3];
	float velocity[3];
	float lifetime;
	float dieTime;
};

} // namespace

int main()
{
	// The drawn ramp is the original expression, bit for bit, in every mode,
	// including the lifetime that tells a double 0.3 from a float one.
	{
		bool same = true;
		const float lifetimes[] = {
		    0.0f, 0.05f, 0.1f, 0.29999998f, 0.3f, 0.30000001f, 0.31f, 0.5f, 1.0f, 1.5f, 2.0f };
		const float dieTimes[] = { 0.05f, 0.2f, 1.0f, 2.0f };
		for ( float lifetime : lifetimes )
			for ( float dieTime : dieTimes )
				for ( int mode = 0; mode < 4; ++mode )
				{
					const bool fadeIn = ( mode & 1 ) != 0, fade = ( mode & 2 ) != 0;
					same &= TrailRamp( lifetime, dieTime, fadeIn, fade ) ==
					        OriginalRamp( lifetime, dieTime, fadeIn, fade );
				}
		Check( same, "the ramp is CTrailParticles' original expression in every mode" );
	}

	// Emission: the ramp times the share of the trail still drawn.
	Check( TrailEmission( 0.0f, 1.0f, false, false ) == 1.0f, "a new spark emits fully" );
	Check( Near( TrailEmission( 0.25f, 1.0f, false, false ), 0.75f ),
	    "a spark's emission shrinks with its remaining life" );
	Check( Near( TrailEmission( 0.5f, 1.0f, false, true ), 0.25f ),
	    "a fading spark's emission is its ramp times its remaining life" );
	Check( Near( TrailEmission( 0.1f, 1.0f, true, false ), 0.09f ),
	    "a fading-in spark's emission follows its ramp" );
	Check( TrailEmission( 1.0f, 1.0f, false, false ) == 0.0f &&
	           TrailEmission( 1.5f, 1.0f, false, false ) == 0.0f,
	    "a spark at or past its death emits nothing" );
	Check(
	    TrailEmission( 0.0f, 0.0f, false, false ) == 0.0f, "a spark with no life emits nothing" );

	// A burst: the centroid is weighted by emission, and the strength is the
	// frame's emission over the burst's peak.
	{
		BurstUnderTest burst;
		burst.BeginFrame();
		Check( !burst.Current().m_bLit, "a burst with no sparks is dark" );

		const float a[3] = { 0, 0, 0 }, b[3] = { 30, 0, 0 }, c[3] = { 0, 60, 90 };
		burst.BeginFrame();
		burst.Add( a, 1.0f );
		burst.Add( b, 0.5f );
		Light_t light = burst.Current();
		Check( light.m_bLit && Near( light.m_Origin[0], 10.0f ) && Near( light.m_Origin[1], 0.0f ),
		    "the light sits at the emission-weighted centroid" );
		Check( light.m_flScale == 1.0f, "a burst starts at full strength" );
		burst.Add( c, 0.0f );
		burst.Add( c, -1.0f );
		Check( Near( burst.Current().m_Origin[2], 0.0f ),
		    "a spark that emits nothing does not move the light" );

		// The next frame: the same sparks at half emission, in two batches.
		burst.BeginFrame();
		burst.Add( a, 0.5f );
		light = burst.Current();
		Check( Near( light.m_flScale, 1.0f / 3.0f ) && Near( light.m_Origin[0], 0.0f ),
		    "a batch's light covers the sparks added so far this frame" );
		burst.Add( b, 0.25f );
		light = burst.Current();
		Check( Near( light.m_flScale, 0.5f ) && Near( light.m_Origin[0], 10.0f ),
		    "a burst at half its peak emission is at half strength" );

		// A frame brighter than any before sets a new peak.
		burst.BeginFrame();
		burst.Add( c, 3.0f );
		light = burst.Current();
		Check( light.m_flScale == 1.0f && Near( light.m_Origin[1], 60.0f ),
		    "a brighter frame is full strength and sets the peak" );
		burst.BeginFrame();
		burst.Add( c, 1.5f );
		Check( Near( burst.Current().m_flScale, 0.5f ), "later frames fade against the new peak" );

		burst.BeginFrame();
		light = burst.Current();
		Check( !light.m_bLit && light.m_flScale == 0.0f, "a burst whose sparks are gone is dark" );
	}

	// System particles: alpha times the tint's brightest channel.
	{
		const float orange[3] = { 1.0f, 0.5f, 0.2f }, dark[3] = { 0.0f, 0.0f, 0.0f };
		Check( Near( SystemEmission( 0.5f, orange ), 0.5f ) &&
		           Near( SystemEmission( 2.0f, orange ), 1.0f ) &&
		           SystemEmission( -1.0f, orange ) == 0.0f && SystemEmission( 1.0f, dark ) == 0.0f,
		    "a system particle emits its clamped alpha times its brightest tint channel" );
	}

	// Spark materials: every Portal 2 spark material, in any case and either
	// slash; nothing else.
	{
		const char *sparks[] = { "effects/spark", "particle\\sparks\\sparks.vmt",
		    "particle/particle_spark", "particle/glow_spark_01", "particle/sparks/sparks_ob",
		    "PARTICLE/SPARKS/SPARKS_NONTRAIL" };
		bool all = true;
		for ( const char *name : sparks )
			all &= IsSparkMaterial( name );
		Check( all, "every spark material is recognized" );
		Check( !IsSparkMaterial( "particle/smoke1/smoke1" ) && !IsSparkMaterial( "effects/spar" ) &&
		           !IsSparkMaterial( "" ) && !IsSparkMaterial( nullptr ),
		    "other materials are not sparks" );
	}

	// Color: the emission-weighted tint, brightest channel 1.
	{
		BurstUnderTest burst;
		const float at[3] = { 0, 0, 0 };
		const float orange[3] = { 1.0f, 0.5f, 0.0f }, blue[3] = { 0.0f, 0.0f, 1.0f };
		burst.BeginFrame();
		burst.Add( at, 3.0f, orange );
		burst.Add( at, 1.0f, blue );
		const Light_t light = burst.Current();
		Check( Near( light.m_Color[0], 1.0f ) && Near( light.m_Color[1], 0.5f ) &&
		           Near( light.m_Color[2], 1.0f / 3.0f ),
		    "the light's color is the emission-weighted tint, brightest channel 1" );
		BurstUnderTest white;
		white.BeginFrame();
		white.Add( at, 0.25f );
		const Light_t w = white.Current();
		Check( w.m_Color[0] == 1.0f && w.m_Color[1] == 1.0f && w.m_Color[2] == 1.0f,
		    "untinted sparks light white" );
	}

	// A full burst: a system of a few faint sparks lights less than a shower.
	{
		BurstUnderTest burst( kSystemFullEmission );
		const float at[3] = { 0, 0, 0 };
		burst.BeginFrame();
		burst.Add( at, 1.0f );
		burst.Add( at, 1.0f );
		Check( Near( burst.Current().m_flScale, 2.0f / kSystemFullEmission ),
		    "a burst below full emission lights in proportion" );
		burst.BeginFrame();
		for ( int i = 0; i < 16; ++i )
			burst.Add( at, 1.0f );
		Check(
		    burst.Current().m_flScale == 1.0f, "a burst above full emission is at full strength" );
		burst.BeginFrame();
		for ( int i = 0; i < 8; ++i )
			burst.Add( at, 1.0f );
		Check( Near( burst.Current().m_flScale, 0.5f ), "and then fades against its own peak" );
	}

	// Spread: the emission-weighted RMS distance of the sparks from the light,
	// kept precise far from the world origin.
	{
		BurstUnderTest burst;
		const float a[3] = { -30, 0, 0 }, b[3] = { 30, 0, 0 };
		burst.BeginFrame();
		burst.Add( a, 1.0f );
		burst.Add( b, 1.0f );
		Check( Near( burst.Current().m_flSpread, 30.0f, 1e-3f ), "two sparks 60 apart spread 30" );
		const float far0[3] = { 12000 - 30, -9000, 3000 }, far1[3] = { 12000 + 30, -9000, 3000 };
		burst.BeginFrame();
		burst.Add( far0, 1.0f );
		burst.Add( far1, 1.0f );
		const Light_t light = burst.Current();
		Check( Near( light.m_flSpread, 30.0f, 0.05f ) && Near( light.m_Origin[0], 12000.0f, 0.01f ),
		    "the spread and centroid stay precise far from the world origin" );
		burst.BeginFrame();
		burst.Add( a, 1.0f );
		Check( burst.Current().m_bLit && burst.Current().m_flSpread == 0.0f,
		    "one spark has no spread" );
		const float c[3] = { 0, 0, 90 };
		burst.BeginFrame();
		burst.Add( a, 3.0f );
		burst.Add( c, 1.0f );
		// Centroid (-22.5, 0, 22.5): E|p - c|^2 = ( 3 (7.5^2 + 22.5^2) + (22.5^2 + 67.5^2) ) / 4.
		Check( Near( burst.Current().m_flSpread, std::sqrt( 1687.5f ), 1e-3f ),
		    "the spread weighs each spark by its emission" );
	}

	// Radius: the base radius, growing 1.5 times the spread, at most 2.5
	// times the base.
	Check( LightRadius( 160.0f, 0.0f ) == 160.0f && Near( LightRadius( 160.0f, 40.0f ), 220.0f ) &&
	           LightRadius( 160.0f, 1000.0f ) == 400.0f && LightRadius( 160.0f, -5.0f ) == 160.0f,
	    "the light's radius grows with its sparks' spread, up to 2.5 times the base" );

	// A scripted burst of falling sparks: the light fades monotonically,
	// stays among the live sparks, and goes dark when the last spark dies.
	{
		// Thrown from 384 units up, they land on a floor at z = 0 and slide
		// outward: most of their life is spent beyond the base radius of the
		// source.
		std::vector<Spark> sparks;
		const float source[3] = { 0, 0, 384 };
		const float baseRadius = 192.0f;
		for ( int i = 0; i < 24; ++i )
		{
			const float angle = 0.2618f * float( i );
			Spark spark = { { source[0], source[1], source[2] },
			    { 250.0f * std::cos( angle ), 250.0f * std::sin( angle ),
			        100.0f + 5.0f * float( i ) },
			    0.0f, 0.4f + 0.05f * float( i ) };
			sparks.push_back( spark );
		}
		BurstUnderTest burst;
		burst.SetSource( source, baseRadius );
		const float dt = 1.0f / 60.0f;
		float lastScale = 2.0f;
		bool monotone = true, contained = true, darkAtEnd = false, litWhileAlive = true;
		bool covered = true, landed = false;
		for ( int frame = 0; frame < 120; ++frame )
		{
			burst.BeginFrame();
			float lo[3] = { 1e9f, 1e9f, 1e9f }, hi[3] = { -1e9f, -1e9f, -1e9f };
			int alive = 0;
			for ( Spark &spark : sparks )
			{
				spark.lifetime += dt;
				if ( spark.lifetime >= spark.dieTime )
					continue;
				spark.velocity[2] -= 800.0f * dt;
				for ( int k = 0; k < 3; ++k )
					spark.pos[k] += spark.velocity[k] * dt;
				if ( spark.pos[2] <= 0.0f )
				{
					spark.pos[2] = 0.0f;
					spark.velocity[2] = 0.0f;
					landed = true;
				}
				for ( int k = 0; k < 3; ++k )
				{
					lo[k] = std::fmin( lo[k], spark.pos[k] );
					hi[k] = std::fmax( hi[k], spark.pos[k] );
				}
				++alive;
				burst.Add(
				    spark.pos, TrailEmission( spark.lifetime, spark.dieTime, false, false ) );
			}
			const Light_t light = burst.Current();
			if ( alive == 0 )
			{
				darkAtEnd = !light.m_bLit;
				break;
			}
			// Coverage: most of the frame's emission lies within the light's
			// reach, wherever the sparks have gone.
			const float radius = RadiusUnderTest( baseRadius, light.m_flSpread );
			float total = 0.0f, inside = 0.0f;
			for ( const Spark &spark : sparks )
			{
				if ( spark.lifetime >= spark.dieTime )
					continue;
				const float emission = TrailEmission( spark.lifetime, spark.dieTime, false, false );
				float distSq = 0.0f;
				for ( int k = 0; k < 3; ++k )
					distSq +=
					    ( spark.pos[k] - light.m_Origin[k] ) * ( spark.pos[k] - light.m_Origin[k] );
				total += emission;
				if ( distSq <= radius * radius )
					inside += emission;
			}
			covered &= light.m_bLit && inside >= 0.9f * total;
			litWhileAlive &= light.m_bLit;
			monotone &= light.m_flScale <= lastScale;
			lastScale = light.m_flScale;
			for ( int k = 0; k < 3; ++k )
				contained &=
				    light.m_Origin[k] >= lo[k] - 1e-3f && light.m_Origin[k] <= hi[k] + 1e-3f;
		}
		Check( litWhileAlive, "a burst is lit while any spark is alive" );
		Check( monotone, "a burst's light only fades" );
		Check( lastScale < 0.05f, "a burst fades to near nothing before its last spark dies" );
		Check( contained, "the light stays within the live sparks' bounds" );
		Check( darkAtEnd, "a burst goes dark when its last spark dies" );
		Check( landed, "the scripted sparks reach the floor" );
		Check( covered, "the light's reach covers 90% of the sparks' emission in every frame" );
	}

	// Selection: the most important bursts at the viewer are lit, at most
	// the budget, whatever order they asked in.
	{
		const float view[3] = { 0, 0, 0 };
		std::vector<Candidate_t> candidates;
		// Equal bursts, the farthest asking first.
		for ( int i = 0; i < 6; ++i )
			candidates.push_back( MakeCandidate( 600.0f - 100.0f * float( i ), 1.0f ) );
		std::vector<bool> lit;
		const int count = SelectUnderTest( candidates, 4, view, &lit );
		Check( count == 4 && !lit[0] && !lit[1] && lit[2] && lit[3] && lit[4] && lit[5],
		    "the bursts nearest the viewer hold the light, not the first to ask" );
		const std::vector<Candidate_t> strong = {
		    MakeCandidate( 300.0f, 1.0f ), MakeCandidate( 300.0f, 4.0f ) };
		SelectUnderTest( strong, 1, view, &lit );
		Check( !lit[0] && lit[1], "a stronger burst at the same distance is more important" );
		SelectUnderTest( candidates, 0, view, &lit );
		bool none = true;
		for ( bool verdict : lit )
			none &= !verdict;
		Check( none, "a budget of 0 lights no burst" );
		const std::vector<Candidate_t> dark = {
		    MakeCandidate( 10.0f, 0.0f ), MakeCandidate( 500.0f, 1.0f ) };
		Check( SelectUnderTest( dark, 2, view, &lit ) == 1 && !lit[0] && lit[1],
		    "a burst of no strength is never lit" );
		const std::vector<Candidate_t> tie = {
		    MakeCandidate( 200.0f, 1.0f ), MakeCandidate( -200.0f, 1.0f ) };
		SelectUnderTest( tie, 1, view, &lit );
		Check( lit[0] && !lit[1], "ties go to the burst that asked first" );
	}

	// Hysteresis: a lit burst keeps the light against a newcomer only a
	// little more important, not against a clearly stronger one.
	{
		const float view[3] = { 0, 0, 0 };
		std::vector<bool> lit;
		const std::vector<Candidate_t> close = {
		    MakeCandidate( 300.0f, 1.0f, true ), MakeCandidate( 300.0f, 1.1f ) };
		SelectUnderTest( close, 1, view, &lit );
		Check( lit[0] && !lit[1], "a lit burst keeps its light against a slightly stronger newcomer" );
		const std::vector<Candidate_t> clear = {
		    MakeCandidate( 300.0f, 1.0f, true ), MakeCandidate( 300.0f, 1.5f ) };
		SelectUnderTest( clear, 1, view, &lit );
		Check( !lit[0] && lit[1], "a clearly more important burst takes the light" );
	}

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		// The seeded defect must be caught; the build reports the oracle's verdict.
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
