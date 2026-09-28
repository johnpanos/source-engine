//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app EditSession conformance (RFC 0002 "Edit transactions and
//			history", "State ownership and lifetime"): commit, atomic failure,
//			invariant rejection, no-op edits, undo/redo with selection restore,
//			history jumps, the modified flag, published events, selection
//			guards (draft policy) and reentrancy, subscription lifetime and
//			document replacement. Negative checks cover each refusal path.
//
//=============================================================================//

#include "hammer/app/edit_session.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <memory>

using namespace hammer::app;
using namespace hammer::scene;
using mapgeometry::Vec3d;

namespace
{

Solid BoxAt( double x )
{
	FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	return MakeBoxSolid( { Vec3d( x, 0, 0 ), Vec3d( x + 64, 64, 64 ) }, tex );
}

} // namespace

int main()
{
	testing::Checks checks;

	EditSession session;
	std::vector<SessionEvent> events;
	SessionSubscription sub = session.Subscribe(
	    [&]( const SessionEvent &e )
	    {
		    events.push_back( e );
	    } );

	checks.That( !session.IsModified() && session.Document().ObjectCount() == 0,
	    "new session is empty and clean" );

	// Create, selecting the result.
	ObjectId a;
	auto created = session.ExecuteSelecting( "Create block",
	    [&]( DocumentEdit &edit, Selection &after )
	    {
		    a = edit.Add( BoxAt( 0 ) );
		    after = CombineObjects( {}, { a }, SelectMode::Replace );
		    return EditResult{};
	    } );
	checks.That( created && created.Value().committed && created.Value().created.size() == 1,
	    "create committed" );
	checks.That( session.Document().FindSolid( a ) && session.CurrentSelection().Contains( a ),
	    "created and selected" );
	checks.That( session.IsModified(), "an edit marks the document modified" );
	checks.That( events.size() == 1 && events[0].kind == SessionEventKind::Edited &&
	                 events[0].changes && events[0].label == "Create block",
	    "one Edited event with the change set" );

	// A failing operation leaves everything untouched.
	const std::uint64_t rev = session.Revision();
	auto failed = session.Execute( "Broken",
	    [&]( DocumentEdit &edit )
	    {
		    edit.Remove( a ); // staged, then the operation refuses
		    return EditResult( Reject( "no" ) );
	    } );
	checks.That( !failed && failed.Error().code == EditErrorCode::Rejected, "refusal propagates" );
	checks.That(
	    session.Document().FindSolid( a ) && session.Revision() == rev && events.size() == 1,
	    "refusal is atomic: no content, history or event change" );

	// An edit that breaks an invariant is rejected before commit (negative).
	auto invalid = session.Execute( "Bad solid",
	    [&]( DocumentEdit &edit )
	    {
		    Solid s = BoxAt( 200 );
		    s.sides.resize( 3 );
		    edit.Add( s );
		    return EditResult{};
	    } );
	checks.That(
	    !invalid && invalid.Error().code == EditErrorCode::Invalid, "invalid edits are refused" );
	checks.Equal( session.Document().ObjectCount(), std::size_t( 1 ),
	    "nothing committed from an invalid edit" );

	// A no-op edit records nothing.
	auto noop = session.Execute( "Nothing",
	    [&]( DocumentEdit &edit )
	    {
		    Solid *s = edit.MutableSolid( a );
		    s->editor.visgroupShown = s->editor.visgroupShown;
		    return EditResult{};
	    } );
	checks.That( noop && !noop.Value().committed && session.History().Size() == 1,
	    "no-op records no history" );

	// Second edit with a different selection, then undo/redo restore selections.
	ObjectId b;
	(void)session.ExecuteSelecting( "Create block 2",
	    [&]( DocumentEdit &edit, Selection &after )
	    {
		    b = edit.Add( BoxAt( 100 ) );
		    after = CombineObjects( {}, { b }, SelectMode::Replace );
		    return EditResult{};
	    } );
	session.MarkSaved();
	checks.That( !session.IsModified(), "saved" );
	checks.That( session.Undo().HasValue(), "undo" );
	checks.That( !session.Document().FindSolid( b ) && session.CurrentSelection().Contains( a ) &&
	                 !session.CurrentSelection().Contains( b ),
	    "undo removes and restores the prior selection" );
	checks.That( session.IsModified(), "undo past the save is modified" );
	checks.That(
	    events.back().kind == SessionEventKind::Undone && events.back().label == "Create block 2",
	    "Undone event" );
	checks.That( session.Redo().HasValue() && session.Document().FindSolid( b ) &&
	                 session.CurrentSelection().Contains( b ) && !session.IsModified(),
	    "redo restores content, selection and the clean state" );
	checks.That( !session.Redo() && session.Redo().Error().code == EditErrorCode::Nothing,
	    "redo past the end (negative)" );

	// Jump through history.
	checks.That(
	    session.JumpTo( 0 ).HasValue() && session.Document().ObjectCount() == 0, "jump to start" );
	checks.That(
	    session.JumpTo( 2 ).HasValue() && session.Document().ObjectCount() == 2, "jump to end" );
	checks.That( !session.JumpTo( 7 ), "jump out of range (negative)" );

	// Selection changes: no history; guards may veto.
	const std::size_t historyBefore = session.History().Size();
	checks.That( session.SelectObjects( { a, b }, SelectMode::Replace ).HasValue(), "select" );
	checks.That(
	    session.CurrentSelection().objects.size() == 2 && session.History().Size() == historyBefore,
	    "selection records no history" );
	checks.That(
	    events.back().kind == SessionEventKind::SelectionChanged, "SelectionChanged event" );
	bool allow = false;
	int guardCalls = 0;
	{
		SessionSubscription guard = session.AddSelectionGuard(
		    [&]()
		    {
			    ++guardCalls;
			    return allow;
		    } );
		auto vetoed = session.ClearSelection();
		checks.That( !vetoed && vetoed.Error().code == EditErrorCode::Vetoed &&
		                 session.CurrentSelection().objects.size() == 2,
		    "a guard vetoes the change" );
		allow = true;
		checks.That( session.ClearSelection().HasValue() && session.CurrentSelection().Empty(),
		    "a guard that resolves its draft allows it" );
		checks.That(
		    session.ClearSelection().HasValue(), "an unchanged selection skips the guards" );
		checks.Equal( guardCalls, 2, "guards run once per real change" );
	}
	allow = false;
	checks.That( session.SelectObjects( { a }, SelectMode::Add ).HasValue(),
	    "a released guard no longer runs" );

	// A guard may commit its draft before the selection changes, but may not
	// change the selection or move through history itself.
	{
		(void)session.SelectObjects( { a }, SelectMode::Replace );
		std::optional<bool> committed;
		std::optional<EditErrorCode> undoInGuard;
		const std::size_t entries = session.History().Size();
		SessionSubscription draft = session.AddSelectionGuard(
		    [&]()
		    {
			    auto r = session.Execute( "Edit properties",
			        [&]( DocumentEdit &edit )
			        {
				        edit.MutableSolid( a )->editor.comments = std::string( "draft" );
				        return EditResult{};
			        } );
			    committed = r.HasValue() && r.Value().committed;
			    auto u = session.Undo();
			    undoInGuard = u ? EditErrorCode::Rejected : u.Error().code;
			    return true;
		    } );
		checks.That(
		    session.SelectObjects( { b }, SelectMode::Replace ).HasValue(), "the change proceeds" );
		checks.That( committed == true && session.History().Size() == entries + 1,
		    "a guard commits its draft first" );
		checks.That( session.History().Entries().back().selectionBefore.Contains( a ),
		    "the draft's undo step restores the old selection" );
		checks.That( session.CurrentSelection().Contains( b ), "then the selection changes" );
		checks.That( undoInGuard == EditErrorCode::Busy, "undo inside a guard is Busy (negative)" );
	}

	// Reentrancy: mutation from an observer is refused.
	{
		std::optional<EditErrorCode> inner;
		SessionSubscription reentrant = session.Subscribe(
		    [&]( const SessionEvent &e )
		    {
			    if ( e.kind == SessionEventKind::SelectionChanged && !inner )
			    {
				    auto r = session.Undo();
				    inner = r ? EditErrorCode::Rejected : r.Error().code;
			    }
		    } );
		(void)session.SelectObjects( { a }, SelectMode::Toggle );
		checks.That(
		    inner == EditErrorCode::Busy, "mutation inside an observer is Busy (negative)" );
	}

	// Subscriptions may outlive the session.
	{
		auto temp = std::make_unique<EditSession>( 5u );
		SessionSubscription late = temp->Subscribe( []( const SessionEvent & ) {} );
		temp.reset();
		late.Reset(); // must not touch the destroyed session
		checks.That( true, "releasing after the session is gone is safe" );
	}

	// Replace resets history and selection.
	MapDocument fresh( 1 );
	checks.That( session.Replace( std::move( fresh ) ).HasValue(), "replace" );
	checks.That( session.History().Size() == 0 && !session.IsModified() &&
	                 session.CurrentSelection().Empty() &&
	                 events.back().kind == SessionEventKind::Replaced,
	    "replace resets" );

	// Pruning: undoing a creation drops it from the selection.
	ObjectId c;
	(void)session.ExecuteSelecting( "Create",
	    [&]( DocumentEdit &edit, Selection &after )
	    {
		    c = edit.Add( BoxAt( 0 ) );
		    after = CombineObjects( {}, { c }, SelectMode::Replace );
		    return EditResult{};
	    } );
	(void)session.Undo();
	checks.That( session.CurrentSelection().Empty(), "selection pruned after undo" );

	// History limit.
	session.SetHistoryLimit( 2 );
	for ( int i = 0; i < 4; ++i )
		(void)session.Execute( "Create",
		    [&]( DocumentEdit &edit )
		    {
			    edit.Add( BoxAt( 1000 + 100 * i ) );
			    return EditResult{};
		    } );
	checks.Equal( session.History().Size(), std::size_t( 2 ), "history limit respected" );

	return checks.Report();
}
