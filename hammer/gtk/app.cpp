//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GTK4 + libadwaita desktop shell for Hammer (RFC 0002,
//			linux-gtk-desktop). This is the delivery-target host: it composes the
//			strict headless editor core (hammer::app::EditorDocument, the VMF codec,
//			and the VMF -> convex brush geometry bridge) with a native UI laid out
//			like the classic Hammer editor -- menu bar, tool palette, the four
//			viewports (3D camera + Top/Front/Side 2D wireframes), the object bar,
//			and a status bar -- plus modern touchpad navigation (two-finger pan,
//			pinch-to-zoom anchored at the cursor, kinetic scrolling).
//
//			All GTK/GDK/GL native detail is confined to this product; the editor
//			core it drives has no display, GPU, or MFC dependency. A headless
//			"--screenshot OUT.ppm IN.vmf" path (offscreen.cpp) shares the same
//			renderer so the 3D preview can be verified without a window server.
//
//=============================================================================//

#include "hammer/app/editor_document.h"
#include "hammer/geometry/brush.h"

#include "posix_file_store.h"
#include "renderer.h"

#include <array>
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

struct AppState;

// One of the four Hammer viewports: a GtkGLArea driven by its own Renderer in a
// fixed view mode, plus per-view input bookkeeping.
struct Viewport
{
	AppState *app = nullptr;
	hammergtk::Renderer renderer;
	hammergtk::ViewMode mode = hammergtk::ViewMode::Perspective;
	const char *label = "";
	GtkGLArea *area = nullptr;
	bool glReady = false;
	bool pending = false;
	double dragPrevX = 0.0;
	double dragPrevY = 0.0;
	double cursorX = 0.0;
	double cursorY = 0.0;
	double pinchPrev = 1.0;
};

struct AppState
{
	std::array<Viewport, 4> viewports;
	hammer::geometry::WorldScene scene;
	std::string currentPath;
	std::string openOnStart;

	GtkApplication *application = nullptr;
	GtkWidget *window = nullptr;
	GtkListBox *visgroups = nullptr;
	GtkLabel *helpLabel = nullptr;
	GtkLabel *coordLabel = nullptr;
	GtkLabel *snapLabel = nullptr;
	GtkButton *groupsBtn = nullptr;
	GtkLabel *objectsCount = nullptr;
};

// ---------------------------------------------------------------------------
// Shared load path: VMF text -> document -> renderable scene.
// ---------------------------------------------------------------------------

bool LoadSceneFromFile(
    const std::string &path, hammer::geometry::WorldScene &scene, std::string &error )
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

void SetHelp( AppState *st, const std::string &text )
{
	if ( st->helpLabel )
	{
		gtk_label_set_text( st->helpLabel, text.c_str() );
	}
}

const char *AxisLetter( int axis )
{
	return axis == 0 ? "x" : ( axis == 1 ? "y" : "z" );
}

void ViewAxes( hammergtk::ViewMode mode, int &u, int &v )
{
	switch ( mode )
	{
	case hammergtk::ViewMode::Top:
		u = 0;
		v = 1;
		break;
	case hammergtk::ViewMode::Front:
		u = 0;
		v = 2;
		break;
	case hammergtk::ViewMode::Side:
		u = 1;
		v = 2;
		break;
	default:
		u = 0;
		v = 1;
		break;
	}
}

void PopulateVisGroups( AppState *st )
{
	if ( !st->visgroups )
	{
		return;
	}
	gtk_list_box_remove_all( st->visgroups );

	// VisGroups in Hammer classify map objects; here we surface the map's entity
	// classes as toggleable groups, mirroring the classic checkbox list.
	auto addRow = [&]( const char *name, bool checked )
	{
		GtkWidget *row = gtk_list_box_row_new();
		GtkWidget *box = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
		gtk_widget_set_margin_start( box, 4 );
		gtk_widget_set_margin_top( box, 1 );
		gtk_widget_set_margin_bottom( box, 1 );
		GtkWidget *check = gtk_check_button_new();
		gtk_check_button_set_active( GTK_CHECK_BUTTON( check ), checked );
		GtkWidget *lbl = gtk_label_new( name );
		gtk_label_set_xalign( GTK_LABEL( lbl ), 0.0f );
		gtk_box_append( GTK_BOX( box ), check );
		gtk_box_append( GTK_BOX( box ), lbl );
		gtk_list_box_row_set_child( GTK_LIST_BOX_ROW( row ), box );
		gtk_list_box_row_set_activatable( GTK_LIST_BOX_ROW( row ), FALSE );
		gtk_list_box_append( st->visgroups, row );
	};

	addRow( "World geometry", true );
	bool seen[64] = { false };
	int distinct = 0;
	for ( const hammer::geometry::SceneEntity &e : st->scene.entities )
	{
		// De-duplicate classnames cheaply for the small maps this slice targets.
		std::size_t h = std::hash<std::string>{}( e.classname ) % 64;
		if ( seen[h] )
		{
			continue;
		}
		seen[h] = true;
		addRow( e.classname.empty() ? "(entity)" : e.classname.c_str(), true );
		if ( ++distinct >= 32 )
		{
			break;
		}
	}
}

void ApplyScene( AppState *st )
{
	for ( Viewport &vp : st->viewports )
	{
		if ( vp.glReady && vp.area )
		{
			gtk_gl_area_make_current( vp.area );
			if ( gtk_gl_area_get_error( vp.area ) == nullptr )
			{
				vp.renderer.SetScene( st->scene );
			}
			vp.pending = false;
			gtk_gl_area_queue_render( vp.area );
		}
		else
		{
			vp.pending = true;
		}
	}

	PopulateVisGroups( st );

	if ( st->objectsCount )
	{
		char buf[128];
		std::snprintf( buf, sizeof( buf ), "%zu solids · %zu faces · %zu entities",
		    st->scene.solids.size(), st->scene.TotalFaces(), st->scene.entities.size() );
		gtk_label_set_text( st->objectsCount, buf );
	}

	const std::string base = st->currentPath.substr( st->currentPath.find_last_of( "/" ) + 1 );
	if ( st->window )
	{
		gtk_window_set_title( GTK_WINDOW( st->window ), ( "Hammer - [" + base + "]" ).c_str() );
	}
	SetHelp( st, "Loaded " + base );
}

void LoadPath( AppState *st, const std::string &path )
{
	hammer::geometry::WorldScene scene;
	std::string error;
	if ( !LoadSceneFromFile( path, scene, error ) )
	{
		SetHelp( st, std::string( "Failed to open: " ) + error );
		return;
	}
	st->scene = std::move( scene );
	st->currentPath = path;
	ApplyScene( st );
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
		return;
	}
	char *path = g_file_get_path( file );
	if ( path )
	{
		LoadPath( st, path );
		g_free( path );
	}
	g_object_unref( file );
}

void OpenDialog( AppState *st )
{
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

// ---- GAction handlers ------------------------------------------------------

void ActionOpen( GSimpleAction *, GVariant *, gpointer user_data )
{
	OpenDialog( static_cast<AppState *>( user_data ) );
}

void ActionQuit( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->window )
	{
		gtk_window_close( GTK_WINDOW( st->window ) );
	}
}

void ActionResetViews( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	for ( Viewport &vp : st->viewports )
	{
		vp.renderer.FrameScene();
		if ( vp.area )
		{
			gtk_gl_area_queue_render( vp.area );
		}
	}
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
	if ( vp->pending )
	{
		vp->renderer.SetScene( vp->app->scene );
		vp->pending = false;
	}
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

// ---- Input: drag, scroll, pinch, motion ------------------------------------

void UpdateCoords( Viewport *vp )
{
	AppState *st = vp->app;
	if ( !st->coordLabel || vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
	float wu = 0.0f;
	float wv = 0.0f;
	vp->renderer.PixelToWorld( static_cast<float>( vp->cursorX * scale ),
	    static_cast<float>( vp->cursorY * scale ), w, h, wu, wv );
	int u = 0;
	int v = 1;
	ViewAxes( vp->mode, u, v );
	char buf[96];
	std::snprintf(
	    buf, sizeof( buf ), "%s %.0f  %s %.0f", AxisLetter( u ), wu, AxisLetter( v ), wv );
	gtk_label_set_text( st->coordLabel, buf );
}

void OnMotion( GtkEventControllerMotion *, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->cursorX = x;
	vp->cursorY = y;
	UpdateCoords( vp );
}

void OnDragBegin( GtkGestureDrag *, double, double, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->dragPrevX = 0.0;
	vp->dragPrevY = 0.0;
}

void OnDragUpdate( GtkGestureDrag *, double offsetX, double offsetY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const double dx = offsetX - vp->dragPrevX;
	const double dy = offsetY - vp->dragPrevY;
	vp->dragPrevX = offsetX;
	vp->dragPrevY = offsetY;
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
			// Two-finger scroll orbits the camera.
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

// ---- Widget construction ---------------------------------------------------

GtkWidget *MakeViewport( AppState *st, int index, hammergtk::ViewMode mode, const char *label )
{
	Viewport &vp = st->viewports[index];
	vp.app = st;
	vp.mode = mode;
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

	// Mouse drag (orbit/pan fallback).
	GtkGesture *drag = gtk_gesture_drag_new();
	g_signal_connect( drag, "drag-begin", G_CALLBACK( OnDragBegin ), &vp );
	g_signal_connect( drag, "drag-update", G_CALLBACK( OnDragUpdate ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( drag ) );

	// Touchpad two-finger scroll (pan/orbit) with kinetic momentum; ctrl = zoom.
	GtkEventController *scroll =
	    gtk_event_controller_scroll_new( static_cast<GtkEventControllerScrollFlags>(
	        GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES | GTK_EVENT_CONTROLLER_SCROLL_KINETIC ) );
	g_signal_connect( scroll, "scroll", G_CALLBACK( OnScroll ), &vp );
	gtk_widget_add_controller( glarea, scroll );

	// Pinch-to-zoom anchored at the pinch centre.
	GtkGesture *pinch = gtk_gesture_zoom_new();
	g_signal_connect( pinch, "begin", G_CALLBACK( OnZoomBegin ), &vp );
	g_signal_connect( pinch, "scale-changed", G_CALLBACK( OnZoomChanged ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( pinch ) );

	// Motion for the coordinate read-out.
	GtkEventController *motion = gtk_event_controller_motion_new();
	g_signal_connect( motion, "motion", G_CALLBACK( OnMotion ), &vp );
	gtk_widget_add_controller( glarea, motion );

	// A corner label like Hammer's per-view caption.
	GtkWidget *overlay = gtk_overlay_new();
	gtk_overlay_set_child( GTK_OVERLAY( overlay ), glarea );
	GtkWidget *caption = gtk_label_new( label );
	gtk_widget_add_css_class( caption, "osd" );
	gtk_widget_set_halign( caption, GTK_ALIGN_START );
	gtk_widget_set_valign( caption, GTK_ALIGN_START );
	gtk_widget_set_margin_start( caption, 4 );
	gtk_widget_set_margin_top( caption, 4 );
	gtk_label_set_xalign( GTK_LABEL( caption ), 0.0f );
	gtk_overlay_add_overlay( GTK_OVERLAY( overlay ), caption );

	GtkWidget *frame = gtk_frame_new( nullptr );
	gtk_frame_set_child( GTK_FRAME( frame ), overlay );
	return frame;
}

GtkWidget *MakeToolButton( const char *icon, const char *tip )
{
	GtkWidget *b = gtk_button_new_from_icon_name( icon );
	gtk_widget_set_tooltip_text( b, tip );
	gtk_button_set_has_frame( GTK_BUTTON( b ), FALSE );
	return b;
}

GtkWidget *MakeToggleTool( const char *icon, const char *tip, GtkToggleButton *group, bool active )
{
	GtkWidget *b = gtk_toggle_button_new();
	gtk_button_set_child( GTK_BUTTON( b ), gtk_image_new_from_icon_name( icon ) );
	gtk_widget_set_tooltip_text( b, tip );
	if ( group )
	{
		gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( b ), group );
	}
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( b ), active );
	return b;
}

void DrawTextureSwatch( GtkDrawingArea *, cairo_t *cr, int w, int h, gpointer )
{
	// A brick-ish placeholder swatch for the "current texture" slot.
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
	gtk_widget_set_size_request( bar, 190, -1 );
	gtk_widget_set_margin_start( bar, 6 );
	gtk_widget_set_margin_end( bar, 6 );
	gtk_widget_set_margin_top( bar, 6 );
	gtk_widget_set_margin_bottom( bar, 6 );

	// Select: Groups / Objects / Solids (linked toggle row).
	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Select:" ) );
	GtkWidget *selRow = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( selRow, "linked" );
	GtkWidget *groups = gtk_toggle_button_new_with_label( "Groups" );
	GtkWidget *objects = gtk_toggle_button_new_with_label( "Objects" );
	GtkWidget *solids = gtk_toggle_button_new_with_label( "Solids" );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( objects ), GTK_TOGGLE_BUTTON( groups ) );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( solids ), GTK_TOGGLE_BUTTON( groups ) );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( objects ), TRUE );
	gtk_widget_set_hexpand( groups, TRUE );
	gtk_widget_set_hexpand( objects, TRUE );
	gtk_widget_set_hexpand( solids, TRUE );
	gtk_box_append( GTK_BOX( selRow ), groups );
	gtk_box_append( GTK_BOX( selRow ), objects );
	gtk_box_append( GTK_BOX( selRow ), solids );
	gtk_box_append( GTK_BOX( bar ), selRow );
	st->groupsBtn = GTK_BUTTON( groups );

	GtkWidget *count = gtk_label_new( "no map loaded" );
	gtk_label_set_xalign( GTK_LABEL( count ), 0.0f );
	gtk_widget_add_css_class( count, "dim-label" );
	gtk_label_set_wrap( GTK_LABEL( count ), TRUE );
	st->objectsCount = GTK_LABEL( count );
	gtk_box_append( GTK_BOX( bar ), count );

	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	// Texture group + current texture.
	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Texture group:" ) );
	const char *groupsList[] = { "All Textures", "brick", "concrete", "dev", "metal", nullptr };
	GtkWidget *texGroup = gtk_drop_down_new_from_strings( groupsList );
	gtk_box_append( GTK_BOX( bar ), texGroup );

	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "Current texture:" ) );
	GtkWidget *swatch = gtk_drawing_area_new();
	gtk_widget_set_size_request( swatch, -1, 110 );
	gtk_drawing_area_set_draw_func(
	    GTK_DRAWING_AREA( swatch ), DrawTextureSwatch, nullptr, nullptr );
	gtk_box_append( GTK_BOX( bar ), swatch );
	GtkWidget *texName = gtk_label_new( "brick/brickfloor001a  512x512" );
	gtk_widget_add_css_class( texName, "caption" );
	gtk_box_append( GTK_BOX( bar ), texName );

	GtkWidget *texBtns = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( texBtns, "linked" );
	GtkWidget *browse = gtk_button_new_with_label( "Browse…" );
	GtkWidget *replace = gtk_button_new_with_label( "Replace…" );
	gtk_widget_set_hexpand( browse, TRUE );
	gtk_widget_set_hexpand( replace, TRUE );
	gtk_box_append( GTK_BOX( texBtns ), browse );
	gtk_box_append( GTK_BOX( texBtns ), replace );
	gtk_box_append( GTK_BOX( bar ), texBtns );

	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	// VisGroups.
	gtk_box_append( GTK_BOX( bar ), gtk_label_new( "VisGroups:" ) );
	GtkWidget *vgScroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand( vgScroll, TRUE );
	GtkWidget *vg = gtk_list_box_new();
	gtk_list_box_set_selection_mode( GTK_LIST_BOX( vg ), GTK_SELECTION_NONE );
	st->visgroups = GTK_LIST_BOX( vg );
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

GtkWidget *MakeToolPalette()
{
	GtkWidget *palette = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
	gtk_widget_set_margin_start( palette, 3 );
	gtk_widget_set_margin_end( palette, 3 );
	gtk_widget_set_margin_top( palette, 3 );

	struct Tool
	{
		const char *icon;
		const char *tip;
	};
	const Tool tools[] = {
	    { "edit-select-all-symbolic", "Selection Tool" },
	    { "zoom-in-symbolic", "Magnify" },
	    { "camera-photo-symbolic", "Camera" },
	    { "insert-object-symbolic", "Entity Tool" },
	    { "view-grid-symbolic", "Block Tool" },
	    { "edit-cut-symbolic", "Clipping Tool" },
	    { "format-justify-fill-symbolic", "Vertex Tool" },
	    { "applications-graphics-symbolic", "Apply Texture" },
	    { "object-flip-horizontal-symbolic", "Toggle Texture Application" },
	    { "insert-link-symbolic", "Apply Decal" },
	};
	GtkToggleButton *group = nullptr;
	bool first = true;
	for ( const Tool &t : tools )
	{
		GtkWidget *b = MakeToggleTool( t.icon, t.tip, group, first );
		if ( first )
		{
			group = GTK_TOGGLE_BUTTON( b );
			first = false;
		}
		gtk_box_append( GTK_BOX( palette ), b );
	}
	return palette;
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
	g_menu_append( edit, "Cut", "app.cut" );
	g_menu_append( edit, "Copy", "app.copy" );
	g_menu_append( edit, "Paste", "app.paste" );
	g_menu_append_submenu( bar, "Edit", G_MENU_MODEL( edit ) );

	GMenu *map = g_menu_new();
	g_menu_append( map, "Snap to Grid", "app.snap" );
	g_menu_append( map, "Show Grid", "app.grid" );
	g_menu_append( map, "Entity Report…", "app.entreport" );
	g_menu_append_submenu( bar, "Map", G_MENU_MODEL( map ) );

	GMenu *view = g_menu_new();
	g_menu_append( view, "Reset Views", "app.reset-views" );
	g_menu_append( view, "3D Textured", "app.textured" );
	g_menu_append( view, "Center on Selection", "app.center" );
	g_menu_append_submenu( bar, "View", G_MENU_MODEL( view ) );

	GMenu *tools = g_menu_new();
	g_menu_append( tools, "Apply Current Texture", "app.applytex" );
	g_menu_append( tools, "Options…", "app.options" );
	g_menu_append_submenu( bar, "Tools", G_MENU_MODEL( tools ) );

	GMenu *inst = g_menu_new();
	g_menu_append( inst, "Create Instance", "app.instance" );
	g_menu_append_submenu( bar, "Instancing", G_MENU_MODEL( inst ) );

	GMenu *win = g_menu_new();
	g_menu_append( win, "Cascade", "app.cascade" );
	g_menu_append( win, "Tile", "app.tile" );
	g_menu_append_submenu( bar, "Window", G_MENU_MODEL( win ) );

	GMenu *help = g_menu_new();
	g_menu_append( help, "About Hammer", "app.about" );
	g_menu_append_submenu( bar, "Help", G_MENU_MODEL( help ) );

	g_object_unref( file );
	g_object_unref( edit );
	g_object_unref( map );
	g_object_unref( view );
	g_object_unref( tools );
	g_object_unref( inst );
	g_object_unref( win );
	g_object_unref( help );
	return bar;
}

GtkWidget *MakeToolbar( AppState * )
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
	    { "edit-cut-symbolic", "Cut", "app.cut" },
	    { "edit-copy-symbolic", "Copy", "app.copy" },
	    { "edit-paste-symbolic", "Paste", "app.paste" },
	    { "view-restore-symbolic", "Reset Views", "app.reset-views" },
	};
	for ( const Item &it : items )
	{
		GtkWidget *b = MakeToolButton( it.icon, it.tip );
		gtk_actionable_set_action_name( GTK_ACTIONABLE( b ), it.action );
		gtk_box_append( GTK_BOX( tb ), b );
		if ( std::string( it.action ) == "app.save" || std::string( it.action ) == "app.redo" ||
		     std::string( it.action ) == "app.paste" )
		{
			gtk_box_append( GTK_BOX( tb ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
		}
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

	GtkWidget *help = gtk_label_new( "For Help, press F1" );
	gtk_label_set_xalign( GTK_LABEL( help ), 0.0f );
	gtk_widget_set_hexpand( help, TRUE );
	st->helpLabel = GTK_LABEL( help );
	gtk_box_append( GTK_BOX( bar ), help );

	GtkWidget *coord = gtk_label_new( "x 0  y 0" );
	st->coordLabel = GTK_LABEL( coord );
	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( bar ), coord );

	GtkWidget *snap = gtk_label_new( "Snap: On  Grid: 64" );
	st->snapLabel = GTK_LABEL( snap );
	gtk_box_append( GTK_BOX( bar ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( bar ), snap );

	return bar;
}

GtkWidget *MakeViewportGrid( AppState *st )
{
	// Nested paned so the four views are resizable like Hammer's splitters.
	GtkWidget *topPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child(
	    GTK_PANED( topPane ), MakeViewport( st, 0, hammergtk::ViewMode::Perspective, "camera" ) );
	gtk_paned_set_end_child(
	    GTK_PANED( topPane ), MakeViewport( st, 1, hammergtk::ViewMode::Top, "top (x/y)" ) );
	gtk_paned_set_resize_start_child( GTK_PANED( topPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( topPane ), TRUE );

	GtkWidget *bottomPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child(
	    GTK_PANED( bottomPane ), MakeViewport( st, 2, hammergtk::ViewMode::Front, "front (x/z)" ) );
	gtk_paned_set_end_child(
	    GTK_PANED( bottomPane ), MakeViewport( st, 3, hammergtk::ViewMode::Side, "side (y/z)" ) );
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

void OnActivate( GtkApplication *app, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->application = app;

	GtkWidget *window = gtk_application_window_new( app );
	st->window = window;
	gtk_window_set_default_size( GTK_WINDOW( window ), 1280, 820 );
	gtk_window_set_title( GTK_WINDOW( window ), "Hammer" );

	GtkWidget *root = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );

	// Menu bar.
	GMenu *menuModel = MakeMenuModel();
	GtkWidget *menubar = gtk_popover_menu_bar_new_from_model( G_MENU_MODEL( menuModel ) );
	g_object_unref( menuModel );
	gtk_box_append( GTK_BOX( root ), menubar );

	// Toolbar.
	gtk_box_append( GTK_BOX( root ), MakeToolbar( st ) );
	gtk_box_append( GTK_BOX( root ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );

	// Body: tool palette | viewports | object bar.
	GtkWidget *body = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_set_vexpand( body, TRUE );
	gtk_box_append( GTK_BOX( body ), MakeToolPalette() );
	gtk_box_append( GTK_BOX( body ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( body ), MakeViewportGrid( st ) );
	gtk_box_append( GTK_BOX( body ), gtk_separator_new( GTK_ORIENTATION_VERTICAL ) );
	gtk_box_append( GTK_BOX( body ), MakeObjectBar( st ) );
	gtk_box_append( GTK_BOX( root ), body );

	// Status bar.
	gtk_box_append( GTK_BOX( root ), gtk_separator_new( GTK_ORIENTATION_HORIZONTAL ) );
	gtk_box_append( GTK_BOX( root ), MakeStatusBar( st ) );

	gtk_window_set_child( GTK_WINDOW( window ), root );

	if ( !st->openOnStart.empty() )
	{
		LoadPath( st, st->openOnStart );
	}

	gtk_window_present( GTK_WINDOW( window ) );
}

void AddActions( GtkApplication *app, AppState *st )
{
	const GActionEntry entries[] = {
	    { "open", ActionOpen, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "quit", ActionQuit, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "reset-views", ActionResetViews, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	};
	g_action_map_add_action_entries( G_ACTION_MAP( app ), entries, G_N_ELEMENTS( entries ), st );

	const char *openAccel[] = { "<Control>o", nullptr };
	gtk_application_set_accels_for_action( app, "app.open", openAccel );
	const char *quitAccel[] = { "<Control>q", nullptr };
	gtk_application_set_accels_for_action( app, "app.quit", quitAccel );
}

int RunApp( AppState *st, int, char **argv )
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

int main( int argc, char **argv )
{
	std::string screenshotOut;
	std::string screenshotIn;
	std::string quadOut;
	std::string quadIn;
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
			std::printf(
			    "Usage: hammer_gtk [--open MAP.vmf]\n"
			    "       hammer_gtk --screenshot OUT.ppm MAP.vmf [--width W --height H]\n" );
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

	AppState st;
	st.openOnStart = openPath;
	return RunApp( &st, argc, argv );
}
