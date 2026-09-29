//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's Object Properties window (RFC 0002,
//			linux-gtk-desktop; legacy Hammer's Object Properties sheet, Alt+Enter,
//			and the Source 2 Object Properties panel). A non-modal window bound
//			entirely to the workspace's presenters::EntityInspector: it shows the
//			inspector's class, rows and flags and turns each widget change into
//			one inspector call. Aggregation, validation, the draft, class
//			choices, colour text and every edit belong to the inspector; this
//			window holds widgets only.
//
//			  * Class: a searchable dropdown of EntityInspector::ClassChoices();
//			    picking one is SetClass (one undo step).
//			  * SmartEdit (on): a typed editor per row from the FGD type the row
//			    reports: text, numbers, a choice dropdown, a check box for
//			    booleans, a colour button beside color255/color1 text. Off: raw
//			    key/value text, with Add and per-key Remove as in legacy.
//			  * Edits of key values are the inspector's draft: Apply commits it
//			    as one undo step over every selected entity (Enter in a text
//			    field applies too); Cancel discards it. Flags toggle at once.
//			  * Mixed values show as "(different values)"; a mixed flag or
//			    boolean is an inconsistent check box.
//			  * It follows the selection while open (session events refresh it
//			    on the main loop). Escape in a drafted field reverts that
//			    field; elsewhere it closes the window, and closing settles the
//			    draft (EntityInspector::Settle), as RFC 0018 F4 describes.
//
//			Every editor's accessible name is its key, so tests find it by key.
//
//=============================================================================//

#ifndef HAMMER_GTK_PROPERTIES_DIALOG_H
#define HAMMER_GTK_PROPERTIES_DIALOG_H

#include "hammer/presenters/editor_workspace.h"

#include <functional>
#include <string>

#include <gtk/gtk.h>

namespace hammer::gtk
{

// Called after the window changed the document or reported something: the
// host refreshes its views and shows 'status' when it is not empty.
using PropertiesChanged = std::function<void( const std::string &status )>;

// Shows the Object Properties window of 'parent', creating it on first use
// (one per parent; it is destroyed with the parent). 'workspace' must outlive
// the parent window.
void ShowPropertiesDialog(
    GtkWindow *parent, presenters::EditorWorkspace &workspace, PropertiesChanged changed );

} // namespace hammer::gtk

#endif // HAMMER_GTK_PROPERTIES_DIALOG_H
