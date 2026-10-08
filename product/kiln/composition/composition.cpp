//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.composition: the default provider catalog (RFC 0027 L0).
//
//=============================================================================//

#include "kiln/composition.h"

#include "jobsystem/graph_executor.h"
#include "product/stage_waf.h"
#include "product/toolchain_linux.h"
#include "product/toolchain_n3ds.h"

#include "../../../platform/posix/tool_process_provider.h"

#include <chrono>

namespace kiln
{

DefaultComposition::DefaultComposition() = default;
DefaultComposition::DefaultComposition( DefaultComposition && ) noexcept = default;
DefaultComposition &DefaultComposition::operator=( DefaultComposition && ) noexcept = default;
DefaultComposition::~DefaultComposition()
{
	// The catalog borrows the process provider; release it first.
	catalog = product::ProviderCatalog();
}

namespace
{
template <typename T>
std::optional<Error> AddTo( product::ProviderCatalog &catalog, std::unique_ptr<T> provider )
{
	auto added = catalog.Add( std::move( provider ) );
	if ( !added )
		return Error{ "catalog", added.Error().Describe() };
	return std::nullopt;
}

// "<os>-<machine>" as `uname -s -m` reports it, lowercased; asked through
// the process contract so this module needs no native header.
std::string HostTag( platform::IToolProcessProvider &processes )
{
	platform::ToolProcessRequest request;
	request.argv = { "uname", "-s", "-m" };
	request.workingDirectory = "/";
	request.executionTimeout = std::chrono::seconds( 10 );
	request.cancellationTimeout = std::chrono::seconds( 1 );
	const platform::ToolProcessResult result = processes.Run( request );
	if ( !result.Succeeded() )
		return "unknown-unknown";
	std::string tag;
	for ( const char c : result.stdoutData )
	{
		if ( c == '\n' || c == '\r' )
			break;
		tag += c == ' ' ? '-' : static_cast<char>( c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c );
	}
	return tag;
}
} // namespace

foundation::Expected<product::ProviderCatalog, Error> ComposeDefaultCatalog(
    platform::IToolProcessProvider &processes )
{
	product::ProviderCatalog catalog;
	if ( auto error = AddTo( catalog, product::CreateLinuxGccToolchain( processes ) ) )
		return foundation::MakeUnexpected( *error );
	if ( auto error = AddTo( catalog, product::CreateLinuxClangToolchain( processes ) ) )
		return foundation::MakeUnexpected( *error );
	if ( auto error = AddTo( catalog, product::CreateN3dsToolchain( processes ) ) )
		return foundation::MakeUnexpected( *error );
	if ( auto error = AddTo( catalog, product::CreateWafEngineStage() ) )
		return foundation::MakeUnexpected( *error );
	return catalog;
}

foundation::Expected<DefaultComposition, Error> ComposeDefault()
{
	DefaultComposition composition;
	composition.processes = platform::CreatePosixToolProcessProvider();
	if ( !composition.processes )
		return foundation::MakeUnexpected( Error{ "composition", "no process provider" } );
	composition.executor = std::make_unique<jobsystem::DeterministicExecutor>();
	auto catalog = ComposeDefaultCatalog( *composition.processes );
	if ( !catalog )
		return foundation::MakeUnexpected( catalog.Error() );
	composition.catalog = std::move( catalog ).Value();
	composition.hostTag = HostTag( *composition.processes );
	return composition;
}

} // namespace kiln
