//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: kiln.core: the launch plan of a profile (RFC 0027 L1). Launches
//			are profile data: `launch.arguments` is a template over named
//			variables, switches set variables or append arguments, and the
//			workspace binds the personal values. Names no platform.
//
//=============================================================================//

#include "kiln/api.h"

#include <algorithm>
#include <map>

namespace kiln
{

namespace
{

using foundation::json::Value;
using Variables = std::map<std::string, std::vector<std::string>>;

Error Fail( std::string code, std::string detail )
{
	return Error{ std::move( code ), std::move( detail ) };
}

std::vector<std::string> Strings( const Value *value )
{
	std::vector<std::string> list;
	if ( value && value->IsArray() )
	{
		for ( const Value &item : value->Items() )
		{
			if ( item.IsString() )
				list.push_back( item.Text() );
		}
	}
	return list;
}

// "{name}" exactly: the variable's name; otherwise empty.
std::string Placeholder( const std::string &element )
{
	if ( element.size() > 2 && element.front() == '{' && element.back() == '}' &&
	     element.find( '{', 1 ) == std::string::npos )
		return element.substr( 1, element.size() - 2 );
	return {};
}

// Substitutes "{name}" inside a string; each variable must be one value.
std::optional<std::string> Substitute(
    const std::string &text, const Variables &variables, std::string &missing )
{
	std::string out;
	size_t at = 0;
	while ( at < text.size() )
	{
		const size_t open = text.find( '{', at );
		if ( open == std::string::npos )
		{
			out += text.substr( at );
			break;
		}
		const size_t close = text.find( '}', open );
		if ( close == std::string::npos )
		{
			out += text.substr( at );
			break;
		}
		out += text.substr( at, open - at );
		const std::string name = text.substr( open + 1, close - open - 1 );
		auto it = variables.find( name );
		if ( name == "inherit" )
			out += "{inherit}"; // filled by the run provider from the inherited value
		else if ( it == variables.end() || it->second.size() > 1 )
		{
			missing = name;
			return std::nullopt;
		}
		else if ( !it->second.empty() )
			out += it->second.front();
		at = close + 1;
	}
	return out;
}

foundation::Expected<std::vector<std::string>, Error> Expand(
    const std::vector<std::string> &templateList, const Variables &variables,
    const std::string &where )
{
	std::vector<std::string> out;
	for ( const std::string &element : templateList )
	{
		const std::string name = Placeholder( element );
		if ( !name.empty() )
		{
			auto it = variables.find( name );
			if ( it == variables.end() )
				return foundation::MakeUnexpected(
				    Fail( "launch", where + ": \"{" + name + "}\" is not a launch variable" ) );
			// The request's own tail is literal: a game argument may contain braces.
			if ( name == "args" )
			{
				out.insert( out.end(), it->second.begin(), it->second.end() );
				continue;
			}
			// Profile values may name single-valued variables ({root}, {runtime}).
			for ( const std::string &value : it->second )
			{
				std::string missing;
				auto text = Substitute( value, variables, missing );
				if ( !text )
					return foundation::MakeUnexpected(
					    Fail( "launch", where + ": {" + name + "} uses \"{" + missing +
					                        "}\", not a single-valued launch variable" ) );
				out.push_back( *text );
			}
			continue;
		}
		std::string missing;
		auto text = Substitute( element, variables, missing );
		if ( !text )
			return foundation::MakeUnexpected( Fail( "launch",
			    where + ": \"{" + missing + "}\" is not a single-valued launch variable" ) );
		out.push_back( *text );
	}
	return out;
}

std::string NumberText( const Value &value )
{
	return value.IsNumber() ? value.Text() : value.IsString() ? value.Text() : std::string();
}

} // namespace

foundation::Expected<std::vector<product::Switch>, Error> Session::Switches(
    const std::string &nameOrAlias ) const
{
	auto profile = ResolveForLaunch( nameOrAlias );
	if ( !profile )
		return foundation::MakeUnexpected( profile.Error() );
	return profile.Value().switches;
}

foundation::Expected<product::DisplayEnvironment, Error> Session::OpenDisplay(
    const std::string &name, const product::DisplayRequest &request ) const
{
	auto session = m_Catalog.DisplaySession( name );
	if ( !session )
		return foundation::MakeUnexpected( Fail( "display", session.Error().Describe() ) );
	auto opened = session.Value()->Open( request );
	if ( !opened )
		return foundation::MakeUnexpected(
		    Fail( "display", name + ": " + opened.Error().code + ": " + opened.Error().detail ) );
	return opened.Value();
}

void Session::CloseDisplay( const std::string &name ) const
{
	if ( auto session = m_Catalog.DisplaySession( name ) )
		session.Value()->Close();
}

foundation::Expected<LaunchPlan, Error> Session::PlanLaunch( const PlayRequest &request ) const
{
	auto resolved = ResolveForLaunch( request.profile );
	if ( !resolved )
		return foundation::MakeUnexpected( resolved.Error() );
	const product::ResolvedProfile &profile = resolved.Value();
	const Value *launch = profile.document.Find( "launch" );
	if ( !launch || !launch->Find( "arguments" ) )
		return foundation::MakeUnexpected(
		    Fail( "launch", profile.name + " declares no launch.arguments" ) );

	LaunchPlan plan;
	plan.profile = profile.name;
	if ( profile.Buildable() )
	{
		plan.flavor = request.flavor.value_or( profile.defaultFlavor );
		if ( !profile.FindFlavor( plan.flavor ) )
			return foundation::MakeUnexpected(
			    Fail( "profile", "\"" + plan.flavor + "\" is not a flavor of " + profile.name ) );
		plan.tree = m_Config.outRoot / profile.TreeName( plan.flavor );
		plan.runtime = plan.tree / profile.PackageDirectoryName();
	}
	else
		plan.runtime = m_Config.outRoot / profile.name / "runtime";
	if ( request.runtime )
		plan.runtime = *request.runtime;
	plan.workingDirectory = plan.runtime;
	if ( const std::string *display = launch->FindString( "display_session" ) )
		plan.displaySession = *display;
	if ( request.displaySession )
	{
		if ( !m_Catalog.DisplaySession( *request.displaySession ) )
			return foundation::MakeUnexpected(
			    Fail( "catalog", "\"" + *request.displaySession + "\" is not a display session" ) );
		plan.displaySession = *request.displaySession;
	}
	if ( const std::string *run = launch->FindString( "run" ) )
		plan.runProvider = *run;

	// Variables: the profile's, then the workspace's personal values, then
	// the switches in the order given.
	Variables variables = profile.launchVariables;
	auto workspace = LoadWorkspace();
	if ( !workspace )
		return foundation::MakeUnexpected( workspace.Error() );
	const Value &personal = workspace.Value().document;
	if ( const Value *resolution = personal.Find( "resolution" ) )
	{
		if ( !resolution->IsArray() || resolution->Items().size() != 2 )
			return foundation::MakeUnexpected(
			    Fail( "workspace", "resolution is [width, height]" ) );
		variables["width"] = { NumberText( resolution->Items()[0] ) };
		variables["height"] = { NumberText( resolution->Items()[1] ) };
	}
	if ( const Value *frameCap = personal.Find( "frame_cap" ) )
		variables["frame_cap"] = { NumberText( *frameCap ) };
	if ( const Value *windowed = personal.Find( "windowed" ) )
	{
		if ( windowed->IsBool() && !windowed->AsBool() )
			variables["windowed"] = {};
	}

	for ( const std::string &name : request.switches )
	{
		auto it = std::find_if( profile.switches.begin(), profile.switches.end(),
		    [&]( const product::Switch &entry )
		    {
			    return entry.name == name;
		    } );
		if ( it == profile.switches.end() )
			return foundation::MakeUnexpected(
			    Fail( "switch", "\"" + name + "\" is not a switch of " + profile.name +
			                        " (kiln switches " + profile.name + ")" ) );
		for ( const std::string &other : request.switches )
		{
			if ( std::find( it->conflicts.begin(), it->conflicts.end(), other ) !=
			     it->conflicts.end() )
				return foundation::MakeUnexpected(
				    Fail( "switch", "\"" + name + "\" conflicts with \"" + other + "\"" ) );
		}
		if ( std::count( request.switches.begin(), request.switches.end(), name ) > 1 )
			return foundation::MakeUnexpected(
			    Fail( "switch", "\"" + name + "\" is given twice" ) );
		for ( const auto &set : it->sets )
			variables[set.first] = set.second;
		plan.switches.push_back( name );
	}

	std::vector<std::string> switchArguments;
	for ( const std::string &name : plan.switches )
	{
		for ( const product::Switch &entry : profile.switches )
		{
			if ( entry.name == name )
				switchArguments.insert(
				    switchArguments.end(), entry.arguments.begin(), entry.arguments.end() );
		}
	}
	variables["switches"] = switchArguments;
	variables["args"] = request.arguments;
	variables["root"] = { m_Config.sourceRoot.string() };
	auto locations = ResolveLocations( profile );
	if ( !locations )
		return foundation::MakeUnexpected( locations.Error() );
	for ( const auto &[name, path] : locations.Value() )
		variables["locator:" + name] = { path.string() };
	variables["runtime"] = { plan.runtime.string() };
	if ( const std::string *game = launch->FindString( "game" ) )
		variables["game"] = { *game };

	std::optional<std::string> map = request.map;
	if ( !map )
	{
		if ( const std::string *defaultMap = launch->FindString( "default_map" ) )
			map = *defaultMap;
		if ( workspace.Value().defaultMap && launch->FindString( "default_map" ) )
			map = *workspace.Value().defaultMap;
	}
	variables["map"] = map ? std::vector<std::string>{ *map } : std::vector<std::string>{};
	variables["map_arguments"] = {};
	if ( map )
	{
		auto mapArguments =
		    Expand( Strings( launch->Find( "map_arguments" ) ), variables, "launch.map_arguments" );
		if ( !mapArguments )
			return foundation::MakeUnexpected( mapArguments.Error() );
		variables["map_arguments"] = mapArguments.Value();
	}

	if ( const std::string *directory = launch->FindString( "working_directory" ) )
	{
		std::string missing;
		auto resolved = Substitute( *directory, variables, missing );
		if ( !resolved )
			return foundation::MakeUnexpected(
			    Fail( "launch", "launch.working_directory: \"{" + missing + "}\" is unknown" ) );
		plan.workingDirectory = *resolved;
	}
	const std::string *executable = launch->FindString( "executable" );
	if ( !executable )
		return foundation::MakeUnexpected(
		    Fail( "launch", profile.name + " declares no launch.executable" ) );
	if ( request.exactArguments && ( !request.switches.empty() || request.map ||
	                                   !request.arguments.empty() || launch->Find( "peers" ) ) )
		return foundation::MakeUnexpected( Fail( "request",
		    "exact arguments take no switches, map or arguments, and no profile with peers" ) );
	auto arguments =
	    request.exactArguments
	        ? foundation::Expected<std::vector<std::string>, Error>( *request.exactArguments )
	        : Expand( Strings( launch->Find( "arguments" ) ), variables, "launch.arguments" );
	if ( !arguments )
		return foundation::MakeUnexpected( arguments.Error() );
	std::string missingExecutable;
	auto program = Substitute( *executable, variables, missingExecutable );
	if ( !program )
		return foundation::MakeUnexpected(
		    Fail( "launch", "launch.executable: \"{" + missingExecutable +
		                        "}\" is not a single-valued launch variable" ) );
	if ( !request.wrapper.empty() && launch->Find( "peers" ) )
		return foundation::MakeUnexpected( Fail( "request", "a wrapper needs a single program" ) );
	plan.argv = request.wrapper;
	// A program path as given (an installed game); a bare name is in the runtime.
	plan.argv.push_back( program->find( '/' ) != std::string::npos ? *program : "./" + *program );
	plan.argv.insert( plan.argv.end(), arguments.Value().begin(), arguments.Value().end() );

	if ( const Value *peers = launch->Find( "peers" ) )
	{
		// Each peer's arguments take the {args} place of the base template;
		// {args} inside them is the request's own tail.
		for ( const Value &peer : peers->IsArray() ? peers->Items() : std::vector<Value>{} )
		{
			const std::string *name = peer.FindString( "name" );
			if ( !name )
				return foundation::MakeUnexpected(
				    Fail( "launch", "every launch.peers entry has a name" ) );
			auto peerArguments =
			    Expand( Strings( peer.Find( "arguments" ) ), variables, "launch.peers." + *name );
			if ( !peerArguments )
				return foundation::MakeUnexpected( peerArguments.Error() );
			Variables peerVariables = variables;
			peerVariables["args"] = peerArguments.Value();
			auto peerArgv =
			    Expand( Strings( launch->Find( "arguments" ) ), peerVariables, "launch.arguments" );
			if ( !peerArgv )
				return foundation::MakeUnexpected( peerArgv.Error() );
			LaunchPlan::Peer entry;
			entry.name = *name;
			entry.argv.push_back( plan.argv.front() );
			entry.argv.insert( entry.argv.end(), peerArgv.Value().begin(), peerArgv.Value().end() );
			plan.peers.push_back( std::move( entry ) );
		}
	}
	if ( const Value *facts = launch->Find( "facts" ) )
	{
		for ( const auto &member :
		    facts->IsObject() ? facts->Members() : std::vector<std::pair<std::string, Value>>{} )
		{
			std::string missing;
			auto value = member.second.IsString()
			                 ? Substitute( member.second.Text(), variables, missing )
			                 : std::nullopt;
			if ( !value )
				return foundation::MakeUnexpected(
				    Fail( "launch", "launch.facts." + member.first +
				                        " is not a string over single-valued variables" ) );
			plan.facts[member.first] = *value;
		}
	}
	if ( const Value *environment = launch->Find( "environment" ) )
	{
		for ( const auto &member : environment->Members() )
		{
			std::string missing;
			auto value = Substitute( member.second.Text(), variables, missing );
			if ( !value )
				return foundation::MakeUnexpected(
				    Fail( "launch", "launch.environment." + member.first + ": \"{" + missing +
				                        "}\" is not a single-valued launch variable" ) );
			plan.environment.push_back( { member.first, *value } );
		}
	}
	plan.environment.insert(
	    plan.environment.end(), request.environment.begin(), request.environment.end() );
	return plan;
}

foundation::Expected<PipelineResult, Error> Session::BuildForPlay( const PlayRequest &request )
{
	PipelineRequest build;
	build.profile = request.profile;
	build.flavor = request.flavor;
	build.upTo = product::StageRole::kPackage;
	build.device = request.device;
	build.mountSets = request.mountSets;
	build.runtime = request.runtime;
	build.cancel = request.cancel;
	return Run( build );
}

foundation::Expected<int, Error> Session::Launch(
    const PlayRequest &request, platform::IProcessSpawner &spawner ) const
{
	auto plan = PlanLaunch( request );
	if ( !plan )
		return foundation::MakeUnexpected( plan.Error() );
	const std::string displayName =
	    plan.Value().displaySession.empty() ? "user" : plan.Value().displaySession;
	const std::string runName =
	    plan.Value().runProvider.empty() ? "single" : plan.Value().runProvider;
	auto display = m_Catalog.DisplaySession( displayName );
	if ( !display )
		return foundation::MakeUnexpected( Fail( "catalog", display.Error().Describe() ) );
	auto run = m_Catalog.RunProvider( runName );
	if ( !run )
		return foundation::MakeUnexpected( Fail( "catalog", run.Error().Describe() ) );
	product::DisplayRequest displayRequest;
	// A chosen runtime keeps its session beside it, so runs from different
	// runtimes never share one.
	displayRequest.scratch =
	    request.runtime ? request.runtime->parent_path() /
	                          ( request.runtime->filename().string() + ".display-session" )
	                    : ( plan.Value().tree.empty() ? m_Config.outRoot / plan.Value().profile
	                                                  : plan.Value().tree ) /
	                          "display-session";
	if ( request.displayMode )
	{
		displayRequest.width = request.displayMode->width;
		displayRequest.height = request.displayMode->height;
		displayRequest.refreshHz = request.displayMode->refreshHz;
	}
	displayRequest.cancel = request.cancel;
	auto environment = display.Value()->Open( displayRequest );
	if ( !environment )
		return foundation::MakeUnexpected(
		    Fail( "display", environment.Error().code + ": " + environment.Error().detail ) );
	product::RunRequest runRequest;
	runRequest.display = environment.Value();
	runRequest.facts = plan.Value().facts;
	runRequest.spawner = &spawner;
	runRequest.diagnostics = &m_Diagnostics;
	runRequest.cancel = request.cancel;
	if ( request.started )
	{
		runRequest.started = [&request]( const std::string &name, platform::SpawnedProcess process )
		{
			request.started( name, process.id );
		};
	}
	const auto spec = [&]( const std::string &name, const std::vector<std::string> &argv )
	{
		product::LaunchSpec launch;
		launch.name = name;
		launch.argv = argv;
		launch.environment = plan.Value().environment;
		launch.workingDirectory = plan.Value().workingDirectory;
		if ( request.log )
			launch.outputFile = plan.Value().peers.empty()
			                        ? *request.log
			                        : request.log->parent_path() /
			                              ( request.log->filename().string() + "." + name );
		return launch;
	};
	if ( plan.Value().peers.empty() )
		runRequest.launches.push_back( spec( "game", plan.Value().argv ) );
	for ( const LaunchPlan::Peer &peer : plan.Value().peers )
		runRequest.launches.push_back( spec( peer.name, peer.argv ) );
	auto status = run.Value()->Run( runRequest );
	display.Value()->Close();
	if ( !status )
		return foundation::MakeUnexpected(
		    Fail( "run", status.Error().code + ": " + status.Error().detail ) );
	return status.Value();
}

Value ToJson( const LaunchPlan &plan )
{
	Value value = Value::Object();
	value.Set( "schema", Value::String( std::string( kJsonSchema ) ) );
	value.Set( "profile", Value::String( plan.profile ) );
	value.Set( "flavor", Value::String( plan.flavor ) );
	value.Set( "runtime", Value::String( plan.runtime.string() ) );
	value.Set( "working_directory", Value::String( plan.workingDirectory.string() ) );
	Value &argv = value.Set( "argv", Value::Array() );
	for ( const std::string &argument : plan.argv )
		argv.Push( Value::String( argument ) );
	Value &environment = value.Set( "environment", Value::Object() );
	for ( const auto &entry : plan.environment )
		environment.Set( entry.name, entry.value ? Value::String( *entry.value ) : Value() );
	Value &switches = value.Set( "switches", Value::Array() );
	for ( const std::string &name : plan.switches )
		switches.Push( Value::String( name ) );
	value.Set( "display_session", Value::String( plan.displaySession ) );
	value.Set( "run", Value::String( plan.runProvider ) );
	Value peers = Value::Array();
	for ( const LaunchPlan::Peer &peer : plan.peers )
	{
		Value entry = Value::Object();
		entry.Set( "name", Value::String( peer.name ) );
		Value peerArgv = Value::Array();
		for ( const std::string &argument : peer.argv )
			peerArgv.Push( Value::String( argument ) );
		entry.Set( "argv", std::move( peerArgv ) );
		peers.Push( std::move( entry ) );
	}
	value.Set( "peers", std::move( peers ) );
	Value facts = Value::Object();
	for ( const auto &[name, fact] : plan.facts )
		facts.Set( name, Value::String( fact ) );
	value.Set( "facts", std::move( facts ) );
	return value;
}

} // namespace kiln
