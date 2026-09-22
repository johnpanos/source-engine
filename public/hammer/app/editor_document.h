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
#include "hammer/app/entity_selection.h"
#include "hammer/app/property_value.h"
#include "hammer/app/save_orchestrator.h"
#include "hammer/formats/keyvalues.h"
#include "hammer/ports/file_store.h"

#include <cstddef>
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
	//
	// This is a thin convenience over the entity-editor path below (a selection of
	// entity 0); it is retained for the existing single-target callers.
	bool SetFirstBlockKey( const std::string &key, const std::string &value );

	// --- Entity editor (shared by both UI shells) --------------------------------
	//
	// The document's "entities" are its top-level blocks (worldspawn and each
	// entity/group block), addressed by a stable index in document order. A UI
	// shell owns a selection (EntitySelection) over these indices; the aggregation
	// and mutation policy below is the single authority both siblings route through,
	// so multi-selection editing behaves identically regardless of toolkit.

	// Number of top-level entities (== Content().children.size()).
	std::size_t EntityCount() const { return m_content.children.size(); }

	// The block name of entity 'index' (e.g. "world", "entity"), or "" if out of
	// range. This is the VMF block keyword, not a property.
	const std::string &EntityName( std::size_t index ) const;

	// The "classname" property of entity 'index', or "" if the entity has no
	// classname or the index is out of range.
	const std::string &EntityClassName( std::size_t index ) const;

	// Aggregates property 'key' across the selection into the RFC's
	// empty-vs-unset-vs-mixed distinction (see PropertyValue). Policy (explicit):
	//   * Only entities that DEFINE the key contribute -- an entity lacking the key
	//     is a non-contributor, matching PropertyValue::Unset ("no selected object
	//     contributes this property at all").
	//   * An entity whose value is the empty string DOES contribute "" (empty is a
	//     legitimate single value, distinct from unset).
	//   * Single iff every contributor agrees; Mixed iff contributors disagree.
	// Out-of-range indices are ignored. An empty selection aggregates to Unset.
	PropertyValue AggregateProperty(
	    const EntitySelection &selection, const std::string &key ) const;

	// Sets 'key'='value' on every selected entity, adding the key where absent.
	// Commits exactly ONE history unit iff at least one selected entity's content
	// actually changed -- a multi-entity edit is a single undo step. A no-op edit
	// (every selected entity already has that value) records nothing and does not
	// mark the document modified. Out-of-range indices are ignored. Returns whether
	// the document changed.
	bool SetPropertyOnSelection(
	    const EntitySelection &selection, const std::string &key, const std::string &value );

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
