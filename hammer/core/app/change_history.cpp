//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/change_history.h.
//
//=============================================================================//

#include "hammer/app/change_history.h"

namespace hammer::app
{

ChangeHistory::ChangeHistory( std::size_t limit ) : m_limit( limit )
{
}

void ChangeHistory::Push( HistoryEntry entry )
{
	m_entries.resize( m_position );
	if ( m_saved && *m_saved > m_position )
	{
		m_saved.reset(); // the saved state was in the discarded redo tail
	}
	m_entries.push_back( std::move( entry ) );
	++m_position;
	++m_revision;
	Trim();
}

const HistoryEntry *ChangeHistory::UndoEntry() const
{
	return CanUndo() ? &m_entries[m_position - 1] : nullptr;
}

const HistoryEntry *ChangeHistory::RedoEntry() const
{
	return CanRedo() ? &m_entries[m_position] : nullptr;
}

bool ChangeHistory::StepBack()
{
	if ( !CanUndo() )
	{
		return false;
	}
	--m_position;
	++m_revision;
	return true;
}

bool ChangeHistory::StepForward()
{
	if ( !CanRedo() )
	{
		return false;
	}
	++m_position;
	++m_revision;
	return true;
}

void ChangeHistory::MarkSaved()
{
	m_saved = m_position;
}

bool ChangeHistory::IsModified() const
{
	return !m_saved || *m_saved != m_position;
}

void ChangeHistory::Reset()
{
	m_entries.clear();
	m_position = 0;
	m_saved = 0;
	++m_revision;
}

void ChangeHistory::SetLimit( std::size_t limit )
{
	m_limit = limit;
	Trim();
}

void ChangeHistory::Trim()
{
	if ( m_limit == 0 || m_entries.size() <= m_limit )
	{
		return;
	}
	const std::size_t drop = m_entries.size() - m_limit;
	// Never drop entries at or after the position: those are the undo target.
	const std::size_t dropped = drop < m_position ? drop : m_position;
	m_entries.erase(
	    m_entries.begin(), m_entries.begin() + static_cast<std::ptrdiff_t>( dropped ) );
	m_position -= dropped;
	if ( m_saved )
	{
		if ( *m_saved < dropped )
		{
			m_saved.reset();
		}
		else
		{
			*m_saved -= dropped;
		}
	}
}

} // namespace hammer::app
