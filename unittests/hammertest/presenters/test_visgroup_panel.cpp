//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters visgroup panel (RFC 0002, R08 domain logic) over
//			a real EditSession and SessionCommands: the tree in pre-order with
//			depths and parents, recursive and direct member counts, the
//			selected-members state, visibility states through hide/show/toggle
//			(one undo step each), rename, reparent, delete, "select members",
//			expansion surviving rebuilds. Negative checks: empty visgroups
//			cannot be toggled or marked, unknown ids refused with nothing
//			changed, replacement clears the tree, destruction after the session.
//
//=============================================================================//

#include "hammer/presenters/visgroup_panel.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/app/session_commands.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <cstdlib>
#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::SelectMode;
using app::ops::VisgroupState;
using mapgeometry::Vec3d;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "dev/dev_measuregeneric01b";
	scene::MapDocument doc;
	ObjectId a, b, c;
	{
		scene::DocumentEdit edit( doc );
		a = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, tex ) );
		b = edit.Add( scene::MakeBoxSolid( { Vec3d( 16, 0, 0 ), Vec3d( 24, 8, 8 ) }, tex ) );
		c = edit.Add( scene::MakeBoxSolid( { Vec3d( 32, 0, 0 ), Vec3d( 40, 8, 8 ) }, tex ) );
		scene::CommitEdit( doc, edit );
	}

	auto session = std::make_unique<app::EditSession>( doc );
	app::EditorSettings settings;
	app::SessionCommands commands( *session, settings, app::SessionServices{} );
	auto panel = std::make_unique<VisgroupPanel>( *session, commands );
	checks.That( panel->Rows().empty(), "no visgroups" );

	auto walls = panel->Create( "Walls" );
	checks.That( walls.HasValue(), "create" );
	const int wallsId = walls ? std::atoi( walls.Value().c_str() ) : 0;
	auto inner = panel->Create( "Inner", wallsId );
	const int innerId = inner ? std::atoi( inner.Value().c_str() ) : 0;
	checks.That( inner.HasValue() && innerId != wallsId, "create nested" );
	{
		const auto &rows = panel->Rows();
		checks.That( rows.size() == 2 && rows[0].name == "Walls" && rows[0].childCount == 1 &&
		                 rows[1].depth == 1 && rows[1].parent == std::optional<std::size_t>( 0 ) &&
		                 rows[0].visibility == VisgroupState::Empty,
		    "tree with depths" );
	}
	checks.That( !panel->ToggleVisible( wallsId ) && !panel->SelectMembers( wallsId ),
	    "an empty visgroup cannot be toggled or marked (negative)" );

	// Membership.
	checks.That( session->SelectObjects( { a, b }, SelectMode::Replace ).HasValue() &&
	                 panel->AddSelection( wallsId ).HasValue(),
	    "add the selection" );
	checks.That( session->SelectObjects( { c }, SelectMode::Replace ).HasValue() &&
	                 panel->AddSelection( innerId ).HasValue(),
	    "add to the nested group" );
	{
		const auto &rows = panel->Rows();
		checks.That(
		    rows[0].memberCount == 3 && rows[0].directMemberCount == 2 && rows[1].memberCount == 1,
		    "recursive and direct counts" );
		checks.That(
		    rows[0].selected == SelectedMembers::Some && rows[1].selected == SelectedMembers::All,
		    "selected-members state" );
		checks.That( rows[0].visibility == VisgroupState::Shown, "shown" );
	}

	// Visibility.
	const std::size_t position = session->History().Position();
	checks.That( panel->SetVisible( innerId, false ).HasValue(), "hide nested" );
	checks.That( session->History().Position() == position + 1 &&
	                 panel->Rows()[0].visibility == VisgroupState::Mixed &&
	                 panel->Rows()[1].visibility == VisgroupState::Hidden &&
	                 !scene::IsVisible( session->Document(), c ),
	    "one step; parent Mixed" );
	checks.That( panel->ToggleVisible( wallsId ).HasValue() &&
	                 panel->Rows()[0].visibility == VisgroupState::Shown,
	    "toggling a Mixed group shows it" );
	checks.That( panel->ToggleVisible( wallsId ).HasValue() &&
	                 panel->Rows()[0].visibility == VisgroupState::Hidden &&
	                 !scene::IsVisible( session->Document(), a ),
	    "toggling a Shown group hides it" );
	checks.That(
	    session->Undo().HasValue() && panel->Rows()[0].visibility == VisgroupState::Shown, "undo" );

	// Select members, rename, move, delete.
	checks.That( panel->SelectMembers( wallsId ).HasValue() &&
	                 session->CurrentSelection().objects == std::vector<ObjectId>{ a, b, c },
	    "select members (recursive)" );
	checks.That( panel->Rows()[0].selected == SelectedMembers::All, "all selected" );
	checks.That(
	    panel->Rename( innerId, "Trim" ).HasValue() && panel->Rows()[1].name == "Trim", "rename" );
	panel->SetExpanded( wallsId, false );
	checks.That( panel->Move( innerId, 0 ).HasValue() && panel->Rows().size() == 2 &&
	                 panel->Rows()[1].depth == 0 && panel->Rows()[0].childCount == 0,
	    "reparent to the top level" );
	checks.That(
	    !panel->IsExpanded( wallsId ) && !panel->Rows()[0].expanded && panel->IsExpanded( innerId ),
	    "expansion survives rebuilds" );

	const std::uint64_t revision = session->Revision();
	checks.That(
	    !panel->Rename( 999, "x" ) && !panel->SetVisible( 999, true ) && !panel->Delete( 999 ),
	    "unknown ids refused (negative)" );
	checks.Equal( session->Revision(), revision, "nothing changed" );

	checks.That( panel->Delete( innerId ).HasValue() && panel->Rows().size() == 1 &&
	                 panel->Rows()[0].memberCount == 2,
	    "delete keeps members out of the tree" );
	checks.That( session->SelectObjects( { a }, SelectMode::Replace ).HasValue() &&
	                 panel->RemoveSelection( wallsId ).HasValue() &&
	                 panel->Rows()[0].memberCount == 1,
	    "remove the selection" );

	// A new visgroup from the selection: one step, followed through undo/redo.
	checks.That( session->SelectObjects( { b, c }, SelectMode::Replace ).HasValue() &&
	                 panel->SelectionVisgroupName() == "2 objects",
	    "legacy default name for the selection" );
	const std::size_t beforeNew = session->History().Position();
	auto picked = panel->CreateFromSelection( panel->SelectionVisgroupName(), wallsId );
	const int pickedId = picked ? std::atoi( picked.Value().c_str() ) : 0;
	{
		const std::optional<std::size_t> row = panel->FindRow( pickedId );
		checks.That( picked.HasValue() && session->History().Position() == beforeNew + 1 && row &&
		                 panel->Rows()[*row].name == "2 objects" &&
		                 panel->Rows()[*row].depth == 1 &&
		                 panel->Rows()[*row].directMemberCount == 2 &&
		                 panel->Rows()[*row].selected == SelectedMembers::All,
		    "create from the selection: nested, holding the selection, one step" );
	}
	checks.That( panel->MoveTargets( wallsId ) == std::vector<int>{} &&
	                 panel->MoveTargets( pickedId ) == std::vector<int>{ wallsId },
	    "move targets exclude the visgroup and its descendants" );
	checks.That( session->Undo().HasValue() && !panel->FindRow( pickedId ) &&
	                 session->Redo().HasValue() && panel->FindRow( pickedId ),
	    "rows follow undo and redo" );
	(void)session->Undo();
	checks.That( session->SelectObjects( {}, SelectMode::Replace ).HasValue() &&
	                 !panel->CreateFromSelection( "none" ) && panel->Rows().size() == 1,
	    "an empty selection makes no visgroup (negative)" );

	checks.That(
	    session->Replace( doc ).HasValue() && panel->Rows().empty() && panel->IsExpanded( wallsId ),
	    "replacement clears the tree and expansion" );

	session.reset();
	panel.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
