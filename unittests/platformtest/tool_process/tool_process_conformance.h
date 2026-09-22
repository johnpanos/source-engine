//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared conformance predicate for structured tool processes.
//
//=============================================================================//

#ifndef PLATFORMTEST_TOOL_PROCESS_CONFORMANCE_H
#define PLATFORMTEST_TOOL_PROCESS_CONFORMANCE_H

#include "tool_process_fixture.h"

#include "platform/contracts/tool_process.h"
#include "platform/tool_process_client.h"

#include <cstdio>

namespace platformtest
{

struct ToolProcessReport
{
	int checks = 0;
	int failures = 0;
	const char *firstFailure = nullptr;
	int firstFailureLine = 0;

	void Record( bool ok, const char *what, int line )
	{
		++checks;
		if ( !ok )
		{
			++failures;
			if ( firstFailure == nullptr )
			{
				firstFailure = what;
				firstFailureLine = line;
			}
		}
	}
};

#define TP_CHECK( report, condition ) ( report ).Record( ( condition ), #condition, __LINE__ )

inline ToolProcessReport RunToolProcessConformance(
    platform::IToolProcessProvider &provider, const ToolProcessFixture &fixture )
{
	using platform::ToolProcessCompletion;
	using platform::ToolProcessErrorCode;

	ToolProcessReport report;
	platform::ToolProcessClient client( provider );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	// A real portable consumer invokes the provider. The registered fake child
	// exists only for this exact argv/cwd/environment/timeout tuple, so flattening
	// or dropping any input cannot accidentally select it.
	const platform::ToolProcessRequest exactBefore = fixture.exactRequest;
	platform::ToolProcessResult exact = client.Run( fixture.exactRequest );
	TP_CHECK( report, SameRequest( fixture.exactRequest, exactBefore ) );
	TP_CHECK( report, exact.completion == ToolProcessCompletion::kExited );
	TP_CHECK( report, exact.exitCode == 23 );
	TP_CHECK( report, exact.stdoutData == fixture.exactStdout );
	TP_CHECK( report, exact.stderrData == fixture.exactStderr );
	TP_CHECK( report, exact.error.IsOk() );
	TP_CHECK( report, !exact.cancellationAcknowledged );
	TP_CHECK( report, exact.cleanupComplete );
	TP_CHECK( report, !exact.Succeeded() );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	platform::ToolProcessResult success = client.Run( fixture.successRequest );
	TP_CHECK( report, success.completion == ToolProcessCompletion::kExited );
	TP_CHECK( report, success.exitCode == 0 );
	TP_CHECK( report, success.stdoutData == fixture.successStdout );
	TP_CHECK( report, success.stderrData.empty() );
	TP_CHECK( report, success.error.IsOk() );
	TP_CHECK( report, success.cleanupComplete );
	TP_CHECK( report, success.Succeeded() );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	platform::ToolProcessResult missing = client.Run( fixture.missingRequest );
	TP_CHECK( report, missing.completion == ToolProcessCompletion::kSpawnFailed );
	TP_CHECK( report, !missing.exitCode.has_value() );
	TP_CHECK( report, missing.stdoutData.empty() );
	TP_CHECK( report, missing.stderrData.empty() );
	TP_CHECK( report, missing.error.code == ToolProcessErrorCode::kExecutableNotFound );
	TP_CHECK( report, missing.error.requestedExecutable == fixture.missingRequest.argv[0] );
	TP_CHECK( report, !missing.cancellationAcknowledged );
	TP_CHECK( report, missing.cleanupComplete );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	platform::ToolProcessRequest invalid;
	platform::ToolProcessResult invalidResult = client.Run( invalid );
	TP_CHECK( report, invalidResult.completion == ToolProcessCompletion::kInvalidRequest );
	TP_CHECK( report, !invalidResult.exitCode.has_value() );
	TP_CHECK( report, invalidResult.error.code == ToolProcessErrorCode::kInvalidArgument );
	TP_CHECK( report, invalidResult.cleanupComplete );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	platform::ToolProcessResult timedOut = client.Run( fixture.timeoutRequest );
	TP_CHECK( report, timedOut.completion == ToolProcessCompletion::kTimedOut );
	TP_CHECK( report, !timedOut.exitCode.has_value() );
	TP_CHECK( report, timedOut.stdoutData == fixture.timeoutStdout );
	TP_CHECK( report, timedOut.stderrData == fixture.timeoutStderr );
	TP_CHECK( report, timedOut.error.IsOk() );
	TP_CHECK( report, timedOut.cancellationAcknowledged );
	TP_CHECK( report, timedOut.cleanupComplete );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	platform::ToolProcessResult canceled = client.Run( fixture.canceledRequest );
	TP_CHECK( report, canceled.completion == ToolProcessCompletion::kCanceled );
	TP_CHECK( report, !canceled.exitCode.has_value() );
	TP_CHECK( report, canceled.stdoutData == fixture.canceledStdout );
	TP_CHECK( report, canceled.stderrData == fixture.canceledStderr );
	TP_CHECK( report, canceled.error.IsOk() );
	TP_CHECK( report, canceled.cancellationAcknowledged );
	TP_CHECK( report, canceled.cleanupComplete );
	TP_CHECK( report, provider.LiveProcessCount() == 0 );

	return report;
}

inline int ReportToolProcessConformance( const char *suiteName, const ToolProcessReport &report )
{
	if ( report.failures != 0 )
	{
		std::printf( "FAIL %s: %d/%d checks failed; first: %s (line %d)\n", suiteName,
		    report.failures, report.checks, report.firstFailure, report.firstFailureLine );
		return 1;
	}
	std::printf( "ok %s: %d checks passed\n", suiteName, report.checks );
	return 0;
}

} // namespace platformtest

#endif // PLATFORMTEST_TOOL_PROCESS_CONFORMANCE_H
