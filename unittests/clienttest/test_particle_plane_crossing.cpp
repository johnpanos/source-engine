//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: client.particle-plane-crossing.v1 - when a particle's step hits one
//          of its collision planes (game/client/particle_plane_crossing.h).
//          The sensitivity build restores the original predicate, which lets
//          a particle that starts on a surface fall through it; the oracle
//          must reject it.
//
//===========================================================================//

#include "game/client/particle_plane_crossing.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;
#ifdef PARTICLE_PLANE_CROSSING_SEEDED_ORIGINAL
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

constexpr float kEpsilon = 0.01f; // particle_collision.cpp COLLISION_EPSILON

bool Hits( float start, float end, float *fraction )
{
#ifdef PARTICLE_PLANE_CROSSING_SEEDED_ORIGINAL
	// CBaseSimpleCollision::TraceLine before the fix.
	if ( start < -kEpsilon )
		return false;
	if ( ( start > kEpsilon ) == ( end > kEpsilon ) )
		return false;
	*fraction = start / ( start - end );
	return true;
#else
	return ParticlePlaneCrossing::Hits( start, end, kEpsilon, fraction );
#endif
}

} // namespace

int main()
{
	float t = -1.0f;
	Check( Hits( 10.0f, -10.0f, &t ) && std::fabs( t - 0.5f ) < 1e-6f,
	    "a step through the plane hits it halfway" );
	Check( Hits( 4.0f, 0.0f, &t ) && t == 1.0f, "a step that ends on the plane hits it" );
	Check( !Hits( 10.0f, 5.0f, &t ), "a step that stays in front misses" );
	Check( !Hits( -5.0f, -10.0f, &t ) && !Hits( -5.0f, 5.0f, &t ),
	    "a particle behind the plane never hits it" );

	// The fix: a particle that starts on the surface.
	t = -1.0f;
	Check( Hits( 0.0f, -3.0f, &t ) && t == 0.0f,
	    "a particle on the plane moving into it hits it at once" );
	Check( Hits( 0.005f, -3.0f, &t ) && t > 0.0f && t < 0.01f,
	    "a particle within epsilon of the plane moving into it hits it" );
	Check( Hits( -0.005f, -3.0f, &t ), "a particle a hair behind moving in still hits" );
	Check( !Hits( 0.0f, 3.0f, &t ) && !Hits( 0.005f, 2.0f, &t ),
	    "a particle leaving the plane (just bounced) does not hit it again" );
	Check( !Hits( 0.0f, 0.005f, &t ) && !Hits( 0.0f, -0.005f, &t ),
	    "a particle sliding within the epsilon band does not hit" );

	// Every hit's fraction lies within the step.
	bool bounded = true;
	for ( int a = -20; a <= 20; ++a )
		for ( int b = -20; b <= 20; ++b )
		{
			const float start = 0.001f * float( a * a * a ) / 8.0f;
			const float end = 0.001f * float( b * b * b ) / 8.0f;
			float f = 0.0f;
			if ( Hits( start, end, &f ) )
				bounded &= f >= -1e-6f && f <= 1.0f + 1e-6f && end < start;
		}
	Check( bounded, "every hit moves toward the plane with a fraction in [0, 1]" );

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
