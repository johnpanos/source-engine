//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The undo history panel presentation model (RFC 0002,
//			hammer.presenters; Source 2's undo-history jump): one row per
//			history entry of the session, the current position, and the
//			Edit-menu labels.
//
//			Row k (0-based entry) has position k + 1: JumpTo( position ) leaves
//			that entry as the last one applied; JumpTo( 0 ) undoes everything.
//			done = the entry is applied (k < Position()); current = it is the
//			last applied one. UndoLabel() is "Undo <label>" (or "Undo" when
//			nothing can be undone), RedoLabel() likewise. The model refreshes on
//			every history change (edit, undo, redo, replace, save) and ignores
//			selection-only changes.
//
//			The subscription is RAII; the panel may be destroyed before or after
//			its session, but calls need a live session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_HISTORY_PANEL_H
#define HAMMER_PRESENTERS_HISTORY_PANEL_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hammer::presenters
{

struct HistoryRow
{
	std::size_t position = 0; // JumpTo( position ) applies this entry and all before it
	std::string label;
	bool done = false;
	bool current = false;
};

class HistoryPanel
{
public:
	using Result = foundation::Expected<void, app::EditError>;

	explicit HistoryPanel( app::EditSession &session );
	HistoryPanel( const HistoryPanel & ) = delete;
	HistoryPanel &operator=( const HistoryPanel & ) = delete;

	std::uint64_t Revision() const { return m_revision; }
	const std::vector<HistoryRow> &Rows() const { return m_rows; }
	std::size_t Position() const { return m_position; }
	bool CanUndo() const { return m_canUndo; }
	bool CanRedo() const { return m_canRedo; }
	const std::string &UndoLabel() const { return m_undoLabel; }
	const std::string &RedoLabel() const { return m_redoLabel; }
	bool IsModified() const { return m_modified; }

	Result JumpTo( std::size_t position );
	Result Undo();
	Result Redo();

private:
	void Rebuild();

	app::EditSession &m_session;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::vector<HistoryRow> m_rows;
	std::size_t m_position = 0;
	bool m_canUndo = false;
	bool m_canRedo = false;
	bool m_modified = false;
	std::string m_undoLabel;
	std::string m_redoLabel;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_HISTORY_PANEL_H
