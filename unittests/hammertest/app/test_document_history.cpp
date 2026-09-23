//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance oracle for hammer::app::DocumentHistory (RFC 0002). Pins
//			the transaction/history semantics: no-op commits change nothing, redo
//			tail is dropped on a new commit, the revision counter is monotonic
//			across undo, and -- the subtle one -- undoing back to the saved
//			position clears the modified indicator even though the revision has
//			advanced.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/document_history.h"
#include "testing/conformance_result.h"

#include <cstdio>

using hammer::app::DocumentHistory;

namespace
{

int g_failures = 0;
int g_checks = 0;

void Check( bool condition, const char *expression, int line )
{
	++g_checks;
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_document_history.cpp", line, expression );
		++g_failures;
	}
}

} // namespace

#define CHECK( expr ) Check( ( expr ), #expr, __LINE__ )

// A no-op commit records nothing and does not mark the document modified.
void TestNoOpCommit()
{
	DocumentHistory history;
	CHECK( !history.IsModified() );
	const std::uint64_t revision = history.Revision();

	CHECK( !history.Commit( false ) ); // no-op
	CHECK( history.Revision() == revision );
	CHECK( history.Depth() == 0 );
	CHECK( !history.IsModified() );
	CHECK( !history.CanUndo() );
}

// Basic commit/undo/redo and the monotonic revision counter.
void TestCommitUndoRedo()
{
	DocumentHistory history;
	CHECK( history.Commit( true ) );
	CHECK( history.Commit( true ) );
	CHECK( history.Depth() == 2 && history.Position() == 2 );
	CHECK( history.IsModified() );

	const std::uint64_t afterCommits = history.Revision();
	CHECK( history.Undo() );
	CHECK( history.Position() == 1 );
	CHECK( history.Revision() > afterCommits ); // undo advances the revision

	CHECK( history.Redo() );
	CHECK( history.Position() == 2 );
	CHECK( !history.CanRedo() );
}

// Committing after an undo discards the redo tail.
void TestRedoTailDropped()
{
	DocumentHistory history;
	history.Commit( true );
	history.Commit( true );
	history.Commit( true ); // depth 3, position 3
	history.Undo();
	history.Undo(); // position 1, depth 3

	CHECK( history.CanRedo() );
	history.Commit( true ); // new branch: drops the two-deep redo tail
	CHECK( history.Depth() == 2 && history.Position() == 2 );
	CHECK( !history.CanRedo() );
}

// The critical clause: undoing back to the saved position clears the modified
// indicator, even though the revision counter has advanced past the save.
void TestSavedPositionVsRevision()
{
	DocumentHistory history;
	history.Commit( true );
	history.MarkSaved(); // saved at position 1
	CHECK( !history.IsModified() );

	history.Commit( true ); // position 2, modified
	CHECK( history.IsModified() );
	const std::uint64_t revisionAtDirty = history.Revision();

	CHECK( history.Undo() );                       // back to position 1 == saved position
	CHECK( !history.IsModified() );                // modified cleared...
	CHECK( history.Revision() > revisionAtDirty ); // ...even though revision grew

	history.Redo(); // position 2 again -> modified once more
	CHECK( history.IsModified() );
}

int main()
{
	TestNoOpCommit();
	TestCommitUndoRedo();
	TestRedoTailDropped();
	TestSavedPositionVsRevision();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app DocumentHistory: %d check(s) FAILED\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "hammer.app DocumentHistory: all checks passed\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
