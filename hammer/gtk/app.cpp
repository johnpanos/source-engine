//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GTK4 + libadwaita desktop shell for Hammer (RFC 0002,
//			linux-gtk-desktop). This is the delivery-target host: it presents the
//			classic Hammer layout (menu, toolbar, tool palette, the four viewports,
//			object bar, status bar) over the strict headless editor core.
//
//			It is a THIN PRESENTER: the single live editing authority is
//			hammer::app::EditorController (spatial tools, selection, one undo
//			history). This host only translates GTK gestures/keys into normalized
//			controller calls and renders controller.BuildScene(); it holds no second
//			document or undo stack. Save routes through the shared SaveDocument
//			orchestrator over hammer::adapters::platform::DiskFileStore; load reads
//			the file and feeds the text to EditorController::LoadVmf. All GTK/GDK/GL
//			native detail is confined here. A headless "--screenshot/--quad" path
//			(offscreen.cpp) shares the renderer for windowless verification.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/app/editor_controller.h"
#include "hammer/app/save_orchestrator.h"

#include "renderer.h"

#include <array>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <adwaita.h>
#include <epoxy/gl.h>
#include <gtk/gtk.h>

namespace
{

constexpr const char *kAppId = "com.valvesoftware.HammerGtk";
constexpr int kNoHighlight = INT_MIN;

struct AppState;

struct Viewport
{
	AppState *app = nullptr;
	hammergtk::Renderer renderer;
	hammergtk::ViewMode mode = hammergtk::ViewMode::Perspective;
	hammer::app::ViewId vid = hammer::app::ViewId::Camera;
	const char *label = "";
	GtkGLArea *area = nullptr;
	bool glReady = false;
	bool pending = false;

	double startX = 0.0; // tool-drag start (widget coords)
	double startY = 0.0;
	double orbitPrevX = 0.0; // incremental orbit/pan bookkeeping
	double orbitPrevY = 0.0;
	double panPrevX = 0.0;
	double panPrevY = 0.0;
	double cursorX = 0.0;
	double cursorY = 0.0;
	double pinchPrev = 1.0;
};

struct AppState
{
	hammer::app::EditorController controller;
	std::array<Viewport, 4> viewports;
	std::string currentPath;
	std::string openOnStart;

	GtkApplication *application = nullptr;
	GtkWidget *window = nullptr;
	GtkLabel *helpLabel = nullptr;
	GtkLabel *coordLabel = nullptr;
	GtkLabel *snapLabel = nullptr;
	GtkLabel *objectsCount = nullptr;
	GtkToggleButton *selectBtn = nullptr;
	GtkToggleButton *blockBtn = nullptr;
	bool suppressToolSignal = false;
};

// ---------------------------------------------------------------------------
// Presentation refresh: push the controller's scene to every viewport.
// ---------------------------------------------------------------------------

void SetHelp( AppState *st, const std::string &text )
{
	if ( st->helpLabel )
	{
		gtk_label_set_text( st->helpLabel, text.c_str() );
	}
}

void UpdateChrome( AppState *st )
{
	if ( st->objectsCount )
	{
		char buf[128];
		std::snprintf( buf, sizeof( buf ), "%zu brush(es)%s", st->controller.Brushes().size(),
		    st->controller.Selection() ? "  ·  1 selected" : "" );
		gtk_label_set_text( st->objectsCount, buf );
	}
	if ( st->snapLabel )
	{
		char buf[64];
		std::snprintf( buf, sizeof( buf ), "Grid: %d  ·  %s", st->controller.GridSize(),
		    st->controller.CurrentTool() == hammer::app::Tool::Block ? "Block" : "Select" );
		gtk_label_set_text( st->snapLabel, buf );
	}
	if ( st->window )
	{
		const std::string base =
		    st->currentPath.empty()
		        ? std::string( "untitled" )
		        : st->currentPath.substr( st->currentPath.find_last_of( "/" ) + 1 );
		const std::string title =
		    "Hammer - [" + base + ( st->controller.IsModified() ? " *]" : "]" );
		gtk_window_set_title( GTK_WINDOW( st->window ), title.c_str() );
	}
}

// Uploads the current scene to every ready viewport. 'frame' re-centres each view
// (used on load/new/reset); live edits pass false so the view stays put.
void RefreshScene( AppState *st, bool frame )
{
	const hammer::geometry::WorldScene scene = st->controller.BuildScene();
	const int selId = st->controller.Selection() ? *st->controller.Selection() : kNoHighlight;
	for ( Viewport &vp : st->viewports )
	{
		if ( vp.glReady && vp.area )
		{
			gtk_gl_area_make_current( vp.area );
			if ( gtk_gl_area_get_error( vp.area ) == nullptr )
			{
				vp.renderer.SetHighlight( selId );
				vp.renderer.SetScene( scene );
				if ( frame )
				{
					vp.renderer.FrameScene();
				}
			}
			gtk_gl_area_queue_render( vp.area );
		}
		else
		{
			vp.pending = true;
		}
	}
	UpdateChrome( st );
}

// ---------------------------------------------------------------------------
// File operations (single save path via SaveDocument + DiskFileStore).
// ---------------------------------------------------------------------------

void DoOpen( AppState *st, const std::string &path )
{
	hammer::adapters::platform::DiskFileStore store;
	std::string text;
	if ( !store.Read( path, text ) )
	{
		SetHelp( st, "Failed to read: " + path );
		return;
	}
	std::string error;
	if ( !st->controller.LoadVmf( text, error ) )
	{
		SetHelp( st, "Failed to parse VMF: " + error );
		return;
	}
	st->currentPath = path;
	RefreshScene( st, true );
	SetHelp( st, "Opened " + path );
}

void DoSave( AppState *st, const std::string &path )
{
	hammer::adapters::platform::DiskFileStore store;
	const std::string text = st->controller.ToVmf();
	const hammer::app::SaveStatus status = hammer::app::SaveDocument( store, path, text );
	if ( status == hammer::app::SaveStatus::kOk )
	{
		st->controller.MarkSaved();
		st->currentPath = path;
		UpdateChrome( st );
		SetHelp( st, "Saved " + path );
	}
	else
	{
		SetHelp( st, "Save failed: " + path );
	}
}

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
		return;
	}
	char *path = g_file_get_path( file );
	if ( path )
	{
		DoOpen( st, path );
		g_free( path );
	}
	g_object_unref( file );
}

GtkFileDialog *MakeVmfDialog()
{
	GtkFileDialog *dialog = gtk_file_dialog_new();
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name( filter, "Valve Map Format (*.vmf)" );
	gtk_file_filter_add_pattern( filter, "*.vmf" );
	GListStore *filters = g_list_store_new( GTK_TYPE_FILE_FILTER );
	g_list_store_append( filters, filter );
	gtk_file_dialog_set_filters( dialog, G_LIST_MODEL( filters ) );
	g_object_unref( filter );
	g_object_unref( filters );
	return dialog;
}

void OnSaveFinished( GObject *source, GAsyncResult *res, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GError *err = nullptr;
	GFile *file = gtk_file_dialog_save_finish( GTK_FILE_DIALOG( source ), res, &err );
	if ( !file )
	{
		if ( err )
		{
			g_error_free( err );
		}
		return;
	}
	char *path = g_file_get_path( file );
	if ( path )
	{
		DoSave( st, path );
		g_free( path );
	}
	g_object_unref( file );
}

// ---- GAction handlers ------------------------------------------------------

void ActionOpen( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GtkFileDialog *dialog = MakeVmfDialog();
	gtk_file_dialog_set_title( dialog, "Open VMF map" );
	gtk_file_dialog_open( dialog, GTK_WINDOW( st->window ), nullptr, OnOpenFinished, st );
	g_object_unref( dialog );
}

void ActionSaveAs( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GtkFileDialog *dialog = MakeVmfDialog();
	gtk_file_dialog_set_title( dialog, "Save VMF map" );
	gtk_file_dialog_set_initial_name( dialog, "untitled.vmf" );
	gtk_file_dialog_save( dialog, GTK_WINDOW( st->window ), nullptr, OnSaveFinished, st );
	g_object_unref( dialog );
}

void ActionSave( GSimpleAction *action, GVariant *param, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->currentPath.empty() )
	{
		ActionSaveAs( action, param, user_data );
	}
	else
	{
		DoSave( st, st->currentPath );
	}
}

void ActionNew( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->controller.NewMap();
	st->currentPath.clear();
	RefreshScene( st, true );
	SetHelp( st, "New map" );
}

void ActionQuit( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->window )
	{
		gtk_window_close( GTK_WINDOW( st->window ) );
	}
}

void ActionUndo( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->controller.Undo() )
	{
		RefreshScene( st, false );
		SetHelp( st, "Undo" );
	}
}

void ActionRedo( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->controller.Redo() )
	{
		RefreshScene( st, false );
		SetHelp( st, "Redo" );
	}
}

void ActionDelete( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->controller.DeleteSelection() )
	{
		RefreshScene( st, false );
	}
}

void ActionResetViews( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	for ( Viewport &vp : st->viewports )
	{
		if ( vp.area )
		{
			gtk_gl_area_make_current( vp.area );
			vp.renderer.FrameScene();
			gtk_gl_area_queue_render( vp.area );
		}
	}
}

void SetToolUi( AppState *st, hammer::app::Tool tool )
{
	st->controller.SetTool( tool );
	st->suppressToolSignal = true;
	if ( st->selectBtn )
	{
		gtk_toggle_button_set_active( st->selectBtn, tool == hammer::app::Tool::Select );
	}
	if ( st->blockBtn )
	{
		gtk_toggle_button_set_active( st->blockBtn, tool == hammer::app::Tool::Block );
	}
	st->suppressToolSignal = false;
	RefreshScene( st, false );
	SetHelp( st, tool == hammer::app::Tool::Block ? "Block tool: drag in a 2D view, Enter to create"
	                                              : "Selection tool: click a brush, drag to move" );
}

void ActionToolSelect( GSimpleAction *, GVariant *, gpointer user_data )
{
	SetToolUi( static_cast<AppState *>( user_data ), hammer::app::Tool::Select );
}

void ActionToolBlock( GSimpleAction *, GVariant *, gpointer user_data )
{
	SetToolUi( static_cast<AppState *>( user_data ), hammer::app::Tool::Block );
}

// ---- GtkGLArea callbacks ---------------------------------------------------

void OnGlRealize( GtkGLArea *area, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	gtk_gl_area_make_current( area );
	if ( gtk_gl_area_get_error( area ) != nullptr )
	{
		return;
	}
	if ( std::getenv( "HAMMER_GTK_DEBUG" ) != nullptr )
	{
		const char *ver = reinterpret_cast<const char *>( glGetString( GL_VERSION ) );
		std::fprintf( stderr, "[hammer_gtk] %s GL_VERSION=%s\n", vp->label, ver ? ver : "?" );
	}
	std::string error;
	if ( !vp->renderer.Init( error ) )
	{
		GError *gerr =
		    g_error_new_literal( g_quark_from_static_string( "hammergtk" ), 1, error.c_str() );
		gtk_gl_area_set_error( area, gerr );
		g_error_free( gerr );
		return;
	}
	vp->renderer.SetViewMode( vp->mode );
	vp->glReady = true;

	const int selId =
	    vp->app->controller.Selection() ? *vp->app->controller.Selection() : kNoHighlight;
	vp->renderer.SetHighlight( selId );
	vp->renderer.SetScene( vp->app->controller.BuildScene() );
	vp->renderer.FrameScene();
	vp->pending = false;
}

void OnGlUnrealize( GtkGLArea *area, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	gtk_gl_area_make_current( area );
	vp->glReady = false;
}

gboolean OnGlRender( GtkGLArea *area, GdkGLContext *, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( area ) ) * scale;
	vp->renderer.Render( w, h );
	return TRUE;
}

// ---- Input helpers ---------------------------------------------------------

void WidgetToWorld( Viewport *vp, double wx, double wy, double &u, double &v )
{
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
	float fu = 0.0f;
	float fv = 0.0f;
	vp->renderer.PixelToWorld(
	    static_cast<float>( wx * scale ), static_cast<float>( wy * scale ), w, h, fu, fv );
	u = fu;
	v = fv;
}

void UpdateCoords( Viewport *vp )
{
	AppState *st = vp->app;
	if ( !st->coordLabel || vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}
	double u = 0.0;
	double v = 0.0;
	WidgetToWorld( vp, vp->cursorX, vp->cursorY, u, v );
	int ua = 0;
	int va = 1;
	int fa = 2;
	switch ( vp->mode )
	{
	case hammergtk::ViewMode::Front:
		ua = 0;
		va = 2;
		break;
	case hammergtk::ViewMode::Side:
		ua = 1;
		va = 2;
		break;
	default:
		break;
	}
	(void)fa;
	const char *letters = "xyz";
	char buf[96];
	std::snprintf( buf, sizeof( buf ), "%c %.0f  %c %.0f", letters[ua], u, letters[va], v );
	gtk_label_set_text( st->coordLabel, buf );
}

void OnMotion( GtkEventControllerMotion *, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->cursorX = x;
	vp->cursorY = y;
	UpdateCoords( vp );
}

// Tool drag (left button): 2D views drive the active editing tool; the camera
// view orbits.
void OnToolBegin( GtkGestureDrag *, double startX, double startY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->startX = startX;
	vp->startY = startY;
	vp->orbitPrevX = 0.0;
	vp->orbitPrevY = 0.0;
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}
	double u = 0.0;
	double v = 0.0;
	WidgetToWorld( vp, startX, startY, u, v );
	vp->app->controller.PointerDown( vp->vid, u, v );
	RefreshScene( vp->app, false );
}

void OnToolUpdate( GtkGestureDrag *, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		const double dx = offX - vp->orbitPrevX;
		const double dy = offY - vp->orbitPrevY;
		vp->orbitPrevX = offX;
		vp->orbitPrevY = offY;
		vp->renderer.DragBy( static_cast<float>( dx ), static_cast<float>( dy ) );
		gtk_gl_area_queue_render( vp->area );
		return;
	}
	double u = 0.0;
	double v = 0.0;
	WidgetToWorld( vp, vp->startX + offX, vp->startY + offY, u, v );
	vp->app->controller.PointerDrag( vp->vid, u, v );
	RefreshScene( vp->app, false );
}

void OnToolEnd( GtkGestureDrag *, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}
	double u = 0.0;
	double v = 0.0;
	WidgetToWorld( vp, vp->startX + offX, vp->startY + offY, u, v );
	vp->app->controller.PointerUp( vp->vid, u, v );
	RefreshScene( vp->app, false );
}

// Pan drag (middle button): pans a 2D view / orbits the camera.
void OnPanBegin( GtkGestureDrag *, double, double, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->panPrevX = 0.0;
	vp->panPrevY = 0.0;
}

void OnPanUpdate( GtkGestureDrag *, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const double dx = offX - vp->panPrevX;
	const double dy = offY - vp->panPrevY;
	vp->panPrevX = offX;
	vp->panPrevY = offY;
	vp->renderer.DragBy( static_cast<float>( dx ), static_cast<float>( dy ) );
	gtk_gl_area_queue_render( vp->area );
}

gboolean OnScroll( GtkEventControllerScroll *ctrl, double dx, double dy, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
	const float px = static_cast<float>( vp->cursorX * scale );
	const float py = static_cast<float>( vp->cursorY * scale );

	const GdkScrollUnit unit = gtk_event_controller_scroll_get_unit( ctrl );
	const float gain = ( unit == GDK_SCROLL_UNIT_WHEEL ) ? 42.0f : 2.0f;
	const GdkModifierType state =
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( ctrl ) );
	const bool ctrlHeld = ( state & GDK_CONTROL_MASK ) != 0;

	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		if ( ctrlHeld )
		{
			vp->renderer.ZoomBy( std::pow( 1.1f, static_cast<float>( dy ) ) );
		}
		else
		{
			vp->renderer.PanScroll(
			    static_cast<float>( dx ) * gain * 0.5f, static_cast<float>( dy ) * gain * 0.5f );
		}
	}
	else if ( ctrlHeld )
	{
		vp->renderer.ZoomAtPixel( std::pow( 1.1f, static_cast<float>( -dy ) ), px, py, w, h );
		UpdateCoords( vp );
	}
	else
	{
		vp->renderer.PanScroll( static_cast<float>( dx ) * gain, static_cast<float>( dy ) * gain );
		UpdateCoords( vp );
	}
	gtk_gl_area_queue_render( vp->area );
	return TRUE;
}

void OnZoomBegin( GtkGesture *, GdkEventSequence *, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->pinchPrev = 1.0;
}

void OnZoomChanged( GtkGestureZoom *zoom, double scaleRatio, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->pinchPrev <= 0.0 )
	{
		vp->pinchPrev = 1.0;
	}
	const double delta = scaleRatio / vp->pinchPrev;
	vp->pinchPrev = scaleRatio;
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
	double cx = vp->cursorX;
	double cy = vp->cursorY;
	gtk_gesture_get_bounding_box_center( GTK_GESTURE( zoom ), &cx, &cy );
	vp->renderer.ZoomAtPixel( static_cast<float>( delta ), static_cast<float>( cx * scale ),
	    static_cast<float>( cy * scale ), w, h );
	gtk_gl_area_queue_render( vp->area );
}

gboolean OnKeyPressed(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	switch ( keyval )
	{
	case GDK_KEY_Return:
	case GDK_KEY_KP_Enter:
		if ( st->controller.Commit() )
		{
			RefreshScene( st, false );
			SetHelp( st, "Brush created" );
		}
		return TRUE;
	case GDK_KEY_Delete:
	case GDK_KEY_BackSpace:
		if ( st->controller.DeleteSelection() )
		{
			RefreshScene( st, false );
		}
		return TRUE;
	case GDK_KEY_Escape:
		SetToolUi( st, st->controller.CurrentTool() ); // clears any pending box
		return TRUE;
	case GDK_KEY_b:
	case GDK_KEY_B:
		SetToolUi( st, hammer::app::Tool::Block );
		return TRUE;
	case GDK_KEY_s:
	case GDK_KEY_S:
		SetToolUi( st, hammer::app::Tool::Select );
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

// ---- Widget construction ---------------------------------------------------

GtkWidget *MakeViewport(
    AppState *st, int index, hammergtk::ViewMode mode, hammer::app::ViewId vid, const char *label )
{
	Viewport &vp = st->viewports[index];
	vp.app = st;
	vp.mode = mode;
	vp.vid = vid;
	vp.label = label;

	GtkWidget *glarea = gtk_gl_area_new();
	vp.area = GTK_GL_AREA( glarea );
	gtk_gl_area_set_allowed_apis( GTK_GL_AREA( glarea ), GDK_GL_API_GL );
	gtk_gl_area_set_required_version( GTK_GL_AREA( glarea ), 3, 3 );
	gtk_gl_area_set_has_depth_buffer( GTK_GL_AREA( glarea ), TRUE );
	gtk_widget_set_hexpand( glarea, TRUE );
	gtk_widget_set_vexpand( glarea, TRUE );
	g_signal_connect( glarea, "realize", G_CALLBACK( OnGlRealize ), &vp );
	g_signal_connect( glarea, "unrealize", G_CALLBACK( OnGlUnrealize ), &vp );
	g_signal_connect( glarea, "render", G_CALLBACK( OnGlRender ), &vp );

	// Left button: tool / orbit.
	GtkGesture *tool = gtk_gesture_drag_new();
	gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( tool ), GDK_BUTTON_PRIMARY );
	g_signal_connect( tool, "drag-begin", G_CALLBACK( OnToolBegin ), &vp );
	g_signal_connect( tool, "drag-update", G_CALLBACK( OnToolUpdate ), &vp );
	g_signal_connect( tool, "drag-end", G_CALLBACK( OnToolEnd ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( tool ) );

	// Middle button: pan / orbit.
	GtkGesture *pan = gtk_gesture_drag_new();
	gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( pan ), GDK_BUTTON_MIDDLE );
	g_signal_connect( pan, "drag-begin", G_CALLBACK( OnPanBegin ), &vp );
	g_signal_connect( pan, "drag-update", G_CALLBACK( OnPanUpdate ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( pan ) );

	// Touchpad two-finger scroll (pan/orbit, kinetic) + ctrl/pinch zoom.
	GtkEventController *scroll =
	    gtk_event_controller_scroll_new( static_cast<GtkEventControllerScrollFlags>(
	        GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES | GTK_EVENT_CONTROLLER_SCROLL_KINETIC ) );
	g_signal_connect( scroll, "scroll", G_CALLBACK( OnScroll ), &vp );
	gtk_widget_add_controller( glarea, scroll );

	GtkGesture *pinch = gtk_gesture_zoom_new();
	g_signal_connect( pinch, "begin", G_CALLBACK( OnZoomBegin ), &vp );
	g_signal_connect( pinch, "scale-changed", G_CALLBACK( OnZoomChanged ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( pinch ) );

	GtkEventController *motion = gtk_event_controller_motion_new();
	g_signal_connect( motion, "motion", G_CALLBACK( OnMotion ), &vp );
	gtk_widget_add_controller( glarea, motion );

	GtkWidget *overlay = gtk_overlay_new();
	gtk_overlay_set_child( GTK_OVERLAY( overlay ), glarea );
	GtkWidget *caption = gtk_label_new( label );
	gtk_widget_add_css_class( caption, "osd" );
	gtk_widget_set_halign( caption, GTK_ALIGN_START );
	gtk_widget_set_valign( caption, GTK_ALIGN_START );
	gtk_widget_set_margin_start( caption, 4 );
	gtk_widget_set_margin_top( caption, 4 );
	gtk_overlay_add_overlay( GTK_OVERLAY( overlay ), caption );

	GtkWidget *frame = gtk_frame_new( nullptr );
	gtk_frame_set_child( GTK_FRAME( frame ), overlay );
	return frame;
}

void OnSelectToggled( GtkToggleButton *btn, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->suppressToolSignal )
	{
		return;
	}
	if ( gtk_toggle_button_get_active( btn ) )
	{
		SetToolUi( st, hammer::app::Tool::Select );
	}
}

void OnBlockToggled( GtkToggleButton *btn, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->suppressToolSignal )
	{
		return;
	}
	if ( gtk_toggle_button_get_active( btn ) )
	{
		SetToolUi( st, hammer::app::Tool::Block );
	}
}

GtkWidget *MakeToolPalette( AppState *st )
{
	GtkWidget *palette = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
	gtk_widget_set_margin_start( palette, 3 );
	gtk_widget_set_margin_end( palette, 3 );
	gtk_widget_set_margin_top( palette, 3 );

	GtkWidget *select = gtk_toggle_button_new();
	gtk_button_set_child(
	    GTK_BUTTON( select ), gtk_image_new_from_icon_name( "edit-select-all-symbolic" ) );
	gtk_widget_set_tooltip_text( select, "Selection Tool (S)" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( select ), TRUE );
	st->selectBtn = GTK_TOGGLE_BUTTON( select );
	g_signal_connect( select, "toggled", G_CALLBACK( OnSelectToggled ), st );
	gtk_box_append( GTK_BOX( palette ), select );

	GtkWidget *block = gtk_toggle_button_new();
	gtk_button_set_child(
	    GTK_BUTTON( block ), gtk_image_new_from_icon_name( "view-grid-symbolic" ) );
	gtk_widget_set_tooltip_text( block, "Block Tool (B)" );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( block ), GTK_TOGGLE_BUTTON( select ) );
	st->blockBtn = GTK_TOGGLE_BUTTON( block );
	g_signal_connect( block, "toggled", G_CALLBACK( OnBlockToggled ), st );
	gtk_box_append( GTK_BOX( palette ), block );

	// Remaining classic tools (laid out; not yet functional).
	struct Tool
	{
		const char *icon;
		const char *tip;
	};
	const Tool rest[] = {
	    { "zoom-in-symbolic", "Magnify" },
	    { "camera-photo-symbolic", "Camera" },
	    { "insert-object-symbolic", "Entity Tool" },
	    { "edit-cut-symbolic", "Clipping Tool" },
	    { "format-justify-fill-symbolic", "Vertex Tool" },
	    { "applications-graphics-symbolic", "Apply Texture" },
	    { "insert-link-symbolic", "Apply Decal" },
	};
	for ( const Tool &t : rest )
	{
		GtkWidget *b = gtk_toggle_button_new();
		gtk_button_set_child( GTK_BUTTON( b ), gtk_image_new_from_icon_name( t.icon ) );
		gtk_widget_set_tooltip_text( b, t.tip );
		gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( b ), GTK_TOGGLE_BUTTON( select ) );
		gtk_box_append( GTK_BOX( palette ), b );
	}
	return palette;
}

void DrawTextureSwatch( GtkDrawingArea *, cairo_t *cr, int w, int h, gpointer )
{
	cairo_set_source_rgb( cr, 0.42, 0.28, 0.22 );
	cairo_rectangle( cr, 0, 0, w, h );
	cairo_fill( cr );
	cairo_set_source_rgb( cr, 0.30, 0.19, 0.15 );
	cairo_set_line_width( cr, 2.0 );
	const int rows = 6;
	const int cols = 4;
	for ( int r = 0; r < rows; ++r )
	{
		const double y = ( h * ( r + 1.0 ) ) / rows;
		cairo_move_to( cr, 0, y );
		cairo_line_to( cr, w, y );
		for ( int c = 0; c < cols; ++c )
		{
			const double offset = ( r % 2 ) ? ( w / ( 2.0 * cols ) ) : 0.0;
			const double x = ( w * ( c + 1.0 ) ) / cols - offset;
			cairo_move_to( cr, x, ( h * r ) / rows );
			cairo_line_to( cr, x, y );
		}
	}
	cairo_stroke( cr );
}

GtkWidget *MakeObjectBar( AppState *st )
{
	GtkWidget *bar = gtk_box_new( GTK_ORIENTATION_VERTICAL, 8 );
	gtk_widget_set_margin_start( bar, 6 );
	gtk_widget_set_margin_end( bar, 6 );
	gtk_widget_set_margin_top( bar, 6 );
	gtk_widget_set_margin_bottom( bar, 6 );

	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Select:" ) );
	GtkWidget *selRow = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( selRow, "linked" );
	GtkWidget *groups = gtk_toggle_button_new_with_label( "Groups" );
	GtkWidget *objects = gtk_toggle_button_new_with_label( "Objects" );
	GtkWidget *solids = gtk_toggle_button_new_with_label( "Solids" );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( objects ), GTK_TOGGLE_BUTTON( groups ) );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( solids ), GTK_TOGGLE_BUTTON( groups ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( objects ), TRUE );
	for ( GtkWidget *b : { groups, objects, solids } )
	{
		gtk_widget_set_hexpand( b, TRUE );
		gtk_box_append( GTK_BOX( selRow ), b );
	}
	gtk_box_append( GTK_BOX( bar ), selRow );

	GtkWidget *count = gtk_label_new( "no map loaded" );
	gtk_label_set_xalign( GTK_LABEL( count ), 0.0f );
	gtk_widget_add_css_class( count, "dim-label" );
	st->objectsCount = GTK_LABEL( count );
	gtk_box_append( GTK_BOX( bar ), count );

	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Texture group:" ) );
	const char *groupsList[] = { "All Textures", "brick", "concrete", "dev", "metal", nullptr };
	gtk_box_append( GTK_BOX( bar ), gtk_drop_down_new_from_strings( groupsList ) );

	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Current texture:" ) );
	GtkWidget *swatch = gtk_drawing_area_new();
	gtk_widget_set_size_request( swatch, -1, 100 );
	gtk_drawing_area_set_draw_func(
	    GTK_DRAWING_AREA( swatch ), DrawTextureSwatch, nullptr, nullptr );
	gtk_box_append( GTK_BOX( bar ), swatch );
	GtkWidget *texName = gtk_label_new( "brick/brickfloor001a  512x512" );
	gtk_widget_add_css_class( texName, "caption" );
	gtk_box_append( GTK_BOX( bar ), texName );

	GtkWidget *texBtns = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( texBtns, "linked" );
	for ( const char *name : { "Browse…", "Replace…" } )
	{
		GtkWidget *b = gtk_button_new_with_label( name );
		gtk_widget_set_hexpand( b, TRUE );
		gtk_box_append( GTK_BOX( texBtns ), b );
	}
	gtk_box_append( GTK_BOX( bar ), texBtns );

	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "VisGroups:" ) );
	GtkWidget *vgScroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand( vgScroll, TRUE );
	GtkWidget *vg = gtk_list_box_new();
	gtk_list_box_set_selection_mode( GTK_LIST_BOX( vg ), GTK_SELECTION_NONE );
	for ( const char *name : { "World geometry", "Entities", "Displacements", "Triggers" } )
	{
		GtkWidget *row = gtk_list_box_row_new();
		GtkWidget *box = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
		GtkWidget *check = gtk_check_button_new();
		gtk_check_button_set_active( GTK_CHECK_BUTTON( check ), TRUE );
		GtkWidget *lbl = gtk_label_new( name );
		gtk_box_append( GTK_BOX( box ), check );
		gtk_box_append( GTK_BOX( box ), lbl );
		gtk_list_box_row_set_child( GTK_LIST_BOX_ROW( row ), box );
		gtk_list_box_append( GTK_LIST_BOX( vg ), row );
	}
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( vgScroll ), vg );
	gtk_box_append( GTK_BOX( bar ), vgScroll );

	GtkWidget *vgBtns = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( vgBtns, "linked" );
	for ( const char *name : { "Show", "Edit", "Mark" } )
	{
		GtkWidget *b = gtk_button_new_with_label( name );
		gtk_widget_set_hexpand( b, TRUE );
		gtk_box_append( GTK_BOX( vgBtns ), b );
	}
	gtk_box_append( GTK_BOX( bar ), vgBtns );

	return bar;
}

GtkWidget *MakeToolbar()
{
	GtkWidget *tb = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 2 );
	gtk_widget_add_css_class( tb, "toolbar" );
	gtk_widget_set_margin_start( tb, 4 );
	gtk_widget_set_margin_end( tb, 4 );
	gtk_widget_set_margin_top( tb, 2 );
	gtk_widget_set_margin_bottom( tb, 2 );

	struct Item
	{
		const char *icon;
		const char *tip;
		const char *action;
	};
	const Item items[] = {
	    { "document-new-symbolic", "New", "app.new" },
	    { "document-open-symbolic", "Open", "app.open" },
	    { "document-save-symbolic", "Save", "app.save" },
	    { "edit-undo-symbolic", "Undo", "app.undo" },
	    { "edit-redo-symbolic", "Redo", "app.redo" },
	    { "edit-delete-symbolic", "Delete", "app.delete" },
	    { "view-restore-symbolic", "Reset Views", "app.reset-views" },
	};
	for ( const Item &it : items )
	{
		GtkWidget *b = gtk_button_new_from_icon_name( it.icon );
		gtk_widget_set_tooltip_text( b, it.tip );
		gtk_button_set_has_frame( GTK_BUTTON( b ), FALSE );
		gtk_actionable_set_action_name( GTK_ACTIONABLE( b ), it.action );
		gtk_box_append( GTK_BOX( tb ), b );
	}
	return tb;
}

GtkWidget *MakeStatusBar( AppState *st )
{
	GtkWidget *bar = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 12 );
	gtk_widget_add_css_class( bar, "statusbar" );
	gtk_widget_set_margin_start( bar, 8 );
	gtk_widget_set_margin_end( bar, 8 );
	gtk_widget_set_margin_top( bar, 2 );
	gtk_widget_set_margin_bottom( bar, 2 );

	GtkWidget *help =
	    gtk_label_new( "For Help, press F1  ·  drag to draw/select, MMB or scroll to pan" );
	gtk_label_set_xalign( GTK_LABEL( help ), 0.0f );
	gtk_widget_set_hexpand( help, TRUE );
	st->helpLabel = GTK_LABEL( help );
	gtk_box_append( GTK_BOX( bar ), help );

	GtkWidget *coord = gtk_label_new( "x 0  y 0" );
	st->coordLabel = GTK_LABEL( coord );
	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( bar ), coord );

	GtkWidget *snap = gtk_label_new( "Grid: 64  ·  Select" );
	st->snapLabel = GTK_LABEL( snap );
	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( bar ), snap );

	return bar;
}

GtkWidget *MakeViewportGrid( AppState *st )
{
	GtkWidget *topPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child(
	    GTK_PANED( topPane ), MakeViewport( st, 0, hammergtk::ViewMode::Perspective,
	                              hammer::app::ViewId::Camera, "camera" ) );
	gtk_paned_set_end_child( GTK_PANED( topPane ),
	    MakeViewport( st, 1, hammergtk::ViewMode::Top, hammer::app::ViewId::Top, "top (x/y)" ) );
	gtk_paned_set_resize_start_child( GTK_PANED( topPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( topPane ), TRUE );

	GtkWidget *bottomPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child(
	    GTK_PANED( bottomPane ), MakeViewport( st, 2, hammergtk::ViewMode::Front,
	                                 hammer::app::ViewId::Front, "front (x/z)" ) );
	gtk_paned_set_end_child( GTK_PANED( bottomPane ),
	    MakeViewport( st, 3, hammergtk::ViewMode::Side, hammer::app::ViewId::Side, "side (y/z)" ) );
	gtk_paned_set_resize_start_child( GTK_PANED( bottomPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( bottomPane ), TRUE );

	GtkWidget *vPane = gtk_paned_new( GTK_ORIENTATION_VERTICAL );
	gtk_paned_set_start_child( GTK_PANED( vPane ), topPane );
	gtk_paned_set_end_child( GTK_PANED( vPane ), bottomPane );
	gtk_paned_set_resize_start_child( GTK_PANED( vPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( vPane ), TRUE );
	gtk_widget_set_hexpand( vPane, TRUE );
	gtk_widget_set_vexpand( vPane, TRUE );
	return vPane;
}

GMenu *MakeMenuModel()
{
	GMenu *bar = g_menu_new();

	GMenu *file = g_menu_new();
	g_menu_append( file, "New", "app.new" );
	g_menu_append( file, "Open…", "app.open" );
	g_menu_append( file, "Save", "app.save" );
	g_menu_append( file, "Save As…", "app.saveas" );
	GMenu *fileEnd = g_menu_new();
	g_menu_append( fileEnd, "Quit", "app.quit" );
	g_menu_append_section( file, nullptr, G_MENU_MODEL( fileEnd ) );
	g_menu_append_submenu( bar, "File", G_MENU_MODEL( file ) );
	g_object_unref( fileEnd );

	GMenu *edit = g_menu_new();
	g_menu_append( edit, "Undo", "app.undo" );
	g_menu_append( edit, "Redo", "app.redo" );
	g_menu_append( edit, "Delete", "app.delete" );
	g_menu_append_submenu( bar, "Edit", G_MENU_MODEL( edit ) );

	GMenu *tools = g_menu_new();
	g_menu_append( tools, "Selection Tool", "app.tool-select" );
	g_menu_append( tools, "Block Tool", "app.tool-block" );
	g_menu_append_submenu( bar, "Tools", G_MENU_MODEL( tools ) );

	GMenu *view = g_menu_new();
	g_menu_append( view, "Reset Views", "app.reset-views" );
	g_menu_append_submenu( bar, "View", G_MENU_MODEL( view ) );

	for ( const char *name : { "Map", "Instancing", "Window", "Help" } )
	{
		GMenu *stub = g_menu_new();
		g_menu_append( stub, "(not implemented)", "app.noop" );
		g_menu_append_submenu( bar, name, G_MENU_MODEL( stub ) );
		g_object_unref( stub );
	}

	g_object_unref( file );
	g_object_unref( edit );
	g_object_unref( tools );
	g_object_unref( view );
	return bar;
}

void OnActivate( GtkApplication *app, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->application = app;

	GtkWidget *window = gtk_application_window_new( app );
	st->window = window;
	gtk_window_set_default_size( GTK_WINDOW( window ), 1320, 840 );
	gtk_window_set_title( GTK_WINDOW( window ), "Hammer - [untitled]" );

	GtkWidget *root = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );

	GMenu *menuModel = MakeMenuModel();
	GtkWidget *menubar = gtk_popover_menu_bar_new_from_model( G_MENU_MODEL( menuModel ) );
	g_object_unref( menuModel );
	gtk_box_append( GTK_BOX( root ), menubar );

	gtk_box_append( GTK_BOX( root ), MakeToolbar() );
	gtk_box_append( GTK_BOX( root ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	// Body: [tool palette | [viewports | object bar]] with drag-resizable panels.
	GtkWidget *rightPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child( GTK_PANED( rightPane ), MakeViewportGrid( st ) );
	gtk_paned_set_end_child( GTK_PANED( rightPane ), MakeObjectBar( st ) );
	gtk_paned_set_resize_start_child( GTK_PANED( rightPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( rightPane ), FALSE );
	gtk_paned_set_shrink_end_child( GTK_PANED( rightPane ), FALSE );
	gtk_paned_set_position( GTK_PANED( rightPane ), 1050 );

	GtkWidget *bodyPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child( GTK_PANED( bodyPane ), MakeToolPalette( st ) );
	gtk_paned_set_end_child( GTK_PANED( bodyPane ), rightPane );
	gtk_paned_set_resize_start_child( GTK_PANED( bodyPane ), FALSE );
	gtk_paned_set_shrink_start_child( GTK_PANED( bodyPane ), FALSE );
	gtk_paned_set_resize_end_child( GTK_PANED( bodyPane ), TRUE );
	gtk_paned_set_position( GTK_PANED( bodyPane ), 48 );
	gtk_widget_set_vexpand( bodyPane, TRUE );
	gtk_box_append( GTK_BOX( root ), bodyPane );

	gtk_box_append( GTK_BOX( root ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );
	gtk_box_append( GTK_BOX( root ), MakeStatusBar( st ) );

	// Window-level key handling (Enter/Delete/Esc/tool shortcuts).
	GtkEventController *keys = gtk_event_controller_key_new();
	g_signal_connect( keys, "key-pressed", G_CALLBACK( OnKeyPressed ), st );
	gtk_widget_add_controller( window, keys );

	gtk_window_set_child( GTK_WINDOW( window ), root );

	if ( !st->openOnStart.empty() )
	{
		DoOpen( st, st->openOnStart );
	}

	gtk_window_present( GTK_WINDOW( window ) );
}

void AddActions( GtkApplication *app, AppState *st )
{
	const GActionEntry entries[] = {
	    { "new", ActionNew, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "open", ActionOpen, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "save", ActionSave, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "saveas", ActionSaveAs, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "quit", ActionQuit, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "undo", ActionUndo, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "redo", ActionRedo, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "delete", ActionDelete, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "reset-views", ActionResetViews, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "tool-select", ActionToolSelect, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "tool-block", ActionToolBlock, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "noop", nullptr, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	};
	g_action_map_add_action_entries( G_ACTION_MAP( app ), entries, G_N_ELEMENTS( entries ), st );

	struct Accel
	{
		const char *action;
		const char *accel;
	};
	const Accel accels[] = {
	    { "app.new", "<Control>n" },
	    { "app.open", "<Control>o" },
	    { "app.save", "<Control>s" },
	    { "app.saveas", "<Control><Shift>s" },
	    { "app.quit", "<Control>q" },
	    { "app.undo", "<Control>z" },
	    { "app.redo", "<Control>y" },
	    { "app.reset-views", "<Control>r" },
	};
	for ( const Accel &a : accels )
	{
		const char *v[] = { a.accel, nullptr };
		gtk_application_set_accels_for_action( app, a.action, v );
	}
}

int RunApp( AppState *st, char **argv )
{
	AdwApplication *app = adw_application_new( kAppId, G_APPLICATION_NON_UNIQUE );
	AddActions( GTK_APPLICATION( app ), st );
	g_signal_connect( app, "activate", G_CALLBACK( OnActivate ), st );
	char *only[] = { argv[0], nullptr };
	const int status = g_application_run( G_APPLICATION( app ), 1, only );
	g_object_unref( app );
	return status;
}

} // namespace

// Headless offscreen rendering (offscreen.cpp).
int RenderScreenshot(
    const std::string &vmfPath, const std::string &outPpm, int width, int height );
int RenderQuad( const std::string &vmfPath, const std::string &outPpm, int tileW, int tileH );
int RenderControllerDemo( const std::string &outPpm, int tileW, int tileH );
int RenderControllerLoad(
    const std::string &vmfPath, const std::string &outPpm, int tileW, int tileH );

int main( int argc, char **argv )
{
	std::string screenshotOut;
	std::string screenshotIn;
	std::string quadOut;
	std::string quadIn;
	std::string demoOut;
	std::string cquadOut;
	std::string cquadIn;
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
		else if ( a == "--quad" && i + 2 < argc )
		{
			quadOut = argv[++i];
			quadIn = argv[++i];
		}
		else if ( a == "--demo" && i + 1 < argc )
		{
			demoOut = argv[++i];
		}
		else if ( a == "--cquad" && i + 2 < argc )
		{
			cquadOut = argv[++i];
			cquadIn = argv[++i];
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
			             "       hammer_gtk --screenshot OUT.ppm MAP.vmf [--width W --height H]\n"
			             "       hammer_gtk --quad OUT.ppm MAP.vmf [--width W --height H]\n" );
			return 0;
		}
	}

	if ( !screenshotOut.empty() )
	{
		return RenderScreenshot( screenshotIn, screenshotOut, width, height );
	}
	if ( !quadOut.empty() )
	{
		return RenderQuad( quadIn, quadOut, width / 2, height / 2 );
	}
	if ( !demoOut.empty() )
	{
		return RenderControllerDemo( demoOut, width / 2, height / 2 );
	}
	if ( !cquadOut.empty() )
	{
		return RenderControllerLoad( cquadIn, cquadOut, width / 2, height / 2 );
	}

	AppState st;
	st.openOnStart = openPath;
	return RunApp( &st, argv );
}
