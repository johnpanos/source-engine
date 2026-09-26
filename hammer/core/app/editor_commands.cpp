//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Named, serializable editor commands over EditorController
//			(RFC 0002, hammer.app). One table owns every command's name,
//			arguments, summary and handler, so dispatch and the catalog cannot
//			disagree.
//
//=============================================================================//

#include "hammer/app/editor_commands.h"

#include "hammer/app/save_orchestrator.h"

#include <cerrno>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string>

namespace hammer::app
{

namespace
{

using geometry::Vec3d;
using Result = foundation::Expected<std::string, CommandError>;

foundation::Unexpected<CommandError> Fail(
    CommandStatus status, std::string_view command, std::string detail )
{
	return foundation::MakeUnexpected(
	    CommandError{ status, std::string( command ), std::move( detail ), 0 } );
}

std::optional<double> ParseNumber( const std::string &text )
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

std::optional<Vec3d> ParseVector( const std::string &text )
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
	return Vec3d( v[0], v[1], v[2] );
}

struct Context
{
	EditorController &controller;
	ports::IFileStore &store;
	const CommandArgs &args;
	std::string_view name;

	const std::string &Arg( const char *key ) const { return args.at( key ); }

	foundation::Expected<double, CommandError> Number( const char *key ) const
	{
		if ( auto value = ParseNumber( Arg( key ) ) )
			return *value;
		return Fail( CommandStatus::InvalidArgument, name,
		    std::string( key ) + " is not a number: " + Arg( key ) );
	}

	foundation::Expected<int, CommandError> Id( const char *key ) const
	{
		const auto value = ParseNumber( Arg( key ) );
		if ( !value || *value != static_cast<double>( static_cast<int>( *value ) ) )
			return Fail( CommandStatus::InvalidArgument, name,
			    std::string( key ) + " is not an integer id: " + Arg( key ) );
		return static_cast<int>( *value );
	}

	foundation::Expected<Vec3d, CommandError> Vector( const char *key ) const
	{
		if ( auto value = ParseVector( Arg( key ) ) )
			return *value;
		return Fail( CommandStatus::InvalidArgument, name,
		    std::string( key ) + " is not an \"x y z\" vector: " + Arg( key ) );
	}

	Result Done( bool accepted, const char *what ) const
	{
		if ( !accepted )
			return Fail( CommandStatus::Rejected, name, what );
		return std::string();
	}
};

struct CommandEntry
{
	CommandInfo info;
	std::function<Result( const Context & )> run;
};

const std::vector<CommandEntry> &Table()
{
	static const std::vector<CommandEntry> table = {
	    { { "new_map", {}, {}, "Start an empty map (clears history)." },
	        []( const Context &c ) -> Result
	        {
		        c.controller.NewMap();
		        return std::string();
	        } },
	    { { "open", { "path" }, {}, "Load a VMF from the file store." },
	        []( const Context &c ) -> Result
	        {
		        std::string text;
		        if ( !c.store.Read( c.Arg( "path" ), text ) )
			        return Fail(
			            CommandStatus::IoFailure, c.name, "cannot read " + c.Arg( "path" ) );
		        std::string error;
		        if ( !c.controller.LoadVmf( text, error ) )
			        return Fail( CommandStatus::Rejected, c.name, "not a VMF: " + error );
		        return std::string();
	        } },
	    { { "save", { "path" }, {}, "Write the map as VMF (atomic replace) and mark it saved." },
	        []( const Context &c ) -> Result
	        {
		        const SaveStatus status =
		            SaveDocument( c.store, c.Arg( "path" ), c.controller.ToVmf() );
		        if ( status != SaveStatus::kOk )
			        return Fail(
			            CommandStatus::IoFailure, c.name, "cannot save " + c.Arg( "path" ) );
		        c.controller.MarkSaved();
		        return std::string();
	        } },
	    { { "create_block", { "mins", "maxs" }, {},
	          "Add an axis-aligned world brush with the active material; outputs its id." },
	        []( const Context &c ) -> Result
	        {
		        auto mins = c.Vector( "mins" );
		        if ( !mins )
			        return foundation::MakeUnexpected( mins.Error() );
		        auto maxs = c.Vector( "maxs" );
		        if ( !maxs )
			        return foundation::MakeUnexpected( maxs.Error() );
		        const auto id = c.controller.CreateBlock( mins.Value(), maxs.Value() );
		        if ( !id )
			        return Fail( CommandStatus::Rejected, c.name, "degenerate box" );
		        return std::to_string( *id );
	        } },
	    { { "place_entity", { "classname", "origin" }, {},
	          "Place a point entity; outputs its id." },
	        []( const Context &c ) -> Result
	        {
		        auto origin = c.Vector( "origin" );
		        if ( !origin )
			        return foundation::MakeUnexpected( origin.Error() );
		        const auto id = c.controller.PlaceEntity( c.Arg( "classname" ), origin.Value() );
		        if ( !id )
			        return Fail( CommandStatus::Rejected, c.name, "empty classname" );
		        return std::to_string( *id );
	        } },
	    { { "set_entity_origin", { "id", "origin" }, {}, "Move a point entity." },
	        []( const Context &c ) -> Result
	        {
		        auto id = c.Id( "id" );
		        if ( !id )
			        return foundation::MakeUnexpected( id.Error() );
		        auto origin = c.Vector( "origin" );
		        if ( !origin )
			        return foundation::MakeUnexpected( origin.Error() );
		        return c.Done( c.controller.SetEntityOrigin( id.Value(), origin.Value() ),
		            "unknown entity or unchanged origin" );
	        } },
	    { { "set_entity_property", { "id", "key", "value" }, {},
	          "Set a keyvalue on a point entity." },
	        []( const Context &c ) -> Result
	        {
		        auto id = c.Id( "id" );
		        if ( !id )
			        return foundation::MakeUnexpected( id.Error() );
		        return c.Done(
		            c.controller.SetEntityProperty( id.Value(), c.Arg( "key" ), c.Arg( "value" ) ),
		            "unknown entity, reserved key or unchanged value" );
	        } },
	    { { "set_world_property", { "key", "value" }, {},
	          "Set a worldspawn keyvalue (e.g. skyname)." },
	        []( const Context &c ) -> Result
	        {
		        return c.Done( c.controller.SetWorldProperty( c.Arg( "key" ), c.Arg( "value" ) ),
		            "reserved key or unchanged value" );
	        } },
	    { { "set_material", { "material" }, {},
	          "Choose the material new blocks and apply_material use." },
	        []( const Context &c ) -> Result
	        {
		        c.controller.SetActiveMaterial( c.Arg( "material" ) );
		        return std::string();
	        } },
	    { { "apply_material", {}, {}, "Apply the active material to the selected brushes." },
	        []( const Context &c ) -> Result
	        {
		        return c.Done( c.controller.ApplyActiveMaterialToSelection(),
		            "no brush selected, or it already has the material" );
	        } },
	    { { "delete_selection", {}, {}, "Delete the selected brushes or entity." },
	        []( const Context &c ) -> Result
	        {
		        return c.Done( c.controller.DeleteSelection(), "nothing selected" );
	        } },
	    { { "set_grid", { "size" }, {}, "Set the grid size used by pointer snapping." },
	        []( const Context &c ) -> Result
	        {
		        auto size = c.Id( "size" );
		        if ( !size )
			        return foundation::MakeUnexpected( size.Error() );
		        c.controller.SetGridSize( size.Value() );
		        return std::string();
	        } },
	    { { "undo", {}, {}, "Undo the last edit." },
	        []( const Context &c ) -> Result
	        {
		        return c.Done( c.controller.Undo(), "nothing to undo" );
	        } },
	    { { "redo", {}, {}, "Redo the last undone edit." },
	        []( const Context &c ) -> Result
	        {
		        return c.Done( c.controller.Redo(), "nothing to redo" );
	        } },
	    { { "info", {}, {}, "Report brush and entity counts and the modified flag." },
	        []( const Context &c ) -> Result
	        {
		        return "brushes=" + std::to_string( c.controller.Brushes().size() ) +
		               " entities=" + std::to_string( c.controller.Entities().size() ) +
		               " modified=" + ( c.controller.IsModified() ? "1" : "0" );
	        } },
	};
	return table;
}

bool Contains( const std::vector<std::string> &keys, const std::string &key )
{
	for ( const std::string &k : keys )
	{
		if ( k == key )
			return true;
	}
	return false;
}

} // namespace

EditorCommands::EditorCommands( EditorController &controller, ports::IFileStore &store )
    : m_controller( controller ), m_store( store )
{
}

foundation::Expected<std::string, CommandError> EditorCommands::Execute(
    std::string_view name, const CommandArgs &args )
{
	for ( const CommandEntry &entry : Table() )
	{
		if ( entry.info.name != name )
			continue;
		for ( const std::string &key : entry.info.required )
		{
			if ( args.find( key ) == args.end() )
				return Fail( CommandStatus::MissingArgument, name, "missing " + key );
		}
		for ( const auto &pair : args )
		{
			if ( !Contains( entry.info.required, pair.first ) &&
			     !Contains( entry.info.optional, pair.first ) )
				return Fail(
				    CommandStatus::InvalidArgument, name, "unknown argument " + pair.first );
		}
		return entry.run( Context{ m_controller, m_store, args, name } );
	}
	return Fail( CommandStatus::UnknownCommand, name, "no such command" );
}

foundation::Expected<std::vector<std::string>, CommandError> EditorCommands::Run(
    const std::vector<ScriptCommand> &script )
{
	std::vector<std::string> outputs;
	for ( const ScriptCommand &command : script )
	{
		auto result = Execute( command.name, command.args );
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

const std::vector<CommandInfo> &EditorCommands::Catalog()
{
	static const std::vector<CommandInfo> catalog = []
	{
		std::vector<CommandInfo> infos;
		for ( const CommandEntry &entry : Table() )
			infos.push_back( entry.info );
		return infos;
	}();
	return catalog;
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
