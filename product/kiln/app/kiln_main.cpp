//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `kiln` application (RFC 0027): argument parsing and output
//			formatting only. Each command is one kiln.api call over the
//			kiln.composition catalog; no pipeline logic lives here.
//
//=============================================================================//

#include "foundation/json.h"
#include "kiln/api.h"
#include "kiln/composition.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../../../platform/posix/process_exec.h"

namespace
{

namespace fs = std::filesystem;
using foundation::json::Value;

constexpr const char *kUsage =
    "usage: kiln [--json] [--root <dir>] <command>\n"
    "  profiles list                  every profile and fragment\n"
    "  profiles resolve <profile>     the profile with its extends chain merged\n"
    "  profiles explain <profile>     provenance and derived facts\n"
    "  doctor <profile>               host prerequisites, present or unavailable\n"
    "  build <profile> [--flavor <f>] Waf configure (when changed), build and install\n"
    "  content <profile> [--flavor <f>] build, then the content stages\n"
    "  package <profile> [--flavor <f>] build, then lay out the platform package (the runtime)\n"
    "  switches <profile>             the profile's launch switches\n"
    "  play <profile> [map] [--set <switch>]... [--mounts <set>]... [--flavor <f>] [--dry-run] [-- "
    "args]\n"
    "  run <profile> [map] [--set <switch>]... [--flavor <f>] [--dry-run] [-- args]\n"
    "  package, play and run take --runtime <dir>: package into (and launch from) <dir> instead\n"
    "  of the tree's runtime, e.g. a test's private runtime\n";

class StderrSink final : public product::IDiagnosticSink
{
public:
	explicit StderrSink( bool quiet ) : m_Quiet( quiet ) {}
	void Report(
	    product::Severity severity, std::string_view source, std::string_view message ) override
	{
		if ( m_Quiet && severity == product::Severity::kInfo )
			return;
		std::cerr << "kiln: [" << source << "] " << message << '\n';
	}

private:
	bool m_Quiet;
};

int Usage()
{
	std::cerr << kUsage;
	return 2;
}

int Failure( bool json, const kiln::Error &error )
{
	if ( json )
	{
		Value value = Value::Object();
		value.Set( "schema", Value::String( std::string( kiln::kJsonSchema ) ) );
		value.Set( "error", Value::String( error.code ) );
		value.Set( "detail", Value::String( error.detail ) );
		std::cout << value.WritePretty() << '\n';
	}
	std::cerr << "kiln: " << error.Describe() << '\n';
	return 1;
}

// Become the planned program (kiln play/run on this host).
int Exec( const kiln::LaunchPlan &plan )
{
	std::cout.flush();
	std::cerr.flush();
	std::string error;
	platform::ExecReplacingProcess(
	    plan.argv, plan.environment, plan.workingDirectory.string(), error );
	std::cerr << "kiln: " << error << " (kiln play builds and packages first)\n";
	return 127;
}

} // namespace

int main( int argc, char **argv )
{
	bool json = false;
	fs::path root;
	std::vector<std::string> args;
	for ( int i = 1; i < argc; ++i )
	{
		const std::string arg = argv[i];
		if ( arg == "--" )
		{
			// Everything after `--` belongs to the game, verbatim.
			for ( ; i < argc; ++i )
				args.push_back( argv[i] );
			break;
		}
		if ( arg == "--json" )
			json = true;
		else if ( arg == "--root" && i + 1 < argc )
			root = argv[++i];
		else if ( arg == "-h" || arg == "--help" )
		{
			std::cout << kUsage;
			return 0;
		}
		else
			args.push_back( arg );
	}
	if ( args.empty() )
		return Usage();
	if ( root.empty() )
		root = fs::current_path();
	root = fs::absolute( root ).lexically_normal();
	if ( !fs::exists( root / "wscript" ) )
	{
		std::cerr << "kiln: " << root.string() << " is not the repository root (pass --root)\n";
		return 2;
	}

	auto composition = kiln::ComposeDefault( argc, argv );
	if ( !composition )
		return Failure( json, composition.Error() );
	const kiln::SessionConfig config = kiln::DefaultSessionConfig(
	    root, composition.Value().hostTag, *composition.Value().environment );
	StderrSink sink( json );
	kiln::Session session( composition.Value().catalog, *composition.Value().processes,
	    *composition.Value().executor, sink, config );

	const std::string &command = args[0];
	if ( command == "profiles" && args.size() == 2 && args[1] == "list" )
	{
		const auto profiles = session.ListProfiles();
		if ( json )
		{
			std::cout << kiln::ToJson( profiles ).WritePretty() << '\n';
		}
		else
		{
			for ( const auto &profile : profiles )
			{
				std::string aliases;
				for ( const auto &alias : profile.aliases )
					aliases += ( aliases.empty() ? " (" : ", " ) + alias;
				std::cout << ( profile.error         ? "! "
				                 : profile.buildable ? "  "
				                                     : "- " )
				          << profile.file << aliases << ( aliases.empty() ? "" : ")" );
				if ( profile.error )
					std::cout << "  " << *profile.error;
				else if ( !profile.buildable )
					std::cout << "  [" << ( profile.fragment ? "fragment" : profile.schema ) << "]";
				std::cout << '\n';
			}
		}
		return 0;
	}
	if ( command == "profiles" && args.size() == 3 &&
	     ( args[1] == "resolve" || args[1] == "explain" ) )
	{
		auto profile = session.ResolveProfile( args[2] );
		if ( !profile )
			return Failure( json, profile.Error() );
		// `resolve` prints the merged document itself (with or without
		// --json), the form the retired profile_extends.py printed.
		std::cout << ( args[1] == "resolve" ? profile.Value().document.WritePretty()
		                                    : kiln::ExplainJson( profile.Value() ).WritePretty() )
		          << '\n';
		return 0;
	}
	if ( command == "doctor" && args.size() == 2 )
	{
		auto result = session.Doctor( args[1] );
		if ( !result )
			return Failure( json, result.Error() );
		bool ok = true;
		for ( const auto &item : result.Value().report.items )
			ok &= item.present;
		if ( json )
			std::cout << kiln::ToJson( result.Value() ).WritePretty() << '\n';
		else
		{
			for ( const auto &item : result.Value().report.items )
				std::cout << ( item.present ? "  present      " : "  unavailable  " ) << item.name
				          << "  " << item.detail << '\n';
		}
		return ok ? 0 : 1;
	}
	if ( ( command == "build" || command == "content" || command == "package" ) &&
	     args.size() >= 2 )
	{
		kiln::PipelineRequest request;
		request.profile = args[1];
		request.upTo = command == "build"     ? product::StageRole::kEngine
		               : command == "content" ? product::StageRole::kContent
		                                      : product::StageRole::kPackage;
		for ( size_t i = 2; i < args.size(); ++i )
		{
			if ( args[i] == "--flavor" && i + 1 < args.size() )
				request.flavor = args[++i];
			else if ( args[i] == "--mounts" && i + 1 < args.size() )
				request.mountSets.push_back( args[++i] );
			else if ( args[i] == "--runtime" && i + 1 < args.size() )
				request.runtime = fs::absolute( args[++i] );
			else
				return Usage();
		}
		auto result = session.Run( request );
		if ( !result )
			return Failure( json, result.Error() );
		if ( json )
			std::cout << kiln::ToJson( result.Value() ).WritePretty() << '\n';
		else
		{
			for ( const auto &stage : result.Value().stages )
				std::cout << stage.name << ": " << stage.summary << '\n';
			std::cout << "tree: " << result.Value().tree.string() << '\n'
			          << "evidence: " << result.Value().evidenceFile.string() << '\n';
		}
		return 0;
	}
	if ( command == "switches" && args.size() == 2 )
	{
		auto switches = session.Switches( args[1] );
		if ( !switches )
			return Failure( json, switches.Error() );
		if ( json )
		{
			Value list = Value::Array();
			for ( const auto &entry : switches.Value() )
			{
				Value item = Value::Object();
				item.Set( "name", Value::String( entry.name ) );
				item.Set( "description", Value::String( entry.description ) );
				list.Push( std::move( item ) );
			}
			std::cout << list.WritePretty() << '\n';
		}
		else
		{
			for ( const auto &entry : switches.Value() )
				std::cout << "  --set " << entry.name << "\n      " << entry.description << '\n';
		}
		return 0;
	}
	if ( ( command == "play" || command == "run" ) && args.size() >= 2 )
	{
		kiln::PlayRequest request;
		request.profile = args[1];
		bool dryRun = false;
		for ( size_t i = 2; i < args.size(); ++i )
		{
			if ( args[i] == "--" )
			{
				request.arguments.assign( args.begin() + static_cast<long>( i ) + 1, args.end() );
				break;
			}
			if ( args[i] == "--set" && i + 1 < args.size() )
				request.switches.push_back( args[++i] );
			else if ( args[i] == "--flavor" && i + 1 < args.size() )
				request.flavor = args[++i];
			else if ( args[i] == "--device" && i + 1 < args.size() )
				request.device = args[++i];
			else if ( args[i] == "--display" && i + 1 < args.size() )
				request.displaySession = args[++i];
			else if ( args[i] == "--mounts" && i + 1 < args.size() )
				request.mountSets.push_back( args[++i] );
			else if ( args[i] == "--runtime" && i + 1 < args.size() )
				request.runtime = fs::absolute( args[++i] );
			else if ( args[i] == "--dry-run" )
				dryRun = true;
			else if ( args[i].rfind( "-", 0 ) != 0 && !request.map )
				request.map = args[i];
			else
				return Usage();
		}
		auto plan = session.PlanLaunch( request );
		if ( !plan )
			return Failure( json, plan.Error() );
		if ( dryRun )
		{
			std::cout << kiln::ToJson( plan.Value() ).WritePretty() << '\n';
			return 0;
		}
		if ( command == "play" )
		{
			auto built = session.BuildForPlay( request );
			if ( !built )
				return Failure( json, built.Error() );
			for ( const auto &stage : built.Value().stages )
				std::cerr << "kiln: " << stage.name << ": " << stage.summary << '\n';
		}
		// One program in the user's own session replaces kiln, as the old
		// launchers' exec did; anything else runs under its run provider.
		const bool single =
		    ( plan.Value().runProvider.empty() || plan.Value().runProvider == "single" ) &&
		    ( plan.Value().displaySession.empty() || plan.Value().displaySession == "user" );
		if ( single )
			return Exec( plan.Value() );
		auto status = session.Launch( request, *composition.Value().spawner );
		if ( !status )
			return Failure( json, status.Error() );
		return status.Value();
	}
	return Usage();
}
