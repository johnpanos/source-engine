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
			// A spliced value may name single-valued variables ({root}, {runtime}).
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
		plan.runtime = plan.tree / "runtime";
	}
	else
		plan.runtime = m_Config.outRoot / profile.name / "runtime";
	plan.workingDirectory = plan.runtime;
	if ( const std::string *display = launch->FindString( "display_session" ) )
		plan.displaySession = *display;
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

	const std::string *executable = launch->FindString( "executable" );
	if ( !executable )
		return foundation::MakeUnexpected(
		    Fail( "launch", profile.name + " declares no launch.executable" ) );
	auto arguments =
	    Expand( Strings( launch->Find( "arguments" ) ), variables, "launch.arguments" );
	if ( !arguments )
		return foundation::MakeUnexpected( arguments.Error() );
	plan.argv.push_back( "./" + *executable );
	plan.argv.insert( plan.argv.end(), arguments.Value().begin(), arguments.Value().end() );

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
	return plan;
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
	return value;
}

} // namespace kiln
