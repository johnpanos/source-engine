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

	// The Source 2 way: one block, hollowed into a sealed room; select, move,
	// describe; the selection rule is brushes or one entity.
	{
		EditorController controller;
		InMemoryFileStore store;
		EditorCommands commands( controller, store );
		auto run = [&]( const char *name, hammer::app::CommandArgs args )
		{
			return commands.Execute( name, args );
		};
		check( run( "new_map", {} ).HasValue() );
		auto block =
		    run( "create_block", { { "mins", "-144 -144 -16" }, { "maxs", "144 144 144" } } );
		check( block.HasValue() && block.Value() == "1" );
		auto walls = run( "hollow", { { "id", "1" }, { "thickness", "16" } } );
		check( walls.HasValue() && walls.Value() == "2,3,4,5,6,7" );
		check( controller.Brushes().size() == 6 && controller.SelectionCount() == 6 );
		check(
		    run( "describe", { { "id", "2" } } ).Value() ==
		    "brush mins=\"-144 -144 -16\" maxs=\"144 144 0\" material=DEV/DEV_MEASUREGENERIC01B" );
		check( !run( "hollow", { { "id", "2" }, { "thickness", "16" } } ).HasValue() ); // too thin
		check( !run( "hollow", { { "id", "99" }, { "thickness", "16" } } ).HasValue() );
		check( run( "undo", {} ).HasValue() && controller.Brushes().size() == 1 ); // one unit
		check( run( "redo", {} ).HasValue() && controller.Brushes().size() == 6 );

		auto start =
		    run( "place_entity", { { "classname", "info_player_start" }, { "origin", "0 0 1" } } );
		check( start.HasValue() && start.Value() == "8" );
		check( !run( "select", { { "ids", "2,8" } } ).HasValue() ); // brushes + entity
		check( !run( "select", { { "ids", "2,x" } } ).HasValue() );
		check( !run( "select", { { "ids", "2" }, { "mode", "sideways" } } ).HasValue() );
		check(
		    run( "select", { { "ids", "2,3" } } ).HasValue() && controller.SelectionCount() == 2 );
		check( run( "select", { { "ids", "3" }, { "mode", "toggle" } } ).HasValue() &&
		       controller.SelectionCount() == 1 );
		check( run( "select", { { "ids", "4" }, { "mode", "add" } } ).HasValue() &&
		       controller.SelectionCount() == 2 );
		check( run( "select", { { "ids", "8" } } ).HasValue() && controller.SelectedEntity() == 8 &&
		       controller.SelectionCount() == 0 );
		check( run( "move_selection", { { "delta", "16 0 0" } } ).HasValue() );
		check( run( "describe", { { "id", "8" } } ).Value() ==
		       "entity classname=info_player_start origin=\"16 0 1\"" );
		check( !run( "move_selection", { { "delta", "0 0 0" } } ).HasValue() );
		check( run( "select_none", {} ).HasValue() && !controller.SelectedEntity() );
		check( !run( "move_selection", { { "delta", "1 0 0" } } ).HasValue() );
		check( !run( "describe", { { "id", "99" } } ).HasValue() );
		// Source 2's Entity tool: click the floor in 3D (a downward ray from
		// inside the hollowed room) and the entity stands one unit above it.
		auto hit = run( "raycast", { { "origin", "0 0 100" }, { "dir", "0 0 -1" } } );
		check( hit.HasValue() && hit.Value() == "id=2 point=\"0 0 0\" normal=\"0 0 1\"" );
		auto light = run( "place_on_surface",
		    { { "classname", "light" }, { "origin", "0 0 100" }, { "dir", "0 0 -1" } } );
		check( light.HasValue() && light.Value() == "9" );
		check( run( "describe", { { "id", "9" } } ).Value() ==
		       "entity classname=light origin=\"0 0 1\"" );
		check( !run( "place_on_surface",
		    { { "classname", "light" }, { "origin", "0 0 500" }, { "dir", "0 0 1" } } )
		        .HasValue() );
		// A ray starting inside the floor brush hits it with a zero normal, and
		// nothing can be placed on it.
		auto inside = run( "raycast", { { "origin", "0 0 -8" }, { "dir", "0 0 -1" } } );
		check(
		    inside.HasValue() && inside.Value().find( "normal=\"0 0 0\"" ) != std::string::npos );
		check( !run( "place_on_surface",
		    { { "classname", "light" }, { "origin", "0 0 -8" }, { "dir", "0 0 -1" } } )
		        .HasValue() );
	}

	// build_map saves exactly the document, then asks the injected builder.
	{
		struct FakeBuilder final : hammer::ports::IMapBuilder
		{
			std::vector<hammer::ports::MapBuildRequest> requests;
			hammer::ports::MapBuildResult next{ true, "pass", "" };
			hammer::ports::MapBuildResult Build( const hammer::ports::MapBuildRequest &r ) override
			{
				requests.push_back( r );
				return next;
			}
		};
		EditorController controller;
		InMemoryFileStore store;
		FakeBuilder builder;
		EditorCommands none( controller, store );
		EditorCommands commands( controller, store, &builder );
		controller.NewMap();
		check( commands.Execute( "create_block", { { "mins", "0 0 0" }, { "maxs", "64 64 64" } } )
		        .HasValue() );
		auto missing = none.Execute( "build_map", { { "path", "a.vmf" } } );
		check( !missing && missing.Error().status == CommandStatus::Rejected );
		auto built =
		    commands.Execute( "build_map", { { "path", "maps/a.vmf" }, { "publish", "1" } } );
		check( built.HasValue() && built.Value() == "pass" && !controller.IsModified() );
		check( builder.requests.size() == 1 && builder.requests[0].vmfPath == "maps/a.vmf" &&
		       builder.requests[0].publish && !builder.requests[0].fullQuality );
		check( store.files["maps/a.vmf"] == controller.ToVmf() );
		check( !commands.Execute( "build_map", { { "path", "a.vmf" }, { "quality", "slow" } } )
		        .HasValue() );
		check( builder.requests.size() == 1 ); // rejected before saving or building
		builder.next = { false, "leak", "map leaks near 0 0 0" };
		auto leak = commands.Execute( "build_map", { { "path", "a.vmf" }, { "quality", "full" } } );
		check( !leak && leak.Error().detail == "leak: map leaks near 0 0 0" );
		check( builder.requests.size() == 2 && builder.requests[1].fullQuality );
		store.failAllWrites = true;
		check( !commands.Execute( "build_map", { { "path", "b.vmf" } } ).HasValue() );
		check( builder.requests.size() == 2 ); // an unsaved map is not built
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
