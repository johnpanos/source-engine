//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's Visgroups panel (RFC 0002, linux-gtk-desktop;
//			RFC 0018 F6; legacy Hammer's VisGroups tab, CFilterControl). The
//			panel is bound entirely to the workspace's presenters::VisgroupPanel:
//			it shows the presenter's rows and turns each widget action into one
//			presenter call, so every change is one undo step through the command
//			layer and visibility reaches the viewports through the document
//			(scene::IsVisible, the scene revision and the render snapshot),
//			never through a filter here.
//
//			  * The tree: one row per visgroup in pre-order, indented by depth,
//			    with an expander for groups with children, a show/hide check
//			    (inconsistent for Mixed: some members hidden, in the group or
//			    its children; insensitive for an empty group), the member count
//			    and a mark when the selection holds members (all or some).
//			  * Actions (buttons, and the same items on a row's right-click
//			    menu): New (a visgroup holding the selection, legacy's default
//			    name "N objects"), Add and Remove the selection, Move the
//			    selection (exclusive, legacy "move to visgroup"), Mark (select
//			    the members), Rename, Delete, and Move To (under another
//			    visgroup or the top level). Dragging a row onto another row
//			    moves it under that visgroup; onto the list's empty area, to
//			    the top level.
//			  * It follows the document: every session event (edits, undo,
//			    redo, open and replace) refreshes it on the main loop.
//
//			Accessible names: the list "Visgroups"; each check box is named for
//			its visgroup (its description gives the member count and state);
//			the buttons "New Visgroup", "Add Selection to Visgroup", "Remove
//			Selection from Visgroup", "Move Selection to Visgroup", "Select
//			Visgroup Members", "Rename Visgroup", "Delete Visgroup" and "Move
//			Visgroup To"; the rename field "Visgroup name".
//
//=============================================================================//

#ifndef HAMMER_GTK_VISGROUPS_PANEL_H
#define HAMMER_GTK_VISGROUPS_PANEL_H

#include "hammer/presenters/editor_workspace.h"

#include <functional>
#include <string>

#include <gtk/gtk.h>

namespace hammer::gtk
{

// Called after the panel changed the document or reported a refusal: the host
// refreshes its views and shows 'status' when it is not empty.
using VisgroupsChanged = std::function<void( const std::string &status )>;

// Builds the panel. 'workspace' must outlive the returned widget; the panel's
// state goes with the widget.
GtkWidget *MakeVisgroupsPanel( presenters::EditorWorkspace &workspace, VisgroupsChanged changed );

} // namespace hammer::gtk

#endif // HAMMER_GTK_VISGROUPS_PANEL_H
