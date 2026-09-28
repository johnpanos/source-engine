//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/edit_session.h.
//
//=============================================================================//

#include "hammer/app/edit_session.h"

#include <map>

namespace hammer::app
{

const char *EditErrorName( EditErrorCode code )
{
	switch ( code )
	{
	case EditErrorCode::Rejected:
		return "rejected";
	case EditErrorCode::Invalid:
		return "invalid";
	case EditErrorCode::Vetoed:
		return "vetoed";
	case EditErrorCode::Busy:
		return "busy";
	case EditErrorCode::Nothing:
		return "nothing to do";
	}
	return "error";
}

SessionSubscription::SessionSubscription( SessionSubscription &&other ) noexcept
    : m_release( std::move( other.m_release ) )
{
	other.m_release = nullptr;
}

SessionSubscription &SessionSubscription::operator=( SessionSubscription &&other ) noexcept
{
	if ( this != &other )
	{
		Reset();
		m_release = std::move( other.m_release );
		other.m_release = nullptr;
	}
	return *this;
}

void SessionSubscription::Reset()
{
	if ( m_release )
	{
		std::function<void()> release = std::move( m_release );
		m_release = nullptr;
		release();
	}
}

// Observers and guards live here, shared with the subscriptions that remove
// them, so a subscription may safely outlive its session (release is a no-op
// once the session is gone).
struct EditSession::Registry
{
	std::uint64_t next = 1;
	std::map<std::uint64_t, Observer> observers;
	std::map<std::uint64_t, SelectionGuard> guards;
};

namespace
{

class BusyScope
{
public:
	explicit BusyScope( int &busy ) : m_busy( busy ) { ++m_busy; }
	~BusyScope() { --m_busy; }
	BusyScope( const BusyScope & ) = delete;
	BusyScope &operator=( const BusyScope & ) = delete;

private:
	int &m_busy;
};

foundation::Unexpected<EditError> Busy()
{
	return foundation::MakeUnexpected(
	    EditError{ EditErrorCode::Busy, "the session is notifying; retry after the callback" } );
}

} // namespace

EditSession::EditSession( std::uint32_t serial )
    : m_document( serial ), m_registry( std::make_shared<Registry>() )
{
}

EditSession::EditSession( scene::MapDocument document )
    : m_document( std::move( document ) ), m_registry( std::make_shared<Registry>() )
{
}

EditSession::~EditSession()
{
	m_registry->observers.clear();
	m_registry->guards.clear();
}

bool EditSession::RunGuards()
{
	BusyScope busy( m_busy );
	// Copy: a guard may drop its own subscription.
	const std::map<std::uint64_t, SelectionGuard> guards = m_registry->guards;
	for ( const auto &entry : guards )
	{
		if ( !entry.second() )
		{
			return false;
		}
	}
	return true;
}

void EditSession::Publish( const SessionEvent &event )
{
	BusyScope busy( m_busy );
	const std::map<std::uint64_t, Observer> observers = m_registry->observers;
	for ( const auto &entry : observers )
	{
		// Skip observers unsubscribed by an earlier observer in this round.
		if ( m_registry->observers.count( entry.first ) )
		{
			entry.second( event );
		}
	}
}

foundation::Expected<CommitInfo, EditError> EditSession::Commit(
    const std::string &label, scene::DocumentEdit &edit, const std::optional<Selection> &selectionAfter )
{
	const std::vector<std::string> problems = scene::ValidateEdit( edit );
	if ( !problems.empty() )
	{
		std::string message = label + ": ";
		for ( std::size_t i = 0; i < problems.size(); ++i )
		{
			message += ( i ? "; " : "" ) + problems[i];
		}
		return foundation::MakeUnexpected( EditError{ EditErrorCode::Invalid, message } );
	}

	scene::ChangeSet changes = edit.Finish();
	CommitInfo info;
	if ( changes.Empty() )
	{
		// A no-op records nothing; id allocations are still retired.
		m_document.AdvanceCounters( edit.NextLocal(), edit.NextVmfId() );
		info.revision = m_history.Revision();
		return info;
	}

	const Selection before = m_selection;
	scene::Apply( m_document, changes, scene::ApplyDirection::Forward );
	m_document.AdvanceCounters( edit.NextLocal(), edit.NextVmfId() );
	m_selection = Prune( m_document, selectionAfter ? *selectionAfter : m_selection );

	info.committed = true;
	info.created = changes.Created();
	m_history.Push( HistoryEntry{ label, std::move( changes ), before, m_selection } );
	info.revision = m_history.Revision();

	SessionEvent event;
	event.kind = SessionEventKind::Edited;
	event.revision = info.revision;
	event.changes = &m_history.UndoEntry()->changes;
	event.label = label;
	Publish( event );
	return info;
}

foundation::Expected<CommitInfo, EditError> EditSession::Execute(
    const std::string &label, const Operation &operation, const std::optional<Selection> &selectionAfter )
{
	if ( m_busy )
	{
		return Busy();
	}
	scene::DocumentEdit edit( m_document );
	EditResult result = operation( edit );
	if ( !result )
	{
		return foundation::MakeUnexpected( std::move( result ).Error() );
	}
	return Commit( label, edit, selectionAfter );
}

foundation::Expected<CommitInfo, EditError> EditSession::ExecuteSelecting(
    const std::string &label, const SelectingOperation &operation )
{
	if ( m_busy )
	{
		return Busy();
	}
	scene::DocumentEdit edit( m_document );
	Selection after = m_selection;
	EditResult result = operation( edit, after );
	if ( !result )
	{
		return foundation::MakeUnexpected( std::move( result ).Error() );
	}
	return Commit( label, edit, after );
}

foundation::Expected<void, EditError> EditSession::Undo()
{
	if ( m_busy )
	{
		return Busy();
	}
	const HistoryEntry *entry = m_history.UndoEntry();
	if ( !entry )
	{
		return NothingToDo( "nothing to undo" );
	}
	scene::Apply( m_document, entry->changes, scene::ApplyDirection::Backward );
	m_selection = Prune( m_document, entry->selectionBefore );
	m_history.StepBack();
	SessionEvent event;
	event.kind = SessionEventKind::Undone;
	event.revision = m_history.Revision();
	event.changes = &entry->changes;
	event.label = entry->label;
	Publish( event );
	return {};
}

foundation::Expected<void, EditError> EditSession::Redo()
{
	if ( m_busy )
	{
		return Busy();
	}
	const HistoryEntry *entry = m_history.RedoEntry();
	if ( !entry )
	{
		return NothingToDo( "nothing to redo" );
	}
	scene::Apply( m_document, entry->changes, scene::ApplyDirection::Forward );
	m_selection = Prune( m_document, entry->selectionAfter );
	m_history.StepForward();
	SessionEvent event;
	event.kind = SessionEventKind::Redone;
	event.revision = m_history.Revision();
	event.changes = &entry->changes;
	event.label = entry->label;
	Publish( event );
	return {};
}

foundation::Expected<void, EditError> EditSession::JumpTo( std::size_t position )
{
	if ( position > m_history.Size() )
	{
		return foundation::MakeUnexpected(
		    EditError{ EditErrorCode::Rejected, "history position out of range" } );
	}
	while ( m_history.Position() > position )
	{
		if ( auto r = Undo(); !r )
		{
			return r;
		}
	}
	while ( m_history.Position() < position )
	{
		if ( auto r = Redo(); !r )
		{
			return r;
		}
	}
	return {};
}

void EditSession::MarkSaved()
{
	m_history.MarkSaved();
	SessionEvent event;
	event.kind = SessionEventKind::Saved;
	event.revision = m_history.Revision();
	Publish( event );
}

foundation::Expected<void, EditError> EditSession::Replace( scene::MapDocument document )
{
	if ( m_busy )
	{
		return Busy();
	}
	if ( !RunGuards() )
	{
		return foundation::MakeUnexpected(
		    EditError{ EditErrorCode::Vetoed, "a pending edit kept the document open" } );
	}
	m_document = std::move( document );
	m_selection = Selection{};
	m_history.Reset();
	SessionEvent event;
	event.kind = SessionEventKind::Replaced;
	event.revision = m_history.Revision();
	Publish( event );
	return {};
}

foundation::Expected<void, EditError> EditSession::ChangeSelection( Selection next )
{
	if ( m_busy )
	{
		return Busy();
	}
	next = Prune( m_document, next );
	if ( next == m_selection )
	{
		return {};
	}
	if ( !RunGuards() )
	{
		return foundation::MakeUnexpected(
		    EditError{ EditErrorCode::Vetoed, "a pending edit kept the selection" } );
	}
	m_selection = std::move( next );
	SessionEvent event;
	event.kind = SessionEventKind::SelectionChanged;
	event.revision = m_history.Revision();
	Publish( event );
	return {};
}

foundation::Expected<void, EditError> EditSession::SetSelection( const Selection &selection )
{
	return ChangeSelection( selection );
}

foundation::Expected<void, EditError> EditSession::SelectObjects(
    const std::vector<scene::ObjectId> &ids, SelectMode mode )
{
	return ChangeSelection( CombineObjects( m_selection, ids, mode ) );
}

foundation::Expected<void, EditError> EditSession::SelectFaces(
    const std::vector<scene::FaceRef> &faces, SelectMode mode )
{
	return ChangeSelection( CombineFaces( m_selection, faces, mode ) );
}

foundation::Expected<void, EditError> EditSession::ClearSelection()
{
	return ChangeSelection( Selection{} );
}

SessionSubscription EditSession::Subscribe( Observer observer )
{
	const std::uint64_t id = m_registry->next++;
	m_registry->observers.emplace( id, std::move( observer ) );
	std::weak_ptr<Registry> weak = m_registry;
	return SessionSubscription( [weak, id]()
	    {
		    if ( const std::shared_ptr<Registry> r = weak.lock() )
		    {
			    r->observers.erase( id );
		    }
	    } );
}

SessionSubscription EditSession::AddSelectionGuard( SelectionGuard guard )
{
	const std::uint64_t id = m_registry->next++;
	m_registry->guards.emplace( id, std::move( guard ) );
	std::weak_ptr<Registry> weak = m_registry;
	return SessionSubscription( [weak, id]()
	    {
		    if ( const std::shared_ptr<Registry> r = weak.lock() )
		    {
			    r->guards.erase( id );
		    }
	    } );
}

} // namespace hammer::app
