//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The per-document editing authority (RFC 0002, hammer.app;
//			"State ownership and lifetime": the EditorSession). One session owns
//			one MapDocument, its selection and its history, and is the only way
//			content changes:
//
//			  request -> stage in a DocumentEdit -> validate -> commit change set
//			  -> record history -> publish one event
//
//			An operation that fails, or whose edit fails validation, leaves the
//			document, selection and history untouched. A no-op edit records
//			nothing and does not mark the document modified. Undo and redo apply
//			recorded change sets and restore the selection recorded with them.
//
//			Selection is not document content: changing it records no history,
//			but it passes the selection guards first (an inspector with an
//			uncommitted draft resolves or vetoes it), so pointer, keyboard and
//			menu entry points share one draft policy.
//
//			Threading: single-sequence. Observers run synchronously after a
//			change is committed; a mutation requested from inside an observer
//			or guard is refused with EditErrorCode::Busy.
//
//=============================================================================//

#ifndef HAMMER_APP_EDIT_SESSION_H
#define HAMMER_APP_EDIT_SESSION_H

#include "foundation/expected.h"
#include "hammer/app/change_history.h"
#include "hammer/app/selection.h"
#include "hammer/scene/change_set.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hammer::app
{

enum class EditErrorCode
{
	Rejected, // the operation refused the request (degenerate input, unknown id)
	Invalid,  // the staged result broke a document invariant (an operation bug)
	Vetoed,   // a selection guard refused the selection change
	Busy,     // requested from inside an observer or guard
	Nothing,  // nothing to act on (empty selection, nothing to undo)
};

const char *EditErrorName( EditErrorCode code );

struct EditError
{
	EditErrorCode code = EditErrorCode::Rejected;
	std::string message;
};

// The value an operation returns: success, or why it refused.
using EditResult = foundation::Expected<void, EditError>;

inline foundation::Unexpected<EditError> Reject( std::string message )
{
	return foundation::MakeUnexpected( EditError{ EditErrorCode::Rejected, std::move( message ) } );
}

inline foundation::Unexpected<EditError> NothingToDo( std::string message )
{
	return foundation::MakeUnexpected( EditError{ EditErrorCode::Nothing, std::move( message ) } );
}

enum class SessionEventKind
{
	Edited,           // an edit was committed
	Undone,           // an undo applied 'changes' backward
	Redone,           // a redo applied 'changes' forward
	Replaced,         // the whole document was replaced (load, new map)
	SelectionChanged, // only the selection changed
	Saved,            // the saved position moved (modified flag may change)
};

struct SessionEvent
{
	SessionEventKind kind = SessionEventKind::Edited;
	std::uint64_t revision = 0;
	const scene::ChangeSet *changes = nullptr; // Edited/Undone/Redone
	std::string label;                         // the history label, when any
};

// An RAII registration: destroying it unregisters the callback.
class SessionSubscription
{
public:
	SessionSubscription() = default;
	explicit SessionSubscription( std::function<void()> release ) : m_release( std::move( release ) ) {}
	~SessionSubscription() { Reset(); }
	SessionSubscription( SessionSubscription &&other ) noexcept;
	SessionSubscription &operator=( SessionSubscription &&other ) noexcept;
	SessionSubscription( const SessionSubscription & ) = delete;
	SessionSubscription &operator=( const SessionSubscription & ) = delete;
	void Reset();

private:
	std::function<void()> m_release;
};

struct CommitInfo
{
	bool committed = false; // false: the edit was a no-op
	std::uint64_t revision = 0;
	std::vector<scene::ObjectId> created;
};

class EditSession
{
public:
	// A new, empty, unmodified map.
	explicit EditSession( std::uint32_t serial = 1 );
	// Adopts a loaded document as the unmodified base state.
	explicit EditSession( scene::MapDocument document );
	~EditSession();

	EditSession( const EditSession & ) = delete;
	EditSession &operator=( const EditSession & ) = delete;

	const scene::MapDocument &Document() const { return m_document; }
	const Selection &CurrentSelection() const { return m_selection; }
	const ChangeHistory &History() const { return m_history; }
	std::uint64_t Revision() const { return m_history.Revision(); }
	bool IsModified() const { return m_history.IsModified(); }

	// --- Editing ------------------------------------------------------------
	// Stages an edit with 'operation' and commits it as one history unit
	// labeled 'label'. 'selectionAfter', when given, becomes the selection
	// (pruned to live objects); otherwise the current selection is pruned.
	using Operation = std::function<EditResult( scene::DocumentEdit & )>;
	foundation::Expected<CommitInfo, EditError> Execute( const std::string &label,
	    const Operation &operation, const std::optional<Selection> &selectionAfter = std::nullopt );

	// Like Execute, but the operation also chooses the selection after it.
	using SelectingOperation =
	    std::function<EditResult( scene::DocumentEdit &, Selection &selectionAfter )>;
	foundation::Expected<CommitInfo, EditError> ExecuteSelecting(
	    const std::string &label, const SelectingOperation &operation );

	foundation::Expected<void, EditError> Undo();
	foundation::Expected<void, EditError> Redo();
	// Undoes or redoes until the history position is 'position'.
	foundation::Expected<void, EditError> JumpTo( std::size_t position );

	void MarkSaved();
	// Replaces the document (open, new map): history cleared, selection empty,
	// unmodified. Passes the selection guards first.
	foundation::Expected<void, EditError> Replace( scene::MapDocument document );
	void SetHistoryLimit( std::size_t limit ) { m_history.SetLimit( limit ); }

	// --- Selection ------------------------------------------------------------
	foundation::Expected<void, EditError> SetSelection( const Selection &selection );
	foundation::Expected<void, EditError> SelectObjects(
	    const std::vector<scene::ObjectId> &ids, SelectMode mode );
	foundation::Expected<void, EditError> SelectFaces(
	    const std::vector<scene::FaceRef> &faces, SelectMode mode );
	foundation::Expected<void, EditError> ClearSelection();

	// --- Observation ------------------------------------------------------------
	using Observer = std::function<void( const SessionEvent & )>;
	[[nodiscard]] SessionSubscription Subscribe( Observer observer );

	// A guard runs before every selection change and document replacement. It
	// returns true to allow it (after committing or discarding its draft) or
	// false to veto it.
	using SelectionGuard = std::function<bool()>;
	[[nodiscard]] SessionSubscription AddSelectionGuard( SelectionGuard guard );

private:
	struct Registry;

	bool RunGuards();
	void Publish( const SessionEvent &event );
	foundation::Expected<CommitInfo, EditError> Commit( const std::string &label,
	    scene::DocumentEdit &edit, const std::optional<Selection> &selectionAfter );
	foundation::Expected<void, EditError> ChangeSelection( Selection next );

	scene::MapDocument m_document;
	Selection m_selection;
	ChangeHistory m_history;
	std::shared_ptr<Registry> m_registry; // shared with subscriptions' release
	int m_busy = 0;
};

} // namespace hammer::app

#endif // HAMMER_APP_EDIT_SESSION_H
