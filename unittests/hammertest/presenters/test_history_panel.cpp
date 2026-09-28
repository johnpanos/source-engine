//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters history panel (RFC 0002, R08 domain logic) over a
//			real EditSession: rows with positions and done/current flags, the
//			Undo/Redo labels, rows after undo (the redo tail stays, not done),
//			jumps in both directions, the redo tail dropped by a new edit, the
//			modified flag across save. Negative checks: jumps out of range and
//			undo with nothing to undo refuse and change nothing, selection-only
//			changes leave the model alone, replacement clears it, destruction
//			after the session.
//
//=============================================================================//

#include "hammer/presenters/history_panel.h"

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/entity_ops.h"
#include "testing/checks.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::EditErrorCode;

int main()
{
	testing::Checks checks;

	scene::MapDocument doc;
	scene::ObjectId lamp;
	{
		scene::DocumentEdit edit( doc );
		scene::Entity e;
		e.classname = "light";
		lamp = edit.Add( e );
		scene::CommitEdit( doc, edit );
	}
	auto session = std::make_unique<app::EditSession>( doc );
	auto panel = std::make_unique<HistoryPanel>( *session );

	checks.That( panel->Rows().empty() && !panel->CanUndo() && !panel->CanRedo() &&
	                 panel->UndoLabel() == "Undo" && panel->RedoLabel() == "Redo" &&
	                 !panel->IsModified(),
	    "empty history" );
	checks.That(
	    panel->Undo().Error().code == EditErrorCode::Nothing, "nothing to undo (negative)" );

	auto edit = [&]( const std::string &label, const std::string &value )
	{
		return session
		    ->Execute( label,
		        [&]( scene::DocumentEdit &e )
		        {
			        return app::ops::SetKey( e, { lamp }, "k", value );
		        } )
		    .HasValue();
	};
	checks.That(
	    edit( "Move", "1" ) && edit( "Rotate", "2" ) && edit( "Scale", "3" ), "three edits" );
	{
		const auto &rows = panel->Rows();
		checks.That( rows.size() == 3 && rows[0].label == "Move" && rows[0].position == 1 &&
		                 rows[2].position == 3 && rows[0].done && rows[2].done && rows[2].current &&
		                 !rows[1].current,
		    "rows with positions" );
	}
	checks.That( panel->Position() == 3 && panel->UndoLabel() == "Undo Scale" &&
	                 !panel->CanRedo() && panel->IsModified(),
	    "labels and modified" );

	checks.That( panel->Undo().HasValue(), "undo" );
	{
		const auto &rows = panel->Rows();
		checks.That( rows.size() == 3 && !rows[2].done && rows[1].done && rows[1].current &&
		                 panel->Position() == 2,
		    "rows after undo: the tail stays, not done" );
	}
	checks.That( panel->UndoLabel() == "Undo Rotate" && panel->RedoLabel() == "Redo Scale",
	    "labels after undo" );

	checks.That( panel->JumpTo( 0 ).HasValue() && panel->Position() == 0 &&
	                 !panel->Rows()[0].done && panel->RedoLabel() == "Redo Move" &&
	                 !panel->IsModified(),
	    "jump to the start" );
	checks.That( panel->JumpTo( 3 ).HasValue() && panel->Position() == 3 &&
	                 panel->Rows()[2].current &&
	                 *session->Document().FindEntity( lamp )->Key( "k" ) == "3",
	    "jump to the end" );

	const std::uint64_t revision = panel->Revision();
	checks.That(
	    panel->JumpTo( 4 ).Error().code == EditErrorCode::Rejected && panel->Position() == 3,
	    "out of range refused (negative)" );
	checks.That( session->SelectObjects( { lamp }, app::SelectMode::Replace ).HasValue() &&
	                 panel->Revision() == revision,
	    "selection-only changes leave the model alone" );

	session->MarkSaved();
	checks.That( !panel->IsModified() && panel->Revision() > revision, "save clears modified" );
	checks.That(
	    panel->JumpTo( 1 ).HasValue() && edit( "Rename", "x" ), "undo twice, then a new edit" );
	checks.That( panel->Rows().size() == 2 && panel->Rows()[1].label == "Rename" &&
	                 !panel->CanRedo() && panel->IsModified(),
	    "a new edit drops the redo tail" );

	checks.That(
	    session->Replace( doc ).HasValue() && panel->Rows().empty() && panel->Position() == 0,
	    "replacement clears the history" );

	session.reset();
	panel.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
