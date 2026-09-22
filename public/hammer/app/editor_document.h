//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless editor document integrating the RFC 0002 seams (hammer.app).
//			It composes the VMF codec (hammer.formats), the history model
//			(DocumentHistory), and transactional save (SaveDocument over the
//			IFileStore port) into the RFC's reference workflow: open a VMF, edit,
//			undo/redo, save, reopen, compare. No MFC/tier0/PCH/GPU.
//
//			The in-memory authored content is a keyvalues tree for this first
//			slice; richer typed scene content (typed brushes/faces addressed by
//			NodeHandle) is a later step. Edits go through one history authority;
//			undo restores a recorded content snapshot (not a numeric inverse), per
//			the RFC. Load builds a DETACHED candidate and only publishes on success.
//
//=============================================================================//

#ifndef HAMMER_APP_EDITOR_DOCUMENT_H
#define HAMMER_APP_EDITOR_DOCUMENT_H

#include "hammer/app/document_history.h"
#include "hammer/app/save_orchestrator.h"
#include "hammer/formats/keyvalues.h"
#include "hammer/ports/file_store.h"

#include <string>
#include <vector>

namespace hammer::app
{

class EditorDocument
{
public:
	EditorDocument();

	// Parses VMF text into a detached candidate and, only on success, publishes it
	// as the document content with a fresh, saved (unmodified) history. On failure
	// the current content is left unchanged and 'error' is set.
	bool LoadFromText( const std::string &vmf, std::string &error );

	// Reads 'path' from the store and loads it (see LoadFromText).
	bool Load( ports::IFileStore &store, const std::string &path, std::string &error );

	// Serializes the content to VMF and saves it transactionally. On success the
	// history's saved position advances (document becomes unmodified).
	SaveStatus Save( ports::IFileStore &store, const std::string &path );

	// Sets/replaces a key on the first top-level block. Commits exactly one history
	// unit iff the content actually changed; a no-op change adds no history and
	// does not mark the document modified. Returns whether it changed.
	bool SetFirstBlockKey( const std::string &key, const std::string &value );

	bool CanUndo() const { return m_history.CanUndo(); }
	bool CanRedo() const { return m_history.CanRedo(); }

	// Undo/redo restore the recorded content snapshot for the new history position.
	bool Undo();
	bool Redo();

	bool IsModified() const { return m_history.IsModified(); }

	const formats::KeyValueNode &Content() const { return m_content; }

private:
	void ResetTo( formats::KeyValueNode content );
	void CommitSnapshot( formats::KeyValueNode next );

	formats::KeyValueNode m_content;                // published authored content
	std::vector<formats::KeyValueNode> m_snapshots; // snapshots[pos] == content at history position
	DocumentHistory m_history;
};

} // namespace hammer::app

#endif // HAMMER_APP_EDITOR_DOCUMENT_H
