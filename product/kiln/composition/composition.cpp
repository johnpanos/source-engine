//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.composition: the default provider catalog (RFC 0027 L0).
//
//=============================================================================//

#include "kiln/composition.h"

#include "jobsystem/graph_executor.h"
#include "product/package_linux_dir.h"
#include "product/display_desktop.h"
#include "product/run_desktop.h"
#include "product/stage_fstop.h"
#include "product/stage_video_av1.h"
#include "product/stage_waf.h"
#include "product/toolchain_linux.h"
#include "product/toolchain_msvc_wine.h"
#include "product/platform_android.h"

#include "../../../platform/posix/foundation_providers.h"
#include "../../../platform/posix/process_spawner.h"
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

// Adds each provider in order; stops at the first refusal.
template <typename... P>
std::optional<Error> AddAll( product::ProviderCatalog &catalog, std::unique_ptr<P>... providers )
{
	std::optional<Error> error;
	( ( error = error ? error : AddTo( catalog, std::move( providers ) ) ), ... );
	return error;
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
	const std::vector<std::filesystem::path> dbusConfigs = { "/usr/share/dbus-1/session.conf",
	    "/etc/dbus-1/session.conf" };
	if ( auto error = AddAll( catalog, product::CreateLinuxGccToolchain( processes ),
	         product::CreateLinuxClangToolchain( processes ),
	         product::CreateAndroidNdkToolchain( processes ),
	         product::CreateAndroidNativeLibsStage( std::string( product::kEngineInstallArtifact ) ),
	         product::CreateAndroidApkPackager( processes ), product::CreateAdbTransport( processes ),
	         product::CreateMsvcWineToolchain( processes ), product::CreateWafEngineStage(),
	         product::CreateFstopContentStage(), product::CreateVideoAv1Stage(),
	         product::CreateLinuxDirPackager(), product::CreateWindowsDirPackager(),
	         product::CreateUserDisplaySession(), product::CreateHeadlessDisplaySession(),
	         product::CreatePrivateDisplaySession( dbusConfigs ),
	         product::CreatePrivateX11DisplaySession( dbusConfigs ) ) )
		return foundation::MakeUnexpected( *error );
	for ( auto *create :
	    { &product::CreateSingleRunProvider, &product::CreateExternalInstallRunProvider,
	        &product::CreateCoopPairRunProvider, &product::CreateBrowserPageRunProvider } )
	{
		if ( auto error = AddTo( catalog, ( *create )() ) )
			return foundation::MakeUnexpected( *error );
	}
	return catalog;
}

foundation::Expected<DefaultComposition, Error> ComposeDefault( int argc, const char *const *argv )
{
	DefaultComposition composition;
	composition.environment = platform::CreatePosixProcessEnvironment( argc, argv );
	composition.processes = platform::CreatePosixToolProcessProvider();
	if ( !composition.processes )
		return foundation::MakeUnexpected( Error{ "composition", "no process provider" } );
	composition.executor = std::make_unique<jobsystem::DeterministicExecutor>();
	composition.spawner = platform::CreatePosixProcessSpawner();
	auto catalog = ComposeDefaultCatalog( *composition.processes );
	if ( !catalog )
		return foundation::MakeUnexpected( catalog.Error() );
	composition.catalog = std::move( catalog ).Value();
	composition.hostTag = HostTag( *composition.processes );
	return composition;
}

} // namespace kiln
