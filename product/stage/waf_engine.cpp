//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.stage.waf: the Waf engine build stage (RFC 0027 L0).
//
//=============================================================================//

#include "product/stage_waf.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace product
{

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kLockName = ".lock-waf-kiln";
constexpr const char *kStampName = ".kiln-configure.json";

// Configuration inputs besides the wscripts Waf recorded (the same set
// tools/quality/ensure_configured.py watches).
constexpr const char *kInputRoots[] = { "waf", "scripts/waifulib", "quality/toolchain",
    "quality/profiles", "quality/product_profiles", "architecture/modules.json" };

std::string ReadFile( const fs::path &path )
{
	std::ifstream stream( path, std::ios::binary );
	std::ostringstream text;
	text << stream.rdbuf();
	return text.str();
}

bool WriteFileAtomic( const fs::path &path, const std::string &text )
{
	const fs::path staging = path.string() + ".staging";
	{
		std::ofstream stream( staging, std::ios::binary | std::ios::trunc );
		if ( !stream.write( text.data(), static_cast<std::streamsize>( text.size() ) ) )
			return false;
	}
	std::error_code ec;
	fs::rename( staging, path, ec );
	return !ec;
}

// The wscripts the tree's last configure read: the quoted entries of the
// lock file's `files = [...]` line.
std::vector<fs::path> RecordedFiles( const fs::path &tree )
{
	std::vector<fs::path> files;
	std::istringstream lock( ReadFile( tree / kLockName ) );
	std::string line;
	while ( std::getline( lock, line ) )
	{
		if ( line.rfind( "files = ", 0 ) != 0 )
			continue;
		size_t at = 0;
		while ( ( at = line.find( '\'', at ) ) != std::string::npos )
		{
			const size_t end = line.find( '\'', at + 1 );
			if ( end == std::string::npos )
				break;
			files.emplace_back( line.substr( at + 1, end - at - 1 ) );
			at = end + 1;
		}
	}
	return files;
}

// The first configuration input newer than the tree's configuration, if any.
std::optional<std::string> StaleInput( const fs::path &source, const fs::path &tree )
{
	std::error_code ec;
	const auto configured = fs::last_write_time( tree / "c4che" / "_cache.py", ec );
	if ( ec )
		return std::string( "no configuration" );
	const auto newer = [&]( const fs::path &path )
	{
		std::error_code inner;
		const auto time = fs::last_write_time( path, inner );
		return !inner && time > configured;
	};
	const std::vector<fs::path> recorded = RecordedFiles( tree );
	if ( recorded.empty() )
		return std::string( "no recorded wscripts" );
	for ( const fs::path &file : recorded )
	{
		if ( !fs::exists( file, ec ) || newer( file ) )
			return file.string();
	}
	for ( const char *root : kInputRoots )
	{
		const fs::path path = source / root;
		if ( fs::is_regular_file( path, ec ) )
		{
			if ( newer( path ) )
				return path.string();
			continue;
		}
		for ( auto it = fs::recursive_directory_iterator( path, ec );
		    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
		{
			if ( it->is_regular_file() && newer( it->path() ) )
				return it->path().string();
		}
	}
	return std::nullopt;
}

std::string Tail( const std::string &text, size_t lines )
{
	size_t at = text.size();
	for ( size_t count = 0; at > 0 && count <= lines; )
	{
		--at;
		if ( text[at] == '\n' )
			++count;
	}
	return text.substr( at );
}

struct WafCounts
{
	std::uint64_t tasks = 0;
	std::uint64_t installs = 0;
};

// Waf's task lines are "[ 12/345] Compiling ..."; installs that copied a
// file print "+ install ..." (unchanged files print "- install").
WafCounts CountWork( const std::string &output )
{
	WafCounts counts;
	std::istringstream stream( output );
	std::string line;
	while ( std::getline( stream, line ) )
	{
		const size_t start = line.find_first_not_of( ' ' );
		if ( start == std::string::npos )
			continue;
		if ( line[start] == '[' )
		{
			const size_t slash = line.find( '/', start );
			const size_t close = line.find( ']', start );
			if ( slash != std::string::npos && close != std::string::npos && slash < close )
				++counts.tasks;
		}
		else if ( line.compare( start, 10, "+ install " ) == 0 ||
		          line.compare( start, 9, "+ symlink" ) == 0 )
			++counts.installs;
	}
	return counts;
}

std::string InstallDigest( const fs::path &install )
{
	std::vector<std::string> entries;
	std::error_code ec;
	for ( auto it = fs::recursive_directory_iterator( install, ec );
	    !ec && it != fs::recursive_directory_iterator(); it.increment( ec ) )
	{
		std::error_code inner;
		if ( !it->is_regular_file( inner ) )
			continue;
		const auto time = it->last_write_time( inner ).time_since_epoch().count();
		entries.push_back( fs::relative( it->path(), install, inner ).generic_string() + " " +
		                   std::to_string( it->file_size( inner ) ) + " " +
		                   std::to_string( time ) );
	}
	std::sort( entries.begin(), entries.end() );
	std::string joined;
	for ( const std::string &entry : entries )
		joined += entry + "\n";
	return HashHex( joined );
}

class WafEngineStage final : public IProductStage
{
public:
	explicit WafEngineStage( std::string python ) : m_Python( std::move( python ) ) {}

	std::string_view Name() const noexcept override { return kWafEngineStage; }

	StageDescriptor Describe( const ResolvedProfile & ) const override
	{
		StageDescriptor descriptor;
		descriptor.role = StageRole::kEngine;
		descriptor.produces = { std::string( kEngineInstallArtifact ) };
		descriptor.determinism = Determinism::kExact;
		return descriptor;
	}

	foundation::Expected<StageResult, ProviderError> Run(
	    StageInputs &inputs, StageOutputs &outputs ) override
	{
		if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( kCancelled ), "before configure" } );
		if ( !inputs.Processes() )
			return foundation::MakeUnexpected(
			    ProviderError{ "invalid-request", "no process provider" } );
		auto arguments = inputs.Profile().WafArguments( inputs.Flavor() );
		if ( !arguments )
			return foundation::MakeUnexpected(
			    ProviderError{ "invalid-request", arguments.Error().Describe() } );

		const fs::path source = inputs.SourceRoot();
		// out/<profile>/<flavor>/: the Waf tree (and its lock) in build/, the
		// installed products in install/, beside the runtime and artifacts.
		const fs::path tree = inputs.TreeRoot() / "build";
		const fs::path prefix = inputs.TreeRoot() / "install";
		std::error_code ec;
		fs::create_directories( tree, ec );
		if ( ec )
			return foundation::MakeUnexpected(
			    ProviderError{ "io", "cannot create " + tree.string() } );

		const std::string identity =
		    inputs.Toolchain() ? inputs.Toolchain()->identity.Hex() : std::string();
		std::string joined;
		for ( const std::string &argument : arguments.Value() )
			joined += argument + "\n";
		const std::string digest = HashHex( joined + "toolchain=" + identity );

		std::string reason;
		const Value stamp =
		    foundation::json::Parse( ReadFile( tree / kStampName ) ).ValueOr( Value() );
		const std::string *recorded = stamp.IsObject() ? stamp.FindString( "digest" ) : nullptr;
		if ( !recorded )
			reason = "first configure";
		else if ( *recorded != digest )
			reason = "configure options or toolchain changed";
		else if ( const std::string *cache = stamp.FindString( "cache_digest" );
		    !cache || *cache != HashHex( ReadFile( tree / "c4che" / "_cache.py" ) ) )
			reason = "the tree was configured outside kiln";
		else if ( auto stale = StaleInput( source, tree ) )
			reason = "configuration input changed: " + *stale;

		Value evidence = Value::Object();
		// A value, not a reference into `evidence`: later Set calls may move it.
		Value argumentList = Value::Array();
		for ( const std::string &argument : arguments.Value() )
			argumentList.Push( Value::String( argument ) );
		evidence.Set( "configure_arguments", argumentList );
		evidence.Set( "configure_digest", Value::String( digest ) );
		evidence.Set( "tree", Value::String( tree.string() ) );

		if ( !reason.empty() )
		{
			std::vector<std::string> argv = { m_Python, ( source / "waf" ).string(), "configure",
			    "-o", tree.string(), "--prefix=" + prefix.string() };
			argv.insert( argv.end(), arguments.Value().begin(), arguments.Value().end() );
			if ( inputs.Diagnostics() )
				inputs.Diagnostics()->Report(
				    Severity::kInfo, Name(), "configuring " + tree.string() + " (" + reason + ")" );
			auto configured = Waf( inputs, std::move( argv ), source );
			if ( !configured )
				return foundation::MakeUnexpected( configured.Error() );
			Value newStamp = Value::Object();
			newStamp.Set( "digest", Value::String( digest ) );
			newStamp.Set( "arguments", argumentList );
			newStamp.Set( "toolchain", Value::String( identity ) );
			newStamp.Set( "cache_digest",
			    Value::String( HashHex( ReadFile( tree / "c4che" / "_cache.py" ) ) ) );
			if ( !WriteFileAtomic( tree / kStampName, newStamp.WritePretty() + "\n" ) )
				return foundation::MakeUnexpected(
				    ProviderError{ "io", "cannot record the configure digest" } );
		}
		evidence.Set( "configured", Value::Bool( !reason.empty() ) );
		evidence.Set( "configure_reason", Value::String( reason ) );

		if ( inputs.Cancel() && inputs.Cancel()->IsCancelled() )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( kCancelled ), "before build" } );
		auto built = Waf( inputs, { m_Python, ( source / "waf" ).string(), "install" }, tree );
		if ( !built )
			return foundation::MakeUnexpected( built.Error() );
		const WafCounts counts = CountWork( built.Value() );
		evidence.Set( "waf_tasks", Value::Number( static_cast<long long>( counts.tasks ) ) );
		evidence.Set(
		    "installed_files", Value::Number( static_cast<long long>( counts.installs ) ) );

		Artifact install;
		install.name = std::string( kEngineInstallArtifact );
		install.type = "install";
		install.path = prefix;
		install.digest = InstallDigest( install.path );
		install.facts.Set( "tree", Value::String( tree.string() ) );
		outputs.Publish( std::move( install ) );

		StageResult result;
		result.workItems = counts.tasks + counts.installs + ( reason.empty() ? 0 : 1 );
		result.upToDate = result.workItems == 0;
		result.summary = result.upToDate
		                     ? "up to date"
		                     : ( reason.empty() ? "" : "configured; " ) +
		                           std::to_string( counts.tasks ) + " Waf tasks, " +
		                           std::to_string( counts.installs ) + " files installed";
		result.evidence = std::move( evidence );
		return result;
	}

private:
	foundation::Expected<std::string, ProviderError> Waf(
	    StageInputs &inputs, std::vector<std::string> argv, const fs::path &directory )
	{
		platform::ToolProcessRequest request;
		request.argv = std::move( argv );
		request.workingDirectory = directory.string();
		if ( inputs.Toolchain() )
			request.environment = inputs.Toolchain()->environment;
		request.environment.push_back( { "WAFLOCK", std::string( kLockName ) } );
		request.environment.push_back( { "NO_LOCK_IN_TOP", std::string( "1" ) } );
		request.environment.push_back( { "NO_LOCK_IN_RUN", std::string( "1" ) } );
		request.executionTimeout = std::chrono::hours( 6 );
		request.cancellationTimeout = std::chrono::seconds( 20 );
		CancellationAdapter cancel( inputs.Cancel() );
		request.cancellation = &cancel;
		platform::ToolProcessResult result = inputs.Processes()->Run( request );
		if ( result.completion == platform::ToolProcessCompletion::kCanceled )
			return foundation::MakeUnexpected(
			    ProviderError{ std::string( kCancelled ), request.argv[2] } );
		if ( !result.Succeeded() )
		{
			std::string detail = request.argv[2] + " failed";
			if ( result.exitCode )
				detail += " (exit " + std::to_string( *result.exitCode ) + ")";
			if ( !result.error.detail.empty() )
				detail += ": " + result.error.detail;
			detail += "\n" + Tail( result.stdoutData, 30 ) + Tail( result.stderrData, 30 );
			return foundation::MakeUnexpected( ProviderError{ "waf-failed", detail } );
		}
		return result.stdoutData + result.stderrData;
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

	std::string m_Python;
};

} // namespace

std::unique_ptr<IProductStage> CreateWafEngineStage( std::string python )
{
	return std::make_unique<WafEngineStage>( std::move( python ) );
}

} // namespace product
