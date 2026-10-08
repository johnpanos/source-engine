//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.msvc-wine: the windows-msvc-wine target
//			toolchain (RFC 0027). See public/product/toolchain_msvc_wine.h.
//
//=============================================================================//

#include "product/toolchain_msvc_wine.h"

#include <chrono>
#include <fstream>
#include <sstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kProvision = "python3 tools/windows/msvc_wine.py provision";

std::string FirstLine( const std::string &text )
{
	const size_t start = text.find_first_not_of( "\r\n " );
	if ( start == std::string::npos )
		return {};
	const size_t end = text.find_first_of( "\r\n", start );
	return text.substr( start, end == std::string::npos ? std::string::npos : end - start );
}

// cl's banner line ("... Compiler Version 19.44.35229 for x64"); cl prints it
// on stderr and its usage on stdout.
std::string BannerLine( const std::string &text )
{
	std::istringstream lines( text );
	for ( std::string line; std::getline( lines, line ); )
	{
		if ( line.find( "Compiler Version " ) != std::string::npos )
			return FirstLine( line );
	}
	return FirstLine( text );
}

std::string ReadFile( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

// The profile's toolchain.msvc_wine pins that select and verify an install.
struct Pins
{
	std::string version; // toolchain.version: cl's "Version <version>." banner
	std::string commit, toolset, msvcVersion, sdk, manifestSha256;
	std::string runner =
	    "wine"; // toolchain.msvc_wine.runner: the host program that runs Windows code
};

foundation::Expected<Pins, ProviderError> ReadPins( const ResolvedProfile &profile )
{
	const Value *toolchain = profile.document.Find( "toolchain" );
	const std::string *family = toolchain ? toolchain->FindString( "family" ) : nullptr;
	const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
	const Value *pins = toolchain ? toolchain->Find( "msvc_wine" ) : nullptr;
	if ( !family || !version || version->empty() || !pins || !pins->IsObject() )
		return foundation::MakeUnexpected( ProviderError{ "missing-pin",
		    "toolchain.family, toolchain.version and toolchain.msvc_wine are required" } );
	if ( *family != "msvc" )
		return foundation::MakeUnexpected( ProviderError{ "toolchain-mismatch",
		    "windows-msvc-wine builds toolchain.family \"msvc\", the profile pins \"" + *family +
		        "\"" } );
	const Value *manifest = pins->Find( "installer_manifest" );
	const std::string *commit = pins->FindString( "commit" );
	const std::string *toolset = pins->FindString( "msvc_toolset" );
	const std::string *msvcVersion = pins->FindString( "msvc_version" );
	const std::string *sdk = pins->FindString( "sdk_version" );
	const std::string *sha = manifest ? manifest->FindString( "sha256" ) : nullptr;
	if ( !commit || !toolset || !msvcVersion || !sdk || !sha )
		return foundation::MakeUnexpected( ProviderError{ "missing-pin",
		    "toolchain.msvc_wine needs commit, msvc_toolset, msvc_version, sdk_version and "
		    "installer_manifest.sha256" } );
	Pins result{ *version, *commit, *toolset, *msvcVersion, *sdk, *sha };
	if ( const std::string *runner = pins->FindString( "runner" ) )
		result.runner = *runner;
	return result;
}

// The install's stamp (written by msvc_wine.py) records the pins it was made from.
bool StampMatches( const fs::path &install, const Pins &pins )
{
	auto stamp = foundation::json::Parse( ReadFile( install / "msvc-wine-stamp.json" ) );
	if ( !stamp || !stamp.Value().IsObject() )
		return false;
	const Value &value = stamp.Value();
	const Value *manifest = value.Find( "installer_manifest" );
	const std::string *sha = manifest ? manifest->FindString( "sha256" ) : nullptr;
	auto equals = [&]( const char *key, const std::string &expected )
	{
		const std::string *found = value.FindString( key );
		return found && *found == expected;
	};
	return equals( "commit", pins.commit ) && equals( "msvc_toolset", pins.toolset ) &&
	       equals( "msvc_version", pins.msvcVersion ) && equals( "sdk_version", pins.sdk ) && sha &&
	       *sha == pins.manifestSha256;
}

class MsvcWineToolchain final : public ITargetToolchain
{
public:
	explicit MsvcWineToolchain( platform::IToolProcessProvider &processes )
	    : m_Processes( processes )
	{
	}

	std::string_view Name() const noexcept override { return kMsvcWineToolchain; }

	foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) override
	{
		if ( request.cancel && request.cancel->IsCancelled() )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( kCancelled ), "before prepare" } );
		if ( !request.profile )
			return foundation::MakeUnexpected( ProviderError{ "invalid-request", "no profile" } );
		auto pins = ReadPins( *request.profile );
		if ( !pins )
			return foundation::MakeUnexpected( pins.Error() );
		const fs::path install = Install( request.dependencyRoot, pins.Value() );
		const fs::path cl = install / "bin" / "x64" / "cl";
		std::error_code ec;
		if ( !fs::is_regular_file( cl, ec ) )
			return foundation::MakeUnexpected( ProviderError{ "missing-compiler",
			    cl.string() + " is not installed; run " + std::string( kProvision ) } );
		if ( !StampMatches( install, pins.Value() ) )
			return foundation::MakeUnexpected( ProviderError{
			    "pin-mismatch", install.string() + " was not provisioned from these pins; run " +
			                        std::string( kProvision ) } );
		auto banner = Query( { cl.string() }, request.sourceRoot );
		if ( !banner )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-compiler", cl.string() + ": " + banner.Error() } );
		const std::string line = BannerLine( banner.Value() );
		if ( line.find( "Version " + pins.Value().version + "." ) == std::string::npos )
			return foundation::MakeUnexpected(
			    ProviderError{ "pin-mismatch", "toolchain.version pins " + pins.Value().version +
			                                       "; the installed cl is \"" + line + "\"" } );
		auto wine = Query( { pins.Value().runner, "--version" }, request.sourceRoot );
		if ( !wine )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-runner", pins.Value().runner + ": " + wine.Error() } );

		ToolchainEnvironment environment;
		environment.identity.provider = std::string( kMsvcWineToolchain );
		environment.identity.facts["cxx"] = cl.string();
		environment.identity.facts["cxx.version"] = line;
		environment.identity.facts["target"] = "x86_64-pc-windows-msvc";
		environment.identity.facts["sdk"] = "windows-sdk:" + pins.Value().sdk;
		environment.identity.facts["msvc_toolset"] = pins.Value().toolset;
		environment.identity.facts["msvc_wine"] = pins.Value().commit;
		environment.identity.facts["runner"] = FirstLine( wine.Value() );
		environment.identity.facts["pin.version"] = pins.Value().version;
		environment.environment.push_back( { "CXX", cl.string() } );
		environment.environment.push_back( { "CC", cl.string() } );
		environment.environment.push_back( { "WINEDEBUG", std::string( "-all" ) } );
		environment.wafOptions.push_back( "--msvc-wine=" + install.string() );
		// cl takes no --version; run alone it prints its banner and succeeds.
		environment.compilerProbe = { cl.string() };
		return environment;
	}

	void CheckHost( const ResolvedProfile *profile, DoctorReport &report ) override
	{
		auto pins =
		    profile ? ReadPins( *profile ) : foundation::Expected<Pins, ProviderError>( Pins{} );
		const std::string runner = pins ? pins.Value().runner : std::string( "wine" );
		auto wine = Query( { runner, "--version" }, "/" );
		report.Add( "windows-msvc-wine: " + runner, wine.HasValue(),
		    wine ? FirstLine( wine.Value() ) : wine.Error() );
		if ( !profile )
			return;
		if ( !pins )
		{
			report.Add( "windows-msvc-wine: pins", false, pins.Error().detail );
			return;
		}
		report.Add( "windows-msvc-wine: pins", true,
		    "MSVC " + pins.Value().toolset + ", SDK " + pins.Value().sdk );
	}

private:
	static fs::path Install( const fs::path &dependencyRoot, const Pins &pins )
	{
		return dependencyRoot / std::string( kMsvcWineToolchain ) /
		       ( pins.toolset + "-" + pins.sdk );
	}

	foundation::Expected<std::string, std::string> Query(
	    std::vector<std::string> argv, const fs::path &directory )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = directory.empty() ? std::string( "/" ) : directory.string();
		request.environment.push_back( { "WINEDEBUG", std::string( "-all" ) } );
		request.executionTimeout = std::chrono::milliseconds( 300000 );
		request.cancellationTimeout = std::chrono::milliseconds( 2000 );
		platform::ToolProcessResult result = m_Processes.Run( request );
		if ( !result.Succeeded() )
		{
			std::string detail = result.error.detail;
			if ( detail.empty() )
				detail = result.exitCode ? "exit status " + std::to_string( *result.exitCode )
				                         : "did not run";
			return foundation::MakeUnexpected( detail );
		}
		return result.stdoutData + result.stderrData;
	}

	platform::IToolProcessProvider &m_Processes;
};

} // namespace

std::unique_ptr<ITargetToolchain> CreateMsvcWineToolchain(
    platform::IToolProcessProvider &processes )
{
	return std::make_unique<MsvcWineToolchain>( processes );
}

} // namespace product
