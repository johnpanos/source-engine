//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The status bar presentation model (RFC 0002, hammer.presenters):
//			the texts legacy Hammer's and Source 2's status bars show, with no
//			toolkit.
//
//			Selection summary: counts of the selected objects by kind, in the
//			order solids, entities, groups, faces, joined by ", " with
//			singular/plural nouns; the entity count names the class when every
//			selected entity has the same one: "2 solids, 1 entity (light)",
//			"3 entities", "1 group, 4 faces". Nothing selected: "No selection".
//			Objects are counted as selected (a group counts once, not its
//			members).
//
//			Size: the extent of the selection's bounds (scene::ObjectsBounds of
//			the objects, extended by the selected faces' polygons) as
//			"w <x extent> h <z extent> d <y extent>" -- width, height (Z is up)
//			and depth -- with scene::FormatNumber numbers; "" when the selection
//			has no extent.
//
//			Pointer: the host passes the world point under the pointer and the
//			view it is in; 2D views show their two axes (Top "x 12 y -4", Front
//			"x .. z ..", Side "y .. z .."), the 3D view all three. Values are
//			rounded to 0.01. "" until set or after ClearPointer().
//
//			Grid and snap: read live from app::EditorSettings, their one owner:
//			"Grid 64", "Snap on" / "Snap off".
//
//			Tool: the active tool's name, provided by the host.
//
//			Message: a transient slot for operation errors (ShowError uses the
//			EditError text) or notices; it stays until ClearMessage(), a new
//			message, or the next committed change to the document (edit, undo,
//			redo, replacement).
//
//			The subscription is RAII; the status bar may be destroyed before or
//			after its session, but calls need a live session.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_STATUS_BAR_H
#define HAMMER_PRESENTERS_STATUS_BAR_H

#include "foundation/expected.h"
#include "hammer/app/edit_session.h"
#include "hammer/app/editor_settings.h"
#include "hammer/viewport/camera.h"
#include "mapgeometry/brush.h"

#include <cstdint>
#include <string>

namespace hammer::presenters
{

// The selection summary text for 'selection' in 'doc' (see above).
std::string SelectionSummary( const scene::DocumentReader &doc, const app::Selection &selection );
// The size text for 'selection' in 'doc' (see above).
std::string SelectionSizeText( const scene::DocumentReader &doc, const app::Selection &selection );

class StatusBar
{
public:
	StatusBar( app::EditSession &session, const app::EditorSettings &settings );
	StatusBar( const StatusBar & ) = delete;
	StatusBar &operator=( const StatusBar & ) = delete;

	std::uint64_t Revision() const { return m_revision; }

	const std::string &SelectionText() const { return m_selectionText; }
	const std::string &SizeText() const { return m_sizeText; }

	const std::string &PointerText() const { return m_pointerText; }
	void SetPointer( viewport::ViewKind view, const mapgeometry::Vec3d &world );
	void ClearPointer();

	std::string GridText() const;
	std::string SnapText() const;

	const std::string &ToolText() const { return m_toolText; }
	void SetTool( const std::string &name );

	const std::string &Message() const { return m_message; }
	void ShowError( const app::EditError &error );
	void ShowMessage( const std::string &message );
	// Shows the error of a failed result; a success leaves the slot alone.
	void Report( const foundation::Expected<void, app::EditError> &result );
	void ClearMessage();

private:
	void Rebuild( bool documentChanged );

	app::EditSession &m_session;
	const app::EditorSettings &m_settings;
	app::SessionSubscription m_subscription;
	std::uint64_t m_revision = 0;
	std::string m_selectionText;
	std::string m_sizeText;
	std::string m_pointerText;
	std::string m_toolText;
	std::string m_message;
};

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_STATUS_BAR_H
