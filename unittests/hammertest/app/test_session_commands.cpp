//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app session command layer (RFC 0002, R08 domain logic): the
//			catalog (unique names), a sealed room authored by script (primitive,
//			hollow, entities, keys, outputs) saved and reopened through the
//			codec and file-store ports, one undo step per command, selection
//			defaults, every command family exercised once, settings commands,
//			and the history/jump queries. Negative checks: unknown commands,
//			missing and undeclared arguments, malformed numbers, vectors, ids
//			and faces, unknown ids, missing services, script errors with lines.
//
//=============================================================================//

#include "hammer/app/session_commands.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"
#include "fakes/fake_entity_catalog.h"
#include "fakes/fake_map_codec.h"
#include "fakes/fake_material_info.h"

#include <set>

using namespace hammer;
using namespace hammer::app;

namespace
{

class Builder final : public ports::IMapBuilder
{
public:
	ports::MapBuildResult Build( const ports::MapBuildRequest &request ) override
	{
		last = request;
		return { true, "pass", "" };
	}
	ports::MapBuildRequest last;
};

const char kRoom[] = R"(# a sealed room with a light and a player start
new_map
set_material material=DEV/DEV_MEASUREGENERIC01B
create_block mins="-144 -144 -16" maxs="144 144 144"
hollow thickness=16
place_entity classname=info_player_start origin="0 0 1"
place_entity classname=light origin="0 0 96"
set_key key=_light value="255 255 255 400"
save path=maps/room.vmf
info
)";

} // namespace

int main()
{
	testing::Checks checks;

	// Catalog: unique names, each with a summary.
	{
		std::set<std::string> names;
		bool summaries = true;
		for ( const CommandInfo &info : SessionCommands::Catalog() )
		{
			names.insert( info.name );
			summaries = summaries && !info.summary.empty();
		}
		checks.Equal( names.size(), SessionCommands::Catalog().size(), "command names are unique" );
		checks.That( summaries, "every command has a summary" );
		checks.That(
		    SessionCommands::Catalog().size() >= 60, "the catalog covers the operation families" );
	}

	hammertest::FakeEntityCatalog catalog;
	catalog.AddPoint( "info_player_start" )
	    .AddPoint( "light",
	        { hammertest::FakeEntityCatalog::Key( "_light", "color255", "255 255 255 200" ) } )
	    .AddSolid( "func_detail" )
	    .AddSolid( "func_door" );
	hammertest::FakeMaterialInfo materials;
	materials.Add( "DEV/DEV_MEASUREGENERIC01B", 128, 128 );
	hammertest::FakeMapCodec codec;
	hammertest::InMemoryFileStore store;
	Builder builder;
	EditSession session;
	EditorSettings settings;
	MapFragment clipboard;
	SessionCommands commands(
	    session, settings, { &codec, &store, &builder, &catalog, &materials, &clipboard } );

	auto run = [&]( const std::string &text )
	{
		return commands.Run( ParseCommandScript( text ).Value() );
	};

	// The room script.
	{
		auto out = run( kRoom );
		checks.That( out.HasValue(), "the room script runs" );
		if ( out )
		{
			checks.That( out.Value().back().find( "solids=6 entities=2 groups=1 modified=0" ) == 0,
			    "six walls in a group, two entities, saved" );
		}
		const scene::Entity *light = nullptr;
		for ( const auto &[id, e] : session.Document().Entities() )
			if ( e.classname == "light" )
				light = &e;
		checks.That( light && *light->Key( "_light" ) == "255 255 255 400",
		    "set_key acted on the selection (the light)" );
		checks.That( store.files.count( "maps/room.vmf" ) == 1, "saved through the store" );
		checks.That( commands.Execute( "open", { { "path", "maps/room.vmf" } } ).HasValue() &&
		                 session.Document().Solids().size() == 6,
		    "reopened" );
	}

	// One undo step per command; history and jump.
	{
		(void)commands.Execute( "select_all", {} );
		const std::size_t before = session.History().Size();
		checks.That( commands.Execute( "move", { { "delta", "0 0 16" } } ).HasValue(),
		    "move the selection" );
		checks.Equal( session.History().Size(), before + 1, "one command, one history entry" );
		auto history = commands.Execute( "history", {} );
		checks.That( history && history.Value().find( "Move" ) != std::string::npos,
		    "history lists the label" );
		checks.That( commands.Execute( "jump", { { "position", "0" } } ).HasValue() &&
		                 session.History().Position() == 0,
		    "jump" );
		checks.That( commands.Execute( "redo", {} ).HasValue(), "redo" );
	}

	// Every family once.
	auto id = [&]( const std::string &text )
	{
		return text.substr( 0, text.find( ' ' ) );
	};
	{
		auto block = commands.Execute(
		    "create_primitive", { { "kind", "cylinder" }, { "mins", "300 0 0" },
		                            { "maxs", "364 64 64" }, { "sides", "12" } } );
		checks.That( block.HasValue(), "create a cylinder" );
		const std::string cyl = block.ValueOr( std::string() );
		auto describe = commands.Execute( "describe", { { "id", cyl } } );
		checks.That( describe && describe.Value().find( "sides=14" ) != std::string::npos,
		    "describe a 12-sided cylinder" );
		checks.That(
		    commands.Execute( "rotate", { { "axis", "2" }, { "degrees", "90" }, { "ids", cyl } } )
		        .HasValue(),
		    "rotate" );
		checks.That( commands.Execute( "mirror", { { "axis", "0" }, { "ids", cyl } } ).HasValue(),
		    "mirror" );
		checks.That( commands
		                 .Execute( "scale",
		                     { { "mins", "300 0 0" }, { "maxs", "428 64 64" }, { "ids", cyl } } )
		                 .HasValue(),
		    "scale to a box" );
		checks.That( commands
		                 .Execute( "clip", { { "normal", "1 0 0" }, { "point", "364 0 0" },
		                                       { "keep", "both" }, { "ids", cyl } } )
		                 .HasValue(),
		    "clip keep both" );
		auto arch = commands.Execute( "create_arch",
		    { { "mins", "-500 -500 0" }, { "maxs", "-300 -300 32" }, { "arc", "180" } } );
		checks.That( arch.HasValue(), "create an arch" );
		auto carver = commands.Execute(
		    "create_block", { { "mins", "320 16 16" }, { "maxs", "340 48 80" } } );
		checks.That( carver.HasValue() &&
		                 commands.Execute( "carve", { { "ids", carver.ValueOr( std::string() ) } } )
		                     .HasValue(),
		    "carve" );
		checks.That(
		    commands.Execute( "delete", { { "ids", carver.ValueOr( std::string() ) } } ).HasValue(),
		    "delete the carver" );

		auto box =
		    commands.Execute( "create_block", { { "mins", "0 400 0" }, { "maxs", "64 464 64" } } );
		const std::string b = box.ValueOr( std::string() );
		auto ray =
		    commands.Execute( "raycast", { { "origin", "32 432 500" }, { "dir", "0 0 -1" } } );
		checks.That( ray && ray.Value().find( "id=" + b + " " ) == 0 &&
		                 ray.Value().find( "normal=\"0 0 1\"" ) != std::string::npos,
		    "raycast hits the top of the new block" );
		const std::string side =
		    ray.ValueOr( std::string() ).substr( ray.ValueOr( std::string() ).find( "side=" ) + 5 );
		const std::string face = b + ":" + id( side );
		auto onTop = commands.Execute( "place_on_surface",
		    { { "classname", "light" }, { "origin", "32 432 500" }, { "dir", "0 0 -1" } } );
		checks.That( onTop.HasValue(), "place on a surface" );
		auto lamp = commands.Execute( "describe", { { "id", onTop.ValueOr( std::string() ) } } );
		checks.That( lamp && lamp.Value().find( "origin=\"32 432 65\"" ) != std::string::npos,
		    "one unit off the top face" );
		checks.That(
		    commands.Execute( "select_faces", { { "faces", face } } ).HasValue(), "select a face" );
		checks.That(
		    commands.Execute( "set_texture", { { "scale_u", "0.5" }, { "rotation", "45" } } )
		        .HasValue(),
		    "set texture values" );
		checks.That(
		    commands.Execute( "justify", { { "mode", "fit" } } ).HasValue(), "justify fit" );
		checks.That( commands.Execute( "align_texture", { { "mode", "face" } } ).HasValue(),
		    "align texture" );
		checks.That( commands.Execute( "apply_material", { { "material", "TOOLS/TOOLSNODRAW" } } )
		                 .HasValue(),
		    "apply material to the selected face" );
		checks.That(
		    commands.Execute( "push_face", { { "face", face }, { "distance", "32" } } ).HasValue(),
		    "push face" );
		auto extruded =
		    commands.Execute( "extrude_face", { { "face", face }, { "distance", "16" } } );
		checks.That( extruded.HasValue(), "extrude face" );
		checks.That( commands.Execute( "move_vertices",
		                         { { "id", b }, { "indices", "0" }, { "delta", "-8 0 0" } } )
		                     .HasValue() ||
		                 commands
		                     .Execute( "move_vertices",
		                         { { "id", b }, { "indices", "0" }, { "delta", "8 0 0" } } )
		                     .HasValue(),
		    "move a vertex outward" );
		auto replaced = commands.Execute( "replace_material",
		    { { "find", "TOOLS/TOOLSNODRAW" }, { "replace", "BRICK/BRICK01" } } );
		checks.That( replaced && replaced.Value() != "0", "replace material reports a count" );

		auto door =
		    commands.Execute( "tie_to_entity", { { "classname", "func_door" }, { "ids", b } } );
		checks.That( door.HasValue(), "tie to entity" );
		const std::string d = door.ValueOr( std::string() );
		checks.That(
		    commands.Execute( "set_flag", { { "flag", "256" }, { "on", "1" }, { "ids", d } } )
		        .HasValue(),
		    "spawnflag" );
		checks.That(
		    commands.Execute( "rename_entity", { { "id", d }, { "name", "door_a" } } ).HasValue(),
		    "rename" );
		checks.That(
		    commands
		        .Execute( "add_output", { { "output", "OnOpen" }, { "target", "door_a" },
		                                    { "input", "Close" }, { "delay", "2" }, { "ids", d } } )
		        .HasValue(),
		    "add output" );
		checks.That( commands.Execute( "remove_outputs", { { "target", "door_a" }, { "ids", d } } )
		                 .HasValue(),
		    "remove outputs" );
		checks.That(
		    commands.Execute( "set_class", { { "classname", "func_detail" }, { "ids", d } } )
		        .HasValue(),
		    "set class" );
		checks.That(
		    commands.Execute( "move_to_world", { { "ids", d } } ).HasValue(), "move to world" );
		auto grouped =
		    commands.Execute( "group", { { "ids", b + " " + extruded.ValueOr( std::string() ) } } );
		checks.That( grouped.HasValue() &&
		                 commands.Execute( "ungroup", { { "ids", grouped.Value() } } ).HasValue(),
		    "group, ungroup" );
		checks.That( commands.Execute( "hide", { { "ids", b } } ).HasValue() &&
		                 commands.Execute( "unhide_all", {} ).HasValue(),
		    "hide, unhide" );
		checks.That(
		    commands.Execute( "move", { { "delta", "3 0 0" }, { "ids", b } } ).HasValue() &&
		        commands.Execute( "snap", { { "ids", b } } ).HasValue(),
		    "snap an off-grid object" );
		auto aligned = commands.Execute( "snap", { { "ids", b } } );
		checks.That( !aligned && aligned.Error().detail.find( "nothing to do" ) == 0,
		    "snapping an aligned object is nothing to do (negative)" );
		checks.That(
		    commands.Execute( "set_world_key", { { "key", "skyname" }, { "value", "sky_black" } } )
		        .HasValue(),
		    "world key" );
		checks.That( commands.Execute( "select_class", { { "classname", "light*" } } ).HasValue() &&
		                 session.CurrentSelection().objects.size() == 2,
		    "select by class (the room light and the surface light)" );
		auto built = commands.Execute( "build_map",
		    { { "path", "maps/room.vmf" }, { "quality", "full" }, { "publish", "1" } } );
		checks.That(
		    built && built.Value() == "pass" && builder.last.fullQuality && builder.last.publish,
		    "build" );
	}

	// Clipboard, visgroups, cordons, displacements, map check.
	{
		(void)commands.Execute( "new_map", {} );
		auto block =
		    commands.Execute( "create_block", { { "mins", "0 0 0" }, { "maxs", "128 128 16" } } );
		const std::string blk = block.ValueOr( std::string() );
		checks.That( commands.Execute( "copy", {} ).HasValue(), "copy the selection" );
		auto pasted = commands.Execute(
		    "paste", { { "offset", "256 0 0" }, { "copies", "3" }, { "group", "1" } } );
		checks.That( pasted && session.CurrentSelection().objects.size() == 3 &&
		                 session.Document().Groups().size() == 3,
		    "paste special: three grouped copies, selected" );
		auto dup = commands.Execute( "duplicate", { { "ids", blk } } );
		checks.That( dup.HasValue() && session.Document().Solids().size() == 5, "duplicate" );
		checks.That(
		    commands.Execute( "cut", { { "ids", dup.ValueOr( std::string() ) } } ).HasValue() &&
		        session.Document().Solids().size() == 4,
		    "cut deletes" );
		checks.That(
		    commands.Execute( "paste", {} ).HasValue() && session.Document().Solids().size() == 5,
		    "paste the cut" );

		auto vg = commands.Execute( "visgroup_create", { { "name", "floors" } } );
		checks.That( vg.HasValue(), "create a visgroup" );
		const std::string v = vg.ValueOr( std::string() );
		checks.That(
		    commands.Execute( "visgroup_add", { { "visgroup", v }, { "ids", blk } } ).HasValue(),
		    "add to it" );
		checks.That(
		    commands.Execute( "visgroup_show", { { "visgroup", v }, { "on", "0" } } ).HasValue() &&
		        !session.Document()
		            .FindSolid(
		                commands.FromScriptId( static_cast<std::uint32_t>( std::stoul( blk ) ) ) )
		            ->editor.visgroupShown,
		    "hide the visgroup" );
		checks.That(
		    commands.Execute( "visgroup_rename", { { "visgroup", v }, { "name", "ground" } } )
		        .HasValue(),
		    "rename it" );
		checks.That(
		    commands.Execute( "visgroup_delete", { { "visgroup", v } } ).HasValue(), "delete it" );

		auto cordon = commands.Execute(
		    "cordon_add", { { "mins", "-512 -512 -512" }, { "maxs", "512 512 512" } } );
		checks.That(
		    cordon && cordon.Value() == "0" && session.Document().Settings().cordons.size() == 1,
		    "add a cordon" );
		checks.That( commands.Execute( "cordons_enabled", { { "on", "1" } } ).HasValue() &&
		                 session.Document().Settings().cordonsActive,
		    "enable cordons" );
		checks.That(
		    !commands.Execute( "cordon_box",
		        { { "index", "0" }, { "box", "0" }, { "mins", "1 1 1" }, { "maxs", "0 0 0" } } ),
		    "an inverted cordon box (negative)" );

		auto ray =
		    commands.Execute( "raycast", { { "origin", "64 64 100" }, { "dir", "0 0 -1" } } );
		const std::string r = ray.ValueOr( std::string() );
		const std::string side = r.substr(
		    r.find( "side=" ) + 5, r.find( ' ', r.find( "side=" ) ) - r.find( "side=" ) - 5 );
		const std::string top = r.substr( 3, r.find( ' ' ) - 3 ) + ":" + side;
		checks.That(
		    commands.Execute( "create_displacement", { { "power", "2" }, { "faces", top } } )
		        .HasValue(),
		    "make the top a displacement" );
		checks.That( commands
		                 .Execute( "sculpt", { { "center", "64 64 16" }, { "radius", "40" },
		                                         { "amount", "8" }, { "faces", top } } )
		                 .HasValue(),
		    "sculpt it" );
		checks.That( commands
		                 .Execute( "paint_alpha", { { "center", "64 64 24" }, { "radius", "40" },
		                                              { "value", "255" }, { "faces", top } } )
		                 .HasValue(),
		    "paint alpha" );
		checks.That(
		    commands.Execute( "displacement_power", { { "power", "3" }, { "faces", top } } )
		        .HasValue(),
		    "resample" );
		checks.That( !commands.Execute(
		                 "sculpt", { { "center", "0 0 0" }, { "radius", "1" }, { "amount", "1" },
		                               { "mode", "melt" }, { "faces", top } } ),
		    "unknown sculpt mode (negative)" );

		auto problems = commands.Execute( "check_map", {} );
		checks.That(
		    problems && problems.Value().find( "fixable" ) != std::string::npos, "check the map" );
		(void)commands.Execute( "fix_all", {} );
		auto after = commands.Execute( "check_map", {} );
		checks.That( after && after.Value().find( "fixable=1" ) == std::string::npos,
		    "fix_all leaves nothing fixable" );

		EditSession other;
		EditorSettings otherSettings;
		SessionCommands noClipboard( other, otherSettings, {} );
		checks.That(
		    !noClipboard.Execute( "copy", {} ), "no clipboard in the composition (negative)" );
	}

	// Settings.
	{
		checks.That( commands.Execute( "set_grid", { { "size", "16" } } ).HasValue() &&
		                 settings.gridSize == 16,
		    "grid" );
		checks.That( !commands.Execute( "set_grid", { { "size", "12" } } ),
		    "grid must be a power of two (negative)" );
		checks.That( commands.Execute( "set_granularity", { { "mode", "solids" } } ).HasValue() &&
		                 settings.granularity == SelectionGranularity::Solids,
		    "granularity" );
		checks.That( commands.Execute( "set_texture_lock", { { "on", "0" } } ).HasValue() &&
		                 !settings.textureLock,
		    "texture lock" );
		checks.That( !commands.Execute( "set_entity_class", { { "classname", "nope" } } ),
		    "unknown class (negative)" );
	}

	// Errors.
	{
		auto unknown = commands.Execute( "fly", {} );
		checks.That( !unknown && unknown.Error().status == CommandStatus::UnknownCommand,
		    "unknown command (negative)" );
		auto missing = commands.Execute( "create_block", { { "mins", "0 0 0" } } );
		checks.That( !missing && missing.Error().status == CommandStatus::MissingArgument,
		    "missing argument (negative)" );
		auto extra = commands.Execute( "undo", { { "force", "1" } } );
		checks.That( !extra && extra.Error().status == CommandStatus::InvalidArgument,
		    "undeclared argument (negative)" );
		auto vec = commands.Execute( "move", { { "delta", "1 2" } } );
		checks.That(
		    !vec && vec.Error().status == CommandStatus::InvalidArgument, "bad vector (negative)" );
		auto num = commands.Execute( "rotate", { { "axis", "z" }, { "degrees", "90" } } );
		checks.That( !num && num.Error().status == CommandStatus::InvalidArgument,
		    "bad integer (negative)" );
		auto noid = commands.Execute( "describe", { { "id", "999999" } } );
		checks.That(
		    !noid && noid.Error().status == CommandStatus::Rejected, "unknown id (negative)" );
		auto badid = commands.Execute( "describe", { { "id", "x" } } );
		checks.That( !badid && badid.Error().status == CommandStatus::InvalidArgument,
		    "malformed id (negative)" );
		auto badface = commands.Execute( "select_faces", { { "faces", "3-4" } } );
		checks.That( !badface && badface.Error().status == CommandStatus::InvalidArgument,
		    "malformed face (negative)" );
		auto degenerate =
		    commands.Execute( "create_block", { { "mins", "0 0 0" }, { "maxs", "0 64 64" } } );
		checks.That( !degenerate && degenerate.Error().status == CommandStatus::Rejected,
		    "degenerate block (negative)" );

		EditSession other;
		EditorSettings otherSettings;
		SessionCommands bare( other, otherSettings, {} );
		checks.That( !bare.Execute( "save", { { "path", "x.vmf" } } ),
		    "no codec in the composition (negative)" );
		checks.That( !bare.Execute( "build_map", { { "path", "x.vmf" } } ),
		    "no builder in the composition (negative)" );

		auto script =
		    run( "new_map\ncreate_block mins=\"0 0 0\" maxs=\"8 8 8\"\nundo\nundo\nundo\n" );
		checks.That(
		    !script && script.Error().line == 4, "a failing script line is reported (negative)" );
	}

	return checks.Report();
}
