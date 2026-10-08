//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Positive conformance suite for the RFC 0001 threading capability
//			(PLAT-THREAD-001, Q-FOUNDATION). Runs the shared suite against the
//			deterministic backend with and without names and priorities.
//
//			Certifies contract semantics via a fake; NOT evidence of native thread
//			behavior.
//
//			Build/run: tools/quality/conformance.py check --suite platform.thread
//
//=============================================================================//

#include "fake_threads.h"
#include "thread_conformance.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

void RunVariant( const char *name, bool names, bool priority, int &checks, int &failures )
{
	platformtest::CFakeMonotonicClock clock;
	platformtest::CFakeThreads threads( clock, names, priority );
	const platformtest::ThreadReport r = platformtest::RunThreadConformance( threads, clock );
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
	RunVariant( "test_thread[names,priority]", true, true, checks, failures );
	RunVariant( "test_thread[no-names]", false, true, checks, failures );
	RunVariant( "test_thread[no-priority]", true, false, checks, failures );
	return testing::ReportConformance( checks, failures );
}
