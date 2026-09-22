//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Thin GTK4/libadwaita UI sibling for the Hammer editor (RFC 0002,
//			hammer.adapters.gtk). This is one of the two sibling shells the
//			architecture calls for: it holds ONLY toolkit glue -- widgets, signal
//			handlers, and rendering of state -- and delegates every editor decision
//			to the reusable, MFC-free libraries. The MFC shell
//			(hammer.adapters.mfc) is its sibling and drives the SAME libraries.
//
//			Scope note: this sibling is deliberately the ENTITY-EDITING shell -- it
//			wires New/Open/Save/Undo/Redo plus entity selection and multi-selection
//			property editing through hammer::app::EditorDocument and the shared
//			entity editor authority (hammer::app::EntitySelection +
//			EditorDocument::AggregateProperty / SetPropertyOnSelection). The full
//			Hammer 4-viewport layout with a real Source-material renderer and the
//			Block/Selection *geometry* tools (hammer::app::EditorController) is the
//			separate hammer/gtk product. The two must not duplicate a layout; keep
//			this one minimal and focused on the entity/property-editing seam.
//
//			Editor core (no GTK): hammer::app::EditorDocument owns content, history
//			(undo/redo), and transactional save; the entity editor turns a
//			selection over top-level entities into the empty-vs-unset-vs-mixed
//			property display and an atomic multi-entity edit. hammer::formats
//			serializes; the document persists through hammer::ports::IFileStore,
//			provided here by the real hammer::adapters::platform::DiskFileStore.
//			Nothing in this file implements editor policy; it maps GtkListBox
//			selection and button clicks to document calls and document state to
//			widgets. That is what makes the shell "thin".
//
//			Build (headless compile/link is sufficient to prove the sibling binds
//			to the libraries; running needs a display). See
//			unittests/hammertest/adapters/build_gtk_shell.sh for the exact command,
//			and unittests/hammertest/adapters/test_shell_workflow.cpp for the
//			headless verification of the composition this shell drives.
//
//=============================================================================//

#include "hammer/app/editor_document.h"
#include "hammer/app/entity_selection.h"
#include "hammer/app/property_value.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/formats/keyvalues.h"

#include <adwaita.h>
#include <gtk/gtk.h>

#include <cstddef>
#include <string>

namespace
{

// The shell's state: the reusable editor core plus the widgets it renders into,
// plus the selection (pure view state the core consumes). No editor policy here.
struct HammerShell
{
	hammer::app::EditorDocument document;
	hammer::adapters::platform::DiskFileStore store;
	hammer::app::EntitySelection selection; // which entities the list has picked
	std::string path = "untitled.vmf";
	bool syncing = false; // guards list-repopulation against the selection handler

	GtkTextView *content = nullptr;
	GtkListBox *entityList = nullptr;
	GtkWidget *status = nullptr;
	GtkEntry *keyEntry = nullptr;
	GtkEntry *valueEntry = nullptr;
	GtkWidget *undoButton = nullptr;
	GtkWidget *redoButton = nullptr;
};

const char *const kSeedDocument =
    "world\n{\n\t\"classname\" \"worldspawn\"\n}\n"
    "entity\n{\n\t\"classname\" \"info_player_start\"\n\t\"targetname\" \"spawn\"\n}\n";

// Reflects the aggregate value of the key across the current selection into the
// value entry, using the shared empty-vs-unset-vs-mixed distinction. The shell
// does not compute this itself; it asks the core.
void ShowAggregateValue( HammerShell *shell )
{
	const char *key = gtk_editable_get_text( GTK_EDITABLE( shell->keyEntry ) );
	if ( key == nullptr || *key == '\0' || shell->selection.Empty() )
	{
		gtk_entry_set_placeholder_text( shell->valueEntry, "value" );
		return;
	}

	const hammer::app::PropertyValue value =
	    shell->document.AggregateProperty( shell->selection, key );
	shell->syncing = true;
	if ( value.IsMixed() )
	{
		gtk_editable_set_text( GTK_EDITABLE( shell->valueEntry ), "" );
		gtk_entry_set_placeholder_text( shell->valueEntry, "<multiple values>" );
	}
	else if ( value.IsSingle() )
	{
		gtk_editable_set_text( GTK_EDITABLE( shell->valueEntry ), value.Value().c_str() );
	}
	else // Unset
	{
		gtk_editable_set_text( GTK_EDITABLE( shell->valueEntry ), "" );
		gtk_entry_set_placeholder_text( shell->valueEntry, "(unset)" );
	}
	shell->syncing = false;
}

// Rebuilds the selection from the list's currently-selected rows, then refreshes
// the aggregate display. Reads only the core's public enumeration.
void RebuildSelection( HammerShell *shell )
{
	shell->selection.Clear();
	GList *rows = gtk_list_box_get_selected_rows( shell->entityList );
	for ( GList *r = rows; r != nullptr; r = r->next )
	{
		auto *row = static_cast<GtkListBoxRow *>( r->data );
		const int index = gtk_list_box_row_get_index( row );
		if ( index >= 0 )
		{
			shell->selection.Add( static_cast<std::size_t>( index ) );
		}
	}
	g_list_free( rows );
	ShowAggregateValue( shell );
}

void OnSelectionChanged( GtkListBox *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	if ( shell->syncing )
	{
		return; // list is being repopulated; ignore transient selection churn
	}
	RebuildSelection( shell );
}

// Repopulates the entity list from the document (one row per top-level entity),
// labelled "name · classname". Called after any load that changes the entity set.
void RebuildEntityList( HammerShell *shell )
{
	shell->syncing = true;
	GtkWidget *child = gtk_widget_get_first_child( GTK_WIDGET( shell->entityList ) );
	while ( child != nullptr )
	{
		GtkWidget *next = gtk_widget_get_next_sibling( child );
		gtk_list_box_remove( shell->entityList, child );
		child = next;
	}

	for ( std::size_t i = 0; i < shell->document.EntityCount(); ++i )
	{
		std::string label = shell->document.EntityName( i );
		const std::string &classname = shell->document.EntityClassName( i );
		if ( !classname.empty() )
		{
			label += "  \xC2\xB7  " + classname; // middle dot separator
		}
		gtk_list_box_append( shell->entityList, gtk_label_new( label.c_str() ) );
	}
	shell->syncing = false;

	shell->selection.Clear();
	ShowAggregateValue( shell );
}

// Push document state into the widgets. This is the only place the UI reads the
// core, and it reads only public accessors.
void Refresh( HammerShell *shell )
{
	const std::string text = hammer::formats::WriteKeyValues( shell->document.Content() );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer( shell->content );
	gtk_text_buffer_set_text( buffer, text.c_str(), -1 );

	std::string status = shell->path;
	status += shell->document.IsModified() ? "  \xE2\x80\xA2 modified" : "  (saved)";
	gtk_label_set_text( GTK_LABEL( shell->status ), status.c_str() );

	gtk_widget_set_sensitive( shell->undoButton, shell->document.CanUndo() );
	gtk_widget_set_sensitive( shell->redoButton, shell->document.CanRedo() );

	ShowAggregateValue( shell );
}

void OnNew( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	std::string error;
	shell->document.LoadFromText( kSeedDocument, error ); // core decides; UI just calls
	RebuildEntityList( shell );
	Refresh( shell );
}

void OnSetKey( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	const char *key = gtk_editable_get_text( GTK_EDITABLE( shell->keyEntry ) );
	const char *value = gtk_editable_get_text( GTK_EDITABLE( shell->valueEntry ) );
	if ( key != nullptr && *key != '\0' )
	{
		// One atomic edit across every selected entity; the core owns the policy.
		shell->document.SetPropertyOnSelection( shell->selection, key,
		    value != nullptr ? value : "" );
		Refresh( shell );
	}
}

void OnKeyChanged( GtkEditable *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	if ( !shell->syncing )
	{
		ShowAggregateValue( shell ); // retarget the aggregate display to the new key
	}
}

void OnUndo( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	shell->document.Undo();
	Refresh( shell );
}

void OnRedo( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	shell->document.Redo();
	Refresh( shell );
}

void OnSave( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	shell->document.Save( shell->store, shell->path );
	Refresh( shell );
}

void OnOpen( GtkButton *, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );
	std::string error;
	shell->document.Load( shell->store, shell->path, error );
	RebuildEntityList( shell );
	Refresh( shell );
}

GtkWidget *ToolButton( const char *label, GCallback handler, HammerShell *shell )
{
	GtkWidget *button = gtk_button_new_with_label( label );
	g_signal_connect( button, "clicked", handler, shell );
	return button;
}

void OnActivate( GtkApplication *app, gpointer user_data )
{
	auto *shell = static_cast<HammerShell *>( user_data );

	GtkWidget *window = adw_application_window_new( app );
	gtk_window_set_title( GTK_WINDOW( window ), "Hammer (GTK sibling)" );
	gtk_window_set_default_size( GTK_WINDOW( window ), 820, 560 );

	GtkWidget *toolbar = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
	gtk_widget_set_margin_top( toolbar, 6 );
	gtk_widget_set_margin_bottom( toolbar, 6 );
	gtk_widget_set_margin_start( toolbar, 6 );
	gtk_widget_set_margin_end( toolbar, 6 );

	shell->keyEntry = GTK_ENTRY( gtk_entry_new() );
	gtk_entry_set_placeholder_text( shell->keyEntry, "key" );
	g_signal_connect( shell->keyEntry, "changed", G_CALLBACK( OnKeyChanged ), shell );
	shell->valueEntry = GTK_ENTRY( gtk_entry_new() );
	gtk_entry_set_placeholder_text( shell->valueEntry, "value" );
	shell->undoButton = ToolButton( "Undo", G_CALLBACK( OnUndo ), shell );
	shell->redoButton = ToolButton( "Redo", G_CALLBACK( OnRedo ), shell );

	gtk_box_append( GTK_BOX( toolbar ), ToolButton( "New", G_CALLBACK( OnNew ), shell ) );
	gtk_box_append( GTK_BOX( toolbar ), GTK_WIDGET( shell->keyEntry ) );
	gtk_box_append( GTK_BOX( toolbar ), GTK_WIDGET( shell->valueEntry ) );
	gtk_box_append( GTK_BOX( toolbar ), ToolButton( "Set Key", G_CALLBACK( OnSetKey ), shell ) );
	gtk_box_append( GTK_BOX( toolbar ), shell->undoButton );
	gtk_box_append( GTK_BOX( toolbar ), shell->redoButton );
	gtk_box_append( GTK_BOX( toolbar ), ToolButton( "Open", G_CALLBACK( OnOpen ), shell ) );
	gtk_box_append( GTK_BOX( toolbar ), ToolButton( "Save", G_CALLBACK( OnSave ), shell ) );

	// Left: the entity list (multi-select drives the shared entity editor).
	shell->entityList = GTK_LIST_BOX( gtk_list_box_new() );
	gtk_list_box_set_selection_mode( shell->entityList, GTK_SELECTION_MULTIPLE );
	g_signal_connect( shell->entityList, "selected-rows-changed",
	    G_CALLBACK( OnSelectionChanged ), shell );
	GtkWidget *listScroller = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( listScroller ),
	    GTK_WIDGET( shell->entityList ) );
	gtk_widget_set_size_request( listScroller, 220, -1 );

	// Right: the serialized document (read-only mirror of the core's content).
	shell->content = GTK_TEXT_VIEW( gtk_text_view_new() );
	gtk_text_view_set_editable( shell->content, FALSE );
	gtk_text_view_set_monospace( shell->content, TRUE );
	GtkWidget *scroller = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( scroller ), GTK_WIDGET( shell->content ) );
	gtk_widget_set_hexpand( scroller, TRUE );

	GtkWidget *panes = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_append( GTK_BOX( panes ), listScroller );
	gtk_box_append( GTK_BOX( panes ), scroller );
	gtk_widget_set_vexpand( panes, TRUE );

	shell->status = gtk_label_new( "" );
	gtk_widget_set_halign( shell->status, GTK_ALIGN_START );
	gtk_widget_set_margin_start( shell->status, 6 );
	gtk_widget_set_margin_bottom( shell->status, 6 );

	GtkWidget *content = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
	GtkWidget *header = adw_header_bar_new();
	gtk_box_append( GTK_BOX( content ), header );
	gtk_box_append( GTK_BOX( content ), toolbar );
	gtk_box_append( GTK_BOX( content ), panes );
	gtk_box_append( GTK_BOX( content ), shell->status );

	adw_application_window_set_content( ADW_APPLICATION_WINDOW( window ), content );

	std::string error;
	shell->document.LoadFromText( kSeedDocument, error ); // start on a fresh document
	RebuildEntityList( shell );
	Refresh( shell );

	gtk_window_present( GTK_WINDOW( window ) );
}

} // namespace

int main( int argc, char **argv )
{
	HammerShell shell;

	AdwApplication *app = adw_application_new( "org.valve.HammerGtk", G_APPLICATION_DEFAULT_FLAGS );
	g_signal_connect( app, "activate", G_CALLBACK( OnActivate ), &shell );

	const int status = g_application_run( G_APPLICATION( app ), argc, argv );
	g_object_unref( app );
	return status;
}
