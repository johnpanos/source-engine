//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic provider for the structured tool-process contract.
//
//=============================================================================//

#ifndef PLATFORMTEST_FAKE_TOOL_PROCESS_H
#define PLATFORMTEST_FAKE_TOOL_PROCESS_H

#include "tool_process_fixture.h"

#include "platform/contracts/tool_process.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace platformtest
{

enum class FakeToolProcessDefect
{
	kNone = 0,
	kFlattenArguments,
	kIgnoreWorkingDirectory,
	kIgnoreEnvironment,
	kMergeOutputStreams,
	kWrongExitCode,
	kSpawnFailureAsSuccess,
	kTimeoutAsExit,
	kCancellationUnacknowledged,
	kIncompleteCleanup,
	kLeakProcess,
};

enum class FakeToolProcessBehavior
{
	kExit = 0,
	kWaitForTermination,
};

struct FakeToolProcessScript
{
	platform::ToolProcessRequest request;
	FakeToolProcessBehavior behavior = FakeToolProcessBehavior::kExit;
	int exitCode = 0;
	std::string stdoutData;
	std::string stderrData;
};

class CFakeToolProcessProvider final : public platform::IToolProcessProvider
{
public:
	explicit CFakeToolProcessProvider( const ToolProcessFixture &fixture,
	    FakeToolProcessDefect defect = FakeToolProcessDefect::kNone )
	    : m_Defect( defect )
	{
		m_Scripts.push_back( { fixture.exactRequest, FakeToolProcessBehavior::kExit, 23,
		    fixture.exactStdout, fixture.exactStderr } );
		m_Scripts.push_back( { fixture.successRequest, FakeToolProcessBehavior::kExit, 0,
		    fixture.successStdout, std::string() } );
		m_Scripts.push_back( { fixture.timeoutRequest, FakeToolProcessBehavior::kWaitForTermination,
		    0, fixture.timeoutStdout, fixture.timeoutStderr } );
		m_Scripts.push_back(
		    { fixture.canceledRequest, FakeToolProcessBehavior::kWaitForTermination, 0,
		        fixture.canceledStdout, fixture.canceledStderr } );
	}

	platform::ToolProcessResult Run( const platform::ToolProcessRequest &request ) override
	{
		platform::ToolProcessError validationError = Validate( request );
		if ( !validationError.IsOk() )
		{
			platform::ToolProcessResult result;
			result.completion = platform::ToolProcessCompletion::kInvalidRequest;
			result.error = std::move( validationError );
			result.cleanupComplete = true;
			return result;
		}

		const FakeToolProcessScript *script = FindScript( request );
		if ( script == nullptr )
		{
			if ( m_Defect == FakeToolProcessDefect::kSpawnFailureAsSuccess &&
			     request.argv[0] == "missing-tool" )
			{
				return Exited( 0, std::string(), std::string() );
			}
			platform::ToolProcessResult result;
			result.completion = platform::ToolProcessCompletion::kSpawnFailed;
			result.error.code = platform::ToolProcessErrorCode::kExecutableNotFound;
			result.error.requestedExecutable = request.argv[0];
			result.error.detail = "fixture executable not registered";
			result.cleanupComplete = true;
			return result;
		}

		++m_LiveProcesses;
		platform::ToolProcessResult result;
		const bool canceled =
		    request.cancellation != nullptr && request.cancellation->IsCancellationRequested();
		if ( canceled )
		{
			result.completion = platform::ToolProcessCompletion::kCanceled;
			result.cancellationAcknowledged =
			    m_Defect != FakeToolProcessDefect::kCancellationUnacknowledged;
		}
		else if ( script->behavior == FakeToolProcessBehavior::kWaitForTermination )
		{
			if ( m_Defect == FakeToolProcessDefect::kTimeoutAsExit )
			{
				result.completion = platform::ToolProcessCompletion::kExited;
				result.exitCode = 0;
			}
			else
			{
				result.completion = platform::ToolProcessCompletion::kTimedOut;
				result.cancellationAcknowledged = true;
			}
		}
		else
		{
			result.completion = platform::ToolProcessCompletion::kExited;
			result.exitCode = script->exitCode;
			if ( m_Defect == FakeToolProcessDefect::kWrongExitCode )
			{
				result.exitCode = script->exitCode + 1;
			}
		}

		result.stdoutData = script->stdoutData;
		result.stderrData = script->stderrData;
		if ( m_Defect == FakeToolProcessDefect::kMergeOutputStreams )
		{
			result.stdoutData += result.stderrData;
			result.stderrData.clear();
		}

		result.cleanupComplete = m_Defect != FakeToolProcessDefect::kIncompleteCleanup;
		if ( m_Defect != FakeToolProcessDefect::kLeakProcess )
		{
			--m_LiveProcesses;
		}
		return result;
	}

	int LiveProcessCount() const noexcept override { return m_LiveProcesses; }

private:
	static bool ContainsNul( const std::string &value )
	{
		return value.find( '\0' ) != std::string::npos;
	}

	static platform::ToolProcessError Validate( const platform::ToolProcessRequest &request )
	{
		platform::ToolProcessError error;
		error.requestedExecutable = request.argv.empty() ? std::string() : request.argv[0];
		if ( request.argv.empty() || request.argv[0].empty() ||
		     request.executionTimeout <= std::chrono::milliseconds::zero() ||
		     request.cancellationTimeout <= std::chrono::milliseconds::zero() ||
		     request.workingDirectory.empty() )
		{
			error.code = platform::ToolProcessErrorCode::kInvalidArgument;
			error.detail = "missing executable, cwd, or positive timeout";
			return error;
		}
		for ( const std::string &argument : request.argv )
		{
			if ( ContainsNul( argument ) )
			{
				error.code = platform::ToolProcessErrorCode::kInvalidArgument;
				error.detail = "argument contains NUL";
				return error;
			}
		}
		if ( ContainsNul( request.workingDirectory ) )
		{
			error.code = platform::ToolProcessErrorCode::kInvalidArgument;
			error.detail = "working directory contains NUL";
			return error;
		}
		for ( std::size_t i = 0; i < request.environment.size(); ++i )
		{
			const auto &entry = request.environment[i];
			if ( entry.name.empty() || entry.name.find( '=' ) != std::string::npos ||
			     ContainsNul( entry.name ) || ( entry.value && ContainsNul( *entry.value ) ) )
			{
				error.code = platform::ToolProcessErrorCode::kInvalidArgument;
				error.detail = "invalid environment override";
				return error;
			}
			for ( std::size_t j = 0; j < i; ++j )
			{
				if ( request.environment[j].name == entry.name )
				{
					error.code = platform::ToolProcessErrorCode::kInvalidArgument;
					error.detail = "duplicate environment override";
					return error;
				}
			}
		}
		return error;
	}

	const FakeToolProcessScript *FindScript( const platform::ToolProcessRequest &request ) const
	{
		platform::ToolProcessRequest effective = request;
		if ( m_Defect == FakeToolProcessDefect::kFlattenArguments )
		{
			std::string command;
			for ( const std::string &argument : effective.argv )
			{
				if ( !command.empty() )
				{
					command += ' ';
				}
				command += argument;
			}
			effective.argv = { command };
		}
		if ( m_Defect == FakeToolProcessDefect::kIgnoreWorkingDirectory )
		{
			effective.workingDirectory.clear();
		}
		if ( m_Defect == FakeToolProcessDefect::kIgnoreEnvironment )
		{
			effective.environment.clear();
		}

		const auto found = std::find_if( m_Scripts.begin(), m_Scripts.end(),
		    [&]( const FakeToolProcessScript &script )
		    {
			    return script.request.argv == effective.argv &&
			           script.request.workingDirectory == effective.workingDirectory &&
			           script.request.environment == effective.environment &&
			           script.request.executionTimeout == effective.executionTimeout &&
			           script.request.cancellationTimeout == effective.cancellationTimeout;
		    } );
		return found == m_Scripts.end() ? nullptr : &*found;
	}

	static platform::ToolProcessResult Exited(
	    int exitCode, std::string stdoutData, std::string stderrData )
	{
		platform::ToolProcessResult result;
		result.completion = platform::ToolProcessCompletion::kExited;
		result.exitCode = exitCode;
		result.stdoutData = std::move( stdoutData );
		result.stderrData = std::move( stderrData );
		result.cleanupComplete = true;
		return result;
	}

	std::vector<FakeToolProcessScript> m_Scripts;
	FakeToolProcessDefect m_Defect;
	int m_LiveProcesses = 0;
};

} // namespace platformtest

#endif // PLATFORMTEST_FAKE_TOOL_PROCESS_H
