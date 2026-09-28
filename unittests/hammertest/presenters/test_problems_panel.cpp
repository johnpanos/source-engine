//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.presenters problems panel (RFC 0002, R08 domain logic) over
//			a real EditSession: rows in CheckMap order with code names,
//			severities and object labels, counts by severity, revision-keyed
//			rescans (edits, undo and replacement rescan; selection changes and
//			saves do not), go to, one fix and fix-all as one labeled undo step
//			each. Negative checks: an unfixable problem and out-of-range rows
//			refuse and change nothing, go to on a map-wide problem is Nothing,
//			fix-all with nothing fixable, destruction after the session.
//
//=============================================================================//

#include "hammer/presenters/problems_panel.h"

#include "hammer/app/edit_session.h"
#include "testing/checks.h"

#include <memory>

using namespace hammer;
using namespace hammer::presenters;
using app::EditErrorCode;
using app::MapProblem;
using scene::ObjectId;

int main()
{
	testing::Checks checks;

	scene::MapDocument doc;
	ObjectId relay, dup, group;
	{
		scene::DocumentEdit edit( doc );
		scene::Entity r;
		r.classname = "logic_relay";
		r.SetKey( "targetname", "relay" );
		r.connections.push_back( *scene::ParseConnection( "OnTrigger", "nobody,Kill,,0,-1" ) );
		relay = edit.Add( r );
		scene::Entity d;
		d.classname = "info_target";
		d.keys.push_back( { "message", "first" } );
		d.keys.push_back( { "message", "second" } );
		dup = edit.Add( d );
		group = edit.Add( scene::Group{} );
		scene::CommitEdit( doc, edit );
	}

	auto session = std::make_unique<app::EditSession>( doc );
	auto panel = std::make_unique<ProblemsPanel>( *session, nullptr, nullptr );

	const auto &rows = panel->Rows();
	checks.Equal( rows.size(), std::size_t( 4 ), "four problems" );
	checks.That( rows.size() == 4 && rows[0].codeName == "no-player-start" &&
	                 rows[0].severityName == "error" && !rows[0].problem.fixable &&
	                 rows[0].problem.objects.empty(),
	    "map-wide error first" );
	checks.That(
	    rows.size() == 4 && rows[1].problem.code == MapProblem::Code::ConnectionMissingTarget &&
	        rows[1].objectLabels == std::vector<std::string>{ "relay" } && rows[1].problem.fixable,
	    "a missing connection target with its object label" );
	checks.That( rows.size() == 4 && rows[2].problem.code == MapProblem::Code::DuplicateKeys &&
	                 rows[3].problem.code == MapProblem::Code::EmptyGroup &&
	                 rows[3].severityName == "warning",
	    "CheckMap order" );
	checks.That(
	    panel->ErrorCount() == 1 && panel->WarningCount() == 3 && panel->FixableCount() == 3,
	    "counts by severity" );

	// Go to.
	const std::size_t scans = panel->ScanCount();
	checks.That( panel->GoTo( 1 ).HasValue() &&
	                 session->CurrentSelection().objects == std::vector<ObjectId>{ relay },
	    "go to selects the objects" );
	checks.Equal( panel->ScanCount(), scans, "a selection change does not rescan" );
	checks.That( panel->GoTo( 0 ).Error().code == EditErrorCode::Nothing,
	    "a map-wide problem has nothing to select (negative)" );
	session->MarkSaved();
	checks.Equal( panel->ScanCount(), scans, "a save does not rescan" );

	// Negative: refusals change nothing.
	const std::uint64_t revision = session->Revision();
	checks.That( panel->Fix( 0 ).Error().code == EditErrorCode::Rejected, "no fix (negative)" );
	checks.That( !panel->Fix( 99 ) && !panel->GoTo( 99 ), "out of range (negative)" );
	checks.That( session->Revision() == revision && panel->Rows().size() == 4, "nothing changed" );

	// One fix.
	checks.That( panel->Fix( 2 ).HasValue(), "fix duplicate keys" );
	checks.That( session->History().UndoEntry()->label == "Fix duplicate-keys" &&
	                 session->Document().FindEntity( dup )->keys.size() == 1 &&
	                 *session->Document().FindEntity( dup )->Key( "message" ) == "first",
	    "one labeled step keeping the first key" );
	checks.That( panel->Rows().size() == 3 && panel->ScanCount() == scans + 1 &&
	                 panel->CheckedRevision() == session->Revision(),
	    "an edit rescans" );
	checks.That( session->Undo().HasValue() && panel->Rows().size() == 4, "undo rescans" );

	// Fix all.
	const std::size_t position = session->History().Position();
	checks.That( panel->FixAll().HasValue(), "fix all" );
	checks.That( session->History().Position() == position + 1 &&
	                 session->History().UndoEntry()->label == "Fix all problems",
	    "one step" );
	checks.That( panel->Rows().size() == 1 && panel->FixableCount() == 0 &&
	                 panel->ErrorCount() == 1 && !session->Document().FindGroup( group ),
	    "only the unfixable problem remains" );
	checks.That(
	    panel->FixAll().Error().code == EditErrorCode::Nothing, "nothing fixable left (negative)" );

	// Replacement rescans.
	checks.That( session->Replace( scene::MapDocument( 1 ) ).HasValue() &&
	                 panel->Rows().size() == 1 && panel->Rows()[0].codeName == "no-player-start",
	    "replacement rescans" );

	session.reset();
	panel.reset();
	checks.That( true, "destroyed after the session" );
	return checks.Report();
}
