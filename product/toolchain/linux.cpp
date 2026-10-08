//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.linux: the linux-gcc and linux-clang host
//			toolchains (RFC 0027 L0).
//
//=============================================================================//

#include "product/toolchain_linux.h"

#include <chrono>

namespace product
{

namespace
{

struct Family
{
	const char *provider;   // "linux-gcc"
	const char *family;     // the profile's toolchain.family
	const char *defaultCc;  // used when the profile names no `cc`
	const char *defaultCxx; // used when the profile names no `cxx`
};

constexpr Family kGcc{ "linux-gcc", "gcc", "gcc", "g++" };
constexpr Family kClang{ "linux-clang", "clang", "clang", "clang++" };

std::string FirstLine( const std::string &text )
{
	const size_t end = text.find( '\n' );
	return text.substr( 0, end );
}

std::string Trim( std::string text )
{
	while ( !text.empty() && ( text.back() == '\n' || text.back() == '\r' || text.back() == ' ' ) )
		text.pop_back();
	return text;
}

// A version pin matches when it appears as a whole token of the compiler's
// --version line ("g++ (GCC) 16.2.1 20260819", "clang version 22.1.8").
bool VersionMatches( const std::string &line, const std::string &pin )
{
	size_t at = line.find( pin );
	while ( at != std::string::npos )
	{
		const bool startOk = at == 0 || line[at - 1] == ' ' || line[at - 1] == '(';
		const size_t end = at + pin.size();
		const bool endOk =
		    end == line.size() || line[end] == ' ' || line[end] == ')' || line[end] == '-';
		if ( startOk && endOk )
			return true;
		at = line.find( pin, at + 1 );
	}
	return false;
}

class LinuxToolchain final : public ITargetToolchain
{
public:
	LinuxToolchain( const Family &family, platform::IToolProcessProvider &processes )
	    : m_Family( family ), m_Processes( processes )
	{
	}

	std::string_view Name() const noexcept override { return m_Family.provider; }

	foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) override
	{
		if ( request.cancel && request.cancel->IsCancelled() )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( kCancelled ), "before prepare" } );
		if ( !request.profile )
			return foundation::MakeUnexpected( ProviderError{ "invalid-request", "no profile" } );
		const foundation::json::Value *toolchain = request.profile->document.Find( "toolchain" );
		const std::string *family = toolchain ? toolchain->FindString( "family" ) : nullptr;
		const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
		if ( !family || !version || version->empty() )
			return foundation::MakeUnexpected( ProviderError{
			    "missing-pin", "toolchain.family and toolchain.version are required" } );
		if ( *family != m_Family.family )
			return foundation::MakeUnexpected( ProviderError{ "toolchain-mismatch",
			    std::string( m_Family.provider ) + " builds toolchain.family \"" + m_Family.family +
			        "\", the profile pins \"" + *family + "\"" } );
		const std::string cxx = Pick( toolchain, "cxx", m_Family.defaultCxx );
		const std::string cc = Pick( toolchain, "cc", m_Family.defaultCc );

		auto cxxVersion = Query( { cxx, "--version" }, request.sourceRoot );
		if ( !cxxVersion )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-compiler", cxx + ": " + cxxVersion.Error() } );
		const std::string line = FirstLine( cxxVersion.Value() );
		if ( !VersionMatches( line, *version ) )
			return foundation::MakeUnexpected(
			    ProviderError{ "pin-mismatch", "toolchain.version pins " + *version +
			                                       "; the host " + cxx + " is \"" + line + "\"" } );
		auto ccVersion = Query( { cc, "--version" }, request.sourceRoot );
		if ( !ccVersion )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-compiler", cc + ": " + ccVersion.Error() } );
		if ( !VersionMatches( FirstLine( ccVersion.Value() ), *version ) )
			return foundation::MakeUnexpected( ProviderError{
			    "pin-mismatch", "toolchain.version pins " + *version + "; the host " + cc +
			                        " is \"" + FirstLine( ccVersion.Value() ) + "\"" } );
		auto machine = Query( { cxx, "-dumpmachine" }, request.sourceRoot );
		if ( !machine )
			return foundation::MakeUnexpected(
			    ProviderError{ "missing-compiler", cxx + ": " + machine.Error() } );

		ToolchainEnvironment environment;
		environment.identity.provider = m_Family.provider;
		environment.identity.facts["cc"] = cc;
		environment.identity.facts["cc.version"] = FirstLine( ccVersion.Value() );
		environment.identity.facts["cxx"] = cxx;
		environment.identity.facts["cxx.version"] = line;
		environment.identity.facts["target"] = Trim( machine.Value() );
		environment.identity.facts["pin.version"] = *version;
		if ( const std::string *library = toolchain->FindString( "standard_library" ) )
			environment.identity.facts["standard_library"] = *library;
		// The host SDK is the system: its identity is the compiler's target.
		environment.identity.facts["sdk"] = "host:" + Trim( machine.Value() );
		environment.environment.push_back( { "CC", cc } );
		environment.environment.push_back( { "CXX", cxx } );
		return environment;
	}

	void CheckHost( const ResolvedProfile *profile, DoctorReport &report ) override
	{
		const foundation::json::Value *toolchain =
		    profile ? profile->document.Find( "toolchain" ) : nullptr;
		const std::string cxx = Pick( toolchain, "cxx", m_Family.defaultCxx );
		const std::string cc = Pick( toolchain, "cc", m_Family.defaultCc );
		for ( const std::string &tool : { cc, cxx } )
		{
			auto version = Query( { tool, "--version" }, "/" );
			report.Add( std::string( m_Family.provider ) + ": " + tool, version.HasValue(),
			    version ? FirstLine( version.Value() ) : version.Error() );
		}
		const std::string *pin = toolchain ? toolchain->FindString( "version" ) : nullptr;
		if ( pin )
		{
			auto version = Query( { cxx, "--version" }, "/" );
			const bool matches = version && VersionMatches( FirstLine( version.Value() ), *pin );
			report.Add( std::string( m_Family.provider ) + ": pin " + *pin, matches,
			    matches ? "host compiler matches the profile"
			            : "host compiler differs from the pin" );
		}
	}

private:
	static std::string Pick(
	    const foundation::json::Value *toolchain, const char *key, const char *fallback )
	{
		const std::string *value = toolchain ? toolchain->FindString( key ) : nullptr;
		return value && !value->empty() ? *value : std::string( fallback );
	}

	foundation::Expected<std::string, std::string> Query(
	    std::vector<std::string> argv, const std::filesystem::path &directory )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = directory.empty() ? std::string( "/" ) : directory.string();
		request.executionTimeout = std::chrono::milliseconds( 30000 );
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
		return result.stdoutData;
	}

	const Family &m_Family;
	platform::IToolProcessProvider &m_Processes;
};

} // namespace

std::unique_ptr<ITargetToolchain> CreateLinuxGccToolchain(
    platform::IToolProcessProvider &processes )
{
	return std::make_unique<LinuxToolchain>( kGcc, processes );
}

std::unique_ptr<ITargetToolchain> CreateLinuxClangToolchain(
    platform::IToolProcessProvider &processes )
{
	return std::make_unique<LinuxToolchain>( kClang, processes );
}

} // namespace product
