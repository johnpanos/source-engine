//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Replace Textures dialog model (RFC 0002, hammer.presenters;
//			legacy CReplaceTexDlg and CMapDoc::OnEditReplacetex). A host shows
//			the draft, binds its widgets to Draft() and calls Apply(); every
//			rule lives here or in ops::ReplaceMaterial, so a GTK window, a test
//			and an agent get the same behavior.
//
//			Open() starts a draft with the legacy defaults: Find is the current
//			material (EditorSettings::faceTexture), Replace is empty, the scope
//			is the selected objects when there are any (otherwise everything,
//			and the selection scope is unavailable), exact matching, and mark
//			only, hidden objects and rescale off.
//
//			Apply() runs one command through SessionCommands:
//			  replace_material (one "Replace Textures" undo step), or
//			  mark_material when 'markOnly' (a selection change: marks solids,
//			  or faces when the face tool was active at Open, as legacy did
//			  with the face-edit material tool). Unlike legacy, marking within
//			  the selected objects marks within them; legacy cleared the
//			  selection before reading it and marked nothing.
//			The result message is the legacy text: "N textures replaced.",
//			"N solids marked." or "N faces marked." A refusal is not applied
//			and its reason is the message.
//
//			Names follow RFC 0015's identity rule (see ops/texture_ops.h).
//			UsedMaterials() lists the materials the document uses (the legacy
//			Find browser showed used textures), Candidates() the catalog's
//			names (the Replace browser), and Preview() what Apply would touch.
//
//			Threading: the session's sequence.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_REPLACE_TEXTURES_H
#define HAMMER_PRESENTERS_REPLACE_TEXTURES_H

#include "hammer/app/edit_session.h"
#include "hammer/app/ops/texture_ops.h"
#include "hammer/app/session_commands.h"
#include "hammer/ports/material_info.h"

#include <string>
#include <vector>

namespace hammer::presenters
{

class ReplaceTexturesDialog
{
public:
	enum class Scope
	{
		Selection, // the selected objects (legacy "Marked objects")
		Everything,
	};

	struct DraftValues
	{
		std::string find;
		std::string replace;
		Scope scope = Scope::Everything;
		app::ops::MaterialMatch match = app::ops::MaterialMatch::Exact;
		bool markOnly = false;
		bool includeHidden = false;
		bool rescale = false;
	};

	struct PreviewCount
	{
		int faces = 0;
		int solids = 0;
	};

	struct UsedMaterial
	{
		std::string name; // the normalized identity
		int faces = 0;
	};

	struct Outcome
	{
		bool applied = false; // the document or selection changed
		bool done = false;    // the request ran (applied, or nothing matched):
		                      // a dialog closes; a refusal keeps it open
		std::string message;
	};

	// 'materials' may be null: rescaling is then unavailable.
	ReplaceTexturesDialog( app::EditSession &session, app::SessionCommands &commands,
	    const ports::IMaterialInfo *materials );

	void Open( const std::string &currentMaterial, bool faceToolActive );
	DraftValues &Draft() { return m_draft; }
	const DraftValues &Draft() const { return m_draft; }

	bool SelectionScopeAvailable() const;
	bool RescaleAvailable() const { return m_materials != nullptr; }
	// Hidden objects are never marked (legacy forced it off with mark only).
	bool HiddenAvailable() const { return !m_draft.markOnly; }
	bool MarksFaces() const { return m_marksFaces; }

	// Empty when Apply may run; otherwise why not, for the dialog to show.
	std::string Validate() const;
	PreviewCount Preview() const;
	std::vector<UsedMaterial> UsedMaterials() const;
	std::vector<std::string> Candidates() const;

	Outcome Apply();

private:
	app::ops::MaterialReplace Query() const;
	std::string IdsArgument() const;

	app::EditSession &m_session;
	app::SessionCommands &m_commands;
	const ports::IMaterialInfo *m_materials;
	DraftValues m_draft;
	bool m_marksFaces = false;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_REPLACE_TEXTURES_H
