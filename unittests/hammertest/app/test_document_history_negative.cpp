//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Sensitivity check for the modified-state clause (RFC 0002/0005). A
//			provider that decides "modified" from the monotonic revision counter
//			(instead of the saved history position) reports the document modified
//			after an undo back to the saved state. The oracle must detect this;
//			otherwise it does not actually pin the RFC's revision-vs-save rule.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/document_history.h"

#include <cstdint>
#include <cstdio>

using hammer::app::DocumentHistory;

namespace
{

// Broken provider: tracks a saved REVISION and reports modified whenever the
// current revision differs from it. Because undo advances the revision, undoing
// to the saved position is wrongly reported as modified.
class RevisionModifiedHistory
{
public:
	bool Commit( bool changed ) { return m_inner.Commit( changed ); }
	bool Undo() { return m_inner.Undo(); }
	bool Redo() { return m_inner.Redo(); }
	void MarkSaved() { m_savedRevision = m_inner.Revision(); }
	bool IsModified() const { return m_inner.Revision() != m_savedRevision; }

private:
	DocumentHistory m_inner;
	std::uint64_t m_savedRevision = 0;
};

// The revision-vs-save clause: commit, save, commit, undo-back-to-save; the
// document must then report NOT modified. Returns true when the provider
// implements the rule correctly.
template <typename History> bool ConformsUndoToSavedClearsModified( History &history )
{
	history.Commit( true );
	history.MarkSaved();
	history.Commit( true );
	history.Undo(); // back to the saved position
	return !history.IsModified();
}

} // namespace

int main()
{
	DocumentHistory realHistory;
	RevisionModifiedHistory brokenHistory;

	const bool realConforms = ConformsUndoToSavedClearsModified( realHistory );
	const bool brokenConforms = ConformsUndoToSavedClearsModified( brokenHistory );

	int failures = 0;

	if ( !realConforms )
	{
		std::printf( "FAIL: real DocumentHistory did not clear modified on undo-to-saved\n" );
		++failures;
	}
	if ( brokenConforms )
	{
		std::printf( "FAIL: predicate did NOT detect the revision-based modified provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.app DocumentHistory negative: %d check(s) FAILED\n", failures );
		return 1;
	}
	std::printf( "hammer.app DocumentHistory negative: oracle detects violations (real passes, "
	             "revision-based caught)\n" );
	return 0;
}
