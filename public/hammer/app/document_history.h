//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Independent document edit-transaction / history reference model
//			(RFC 0002, hammer.app). This is the "small independent document/
//			history model" the RFC verification strategy calls for: it pins the
//			transaction and undo/redo semantics -- especially the distinction
//			between the monotonic revision counter and the save/history position
//			used to decide whether the document is modified -- without any MFC
//			CDocument, CHistory, tier0, PCH, or GPU dependency.
//
//			It is a semantics oracle ahead of the full H3 application authority,
//			not yet the live editor history. It intentionally models only the
//			bookkeeping (positions, revision, modified state), not the reversible
//			change payloads.
//
//=============================================================================//

#ifndef HAMMER_APP_DOCUMENT_HISTORY_H
#define HAMMER_APP_DOCUMENT_HISTORY_H

#include <cstddef>
#include <cstdint>

namespace hammer::app
{

class DocumentHistory
{
public:
	DocumentHistory();

	// Records one committed change unit. A change (changed == true) truncates any
	// redo tail, advances the history position, and bumps the revision. A no-op
	// (changed == false) records nothing: no history entry, no revision change,
	// and the modified state is untouched. Returns true iff a unit was recorded.
	bool Commit( bool changed );

	bool CanUndo() const;
	bool CanRedo() const;

	// Undo/redo move the history position by one unit if possible and bump the
	// monotonic revision (an undo is itself a state transition). Return true iff
	// they moved.
	bool Undo();
	bool Redo();

	// Marks the current position as the saved position.
	void MarkSaved();

	// The document is modified iff the current history position differs from the
	// saved position. Undoing back to the saved position clears this even though
	// the revision counter has advanced.
	bool IsModified() const;

	// Monotonic revision counter: strictly increases on every committed change and
	// on every undo/redo move. Never decreases, even on undo.
	std::uint64_t Revision() const { return m_revision; }

	std::size_t Position() const { return m_position; }
	std::size_t Depth() const { return m_depth; }

private:
	std::size_t m_depth = 0;        // number of units currently in the history
	std::size_t m_position = 0;     // 0..m_depth; units before position are "done"
	std::size_t m_savedPosition = 0;
	std::uint64_t m_revision = 0;
};

} // namespace hammer::app

#endif // HAMMER_APP_DOCUMENT_HISTORY_H
