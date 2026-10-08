//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.n3ds: the n3ds-devkitarm cross toolchain
//			(RFC 0027, RFC 0026). See public/product/toolchain_n3ds.h.
//
//=============================================================================//

#include "product/toolchain_n3ds.h"

#include <chrono>
#include <system_error>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kFamily = "devkitarm";
constexpr const char *kDefaultCc = "arm-none-eabi-gcc";
constexpr const char *kDefaultCxx = "arm-none-eabi-g++";

std::string FirstLine( const std::string &text )
{
	return text.substr( 0, text.find( '\n' ) );
}

// A version pin matches when it is a whole token of the --version line
// ("arm-none-eabi-gcc (devkitARM) 16.1.0").
bool VersionMatches( const std::string &line, const std::string &pin )
{
	for ( size_t at = line.find( pin ); at != std::string::npos; at = line.find( pin, at + 1 ) )
	{
		const size_t end = at + pin.size();
		const bool startOk = at == 0 || line[at - 1] == ' ' || line[at - 1] == '(';
		const bool endOk = end == line.size() || line[end] == ' ' || line[end] == ')';
		if ( startOk && endOk )
			return true;
	}
	return false;
}

// The digest part of "name@sha256:<hex>", or empty when the image is not
// pinned by digest (a tag alone can move, so it is refused).
std::string ImageDigest( const std::string &image )
{
	const size_t at = image.find( "@sha256:" );
	if ( at == std::string::npos )
		return {};
	const std::string hex = image.substr( at + 8 );
	if ( hex.size() != 64 || hex.find_first_not_of( "0123456789abcdef" ) != std::string::npos )
		return {};
	return hex;
}

ProviderError Error( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

class N3dsToolchain final : public ITargetToolchain
{
public:
	explicit N3dsToolchain( platform::IToolProcessProvider &processes ) : m_Processes( processes )
	{
	}

	std::string_view Name() const noexcept override { return kN3dsToolchain; }

	foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) override
	{
		if ( Cancelled( request.cancel ) )
			return foundation::MakeUnexpected( Error( std::string( kCancelled ), "before prepare" ) );
		if ( !request.profile )
			return foundation::MakeUnexpected( Error( "invalid-request", "no profile" ) );
		const Value &document = request.profile->document;
		const Value *toolchain = document.Find( "toolchain" );
		const std::string *family = toolchain ? toolchain->FindString( "family" ) : nullptr;
		const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
		if ( !family || !version || version->empty() )
			return foundation::MakeUnexpected( Error(
			    "missing-pin", "toolchain.family and toolchain.version are required" ) );
		if ( *family != kFamily )
			return foundation::MakeUnexpected( Error( "toolchain-mismatch",
			    std::string( kN3dsToolchain ) + " builds toolchain.family \"" + kFamily +
			        "\", the profile pins \"" + *family + "\"" ) );
		const Value *dependencies = document.Find( "dependencies" );
		const std::string *image =
		    dependencies ? dependencies->FindString( "container_image" ) : nullptr;
		const std::string digest = image ? ImageDigest( *image ) : std::string();
		if ( digest.empty() )
			return foundation::MakeUnexpected( Error( "missing-pin",
			    "dependencies.container_image must name the devkitARM image by "
			    "\"name@sha256:<digest>\"" ) );

		const fs::path home = request.dependencyRoot / std::string( kN3dsToolchain );
		std::error_code ec;
		fs::create_directories( home, ec );
		if ( ec )
			return foundation::MakeUnexpected(
			    Error( "io", "cannot create " + home.string() + ": " + ec.message() ) );

		// devkitPro, keyed by the image digest.
		const fs::path devkitpro = home / ( "devkitpro-" + digest.substr( 0, 12 ) );
		if ( auto extracted = ExtractDevkitPro( *image, devkitpro, request.cancel ); !extracted )
			return foundation::MakeUnexpected( extracted.Error() );
		const fs::path devkitarm = devkitpro / "devkitARM";
		const fs::path bin = devkitarm / "bin";
		const std::string cc = ( bin / Pick( toolchain, "cc", kDefaultCc ) ).string();
		const std::string cxx = ( bin / Pick( toolchain, "cxx", kDefaultCxx ) ).string();
		auto cxxVersion = Query( { cxx, "--version" } );
		if ( !cxxVersion )
			return foundation::MakeUnexpected(
			    Error( "missing-compiler", cxx + ": " + cxxVersion.Error() ) );
		const std::string line = FirstLine( cxxVersion.Value() );
		if ( !VersionMatches( line, *version ) )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    "toolchain.version pins " + *version + "; the image's compiler is \"" + line +
			        "\"" ) );

		ToolchainEnvironment environment;
		Identity &identity = environment.identity;
		identity.provider = std::string( kN3dsToolchain );
		identity.facts["cc"] = cc;
		identity.facts["cxx"] = cxx;
		identity.facts["cxx.version"] = line;
		identity.facts["pin.version"] = *version;
		identity.facts["sdk"] = "devkitpro@sha256:" + digest;
		identity.facts["target"] = "arm-none-eabi";

		const std::vector<platform::ToolProcessEnvironmentOverride> sdk = {
		    { "DEVKITPRO", devkitpro.string() }, { "DEVKITARM", devkitarm.string() } };

		// The pinned source archives, cross-built into prefixes.
		std::string pkgConfigPath;
		const Value *archives = dependencies->Find( "archives" );
		if ( archives && !archives->IsObject() )
			return foundation::MakeUnexpected(
			    Error( "invalid-profile", "dependencies.archives maps names to archives" ) );
		if ( archives )
		{
			for ( const auto &[name, archive] : archives->Members() )
			{
				auto prefix = BuildArchive( request, home, name, archive, identity, sdk );
				if ( !prefix )
					return foundation::MakeUnexpected( prefix.Error() );
				identity.facts["archive." + name] = prefix.Value().filename().string();
				pkgConfigPath += ( prefix.Value() / "lib" / "pkgconfig" ).string() + ":";
			}
		}
		pkgConfigPath += ( devkitpro / "portlibs" / "3ds" / "lib" / "pkgconfig" ).string();

		environment.environment = sdk;
		// devkitPro's .pc files say prefix=/opt/devkitpro; the wrapper relocates.
		environment.environment.push_back(
		    { "PKGCONFIG", ( request.sourceRoot / "tools" / "n3ds" / "pkg-config" ).string() } );
		environment.environment.push_back( { "PKG_CONFIG_LIBDIR", pkgConfigPath } );
		environment.environment.push_back( { "PKG_CONFIG_PATH", std::string() } );
		environment.wafOptions = { "--n3ds" };
		return environment;
	}

	void CheckHost( const ResolvedProfile *profile, DoctorReport &report ) override
	{
		auto docker = Query( { "docker", "--version" } );
		report.Add( std::string( kN3dsToolchain ) + ": docker", docker.HasValue(),
		    docker ? FirstLine( docker.Value() )
		           : "needed once, to extract devkitARM from its pinned image" );
		auto cmake = Query( { "cmake", "--version" } );
		report.Add( std::string( kN3dsToolchain ) + ": cmake", cmake.HasValue(),
		    cmake ? FirstLine( cmake.Value() ) : cmake.Error() );
		const Value *dependencies = profile ? profile->document.Find( "dependencies" ) : nullptr;
		const std::string *image =
		    dependencies ? dependencies->FindString( "container_image" ) : nullptr;
		report.Add( std::string( kN3dsToolchain ) + ": image pinned by digest",
		    image && !ImageDigest( *image ).empty(), image ? *image : "none" );
	}

private:
	static bool Cancelled( const ICancellation *cancel )
	{
		return cancel && cancel->IsCancelled();
	}

	static std::string Pick( const Value *toolchain, const char *key, const char *fallback )
	{
		const std::string *value = toolchain ? toolchain->FindString( key ) : nullptr;
		return value && !value->empty() ? *value : std::string( fallback );
	}

	// devkitPro from the image, through staging and one rename: an
	// interrupted extraction leaves no partial toolchain.
	foundation::Expected<void, ProviderError> ExtractDevkitPro(
	    const std::string &image, const fs::path &devkitpro, const ICancellation *cancel )
	{
		std::error_code ec;
		if ( fs::exists( devkitpro / "devkitARM" / "bin", ec ) )
			return {};
		const fs::path staging = devkitpro.string() + ".staging";
		fs::remove_all( staging, ec );
		auto created = Query( { "docker", "create", image }, cancel, std::chrono::minutes( 30 ) );
		if ( !created )
			return foundation::MakeUnexpected( Error( "unavailable",
			    "extracting devkitARM needs docker and the pinned image " + image + ": " +
			        created.Error() ) );
		const std::string container = FirstLine( created.Value() );
		auto copied = Query( { "docker", "cp", container + ":/opt/devkitpro", staging.string() },
		    cancel, std::chrono::minutes( 30 ) );
		(void)Query( { "docker", "rm", container } );
		if ( !copied )
		{
			fs::remove_all( staging, ec );
			return foundation::MakeUnexpected(
			    Error( "extract-failed", "docker cp: " + copied.Error() ) );
		}
		fs::rename( staging, devkitpro, ec );
		if ( ec )
			return foundation::MakeUnexpected(
			    Error( "io", "cannot publish " + devkitpro.string() + ": " + ec.message() ) );
		return {};
	}

	// One archive (path, sha256, directory, cmake_options) built with CMake
	// and devkitPro's 3DS toolchain file. The prefix's key covers the
	// archive digest, its options and the toolchain identity; the install
	// goes through DESTDIR staging so the published prefix is complete and
	// its .pc files name the final path.
	foundation::Expected<fs::path, ProviderError> BuildArchive( const ToolchainRequest &request,
	    const fs::path &home, const std::string &name, const Value &archive,
	    const Identity &toolchain, const std::vector<platform::ToolProcessEnvironmentOverride> &sdk )
	{
		const std::string field = "dependencies.archives." + name;
		const std::string *path = archive.FindString( "path" );
		const std::string *sha256 = archive.FindString( "sha256" );
		const std::string *directory = archive.FindString( "directory" );
		const Value *options = archive.Find( "cmake_options" );
		if ( !path || !sha256 || sha256->size() != 64 || !directory ||
		     ( options && !options->IsArray() ) )
			return foundation::MakeUnexpected( Error( "missing-pin",
			    field + " needs path, sha256 (64 hex), directory and optional cmake_options" ) );
		std::string optionText;
		std::vector<std::string> cmakeOptions;
		if ( options )
		{
			for ( const Value &option : options->Items() )
			{
				cmakeOptions.push_back( option.Text() );
				optionText += option.Text() + "\n";
			}
		}
		const std::string key =
		    HashHex( *sha256 + "\n" + optionText + "toolchain=" + toolchain.Hex() ).substr( 0, 12 );
		const fs::path prefix = home / ( name + "-" + key );
		std::error_code ec;
		if ( fs::exists( prefix / "lib", ec ) )
			return prefix;

		const fs::path source = request.sourceRoot / *path;
		auto sum = Query( { "sha256sum", source.string() }, request.cancel );
		if ( !sum )
			return foundation::MakeUnexpected(
			    Error( "missing-archive", field + ": " + source.string() + ": " + sum.Error() ) );
		if ( sum.Value().substr( 0, 64 ) != *sha256 )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    field + ": " + source.string() + " is " + sum.Value().substr( 0, 64 ) +
			        ", the profile pins " + *sha256 ) );

		const fs::path work = home / ( name + "-" + key + ".work" );
		fs::remove_all( work, ec );
		fs::create_directories( work / "src", ec );
		if ( ec )
			return foundation::MakeUnexpected( Error( "io", "cannot create " + work.string() ) );
		const auto run = [&]( std::vector<std::string> argv )
		{
			return Query( std::move( argv ), request.cancel, std::chrono::hours( 1 ), sdk );
		};
		std::vector<std::string> configure = { "cmake", "-S", ( work / "src" / *directory ).string(),
		    "-B", ( work / "build" ).string(),
		    "-DCMAKE_TOOLCHAIN_FILE=" + ( sdk[0].value.value_or( "" ) + "/cmake/3DS.cmake" ),
		    "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_INSTALL_PREFIX=" + prefix.string() };
		configure.insert( configure.end(), cmakeOptions.begin(), cmakeOptions.end() );
		for ( auto step : { std::vector<std::string>{ "tar", "xzf", source.string(), "-C",
		                        ( work / "src" ).string() },
		          configure,
		          std::vector<std::string>{ "cmake", "--build", ( work / "build" ).string(), "-j16" } } )
		{
			if ( auto ran = run( step ); !ran )
			{
				fs::remove_all( work, ec );
				return foundation::MakeUnexpected(
				    Error( "archive-build-failed", field + ": " + step[0] + ": " + ran.Error() ) );
			}
		}
		std::vector<platform::ToolProcessEnvironmentOverride> install = sdk;
		install.push_back( { "DESTDIR", ( work / "stage" ).string() } );
		if ( auto ran = Query( { "cmake", "--install", ( work / "build" ).string() }, request.cancel,
		         std::chrono::hours( 1 ), install );
		     !ran )
		{
			fs::remove_all( work, ec );
			return foundation::MakeUnexpected(
			    Error( "archive-build-failed", field + ": install: " + ran.Error() ) );
		}
		const fs::path staged = work / "stage" / prefix.relative_path();
		fs::rename( staged, prefix, ec );
		const std::error_code published = ec;
		fs::remove_all( work, ec );
		if ( published )
			return foundation::MakeUnexpected(
			    Error( "io", "cannot publish " + prefix.string() + ": " + published.message() ) );
		return prefix;
	}

	foundation::Expected<std::string, std::string> Query( std::vector<std::string> argv,
	    const ICancellation *cancel = nullptr,
	    std::chrono::milliseconds timeout = std::chrono::seconds( 30 ),
	    std::vector<platform::ToolProcessEnvironmentOverride> environment = {} )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = "/";
		request.environment = std::move( environment );
		request.executionTimeout = timeout;
		request.cancellationTimeout = std::chrono::seconds( 5 );
		CancellationAdapter adapter( cancel );
		request.cancellation = &adapter;
		platform::ToolProcessResult result = m_Processes.Run( request );
		if ( !result.Succeeded() )
		{
			std::string detail = result.error.detail;
			if ( detail.empty() )
				detail = result.exitCode ? "exit status " + std::to_string( *result.exitCode )
				                         : "did not run";
			if ( !result.stderrData.empty() )
				detail += ": " + FirstLine( result.stderrData );
			return foundation::MakeUnexpected( detail );
		}
		return result.stdoutData;
	}

	class CancellationAdapter final : public platform::IToolProcessCancellation
	{
	public:
		explicit CancellationAdapter( const ICancellation *cancel ) : m_Cancel( cancel ) {}
		bool IsCancellationRequested() const noexcept override
		{
			return m_Cancel && m_Cancel->IsCancelled();
		}

	private:
		const ICancellation *m_Cancel;
	};

	platform::IToolProcessProvider &m_Processes;
};

} // namespace

std::unique_ptr<ITargetToolchain> CreateN3dsToolchain( platform::IToolProcessProvider &processes )
{
	return std::make_unique<N3dsToolchain>( processes );
}

} // namespace product
