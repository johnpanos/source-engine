//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 wall-clock capability
//			(PLAT-WALLCLOCK-001, Q-FOUNDATION). Runs the shared suite against the
//			deterministic backend with positive, negative and absent local zones.
//
//			Certifies contract semantics via a fake; NOT evidence of native clock
//			or zone-database behavior.
//
//			Build/run: tools/quality/conformance.py check --suite platform.wall_clock
//
//=============================================================================//

#include "fake_wall_clock.h"
#include "wall_clock_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

int RunVariant( const char *name, const platform::IWallClock &clock, int &checks, int &failures )
{
	const platformtest::WallClockReport r = platformtest::RunWallClockConformance( clock );
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
	return 0;
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		platformtest::CFakeWallClock clock; // UTC+2
		RunVariant( "test_wall_clock[utc+2]", clock, checks, failures );
	}
	{
		// UTC-9:30 and an instant before the epoch.
		platformtest::CFakeWallClock clock( -86400LL * 1000000000LL - 5, -( 9 * 3600 + 1800 ) );
		RunVariant( "test_wall_clock[utc-9:30/pre-epoch]", clock, checks, failures );
	}
	{
		platformtest::CFakeWallClock clock( 0, 0, /*hasLocalZone=*/false );
		RunVariant( "test_wall_clock[no-local-zone]", clock, checks, failures );
	}
	return testing::ReportConformance( checks, failures );
}
