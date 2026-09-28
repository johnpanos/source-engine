//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The labeled undo history of one document (RFC 0002, hammer.app;
//			"Edit transactions and history"). Each entry is one committed
//			change set with a user-facing label and the selection before and
//			after it, so undo restores what the user was working on.
//
//			Counters follow DocumentHistory's contract: a monotonic revision that
//			bumps on every commit, undo and redo; a position; and a saved
//			position, so undoing back to the saved state clears the modified
//			flag. The history may be bounded: dropping the oldest entries past
//			the limit keeps the position consistent, and when the saved state
//			falls off the end the document stays modified until the next save.
//
//=============================================================================//

#ifndef HAMMER_APP_CHANGE_HISTORY_H
#define HAMMER_APP_CHANGE_HISTORY_H

#include "hammer/app/selection.h"
#include "hammer/scene/change_set.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hammer::app
{

struct HistoryEntry
{
	std::string label;
	scene::ChangeSet changes;
	Selection selectionBefore;
	Selection selectionAfter;
};

class ChangeHistory
{
public:
	// 'limit' bounds the number of entries kept (0 = unbounded).
	explicit ChangeHistory( std::size_t limit = 0 );

	// Records an entry after the current position, discarding any redo tail.
	void Push( HistoryEntry entry );

	bool CanUndo() const { return m_position > 0; }
	bool CanRedo() const { return m_position < m_entries.size(); }

	// The entry an undo would revert / a redo would reapply; nullptr if none.
	const HistoryEntry *UndoEntry() const;
	const HistoryEntry *RedoEntry() const;

	// Move the position by one (the caller applies the entry's changes).
	// Return false when there is nothing to move over.
	bool StepBack();
	bool StepForward();

	void MarkSaved();
	bool IsModified() const;
	// Clears every entry and makes the current state the saved state.
	void Reset();

	std::uint64_t Revision() const { return m_revision; }
	std::size_t Position() const { return m_position; }
	std::size_t Size() const { return m_entries.size(); }
	const std::vector<HistoryEntry> &Entries() const { return m_entries; }
	// For bookkeeping that must follow every recorded state (the saved map
	// version); never for content changes.
	std::vector<HistoryEntry> &MutableEntries() { return m_entries; }
	std::size_t Limit() const { return m_limit; }
	void SetLimit( std::size_t limit );

private:
	void Trim();

	std::vector<HistoryEntry> m_entries;
	std::size_t m_position = 0;
	// The saved position; nothing when the saved state is no longer reachable.
	std::optional<std::size_t> m_saved = 0;
	std::uint64_t m_revision = 0;
	std::size_t m_limit = 0;
};

} // namespace hammer::app

#endif // HAMMER_APP_CHANGE_HISTORY_H
