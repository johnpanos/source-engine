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
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string>

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

namespace
{

using mapgeometry::Vec3d;
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
	ports::IMapBuilder *builder;
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
	    { { "raycast", { "origin", "dir" }, {},
	          "The nearest brush a ray enters: id, hit point and face normal." },
	        []( const Context &c ) -> Result
	        {
		        auto origin = c.Vector( "origin" );
		        if ( !origin )
			        return foundation::MakeUnexpected( origin.Error() );
		        auto dir = c.Vector( "dir" );
		        if ( !dir )
			        return foundation::MakeUnexpected( dir.Error() );
		        const auto hit = c.controller.Raycast( origin.Value(), dir.Value() );
		        if ( !hit )
			        return Fail( CommandStatus::Rejected, c.name, "the ray hits no brush" );
		        char buf[256];
		        std::snprintf( buf, sizeof( buf ), "id=%d point=\"%g %g %g\" normal=\"%g %g %g\"",
		            hit->brushId, hit->point.x, hit->point.y, hit->point.z, hit->normal.x,
		            hit->normal.y, hit->normal.z );
		        return std::string( buf );
	        } },
	    { { "place_on_surface", { "classname", "origin", "dir" }, {},
	          "Place a point entity one unit off the surface a ray hits; outputs its id." },
	        []( const Context &c ) -> Result
	        {
		        auto origin = c.Vector( "origin" );
		        if ( !origin )
			        return foundation::MakeUnexpected( origin.Error() );
		        auto dir = c.Vector( "dir" );
		        if ( !dir )
			        return foundation::MakeUnexpected( dir.Error() );
		        const auto id = c.controller.PlaceEntityOnSurface(
		            c.Arg( "classname" ), origin.Value(), dir.Value() );
		        if ( !id )
			        return Fail( CommandStatus::Rejected, c.name,
			            "the ray hits no surface, or the classname is empty" );
		        return std::to_string( *id );
	        } },
	    { { "select", { "ids" }, { "mode" },
	          "Select brushes or one entity by id (\"1,2\"); mode replace (default), add or "
	          "toggle." },
	        []( const Context &c ) -> Result
	        {
		        std::vector<int> ids;
		        std::string token;
		        const std::string &list = c.Arg( "ids" );
		        for ( std::size_t i = 0; i <= list.size(); ++i )
		        {
			        if ( i < list.size() && list[i] != ',' )
			        {
				        token += list[i];
				        continue;
			        }
			        const auto value = ParseNumber( token );
			        if ( !value || *value != static_cast<double>( static_cast<int>( *value ) ) )
				        return Fail(
				            CommandStatus::InvalidArgument, c.name, "not an id list: " + list );
			        ids.push_back( static_cast<int>( *value ) );
			        token.clear();
		        }
		        auto mode = EditorController::SelectMode::Replace;
		        if ( auto it = c.args.find( "mode" ); it != c.args.end() )
		        {
			        if ( it->second == "add" )
				        mode = EditorController::SelectMode::Add;
			        else if ( it->second == "toggle" )
				        mode = EditorController::SelectMode::Toggle;
			        else if ( it->second != "replace" )
				        return Fail(
				            CommandStatus::InvalidArgument, c.name, "unknown mode " + it->second );
		        }
		        return c.Done( c.controller.SelectObjects( ids, mode ),
		            "unknown id, or brushes mixed with an entity" );
	        } },
	    { { "select_none", {}, {}, "Clear the selection." },
	        []( const Context &c ) -> Result
	        {
		        c.controller.SelectNone();
		        return std::string();
	        } },
	    { { "move_selection", { "delta" }, {},
	          "Move the selected brushes or entity by \"x y z\"." },
	        []( const Context &c ) -> Result
	        {
		        auto delta = c.Vector( "delta" );
		        if ( !delta )
			        return foundation::MakeUnexpected( delta.Error() );
		        const Vec3d d = delta.Value();
		        return c.Done( c.controller.MoveSelectionBy( d.x, d.y, d.z ),
		            "nothing selected, or a zero move" );
	        } },
	    { { "hollow", { "id", "thickness" }, {},
	          "Turn a box brush into a sealed room of six walls; outputs the wall ids." },
	        []( const Context &c ) -> Result
	        {
		        auto id = c.Id( "id" );
		        if ( !id )
			        return foundation::MakeUnexpected( id.Error() );
		        auto thickness = c.Number( "thickness" );
		        if ( !thickness )
			        return foundation::MakeUnexpected( thickness.Error() );
		        const auto walls = c.controller.Hollow( id.Value(), thickness.Value() );
		        if ( !walls )
			        return Fail( CommandStatus::Rejected, c.name,
			            "unknown brush, not a box, or thinner than two walls" );
		        std::string out;
		        for ( int wall : *walls )
			        out += ( out.empty() ? "" : "," ) + std::to_string( wall );
		        return out;
	        } },
	    { { "describe", { "id" }, {},
	          "Describe a brush (bounds, material) or an entity (keyvalues)." },
	        []( const Context &c ) -> Result
	        {
		        auto id = c.Id( "id" );
		        if ( !id )
			        return foundation::MakeUnexpected( id.Error() );
		        auto vec = []( const Vec3d &v )
		        {
			        char buf[96];
			        std::snprintf( buf, sizeof( buf ), "%g %g %g", v.x, v.y, v.z );
			        return std::string( buf );
		        };
		        for ( const MapBrush &b : c.controller.Brushes() )
		        {
			        if ( b.id == id.Value() )
				        return "brush mins=\"" + vec( b.mins ) + "\" maxs=\"" + vec( b.maxs ) +
				               "\" material=" + ( b.materials.empty() ? "" : b.materials[0] );
		        }
		        for ( const MapEntity &e : c.controller.Entities() )
		        {
			        if ( e.id != id.Value() )
				        continue;
			        std::string out =
			            "entity classname=" + e.classname + " origin=\"" + vec( e.origin ) + "\"";
			        for ( const EntityProperty &p : e.properties )
				        out += " " + p.key + "=\"" + p.value + "\"";
			        return out;
		        }
		        return Fail( CommandStatus::Rejected, c.name, "no such id" );
	        } },
	    { { "build_map", { "path" }, { "quality", "publish" },
	          "Save to path, then compile it (quality fast or full; publish=1 makes it "
	          "playable)." },
	        []( const Context &c ) -> Result
	        {
		        if ( !c.builder )
			        return Fail(
			            CommandStatus::Rejected, c.name, "no map builder in this composition" );
		        const auto quality = c.args.find( "quality" );
		        const bool full = quality != c.args.end() && quality->second == "full";
		        if ( quality != c.args.end() && !full && quality->second != "fast" )
			        return Fail(
			            CommandStatus::InvalidArgument, c.name, "quality is fast or full" );
		        const auto publish = c.args.find( "publish" );
		        if ( publish != c.args.end() && publish->second != "0" && publish->second != "1" )
			        return Fail( CommandStatus::InvalidArgument, c.name, "publish is 0 or 1" );
		        // Build exactly what is saved: the same atomic save as "save".
		        if ( SaveDocument( c.store, c.Arg( "path" ), c.controller.ToVmf() ) !=
		             SaveStatus::kOk )
			        return Fail(
			            CommandStatus::IoFailure, c.name, "cannot save " + c.Arg( "path" ) );
		        c.controller.MarkSaved();
		        const ports::MapBuildResult result = c.builder->Build(
		            { c.Arg( "path" ), full, publish != c.args.end() && publish->second == "1" } );
		        if ( !result.ok )
			        return Fail(
			            CommandStatus::Rejected, c.name, result.status + ": " + result.detail );
		        return result.status;
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

EditorCommands::EditorCommands(
    EditorController &controller, ports::IFileStore &store, ports::IMapBuilder *builder )
    : m_controller( controller ), m_store( store ), m_builder( builder )
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
		return entry.run( Context{ m_controller, m_store, m_builder, args, name } );
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
