//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters action catalog (RFC 0002, R08 domain logic) over
//			a real EditSession and SessionCommands: catalog consistency (every
//			command action names a SessionCommands::Catalog() entry, passes only
//			arguments it declares and every argument it requires), unique ids
//			and chords, canonical shortcuts, chord normalization and lookup,
//			categories, enabled states following selection, history and the
//			clipboard, checked toggles, executing actions through the session
//			as one undo step, the Undo label, tool and host targets, status
//			messages. Negative checks: disabled actions run nothing, unknown ids,
//			missing host arguments, failing commands, malformed chords.
//
//=============================================================================//

#include "hammer/presenters/action_catalog.h"

#include "hammer/app/clipboard.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <algorithm>
#include <memory>
#include <set>

using namespace hammer;
using namespace hammer::presenters;
using app::SelectMode;
using mapgeometry::Vec3d;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	// --- Catalog consistency ------------------------------------------------------------
	const auto &commandCatalog = app::SessionCommands::Catalog();
	auto findCommand = [&]( const std::string &name ) -> const app::CommandInfo *
	{
		for ( const app::CommandInfo &info : commandCatalog )
		{
			if ( info.name == name )
			{
				return &info;
			}
		}
		return nullptr;
	};
	std::set<std::string> ids;
	std::set<std::string> chords;
	bool idsUnique = true, chordsUnique = true, canonical = true, commandsExist = true;
	bool argsDeclared = true, requiredProvided = true, categoriesKnown = true, targetsFilled = true;
	for ( const ActionSpec &spec : ActionCatalog::Specs() )
	{
		idsUnique = ids.insert( spec.id ).second && idsUnique;
		for ( const std::string &shortcut : spec.shortcuts )
		{
			chordsUnique = chords.insert( shortcut ).second && chordsUnique;
			const std::optional<std::string> normalized = NormalizeChord( shortcut );
			if ( !normalized || *normalized != shortcut )
			{
				canonical = false;
				std::printf( "not canonical: %s\n", shortcut.c_str() );
			}
		}
		const auto &cats = ActionCatalog::Categories();
		categoriesKnown =
		    categoriesKnown && std::find( cats.begin(), cats.end(), spec.category ) != cats.end();
		if ( spec.target == ActionTarget::Tool )
		{
			targetsFilled = targetsFilled && !spec.tool.empty() && spec.command.empty();
			continue;
		}
		if ( spec.target == ActionTarget::Host )
		{
			targetsFilled = targetsFilled && !spec.host.empty() && spec.command.empty();
			continue;
		}
		const app::CommandInfo *info = findCommand( spec.command );
		if ( !info )
		{
			commandsExist = false;
			std::printf( "missing command: %s (%s)\n", spec.command.c_str(), spec.id.c_str() );
			continue;
		}
		std::set<std::string> passed;
		for ( const auto &arg : spec.args )
		{
			passed.insert( arg.first );
		}
		passed.insert( spec.hostArgs.begin(), spec.hostArgs.end() );
		if ( spec.argRule == ArgRule::Toggle )
		{
			passed.insert( spec.toggleArg );
		}
		if ( spec.argRule == ArgRule::GridHigher || spec.argRule == ArgRule::GridLower )
		{
			passed.insert( "size" );
		}
		for ( const std::string &name : passed )
		{
			const bool declared = std::find( info->required.begin(), info->required.end(), name ) !=
			                          info->required.end() ||
			                      std::find( info->optional.begin(), info->optional.end(), name ) !=
			                          info->optional.end();
			if ( !declared )
			{
				argsDeclared = false;
				std::printf( "undeclared argument %s of %s\n", name.c_str(), spec.id.c_str() );
			}
		}
		for ( const std::string &name : info->required )
		{
			if ( !passed.count( name ) )
			{
				requiredProvided = false;
				std::printf( "unprovided argument %s of %s\n", name.c_str(), spec.id.c_str() );
			}
		}
	}
	checks.That( ActionCatalog::Specs().size() >= 40, "the catalog is populated" );
	checks.That( idsUnique, "unique ids" );
	checks.That( chordsUnique, "unique shortcuts" );
	checks.That( canonical, "shortcuts are stored canonical" );
	checks.That( commandsExist, "every command action names a SessionCommands command" );
	checks.That( argsDeclared, "actions pass only declared arguments" );
	checks.That( requiredProvided, "actions provide every required argument" );
	checks.That( categoriesKnown && targetsFilled, "known categories; tool/host targets filled" );
	checks.That( ActionCatalog::InCategory( "Edit" ).front()->id == "edit.undo" &&
	                 ActionCatalog::InCategory( "Nope" ).empty(),
	    "list by category in menu order" );

	// --- Chords --------------------------------------------------------------------------
	checks.Equal( NormalizeChord( "shift+ctrl+w" ).value_or( "" ), std::string( "Ctrl+Shift+W" ),
	    "modifier order and case" );
	checks.Equal( NormalizeChord( "Control + Del" ).value_or( "" ), std::string( "Ctrl+Delete" ),
	    "aliases and spaces" );
	checks.Equal( NormalizeChord( "f9" ).value_or( "" ), std::string( "F9" ), "function keys" );
	checks.Equal( NormalizeChord( "Ctrl++" ).value_or( "" ), std::string( "Ctrl++" ), "plus key" );
	checks.That(
	    !NormalizeChord( "" ) && !NormalizeChord( "Ctrl+Shift" ) && !NormalizeChord( "A+B" ),
	    "malformed chords (negative)" );
	checks.That( ActionCatalog::FindByShortcut( "b+SHIFT" ) &&
	                 ActionCatalog::FindByShortcut( "b+SHIFT" )->id == "tools.block",
	    "lookup by any spelling" );
	checks.That( ActionCatalog::FindByShortcut( "shift+del" )->id == "edit.cut" &&
	                 ActionCatalog::FindByShortcut( "Ctrl+H" )->id == "view.hide_unselected" &&
	                 ActionCatalog::FindByShortcut( "F" )->id == "tools.hollow" &&
	                 ActionCatalog::FindByShortcut( "Ctrl+D" )->id == "edit.duplicate",
	    "legacy alternates and resolved conflicts" );
	checks.That(
	    !ActionCatalog::FindByShortcut( "Ctrl+Alt+Shift+K" ) && !ActionCatalog::Find( "no.such" ),
	    "unbound chord and unknown id (negative)" );

	// --- Session ---------------------------------------------------------------------------
	scene::FaceTexture tex;
	tex.material = "dev/dev_measuregeneric01b";
	scene::MapDocument doc;
	ObjectId a, b;
	{
		scene::DocumentEdit edit( doc );
		a = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, tex ) );
		b = edit.Add( scene::MakeBoxSolid( { Vec3d( 128, 0, 0 ), Vec3d( 192, 64, 64 ) }, tex ) );
		scene::CommitEdit( doc, edit );
	}
	auto session = std::make_unique<app::EditSession>( doc );
	app::EditorSettings settings;
	app::MapFragment clipboard;
	app::SessionServices services;
	services.clipboard = &clipboard;
	app::SessionCommands commands( *session, settings, services );
	auto actions = std::make_unique<ActionCatalog>( *session, settings, commands, &clipboard );

	checks.That( !actions->IsEnabled( "edit.undo" ) && !actions->IsEnabled( "edit.copy" ) &&
	                 !actions->IsEnabled( "edit.paste" ) && !actions->IsEnabled( "tools.group" ) &&
	                 actions->IsEnabled( "edit.select_all" ) &&
	                 !actions->IsEnabled( "view.unhide_all" ),
	    "enabled states with nothing selected" );
	{
		const std::uint64_t revision = session->Revision();
		auto r = actions->Execute( "edit.undo" );
		checks.That(
		    !r && r.Error().kind == ActionErrorKind::Disabled && session->Revision() == revision,
		    "a disabled action runs nothing (negative)" );
		checks.That( actions->LastMessage() == "Undo is not available now", "and says so" );
	}
	checks.Equal( actions->DisplayLabel( "edit.undo" ), std::string( "Undo" ), "plain undo label" );

	checks.That( session->SelectObjects( { a, b }, SelectMode::Replace ).HasValue(), "select" );
	checks.That( actions->IsEnabled( "edit.copy" ) && actions->IsEnabled( "tools.group" ) &&
	                 actions->IsEnabled( "tools.tie_to_entity" ) &&
	                 !actions->IsEnabled( "tools.ungroup" ) &&
	                 !actions->IsEnabled( "tools.move_to_world" ),
	    "enabled states follow the selection" );

	// Execute through the session: one undo step each.
	const std::size_t position = session->History().Position();
	auto grouped = actions->Execute( "tools.group" );
	checks.That( grouped.HasValue() && grouped.Value().target == ActionTarget::Command &&
	                 session->History().Position() == position + 1,
	    "group is one step" );
	checks.That( actions->UndoLabel() == "Undo Group" &&
	                 actions->DisplayLabel( "edit.undo" ) == "Undo Group" &&
	                 actions->IsEnabled( "edit.undo" ) && actions->IsEnabled( "tools.ungroup" ),
	    "undo label and states after the edit" );
	checks.That( actions->LastMessage().rfind( "Group", 0 ) == 0, "status message" );
	checks.That( actions->Execute( "edit.undo" ).HasValue() &&
	                 session->History().Position() == position &&
	                 actions->RedoLabel() == "Redo Group" && actions->IsEnabled( "edit.redo" ),
	    "undo through the action" );
	checks.That( actions->LastMessage() == "Undo Group", "undo message names the entry" );

	// Clipboard.
	checks.That( actions->Execute( "edit.copy" ).HasValue() && actions->IsEnabled( "edit.paste" ) &&
	                 actions->LastMessage() == "Copy: 2",
	    "copy fills the clipboard" );
	const std::size_t solids = session->Document().Solids().size();
	checks.That( actions->Execute( "edit.paste" ).HasValue() &&
	                 session->Document().Solids().size() == solids + 2,
	    "paste adds the copies" );

	// Hide / unhide.
	checks.That( actions->Execute( "view.hide" ).HasValue() &&
	                 actions->IsEnabled( "view.unhide_all" ) &&
	                 actions->Execute( "view.unhide_all" ).HasValue() &&
	                 !actions->IsEnabled( "view.unhide_all" ),
	    "hide then unhide" );

	// Toggles and rule-derived arguments.
	checks.That(
	    actions->IsChecked( "tools.texture_lock" ) == std::optional<bool>( true ), "lock checked" );
	checks.That( actions->Execute( "tools.texture_lock" ).HasValue() && !settings.textureLock &&
	                 actions->IsChecked( "tools.texture_lock" ) == std::optional<bool>( false ),
	    "toggle off" );
	checks.That( actions->Execute( "tools.ignore_groups" ).HasValue() &&
	                 settings.granularity == app::SelectionGranularity::Objects &&
	                 actions->IsChecked( "tools.select_objects" ) == std::optional<bool>( true ),
	    "ignore groups and the radio states" );
	checks.That( actions->Execute( "tools.select_solids" ).HasValue() &&
	                 actions->IsChecked( "tools.ignore_groups" ) == std::optional<bool>( true ) &&
	                 actions->IsChecked( "tools.select_groups" ) == std::optional<bool>( false ),
	    "granularity radio" );
	checks.That( !actions->IsChecked( "edit.copy" ), "not a toggle" );
	checks.That( actions->Execute( "view.grid_higher" ).HasValue() && settings.gridSize == 128,
	    "grid higher" );
	settings.gridSize = 1;
	checks.That( !actions->IsEnabled( "view.grid_lower" ) && !actions->Execute( "view.grid_lower" ),
	    "grid lower at the minimum is disabled (negative)" );

	// Tool and host targets.
	auto block = actions->Execute( "tools.block" );
	checks.That( block.HasValue() && block.Value().target == ActionTarget::Tool &&
	                 block.Value().tool == "block",
	    "tool actions name the tool" );
	auto snap = actions->Execute( "view.snap_to_grid" );
	checks.That( snap.HasValue() && snap.Value().target == ActionTarget::Host &&
	                 snap.Value().host == "snap_to_grid" &&
	                 actions->IsChecked( "view.snap_to_grid" ) == std::optional<bool>( true ),
	    "host actions name the host action" );

	// Host arguments and failing commands.
	{
		auto r = actions->Execute( "file.save" );
		checks.That(
		    !r && r.Error().kind == ActionErrorKind::MissingArgument, "missing path (negative)" );
		auto s = actions->Execute( "file.save", { { "path", "maps/a.vmf" } } );
		checks.That( !s && s.Error().kind == ActionErrorKind::CommandFailed && s.Error().command &&
		                 actions->LastMessage().rfind( "Save: ", 0 ) == 0,
		    "a failing command reports through the action (no file store)" );
		auto build = actions->Execute( "map.build_and_run", { { "path", "maps/a.vmf" } } );
		checks.That( !build && ActionCatalog::Find( "map.build_and_run" )->runAfter,
		    "build and run carries runAfter; fails without a builder" );
		auto unknown = actions->Execute( "nope" );
		checks.That(
		    !unknown && unknown.Error().kind == ActionErrorKind::UnknownAction, "unknown id" );
	}

	const std::uint64_t revision = actions->Revision();
	checks.That( session->ClearSelection().HasValue() && actions->Revision() > revision,
	    "revision follows the session" );

	session.reset();
	actions.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
