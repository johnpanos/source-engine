//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/command_script.h.
//
//=============================================================================//

#include "hammer/app/command_script.h"

#include <cerrno>
#include <cstdlib>

namespace hammer::app
{

const char *CommandStatusName( CommandStatus status )
{
	switch ( status )
	{
	case CommandStatus::UnknownCommand:
		return "unknown command";
	case CommandStatus::MissingArgument:
		return "missing argument";
	case CommandStatus::InvalidArgument:
		return "invalid argument";
	case CommandStatus::Rejected:
		return "rejected";
	case CommandStatus::IoFailure:
		return "i/o failure";
	case CommandStatus::SyntaxError:
		return "syntax error";
	}
	return "error";
}

std::optional<double> ParseCommandNumber( const std::string &text )
{
	if ( text.empty() )
		return std::nullopt;
	errno = 0;
	char *end = nullptr;
	const double value = std::strtod( text.c_str(), &end );
	if ( errno != 0 || end == text.c_str() || *end != '\0' )
		return std::nullopt;
	return value;
}

std::optional<mapgeometry::Vec3d> ParseCommandVector( const std::string &text )
{
	errno = 0;
	const char *cursor = text.c_str();
	double v[3];
	for ( double &component : v )
	{
		char *end = nullptr;
		component = std::strtod( cursor, &end );
		if ( end == cursor || errno != 0 )
			return std::nullopt;
		cursor = end;
	}
	while ( *cursor == ' ' || *cursor == '\t' )
		++cursor;
	if ( *cursor != '\0' )
		return std::nullopt;
	return mapgeometry::Vec3d( v[0], v[1], v[2] );
}

foundation::Expected<void, CommandError> ValidateCommandArgs(
    const CommandInfo &info, const CommandArgs &args )
{
	auto declared = [&]( const std::string &key )
	{
		for ( const std::string &k : info.required )
		{
			if ( k == key )
				return true;
		}
		for ( const std::string &k : info.optional )
		{
			if ( k == key )
				return true;
		}
		return false;
	};
	for ( const std::string &key : info.required )
	{
		if ( args.find( key ) == args.end() )
			return CommandFailure( CommandStatus::MissingArgument, info.name, "missing " + key );
	}
	for ( const auto &pair : args )
	{
		if ( !declared( pair.first ) )
			return CommandFailure(
			    CommandStatus::InvalidArgument, info.name, "unknown argument " + pair.first );
	}
	return {};
}

foundation::Expected<std::vector<std::string>, CommandError> RunCommandScript(
    const std::vector<ScriptCommand> &script,
    const std::function<CommandResult( std::string_view, const CommandArgs & )> &execute )
{
	std::vector<std::string> outputs;
	for ( const ScriptCommand &command : script )
	{
		auto result = execute( command.name, command.args );
		if ( !result )
		{
			CommandError error = result.Error();
			error.line = command.line;
			return foundation::MakeUnexpected( std::move( error ) );
		}
		outputs.push_back( std::move( result.Value() ) );
	}
	return outputs;
}

foundation::Expected<std::vector<ScriptCommand>, CommandError> ParseCommandScript(
    std::string_view text )
{
	std::vector<ScriptCommand> script;
	int lineNumber = 0;
	std::size_t start = 0;
	while ( start <= text.size() )
	{
		std::size_t end = text.find( '\n', start );
		if ( end == std::string_view::npos )
			end = text.size();
		const std::string_view line = text.substr( start, end - start );
		start = end + 1;
		++lineNumber;

		ScriptCommand command;
		command.line = lineNumber;
		std::size_t i = 0;
		auto skipSpace = [&]
		{
			while ( i < line.size() && ( line[i] == ' ' || line[i] == '\t' || line[i] == '\r' ) )
				++i;
		};
		auto syntax = [&]( std::string detail )
		{
			return foundation::MakeUnexpected( CommandError{
			    CommandStatus::SyntaxError, command.name, std::move( detail ), lineNumber } );
		};
		skipSpace();
		if ( i >= line.size() || line[i] == '#' )
			continue;
		while ( i < line.size() && line[i] != ' ' && line[i] != '\t' && line[i] != '\r' )
			command.name += line[i++];
		for ( ;; )
		{
			skipSpace();
			if ( i >= line.size() || line[i] == '#' )
				break;
			std::string key;
			while ( i < line.size() && line[i] != '=' && line[i] != ' ' && line[i] != '\t' )
				key += line[i++];
			if ( key.empty() || i >= line.size() || line[i] != '=' )
				return syntax( "expected key=value" );
			++i;
			std::string value;
			if ( i < line.size() && line[i] == '"' )
			{
				++i;
				while ( i < line.size() && line[i] != '"' )
					value += line[i++];
				if ( i >= line.size() )
					return syntax( "unterminated quote" );
				++i;
			}
			else
			{
				while ( i < line.size() && line[i] != ' ' && line[i] != '\t' && line[i] != '\r' )
					value += line[i++];
			}
			if ( !command.args.emplace( key, value ).second )
				return syntax( "duplicate argument " + key );
		}
		script.push_back( std::move( command ) );
	}
	return script;
}

} // namespace hammer::app
