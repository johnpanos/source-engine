//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The GTK Hammer shell's Visgroups panel; see visgroups_panel.h.
//
//=============================================================================//

#include "visgroups_panel.h"

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

namespace hammer::gtk
{

namespace
{

using app::ops::VisgroupState;
using presenters::SelectedMembers;
using presenters::VisgroupPanel;
using presenters::VisgroupRow;

constexpr const char *kIdKey = "hammer-visgroup-id";
constexpr int kIndent = 16;

void SetAccessibleName( GtkWidget *widget, const std::string &name )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_LABEL, name.c_str(), -1 );
}

void SetAccessibleDescription( GtkWidget *widget, const std::string &text )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, text.c_str(), -1 );
}

// A label that can shrink to an ellipsis, so the panel never widens the
// object bar (and so narrows the views).
GtkWidget *ShrinkingLabel( const char *text )
{
	GtkWidget *label = gtk_label_new( text );
	gtk_label_set_ellipsize( GTK_LABEL( label ), PANGO_ELLIPSIZE_END );
	return label;
}

int IdOf( gpointer widget )
{
	return GPOINTER_TO_INT( g_object_get_data( G_OBJECT( widget ), kIdKey ) );
}

const char *StateText( VisgroupState state )
{
	switch ( state )
	{
	case VisgroupState::Shown:
		return "shown";
	case VisgroupState::Hidden:
		return "hidden";
	case VisgroupState::Mixed:
		return "partly hidden";
	case VisgroupState::Empty:
		break;
	}
	return "empty";
}

std::string ObjectCount( std::size_t count )
{
	return std::to_string( count ) + ( count == 1 ? " object" : " objects" );
}

class VisgroupsView
{
public:
	VisgroupsView( presenters::EditorWorkspace &workspace, VisgroupsChanged changed );
	GtkWidget *Root() const { return m_root; }

private:
	VisgroupPanel &Panel() { return m_workspace.Visgroups(); }
	const VisgroupRow *CurrentRow();
	bool HasSelection() const { return !m_workspace.Session().CurrentSelection().objects.empty(); }

	void ScheduleRefresh();
	void Refresh();
	GtkWidget *MakeRow( const VisgroupRow &row );
	void RebuildMoveMenu();
	void UpdateActions();
	void Enable( const char *action, bool enabled );
	// Reports one presenter call to the host: its refusal, or 'done'.
	void Report( const app::CommandResult &result, const std::string &done );
	void MoveUnder( int visgroupId, int parentId );

	static void OnDestroy( GtkWidget *, gpointer data );
	static void OnRowSelected( GtkListBox *, GtkListBoxRow *row, gpointer data );
	static void OnCheckToggled( GtkCheckButton *check, gpointer data );
	static void OnExpander( GtkButton *button, gpointer data );
	static void OnRowPressed(
	    GtkGestureClick *gesture, int presses, double x, double y, gpointer data );
	static GdkContentProvider *OnDragPrepare(
	    GtkDragSource *source, double x, double y, gpointer data );
	static gboolean OnDropOnRow(
	    GtkDropTarget *target, const GValue *value, double x, double y, gpointer data );
	static gboolean OnDropOnList(
	    GtkDropTarget *target, const GValue *value, double x, double y, gpointer data );
	static void OnRenameShown( GtkWidget *, gpointer data );
	static void OnRenameActivate( GtkEntry *, gpointer data );
	static void OnAction( GSimpleAction *action, GVariant *parameter, gpointer data );

	presenters::EditorWorkspace &m_workspace;
	VisgroupsChanged m_changed;
	app::SessionSubscription m_subscription;
	guint m_idle = 0;
	bool m_dead = false;     // the panel is being destroyed
	bool m_updating = false; // widgets are being set from the presenter
	int m_current = 0;       // the visgroup the actions apply to (0: none)

	GtkWidget *m_root = nullptr;
	GtkWidget *m_list = nullptr;
	GtkWidget *m_renameButton = nullptr;
	GtkWidget *m_renameEntry = nullptr;
	GtkWidget *m_moveButton = nullptr;
	GtkWidget *m_context = nullptr;
	GSimpleActionGroup *m_actions = nullptr;
	GMenu *m_moveMenu = nullptr;
};

VisgroupsView::VisgroupsView( presenters::EditorWorkspace &workspace, VisgroupsChanged changed )
    : m_workspace( workspace ), m_changed( std::move( changed ) )
{
	m_root = gtk_box_new( GTK_ORIENTATION_VERTICAL, 4 );
	gtk_widget_set_vexpand( m_root, TRUE );
	g_signal_connect( m_root, "destroy", G_CALLBACK( OnDestroy ), this );

	// Actions: the buttons and the row menu share them.
	m_actions = g_simple_action_group_new();
	static const char *const kPlain[] = {
	    "new", "add", "remove", "move-selection", "mark", "rename", "delete" };
	for ( const char *name : kPlain )
	{
		GSimpleAction *action = g_simple_action_new( name, nullptr );
		g_signal_connect( action, "activate", G_CALLBACK( OnAction ), this );
		g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
		g_object_unref( action );
	}
	GSimpleAction *moveTo = g_simple_action_new( "move-to", G_VARIANT_TYPE_INT32 );
	g_signal_connect( moveTo, "activate", G_CALLBACK( OnAction ), this );
	g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( moveTo ) );
	g_object_unref( moveTo );
	gtk_widget_insert_action_group( m_root, "vg", G_ACTION_GROUP( m_actions ) );
	g_object_unref( m_actions );

	GtkWidget *title = gtk_label_new( "VisGroups:" );
	gtk_label_set_xalign( GTK_LABEL( title ), 0.0f );
	gtk_box_append( GTK_BOX( m_root ), title );

	GtkWidget *scroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand( scroll, TRUE );
	gtk_scrolled_window_set_policy(
	    GTK_SCROLLED_WINDOW( scroll ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
	m_list = gtk_list_box_new();
	gtk_list_box_set_selection_mode( GTK_LIST_BOX( m_list ), GTK_SELECTION_SINGLE );
	SetAccessibleName( m_list, "Visgroups" );
	GtkWidget *placeholder = gtk_label_new( "No visgroups" );
	gtk_widget_add_css_class( placeholder, "dim-label" );
	gtk_list_box_set_placeholder( GTK_LIST_BOX( m_list ), placeholder );
	g_signal_connect( m_list, "row-selected", G_CALLBACK( OnRowSelected ), this );
	// Rows are drop targets for "move under"; the list's empty area is the top
	// level.
	GtkDropTarget *drop = gtk_drop_target_new( G_TYPE_INT, GDK_ACTION_MOVE );
	g_signal_connect( drop, "drop", G_CALLBACK( OnDropOnList ), this );
	gtk_widget_add_controller( scroll, GTK_EVENT_CONTROLLER( drop ) );
	// Keys on the list: Delete deletes the visgroup, F2 renames it.
	GtkEventController *keys = gtk_shortcut_controller_new();
	gtk_shortcut_controller_add_shortcut( GTK_SHORTCUT_CONTROLLER( keys ),
	    gtk_shortcut_new( gtk_keyval_trigger_new( GDK_KEY_Delete, GdkModifierType( 0 ) ),
	        gtk_named_action_new( "vg.delete" ) ) );
	gtk_shortcut_controller_add_shortcut( GTK_SHORTCUT_CONTROLLER( keys ),
	    gtk_shortcut_new( gtk_keyval_trigger_new( GDK_KEY_F2, GdkModifierType( 0 ) ),
	        gtk_named_action_new( "vg.rename" ) ) );
	gtk_widget_add_controller( m_list, keys );
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( scroll ), m_list );
	gtk_box_append( GTK_BOX( m_root ), scroll );

	struct ButtonSpec
	{
		const char *label;
		const char *name;
		const char *action;
		const char *tip;
	};
	auto buttonRow = [this]( std::initializer_list<ButtonSpec> specs )
	{
		GtkWidget *row = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
		gtk_widget_add_css_class( row, "linked" );
		for ( const ButtonSpec &spec : specs )
		{
			GtkWidget *b = gtk_button_new();
			gtk_button_set_child( GTK_BUTTON( b ), ShrinkingLabel( spec.label ) );
			gtk_widget_set_hexpand( b, TRUE );
			gtk_widget_set_tooltip_text( b, spec.tip );
			gtk_actionable_set_action_name( GTK_ACTIONABLE( b ), spec.action );
			SetAccessibleName( b, spec.name );
			gtk_box_append( GTK_BOX( row ), b );
		}
		gtk_box_append( GTK_BOX( m_root ), row );
		return row;
	};
	buttonRow( { { "New", "New Visgroup", "vg.new", "New visgroup holding the selection" },
	    { "Add", "Add Selection to Visgroup", "vg.add", "Add the selection to the visgroup" },
	    { "Remove", "Remove Selection from Visgroup", "vg.remove",
	        "Remove the selection from the visgroup" },
	    { "Mark", "Select Visgroup Members", "vg.mark", "Select the visgroup's objects" } } );
	GtkWidget *edit = buttonRow( { { "Move Sel.", "Move Selection to Visgroup", "vg.move-selection",
	                                   "Move the selection to the visgroup (out of all others)" },
	    { "Delete", "Delete Visgroup", "vg.delete", "Delete the visgroup (its objects stay)" } } );

	// Rename: a popover with the name field.
	m_renameButton = gtk_menu_button_new();
	gtk_menu_button_set_child( GTK_MENU_BUTTON( m_renameButton ), ShrinkingLabel( "Rename" ) );
	gtk_menu_button_set_always_show_arrow( GTK_MENU_BUTTON( m_renameButton ), TRUE );
	gtk_widget_set_hexpand( m_renameButton, TRUE );
	gtk_widget_set_tooltip_text( m_renameButton, "Rename the visgroup (F2)" );
	SetAccessibleName( m_renameButton, "Rename Visgroup" );
	GtkWidget *renamePopover = gtk_popover_new();
	m_renameEntry = gtk_entry_new();
	SetAccessibleName( m_renameEntry, "Visgroup name" );
	g_signal_connect( m_renameEntry, "activate", G_CALLBACK( OnRenameActivate ), this );
	gtk_popover_set_child( GTK_POPOVER( renamePopover ), m_renameEntry );
	g_signal_connect( renamePopover, "show", G_CALLBACK( OnRenameShown ), this );
	gtk_menu_button_set_popover( GTK_MENU_BUTTON( m_renameButton ), renamePopover );
	gtk_box_prepend( GTK_BOX( edit ), m_renameButton );

	// Move To: the visgroups this one may move under, and the top level.
	m_moveMenu = g_menu_new();
	m_moveButton = gtk_menu_button_new();
	gtk_menu_button_set_child( GTK_MENU_BUTTON( m_moveButton ), ShrinkingLabel( "Move To" ) );
	gtk_menu_button_set_always_show_arrow( GTK_MENU_BUTTON( m_moveButton ), TRUE );
	gtk_widget_set_hexpand( m_moveButton, TRUE );
	gtk_widget_set_tooltip_text( m_moveButton, "Move the visgroup under another (or drag it)" );
	SetAccessibleName( m_moveButton, "Move Visgroup To" );
	gtk_menu_button_set_menu_model( GTK_MENU_BUTTON( m_moveButton ), G_MENU_MODEL( m_moveMenu ) );
	gtk_box_append( GTK_BOX( edit ), m_moveButton );

	// The row menu (right click): the same actions.
	GMenu *menu = g_menu_new();
	GMenu *members = g_menu_new();
	g_menu_append( members, "New Visgroup from Selection", "vg.new" );
	g_menu_append( members, "Add Selection", "vg.add" );
	g_menu_append( members, "Remove Selection", "vg.remove" );
	g_menu_append( members, "Move Selection Here", "vg.move-selection" );
	g_menu_append( members, "Select Members", "vg.mark" );
	g_menu_append_section( menu, nullptr, G_MENU_MODEL( members ) );
	g_object_unref( members );
	GMenu *tree = g_menu_new();
	g_menu_append( tree, "Rename…", "vg.rename" );
	g_menu_append( tree, "Delete", "vg.delete" );
	g_menu_append_submenu( tree, "Move To", G_MENU_MODEL( m_moveMenu ) );
	g_menu_append_section( menu, nullptr, G_MENU_MODEL( tree ) );
	g_object_unref( tree );
	m_context = gtk_popover_menu_new_from_model( G_MENU_MODEL( menu ) );
	g_object_unref( menu );
	gtk_popover_set_has_arrow( GTK_POPOVER( m_context ), FALSE );
	gtk_widget_set_parent( m_context, m_root ); // a GtkBox unparents it on dispose

	// Follow the document: every session event refreshes the panel on the
	// main loop, after the presenter has rebuilt.
	m_subscription = m_workspace.Session().Subscribe(
	    [this]( const app::SessionEvent & )
	    {
		    ScheduleRefresh();
	    } );
	Refresh();
}

const VisgroupRow *VisgroupsView::CurrentRow()
{
	const std::optional<std::size_t> row = Panel().FindRow( m_current );
	return row ? &Panel().Rows()[*row] : nullptr;
}

void VisgroupsView::ScheduleRefresh()
{
	if ( m_idle || m_dead )
	{
		return;
	}
	m_idle = g_idle_add(
	    []( gpointer data ) -> gboolean
	    {
		    auto *self = static_cast<VisgroupsView *>( data );
		    self->m_idle = 0;
		    self->Refresh();
		    return G_SOURCE_REMOVE;
	    },
	    this );
}

GtkWidget *VisgroupsView::MakeRow( const VisgroupRow &row )
{
	GtkWidget *item = gtk_list_box_row_new();
	g_object_set_data( G_OBJECT( item ), kIdKey, GINT_TO_POINTER( row.id ) );
	SetAccessibleName( item, row.name );

	GtkWidget *box = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 4 );
	gtk_widget_set_margin_start( box, 2 + kIndent * row.depth );
	if ( row.childCount > 0 )
	{
		GtkWidget *expander = gtk_button_new_from_icon_name(
		    row.expanded ? "pan-down-symbolic" : "pan-end-symbolic" );
		gtk_button_set_has_frame( GTK_BUTTON( expander ), FALSE );
		gtk_widget_set_focus_on_click( expander, FALSE );
		g_object_set_data( G_OBJECT( expander ), kIdKey, GINT_TO_POINTER( row.id ) );
		SetAccessibleName( expander, ( row.expanded ? "Collapse " : "Expand " ) + row.name );
		g_signal_connect( expander, "clicked", G_CALLBACK( OnExpander ), this );
		gtk_box_append( GTK_BOX( box ), expander );
	}
	else
	{
		GtkWidget *spacer = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
		gtk_widget_set_size_request( spacer, 26, -1 );
		gtk_box_append( GTK_BOX( box ), spacer );
	}

	// Show/hide: legacy's check per visgroup, indeterminate while some of its
	// members (in it or its children) are hidden.
	GtkWidget *check = gtk_check_button_new();
	gtk_check_button_set_active(
	    GTK_CHECK_BUTTON( check ), row.visibility == VisgroupState::Shown );
	gtk_check_button_set_inconsistent(
	    GTK_CHECK_BUTTON( check ), row.visibility == VisgroupState::Mixed );
	gtk_widget_set_sensitive( check, row.visibility != VisgroupState::Empty );
	gtk_widget_set_tooltip_text( check, "Show or hide the visgroup's objects" );
	g_object_set_data( G_OBJECT( check ), kIdKey, GINT_TO_POINTER( row.id ) );
	SetAccessibleName( check, row.name );
	SetAccessibleDescription(
	    check, ObjectCount( row.memberCount ) + ", " + StateText( row.visibility ) );
	g_signal_connect( check, "toggled", G_CALLBACK( OnCheckToggled ), this );
	gtk_box_append( GTK_BOX( box ), check );

	GtkWidget *name = gtk_label_new( row.name.c_str() );
	gtk_label_set_xalign( GTK_LABEL( name ), 0.0f );
	gtk_label_set_ellipsize( GTK_LABEL( name ), PANGO_ELLIPSIZE_END );
	gtk_widget_set_hexpand( name, TRUE );
	gtk_box_append( GTK_BOX( box ), name );

	// The selection's share of the members: legacy marks a visgroup whose
	// members are selected.
	if ( row.selected != SelectedMembers::None )
	{
		const bool all = row.selected == SelectedMembers::All;
		GtkWidget *mark = gtk_label_new( all ? "●" : "◐" );
		gtk_widget_set_tooltip_text(
		    mark, all ? "Every member is selected" : "Some members are selected" );
		gtk_box_append( GTK_BOX( box ), mark );
	}
	GtkWidget *count = gtk_label_new( std::to_string( row.memberCount ).c_str() );
	gtk_widget_add_css_class( count, "dim-label" );
	gtk_widget_set_tooltip_text( count, ObjectCount( row.memberCount ).c_str() );
	gtk_box_append( GTK_BOX( box ), count );
	gtk_list_box_row_set_child( GTK_LIST_BOX_ROW( item ), box );

	GtkGesture *click = gtk_gesture_click_new();
	gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( click ), GDK_BUTTON_SECONDARY );
	g_signal_connect( click, "pressed", G_CALLBACK( OnRowPressed ), this );
	gtk_widget_add_controller( item, GTK_EVENT_CONTROLLER( click ) );

	GtkDragSource *drag = gtk_drag_source_new();
	gtk_drag_source_set_actions( drag, GDK_ACTION_MOVE );
	g_signal_connect( drag, "prepare", G_CALLBACK( OnDragPrepare ), this );
	gtk_widget_add_controller( item, GTK_EVENT_CONTROLLER( drag ) );
	GtkDropTarget *drop = gtk_drop_target_new( G_TYPE_INT, GDK_ACTION_MOVE );
	g_signal_connect( drop, "drop", G_CALLBACK( OnDropOnRow ), this );
	gtk_widget_add_controller( item, GTK_EVENT_CONTROLLER( drop ) );
	return item;
}

void VisgroupsView::Refresh()
{
	if ( m_dead )
	{
		return;
	}
	m_updating = true;
	const std::vector<VisgroupRow> &rows = Panel().Rows();
	if ( !Panel().FindRow( m_current ) )
	{
		m_current = 0;
	}
	gtk_list_box_remove_all( GTK_LIST_BOX( m_list ) );
	// A row is listed when every ancestor is expanded.
	std::vector<bool> listed( rows.size(), false );
	GtkWidget *current = nullptr;
	for ( std::size_t i = 0; i < rows.size(); ++i )
	{
		const VisgroupRow &row = rows[i];
		listed[i] = !row.parent || ( listed[*row.parent] && rows[*row.parent].expanded );
		if ( !listed[i] )
		{
			continue;
		}
		GtkWidget *item = MakeRow( row );
		gtk_list_box_append( GTK_LIST_BOX( m_list ), item );
		if ( row.id == m_current )
		{
			current = item;
		}
	}
	if ( current )
	{
		gtk_list_box_select_row( GTK_LIST_BOX( m_list ), GTK_LIST_BOX_ROW( current ) );
	}
	RebuildMoveMenu();
	UpdateActions();
	m_updating = false;
}

void VisgroupsView::RebuildMoveMenu()
{
	g_menu_remove_all( m_moveMenu );
	if ( !m_current )
	{
		return;
	}
	GMenu *top = g_menu_new();
	GMenuItem *level = g_menu_item_new( "Top Level", nullptr );
	g_menu_item_set_action_and_target_value( level, "vg.move-to", g_variant_new_int32( 0 ) );
	g_menu_append_item( top, level );
	g_object_unref( level );
	g_menu_append_section( m_moveMenu, nullptr, G_MENU_MODEL( top ) );
	g_object_unref( top );
	GMenu *groups = g_menu_new();
	for ( int id : Panel().MoveTargets( m_current ) )
	{
		const VisgroupRow &row = Panel().Rows()[*Panel().FindRow( id )];
		const std::string label = std::string( 2 * row.depth, ' ' ) + row.name;
		GMenuItem *item = g_menu_item_new( label.c_str(), nullptr );
		g_menu_item_set_action_and_target_value( item, "vg.move-to", g_variant_new_int32( id ) );
		g_menu_append_item( groups, item );
		g_object_unref( item );
	}
	g_menu_append_section( m_moveMenu, nullptr, G_MENU_MODEL( groups ) );
	g_object_unref( groups );
}

void VisgroupsView::Enable( const char *action, bool enabled )
{
	GAction *a = g_action_map_lookup_action( G_ACTION_MAP( m_actions ), action );
	g_simple_action_set_enabled( G_SIMPLE_ACTION( a ), enabled );
}

void VisgroupsView::UpdateActions()
{
	const VisgroupRow *row = CurrentRow();
	const bool selection = HasSelection();
	Enable( "new", selection );
	Enable( "add", row && selection );
	Enable( "remove", row && selection && row->selected != SelectedMembers::None );
	Enable( "move-selection", row && selection );
	Enable( "mark", row && row->memberCount > 0 );
	Enable( "rename", row != nullptr );
	Enable( "delete", row != nullptr );
	Enable( "move-to", row != nullptr );
	gtk_widget_set_sensitive( m_renameButton, row != nullptr );
	gtk_widget_set_sensitive( m_moveButton, row != nullptr );
}

void VisgroupsView::Report( const app::CommandResult &result, const std::string &done )
{
	if ( m_changed )
	{
		m_changed( result ? done : result.Error().command + ": " + result.Error().detail );
	}
	ScheduleRefresh();
}

void VisgroupsView::MoveUnder( int visgroupId, int parentId )
{
	const VisgroupRow *row = nullptr;
	if ( const std::optional<std::size_t> index = Panel().FindRow( visgroupId ) )
	{
		row = &Panel().Rows()[*index];
	}
	if ( !row )
	{
		return;
	}
	const std::string name = row->name;
	std::string where = "the top level";
	if ( parentId )
	{
		const std::optional<std::size_t> parent = Panel().FindRow( parentId );
		where = parent ? "'" + Panel().Rows()[*parent].name + "'" : where;
	}
	m_current = visgroupId;
	Report( Panel().Move( visgroupId, parentId ), "Moved visgroup '" + name + "' under " + where );
}

// ---- Signal handlers ---------------------------------------------------------

void VisgroupsView::OnDestroy( GtkWidget *, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	self->m_dead = true;
	self->m_subscription = app::SessionSubscription();
	if ( self->m_idle )
	{
		g_source_remove( self->m_idle );
		self->m_idle = 0;
	}
	g_clear_object( &self->m_moveMenu );
	// Children are disposed after this handler and may still signal; the
	// handlers see m_dead. The object goes once the main loop is idle.
	g_idle_add_once(
	    []( gpointer state )
	    {
		    delete static_cast<VisgroupsView *>( state );
	    },
	    self );
}

void VisgroupsView::OnRowSelected( GtkListBox *, GtkListBoxRow *row, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead || self->m_updating )
	{
		return;
	}
	self->m_current = row ? IdOf( row ) : 0;
	self->RebuildMoveMenu();
	self->UpdateActions();
}

void VisgroupsView::OnCheckToggled( GtkCheckButton *check, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead || self->m_updating )
	{
		return;
	}
	const int id = IdOf( check );
	const std::optional<std::size_t> index = self->Panel().FindRow( id );
	if ( !index )
	{
		return;
	}
	const std::string name = self->Panel().Rows()[*index].name;
	// Shown hides; Hidden or Mixed shows (legacy: a partly hidden group's
	// check shows it).
	const bool hide = self->Panel().Rows()[*index].visibility == VisgroupState::Shown;
	self->Report( self->Panel().ToggleVisible( id ),
	    ( hide ? "Hid visgroup '" : "Showed visgroup '" ) + name + "'" );
}

void VisgroupsView::OnExpander( GtkButton *button, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead )
	{
		return;
	}
	const int id = IdOf( button );
	self->Panel().SetExpanded( id, !self->Panel().IsExpanded( id ) );
	self->ScheduleRefresh();
}

void VisgroupsView::OnRowPressed( GtkGestureClick *gesture, int, double x, double y, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead )
	{
		return;
	}
	GtkWidget *row = gtk_event_controller_get_widget( GTK_EVENT_CONTROLLER( gesture ) );
	gtk_list_box_select_row( GTK_LIST_BOX( self->m_list ), GTK_LIST_BOX_ROW( row ) );
	graphene_point_t from;
	graphene_point_init( &from, float( x ), float( y ) );
	graphene_point_t at;
	if ( !gtk_widget_compute_point( row, self->m_root, &from, &at ) )
	{
		return;
	}
	const GdkRectangle rect{ int( at.x ), int( at.y ), 1, 1 };
	gtk_popover_set_pointing_to( GTK_POPOVER( self->m_context ), &rect );
	gtk_popover_popup( GTK_POPOVER( self->m_context ) );
}

GdkContentProvider *VisgroupsView::OnDragPrepare( GtkDragSource *source, double, double, gpointer )
{
	GtkWidget *row = gtk_event_controller_get_widget( GTK_EVENT_CONTROLLER( source ) );
	return gdk_content_provider_new_typed( G_TYPE_INT, IdOf( row ) );
}

gboolean VisgroupsView::OnDropOnRow(
    GtkDropTarget *target, const GValue *value, double, double, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead || !G_VALUE_HOLDS_INT( value ) )
	{
		return FALSE;
	}
	const int dragged = g_value_get_int( value );
	const int onto = IdOf( gtk_event_controller_get_widget( GTK_EVENT_CONTROLLER( target ) ) );
	if ( dragged == onto )
	{
		return FALSE;
	}
	self->MoveUnder( dragged, onto );
	return TRUE;
}

gboolean VisgroupsView::OnDropOnList(
    GtkDropTarget *, const GValue *value, double, double, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead || !G_VALUE_HOLDS_INT( value ) )
	{
		return FALSE;
	}
	self->MoveUnder( g_value_get_int( value ), 0 );
	return TRUE;
}

void VisgroupsView::OnRenameShown( GtkWidget *, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	const VisgroupRow *row = self->CurrentRow();
	gtk_editable_set_text( GTK_EDITABLE( self->m_renameEntry ), row ? row->name.c_str() : "" );
	gtk_widget_grab_focus( self->m_renameEntry );
}

void VisgroupsView::OnRenameActivate( GtkEntry *entry, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	const VisgroupRow *row = self->CurrentRow();
	if ( self->m_dead || !row )
	{
		return;
	}
	const std::string name = gtk_editable_get_text( GTK_EDITABLE( entry ) );
	gtk_menu_button_popdown( GTK_MENU_BUTTON( self->m_renameButton ) );
	if ( name == row->name )
	{
		return;
	}
	self->Report( self->Panel().Rename( row->id, name ), "Renamed visgroup to '" + name + "'" );
}

void VisgroupsView::OnAction( GSimpleAction *action, GVariant *parameter, gpointer data )
{
	auto *self = static_cast<VisgroupsView *>( data );
	if ( self->m_dead )
	{
		return;
	}
	const std::string name = g_action_get_name( G_ACTION( action ) );
	VisgroupPanel &panel = self->Panel();
	if ( name == "new" )
	{
		// Legacy "new visgroup" from the selection, named "N objects", at the
		// top level, shown; one undo step.
		const std::string label = panel.SelectionVisgroupName();
		app::CommandResult made = panel.CreateFromSelection( label );
		if ( made )
		{
			self->m_current = std::atoi( made.Value().c_str() );
		}
		self->Report( made, "Created visgroup '" + label + "'" );
		return;
	}
	const VisgroupRow *row = self->CurrentRow();
	if ( !row )
	{
		return;
	}
	const int id = row->id;
	const std::string group = "'" + row->name + "'";
	if ( name == "add" )
	{
		self->Report( panel.AddSelection( id ), "Added the selection to " + group );
	}
	else if ( name == "remove" )
	{
		self->Report( panel.RemoveSelection( id ), "Removed the selection from " + group );
	}
	else if ( name == "move-selection" )
	{
		self->Report( panel.AddSelection( id, true ), "Moved the selection to " + group );
	}
	else if ( name == "mark" )
	{
		self->Report( panel.SelectMembers( id ), "Selected the members of " + group );
	}
	else if ( name == "rename" )
	{
		gtk_menu_button_popup( GTK_MENU_BUTTON( self->m_renameButton ) );
	}
	else if ( name == "delete" )
	{
		// One undo step, so no confirmation (legacy asked; Undo restores it).
		self->Report( panel.Delete( id ), "Deleted visgroup " + group );
	}
	else if ( name == "move-to" && parameter )
	{
		self->MoveUnder( id, g_variant_get_int32( parameter ) );
	}
}

} // namespace

GtkWidget *MakeVisgroupsPanel( presenters::EditorWorkspace &workspace, VisgroupsChanged changed )
{
	auto *view = new VisgroupsView( workspace, std::move( changed ) );
	return view->Root();
}

} // namespace hammer::gtk
