//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's Replace Textures window (RFC 0002,
//			linux-gtk-desktop; legacy Tools > Replace Textures..., and the
//			Texture Application window's Replace... button). A modal window
//			bound entirely to presenters::ReplaceTexturesDialog: each widget
//			change writes the model's draft, and the preview, the status and
//			OK's sensitivity are read back from it. Matching, naming, scope,
//			marking, rescale and the messages belong to the model and
//			ops::ReplaceMaterial; this window holds widgets only.
//
//			Layout follows legacy's dialog: Find and Replace with fields
//			(each with a material list: the map's used materials for Find,
//			the catalog's for Replace, as the legacy Browse buttons were),
//			Replace In (Marked objects / Everything, Hidden objects too),
//			Action (exact, partial, substitute), mark only, rescale, then a
//			live preview ("Matches N faces in M solids") and Cancel / OK.
//
//			OK applies. When the request ran (a replacement, marks, or nothing
//			matched) the window closes and the host shows the model's message
//			on the status bar; a refusal stays open with its reason. Escape
//			and Cancel close without changes.
//
//			Every control's accessible name is its legacy label, so tests
//			find them by name; the preview and status labels are named by their
//			text.
//
//=============================================================================//

#ifndef HAMMER_GTK_REPLACE_TEXTURES_DIALOG_H
#define HAMMER_GTK_REPLACE_TEXTURES_DIALOG_H

#include "hammer/presenters/editor_workspace.h"

#include <functional>
#include <string>

#include <gtk/gtk.h>

namespace hammer::gtk
{

// Called once the window applied a request: the host refreshes its views and
// shows 'message'.
using ReplaceTexturesDone = std::function<void( const std::string &message )>;

// Opens the model (EditorWorkspace::OpenReplaceTextures) and shows the
// window over 'parent'. 'workspace' must outlive the window.
void ShowReplaceTexturesDialog(
    GtkWindow *parent, presenters::EditorWorkspace &workspace, ReplaceTexturesDone done );

} // namespace hammer::gtk

#endif // HAMMER_GTK_REPLACE_TEXTURES_DIALOG_H
