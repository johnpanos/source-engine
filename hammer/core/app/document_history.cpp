//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the document history reference model (RFC 0002).
//			No tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/app/document_history.h"

namespace hammer::app
{

DocumentHistory::DocumentHistory() = default;

bool DocumentHistory::Commit( bool changed )
{
	// A no-op contributes nothing: no history unit, no revision bump, and the
	// modified indicator is left exactly as it was.
	if ( !changed )
	{
		return false;
	}

	// Committing after some undos discards the redo tail: the units ahead of the
	// current position are no longer reachable.
	m_depth = m_position;
	++m_depth;
	++m_position;
	++m_revision;
	return true;
}

bool DocumentHistory::CanUndo() const
{
	return m_position > 0;
}

bool DocumentHistory::CanRedo() const
{
	return m_position < m_depth;
}

bool DocumentHistory::Undo()
{
	if ( !CanUndo() )
	{
		return false;
	}
	--m_position;
	++m_revision;
	return true;
}

bool DocumentHistory::Redo()
{
	if ( !CanRedo() )
	{
		return false;
	}
	++m_position;
	++m_revision;
	return true;
}

void DocumentHistory::MarkSaved()
{
	m_savedPosition = m_position;
}

bool DocumentHistory::IsModified() const
{
	return m_position != m_savedPosition;
}

} // namespace hammer::app
