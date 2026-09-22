//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deterministic fixture requests for tool-process conformance.
//
//=============================================================================//

#ifndef PLATFORMTEST_TOOL_PROCESS_FIXTURE_H
#define PLATFORMTEST_TOOL_PROCESS_FIXTURE_H

#include "platform/contracts/tool_process.h"

#include <atomic>
#include <chrono>
#include <optional>
#include <string>

namespace platformtest
{

class CTestCancellation final : public platform::IToolProcessCancellation
{
public:
	explicit CTestCancellation( bool requested = false ) : m_Requested( requested ) {}

	bool IsCancellationRequested() const noexcept override
	{
		return m_Requested.load( std::memory_order_acquire );
	}

	void Request() noexcept { m_Requested.store( true, std::memory_order_release ); }

private:
	std::atomic<bool> m_Requested;
};

struct ToolProcessFixture
{
	platform::ToolProcessRequest exactRequest;
	platform::ToolProcessRequest successRequest;
	platform::ToolProcessRequest timeoutRequest;
	platform::ToolProcessRequest canceledRequest;
	platform::ToolProcessRequest missingRequest;
	std::string exactStdout;
	std::string exactStderr;
	std::string successStdout;
	std::string timeoutStdout;
	std::string timeoutStderr;
	std::string canceledStdout;
	std::string canceledStderr;
};

inline platform::ToolProcessRequest BaseRequest( const char *mode )
{
	platform::ToolProcessRequest request;
	request.argv = { "fixture-tool", mode };
	request.workingDirectory = "/workspace/tool fixture";
	request.environment = {
	    { "TOOL_MODE", std::string( "value with spaces" ) },
	    { "EMPTY_VALUE", std::string() },
	    { "REMOVE_ME", std::nullopt },
	};
	request.executionTimeout = std::chrono::milliseconds( 250 );
	request.cancellationTimeout = std::chrono::milliseconds( 50 );
	return request;
}

inline ToolProcessFixture MakeToolProcessFixture(
    const platform::IToolProcessCancellation *cancellation )
{
	ToolProcessFixture fixture;
	fixture.exactRequest = BaseRequest( "" );
	fixture.exactRequest.argv.push_back( "argument with spaces" );
	fixture.exactRequest.argv.push_back( "quote\"inside" );
	fixture.exactRequest.argv.push_back( "caf\xc3\xa9" );
	fixture.exactRequest.argv.push_back( "slash\\value" );

	fixture.successRequest = BaseRequest( "--success" );
	fixture.timeoutRequest = BaseRequest( "--timeout" );
	fixture.timeoutRequest.executionTimeout = std::chrono::milliseconds( 1 );
	fixture.canceledRequest = BaseRequest( "--cancel" );
	fixture.canceledRequest.cancellation = cancellation;
	fixture.missingRequest = BaseRequest( "--version" );
	fixture.missingRequest.argv[0] = "missing-tool";

	fixture.exactStdout = std::string( "stdout\0tail", 11 );
	fixture.exactStderr = "stderr line\n";
	fixture.successStdout = "consumer completed\n";
	fixture.timeoutStdout = "partial stdout\n";
	fixture.timeoutStderr = "partial stderr\n";
	fixture.canceledStdout = "stdout before cancellation\n";
	fixture.canceledStderr = "stderr before cancellation\n";
	return fixture;
}

inline bool SameRequest(
    const platform::ToolProcessRequest &left, const platform::ToolProcessRequest &right )
{
	return left.argv == right.argv && left.workingDirectory == right.workingDirectory &&
	       left.environment == right.environment &&
	       left.executionTimeout == right.executionTimeout &&
	       left.cancellationTimeout == right.cancellationTimeout &&
	       left.cancellation == right.cancellation;
}

} // namespace platformtest

#endif // PLATFORMTEST_TOOL_PROCESS_FIXTURE_H
