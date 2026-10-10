//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Running one host process for the Android providers.
//
//=============================================================================//

#ifndef PRODUCT_ANDROID_PROCESS_H
#define PRODUCT_ANDROID_PROCESS_H

#include "product/contracts.h"

#include <chrono>
#include <string>
#include <vector>

namespace product::android
{

struct Output
{
	std::string out;
	std::string err;
};

// Runs argv; success is exit status 0. The error is one line plus the first
// line of stderr. A cancelled run reports "cancelled".
inline foundation::Expected<Output, std::string> Run( platform::IToolProcessProvider &processes,
    std::vector<std::string> argv, const ICancellation *cancel = nullptr,
    std::chrono::milliseconds timeout = std::chrono::seconds( 60 ),
    std::vector<platform::ToolProcessEnvironmentOverride> environment = {},
    std::string workingDirectory = "/" )
{
	class Adapter final : public platform::IToolProcessCancellation
	{
	public:
		explicit Adapter( const ICancellation *cancel ) : m_Cancel( cancel ) {}
		bool IsCancellationRequested() const noexcept override
		{
			return m_Cancel && m_Cancel->IsCancelled();
		}

	private:
		const ICancellation *m_Cancel;
	};
	platform::ToolProcessRequest request;
	request.argv = std::move( argv );
	request.workingDirectory = std::move( workingDirectory );
	request.environment = std::move( environment );
	request.executionTimeout = timeout;
	request.cancellationTimeout = std::chrono::seconds( 5 );
	Adapter adapter( cancel );
	request.cancellation = &adapter;
	platform::ToolProcessResult result = processes.Run( request );
	if ( result.completion == platform::ToolProcessCompletion::kCanceled )
		return foundation::MakeUnexpected( std::string( "cancelled" ) );
	if ( !result.Succeeded() )
	{
		std::string detail = result.error.detail;
		if ( detail.empty() )
			detail = result.exitCode ? "exit status " + std::to_string( *result.exitCode )
			                         : "did not run";
		const std::string &text = result.stderrData.empty() ? result.stdoutData : result.stderrData;
		if ( !text.empty() )
			detail += ": " + text.substr( 0, text.find( '\n' ) );
		return foundation::MakeUnexpected( detail );
	}
	return Output{ std::move( result.stdoutData ), std::move( result.stderrData ) };
}

inline std::string FirstLine( const std::string &text )
{
	return text.substr( 0, text.find( '\n' ) );
}

} // namespace product::android

#endif // PRODUCT_ANDROID_PROCESS_H
