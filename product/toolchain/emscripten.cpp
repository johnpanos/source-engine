//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.toolchain.emscripten: the emscripten cross toolchain
//			(RFC 0027, RFC 0029). See public/product/toolchain_emscripten.h.
//
//=============================================================================//

#include "product/toolchain_emscripten.h"

#include <chrono>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kFamily = "emscripten";

std::string FirstLine( const std::string &text )
{
	return text.substr( 0, text.find( '\n' ) );
}

// The last non-empty line: webgpu_lane.py fetch prints em++'s path last.
std::string LastLine( const std::string &text )
{
	std::string line;
	size_t start = 0;
	while ( start < text.size() )
	{
		size_t end = text.find( '\n', start );
		if ( end == std::string::npos )
			end = text.size();
		if ( end > start )
			line = text.substr( start, end - start );
		start = end + 1;
	}
	return line;
}

// "emcc (...) 6.0.10 (<hash>)": the version is a whole token.
bool VersionMatches( const std::string &line, const std::string &pin )
{
	for ( size_t at = line.find( pin ); at != std::string::npos; at = line.find( pin, at + 1 ) )
	{
		const size_t end = at + pin.size();
		if ( ( at == 0 || line[at - 1] == ' ' ) && ( end == line.size() || line[end] == ' ' ) )
			return true;
	}
	return false;
}

ProviderError Error( std::string code, std::string detail )
{
	return ProviderError{ std::move( code ), std::move( detail ) };
}

class EmscriptenToolchain final : public ITargetToolchain
{
public:
	explicit EmscriptenToolchain( platform::IToolProcessProvider &processes )
	    : m_Processes( processes )
	{
	}

	std::string_view Name() const noexcept override { return kEmscriptenToolchain; }

	foundation::Expected<ToolchainEnvironment, ProviderError> Prepare(
	    const ToolchainRequest &request ) override
	{
		if ( request.cancel && request.cancel->IsCancelled() )
			return foundation::MakeUnexpected( Error( std::string( kCancelled ), "before prepare" ) );
		if ( !request.profile )
			return foundation::MakeUnexpected( Error( "invalid-request", "no profile" ) );
		const Value *toolchain = request.profile->document.Find( "toolchain" );
		const std::string *family = toolchain ? toolchain->FindString( "family" ) : nullptr;
		const std::string *version = toolchain ? toolchain->FindString( "version" ) : nullptr;
		if ( !family || !version || version->empty() )
			return foundation::MakeUnexpected( Error(
			    "missing-pin", "toolchain.family and toolchain.version are required" ) );
		if ( *family != kFamily )
			return foundation::MakeUnexpected( Error( "toolchain-mismatch",
			    std::string( kEmscriptenToolchain ) + " builds toolchain.family \"" + kFamily +
			        "\", the profile pins \"" + *family + "\"" ) );

		// The SDK and the browser's webgpu.h, fetched and verified by their
		// one owner (it writes only under dependencies/).
		const fs::path lane = request.sourceRoot / "tools" / "render" / "webgpu_lane.py";
		auto fetched = Query( { "python3", lane.string(), "fetch" }, request.cancel,
		    std::chrono::hours( 1 ) );
		if ( !fetched )
			return foundation::MakeUnexpected(
			    Error( "unavailable", "webgpu_lane.py fetch: " + fetched.Error() ) );
		const fs::path cxx = LastLine( fetched.Value() );
		const fs::path emscripten = cxx.parent_path();
		const fs::path emsdk = emscripten.parent_path().parent_path();
		auto identity = Query( { cxx.string(), "--version" }, request.cancel );
		if ( !identity )
			return foundation::MakeUnexpected(
			    Error( "missing-compiler", cxx.string() + ": " + identity.Error() ) );
		const std::string line = FirstLine( identity.Value() );
		if ( !VersionMatches( line, *version ) )
			return foundation::MakeUnexpected( Error( "pin-mismatch",
			    "toolchain.version pins " + *version + "; em++ is \"" + line + "\"" ) );

		ToolchainEnvironment environment;
		Identity &id = environment.identity;
		id.provider = std::string( kEmscriptenToolchain );
		id.facts["cc"] = ( emscripten / "emcc" ).string();
		id.facts["cxx"] = cxx.string();
		id.facts["cxx.version"] = line;
		id.facts["pin.version"] = *version;
		id.facts["sdk"] = "emsdk:" + emsdk.filename().string();
		id.facts["target"] = "wasm32-unknown-emscripten";
		environment.environment = { { "EMSDK", emsdk.string() },
		    { "EM_CONFIG", ( emsdk / ".emscripten" ).string() } };
		environment.wafOptions = { "--emscripten", "--emsdk=" + emsdk.string() };
		return environment;
	}

	void CheckHost( const ResolvedProfile *, DoctorReport &report ) override
	{
		auto python = Query( { "python3", "--version" } );
		report.Add( std::string( kEmscriptenToolchain ) + ": python3", python.HasValue(),
		    python ? FirstLine( python.Value() ) : "needed to fetch and verify the pinned SDK" );
		auto node = Query( { "node", "--version" } );
		report.Add( std::string( kEmscriptenToolchain ) + ": node", node.HasValue(),
		    node ? FirstLine( node.Value() ) : "the SDK's compiler driver and the Node lane need it" );
	}

private:
	foundation::Expected<std::string, std::string> Query( std::vector<std::string> argv,
	    const ICancellation *cancel = nullptr,
	    std::chrono::milliseconds timeout = std::chrono::seconds( 30 ) )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = "/";
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

std::unique_ptr<ITargetToolchain> CreateEmscriptenToolchain(
    platform::IToolProcessProvider &processes )
{
	return std::make_unique<EmscriptenToolchain>( processes );
}

} // namespace product
