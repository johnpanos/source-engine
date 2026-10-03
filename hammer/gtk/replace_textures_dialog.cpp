//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/replace_textures_dialog.h.
//
//=============================================================================//

#include "replace_textures_dialog.h"

#include <memory>
#include <vector>

namespace hammer::gtk
{

namespace
{

using presenters::ReplaceTexturesDialog;
using Match = app::ops::MaterialMatch;

void SetAccessibleName( GtkWidget *widget, const char *name )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_LABEL, name, -1 );
}

GtkWidget *Frame( const char *title, GtkWidget *child )
{
	GtkWidget *frame = gtk_frame_new( title );
	gtk_widget_set_margin_top( child, 4 );
	gtk_widget_set_margin_bottom( child, 4 );
	gtk_widget_set_margin_start( child, 6 );
	gtk_widget_set_margin_end( child, 6 );
	gtk_frame_set_child( GTK_FRAME( frame ), child );
	gtk_widget_set_hexpand( frame, TRUE );
	return frame;
}

struct ReplaceWindow
{
	presenters::EditorWorkspace &workspace;
	ReplaceTexturesDialog &model;
	ReplaceTexturesDone done;
	GtkWidget *window = nullptr;
	GtkWidget *find = nullptr;
	GtkWidget *usedList = nullptr;
	GtkWidget *replace = nullptr;
	GtkWidget *candidateList = nullptr;
	GtkWidget *inMarked = nullptr;
	GtkWidget *everything = nullptr;
	GtkWidget *hidden = nullptr;
	GtkWidget *exact = nullptr;
	GtkWidget *partial = nullptr;
	GtkWidget *substitute = nullptr;
	GtkWidget *markOnly = nullptr;
	GtkWidget *rescale = nullptr;
	GtkWidget *preview = nullptr;
	GtkWidget *status = nullptr;
	GtkWidget *ok = nullptr;
	std::vector<std::string> usedNames;
	std::vector<std::string> candidateNames;
	bool updating = false;

	ReplaceWindow( presenters::EditorWorkspace &w, ReplaceTexturesDone d )
	    : workspace( w ), model( w.ReplaceTextures() ), done( std::move( d ) )
	{
	}

	// Widgets -> draft.
	void Read()
	{
		ReplaceTexturesDialog::DraftValues &d = model.Draft();
		d.find = gtk_editable_get_text( GTK_EDITABLE( find ) );
		d.replace = gtk_editable_get_text( GTK_EDITABLE( replace ) );
		d.scope = gtk_check_button_get_active( GTK_CHECK_BUTTON( inMarked ) )
		              ? ReplaceTexturesDialog::Scope::Selection
		              : ReplaceTexturesDialog::Scope::Everything;
		d.includeHidden = gtk_check_button_get_active( GTK_CHECK_BUTTON( hidden ) );
		d.match = gtk_check_button_get_active( GTK_CHECK_BUTTON( substitute ) ) ? Match::Substitute
		          : gtk_check_button_get_active( GTK_CHECK_BUTTON( partial ) )  ? Match::Partial
		                                                                        : Match::Exact;
		d.markOnly = gtk_check_button_get_active( GTK_CHECK_BUTTON( markOnly ) );
		d.rescale = gtk_check_button_get_active( GTK_CHECK_BUTTON( rescale ) );
	}

	// Draft -> widgets (at open).
	void Write()
	{
		updating = true;
		const ReplaceTexturesDialog::DraftValues &d = model.Draft();
		gtk_editable_set_text( GTK_EDITABLE( find ), d.find.c_str() );
		gtk_editable_set_text( GTK_EDITABLE( replace ), d.replace.c_str() );
		gtk_check_button_set_active(
		    GTK_CHECK_BUTTON( inMarked ), d.scope == ReplaceTexturesDialog::Scope::Selection );
		gtk_check_button_set_active(
		    GTK_CHECK_BUTTON( everything ), d.scope == ReplaceTexturesDialog::Scope::Everything );
		gtk_widget_set_sensitive( inMarked, model.SelectionScopeAvailable() );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( hidden ), d.includeHidden );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( exact ), d.match == Match::Exact );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( partial ), d.match == Match::Partial );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( substitute ), d.match == Match::Substitute );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( markOnly ), d.markOnly );
		gtk_check_button_set_label( GTK_CHECK_BUTTON( markOnly ),
		    model.MarksFaces() ? "Do not replace textures (mark found faces)"
		                       : "Do not replace textures (mark found solids)" );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( rescale ), d.rescale );
		gtk_widget_set_sensitive( rescale, model.RescaleAvailable() );
		updating = false;
	}

	// The model's verdicts -> preview, status, sensitivity.
	void Refresh()
	{
		const ReplaceTexturesDialog::PreviewCount count = model.Preview();
		const std::string text = "Matches " + std::to_string( count.faces ) + " faces in " +
		                         std::to_string( count.solids ) + " solids";
		gtk_label_set_text( GTK_LABEL( preview ), text.c_str() );
		const std::string why = model.Validate();
		gtk_label_set_text( GTK_LABEL( status ), why.c_str() );
		gtk_widget_set_sensitive( ok, why.empty() );
		gtk_widget_set_sensitive( hidden, model.HiddenAvailable() );
		gtk_widget_set_sensitive( replace, !model.Draft().markOnly );
		gtk_widget_set_sensitive( candidateList, !model.Draft().markOnly );
	}

	void Changed()
	{
		if ( updating )
			return;
		Read();
		Refresh();
	}

	void Apply()
	{
		Read();
		const ReplaceTexturesDialog::Outcome outcome = model.Apply();
		if ( !outcome.done )
		{
			gtk_label_set_text( GTK_LABEL( status ), outcome.message.c_str() );
			return;
		}
		if ( done )
			done( outcome.message );
		gtk_window_destroy( GTK_WINDOW( window ) );
	}

	static void OnChanged( GtkWidget *, gpointer data )
	{
		static_cast<ReplaceWindow *>( data )->Changed();
	}
	static void OnToggled( GtkCheckButton *, gpointer data )
	{
		static_cast<ReplaceWindow *>( data )->Changed();
	}
	static void OnPicked( GtkDropDown *list, GParamSpec *, gpointer data )
	{
		auto *self = static_cast<ReplaceWindow *>( data );
		const guint item = gtk_drop_down_get_selected( list );
		const bool isFind = GTK_WIDGET( list ) == self->usedList;
		const std::vector<std::string> &names = isFind ? self->usedNames : self->candidateNames;
		// Item 0 is the list's title.
		if ( self->updating || item == 0 || item == GTK_INVALID_LIST_POSITION ||
		     item > names.size() )
			return;
		gtk_editable_set_text(
		    GTK_EDITABLE( isFind ? self->find : self->replace ), names[item - 1].c_str() );
	}
	static void OnOk( GtkButton *, gpointer data )
	{
		static_cast<ReplaceWindow *>( data )->Apply();
	}
	static void OnCancel( GtkButton *, gpointer data )
	{
		gtk_window_destroy( GTK_WINDOW( static_cast<ReplaceWindow *>( data )->window ) );
	}
	static gboolean OnEscape( GtkWidget *, GVariant *, gpointer data )
	{
		gtk_window_destroy( GTK_WINDOW( static_cast<ReplaceWindow *>( data )->window ) );
		return TRUE;
	}
};

GtkWidget *MaterialList(
    const char *accessibleName, const char *title, const std::vector<std::string> &names )
{
	GtkStringList *model = gtk_string_list_new( nullptr );
	gtk_string_list_append( model, title );
	for ( const std::string &name : names )
		gtk_string_list_append( model, name.c_str() );
	GtkWidget *list = gtk_drop_down_new( G_LIST_MODEL( model ), nullptr );
	gtk_drop_down_set_enable_search( GTK_DROP_DOWN( list ), TRUE );
	gtk_drop_down_set_expression( GTK_DROP_DOWN( list ),
	    gtk_property_expression_new( GTK_TYPE_STRING_OBJECT, nullptr, "string" ) );
	gtk_drop_down_set_search_match_mode(
	    GTK_DROP_DOWN( list ), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING );
	SetAccessibleName( list, accessibleName );
	return list;
}

GtkWidget *Radio( const char *label, GtkWidget *group )
{
	GtkWidget *button = gtk_check_button_new_with_label( label );
	if ( group )
		gtk_check_button_set_group( GTK_CHECK_BUTTON( button ), GTK_CHECK_BUTTON( group ) );
	SetAccessibleName( button, label );
	return button;
}

} // namespace

void ShowReplaceTexturesDialog(
    GtkWindow *parent, presenters::EditorWorkspace &workspace, ReplaceTexturesDone done )
{
	workspace.OpenReplaceTextures();
	auto *self = new ReplaceWindow( workspace, std::move( done ) );

	self->window = GTK_WIDGET(
	    g_object_new( GTK_TYPE_WINDOW, "accessible-role", GTK_ACCESSIBLE_ROLE_DIALOG, nullptr ) );
	gtk_window_set_title( GTK_WINDOW( self->window ), "Replace Textures" );
	gtk_window_set_transient_for( GTK_WINDOW( self->window ), parent );
	gtk_window_set_modal( GTK_WINDOW( self->window ), TRUE );
	gtk_window_set_destroy_with_parent( GTK_WINDOW( self->window ), TRUE );
	gtk_window_set_resizable( GTK_WINDOW( self->window ), FALSE );
	g_object_set_data_full( G_OBJECT( self->window ), "replace-textures-state", self,
	    []( gpointer state )
	    {
		    delete static_cast<ReplaceWindow *>( state );
	    } );

	GtkEventController *keys = gtk_shortcut_controller_new();
	gtk_shortcut_controller_add_shortcut( GTK_SHORTCUT_CONTROLLER( keys ),
	    gtk_shortcut_new( gtk_keyval_trigger_new( GDK_KEY_Escape, GdkModifierType( 0 ) ),
	        gtk_callback_action_new( ReplaceWindow::OnEscape, self, nullptr ) ) );
	gtk_widget_add_controller( self->window, keys );

	GtkWidget *root = gtk_box_new( GTK_ORIENTATION_VERTICAL, 8 );
	gtk_widget_set_margin_start( root, 12 );
	gtk_widget_set_margin_end( root, 12 );
	gtk_widget_set_margin_top( root, 12 );
	gtk_widget_set_margin_bottom( root, 12 );
	gtk_window_set_child( GTK_WINDOW( self->window ), root );

	// Find and Replace with fields, each with its material list.
	for ( const bool isFind : { true, false } )
	{
		GtkWidget *row = gtk_box_new( GTK_ORIENTATION_VERTICAL, 4 );
		GtkWidget *label = gtk_label_new( isFind ? "Find:" : "Replace with:" );
		gtk_label_set_xalign( GTK_LABEL( label ), 0.0f );
		gtk_box_append( GTK_BOX( row ), label );
		GtkWidget *fields = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
		GtkWidget *entry = gtk_entry_new();
		gtk_widget_set_hexpand( entry, TRUE );
		// Enter activates OK, the default widget, when it is sensitive.
		gtk_entry_set_activates_default( GTK_ENTRY( entry ), TRUE );
		SetAccessibleName( entry, isFind ? "Find" : "Replace with" );
		g_signal_connect( entry, "changed", G_CALLBACK( ReplaceWindow::OnChanged ), self );
		gtk_box_append( GTK_BOX( fields ), entry );
		GtkWidget *list = nullptr;
		if ( isFind )
		{
			for ( const auto &used : self->model.UsedMaterials() )
				self->usedNames.push_back( used.name );
			list = MaterialList( "Used materials", "Used materials...", self->usedNames );
			self->find = entry;
			self->usedList = list;
		}
		else
		{
			self->candidateNames = self->model.Candidates();
			list = MaterialList( "Materials", "Materials...", self->candidateNames );
			self->replace = entry;
			self->candidateList = list;
		}
		g_signal_connect( list, "notify::selected", G_CALLBACK( ReplaceWindow::OnPicked ), self );
		gtk_box_append( GTK_BOX( fields ), list );
		gtk_box_append( GTK_BOX( row ), fields );
		gtk_box_append( GTK_BOX( root ), row );
	}

	// Replace In and Action, side by side as in legacy.
	GtkWidget *groups = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 8 );
	GtkWidget *scope = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
	self->inMarked = Radio( "Marked objects", nullptr );
	self->everything = Radio( "Everything", self->inMarked );
	self->hidden = gtk_check_button_new_with_label( "Hidden objects too" );
	SetAccessibleName( self->hidden, "Hidden objects too" );
	gtk_box_append( GTK_BOX( scope ), self->inMarked );
	gtk_box_append( GTK_BOX( scope ), self->everything );
	gtk_box_append( GTK_BOX( scope ), self->hidden );
	gtk_box_append( GTK_BOX( groups ), Frame( "Replace In", scope ) );
	GtkWidget *action = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
	self->exact = Radio( "Replace exact matches", nullptr );
	self->partial = Radio( "Replace partial matches", self->exact );
	self->substitute = Radio( "Substitute partial matches", self->exact );
	gtk_box_append( GTK_BOX( action ), self->exact );
	gtk_box_append( GTK_BOX( action ), self->partial );
	gtk_box_append( GTK_BOX( action ), self->substitute );
	gtk_box_append( GTK_BOX( groups ), Frame( "Action", action ) );
	gtk_box_append( GTK_BOX( root ), groups );

	self->markOnly = gtk_check_button_new_with_label( "" );
	SetAccessibleName( self->markOnly, "Do not replace textures" );
	gtk_box_append( GTK_BOX( root ), self->markOnly );
	self->rescale =
	    gtk_check_button_new_with_label( "Rescale texture coordinates for new texture size" );
	SetAccessibleName( self->rescale, "Rescale texture coordinates for new texture size" );
	gtk_box_append( GTK_BOX( root ), self->rescale );
	for ( GtkWidget *button : { self->inMarked, self->everything, self->hidden, self->exact,
	          self->partial, self->substitute, self->markOnly, self->rescale } )
		g_signal_connect( button, "toggled", G_CALLBACK( ReplaceWindow::OnToggled ), self );

	self->preview = gtk_label_new( "" );
	gtk_label_set_xalign( GTK_LABEL( self->preview ), 0.0f );
	gtk_widget_add_css_class( self->preview, "dim-label" );
	gtk_box_append( GTK_BOX( root ), self->preview );
	self->status = gtk_label_new( "" );
	gtk_label_set_xalign( GTK_LABEL( self->status ), 0.0f );
	gtk_label_set_wrap( GTK_LABEL( self->status ), TRUE );
	gtk_widget_add_css_class( self->status, "error" );
	gtk_box_append( GTK_BOX( root ), self->status );

	GtkWidget *buttons = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
	gtk_widget_set_halign( buttons, GTK_ALIGN_END );
	GtkWidget *cancel = gtk_button_new_with_label( "Cancel" );
	g_signal_connect( cancel, "clicked", G_CALLBACK( ReplaceWindow::OnCancel ), self );
	self->ok = gtk_button_new_with_label( "OK" );
	gtk_widget_add_css_class( self->ok, "suggested-action" );
	g_signal_connect( self->ok, "clicked", G_CALLBACK( ReplaceWindow::OnOk ), self );
	gtk_box_append( GTK_BOX( buttons ), cancel );
	gtk_box_append( GTK_BOX( buttons ), self->ok );
	gtk_box_append( GTK_BOX( root ), buttons );
	gtk_window_set_default_widget( GTK_WINDOW( self->window ), self->ok );

	self->Write();
	self->Refresh();
	gtk_window_present( GTK_WINDOW( self->window ) );
	gtk_widget_grab_focus( self->replace );
}

} // namespace hammer::gtk
