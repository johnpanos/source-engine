//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/gtk/properties_dialog.h.
//
//=============================================================================//

#include "properties_dialog.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace hammer::gtk
{

namespace
{

using presenters::EntityInspector;
using presenters::FlagRow;
using presenters::FlagState;
using presenters::KeyRow;

constexpr const char *kWindowKey = "hammer-properties-window";
constexpr const char *kMixedText = "(different values)";

enum class EditorKind
{
	Text,
	Choice,
	Check,
	Color,
};

EditorKind EditorFor( const KeyRow &row, bool smartEdit )
{
	if ( !smartEdit )
	{
		return EditorKind::Text;
	}
	switch ( row.type )
	{
	case ports::KeyType::Choices:
		return row.choices.empty() ? EditorKind::Text : EditorKind::Choice;
	case ports::KeyType::Boolean:
		return EditorKind::Check;
	case ports::KeyType::Color255:
	case ports::KeyType::Color1:
		return EditorKind::Color;
	default:
		return EditorKind::Text;
	}
}

void SetAccessibleName( GtkWidget *widget, const std::string &name )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_LABEL, name.c_str(), -1 );
}

void SetAccessibleDescription( GtkWidget *widget, const char *text )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, text, -1 );
}

// Keeps a dropdown's leading placeholder item (a value that is none of its
// items: mixed, not set, or free text) in step with 'text', then selects
// 'item' (an index into the items after the placeholder) or the placeholder.
// A GtkDropDown always shows a selected item, so an absent or mixed value
// needs an item of its own rather than an invalid position.
void ShowSelection(
    GtkWidget *dropDown, bool &placeholder, const std::string &text, std::optional<guint> item )
{
	GtkStringList *model = GTK_STRING_LIST( gtk_drop_down_get_model( GTK_DROP_DOWN( dropDown ) ) );
	const bool want = !item.has_value();
	if ( want )
	{
		const char *const added[] = { text.c_str(), nullptr };
		gtk_string_list_splice( model, 0, placeholder ? 1 : 0, added );
	}
	else if ( placeholder )
	{
		gtk_string_list_splice( model, 0, 1, nullptr );
	}
	placeholder = want;
	gtk_drop_down_set_selected( GTK_DROP_DOWN( dropDown ), want ? 0 : *item );
}

// The index a selection names among a dropdown's items after its
// placeholder, or nothing for the placeholder.
std::optional<std::size_t> ItemOf( GtkWidget *dropDown, bool placeholder )
{
	const guint selected = gtk_drop_down_get_selected( GTK_DROP_DOWN( dropDown ) );
	if ( selected == GTK_INVALID_LIST_POSITION || ( placeholder && selected == 0 ) )
	{
		return std::nullopt;
	}
	return std::size_t( selected - ( placeholder ? 1 : 0 ) );
}

class PropertiesWindow;

// The widgets of one key row. 'baseline' is the text the row shows without a
// draft; returning an editor to it clears the row's draft.
struct RowView
{
	PropertiesWindow *owner = nullptr;
	std::string key;
	EditorKind kind = EditorKind::Text;
	GtkWidget *label = nullptr;
	GtkWidget *editor = nullptr; // GtkEntry, GtkDropDown or GtkCheckButton
	GtkWidget *color = nullptr;  // GtkColorDialogButton beside a colour's entry
	GtkWidget *error = nullptr;
	std::string baseline;
	bool placeholder = false; // the choice dropdown shows a leading placeholder item
};

struct FlagView
{
	PropertiesWindow *owner = nullptr;
	long long bit = 0;
	GtkWidget *check = nullptr;
};

class PropertiesWindow
{
public:
	PropertiesWindow(
	    GtkWindow *parent, presenters::EditorWorkspace &workspace, PropertiesChanged changed );

	GtkWidget *Window() const { return m_window; }
	void SetChanged( PropertiesChanged changed ) { m_changed = std::move( changed ); }
	void Present();

private:
	EntityInspector &Inspector() { return m_workspace.Inspector(); }

	void ScheduleRefresh();
	void Refresh();
	void RefreshClass();
	void RebuildRows();
	void UpdateRow( RowView &view, const KeyRow &row );
	void RebuildFlags();
	const KeyRow *FindRow( const std::string &key );

	// Runs one inspector call made by a widget, then refreshes and tells the
	// host when the document moved.
	template <typename Call> void Edit( Call &&call );
	void EditValue( RowView &view, const std::string &value );

	// Signal handlers.
	static void OnDestroy( GtkWidget *, gpointer data );
	static gboolean OnCloseRequest( GtkWindow *, gpointer data );
	static gboolean OnEscape( GtkWidget *, GVariant *, gpointer data );
	static void OnClassSelected( GObject *, GParamSpec *, gpointer data );
	static void OnSmartEdit( GtkCheckButton *, gpointer data );
	static void OnApply( GtkButton *, gpointer data );
	static void OnCancel( GtkButton *, gpointer data );
	static void OnAddKey( GtkButton *, gpointer data );
	static void OnTextChanged( GtkEditable *, gpointer data );
	static void OnTextActivate( GtkEntry *, gpointer data );
	static void OnChoiceSelected( GObject *, GParamSpec *, gpointer data );
	static void OnCheckToggled( GtkCheckButton *, gpointer data );
	static void OnColorPicked( GObject *, GParamSpec *, gpointer data );
	static void OnRemoveKey( GtkButton *, gpointer data );
	static void OnFlagToggled( GtkCheckButton *, gpointer data );

	presenters::EditorWorkspace &m_workspace;
	PropertiesChanged m_changed;
	app::SessionSubscription m_subscription;
	guint m_idle = 0;
	bool m_dead = false;     // the window is being destroyed
	bool m_updating = false; // widgets are being set from the inspector

	GtkWidget *m_window = nullptr;
	GtkWidget *m_summary = nullptr;
	GtkWidget *m_classes = nullptr;
	GtkWidget *m_smartEdit = nullptr;
	GtkWidget *m_rowsScroll = nullptr;
	GtkWidget *m_flagsScroll = nullptr;
	GtkWidget *m_addBox = nullptr;
	GtkWidget *m_addKey = nullptr;
	GtkWidget *m_addValue = nullptr;
	GtkWidget *m_status = nullptr;
	GtkWidget *m_apply = nullptr;
	GtkWidget *m_cancel = nullptr;

	std::vector<std::string> m_classList; // what the class dropdown lists
	bool m_classPlaceholder = false;      // ... after a placeholder item
	std::string m_rowsLayout;             // what the rows grid was built for
	std::string m_flagsLayout;
	std::vector<std::unique_ptr<RowView>> m_rows;
	std::vector<std::unique_ptr<FlagView>> m_flags;
};

PropertiesWindow::PropertiesWindow(
    GtkWindow *parent, presenters::EditorWorkspace &workspace, PropertiesChanged changed )
    : m_workspace( workspace ), m_changed( std::move( changed ) )
{
	m_window = GTK_WIDGET(
	    g_object_new( GTK_TYPE_WINDOW, "accessible-role", GTK_ACCESSIBLE_ROLE_DIALOG, nullptr ) );
	gtk_window_set_title( GTK_WINDOW( m_window ), "Object Properties" );
	gtk_window_set_transient_for( GTK_WINDOW( m_window ), parent );
	gtk_window_set_destroy_with_parent( GTK_WINDOW( m_window ), TRUE );
	gtk_window_set_hide_on_close( GTK_WINDOW( m_window ), TRUE );
	gtk_window_set_default_size( GTK_WINDOW( m_window ), 480, 600 );
	g_signal_connect( m_window, "destroy", G_CALLBACK( OnDestroy ), this );
	g_signal_connect( m_window, "close-request", G_CALLBACK( OnCloseRequest ), this );

	GtkEventController *keys = gtk_shortcut_controller_new();
	gtk_shortcut_controller_add_shortcut( GTK_SHORTCUT_CONTROLLER( keys ),
	    gtk_shortcut_new( gtk_keyval_trigger_new( GDK_KEY_Escape, GdkModifierType( 0 ) ),
	        gtk_callback_action_new( OnEscape, this, nullptr ) ) );
	gtk_widget_add_controller( m_window, keys );

	GtkWidget *root = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
	gtk_widget_set_margin_start( root, 10 );
	gtk_widget_set_margin_end( root, 10 );
	gtk_widget_set_margin_top( root, 10 );
	gtk_widget_set_margin_bottom( root, 10 );
	gtk_window_set_child( GTK_WINDOW( m_window ), root );

	// Class row: the class dropdown and the SmartEdit switch, as legacy's
	// Class Info page heads its key list.
	GtkWidget *classRow = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
	gtk_box_append( GTK_BOX( classRow ), gtk_label_new( "Class:" ) );
	m_classes = gtk_drop_down_new( G_LIST_MODEL( gtk_string_list_new( nullptr ) ), nullptr );
	gtk_drop_down_set_enable_search( GTK_DROP_DOWN( m_classes ), TRUE );
	gtk_drop_down_set_expression( GTK_DROP_DOWN( m_classes ),
	    gtk_property_expression_new( GTK_TYPE_STRING_OBJECT, nullptr, "string" ) );
	gtk_drop_down_set_search_match_mode(
	    GTK_DROP_DOWN( m_classes ), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING );
	gtk_widget_set_hexpand( m_classes, TRUE );
	SetAccessibleName( m_classes, "Class" );
	g_signal_connect( m_classes, "notify::selected", G_CALLBACK( OnClassSelected ), this );
	gtk_box_append( GTK_BOX( classRow ), m_classes );
	m_smartEdit = gtk_check_button_new_with_label( "SmartEdit" );
	g_signal_connect( m_smartEdit, "toggled", G_CALLBACK( OnSmartEdit ), this );
	gtk_box_append( GTK_BOX( classRow ), m_smartEdit );
	gtk_box_append( GTK_BOX( root ), classRow );

	m_summary = gtk_label_new( "" );
	gtk_label_set_xalign( GTK_LABEL( m_summary ), 0.0f );
	gtk_widget_add_css_class( m_summary, "dim-label" );
	gtk_box_append( GTK_BOX( root ), m_summary );

	// Pages: Class Info (the keys) and Flags, legacy's first two tabs.
	GtkWidget *pages = gtk_notebook_new();
	gtk_widget_set_vexpand( pages, TRUE );
	GtkWidget *infoPage = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
	m_rowsScroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand( m_rowsScroll, TRUE );
	gtk_box_append( GTK_BOX( infoPage ), m_rowsScroll );
	m_addBox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
	m_addKey = gtk_entry_new();
	gtk_entry_set_placeholder_text( GTK_ENTRY( m_addKey ), "key" );
	SetAccessibleName( m_addKey, "New key" );
	m_addValue = gtk_entry_new();
	gtk_entry_set_placeholder_text( GTK_ENTRY( m_addValue ), "value" );
	gtk_widget_set_hexpand( m_addValue, TRUE );
	SetAccessibleName( m_addValue, "New value" );
	GtkWidget *add = gtk_button_new_with_label( "Add" );
	g_signal_connect( add, "clicked", G_CALLBACK( OnAddKey ), this );
	gtk_box_append( GTK_BOX( m_addBox ), m_addKey );
	gtk_box_append( GTK_BOX( m_addBox ), m_addValue );
	gtk_box_append( GTK_BOX( m_addBox ), add );
	gtk_box_append( GTK_BOX( infoPage ), m_addBox );
	gtk_notebook_append_page( GTK_NOTEBOOK( pages ), infoPage, gtk_label_new( "Class Info" ) );
	m_flagsScroll = gtk_scrolled_window_new();
	gtk_notebook_append_page( GTK_NOTEBOOK( pages ), m_flagsScroll, gtk_label_new( "Flags" ) );
	gtk_box_append( GTK_BOX( root ), pages );

	m_status = gtk_label_new( "" );
	gtk_label_set_xalign( GTK_LABEL( m_status ), 0.0f );
	gtk_label_set_wrap( GTK_LABEL( m_status ), TRUE );
	gtk_widget_add_css_class( m_status, "error" );
	SetAccessibleName( m_status, "Properties status" );
	gtk_box_append( GTK_BOX( root ), m_status );

	GtkWidget *buttons = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
	gtk_widget_set_halign( buttons, GTK_ALIGN_END );
	m_cancel = gtk_button_new_with_label( "Cancel" );
	g_signal_connect( m_cancel, "clicked", G_CALLBACK( OnCancel ), this );
	m_apply = gtk_button_new_with_label( "Apply" );
	gtk_widget_add_css_class( m_apply, "suggested-action" );
	g_signal_connect( m_apply, "clicked", G_CALLBACK( OnApply ), this );
	gtk_box_append( GTK_BOX( buttons ), m_cancel );
	gtk_box_append( GTK_BOX( buttons ), m_apply );
	gtk_box_append( GTK_BOX( root ), buttons );

	// Follow the selection and the document: every session event refreshes
	// the window on the main loop, after the inspector has rebuilt.
	m_subscription = m_workspace.Session().Subscribe(
	    [this]( const app::SessionEvent & )
	    {
		    ScheduleRefresh();
	    } );
}

void PropertiesWindow::Present()
{
	gtk_window_present( GTK_WINDOW( m_window ) );
	Refresh();
}

void PropertiesWindow::ScheduleRefresh()
{
	if ( m_idle || m_dead )
	{
		return;
	}
	m_idle = g_idle_add(
	    []( gpointer data ) -> gboolean
	    {
		    auto *self = static_cast<PropertiesWindow *>( data );
		    self->m_idle = 0;
		    self->Refresh();
		    return G_SOURCE_REMOVE;
	    },
	    this );
}

const KeyRow *PropertiesWindow::FindRow( const std::string &key )
{
	for ( const KeyRow &row : Inspector().Rows() )
	{
		if ( row.key == key )
		{
			return &row;
		}
	}
	return nullptr;
}

template <typename Call> void PropertiesWindow::Edit( Call &&call )
{
	if ( m_updating || m_dead )
	{
		return;
	}
	const std::uint64_t before = m_workspace.Session().Revision();
	Inspector().ClearError();
	call( Inspector() );
	Refresh();
	if ( m_changed && m_workspace.Session().Revision() != before )
	{
		m_changed( std::string() );
	}
}

void PropertiesWindow::EditValue( RowView &view, const std::string &value )
{
	const std::string key = view.key;
	const bool original = value == view.baseline;
	Edit(
	    [&]( EntityInspector &inspector )
	    {
		    if ( original )
		    {
			    inspector.ClearDraft( key );
		    }
		    else
		    {
			    (void)inspector.SetDraft( key, value );
		    }
	    } );
}

void PropertiesWindow::Refresh()
{
	if ( m_dead || !gtk_widget_get_visible( m_window ) )
	{
		return;
	}
	EntityInspector &inspector = Inspector();
	m_updating = true;

	const std::size_t count = inspector.Entities().size();
	const std::string summary = inspector.IsWorld() ? std::string( "World properties" )
	                            : count == 1        ? std::string( "1 entity" )
	                                                : std::to_string( count ) + " entities";
	gtk_label_set_text( GTK_LABEL( m_summary ), summary.c_str() );
	gtk_check_button_set_active( GTK_CHECK_BUTTON( m_smartEdit ), inspector.SmartEdit() );
	gtk_widget_set_visible( m_addBox, !inspector.SmartEdit() );
	RefreshClass();
	RebuildRows();
	RebuildFlags();

	std::string status = inspector.LastError();
	if ( status.empty() )
	{
		for ( const std::string &line : inspector.DraftErrors() )
		{
			status += ( status.empty() ? "" : "\n" ) + line;
		}
	}
	gtk_label_set_text( GTK_LABEL( m_status ), status.c_str() );
	gtk_widget_set_visible( m_status, !status.empty() );
	gtk_widget_set_sensitive( m_apply, inspector.HasDraft() );
	gtk_widget_set_sensitive( m_cancel, inspector.HasDraft() );
	m_updating = false;
}

void PropertiesWindow::RefreshClass()
{
	EntityInspector &inspector = Inspector();
	std::vector<std::string> list = inspector.ClassChoices();
	if ( inspector.IsWorld() )
	{
		list = { "worldspawn" };
	}
	if ( list != m_classList )
	{
		m_classList = list;
		GtkStringList *model = gtk_string_list_new( nullptr );
		for ( const std::string &name : m_classList )
		{
			gtk_string_list_append( model, name.c_str() );
		}
		gtk_drop_down_set_model( GTK_DROP_DOWN( m_classes ), G_LIST_MODEL( model ) );
		g_object_unref( model );
		m_classPlaceholder = false;
	}
	std::optional<guint> selected;
	if ( inspector.Class().IsSingle() )
	{
		for ( std::size_t i = 0; i < m_classList.size(); ++i )
		{
			if ( g_ascii_strcasecmp( m_classList[i].c_str(), inspector.Class().Value().c_str() ) ==
			     0 )
			{
				selected = static_cast<guint>( i );
			}
		}
	}
	ShowSelection( m_classes, m_classPlaceholder,
	    inspector.Class().IsMixed() ? "(different classes)" : "(no class)", selected );
	gtk_widget_set_sensitive(
	    m_classes, !inspector.IsWorld() && !inspector.ClassChoices().empty() );
	SetAccessibleDescription( m_classes, inspector.Class().IsMixed() ? kMixedText : "" );
}

void PropertiesWindow::RebuildRows()
{
	EntityInspector &inspector = Inspector();
	const bool smart = inspector.SmartEdit();
	std::string layout = smart ? "S" : "R";
	for ( const KeyRow &row : inspector.Rows() )
	{
		layout += row.key + '\x1f' + std::to_string( int( EditorFor( row, smart ) ) ) +
		          ( row.readOnly ? "r" : "w" ) + std::to_string( row.choices.size() ) + '\x1e';
	}
	if ( layout != m_rowsLayout )
	{
		m_rowsLayout = layout;
		m_rows.clear();
		GtkWidget *grid = gtk_grid_new();
		gtk_grid_set_row_spacing( GTK_GRID( grid ), 4 );
		gtk_grid_set_column_spacing( GTK_GRID( grid ), 8 );
		gtk_widget_set_margin_start( grid, 4 );
		gtk_widget_set_margin_end( grid, 4 );
		gtk_widget_set_margin_top( grid, 4 );
		int line = 0;
		for ( const KeyRow &row : inspector.Rows() )
		{
			auto view = std::make_unique<RowView>();
			view->owner = this;
			view->key = row.key;
			view->kind = EditorFor( row, smart );
			view->label = gtk_label_new( "" );
			gtk_label_set_xalign( GTK_LABEL( view->label ), 0.0f );
			gtk_widget_set_valign( view->label, GTK_ALIGN_START );
			gtk_widget_set_margin_top( view->label, 6 );
			if ( !row.help.empty() )
			{
				gtk_widget_set_tooltip_text( view->label, row.help.c_str() );
			}
			gtk_grid_attach( GTK_GRID( grid ), view->label, 0, line, 1, 1 );

			GtkWidget *cell = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
			gtk_widget_set_hexpand( cell, TRUE );
			GtkWidget *line1 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 4 );
			switch ( view->kind )
			{
			case EditorKind::Choice:
			{
				GtkStringList *labels = gtk_string_list_new( nullptr );
				for ( const ports::KeyChoice &choice : row.choices )
				{
					gtk_string_list_append(
					    labels, ( choice.label.empty() ? choice.value : choice.label ).c_str() );
				}
				view->editor = gtk_drop_down_new( G_LIST_MODEL( labels ), nullptr );
				g_signal_connect(
				    view->editor, "notify::selected", G_CALLBACK( OnChoiceSelected ), view.get() );
				break;
			}
			case EditorKind::Check:
				view->editor = gtk_check_button_new();
				g_signal_connect(
				    view->editor, "toggled", G_CALLBACK( OnCheckToggled ), view.get() );
				break;
			case EditorKind::Text:
			case EditorKind::Color:
				view->editor = gtk_entry_new();
				g_signal_connect(
				    view->editor, "changed", G_CALLBACK( OnTextChanged ), view.get() );
				g_signal_connect(
				    view->editor, "activate", G_CALLBACK( OnTextActivate ), view.get() );
				break;
			}
			gtk_widget_set_hexpand( view->editor, TRUE );
			gtk_widget_set_sensitive( view->editor, !row.readOnly );
			SetAccessibleName( view->editor, row.key );
			if ( !row.help.empty() )
			{
				gtk_widget_set_tooltip_text( view->editor, row.help.c_str() );
			}
			gtk_box_append( GTK_BOX( line1 ), view->editor );
			if ( view->kind == EditorKind::Color )
			{
				GtkColorDialog *dialog = gtk_color_dialog_new();
				gtk_color_dialog_set_with_alpha( dialog, FALSE );
				view->color = gtk_color_dialog_button_new( dialog );
				gtk_widget_set_sensitive( view->color, !row.readOnly );
				SetAccessibleName( view->color, row.key + " colour" );
				g_signal_connect(
				    view->color, "notify::rgba", G_CALLBACK( OnColorPicked ), view.get() );
				gtk_box_append( GTK_BOX( line1 ), view->color );
			}
			gtk_box_append( GTK_BOX( cell ), line1 );
			view->error = gtk_label_new( "" );
			gtk_label_set_xalign( GTK_LABEL( view->error ), 0.0f );
			gtk_widget_add_css_class( view->error, "error" );
			gtk_widget_add_css_class( view->error, "caption" );
			gtk_box_append( GTK_BOX( cell ), view->error );
			gtk_grid_attach( GTK_GRID( grid ), cell, 1, line, 1, 1 );

			if ( !smart )
			{
				GtkWidget *remove = gtk_button_new_from_icon_name( "list-remove-symbolic" );
				SetAccessibleName( remove, "Remove " + row.key );
				gtk_widget_set_tooltip_text( remove, "Remove this key" );
				gtk_widget_set_valign( remove, GTK_ALIGN_START );
				g_signal_connect( remove, "clicked", G_CALLBACK( OnRemoveKey ), view.get() );
				gtk_grid_attach( GTK_GRID( grid ), remove, 2, line, 1, 1 );
			}
			m_rows.push_back( std::move( view ) );
			++line;
		}
		if ( inspector.Rows().empty() )
		{
			GtkWidget *none = gtk_label_new( "No keys" );
			gtk_widget_add_css_class( none, "dim-label" );
			gtk_grid_attach( GTK_GRID( grid ), none, 0, 0, 2, 1 );
		}
		gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( m_rowsScroll ), grid );
	}
	for ( std::size_t i = 0; i < m_rows.size() && i < inspector.Rows().size(); ++i )
	{
		UpdateRow( *m_rows[i], inspector.Rows()[i] );
	}
}

void PropertiesWindow::UpdateRow( RowView &view, const KeyRow &row )
{
	const bool mixed = row.value.IsMixed() && !row.drafted;
	view.baseline = row.value.IsSingle() ? row.value.Value() : std::string();
	const std::string shown = row.drafted ? row.draftValue : view.baseline;
	const std::string title = row.displayName + ( row.drafted ? " *" : "" );
	gtk_label_set_text( GTK_LABEL( view.label ), title.c_str() );
	SetAccessibleDescription( view.editor, mixed ? kMixedText : "" );

	switch ( view.kind )
	{
	case EditorKind::Text:
	case EditorKind::Color:
		if ( shown != gtk_editable_get_text( GTK_EDITABLE( view.editor ) ) )
		{
			gtk_editable_set_text( GTK_EDITABLE( view.editor ), shown.c_str() );
		}
		gtk_entry_set_placeholder_text(
		    GTK_ENTRY( view.editor ), mixed ? kMixedText : row.defaultValue.c_str() );
		if ( view.color )
		{
			if ( std::optional<presenters::KeyColor> color = presenters::ColorOfRow( row ) )
			{
				const GdkRGBA rgba{ float( color->r ), float( color->g ), float( color->b ), 1.0f };
				gtk_color_dialog_button_set_rgba( GTK_COLOR_DIALOG_BUTTON( view.color ), &rgba );
			}
		}
		break;
	case EditorKind::Choice:
	{
		std::optional<guint> selected;
		for ( std::size_t i = 0; i < row.choices.size(); ++i )
		{
			if ( ( row.drafted || row.value.IsSingle() ) && row.choices[i].value == shown )
			{
				selected = static_cast<guint>( i );
			}
		}
		ShowSelection( view.editor, view.placeholder,
		    mixed                                 ? std::string( kMixedText )
		    : row.drafted || row.value.IsSingle() ? shown
		                                          : std::string( "(not set)" ),
		    selected );
		break;
	}
	case EditorKind::Check:
		gtk_check_button_set_inconsistent( GTK_CHECK_BUTTON( view.editor ), mixed );
		gtk_check_button_set_active( GTK_CHECK_BUTTON( view.editor ), !mixed && shown == "1" );
		break;
	}
	gtk_label_set_text( GTK_LABEL( view.error ), row.draftError.c_str() );
	gtk_widget_set_visible( view.error, !row.draftError.empty() );
}

void PropertiesWindow::RebuildFlags()
{
	EntityInspector &inspector = Inspector();
	std::string layout;
	for ( const FlagRow &flag : inspector.Flags() )
	{
		layout += std::to_string( flag.bit ) + '\x1f' + flag.label + '\x1e';
	}
	if ( layout != m_flagsLayout || m_flags.size() != inspector.Flags().size() )
	{
		m_flagsLayout = layout;
		m_flags.clear();
		GtkWidget *box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
		gtk_widget_set_margin_start( box, 8 );
		gtk_widget_set_margin_top( box, 8 );
		for ( const FlagRow &flag : inspector.Flags() )
		{
			auto view = std::make_unique<FlagView>();
			view->owner = this;
			view->bit = flag.bit;
			const std::string label =
			    flag.label.empty() ? "Flag " + std::to_string( flag.bit ) : flag.label;
			view->check = gtk_check_button_new_with_label( label.c_str() );
			g_signal_connect( view->check, "toggled", G_CALLBACK( OnFlagToggled ), view.get() );
			gtk_box_append( GTK_BOX( box ), view->check );
			m_flags.push_back( std::move( view ) );
		}
		if ( inspector.Flags().empty() )
		{
			GtkWidget *none = gtk_label_new( "No flags" );
			gtk_widget_add_css_class( none, "dim-label" );
			gtk_box_append( GTK_BOX( box ), none );
		}
		gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( m_flagsScroll ), box );
	}
	for ( std::size_t i = 0; i < m_flags.size(); ++i )
	{
		const FlagRow &flag = inspector.Flags()[i];
		GtkCheckButton *check = GTK_CHECK_BUTTON( m_flags[i]->check );
		gtk_check_button_set_inconsistent( check, flag.state == FlagState::Mixed );
		gtk_check_button_set_active( check, flag.state == FlagState::On );
		SetAccessibleDescription(
		    m_flags[i]->check, flag.state == FlagState::Mixed ? kMixedText : "" );
	}
}

// ---- Signal handlers -----------------------------------------------------------

void PropertiesWindow::OnDestroy( GtkWidget *, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	self->m_dead = true;
	self->m_subscription = app::SessionSubscription();
	if ( self->m_idle )
	{
		g_source_remove( self->m_idle );
		self->m_idle = 0;
	}
	if ( GtkWindow *parent = gtk_window_get_transient_for( GTK_WINDOW( self->m_window ) ) )
	{
		g_object_set_data( G_OBJECT( parent ), kWindowKey, nullptr );
	}
	// Children are disposed after this handler and may still signal; the
	// handlers see m_dead. The object goes once the main loop is idle.
	g_idle_add_once(
	    []( gpointer state )
	    {
		    delete static_cast<PropertiesWindow *>( state );
	    },
	    self );
}

gboolean PropertiesWindow::OnCloseRequest( GtkWindow *, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	const std::uint64_t before = self->m_workspace.Session().Revision();
	EntityInspector::Result settled = self->Inspector().Settle();
	if ( self->m_changed && ( !settled || self->m_workspace.Session().Revision() != before ) )
	{
		self->m_changed( settled ? std::string() : self->Inspector().LastError() );
	}
	return FALSE; // hide (hide-on-close)
}

gboolean PropertiesWindow::OnEscape( GtkWidget *, GVariant *, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	// In a drafted field Escape reverts that field; elsewhere it closes the
	// window, which settles the draft (RFC 0018 F4).
	GtkWidget *focus = gtk_root_get_focus( GTK_ROOT( self->m_window ) );
	for ( const std::unique_ptr<RowView> &view : self->m_rows )
	{
		if ( focus && ( focus == view->editor || gtk_widget_is_ancestor( focus, view->editor ) ) )
		{
			const KeyRow *row = self->FindRow( view->key );
			if ( row && row->drafted )
			{
				const std::string key = view->key;
				self->Edit(
				    [&]( EntityInspector &inspector )
				    {
					    inspector.ClearDraft( key );
				    } );
				return TRUE;
			}
		}
	}
	gtk_window_close( GTK_WINDOW( self->m_window ) );
	return TRUE;
}

void PropertiesWindow::OnClassSelected( GObject *, GParamSpec *, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	const std::optional<std::size_t> index = ItemOf( self->m_classes, self->m_classPlaceholder );
	if ( self->m_updating || !index || *index >= self->m_classList.size() ||
	     self->Inspector().IsWorld() )
	{
		return;
	}
	const std::string name = self->m_classList[*index];
	const app::PropertyValue &current = self->Inspector().Class();
	if ( current.IsSingle() && g_ascii_strcasecmp( current.Value().c_str(), name.c_str() ) == 0 )
	{
		return;
	}
	self->Edit(
	    [&]( EntityInspector &inspector )
	    {
		    (void)inspector.SetClass( name );
	    } );
}

void PropertiesWindow::OnSmartEdit( GtkCheckButton *button, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	const bool on = gtk_check_button_get_active( button );
	self->Edit(
	    [&]( EntityInspector &inspector )
	    {
		    inspector.SetSmartEdit( on );
	    } );
}

void PropertiesWindow::OnApply( GtkButton *, gpointer data )
{
	static_cast<PropertiesWindow *>( data )->Edit(
	    []( EntityInspector &inspector )
	    {
		    (void)inspector.Commit();
	    } );
}

void PropertiesWindow::OnCancel( GtkButton *, gpointer data )
{
	static_cast<PropertiesWindow *>( data )->Edit(
	    []( EntityInspector &inspector )
	    {
		    inspector.Cancel();
	    } );
}

void PropertiesWindow::OnAddKey( GtkButton *, gpointer data )
{
	auto *self = static_cast<PropertiesWindow *>( data );
	const std::string key = gtk_editable_get_text( GTK_EDITABLE( self->m_addKey ) );
	const std::string value = gtk_editable_get_text( GTK_EDITABLE( self->m_addValue ) );
	bool added = false;
	self->Edit(
	    [&]( EntityInspector &inspector )
	    {
		    added = inspector.SetDraft( key, value ).HasValue();
	    } );
	if ( added )
	{
		gtk_editable_set_text( GTK_EDITABLE( self->m_addKey ), "" );
		gtk_editable_set_text( GTK_EDITABLE( self->m_addValue ), "" );
	}
}

void PropertiesWindow::OnTextChanged( GtkEditable *editable, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	if ( view->owner->m_updating || view->owner->m_dead )
	{
		return;
	}
	view->owner->EditValue( *view, gtk_editable_get_text( editable ) );
}

void PropertiesWindow::OnTextActivate( GtkEntry *, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	if ( view->owner->Inspector().HasDraft() )
	{
		OnApply( nullptr, view->owner );
	}
}

void PropertiesWindow::OnChoiceSelected( GObject *, GParamSpec *, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	PropertiesWindow *self = view->owner;
	if ( self->m_updating || self->m_dead )
	{
		return;
	}
	const KeyRow *row = self->FindRow( view->key );
	const std::optional<std::size_t> index = ItemOf( view->editor, view->placeholder );
	if ( row && index && *index < row->choices.size() )
	{
		self->EditValue( *view, row->choices[*index].value );
	}
}

void PropertiesWindow::OnCheckToggled( GtkCheckButton *button, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	if ( view->owner->m_updating || view->owner->m_dead )
	{
		return;
	}
	view->owner->EditValue( *view, gtk_check_button_get_active( button ) ? "1" : "0" );
}

void PropertiesWindow::OnColorPicked( GObject *, GParamSpec *, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	PropertiesWindow *self = view->owner;
	if ( self->m_updating || self->m_dead )
	{
		return;
	}
	const KeyRow *row = self->FindRow( view->key );
	const GdkRGBA *rgba =
	    gtk_color_dialog_button_get_rgba( GTK_COLOR_DIALOG_BUTTON( view->color ) );
	if ( row && rgba )
	{
		self->EditValue( *view, presenters::ValueWithColor( *row,
		                            presenters::KeyColor{ rgba->red, rgba->green, rgba->blue } ) );
	}
}

void PropertiesWindow::OnRemoveKey( GtkButton *, gpointer data )
{
	auto *view = static_cast<RowView *>( data );
	const std::string key = view->key;
	view->owner->Edit(
	    [&]( EntityInspector &inspector )
	    {
		    (void)inspector.RemoveKey( key );
	    } );
}

void PropertiesWindow::OnFlagToggled( GtkCheckButton *button, gpointer data )
{
	auto *view = static_cast<FlagView *>( data );
	if ( view->owner->m_updating || view->owner->m_dead )
	{
		return;
	}
	const long long bit = view->bit;
	const bool on = gtk_check_button_get_active( button );
	view->owner->Edit(
	    [&]( EntityInspector &inspector )
	    {
		    (void)inspector.SetFlag( bit, on );
	    } );
}

} // namespace

void ShowPropertiesDialog(
    GtkWindow *parent, presenters::EditorWorkspace &workspace, PropertiesChanged changed )
{
	auto *window =
	    static_cast<PropertiesWindow *>( g_object_get_data( G_OBJECT( parent ), kWindowKey ) );
	if ( !window )
	{
		window = new PropertiesWindow( parent, workspace, std::move( changed ) );
		g_object_set_data( G_OBJECT( parent ), kWindowKey, window );
	}
	else
	{
		window->SetChanged( std::move( changed ) );
	}
	window->Present();
}

} // namespace hammer::gtk
