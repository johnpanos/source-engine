//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: play_embedded (RFC 0027 sample): a kiln.api play request from a
//			  catalog this program composes itself. It links the libraries
//			  only: no kiln application and no default composition. Its
//			  catalog holds the providers a desktop play needs (the gcc
//			  toolchain, the Waf engine stage, the linux-dir packager, a
//			  user and headless displays and the single run provider), and its spawner
//			  records each program instead of starting it unless --start.
//
//			  play_embedded <root> <profile> [map] [--start]
//
//			  Prints the spawned argv, one argument per line, after "spawn:".
//
//=============================================================================//

#include "jobsystem/graph_executor.h"
#include "kiln/api.h"
#include "product/display_desktop.h"
#include "product/package_linux_dir.h"
#include "product/run_desktop.h"
#include "product/stage_waf.h"
#include "product/toolchain_linux.h"

#include "../../../platform/posix/foundation_providers.h"
#include "../../../platform/posix/process_spawner.h"
#include "../../../platform/posix/tool_process_provider.h"

#include <cstring>
#include <iostream>

namespace
{

class Sink final : public product::IDiagnosticSink
{
public:
	void Report( product::Severity, std::string_view source, std::string_view message ) override
	{
		std::cerr << "play_embedded: [" << source << "] " << message << '\n';
	}
};

// Records what would start; delegates to the real spawner with --start.
class RecordingSpawner final : public platform::IProcessSpawner
{
public:
	explicit RecordingSpawner( platform::IProcessSpawner *real ) : m_Real( real ) {}
	platform::SpawnedProcess Spawn(
	    const platform::SpawnRequest &request, std::string &error ) override
	{
		std::cout << "spawn:\n";
		for ( const std::string &argument : request.argv )
			std::cout << argument << '\n';
		if ( m_Real )
			return m_Real->Spawn( request, error );
		return platform::SpawnedProcess{ 1 };
	}
	std::optional<int> Poll( platform::SpawnedProcess process ) override
	{
		return m_Real ? m_Real->Poll( process ) : std::optional<int>( 0 );
	}
	int Wait( platform::SpawnedProcess process ) override
	{
		return m_Real ? m_Real->Wait( process ) : 0;
	}
	void Terminate( platform::SpawnedProcess process, int graceMs ) override
	{
		if ( m_Real )
			m_Real->Terminate( process, graceMs );
	}

private:
	platform::IProcessSpawner *m_Real;
};

template <typename T> bool Add( product::ProviderCatalog &catalog, std::unique_ptr<T> provider )
{
	auto added = catalog.Add( std::move( provider ) );
	if ( !added )
		std::cerr << "play_embedded: " << added.Error().Describe() << '\n';
	return static_cast<bool>( added );
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc < 3 )
	{
		std::cerr << "usage: play_embedded <root> <profile> [map] [--start]\n";
		return 2;
	}
	const std::filesystem::path root = std::filesystem::absolute( argv[1] ).lexically_normal();
	kiln::PlayRequest request;
	request.profile = argv[2];
	request.displaySession = "none";
	bool start = false;
	for ( int i = 3; i < argc; ++i )
	{
		if ( std::strcmp( argv[i], "--start" ) == 0 )
			start = true;
		else
			request.map = argv[i];
	}

	auto processes = platform::CreatePosixToolProcessProvider();
	product::ProviderCatalog catalog;
	if ( !Add( catalog, product::CreateLinuxGccToolchain( *processes ) ) ||
	     !Add( catalog, product::CreateWafEngineStage() ) ||
	     !Add( catalog, product::CreateLinuxDirPackager() ) ||
	     !Add( catalog, product::CreateUserDisplaySession() ) ||
	     !Add( catalog, product::CreateHeadlessDisplaySession() ) ||
	     !Add( catalog, product::CreateSingleRunProvider() ) )
		return 1;

	const auto environment = platform::CreatePosixProcessEnvironment( argc, argv );
	const kiln::SessionConfig config =
	    kiln::DefaultSessionConfig( root, "linux-x86_64", *environment );
	jobsystem::DeterministicExecutor executor;
	Sink sink;
	kiln::Session session( catalog, *processes, executor, sink, config );

	auto real = start ? platform::CreatePosixProcessSpawner() : nullptr;
	RecordingSpawner spawner( real.get() );
	auto status = session.Launch( request, spawner );
	if ( !status )
	{
		std::cerr << "play_embedded: " << status.Error().Describe() << '\n';
		return 1;
	}
	return status.Value();
}
