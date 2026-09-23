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
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

// Runs the shared suite for one variant, prints its summary and adds its check
// counts to the suite-wide totals reported to the runner.
int RunVariant( const char *suiteName, const platform::IMonotonicClock &clock, int sampleCount,
    int &checks, int &failures )
{
	const platformtest::ClockReport r =
	    platformtest::RunMonotonicClockConformance( clock, sampleCount );
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", suiteName, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, r.checks );
	return 0;
}

} // namespace

int main()
{
	int rc = 0;
	int checks = 0;
	int failures = 0;

	// Default resolution/step.
	{
		platformtest::CFakeMonotonicClock clock;
		rc |= RunVariant( "test_clock[1us]", clock, 8, checks, failures );
	}

	// A coarser clock with a larger, odd step -- exercises additivity with tick
	// deltas that are not powers of two.
	{
		platformtest::CFakeMonotonicClock clock( /*resolutionNs=*/1000000, /*stepNs=*/333333 );
		rc |= RunVariant( "test_clock[1ms/odd-step]", clock, 12, checks, failures );
	}

	if ( rc == 0 )
	{
		std::printf( "ok test_clock: all variants passed\n" );
	}
	return testing::ReportConformance( checks, failures );
}
