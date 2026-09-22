//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 monotonic-clock capability
//			(PLAT-CLOCK-001, Q-FOUNDATION). Runs the shared suite against the
//			deterministic virtual-time backend on linux-headless-core.
//
//			Certifies contract semantics via a fake; NOT evidence of native timer
//			behavior (a real monotonic-clock provider runs the same shared suite).
//
//			Build/run: tools/quality/conformance.py check --suite platform.clock
//
//=============================================================================//

#include "clock_conformance.h"
#include "fake_clock.h"

#include <cstdio>

int main()
{
	int rc = 0;

	// Default resolution/step.
	{
		platformtest::CFakeMonotonicClock clock;
		rc |= platformtest::RunClockPositive( "test_clock[1us]", clock, 8 );
	}

	// A coarser clock with a larger, odd step -- exercises additivity with tick
	// deltas that are not powers of two.
	{
		platformtest::CFakeMonotonicClock clock( /*resolutionNs=*/1000000, /*stepNs=*/333333 );
		rc |= platformtest::RunClockPositive( "test_clock[1ms/odd-step]", clock, 12 );
	}

	if ( rc == 0 )
	{
		std::printf( "ok test_clock: all variants passed\n" );
	}
	return rc;
}
