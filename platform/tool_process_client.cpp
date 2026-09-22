//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Small portable consumer of the structured tool-process capability.
//
//=============================================================================//

#include "platform/tool_process_client.h"

#include <utility>

namespace platform
{

namespace
{

ToolProcessResult ClientFailure(
    ToolProcessErrorCode code, const ToolProcessRequest &request, const char *detail )
{
	ToolProcessResult result;
	result.completion = ToolProcessCompletion::kProviderFailure;
	result.error.code = code;
	result.error.requestedExecutable = request.argv.empty() ? std::string() : request.argv[0];
	result.error.detail = detail;
	return result;
}

} // namespace

ToolProcessResult ToolProcessClient::Run( const ToolProcessRequest &request )
{
	const int liveBefore = m_Provider.LiveProcessCount();
	if ( liveBefore != 0 )
	{
		return ClientFailure(
		    ToolProcessErrorCode::kBusy, request, "provider already owns a live process" );
	}

	ToolProcessResult result = m_Provider.Run( request );
	if ( !result.cleanupComplete || m_Provider.LiveProcessCount() != liveBefore )
	{
		ToolProcessResult failure = ClientFailure(
		    ToolProcessErrorCode::kCleanupFailed, request, "provider returned before cleanup" );
		failure.stdoutData = std::move( result.stdoutData );
		failure.stderrData = std::move( result.stderrData );
		return failure;
	}
	return result;
}

} // namespace platform
