//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app change history conformance (RFC 0002 "Edit transactions
//			and history"): labeled entries, redo-tail truncation, the monotonic
//			revision versus the saved position (undo back to the saved state is
//			unmodified), the saved state lost with a discarded redo tail, and
//			the bounded history (oldest entries dropped, position and saved
//			position kept consistent). Negative checks: stepping past either end.
//
//=============================================================================//

#include "hammer/app/change_history.h"
#include "testing/checks.h"

using namespace hammer::app;

namespace
{

HistoryEntry Entry( const char *label )
{
	HistoryEntry e;
	e.label = label;
	return e;
}

} // namespace

int main()
{
	testing::Checks checks;

	ChangeHistory h;
	checks.That( !h.IsModified() && !h.CanUndo() && !h.CanRedo(), "fresh history is clean" );
	checks.That( !h.StepBack() && !h.StepForward(), "nothing to step over (negative)" );
	const std::uint64_t r0 = h.Revision();

	h.Push( Entry( "Create block" ) );
	h.Push( Entry( "Move" ) );
	checks.That( h.IsModified() && h.Size() == 2 && h.Position() == 2, "two entries" );
	checks.That( h.UndoEntry() && h.UndoEntry()->label == "Move", "undo target label" );
	h.MarkSaved();
	checks.That( !h.IsModified(), "saved" );

	checks.That( h.StepBack() && h.IsModified(), "undo past the save is modified" );
	checks.That( h.RedoEntry() && h.RedoEntry()->label == "Move", "redo target label" );
	checks.That( h.StepForward() && !h.IsModified(), "redo back to the save is clean" );
	checks.That( h.Revision() > r0 + 3, "revision is monotonic across undo/redo" );

	// A new edit after undo discards the redo tail, including the saved state.
	h.StepBack();
	h.Push( Entry( "Delete" ) );
	checks.That( !h.CanRedo() && h.Size() == 2, "redo tail discarded" );
	checks.That( h.IsModified(), "the saved state was in the tail: modified" );
	h.StepBack();
	checks.That( h.IsModified(), "and stays unreachable (never clean again until saved)" );
	h.StepForward();
	h.MarkSaved();
	checks.That( !h.IsModified(), "a new save makes it clean" );

	// Bounded history.
	ChangeHistory b( 3 );
	b.MarkSaved(); // saved at position 0
	for ( const char *l : { "a", "b", "c", "d", "e" } )
		b.Push( Entry( l ) );
	checks.Equal( b.Size(), std::size_t( 3 ), "limit keeps three entries" );
	checks.That( b.Entries().front().label == std::string( "c" ), "oldest entries dropped" );
	checks.Equal( b.Position(), std::size_t( 3 ), "position shifted with the trim" );
	checks.That( b.IsModified(), "saved state trimmed away" );
	while ( b.StepBack() )
	{
	}
	checks.That( b.IsModified() && b.Position() == 0, "cannot undo to the trimmed save" );

	ChangeHistory c;
	c.Push( Entry( "a" ) );
	c.Push( Entry( "b" ) );
	c.MarkSaved();
	c.Push( Entry( "c" ) );
	c.SetLimit( 2 );
	checks.That( c.Size() == 2 && c.Position() == 2, "SetLimit trims" );
	c.StepBack();
	checks.That( !c.IsModified(), "saved position shifted by the trim" );

	const std::uint64_t before = c.Revision();
	c.Reset();
	checks.That( c.Size() == 0 && !c.IsModified() && c.Revision() > before, "reset" );

	return checks.Report();
}
