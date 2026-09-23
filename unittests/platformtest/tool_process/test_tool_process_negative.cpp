//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity suite for the structured tool-process contract.
//
//=============================================================================//

#include "fake_tool_process.h"
#include "tool_process_conformance.h"
#include "tool_process_fixture.h"
#include "testing/conformance_result.h"

#include <cstdio>

namespace
{

struct DefectCase
{
	platformtest::FakeToolProcessDefect defect;
	const char *name;
};

} // namespace

int main()
{
	using platformtest::FakeToolProcessDefect;

	platformtest::CTestCancellation cancellation( true );
	const platformtest::ToolProcessFixture fixture =
	    platformtest::MakeToolProcessFixture( &cancellation );
	int checks = 0;
	int failures = 0;

	{
		platformtest::CFakeToolProcessProvider good( fixture );
		const platformtest::ToolProcessReport report =
		    platformtest::RunToolProcessConformance( good, fixture );
		++checks;
		if ( report.failures != 0 )
		{
			std::printf( "FAIL: conforming provider rejected: %s (line %d)\n", report.firstFailure,
			    report.firstFailureLine );
			++failures;
		}
	}

	const DefectCase cases[] = {
	    { FakeToolProcessDefect::kFlattenArguments, "flatten-argv" },
	    { FakeToolProcessDefect::kIgnoreWorkingDirectory, "ignore-cwd" },
	    { FakeToolProcessDefect::kIgnoreEnvironment, "ignore-environment" },
	    { FakeToolProcessDefect::kMergeOutputStreams, "merge-output-streams" },
	    { FakeToolProcessDefect::kWrongExitCode, "wrong-exit-code" },
	    { FakeToolProcessDefect::kSpawnFailureAsSuccess, "spawn-failure-as-success" },
	    { FakeToolProcessDefect::kTimeoutAsExit, "timeout-as-exit" },
	    { FakeToolProcessDefect::kCancellationUnacknowledged, "unacknowledged-cancel" },
	    { FakeToolProcessDefect::kIncompleteCleanup, "incomplete-cleanup" },
	    { FakeToolProcessDefect::kLeakProcess, "leaked-process" },
	};

	for ( const DefectCase &defectCase : cases )
	{
		platformtest::CFakeToolProcessProvider broken( fixture, defectCase.defect );
		const platformtest::ToolProcessReport report =
		    platformtest::RunToolProcessConformance( broken, fixture );
		++checks;
		if ( report.failures == 0 )
		{
			std::printf( "FAIL: broken provider '%s' was not caught\n", defectCase.name );
			++failures;
		}
	}

	if ( failures == 0 )
	{
		std::printf( "ok platform.tool_process.sensitivity: all %zu defects caught\n",
		    sizeof( cases ) / sizeof( cases[0] ) );
	}
	return testing::ReportConformance( checks, failures );
}
