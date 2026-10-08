//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Deliberately bad product contract providers (RFC 0027 L0).
//
//=============================================================================//

#include "bad_providers.h"

#include <chrono>
#include <cstdlib>

namespace bad
{

namespace fs = std::filesystem;
using foundation::json::Value;
using product::ProviderError;

namespace
{

bool Cancelled( const product::ICancellation *cancel )
{
	return cancel && cancel->IsCancelled();
}

//-----------------------------------------------------------------------------

class BadToolchain final : public product::ITargetToolchain
{
public:
	BadToolchain( ToolchainFault fault, platform::IToolProcessProvider &processes )
	    : m_Fault( fault ), m_Processes( processes ),
	      m_Good( fixture::CreateHostToolchain( processes ) )
	{
	}
	std::string_view Name() const noexcept override { return "bad-toolchain"; }
	foundation::Expected<product::ToolchainEnvironment, ProviderError> Prepare(
	    const product::ToolchainRequest &request ) override
	{
		if ( m_Fault == ToolchainFault::kMissingCompiler )
		{
			if ( Cancelled( request.cancel ) )
				return foundation::MakeUnexpected(
				    ProviderError{ std::string( product::kCancelled ), "" } );
			product::ToolchainEnvironment environment;
			environment.identity.provider = std::string( Name() );
			environment.identity.facts = {
			    { "cxx.version", "1" }, { "sdk", "host" }, { "target", "x" } };
			environment.environment.push_back( { "CXX", std::string( "/nonexistent/c++" ) } );
			return environment;
		}
		auto prepared = m_Good->Prepare( request );
		if ( !prepared )
			return prepared;
		prepared.Value().identity.provider = std::string( Name() );
		switch ( m_Fault )
		{
		case ToolchainFault::kIdentityWithoutSdk:
			prepared.Value().identity.facts.erase( "sdk" );
			break;
		case ToolchainFault::kUnpinnedDownload:
		{
			platform::ToolProcessRequest fetch;
			fetch.argv = { "curl", "--version" }; // stands in for an unpinned archive fetch
			fetch.workingDirectory = request.sourceRoot.string();
			fetch.executionTimeout = std::chrono::seconds( 10 );
			fetch.cancellationTimeout = std::chrono::seconds( 1 );
			(void)m_Processes.Run( fetch );
			break;
		}
		case ToolchainFault::kWritesSourceTree:
			fixture::WriteBytes( request.sourceRoot / "toolchain.stamp", "written by a toolchain" );
			break;
		case ToolchainFault::kMissingCompiler:
			break;
		}
		return prepared;
	}
	void CheckHost(
	    const product::ResolvedProfile *profile, product::DoctorReport &report ) override
	{
		m_Good->CheckHost( profile, report );
	}

private:
	ToolchainFault m_Fault;
	platform::IToolProcessProvider &m_Processes;
	std::unique_ptr<product::ITargetToolchain> m_Good;
};

//-----------------------------------------------------------------------------

class FakeRecipeBuilder final : public product::IRecipeBuilder
{
public:
	explicit FakeRecipeBuilder( RecipeFault fault ) : m_Fault( fault ) {}
	std::string_view Name() const noexcept override
	{
		return m_Fault == RecipeFault::kNone ? "fake-recipe" : "bad-recipe";
	}
	foundation::Expected<product::RecipeResult, ProviderError> Build(
	    const product::RecipeRequest &request ) override
	{
		if ( Cancelled( request.cancel ) )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kCancelled ), "" } );
		product::Identity identity = request.toolchain->identity;
		if ( m_Fault == RecipeFault::kIgnoresToolchain )
			identity = product::Identity{};
		const std::string key = product::RecipeKey( request.recipe, identity );
		const fs::path prefix = request.prefixRoot / ( request.recipe.name + "-" + key );
		product::RecipeResult result{
		    prefix, product::RecipeKey( request.recipe, request.toolchain->identity ), false };
		std::error_code ec;
		if ( fs::exists( prefix / "lib" / "recipe.a", ec ) )
			return result;
		const bool fail = request.recipe.definition.Find( "fail" ) != nullptr;
		if ( m_Fault == RecipeFault::kPartialPublish )
		{
			fixture::WriteBytes( prefix / "include" / "half.h", "half" );
			if ( fail )
				return foundation::MakeUnexpected(
				    ProviderError{ "recipe-failed", request.recipe.name } );
			fixture::WriteBytes( prefix / "lib" / "recipe.a", request.recipe.pin );
			result.built = true;
			return result;
		}
		const fs::path staging =
		    request.prefixRoot / ( ".staging-" + request.recipe.name + "-" + key );
		fs::remove_all( staging, ec );
		fixture::WriteBytes( staging / "include" / "recipe.h", request.recipe.name );
		if ( fail )
		{
			fs::remove_all( staging, ec );
			return foundation::MakeUnexpected(
			    ProviderError{ "recipe-failed", request.recipe.name } );
		}
		fixture::WriteBytes( staging / "lib" / "recipe.a", request.recipe.pin );
		if ( !fixture::PublishDirectory( staging, prefix ) )
			return foundation::MakeUnexpected( ProviderError{ "io", "publish" } );
		result.built = true;
		return result;
	}

private:
	RecipeFault m_Fault;
};

//-----------------------------------------------------------------------------

class BadStage final : public product::IProductStage
{
public:
	explicit BadStage( StageFault fault ) : m_Fault( fault ) {}
	std::string_view Name() const noexcept override { return "bad-stage"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		product::StageDescriptor descriptor{
		    product::StageRole::kContent, {}, { "bad-output" }, product::Determinism::kExact };
		if ( m_Fault == StageFault::kPartialPublish )
			descriptor.produces.push_back( "bad-output-index" );
		return descriptor;
	}
	foundation::Expected<product::StageResult, ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		if ( m_Fault != StageFault::kIgnoresCancel && Cancelled( inputs.Cancel() ) )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kCancelled ), "" } );
		if ( m_Fault == StageFault::kReadsUndeclared )
			(void)inputs.Get( "engine-install" );
		const fs::path path = outputs.StagingDirectory() / "out.bin";
		fixture::WriteBytes( path, "output" );
		outputs.Publish( product::Artifact{
		    "bad-output", "file", path, product::HashHex( "output" ), Value::Object() } );
		if ( m_Fault == StageFault::kProducesUndeclared )
			outputs.Publish( product::Artifact{
			    "surprise", "file", path, product::HashHex( "output" ), Value::Object() } );
		return product::StageResult{};
	}

private:
	StageFault m_Fault;
};

//-----------------------------------------------------------------------------

class BadPackager final : public product::IPackager
{
public:
	explicit BadPackager( PackagerFault fault )
	    : m_Fault( fault ), m_Good( fixture::CreateDirectoryPackager() )
	{
	}
	std::string_view Name() const noexcept override { return "bad-packager"; }
	foundation::Expected<product::PackageManifest, ProviderError> Package(
	    const product::PackageRequest &request ) override
	{
		product::PackageRequest changed = request;
		switch ( m_Fault )
		{
		case PackagerFault::kOmitsModule:
			std::erase_if( changed.inputs,
			    []( const product::PackageInput &input )
			    {
				    return input.role == "module";
			    } );
			return m_Good->Package( changed );
		case PackagerFault::kPartialPublish:
		{
			product::PackageManifest manifest;
			manifest.form = std::string( Name() );
			for ( const auto &input : request.inputs )
			{
				std::error_code ec;
				if ( !fs::is_regular_file( input.source, ec ) )
					return foundation::MakeUnexpected(
					    ProviderError{ "invalid-input", input.packagePath } );
				const std::string bytes = fixture::ReadBytes( input.source );
				fixture::WriteBytes( request.output / ( input.packagePath + ".new" ), bytes );
				manifest.entries.push_back(
				    { input.packagePath, product::HashHex( bytes ), input.role, bytes.size() } );
			}
			return m_Good->Package( request );
		}
		default:
			break;
		}
		auto manifest = m_Good->Package( request );
		if ( !manifest )
			return manifest;
		switch ( m_Fault )
		{
		case PackagerFault::kUndeclaredFile:
			fixture::WriteBytes( request.output / "debug.log", "left over" );
			manifest.Value().entries.push_back(
			    { "debug.log", product::HashHex( "left over" ), "content", 9 } );
			break;
		case PackagerFault::kEmbedsCredential:
			for ( const auto &credential : request.credentials )
				fixture::WriteBytes(
				    request.output / "signing.cfg", credential.first + "=" + credential.second );
			break;
		case PackagerFault::kUnstableManifest:
			manifest.Value().entries.push_back( { "build-time",
			    std::to_string( std::chrono::steady_clock::now().time_since_epoch().count() ),
			    "content", 0 } );
			break;
		default:
			break;
		}
		return manifest;
	}

private:
	PackagerFault m_Fault;
	std::unique_ptr<product::IPackager> m_Good;
};

//-----------------------------------------------------------------------------

class BadTransport final : public product::IDeployTransport,
                           product::IInstall,
                           product::IContentSync,
                           product::ILaunch
{
public:
	BadTransport( TransportFault fault, fixture::FakeDevice &device,
	    platform::IToolProcessProvider &processes )
	    : m_Fault( fault ), m_Device( device ),
	      m_Good( fixture::CreateFakeTransport( device, processes ) )
	{
	}
	std::string_view Name() const noexcept override { return "bad-transport"; }
	product::IInstall *Install() noexcept override { return this; }
	product::IContentSync *ContentSync() noexcept override { return this; }
	product::ILaunch *Launch() noexcept override { return this; }

	foundation::Expected<void, ProviderError> Install( const product::DeviceAddress &address,
	    const product::PackageManifest &manifest, const fs::path &package,
	    const product::ICancellation *cancel ) override
	{
		if ( m_Fault == TransportFault::kClaimedNoEffect )
			return {};
		return m_Good->Install()->Install( address, manifest, package, cancel );
	}

	foundation::Expected<product::SyncResult, ProviderError> Sync(
	    const product::DeviceAddress &address, const std::vector<product::SyncEntry> &entries,
	    const product::ICancellation *cancel ) override
	{
		switch ( m_Fault )
		{
		case TransportFault::kSuccessAfterPartial:
		{
			product::SyncResult result;
			for ( const auto &entry : entries )
			{
				const std::string bytes = fixture::ReadBytes( entry.source );
				if ( product::HashHex( bytes ) != entry.hash )
					return result; // stops halfway and reports success
				if ( m_Device.files[entry.path] != bytes )
				{
					m_Device.files[entry.path] = bytes;
					++m_Device.writes;
					++result.transferred;
				}
			}
			return m_Good->ContentSync()->Sync( address, entries, cancel );
		}
		case TransportFault::kDeleteOutsideRoot:
			for ( const auto &entry : entries )
			{
				if ( entry.path.find( ".." ) != std::string::npos )
					m_Device.outside[entry.path] = "deleted";
			}
			break;
		case TransportFault::kIgnoresCancel:
			return m_Good->ContentSync()->Sync( address, entries, nullptr );
		case TransportFault::kResendsUnchanged:
		{
			auto result = m_Good->ContentSync()->Sync( address, entries, cancel );
			if ( result )
			{
				for ( const auto &entry : entries )
				{
					m_Device.files[entry.path] = fixture::ReadBytes( entry.source );
					++m_Device.writes;
				}
				result.Value().transferred = entries.size();
			}
			return result;
		}
		default:
			break;
		}
		return m_Good->ContentSync()->Sync( address, entries, cancel );
	}

	foundation::Expected<product::LaunchResult, ProviderError> Launch(
	    const product::DeviceAddress &address, const product::LaunchRequest &request ) override
	{
		auto launched = m_Good->Launch()->Launch( address, request );
		if ( launched && m_Fault == TransportFault::kCredentialInLog )
		{
			for ( const auto &credential : address.credentials )
				launched.Value().log.push_back(
				    "connecting with " + credential.first + " " + credential.second );
		}
		return launched;
	}

private:
	TransportFault m_Fault;
	fixture::FakeDevice &m_Device;
	std::unique_ptr<product::IDeployTransport> m_Good;
};

//-----------------------------------------------------------------------------

class BadDisplay final : public product::IDisplaySession
{
public:
	explicit BadDisplay( DisplayFault fault ) : m_Fault( fault ) {}
	std::string_view Name() const noexcept override { return "bad-display"; }
	bool ClaimsIsolation() const noexcept override { return true; }
	foundation::Expected<product::DisplayEnvironment, ProviderError> Open(
	    const product::DisplayRequest &request ) override
	{
		const product::ICancellation *cancel = request.cancel;
		if ( m_Fault != DisplayFault::kIgnoresCancel && Cancelled( cancel ) )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kCancelled ), "" } );
		m_Open = true;
		product::DisplayEnvironment environment;
		environment.isolated = true;
		environment.environment = { { "WAYLAND_DISPLAY", std::string( "wayland-private-1" ) },
		    { "DISPLAY", std::nullopt } };
		if ( m_Fault == DisplayFault::kLeaksUserDisplay )
		{
			const char *user = std::getenv( "WAYLAND_DISPLAY" );
			environment.environment[0].value = user ? user : "";
		}
		if ( m_Fault == DisplayFault::kMutatesCaller )
			setenv( "WAYLAND_DISPLAY", "wayland-private-1", 1 );
		return environment;
	}
	void Close() noexcept override
	{
		if ( m_Fault != DisplayFault::kStaysOpen )
			m_Open = false;
	}
	bool IsOpen() const noexcept override { return m_Open; }

private:
	DisplayFault m_Fault;
	bool m_Open = false;
};

class BadRun final : public product::IRunProvider
{
public:
	explicit BadRun( RunFault fault ) : m_Fault( fault ) {}
	std::string_view Name() const noexcept override { return "bad-run"; }
	foundation::Expected<int, ProviderError> Run( const product::RunRequest &request ) override
	{
		if ( request.launches.size() != 1 || !request.spawner )
			return foundation::MakeUnexpected( ProviderError{ "invalid-request", "" } );
		platform::SpawnRequest spawn;
		spawn.argv = request.display.commandPrefix;
		spawn.argv.insert(
		    spawn.argv.end(), request.launches[0].argv.begin(), request.launches[0].argv.end() );
		spawn.workingDirectory = request.launches[0].workingDirectory.string();
		spawn.environment = request.display.environment;
		spawn.environment.insert( spawn.environment.end(), request.launches[0].environment.begin(),
		    request.launches[0].environment.end() );
		std::string error;
		const platform::SpawnedProcess process = request.spawner->Spawn( spawn, error );
		if ( m_Fault == RunFault::kReturnsBeforeExit )
			return 0;                            // leaves the program running
		return request.spawner->Wait( process ); // never looks at cancellation
	}

private:
	RunFault m_Fault;
};

} // namespace

std::unique_ptr<product::IRunProvider> RunProvider( RunFault fault )
{
	return std::make_unique<BadRun>( fault );
}

std::unique_ptr<product::ITargetToolchain> Toolchain(
    ToolchainFault fault, platform::IToolProcessProvider &processes )
{
	return std::make_unique<BadToolchain>( fault, processes );
}
std::unique_ptr<product::IRecipeBuilder> RecipeBuilder( RecipeFault fault )
{
	return std::make_unique<FakeRecipeBuilder>( fault );
}
std::unique_ptr<product::IProductStage> Stage( StageFault fault )
{
	return std::make_unique<BadStage>( fault );
}
std::unique_ptr<product::IPackager> Packager( PackagerFault fault )
{
	return std::make_unique<BadPackager>( fault );
}
std::unique_ptr<product::IDeployTransport> Transport(
    TransportFault fault, fixture::FakeDevice &device, platform::IToolProcessProvider &processes )
{
	return std::make_unique<BadTransport>( fault, device, processes );
}
std::unique_ptr<product::IDisplaySession> Display( DisplayFault fault )
{
	return std::make_unique<BadDisplay>( fault );
}

} // namespace bad
