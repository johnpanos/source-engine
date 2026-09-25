//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.spark-light.v1 - the light a burst of trail sparks emits
//          (render/spark_light.h, RFC 0011 light set): the drawn ramp
//          against the original CTrailParticles expressions, the emission-
//          weighted centroid, the fade against the burst's peak, batching
//          and the burst budget. The sensitivity builds substitute defective
//          policies the oracle must reject.
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
    defined( SPARK_LIGHT_SEEDED_BUDGET_LEAK )
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
	void BeginFrame() { m_burst.BeginFrame(); }
	void Add( const float pos[3], float emission )
	{
#ifdef SPARK_LIGHT_SEEDED_UNWEIGHTED
		// Every live spark counts the same, however much of it is drawn.
		m_burst.Add( pos, emission > 0.0f ? 1.0f : 0.0f );
#else
		m_burst.Add( pos, emission );
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
};

class BudgetUnderTest
{
public:
	bool Acquire( int max ) { return m_budget.Acquire( max ); }
	void Release()
	{
#ifndef SPARK_LIGHT_SEEDED_BUDGET_LEAK
		m_budget.Release();
#endif
	}
	int Held() const { return m_budget.Held(); }

private:
	CBudget m_budget;
};

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

	// A scripted burst of falling sparks: the light fades monotonically,
	// stays among the live sparks, and goes dark when the last spark dies.
	{
		std::vector<Spark> sparks;
		for ( int i = 0; i < 24; ++i )
		{
			const float angle = 0.2618f * float( i );
			Spark spark = { { 0, 0, 64 },
			    { 150.0f * std::cos( angle ), 150.0f * std::sin( angle ),
			        100.0f + 5.0f * float( i ) },
			    0.0f, 0.4f + 0.05f * float( i ) };
			sparks.push_back( spark );
		}
		BurstUnderTest burst;
		const float dt = 1.0f / 60.0f;
		float lastScale = 2.0f;
		bool monotone = true, contained = true, darkAtEnd = false, litWhileAlive = true;
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
				{
					spark.pos[k] += spark.velocity[k] * dt;
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
	}

	// The budget: at most nMax bursts hold a light; a release frees one.
	{
		BudgetUnderTest budget;
		Check( !budget.Acquire( 0 ), "a budget of 0 lights no burst" );
		Check( budget.Acquire( 2 ) && budget.Acquire( 2 ) && !budget.Acquire( 2 ),
		    "no more bursts than the budget hold a light" );
		budget.Release();
		Check( budget.Held() == 1 && budget.Acquire( 2 ), "a released light frees its place" );
		budget.Release();
		budget.Release();
		Check( budget.Held() == 0, "every released light is returned" );
		budget.Release();
		Check( budget.Held() == 0 && budget.Acquire( 1 ) && !budget.Acquire( 1 ),
		    "an extra release does not raise the budget" );
		budget.Release();
		// A lowered budget lights no new burst until enough are released.
		Check( budget.Acquire( 3 ) && budget.Acquire( 3 ) && !budget.Acquire( 1 ),
		    "a lowered budget takes effect for new bursts" );
		budget.Release();
		budget.Release();
		Check( budget.Acquire( 1 ), "a lowered budget lights a burst once enough are released" );
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
