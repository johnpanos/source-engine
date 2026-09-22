//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GTK4 + libadwaita desktop shell for Hammer (RFC 0002,
//			linux-gtk-desktop). This is the delivery-target host: it composes the
//			strict headless editor core (hammer::app::EditorDocument, the VMF codec,
//			and the VMF -> convex brush geometry bridge) with a native Adwaita UI and
//			an OpenGL 3D viewport. All GTK/GDK/GL native detail is confined to this
//			product; the editor core it drives has no display, GPU, or MFC
//			dependency.
//
//			Two entry paths share one renderer (hammergtk::Renderer):
//			  * the interactive window (GtkGLArea viewport + outliner + Open dialog);
//			  * a headless "--screenshot OUT.ppm IN.vmf" path that renders the loaded
//			    map to an offscreen EGL framebuffer and writes a PPM, so the 3D
//			    preview can be verified without a window server.
//
//=============================================================================//

#include "hammer/app/editor_document.h"
#include "hammer/geometry/brush.h"

#include "posix_file_store.h"
#include "renderer.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include <adwaita.h>
#include <epoxy/gl.h>
#include <gtk/gtk.h>

namespace
{

constexpr const char *kAppId = "com.valvesoftware.HammerGtk";

// ---------------------------------------------------------------------------
// Shared load path: VMF text -> document -> renderable scene.
// ---------------------------------------------------------------------------

// Loads a VMF from disk through the editor core and builds a renderable scene.
// Returns false and fills 'error' on failure; on success 'scene' is populated.
bool LoadSceneFromFile( const std::string &path, hammer::geometry::WorldScene &scene,
                        std::string &error )
{
	hammergtk::PosixFileStore store;
	hammer::app::EditorDocument doc;
	if ( !doc.Load( store, path, error ) )
	{
		return false;
	}
	scene = hammer::geometry::BuildSceneFromDocument( doc.Content() );
	return true;
}

// ---------------------------------------------------------------------------
// Interactive application state.
// ---------------------------------------------------------------------------

struct AppState
{
	hammergtk::Renderer renderer;
	hammer::geometry::WorldScene scene;
	bool scenePending = false; // scene loaded before the GL area was realized
	bool glReady = false;

	std::string openOnStart; // optional path from --open

	GtkWidget *window = nullptr;
	AdwWindowTitle *title = nullptr;
	GtkGLArea *glarea = nullptr;
	GtkListBox *outliner = nullptr;
	GtkLabel *status = nullptr;

	// Orbit gesture bookkeeping.
	double dragPrevX = 0.0;
	double dragPrevY = 0.0;
};

void SetStatus( AppState *st, const std::string &text )
{
	if ( st->status )
	{
		gtk_label_set_text( st->status, text.c_str() );
	}
}

void PopulateOutliner( AppState *st )
{
	if ( !st->outliner )
	{
		return;
	}
	gtk_list_box_remove_all( st->outliner );

	char buf[256];

	std::snprintf( buf, sizeof( buf ), "World / brush solids (%zu)", st->scene.solids.size() );
	GtkWidget *solidsHeader = adw_action_row_new();
	adw_preferences_row_set_use_markup( ADW_PREFERENCES_ROW( solidsHeader ), FALSE );
	adw_preferences_row_set_title( ADW_PREFERENCES_ROW( solidsHeader ), buf );
	gtk_widget_add_css_class( solidsHeader, "heading" );
	gtk_list_box_append( st->outliner, solidsHeader );

	for ( const hammer::geometry::BrushSolid &solid : st->scene.solids )
	{
		GtkWidget *row = adw_action_row_new();
		adw_preferences_row_set_use_markup( ADW_PREFERENCES_ROW( row ), FALSE );
		std::snprintf( buf, sizeof( buf ), "Solid #%d", solid.id );
		adw_preferences_row_set_title( ADW_PREFERENCES_ROW( row ), buf );
		std::snprintf( buf, sizeof( buf ), "%zu faces", solid.faces.size() );
		adw_action_row_set_subtitle( ADW_ACTION_ROW( row ), buf );
		gtk_list_box_append( st->outliner, row );
	}

	std::snprintf( buf, sizeof( buf ), "Entities (%zu)", st->scene.entities.size() );
	GtkWidget *entHeader = adw_action_row_new();
	adw_preferences_row_set_use_markup( ADW_PREFERENCES_ROW( entHeader ), FALSE );
	adw_preferences_row_set_title( ADW_PREFERENCES_ROW( entHeader ), buf );
	gtk_widget_add_css_class( entHeader, "heading" );
	gtk_list_box_append( st->outliner, entHeader );

	for ( const hammer::geometry::SceneEntity &ent : st->scene.entities )
	{
		GtkWidget *row = adw_action_row_new();
		adw_preferences_row_set_use_markup( ADW_PREFERENCES_ROW( row ), FALSE );
		adw_preferences_row_set_title( ADW_PREFERENCES_ROW( row ),
		                               ent.classname.empty() ? "(entity)" : ent.classname.c_str() );
		if ( ent.origin )
		{
			std::snprintf( buf, sizeof( buf ), "origin %.0f %.0f %.0f", ent.origin->x, ent.origin->y,
			               ent.origin->z );
		}
		else
		{
			std::snprintf( buf, sizeof( buf ), "%zu solid(s)", ent.solidCount );
		}
		adw_action_row_set_subtitle( ADW_ACTION_ROW( row ), buf );
		gtk_list_box_append( st->outliner, row );
	}
}

void ApplyScene( AppState *st, const std::string &path )
{
	if ( st->glReady )
	{
		gtk_gl_area_make_current( st->glarea );
		if ( gtk_gl_area_get_error( st->glarea ) == nullptr )
		{
			st->renderer.SetScene( st->scene );
		}
		st->scenePending = false;
		gtk_gl_area_queue_render( st->glarea );
	}
	else
	{
		st->scenePending = true; // apply on realize
	}

	PopulateOutliner( st );

	const std::string base = path.substr( path.find_last_of( "/" ) + 1 );
	if ( st->title )
	{
		adw_window_title_set_subtitle( st->title, base.c_str() );
	}
	char buf[256];
	std::snprintf( buf, sizeof( buf ), "%zu solids · %zu faces · %zu entities", st->scene.solids.size(),
	               st->scene.TotalFaces(), st->scene.entities.size() );
	SetStatus( st, buf );
}

void LoadPath( AppState *st, const std::string &path )
{
	hammer::geometry::WorldScene scene;
	std::string error;
	if ( !LoadSceneFromFile( path, scene, error ) )
	{
		SetStatus( st, std::string( "Failed to open: " ) + error );
		return;
	}
	st->scene = std::move( scene );
	ApplyScene( st, path );
}

// ---- GtkFileDialog open flow ----------------------------------------------

void OnOpenFinished( GObject *source, GAsyncResult *res, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GError *err = nullptr;
	GFile *file = gtk_file_dialog_open_finish( GTK_FILE_DIALOG( source ), res, &err );
	if ( !file )
	{
		if ( err )
		{
			g_error_free( err );
		}
		return; // user cancelled
	}
	char *path = g_file_get_path( file );
	if ( path )
	{
		LoadPath( st, path );
		g_free( path );
	}
	g_object_unref( file );
}

void OnOpenClicked( GtkButton *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GtkFileDialog *dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_title( dialog, "Open VMF map" );

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name( filter, "Valve Map Format (*.vmf)" );
	gtk_file_filter_add_pattern( filter, "*.vmf" );
	GListStore *filters = g_list_store_new( GTK_TYPE_FILE_FILTER );
	g_list_store_append( filters, filter );
	gtk_file_dialog_set_filters( dialog, G_LIST_MODEL( filters ) );
	g_object_unref( filter );
	g_object_unref( filters );

	gtk_file_dialog_open( dialog, GTK_WINDOW( st->window ), nullptr, OnOpenFinished, st );
	g_object_unref( dialog );
}

// ---- GtkGLArea callbacks ---------------------------------------------------

void OnGlRealize( GtkGLArea *area, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	gtk_gl_area_make_current( area );
	if ( gtk_gl_area_get_error( area ) != nullptr )
	{
		return;
	}
	if ( std::getenv( "HAMMER_GTK_DEBUG" ) != nullptr )
	{
		if ( const char *ver = reinterpret_cast<const char *>( glGetString( GL_VERSION ) ) )
		{
			GdkGLContext *ctx = gtk_gl_area_get_context( area );
			std::fprintf( stderr, "[hammer_gtk] GL_VERSION=%s  GLES=%d\n", ver,
			              ctx ? gdk_gl_context_get_use_es( ctx ) : -1 );
		}
	}
	std::string error;
	if ( !st->renderer.Init( error ) )
	{
		GError *gerr = g_error_new_literal( g_quark_from_static_string( "hammergtk" ), 1,
		                                    error.c_str() );
		gtk_gl_area_set_error( area, gerr );
		g_error_free( gerr );
		return;
	}
	st->glReady = true;
	if ( st->scenePending )
	{
		st->renderer.SetScene( st->scene );
		st->scenePending = false;
	}
}

void OnGlUnrealize( GtkGLArea *area, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	gtk_gl_area_make_current( area );
	st->glReady = false;
}

gboolean OnGlRender( GtkGLArea *area, GdkGLContext *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( area ) ) * scale;
	st->renderer.Render( w, h );
	return TRUE;
}

// ---- Camera input ----------------------------------------------------------

void OnDragBegin( GtkGestureDrag *, double, double, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->dragPrevX = 0.0;
	st->dragPrevY = 0.0;
}

void OnDragUpdate( GtkGestureDrag *, double offsetX, double offsetY, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	const double dx = offsetX - st->dragPrevX;
	const double dy = offsetY - st->dragPrevY;
	st->dragPrevX = offsetX;
	st->dragPrevY = offsetY;
	st->renderer.OrbitBy( static_cast<float>( -dx ) * 0.4f, static_cast<float>( dy ) * 0.4f );
	if ( st->glarea )
	{
		gtk_gl_area_queue_render( st->glarea );
	}
}

gboolean OnScroll( GtkEventControllerScroll *, double, double dy, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->renderer.ZoomBy( std::pow( 1.12f, static_cast<float>( dy ) ) );
	if ( st->glarea )
	{
		gtk_gl_area_queue_render( st->glarea );
	}
	return TRUE;
}

// ---- Window construction ---------------------------------------------------

void OnActivate( GtkApplication *app, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );

	GtkWidget *window = adw_application_window_new( app );
	st->window = window;
	gtk_window_set_default_size( GTK_WINDOW( window ), 1100, 720 );
	gtk_window_set_title( GTK_WINDOW( window ), "Hammer" );

	GtkWidget *toolbar = adw_toolbar_view_new();

	// Header bar with a window title and an Open button.
	GtkWidget *header = adw_header_bar_new();
	GtkWidget *title = adw_window_title_new( "Hammer", "No map loaded" );
	st->title = ADW_WINDOW_TITLE( title );
	adw_header_bar_set_title_widget( ADW_HEADER_BAR( header ), title );

	GtkWidget *openBtn = gtk_button_new();
	GtkWidget *openContent = adw_button_content_new();
	adw_button_content_set_icon_name( ADW_BUTTON_CONTENT( openContent ), "document-open-symbolic" );
	adw_button_content_set_label( ADW_BUTTON_CONTENT( openContent ), "Open" );
	gtk_button_set_child( GTK_BUTTON( openBtn ), openContent );
	gtk_widget_set_tooltip_text( openBtn, "Open a VMF map" );
	g_signal_connect( openBtn, "clicked", G_CALLBACK( OnOpenClicked ), st );
	adw_header_bar_pack_start( ADW_HEADER_BAR( header ), openBtn );

	adw_toolbar_view_add_top_bar( ADW_TOOLBAR_VIEW( toolbar ), header );

	// Main content: outliner on the left, GL viewport on the right.
	GtkWidget *paned = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_position( GTK_PANED( paned ), 280 );
	gtk_paned_set_shrink_start_child( GTK_PANED( paned ), FALSE );

	GtkWidget *scroller = gtk_scrolled_window_new();
	gtk_widget_set_size_request( scroller, 240, -1 );
	GtkWidget *outliner = gtk_list_box_new();
	gtk_list_box_set_selection_mode( GTK_LIST_BOX( outliner ), GTK_SELECTION_NONE );
	gtk_widget_add_css_class( outliner, "navigation-sidebar" );
	st->outliner = GTK_LIST_BOX( outliner );
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( scroller ), outliner );
	gtk_paned_set_start_child( GTK_PANED( paned ), scroller );

	GtkWidget *glarea = gtk_gl_area_new();
	st->glarea = GTK_GL_AREA( glarea );
	// Match the offscreen EGL path: desktop GL (not GLES) so the #version 330 core
	// shaders compile in the same profile they are validated against.
	gtk_gl_area_set_allowed_apis( GTK_GL_AREA( glarea ), GDK_GL_API_GL );
	gtk_gl_area_set_required_version( GTK_GL_AREA( glarea ), 3, 3 );
	gtk_gl_area_set_has_depth_buffer( GTK_GL_AREA( glarea ), TRUE );
	gtk_gl_area_set_auto_render( GTK_GL_AREA( glarea ), TRUE );
	gtk_widget_set_hexpand( glarea, TRUE );
	gtk_widget_set_vexpand( glarea, TRUE );
	g_signal_connect( glarea, "realize", G_CALLBACK( OnGlRealize ), st );
	g_signal_connect( glarea, "unrealize", G_CALLBACK( OnGlUnrealize ), st );
	g_signal_connect( glarea, "render", G_CALLBACK( OnGlRender ), st );

	// Orbit + zoom controllers.
	GtkGesture *drag = gtk_gesture_drag_new();
	g_signal_connect( drag, "drag-begin", G_CALLBACK( OnDragBegin ), st );
	g_signal_connect( drag, "drag-update", G_CALLBACK( OnDragUpdate ), st );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( drag ) );

	GtkEventController *scroll =
	    gtk_event_controller_scroll_new( GTK_EVENT_CONTROLLER_SCROLL_VERTICAL );
	g_signal_connect( scroll, "scroll", G_CALLBACK( OnScroll ), st );
	gtk_widget_add_controller( glarea, scroll );

	gtk_paned_set_end_child( GTK_PANED( paned ), glarea );

	adw_toolbar_view_set_content( ADW_TOOLBAR_VIEW( toolbar ), paned );

	// Bottom status bar.
	GtkWidget *statusBar = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 8 );
	gtk_widget_set_margin_start( statusBar, 12 );
	gtk_widget_set_margin_end( statusBar, 12 );
	gtk_widget_set_margin_top( statusBar, 4 );
	gtk_widget_set_margin_bottom( statusBar, 4 );
	GtkWidget *status = gtk_label_new( "Open a VMF map to begin  ·  drag to orbit, scroll to zoom" );
	gtk_label_set_xalign( GTK_LABEL( status ), 0.0f );
	st->status = GTK_LABEL( status );
	gtk_box_append( GTK_BOX( statusBar ), status );
	adw_toolbar_view_add_bottom_bar( ADW_TOOLBAR_VIEW( toolbar ), statusBar );

	adw_application_window_set_content( ADW_APPLICATION_WINDOW( window ), toolbar );

	if ( !st->openOnStart.empty() )
	{
		LoadPath( st, st->openOnStart );
	}

	gtk_window_present( GTK_WINDOW( window ) );
}

int RunApp( AppState *st, int argc, char **argv )
{
	AdwApplication *app = adw_application_new( kAppId, G_APPLICATION_NON_UNIQUE );
	g_signal_connect( app, "activate", G_CALLBACK( OnActivate ), st );
	// We parse our own flags; hand GApplication only the program name so it does
	// not try to interpret --open/paths as files-to-open.
	char *only[] = { argv[0], nullptr };
	const int status = g_application_run( G_APPLICATION( app ), 1, only );
	(void)argc;
	g_object_unref( app );
	return status;
}

} // namespace

// Headless offscreen rendering is defined in offscreen.cpp.
int RenderScreenshot( const std::string &vmfPath, const std::string &outPpm, int width, int height );

int main( int argc, char **argv )
{
	std::string screenshotOut;
	std::string screenshotIn;
	std::string openPath;
	int width = 1024;
	int height = 768;

	for ( int i = 1; i < argc; ++i )
	{
		const std::string a = argv[i];
		if ( a == "--screenshot" && i + 2 < argc )
		{
			screenshotOut = argv[++i];
			screenshotIn = argv[++i];
		}
		else if ( a == "--open" && i + 1 < argc )
		{
			openPath = argv[++i];
		}
		else if ( a == "--width" && i + 1 < argc )
		{
			width = std::atoi( argv[++i] );
		}
		else if ( a == "--height" && i + 1 < argc )
		{
			height = std::atoi( argv[++i] );
		}
		else if ( a == "--help" || a == "-h" )
		{
			std::printf( "Usage: hammer_gtk [--open MAP.vmf]\n"
			             "       hammer_gtk --screenshot OUT.ppm MAP.vmf [--width W --height H]\n" );
			return 0;
		}
	}

	if ( !screenshotOut.empty() )
	{
		return RenderScreenshot( screenshotIn, screenshotOut, width, height );
	}

	AppState st;
	st.openOnStart = openPath;
	return RunApp( &st, argc, argv );
}
