//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 diagnostics capability
//			(PLAT-DIAG-001, Q-FOUNDATION). Runs the shared suites against the
//			deterministic debug output and crash reporter, available and not.
//
//			Certifies contract semantics via fakes; NOT evidence of native
//			output or crash capture.
//
//			Build/run: tools/quality/conformance.py check --suite platform.diagnostics
//
//=============================================================================//

#include "diagnostics_conformance.h"
#include "fake_diagnostics.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

void Tally( const char *name, const platformtest::DiagnosticsReport &r, int &checks, int &failures )
{
	checks += r.checks;
	failures += r.failures;
	if ( r.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", name, r.failures,
		    r.checks, r.firstFailure, r.firstFailureLine );
		return;
	}
	std::printf( "ok %s: %d checks passed\n", name, r.checks );
}

} // namespace

int main()
{
	int checks = 0;
	int failures = 0;
	{
		platformtest::CFakeDebugOutput output;
		Tally( "test_diagnostics[debug-output]",
		    platformtest::RunDebugOutputConformance( output, output ), checks, failures );
	}
	{
		platformtest::CFakeCrashReporter reporter;
		Tally( "test_diagnostics[crash-reporter]",
		    platformtest::RunCrashReporterConformance( reporter ), checks, failures );
	}
	{
		platformtest::CFakeCrashReporter reporter( /*available=*/false );
		Tally( "test_diagnostics[crash-reporter-unavailable]",
		    platformtest::RunCrashReporterConformance( reporter ), checks, failures );
	}
	return testing::ReportConformance( checks, failures );
}
