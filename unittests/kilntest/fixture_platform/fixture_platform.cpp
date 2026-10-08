//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.fixture-platform providers (RFC 0027 L0).
//
//=============================================================================//

#include "fixture_platform.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace fixture
{

namespace fs = std::filesystem;
using foundation::json::Value;
using product::Artifact;
using product::ProviderError;

std::string ReadBytes( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

bool WriteBytes( const fs::path &path, const std::string &bytes )
{
	std::error_code ec;
	fs::create_directories( path.parent_path(), ec );
	std::ofstream stream( path, std::ios::binary | std::ios::trunc );
	return static_cast<bool>(
	    stream.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) ) );
}

bool PublishDirectory( const fs::path &staging, const fs::path &output )
{
	std::error_code ec;
	const fs::path previous = output.string() + ".previous";
	fs::remove_all( previous, ec );
	if ( fs::exists( output, ec ) )
	{
		fs::rename( output, previous, ec );
		if ( ec )
			return false;
	}
	fs::rename( staging, output, ec );
	if ( ec )
	{
		std::error_code restore;
		fs::rename( previous, output, restore );
		return false;
	}
	fs::remove_all( previous, ec );
	return true;
}

std::vector<std::uint8_t> EncodeRgb332(
    const std::vector<std::uint8_t> &rgba, int width, int height )
{
	std::vector<std::uint8_t> texels( static_cast<size_t>( width * height ) );
	for ( size_t i = 0; i < texels.size(); ++i )
	{
		const std::uint8_t r = rgba[i * 4], g = rgba[i * 4 + 1], b = rgba[i * 4 + 2];
		texels[i] = static_cast<std::uint8_t>( ( r & 0xE0 ) | ( ( g & 0xE0 ) >> 3 ) | ( b >> 6 ) );
	}
	return texels;
}

std::vector<std::uint8_t> DecodeRgb332(
    const std::vector<std::uint8_t> &texels, int width, int height )
{
	std::vector<std::uint8_t> rgba( static_cast<size_t>( width * height * 4 ) );
	for ( size_t i = 0; i < static_cast<size_t>( width * height ); ++i )
	{
		const std::uint8_t t = texels[i];
		rgba[i * 4] = static_cast<std::uint8_t>( t & 0xE0 );
		rgba[i * 4 + 1] = static_cast<std::uint8_t>( ( t << 3 ) & 0xE0 );
		rgba[i * 4 + 2] = static_cast<std::uint8_t>( ( t << 6 ) & 0xC0 );
		rgba[i * 4 + 3] = 255;
	}
	return rgba;
}

namespace
{
constexpr int kTileOrder[4] = { 3, 0, 2, 1 }; // stored slot s holds texel kTileOrder[s]
int Padded( int extent )
{
	return ( extent + 1 ) & ~1;
}
} // namespace

std::vector<std::uint8_t> ApplyLayout(
    const std::vector<std::uint8_t> &texels, int width, int height )
{
	const int pw = Padded( width ), ph = Padded( height );
	std::vector<std::uint8_t> tiled( static_cast<size_t>( pw * ph ), 0 );
	size_t out = 0;
	for ( int ty = 0; ty < ph; ty += 2 )
	{
		for ( int tx = 0; tx < pw; tx += 2 )
		{
			for ( int slot = 0; slot < 4; ++slot )
			{
				const int x = tx + ( kTileOrder[slot] & 1 ), y = ty + ( kTileOrder[slot] >> 1 );
				tiled[out++] =
				    ( x < width && y < height ) ? texels[static_cast<size_t>( y * width + x )] : 0;
			}
		}
	}
	return tiled;
}

std::vector<std::uint8_t> RemoveLayout(
    const std::vector<std::uint8_t> &tiled, int width, int height )
{
	const int pw = Padded( width ), ph = Padded( height );
	std::vector<std::uint8_t> texels( static_cast<size_t>( width * height ) );
	size_t in = 0;
	for ( int ty = 0; ty < ph; ty += 2 )
	{
		for ( int tx = 0; tx < pw; tx += 2 )
		{
			for ( int slot = 0; slot < 4; ++slot, ++in )
			{
				const int x = tx + ( kTileOrder[slot] & 1 ), y = ty + ( kTileOrder[slot] >> 1 );
				if ( x < width && y < height )
					texels[static_cast<size_t>( y * width + x )] = tiled[in];
			}
		}
	}
	return texels;
}

namespace
{

class Cancel final : public platform::IToolProcessCancellation
{
public:
	explicit Cancel( const product::ICancellation *cancel ) : m_Cancel( cancel ) {}
	bool IsCancellationRequested() const noexcept override
	{
		return m_Cancel && m_Cancel->IsCancelled();
	}

private:
	const product::ICancellation *m_Cancel;
};

platform::ToolProcessResult RunProcess( platform::IToolProcessProvider &processes,
    std::vector<std::string> argv, const fs::path &directory,
    std::vector<platform::ToolProcessEnvironmentOverride> environment = {},
    const product::ICancellation *cancel = nullptr )
{
	platform::ToolProcessRequest request;
	request.argv = std::move( argv );
	request.workingDirectory = directory.string();
	request.environment = std::move( environment );
	request.executionTimeout = std::chrono::seconds( 120 );
	request.cancellationTimeout = std::chrono::seconds( 2 );
	Cancel adapter( cancel );
	request.cancellation = &adapter;
	return processes.Run( request );
}

std::string FirstLine( const std::string &text )
{
	return text.substr( 0, text.find( '\n' ) );
}

bool Cancelled( const product::ICancellation *cancel )
{
	return cancel && cancel->IsCancelled();
}

ProviderError Cancelled()
{
	return ProviderError{ std::string( product::kCancelled ), "" };
}

//-----------------------------------------------------------------------------

class HostToolchain final : public product::ITargetToolchain
{
public:
	explicit HostToolchain( platform::IToolProcessProvider &processes ) : m_Processes( processes )
	{
	}
	std::string_view Name() const noexcept override { return "fixture-host"; }

	foundation::Expected<product::ToolchainEnvironment, ProviderError> Prepare(
	    const product::ToolchainRequest &request ) override
	{
		if ( Cancelled( request.cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		const Value *toolchain =
		    request.profile ? request.profile->document.Find( "toolchain" ) : nullptr;
		const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
		const std::string *cxx = toolchain ? toolchain->FindString( "cxx" ) : nullptr;
		if ( !version || !cxx )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-pin", "toolchain.version and toolchain.cxx" } );
		const auto queried = RunProcess( m_Processes, { *cxx, "--version" }, request.sourceRoot );
		if ( !queried.Succeeded() )
			return foundation::MakeUnexpected( ProviderError{ "missing-compiler", *cxx } );
		const std::string line = FirstLine( queried.stdoutData );
		if ( line.find( *version ) == std::string::npos )
			return foundation::MakeUnexpected( ProviderError{ "pin-mismatch", line } );
		const auto machine =
		    RunProcess( m_Processes, { *cxx, "-dumpmachine" }, request.sourceRoot );
		product::ToolchainEnvironment environment;
		environment.identity.provider = std::string( Name() );
		environment.identity.facts["cxx"] = *cxx;
		environment.identity.facts["cxx.version"] = line;
		environment.identity.facts["target"] = FirstLine( machine.stdoutData );
		environment.identity.facts["sdk"] = "host";
		environment.environment.push_back( { "CXX", *cxx } );
		return environment;
	}

	void CheckHost( const product::ResolvedProfile *, product::DoctorReport &report ) override
	{
		report.Add( "fixture-host: c++",
		    RunProcess( m_Processes, { "c++", "--version" }, "/" ).Succeeded(), "" );
	}

private:
	platform::IToolProcessProvider &m_Processes;
};

//-----------------------------------------------------------------------------

constexpr const char *kProgram = R"(#include <cstdio>
#include <cstdlib>
#include <cstring>
int main( int argc, char **argv )
{
	int status = 0;
	for ( int i = 1; i < argc; ++i )
	{
		std::printf( "arg %s\n", argv[i] );
		if ( std::strncmp( argv[i], "--exit=", 7 ) == 0 )
			status = std::atoi( argv[i] + 7 );
	}
	const char *display = std::getenv( "FIXTURE_DISPLAY" );
	std::printf( "display %s\n", display ? display : "none" );
	return status;
}
)";

// A stage whose output is kept in the tree with its input digest, so an
// unchanged second run publishes the existing artifact and does no work.
class CachedStage : public product::IProductStage
{
protected:
	// The digest file inside the published artifact directory.
	static std::optional<Artifact> Existing( product::StageInputs &inputs, const std::string &name,
	    const std::string &type, const std::string &digest, const char *digestFile )
	{
		const fs::path path = inputs.TreeRoot() / "artifacts" / name;
		if ( ReadBytes( path / digestFile ) != digest )
			return std::nullopt;
		Artifact artifact;
		artifact.name = name;
		artifact.type = type;
		artifact.path = path;
		artifact.digest = digest;
		return artifact;
	}
};

class CompileStage final : public CachedStage
{
public:
	std::string_view Name() const noexcept override { return "fixture-compile"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return {
		    product::StageRole::kEngine, {}, { "engine-install" }, product::Determinism::kExact };
	}
	foundation::Expected<product::StageResult, ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		if ( Cancelled( inputs.Cancel() ) )
			return foundation::MakeUnexpected( Cancelled() );
		const std::string cxx =
		    inputs.Toolchain() ? inputs.Toolchain()->identity.facts.at( "cxx" ) : "c++";
		const std::string digest =
		    product::HashHex( std::string( kProgram ) + inputs.Toolchain()->identity.Hex() );
		product::StageResult result;
		if ( auto existing = Existing(
		         inputs, "engine-install", "install", digest, ".fixture-compile.digest" ) )
		{
			outputs.Publish( *existing );
			result.upToDate = true;
			result.summary = "up to date";
			return result;
		}
		const fs::path install = outputs.StagingDirectory() / "install";
		WriteBytes( outputs.StagingDirectory() / "fixture_app.cpp", kProgram );
		fs::create_directories( install );
		const auto compiled = RunProcess( *inputs.Processes(),
		    { cxx, "-O0", "-o", ( install / "fixture_app" ).string(), "fixture_app.cpp" },
		    outputs.StagingDirectory(), inputs.Toolchain()->environment, inputs.Cancel() );
		if ( compiled.completion == platform::ToolProcessCompletion::kCanceled )
			return foundation::MakeUnexpected( Cancelled() );
		if ( !compiled.Succeeded() )
			return foundation::MakeUnexpected(
			    ProviderError{ "compile-failed", compiled.stderrData } );
		WriteBytes( install / ".fixture-compile.digest", digest );
		Artifact artifact;
		artifact.name = "engine-install";
		artifact.type = "install";
		artifact.path = install;
		artifact.digest = digest;
		outputs.Publish( artifact );
		result.workItems = 1;
		result.summary = "compiled fixture_app";
		return result;
	}
};

class ContentStage final : public CachedStage
{
public:
	std::string_view Name() const noexcept override { return "fixture-content"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return {
		    product::StageRole::kContent, {}, { "content-package" }, product::Determinism::kExact };
	}
	foundation::Expected<product::StageResult, ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		if ( Cancelled( inputs.Cancel() ) )
			return foundation::MakeUnexpected( Cancelled() );
		constexpr int width = 5, height = 3; // odd sizes exercise the layout's padding
		std::vector<std::uint8_t> rgba;
		for ( int y = 0; y < height; ++y )
		{
			for ( int x = 0; x < width; ++x )
			{
				rgba.insert( rgba.end(),
				    { static_cast<std::uint8_t>( x * 60 ), static_cast<std::uint8_t>( y * 120 ),
				        static_cast<std::uint8_t>( ( x + y ) * 40 ), 255 } );
			}
		}
		const auto texels = EncodeRgb332( rgba, width, height );
		const auto tiled = ApplyLayout( texels, width, height );
		if ( RemoveLayout( tiled, width, height ) != texels ||
		     EncodeRgb332( DecodeRgb332( texels, width, height ), width, height ) != texels )
			return foundation::MakeUnexpected(
			    ProviderError{ "codec", "the fixture format does not round-trip" } );
		const std::string bytes( tiled.begin(), tiled.end() );
		const std::string digest = product::HashHex( bytes );
		product::StageResult result;
		if ( auto existing = Existing(
		         inputs, "content-package", "directory", digest, ".fixture-content.digest" ) )
		{
			outputs.Publish( *existing );
			result.upToDate = true;
			result.summary = "up to date";
			return result;
		}
		const fs::path directory = outputs.StagingDirectory() / "content";
		WriteBytes( directory / "texture.rgb332", bytes );
		WriteBytes( directory / ".fixture-content.digest", digest );
		outputs.Publish(
		    Artifact{ "content-package", "directory", directory, digest, Value::Object() } );
		result.workItems = 1;
		result.summary = "1 texture (rgb332, fixture layout)";
		return result;
	}
};

class SymbolStage final : public CachedStage
{
public:
	std::string_view Name() const noexcept override { return "fixture-symbols"; }
	product::StageDescriptor Describe( const product::ResolvedProfile & ) const override
	{
		return { product::StageRole::kExtra, { "engine-install" }, { "symbol-archive" },
		    product::Determinism::kExact };
	}
	foundation::Expected<product::StageResult, ProviderError> Run(
	    product::StageInputs &inputs, product::StageOutputs &outputs ) override
	{
		if ( Cancelled( inputs.Cancel() ) )
			return foundation::MakeUnexpected( Cancelled() );
		const Artifact *install = inputs.Get( "engine-install" );
		if ( !install )
			return foundation::MakeUnexpected( ProviderError{ "missing-input", "engine-install" } );
		product::StageResult result;
		if ( auto existing = Existing( inputs, "symbol-archive", "directory", install->digest,
		         ".fixture-symbols.digest" ) )
		{
			outputs.Publish( *existing );
			result.upToDate = true;
			result.summary = "up to date";
			return result;
		}
		const fs::path directory = outputs.StagingDirectory() / "symbols";
		WriteBytes( directory / "symbols.txt",
		    "fixture_app " + product::HashHex( ReadBytes( install->path / "fixture_app" ) ) +
		        "\n" );
		WriteBytes( directory / ".fixture-symbols.digest", install->digest );
		outputs.Publish( Artifact{
		    "symbol-archive", "directory", directory, install->digest, Value::Object() } );
		result.workItems = 1;
		result.summary = "1 symbol file";
		return result;
	}
};

//-----------------------------------------------------------------------------

class DirectoryPackager final : public product::IPackager
{
public:
	std::string_view Name() const noexcept override { return "fixture-dir"; }
	foundation::Expected<product::PackageManifest, ProviderError> Package(
	    const product::PackageRequest &request ) override
	{
		if ( Cancelled( request.cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		const fs::path staging = request.output.string() + ".staging";
		std::error_code ec;
		fs::remove_all( staging, ec );
		product::PackageManifest manifest;
		manifest.form = std::string( Name() );
		for ( const product::PackageInput &input : request.inputs )
		{
			if ( input.packagePath.empty() || input.packagePath.find( ".." ) != std::string::npos ||
			     input.packagePath.front() == '/' || !fs::is_regular_file( input.source, ec ) )
			{
				fs::remove_all( staging, ec );
				return foundation::MakeUnexpected(
				    ProviderError{ "invalid-input", input.packagePath } );
			}
			const std::string bytes = ReadBytes( input.source );
			if ( !WriteBytes( staging / input.packagePath, bytes ) )
			{
				fs::remove_all( staging, ec );
				return foundation::MakeUnexpected( ProviderError{ "io", input.packagePath } );
			}
			fs::permissions(
			    staging / input.packagePath, fs::status( input.source, ec ).permissions(), ec );
			manifest.entries.push_back(
			    { input.packagePath, product::HashHex( bytes ), input.role, bytes.size() } );
		}
		std::sort( manifest.entries.begin(), manifest.entries.end(),
		    []( const auto &a, const auto &b )
		    {
			    return a.path < b.path;
		    } );
		for ( size_t i = 1; i < manifest.entries.size(); ++i )
		{
			if ( manifest.entries[i].path == manifest.entries[i - 1].path )
			{
				fs::remove_all( staging, ec );
				return foundation::MakeUnexpected(
				    ProviderError{ "duplicate-path", manifest.entries[i].path } );
			}
		}
		if ( Cancelled( request.cancel ) || !PublishDirectory( staging, request.output ) )
		{
			fs::remove_all( staging, ec );
			return foundation::MakeUnexpected(
			    Cancelled( request.cancel ) ? Cancelled() : ProviderError{ "io", "publish" } );
		}
		return manifest;
	}
};

//-----------------------------------------------------------------------------

bool InsideRoot( const std::string &path )
{
	if ( path.empty() || path.front() == '/' )
		return false;
	std::istringstream stream( path );
	std::string segment;
	while ( std::getline( stream, segment, '/' ) )
	{
		if ( segment == ".." || segment.empty() )
			return false;
	}
	return true;
}

class FakeTransport final : public product::IDeployTransport,
                            product::IInstall,
                            product::IContentSync,
                            product::ILaunch,
                            product::IDeviceFacts
{
public:
	FakeTransport( FakeDevice &device, platform::IToolProcessProvider &processes )
	    : m_Device( device ), m_Processes( processes )
	{
	}
	std::string_view Name() const noexcept override { return "fixture-device"; }
	product::IInstall *Install() noexcept override { return this; }
	product::IContentSync *ContentSync() noexcept override { return this; }
	product::ILaunch *Launch() noexcept override { return this; }
	product::IDeviceFacts *DeviceFacts() noexcept override { return this; }

	foundation::Expected<void, ProviderError> Install( const product::DeviceAddress &,
	    const product::PackageManifest &manifest, const fs::path &package,
	    const product::ICancellation *cancel ) override
	{
		if ( !m_Device.reachable )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kUnavailable ), "device" } );
		if ( Cancelled( cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		if ( m_Device.installedDigest == manifest.Digest() && m_Device.installedPackage == package )
			return {};
		m_Device.installedPackage = package;
		m_Device.installedDigest = manifest.Digest();
		++m_Device.installs;
		return {};
	}

	foundation::Expected<product::SyncResult, ProviderError> Sync( const product::DeviceAddress &,
	    const std::vector<product::SyncEntry> &entries,
	    const product::ICancellation *cancel ) override
	{
		if ( !m_Device.reachable )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kUnavailable ), "device" } );
		if ( Cancelled( cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		// Read and verify everything before touching the device.
		std::map<std::string, std::string> wanted;
		for ( const product::SyncEntry &entry : entries )
		{
			if ( !InsideRoot( entry.path ) )
				return foundation::MakeUnexpected( ProviderError{ "outside-root", entry.path } );
			std::string bytes = ReadBytes( entry.source );
			if ( product::HashHex( bytes ) != entry.hash )
				return foundation::MakeUnexpected( ProviderError{ "hash-mismatch", entry.path } );
			wanted[entry.path] = std::move( bytes );
		}
		product::SyncResult result;
		for ( auto &entry : wanted )
		{
			auto it = m_Device.files.find( entry.first );
			if ( it != m_Device.files.end() &&
			     product::HashHex( it->second ) == product::HashHex( entry.second ) )
			{
				++result.unchanged;
				continue;
			}
			m_Device.files[entry.first] = entry.second;
			++m_Device.writes;
			++result.transferred;
		}
		for ( auto it = m_Device.files.begin(); it != m_Device.files.end(); )
		{
			if ( wanted.count( it->first ) )
			{
				++it;
				continue;
			}
			it = m_Device.files.erase( it );
			++m_Device.deletes;
			++result.removed;
		}
		return result;
	}

	foundation::Expected<product::LaunchResult, ProviderError> Launch(
	    const product::DeviceAddress &, const product::LaunchRequest &request ) override
	{
		if ( !m_Device.reachable )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kUnavailable ), "device" } );
		if ( Cancelled( request.cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		if ( m_Device.installedPackage.empty() )
			return foundation::MakeUnexpected( ProviderError{ "not-installed", "" } );
		std::vector<std::string> argv = { ( m_Device.installedPackage / "fixture_app" ).string() };
		argv.insert( argv.end(), request.arguments.begin(), request.arguments.end() );
		const auto ran = RunProcess(
		    m_Processes, argv, m_Device.installedPackage, request.environment, request.cancel );
		if ( ran.completion != platform::ToolProcessCompletion::kExited )
			return foundation::MakeUnexpected( ProviderError{ "launch-failed", ran.error.detail } );
		++m_Device.launches;
		product::LaunchResult result;
		result.exitCode = ran.exitCode.value_or( -1 );
		std::istringstream lines( ran.stdoutData );
		std::string line;
		while ( std::getline( lines, line ) )
			result.log.push_back( line );
		m_Device.log = result.log;
		return result;
	}

	foundation::Expected<Value, ProviderError> Facts( const product::DeviceAddress & ) override
	{
		if ( !m_Device.reachable )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( product::kUnavailable ), "device" } );
		Value facts = Value::Object();
		Value &formats = facts.Set( "pixel_formats", Value::Array() );
		formats.Push( Value::String( "rgb332" ) );
		facts.Set( "layout", Value::String( "fixture-2x2" ) );
		return facts;
	}

private:
	FakeDevice &m_Device;
	platform::IToolProcessProvider &m_Processes;
};

class HeadlessSession final : public product::IDisplaySession
{
public:
	std::string_view Name() const noexcept override { return "fixture-headless"; }
	bool ClaimsIsolation() const noexcept override { return true; }
	foundation::Expected<product::DisplayEnvironment, ProviderError> Open(
	    const product::DisplayRequest &request ) override
	{
		const product::ICancellation *cancel = request.cancel;
		if ( Cancelled( cancel ) )
			return foundation::MakeUnexpected( Cancelled() );
		m_Open = true;
		product::DisplayEnvironment environment;
		environment.environment = { { "FIXTURE_DISPLAY", std::string( "headless" ) },
		    { "WAYLAND_DISPLAY", std::nullopt }, { "DISPLAY", std::nullopt } };
		environment.isolated = true;
		environment.headless = true;
		return environment;
	}
	void Close() noexcept override { m_Open = false; }
	bool IsOpen() const noexcept override { return m_Open; }

private:
	bool m_Open = false;
};

} // namespace

std::unique_ptr<product::ITargetToolchain> CreateHostToolchain(
    platform::IToolProcessProvider &processes )
{
	return std::make_unique<HostToolchain>( processes );
}
std::unique_ptr<product::IProductStage> CreateCompileStage()
{
	return std::make_unique<CompileStage>();
}
std::unique_ptr<product::IProductStage> CreateContentStage()
{
	return std::make_unique<ContentStage>();
}
std::unique_ptr<product::IProductStage> CreateSymbolStage()
{
	return std::make_unique<SymbolStage>();
}
std::unique_ptr<product::IPackager> CreateDirectoryPackager()
{
	return std::make_unique<DirectoryPackager>();
}
std::unique_ptr<product::IDeployTransport> CreateFakeTransport(
    FakeDevice &device, platform::IToolProcessProvider &processes )
{
	return std::make_unique<FakeTransport>( device, processes );
}
std::unique_ptr<product::IDisplaySession> CreateHeadlessSession()
{
	return std::make_unique<HeadlessSession>();
}

product::ProviderCatalog ComposeFixtureCatalog(
    FakeDevice &device, platform::IToolProcessProvider &processes )
{
	product::ProviderCatalog catalog;
	(void)catalog.Add( CreateHostToolchain( processes ) );
	(void)catalog.Add( CreateCompileStage() );
	(void)catalog.Add( CreateContentStage() );
	(void)catalog.Add( CreateSymbolStage() );
	(void)catalog.Add( CreateDirectoryPackager() );
	(void)catalog.Add( CreateFakeTransport( device, processes ) );
	(void)catalog.Add( CreateHeadlessSession() );
	return catalog;
}

} // namespace fixture
