//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/presenters/history_panel.h.
//
//=============================================================================//

#include "hammer/presenters/history_panel.h"

namespace hammer::presenters
{

HistoryPanel::HistoryPanel( app::EditSession &session ) : m_session( session )
{
	m_subscription = m_session.Subscribe(
	    [this]( const app::SessionEvent &event )
	    {
		    if ( event.kind != app::SessionEventKind::SelectionChanged )
		    {
			    Rebuild();
		    }
	    } );
	Rebuild();
}

void HistoryPanel::Rebuild()
{
	const app::ChangeHistory &history = m_session.History();
	m_position = history.Position();
	m_rows.clear();
	const std::vector<app::HistoryEntry> &entries = history.Entries();
	for ( std::size_t k = 0; k < entries.size(); ++k )
	{
		HistoryRow row;
		row.position = k + 1;
		row.label = entries[k].label;
		row.done = k < m_position;
		row.current = k + 1 == m_position;
		m_rows.push_back( std::move( row ) );
	}
	m_canUndo = history.CanUndo();
	m_canRedo = history.CanRedo();
	m_modified = history.IsModified();
	const app::HistoryEntry *undo = history.UndoEntry();
	const app::HistoryEntry *redo = history.RedoEntry();
	m_undoLabel = undo ? "Undo " + undo->label : std::string( "Undo" );
	m_redoLabel = redo ? "Redo " + redo->label : std::string( "Redo" );
	++m_revision;
}

HistoryPanel::Result HistoryPanel::JumpTo( std::size_t position )
{
	return m_session.JumpTo( position );
}

HistoryPanel::Result HistoryPanel::Undo()
{
	return m_session.Undo();
}

HistoryPanel::Result HistoryPanel::Redo()
{
	return m_session.Redo();
}

} // namespace hammer::presenters
