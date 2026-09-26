//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app.editor_commands conformance (RFC 0002 / roadmap R08):
//			the named command layer the GTK host, scripts, UI-driven tests and an
//			MCP server share. Authors a playable room by script, saves it
//			through the file-store port, reloads it, and checks every error path.
//
//=============================================================================//

#include "hammer/app/editor_commands.h"
#include "testing/checks.h"

#include "fake_file_store.h"

#include <cstdio>
#include <string>

namespace
{

using hammer::app::CommandStatus;
using hammer::app::EditorCommands;
using hammer::app::EditorController;
using hammertest::InMemoryFileStore;

// A sealed 256x256x128 room (six 16-unit walls), a light and a player start:
// the smallest map the compile tools accept without a leak.
const char kRoomScript[] = R"(# room.hcmd
new_map
set_material material=DEV/DEV_MEASUREGENERIC01B
create_block mins="-144 -144 -16" maxs="144 144 0"
create_block mins="-144 -144 128" maxs="144 144 144"
create_block mins="-144 -144 0" maxs="-128 144 128"
create_block mins="128 -144 0" maxs="144 144 128"
create_block mins="-128 -144 0" maxs="128 -128 128"
create_block mins="-128 128 0" maxs="128 144 128"
place_entity classname=info_player_start origin="0 0 1"
place_entity classname=light origin="0 0 96"
set_entity_property id=8 key=_light value="255 255 255 400"
save path=maps/room.vmf
info
)";

} // namespace

int main()
{
	testing::Checks checks;
	auto check = [&]( bool value, std::source_location where = std::source_location::current() )
	{
		checks.That( value, "check", where );
	};

	// The catalog lists each command once with its arguments.
	const auto &catalog = EditorCommands::Catalog();
	check( catalog.size() >= 15 );
	int creates = 0;
	for ( const auto &info : catalog )
		creates += info.name == "create_block";
	check( creates == 1 );

	// A script authors, saves and reports a playable room.
	{
		EditorController controller;
		InMemoryFileStore store;
		EditorCommands commands( controller, store );
		auto script = hammer::app::ParseCommandScript( kRoomScript );
		check( script.HasValue() );
		if ( script )
		{
			check( script.Value().size() == 13 );
			auto outputs = commands.Run( script.Value() );
			check( outputs.HasValue() );
			if ( !outputs )
				std::printf( "script failed at line %d (%s): %s\n", outputs.Error().line,
				    outputs.Error().command.c_str(), outputs.Error().detail.c_str() );
			if ( outputs )
			{
				const auto &o = outputs.Value();
				check( o.size() == 13 );
				check( o.size() == 13 && o[2] == "1" && o[7] == "6" && o[8] == "7" && o[9] == "8" );
				check( o.size() == 13 && o[12] == "brushes=6 entities=2 modified=0" );
			}
		}
		check( store.files.count( "maps/room.vmf" ) == 1 );
		const std::string saved = store.files["maps/room.vmf"];
		check( saved.find( "\"classname\" \"info_player_start\"" ) != std::string::npos );
		check( saved.find( "\"_light\" \"255 255 255 400\"" ) != std::string::npos );
		check( saved.find( "\"skyname\" \"sky_day01_01\"" ) != std::string::npos );

		// Reopening through the same port restores the same document.
		EditorController reopened;
		EditorCommands again( reopened, store );
		check( again.Execute( "open", { { "path", "maps/room.vmf" } } ).HasValue() );
		check( reopened.Brushes().size() == 6 && reopened.Entities().size() == 2 );
		check( reopened.ToVmf() == saved );
	}

	// Every error path is structured and names the request.
	{
		EditorController controller;
		InMemoryFileStore store;
		EditorCommands commands( controller, store );
		controller.NewMap();
		auto expect = [&]( const char *name, hammer::app::CommandArgs args, CommandStatus status,
		                  std::source_location where = std::source_location::current() )
		{
			auto result = commands.Execute( name, args );
			checks.That( !result.HasValue(), name, where );
			if ( !result )
			{
				checks.That( result.Error().status == status, name, where );
				checks.That( result.Error().command == name, name, where );
			}
		};
		expect( "no_such_command", {}, CommandStatus::UnknownCommand );
		expect( "create_block", { { "mins", "0 0 0" } }, CommandStatus::MissingArgument );
		expect( "create_block", { { "mins", "0 0" }, { "maxs", "1 1 1" } },
		    CommandStatus::InvalidArgument );
		expect( "create_block", { { "mins", "0 0 0" }, { "maxs", "1 1 1" }, { "colour", "red" } },
		    CommandStatus::InvalidArgument );
		expect(
		    "create_block", { { "mins", "0 0 0" }, { "maxs", "0 1 1" } }, CommandStatus::Rejected );
		expect( "set_entity_origin", { { "id", "1.5" }, { "origin", "0 0 0" } },
		    CommandStatus::InvalidArgument );
		expect( "set_entity_origin", { { "id", "99" }, { "origin", "0 0 0" } },
		    CommandStatus::Rejected );
		expect( "open", { { "path", "missing.vmf" } }, CommandStatus::IoFailure );
		expect( "undo", {}, CommandStatus::Rejected );
		store.failAllWrites = true;
		expect( "save", { { "path", "maps/x.vmf" } }, CommandStatus::IoFailure );
		check( store.files.count( "maps/x.vmf" ) == 0 );

		// A failing script line stops the run and reports its line.
		auto script = hammer::app::ParseCommandScript(
		    "new_map\ncreate_block mins=\"0 0 0\" maxs=\"8 8 8\"\nundo\nundo\n" );
		check( script.HasValue() );
		if ( script )
		{
			auto run = commands.Run( script.Value() );
			check( !run.HasValue() && run.Error().line == 4 );
			check( controller.Brushes().empty() ); // lines 1-3 ran
		}
	}

	// Script syntax errors carry their line.
	{
		auto unterminated = hammer::app::ParseCommandScript( "new_map\nsave path=\"a.vmf\n" );
		check(
		    !unterminated.HasValue() && unterminated.Error().status == CommandStatus::SyntaxError );
		check( !unterminated.HasValue() && unterminated.Error().line == 2 );
		auto bare = hammer::app::ParseCommandScript( "save a.vmf" );
		check( !bare.HasValue() && bare.Error().status == CommandStatus::SyntaxError );
		auto duplicate = hammer::app::ParseCommandScript( "save path=a path=b" );
		check( !duplicate.HasValue() );
		auto comments = hammer::app::ParseCommandScript( "  # only a comment\n\n" );
		check( comments.HasValue() && comments.Value().empty() );
	}

	return checks.Report();
}
