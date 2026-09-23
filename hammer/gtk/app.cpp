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

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/app/editor_controller.h"
#include "hammer/app/save_orchestrator.h"
#include "hammer/formats/material_catalog.h"
#include "hammer/formats/search_path_assets.h"
#include "hammer/formats/vpk_archive.h"
#ifdef HAMMER_KTX_PREVIEW
#include "hammer/adapters/source/ktx2_preview.h"
#endif

#include "renderer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

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

	// --- 3D free-fly / mouse-look navigation (Perspective view only) ---------
	bool mouseLook = false;    // Z toggled free-look: motion rotates the camera
	bool haveLookPrev = false; // whether lookPrev* holds a valid last position
	double lookPrevX = 0.0;    // last pointer pos while in mouse-look
	double lookPrevY = 0.0;    //   (widget coords), for the rotation delta
	bool keyForward = false;   // WASD / QE / RF fly keys currently held
	bool keyBack = false;
	bool keyLeft = false;
	bool keyRight = false;
	bool keyUp = false;
	bool keyDown = false;
	bool keyFast = false;         // Shift held: fly faster
	guint flyTick = 0;            // frame-clock tick source id (fly integration)
	gint64 flyPrevUs = 0;         // last tick time for dt, 0 = uninitialised
	double dragDist = 0.0;        // accumulated |drag| this press, to detect a click
	bool dragIsPan = false;       // this left-drag is a space+drag pan (2D), not a tool
	GtkWidget *caption = nullptr; // OSD label, updated when Tab cycles the 2D view
};

struct AppState
{
	hammer::app::EditorController controller;
	std::array<Viewport, 4> viewports;
	std::string currentPath;
	std::string openOnStart;
	std::string mountOnStart; // comma-separated _dir.vpk paths to mount at launch

	// Mounted game assets and the material catalog resolved over them. The catalog
	// borrows the search path, which borrows the archives; declaration order here
	// keeps them alive together and destroyed in the correct (reverse) order.
	hammer::adapters::platform::DiskByteStore byteStore;
	std::vector<std::unique_ptr<hammer::formats::VpkArchive>> archives;
	hammer::formats::SearchPathAssets assets;
	std::unique_ptr<hammer::formats::MaterialCatalog> catalog;

	GtkApplication *application = nullptr;
	GtkWidget *window = nullptr;
	GtkLabel *helpLabel = nullptr;
	GtkLabel *coordLabel = nullptr;
	GtkLabel *snapLabel = nullptr;
	GtkLabel *objectsCount = nullptr;
	GtkToggleButton *selectBtn = nullptr;
	GtkToggleButton *blockBtn = nullptr;
	bool suppressToolSignal = false;
	bool spaceHeld = false; // Space held: left-drag pans a 2D view (MFC nav idiom)

	// Object-bar texture preview (current material).
	GtkWidget *texSwatch = nullptr;
	GtkLabel *texNameLabel = nullptr;
	// The Texture Application window (Shift+A), when one is open. Only one exists at
	// a time; the swatch/name mirror the object-bar preview so both stay in sync.
	// All three are cleared when the window is destroyed.
	GtkWidget *texToolWindow = nullptr;
	GtkWidget *texToolSwatch = nullptr;
	GtkLabel *texToolName = nullptr;
	std::string currentMaterial;
	std::vector<std::uint8_t> swatchRgba; // RGBA preview shown by DrawTextureSwatch
	int swatchW = 0;
	int swatchH = 0;
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
		char sel[48] = { 0 };
		const std::size_t n = st->controller.SelectionCount();
		if ( n > 0 )
		{
			std::snprintf( sel, sizeof( sel ), "  ·  %zu selected", n );
		}
		std::snprintf(
		    buf, sizeof( buf ), "%zu brush(es)%s", st->controller.Brushes().size(), sel );
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
	const std::vector<int> selectedIds = st->controller.Selections();
	for ( Viewport &vp : st->viewports )
	{
		if ( vp.glReady && vp.area )
		{
			gtk_gl_area_make_current( vp.area );
			if ( gtk_gl_area_get_error( vp.area ) == nullptr )
			{
				vp.renderer.SetMaterialCatalog( st->catalog.get() );
				vp.renderer.SetHighlight( kNoHighlight );
				vp.renderer.SetHighlights( selectedIds ); // highlight the whole selection
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
// Game-asset mounting and material preview.
// ---------------------------------------------------------------------------

// Nearest-neighbour downscale of an RGBA image into a swatch-sized preview stored
// on the AppState, so the (Cairo) object-bar swatch can paint the real texture.
void SetCurrentMaterial( AppState *st, const std::string &name )
{
	st->currentMaterial = name;
	// The current material IS the active material the controller applies (Material
	// tool, Apply-to-Selection) and gives new Block brushes. One owner, one update.
	st->controller.SetActiveMaterial( name );

	std::string label = name;
	st->swatchRgba.clear();
	st->swatchW = 0;
	st->swatchH = 0;

	if ( st->catalog )
	{
		if ( const hammer::formats::VtfImage *img = st->catalog->BaseTextureImage( name ) )
		{
			const int dstW = 128;
			const int dstH = 128;
			st->swatchRgba.assign( static_cast<std::size_t>( dstW ) * dstH * 4, 0 );
			for ( int y = 0; y < dstH; ++y )
			{
				const int sy = img->height > 0 ? ( y * img->height ) / dstH : 0;
				for ( int x = 0; x < dstW; ++x )
				{
					const int sx = img->width > 0 ? ( x * img->width ) / dstW : 0;
					const std::size_t s = ( static_cast<std::size_t>( sy ) * img->width + sx ) * 4;
					const std::size_t d = ( static_cast<std::size_t>( y ) * dstW + x ) * 4;
					if ( s + 3 < img->rgba.size() )
					{
						st->swatchRgba[d + 0] = img->rgba[s + 0];
						st->swatchRgba[d + 1] = img->rgba[s + 1];
						st->swatchRgba[d + 2] = img->rgba[s + 2];
						st->swatchRgba[d + 3] = 255;
					}
				}
			}
			st->swatchW = dstW;
			st->swatchH = dstH;
			char dims[64];
			std::snprintf( dims, sizeof( dims ), "  %dx%d", img->width, img->height );
			label += dims;
		}
		else
		{
			label += "  (no texture)";
		}
	}

	if ( st->texNameLabel )
	{
		gtk_label_set_text( st->texNameLabel, label.c_str() );
	}
	if ( st->texSwatch )
	{
		gtk_widget_queue_draw( st->texSwatch );
	}
	// Mirror into the Texture Application window's current-material preview, if open.
	if ( st->texToolName )
	{
		gtk_label_set_text( st->texToolName, label.c_str() );
	}
	if ( st->texToolSwatch )
	{
		gtk_widget_queue_draw( st->texToolSwatch );
	}
}

// Mounts a comma-separated list of _dir.vpk paths into the asset search path,
// (re)builds the material catalog, and re-textures every viewport. Returns the
// number of materials the catalog can enumerate (0 on total failure).
std::size_t MountAssets( AppState *st, const std::string &vpkList )
{
	st->catalog.reset();
	st->archives.clear();
	st->assets = hammer::formats::SearchPathAssets();

	std::size_t begin = 0;
	while ( begin <= vpkList.size() )
	{
		const std::size_t comma = vpkList.find( ',', begin );
		const std::string path =
		    vpkList.substr( begin, comma == std::string::npos ? std::string::npos : comma - begin );
		if ( !path.empty() )
		{
			std::string err;
			auto vpk = hammer::formats::VpkArchive::Open( st->byteStore, path, err );
			if ( vpk )
			{
				st->assets.AddProvider( vpk.get() );
				st->archives.push_back( std::move( vpk ) );
			}
			else
			{
				SetHelp( st, "Mount failed: " + path + " (" + err + ")" );
			}
		}
		if ( comma == std::string::npos )
		{
			break;
		}
		begin = comma + 1;
	}

	if ( st->assets.ProviderCount() == 0 )
	{
		return 0;
	}

#ifdef HAMMER_KTX_PREVIEW
	st->catalog = std::make_unique<hammer::formats::MaterialCatalog>(
	    st->assets, &hammer::adapters::source::DecodeKtx2Preview );
#else
	st->catalog = std::make_unique<hammer::formats::MaterialCatalog>( st->assets );
#endif
	const std::size_t count = st->catalog->MaterialNames().size();

	// Show the first resolvable material as a proof-of-life preview.
	for ( const std::string &name : st->catalog->MaterialNames() )
	{
		if ( st->catalog->BaseTextureImage( name ) )
		{
			SetCurrentMaterial( st, name );
			break;
		}
	}

	RefreshScene( st, false );
	char msg[128];
	std::snprintf( msg, sizeof( msg ), "Mounted %zu archive(s); %zu materials available",
	    st->archives.size(), count );
	SetHelp( st, msg );
	return count;
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
	vp->renderer.SetMaterialCatalog( vp->app->catalog.get() );
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

// ---- 3D free-fly / mouse-look (classic Hammer camera navigation) -----------

// Degrees of camera rotation per pixel of mouse motion in look mode (MFC uses
// 0.4). World units per second the camera flies with WASD, and the Shift boost.
constexpr double kLookSpeed = 0.4;
constexpr double kFlyBaseSpeed = 640.0;
constexpr double kFlyFastSpeed = 1800.0;

void SetCursorHidden( Viewport *vp, bool hidden )
{
	if ( hidden )
	{
		GdkCursor *none = gdk_cursor_new_from_name( "none", nullptr );
		gtk_widget_set_cursor( GTK_WIDGET( vp->area ), none );
		if ( none )
		{
			g_object_unref( none );
		}
	}
	else
	{
		gtk_widget_set_cursor( GTK_WIDGET( vp->area ), nullptr );
	}
}

// Per-frame fly integration: while any movement key is held, translates the 3D
// camera by speed*dt along the view basis. Runs only while keys are down (started
// and stopped by UpdateFlyTick), so the frame clock idles when nothing moves.
gboolean OnFlyTick( GtkWidget *, GdkFrameClock *clock, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const gint64 now = gdk_frame_clock_get_frame_time( clock );
	if ( vp->flyPrevUs == 0 )
	{
		vp->flyPrevUs = now;
		return G_SOURCE_CONTINUE;
	}
	double dt = static_cast<double>( now - vp->flyPrevUs ) / 1.0e6;
	vp->flyPrevUs = now;
	if ( dt > 0.25 )
	{
		dt = 0.25; // clamp long stalls, like MFC's ProcessInput
	}
	if ( dt <= 0.0 )
	{
		return G_SOURCE_CONTINUE;
	}

	const double step = ( vp->keyFast ? kFlyFastSpeed : kFlyBaseSpeed ) * dt;
	double fwd = 0.0;
	double strafe = 0.0;
	double rise = 0.0;
	if ( vp->keyForward )
	{
		fwd += step;
	}
	if ( vp->keyBack )
	{
		fwd -= step;
	}
	if ( vp->keyRight )
	{
		strafe += step;
	}
	if ( vp->keyLeft )
	{
		strafe -= step;
	}
	if ( vp->keyUp )
	{
		rise += step;
	}
	if ( vp->keyDown )
	{
		rise -= step;
	}
	if ( fwd != 0.0 || strafe != 0.0 || rise != 0.0 )
	{
		vp->renderer.FlyMove(
		    static_cast<float>( fwd ), static_cast<float>( strafe ), static_cast<float>( rise ) );
		gtk_gl_area_queue_render( vp->area );
	}
	return G_SOURCE_CONTINUE;
}

// Starts/stops the frame-clock tick so it runs exactly while a fly key is held.
void UpdateFlyTick( Viewport *vp )
{
	const bool wantTick =
	    vp->keyForward || vp->keyBack || vp->keyLeft || vp->keyRight || vp->keyUp || vp->keyDown;
	if ( wantTick && vp->flyTick == 0 )
	{
		vp->flyPrevUs = 0;
		vp->flyTick =
		    gtk_widget_add_tick_callback( GTK_WIDGET( vp->area ), OnFlyTick, vp, nullptr );
	}
	else if ( !wantTick && vp->flyTick != 0 )
	{
		gtk_widget_remove_tick_callback( GTK_WIDGET( vp->area ), vp->flyTick );
		vp->flyTick = 0;
	}
}

void SetMouseLook( Viewport *vp, bool on )
{
	if ( vp->mode != hammergtk::ViewMode::Perspective || vp->mouseLook == on )
	{
		return;
	}
	vp->mouseLook = on;
	vp->haveLookPrev = false;
	SetCursorHidden( vp, on );
	SetHelp( vp->app, on ? "Mouse-look: move to look, WASD/QE to fly, Z to release"
	                     : "Camera view — click to select, Z for mouse-look" );
}

void ClearFlyKeys( Viewport *vp )
{
	vp->keyForward = false;
	vp->keyBack = false;
	vp->keyLeft = false;
	vp->keyRight = false;
	vp->keyUp = false;
	vp->keyDown = false;
	UpdateFlyTick( vp );
}

void OnMotion( GtkEventControllerMotion *, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->mouseLook && vp->mode == hammergtk::ViewMode::Perspective )
	{
		if ( vp->haveLookPrev )
		{
			// Mouse right -> turn right; mouse up -> look up. (Signs chosen to match
			// a first-person feel; flip kLookSpeed's use here to invert an axis.)
			const double dx = x - vp->lookPrevX;
			const double dy = y - vp->lookPrevY;
			vp->renderer.FlyLook(
			    static_cast<float>( -dx * kLookSpeed ), static_cast<float>( dy * kLookSpeed ) );
			gtk_gl_area_queue_render( vp->area );
		}
		vp->lookPrevX = x;
		vp->lookPrevY = y;
		vp->haveLookPrev = true;
	}
	vp->cursorX = x;
	vp->cursorY = y;
	UpdateCoords( vp );
}

// Tool drag (left button): 2D views drive the active editing tool; the camera
// view orbits.
void OnToolBegin( GtkGestureDrag *gesture, double startX, double startY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->startX = startX;
	vp->startY = startY;
	vp->orbitPrevX = 0.0;
	vp->orbitPrevY = 0.0;
	vp->dragDist = 0.0;
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}
	// Space + left-drag pans a 2D view (MFC's navigation idiom), instead of editing.
	vp->dragIsPan = vp->app->spaceHeld;
	if ( vp->dragIsPan )
	{
		return;
	}
	// Ctrl-click adds/removes the brush from a multi-selection (MFC additive select).
	const GdkModifierType mods =
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( gesture ) );
	const bool additive = ( mods & GDK_CONTROL_MASK ) != 0;
	double u = 0.0;
	double v = 0.0;
	WidgetToWorld( vp, startX, startY, u, v );
	vp->app->controller.PointerDown( vp->vid, u, v, additive );
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
		vp->dragDist += std::abs( dx ) + std::abs( dy );
		vp->renderer.DragBy( static_cast<float>( dx ), static_cast<float>( dy ) );
		gtk_gl_area_queue_render( vp->area );
		return;
	}
	if ( vp->dragIsPan )
	{
		// Space + left-drag pan: incremental like the middle-button pan.
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

void OnToolEnd( GtkGestureDrag *gesture, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		// A left click that barely moved (not an orbit-drag) is a 3D ray pick, so
		// the camera view can select brushes like MFC's selection tool does.
		if ( vp->dragDist < 4.0 )
		{
			const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
			const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
			const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
			const GdkModifierType mods =
			    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( gesture ) );
			const bool additive = ( mods & GDK_CONTROL_MASK ) != 0;
			float o[3] = { 0.0f, 0.0f, 0.0f };
			float d[3] = { 0.0f, 0.0f, 0.0f };
			if ( vp->renderer.PixelToRay( static_cast<float>( vp->startX * scale ),
			         static_cast<float>( vp->startY * scale ), w, h, o, d ) )
			{
				vp->app->controller.PickByRay( hammer::geometry::Vec3d( o[0], o[1], o[2] ),
				    hammer::geometry::Vec3d( d[0], d[1], d[2] ), additive );
				RefreshScene( vp->app, false );
			}
		}
		return;
	}
	if ( vp->dragIsPan )
	{
		vp->dragIsPan = false;
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

// Defined below (with the Texture Application window); Shift+A opens it.
void OpenTextureWindow( AppState *st );

gboolean OnKeyPressed(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	switch ( keyval )
	{
	// Shift+A yields the uppercase keysym; this is legacy Hammer's texture-
	// application tool shortcut. Open the Texture Application window.
	case GDK_KEY_A:
		OpenTextureWindow( st );
		return TRUE;
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

// The two world axes a 2D view edits (its horizontal/vertical screen axes).
void ViewAxes2D( hammergtk::ViewMode mode, int &uAxis, int &vAxis )
{
	switch ( mode )
	{
	case hammergtk::ViewMode::Front: // X / Z
		uAxis = 0;
		vAxis = 2;
		break;
	case hammergtk::ViewMode::Side: // Y / Z
		uAxis = 1;
		vAxis = 2;
		break;
	default: // Top: X / Y
		uAxis = 0;
		vAxis = 1;
		break;
	}
}

// Tab cycles a 2D view's orientation Top -> Front -> Side -> Top, like MFC's
// draw-type cycle. Renderer mode and controller ViewId (edit axes) move together.
void CycleView2D( Viewport *vp )
{
	struct ViewDef
	{
		hammergtk::ViewMode mode;
		hammer::app::ViewId vid;
		const char *label;
	};
	static const ViewDef order[] = {
	    { hammergtk::ViewMode::Top, hammer::app::ViewId::Top, "top (x/y)" },
	    { hammergtk::ViewMode::Front, hammer::app::ViewId::Front, "front (x/z)" },
	    { hammergtk::ViewMode::Side, hammer::app::ViewId::Side, "side (y/z)" },
	};
	int cur = 0;
	for ( int i = 0; i < 3; ++i )
	{
		if ( order[i].mode == vp->mode )
		{
			cur = i;
		}
	}
	const ViewDef &next = order[( cur + 1 ) % 3];
	vp->mode = next.mode;
	vp->vid = next.vid;
	vp->label = next.label;
	vp->renderer.SetViewMode( next.mode );
	vp->renderer.FrameScene(); // reframe for the new axes (pure math, no GL)
	if ( vp->caption )
	{
		gtk_label_set_text( GTK_LABEL( vp->caption ), next.label );
	}
	gtk_gl_area_queue_render( vp->area );
}

// Arrow-key nudge of the selected brush by one grid step along the 2D view axes
// (screen up = +v). One undo unit per nudge, via the controller (single authority).
void Nudge2D( Viewport *vp, int du, int dv )
{
	if ( !vp->app->controller.Selection() )
	{
		return;
	}
	int uAxis = 0;
	int vAxis = 1;
	ViewAxes2D( vp->mode, uAxis, vAxis );
	const double grid = static_cast<double>( vp->app->controller.GridSize() );
	double d[3] = { 0.0, 0.0, 0.0 };
	d[uAxis] += du * grid;
	d[vAxis] += dv * grid;
	if ( vp->app->controller.MoveSelectionBy( d[0], d[1], d[2] ) )
	{
		RefreshScene( vp->app, false );
	}
}

// +/- keyboard zoom for a 2D view, anchored at the cursor like the wheel zoom.
void Zoom2DKey( Viewport *vp, bool zoomIn )
{
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( vp->area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( vp->area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( vp->area ) ) * scale;
	const float px = static_cast<float>( vp->cursorX * scale );
	const float py = static_cast<float>( vp->cursorY * scale );
	vp->renderer.ZoomAtPixel( zoomIn ? 1.2f : ( 1.0f / 1.2f ), px, py, w, h );
	UpdateCoords( vp );
	gtk_gl_area_queue_render( vp->area );
}

// Per-viewport key handling. The 3D camera view: Z toggles mouse-look and WASD/QE
// fly (held state integrated by OnFlyTick); the 2D views: Tab cycles orientation,
// arrows nudge the selection, +/- zoom. Space (either) arms space+drag panning.
// Consuming a view's keys matches MFC, where the focused view captures them;
// unhandled keys fall through to the window shortcuts (B/S tools, Delete, etc.).
gboolean OnViewKeyPressed(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( keyval == GDK_KEY_space )
	{
		vp->app->spaceHeld = true; // used by the left-drag gesture; don't consume
		return FALSE;
	}
	if ( vp->mode != hammergtk::ViewMode::Perspective )
	{
		// Number keys 1..9 jump to preset zoom levels (0.0625 px/unit doubling per
		// step); 0 frames the whole map -- MFC's numeric zoom shortcuts.
		if ( keyval >= GDK_KEY_1 && keyval <= GDK_KEY_9 )
		{
			vp->renderer.SetOrthoScale(
			    0.0625f * std::pow( 2.0f, static_cast<float>( keyval - GDK_KEY_1 ) ) );
			UpdateCoords( vp );
			gtk_gl_area_queue_render( vp->area );
			return TRUE;
		}
		if ( keyval == GDK_KEY_0 )
		{
			vp->renderer.FrameScene();
			UpdateCoords( vp );
			gtk_gl_area_queue_render( vp->area );
			return TRUE;
		}
		switch ( keyval )
		{
		case GDK_KEY_Tab:
		case GDK_KEY_ISO_Left_Tab:
			CycleView2D( vp );
			return TRUE;
		case GDK_KEY_Up:
			Nudge2D( vp, 0, 1 );
			return TRUE;
		case GDK_KEY_Down:
			Nudge2D( vp, 0, -1 );
			return TRUE;
		case GDK_KEY_Left:
			Nudge2D( vp, -1, 0 );
			return TRUE;
		case GDK_KEY_Right:
			Nudge2D( vp, 1, 0 );
			return TRUE;
		case GDK_KEY_plus:
		case GDK_KEY_equal:
		case GDK_KEY_KP_Add:
			Zoom2DKey( vp, true );
			return TRUE;
		case GDK_KEY_minus:
		case GDK_KEY_KP_Subtract:
			Zoom2DKey( vp, false );
			return TRUE;
		default:
			return FALSE;
		}
	}
	vp->keyFast = ( state & GDK_SHIFT_MASK ) != 0;
	switch ( keyval )
	{
	case GDK_KEY_z:
	case GDK_KEY_Z:
		SetMouseLook( vp, !vp->mouseLook );
		return TRUE;
	case GDK_KEY_w:
	case GDK_KEY_W:
		vp->keyForward = true;
		UpdateFlyTick( vp );
		return TRUE;
	case GDK_KEY_s:
	case GDK_KEY_S:
		vp->keyBack = true;
		UpdateFlyTick( vp );
		return TRUE;
	case GDK_KEY_a:
	case GDK_KEY_A:
		vp->keyLeft = true;
		UpdateFlyTick( vp );
		return TRUE;
	case GDK_KEY_d:
	case GDK_KEY_D:
		vp->keyRight = true;
		UpdateFlyTick( vp );
		return TRUE;
	case GDK_KEY_e:
	case GDK_KEY_E:
		vp->keyUp = true;
		UpdateFlyTick( vp );
		return TRUE;
	case GDK_KEY_q:
	case GDK_KEY_Q:
		vp->keyDown = true;
		UpdateFlyTick( vp );
		return TRUE;
	default:
		return FALSE;
	}
}

void OnViewKeyReleased(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( keyval == GDK_KEY_space )
	{
		vp->app->spaceHeld = false;
		return;
	}
	if ( vp->mode != hammergtk::ViewMode::Perspective )
	{
		return;
	}
	vp->keyFast = ( state & GDK_SHIFT_MASK ) != 0;
	// Handle both cases: a held key's release can arrive shifted (w vs W).
	switch ( keyval )
	{
	case GDK_KEY_w:
	case GDK_KEY_W:
		vp->keyForward = false;
		break;
	case GDK_KEY_s:
	case GDK_KEY_S:
		vp->keyBack = false;
		break;
	case GDK_KEY_a:
	case GDK_KEY_A:
		vp->keyLeft = false;
		break;
	case GDK_KEY_d:
	case GDK_KEY_D:
		vp->keyRight = false;
		break;
	case GDK_KEY_e:
	case GDK_KEY_E:
		vp->keyUp = false;
		break;
	case GDK_KEY_q:
	case GDK_KEY_Q:
		vp->keyDown = false;
		break;
	default:
		return;
	}
	UpdateFlyTick( vp );
}

// Give a hovered viewport keyboard focus so its keys reach OnViewKeyPressed
// (MFC makes the view active and grabs focus on mouse-move). On leave, drop any
// held fly keys so the camera doesn't keep drifting once the pointer is away.
void OnViewEnter( GtkEventControllerMotion *, double, double, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	gtk_widget_grab_focus( GTK_WIDGET( vp->area ) );
}

void OnViewLeave( GtkEventControllerMotion *, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	ClearFlyKeys( vp );
	vp->haveLookPrev = false;
}

void OnPopoverClosed( GtkPopover *popover, gpointer )
{
	gtk_widget_unparent( GTK_WIDGET( popover ) );
}

// Right-click context menu. Faithful to MFC: the 2D views get a popup (a
// selection menu when a brush is selected, otherwise the default view menu); the
// 3D camera view has no right-click menu in MFC, so it gets none here either.
void OnContextClick( GtkGestureClick *, int, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->mode == hammergtk::ViewMode::Perspective )
	{
		return;
	}

	GMenu *model = g_menu_new();
	if ( vp->app->controller.Selection() )
	{
		g_menu_append( model, "Delete", "app.delete" );
		GMenu *tools = g_menu_new();
		g_menu_append( tools, "Selection Tool", "app.tool-select" );
		g_menu_append( tools, "Block Tool", "app.tool-block" );
		g_menu_append_section( model, nullptr, G_MENU_MODEL( tools ) );
		g_object_unref( tools );
	}
	else
	{
		g_menu_append( model, "Block Tool", "app.tool-block" );
		g_menu_append( model, "Selection Tool", "app.tool-select" );
		GMenu *view = g_menu_new();
		g_menu_append( view, "Reset Views", "app.reset-views" );
		g_menu_append_section( model, nullptr, G_MENU_MODEL( view ) );
		g_object_unref( view );
	}

	GtkWidget *popover = gtk_popover_menu_new_from_model( G_MENU_MODEL( model ) );
	g_object_unref( model );
	gtk_widget_set_parent( popover, GTK_WIDGET( vp->area ) );
	gtk_popover_set_has_arrow( GTK_POPOVER( popover ), FALSE );
	const GdkRectangle rect = { static_cast<int>( x ), static_cast<int>( y ), 1, 1 };
	gtk_popover_set_pointing_to( GTK_POPOVER( popover ), &rect );
	gtk_popover_set_position( GTK_POPOVER( popover ), GTK_POS_BOTTOM );
	g_signal_connect( popover, "closed", G_CALLBACK( OnPopoverClosed ), nullptr );
	gtk_popover_popup( GTK_POPOVER( popover ) );
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
	// Focusable so a hovered view receives key events (camera fly / mouse-look).
	gtk_widget_set_focusable( glarea, TRUE );
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
	g_signal_connect( motion, "enter", G_CALLBACK( OnViewEnter ), &vp );
	g_signal_connect( motion, "leave", G_CALLBACK( OnViewLeave ), &vp );
	gtk_widget_add_controller( glarea, motion );

	// Per-view keys (Z mouse-look, WASD/QE fly) for the 3D camera view.
	GtkEventController *viewKeys = gtk_event_controller_key_new();
	g_signal_connect( viewKeys, "key-pressed", G_CALLBACK( OnViewKeyPressed ), &vp );
	g_signal_connect( viewKeys, "key-released", G_CALLBACK( OnViewKeyReleased ), &vp );
	gtk_widget_add_controller( glarea, viewKeys );

	// Right button: context menu (2D views; the 3D view has none, like MFC).
	GtkGesture *context = gtk_gesture_click_new();
	gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( context ), GDK_BUTTON_SECONDARY );
	g_signal_connect( context, "pressed", G_CALLBACK( OnContextClick ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( context ) );

	GtkWidget *overlay = gtk_overlay_new();
	gtk_overlay_set_child( GTK_OVERLAY( overlay ), glarea );
	GtkWidget *caption = gtk_label_new( label );
	vp.caption = caption;
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

void DrawTextureSwatch( GtkDrawingArea *, cairo_t *cr, int w, int h, gpointer user_data )
{
	// When a material is mounted and previewed, paint its real decoded texture,
	// scaled to fill the swatch. Otherwise fall back to the placeholder pattern.
	AppState *st = static_cast<AppState *>( user_data );
	if ( st && !st->swatchRgba.empty() && st->swatchW > 0 && st->swatchH > 0 )
	{
		const int sw = st->swatchW;
		const int sh = st->swatchH;
		const int stride = cairo_format_stride_for_width( CAIRO_FORMAT_RGB24, sw );
		std::vector<unsigned char> buf( static_cast<std::size_t>( stride ) * sh, 0 );
		for ( int y = 0; y < sh; ++y )
		{
			auto *row = reinterpret_cast<std::uint32_t *>(
			    buf.data() + static_cast<std::size_t>( y ) * stride );
			for ( int x = 0; x < sw; ++x )
			{
				const std::size_t s = ( static_cast<std::size_t>( y ) * sw + x ) * 4;
				const std::uint32_t r = st->swatchRgba[s + 0];
				const std::uint32_t g = st->swatchRgba[s + 1];
				const std::uint32_t b = st->swatchRgba[s + 2];
				row[x] = ( r << 16 ) | ( g << 8 ) | b; // CAIRO_FORMAT_RGB24 is 0x00RRGGBB
			}
		}
		cairo_surface_t *surface =
		    cairo_image_surface_create_for_data( buf.data(), CAIRO_FORMAT_RGB24, sw, sh, stride );
		cairo_save( cr );
		cairo_scale( cr, static_cast<double>( w ) / sw, static_cast<double>( h ) / sh );
		cairo_set_source_surface( cr, surface, 0, 0 );
		cairo_pattern_set_filter( cairo_get_source( cr ), CAIRO_FILTER_NEAREST );
		cairo_paint( cr );
		cairo_restore( cr );
		cairo_surface_destroy( surface );
		return;
	}

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
	gtk_drawing_area_set_draw_func( GTK_DRAWING_AREA( swatch ), DrawTextureSwatch, st, nullptr );
	gtk_box_append( GTK_BOX( bar ), swatch );
	st->texSwatch = swatch;
	GtkWidget *texName = gtk_label_new( "no assets mounted" );
	gtk_widget_add_css_class( texName, "caption" );
	gtk_label_set_wrap( GTK_LABEL( texName ), TRUE );
	gtk_box_append( GTK_BOX( bar ), texName );
	st->texNameLabel = GTK_LABEL( texName );

	GtkWidget *texBtns = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_widget_add_css_class( texBtns, "linked" );
	GtkWidget *browseBtn = gtk_button_new_with_label( "Browse…" );
	gtk_widget_set_hexpand( browseBtn, TRUE );
	gtk_actionable_set_action_name( GTK_ACTIONABLE( browseBtn ), "app.browse-materials" );
	gtk_box_append( GTK_BOX( texBtns ), browseBtn );
	GtkWidget *mountBtn = gtk_button_new_with_label( "Mount…" );
	gtk_widget_set_hexpand( mountBtn, TRUE );
	gtk_actionable_set_action_name( GTK_ACTIONABLE( mountBtn ), "app.mount-assets" );
	gtk_box_append( GTK_BOX( texBtns ), mountBtn );
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
	GMenu *fileAssets = g_menu_new();
	g_menu_append( fileAssets, "Mount Game Assets…", "app.mount-assets" );
	g_menu_append( fileAssets, "Texture Application (Shift+A)…", "app.browse-materials" );
	g_menu_append_section( file, nullptr, G_MENU_MODEL( fileAssets ) );
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
	g_menu_append( tools, "Texture Application (Shift+A)…", "app.browse-materials" );
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

	if ( !st->mountOnStart.empty() )
	{
		MountAssets( st, st->mountOnStart );
	}

	if ( !st->openOnStart.empty() )
	{
		DoOpen( st, st->openOnStart );
	}

	gtk_window_present( GTK_WINDOW( window ) );
}

// ---------------------------------------------------------------------------
// Asset mounting (folder chooser -> discover *_dir.vpk) and material browser.
// ---------------------------------------------------------------------------

// Joins every "*_dir.vpk" directly under 'dir' into a comma-separated mount list,
// sorted so the mount order is deterministic.
std::string DiscoverVpks( const std::string &dir )
{
	std::vector<std::string> found;
	std::error_code ec;
	for ( const auto &entry : std::filesystem::directory_iterator( dir, ec ) )
	{
		if ( ec )
		{
			break;
		}
		const std::string name = entry.path().filename().string();
		if ( name.size() >= 8 && name.compare( name.size() - 8, 8, "_dir.vpk" ) == 0 )
		{
			found.push_back( entry.path().string() );
		}
	}
	std::sort( found.begin(), found.end() );
	std::string list;
	for ( const std::string &p : found )
	{
		if ( !list.empty() )
		{
			list += ",";
		}
		list += p;
	}
	return list;
}

void OnMountFolderChosen( GObject *source, GAsyncResult *res, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GError *err = nullptr;
	GFile *folder = gtk_file_dialog_select_folder_finish( GTK_FILE_DIALOG( source ), res, &err );
	if ( !folder )
	{
		if ( err )
		{
			g_error_free( err );
		}
		return;
	}
	char *path = g_file_get_path( folder );
	if ( path )
	{
		const std::string list = DiscoverVpks( path );
		if ( list.empty() )
		{
			SetHelp( st, std::string( "No *_dir.vpk found in " ) + path );
		}
		else
		{
			MountAssets( st, list );
		}
		g_free( path );
	}
	g_object_unref( folder );
}

void ActionMountAssets( GSimpleAction *, GVariant *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	GtkFileDialog *dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_title( dialog, "Choose a game directory (containing *_dir.vpk)" );
	gtk_file_dialog_select_folder(
	    dialog, GTK_WINDOW( st->window ), nullptr, OnMountFolderChosen, st );
	g_object_unref( dialog );
}

// A GdkTexture RGBA thumbnail (nearest-downscaled to 'size' x 'size') for a
// decoded base texture, or nullptr when there is none.
GdkTexture *MakeThumbnail( AppState *st, const std::string &name, int size )
{
	if ( !st->catalog )
	{
		return nullptr;
	}
	const hammer::formats::VtfImage *img = st->catalog->BaseTextureImage( name );
	if ( !img || img->width <= 0 || img->height <= 0 )
	{
		return nullptr;
	}
	std::vector<std::uint8_t> small( static_cast<std::size_t>( size ) * size * 4, 0 );
	for ( int y = 0; y < size; ++y )
	{
		const int sy = ( y * img->height ) / size;
		for ( int x = 0; x < size; ++x )
		{
			const int sx = ( x * img->width ) / size;
			const std::size_t s = ( static_cast<std::size_t>( sy ) * img->width + sx ) * 4;
			const std::size_t d = ( static_cast<std::size_t>( y ) * size + x ) * 4;
			if ( s + 3 < img->rgba.size() )
			{
				small[d + 0] = img->rgba[s + 0];
				small[d + 1] = img->rgba[s + 1];
				small[d + 2] = img->rgba[s + 2];
				small[d + 3] = 255;
			}
		}
	}
	GBytes *bytes = g_bytes_new( small.data(), small.size() );
	GdkTexture *tex = gdk_memory_texture_new(
	    size, size, GDK_MEMORY_R8G8B8A8, bytes, static_cast<gsize>( size ) * 4 );
	g_bytes_unref( bytes );
	return tex;
}

// ---------------------------------------------------------------------------
// Texture Application window (Shift+A) — the GTK counterpart of legacy Hammer's
// Face Edit Sheet "texture application tool" (hammer/faceeditsheet.cpp,
// faceedit_materialpage). It shows the active material, a filterable grid of the
// mounted materials, and applies the active material to the selected brush's
// faces. Legacy's per-face UV scale/shift/rotate/lightmap controls are omitted:
// this slice's face model (MapBrush::materials) stores only a material name, not
// texture coordinates, so those controls would have nothing to drive.
// ---------------------------------------------------------------------------

// Picking a material makes it the active material (object-bar + tool-window
// previews refresh via SetCurrentMaterial) and arms the Material tool, so the
// next click on a brush in a viewport retextures it — as the legacy tool does.
void OnTextureChosen( GtkFlowBox *, GtkFlowBoxChild *child, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	const char *name =
	    static_cast<const char *>( g_object_get_data( G_OBJECT( child ), "material" ) );
	if ( !name )
	{
		return;
	}
	SetCurrentMaterial( st, name );
	SetToolUi( st, hammer::app::Tool::Material );
	SetHelp( st, std::string( "Active material: " ) + name +
	                 "  ·  click a brush to apply, or use Apply to Selection" );
}

// Flowbox filter: keep a cell iff its material name contains the (lowercased)
// filter substring. The filter string is owned by the flowbox (see below).
gboolean TextureFilter( GtkFlowBoxChild *child, gpointer user_data )
{
	const std::string *filter = static_cast<const std::string *>( user_data );
	if ( !filter || filter->empty() )
	{
		return TRUE;
	}
	const char *name =
	    static_cast<const char *>( g_object_get_data( G_OBJECT( child ), "material" ) );
	if ( !name )
	{
		return FALSE;
	}
	std::string lower( name );
	std::transform( lower.begin(), lower.end(), lower.begin(),
	    []( unsigned char c )
	    {
		    return static_cast<char>( std::tolower( c ) );
	    } );
	return lower.find( *filter ) != std::string::npos ? TRUE : FALSE;
}

void OnTextureSearch( GtkSearchEntry *entry, gpointer user_data )
{
	GtkFlowBox *flow = GTK_FLOW_BOX( user_data );
	std::string *filter =
	    static_cast<std::string *>( g_object_get_data( G_OBJECT( flow ), "filter" ) );
	if ( !filter )
	{
		return;
	}
	const char *text = gtk_editable_get_text( GTK_EDITABLE( entry ) );
	filter->assign( text ? text : "" );
	std::transform( filter->begin(), filter->end(), filter->begin(),
	    []( unsigned char c )
	    {
		    return static_cast<char>( std::tolower( c ) );
	    } );
	gtk_flow_box_invalidate_filter( flow );
}

// Apply the active material to every face of the selected brush and re-render.
void OnApplyToSelection( GtkButton *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( !st->controller.Selection() )
	{
		SetHelp( st, "Select a brush first, then Apply to Selection" );
		return;
	}
	if ( st->controller.ApplyActiveMaterialToSelection() )
	{
		RefreshScene( st, false );
		SetHelp( st, std::string( "Applied " ) + st->currentMaterial + " to the selected brush" );
	}
	else
	{
		SetHelp( st, "The selected brush already uses that material" );
	}
}

// The window borrows AppState's preview-widget slots while open; release them so
// SetCurrentMaterial does not touch destroyed widgets after it closes.
void OnTextureWindowDestroy( GtkWidget *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	st->texToolWindow = nullptr;
	st->texToolSwatch = nullptr;
	st->texToolName = nullptr;
}

void OpenTextureWindow( AppState *st )
{
	// Only one Texture Application window at a time: raise the existing one. (Two
	// would share the single preview-widget registration below, so closing either
	// would strand the other's live pointers.)
	if ( st->texToolWindow )
	{
		gtk_window_present( GTK_WINDOW( st->texToolWindow ) );
		return;
	}
	if ( !st->catalog )
	{
		SetHelp( st, "No assets mounted -- use Mount… (or File ▸ Mount Game Assets…) first" );
		return;
	}

	GtkWidget *win = gtk_window_new();
	gtk_window_set_title( GTK_WINDOW( win ), "Texture Application" );
	gtk_window_set_default_size( GTK_WINDOW( win ), 760, 620 );
	if ( st->window )
	{
		gtk_window_set_transient_for( GTK_WINDOW( win ), GTK_WINDOW( st->window ) );
	}

	GtkWidget *box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );
	gtk_widget_set_margin_start( box, 8 );
	gtk_widget_set_margin_end( box, 8 );
	gtk_widget_set_margin_top( box, 8 );
	gtk_widget_set_margin_bottom( box, 8 );

	// Header: current-material preview, its name, and Apply to Selection.
	GtkWidget *header = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
	GtkWidget *swatch = gtk_drawing_area_new();
	gtk_widget_set_size_request( swatch, 96, 96 );
	gtk_drawing_area_set_draw_func( GTK_DRAWING_AREA( swatch ), DrawTextureSwatch, st, nullptr );
	gtk_box_append( GTK_BOX( header ), swatch );

	GtkWidget *nameCol = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
	gtk_widget_set_valign( nameCol, GTK_ALIGN_CENTER );
	gtk_widget_set_hexpand( nameCol, TRUE );
	GtkWidget *caption = gtk_label_new( "Current material:" );
	gtk_label_set_xalign( GTK_LABEL( caption ), 0.0f );
	gtk_widget_add_css_class( caption, "dim-label" );
	GtkWidget *curName =
	    gtk_label_new( st->currentMaterial.empty() ? "(none)" : st->currentMaterial.c_str() );
	gtk_label_set_xalign( GTK_LABEL( curName ), 0.0f );
	gtk_label_set_wrap( GTK_LABEL( curName ), TRUE );
	gtk_box_append( GTK_BOX( nameCol ), caption );
	gtk_box_append( GTK_BOX( nameCol ), curName );
	gtk_box_append( GTK_BOX( header ), nameCol );

	GtkWidget *applyBtn = gtk_button_new_with_label( "Apply to Selection" );
	gtk_widget_set_valign( applyBtn, GTK_ALIGN_CENTER );
	gtk_widget_add_css_class( applyBtn, "suggested-action" );
	g_signal_connect( applyBtn, "clicked", G_CALLBACK( OnApplyToSelection ), st );
	gtk_box_append( GTK_BOX( header ), applyBtn );
	gtk_box_append( GTK_BOX( box ), header );

	// Keep the header preview live: SetCurrentMaterial mirrors here while open.
	st->texToolWindow = win;
	st->texToolSwatch = swatch;
	st->texToolName = GTK_LABEL( curName );
	g_signal_connect( win, "destroy", G_CALLBACK( OnTextureWindowDestroy ), st );

	// Name filter over the grid.
	GtkWidget *search = gtk_search_entry_new();
	gtk_widget_set_hexpand( search, TRUE );
	gtk_box_append( GTK_BOX( box ), search );

	GtkWidget *scroll = gtk_scrolled_window_new();
	gtk_widget_set_vexpand( scroll, TRUE );
	GtkWidget *flow = gtk_flow_box_new();
	gtk_flow_box_set_selection_mode( GTK_FLOW_BOX( flow ), GTK_SELECTION_SINGLE );
	gtk_flow_box_set_max_children_per_line( GTK_FLOW_BOX( flow ), 32 );
	gtk_flow_box_set_activate_on_single_click( GTK_FLOW_BOX( flow ), TRUE );
	g_signal_connect( flow, "child-activated", G_CALLBACK( OnTextureChosen ), st );

	// The filter substring is owned by the flowbox and freed when it is destroyed.
	std::string *filter = new std::string();
	g_object_set_data_full( G_OBJECT( flow ), "filter", filter,
	    []( gpointer p )
	    {
		    delete static_cast<std::string *>( p );
	    } );
	gtk_flow_box_set_filter_func( GTK_FLOW_BOX( flow ), TextureFilter, filter, nullptr );
	g_signal_connect( search, "search-changed", G_CALLBACK( OnTextureSearch ), flow );

	// Populate a bounded number of thumbnails (decoding every shipped material up
	// front would be needlessly slow); the status line reports the true total.
	const std::size_t kMaxThumbnails = 400;
	std::size_t shown = 0;
	for ( const std::string &name : st->catalog->MaterialNames() )
	{
		if ( shown >= kMaxThumbnails )
		{
			break;
		}
		GdkTexture *tex = MakeThumbnail( st, name, 96 );
		if ( !tex )
		{
			continue;
		}
		GtkWidget *cell = gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 );
		GtkWidget *pic = gtk_picture_new_for_paintable( GDK_PAINTABLE( tex ) );
		gtk_widget_set_size_request( pic, 96, 96 );
		g_object_unref( tex );
		GtkWidget *lbl = gtk_label_new( name.c_str() );
		gtk_label_set_ellipsize( GTK_LABEL( lbl ), PANGO_ELLIPSIZE_MIDDLE );
		gtk_widget_set_size_request( lbl, 96, -1 );
		gtk_widget_add_css_class( lbl, "caption" );
		gtk_box_append( GTK_BOX( cell ), pic );
		gtk_box_append( GTK_BOX( cell ), lbl );
		gtk_widget_set_tooltip_text( cell, name.c_str() );
		gtk_flow_box_append( GTK_FLOW_BOX( flow ), cell );
		// Attach the material name to the child that GTK wraps our cell in.
		GtkWidget *fchild = gtk_widget_get_parent( cell );
		if ( GTK_IS_FLOW_BOX_CHILD( fchild ) )
		{
			g_object_set_data_full(
			    G_OBJECT( fchild ), "material", g_strdup( name.c_str() ), g_free );
		}
		++shown;
	}
	gtk_scrolled_window_set_child( GTK_SCROLLED_WINDOW( scroll ), flow );

	char status[192];
	std::snprintf( status, sizeof( status ),
	    "Showing %zu of %zu materials  ·  click a thumbnail to make it active, then click a "
	    "brush (Material tool) or use Apply to Selection",
	    shown, st->catalog->MaterialNames().size() );
	GtkWidget *statusLbl = gtk_label_new( status );
	gtk_label_set_wrap( GTK_LABEL( statusLbl ), TRUE );
	gtk_widget_add_css_class( statusLbl, "dim-label" );

	gtk_box_append( GTK_BOX( box ), scroll );
	gtk_box_append( GTK_BOX( box ), statusLbl );
	gtk_window_set_child( GTK_WINDOW( win ), box );
	gtk_window_present( GTK_WINDOW( win ) );
}

void ActionBrowseMaterials( GSimpleAction *, GVariant *, gpointer user_data )
{
	OpenTextureWindow( static_cast<AppState *>( user_data ) );
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
	    { "mount-assets", ActionMountAssets, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "browse-materials", ActionBrowseMaterials, nullptr, nullptr, nullptr, { 0, 0, 0 } },
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
int RenderTexturedScreenshot( const std::string &vmfPath, const std::string &outPpm, int width,
    int height, const std::string &vpkList );

int main( int argc, char **argv )
{
	std::string screenshotOut;
	std::string screenshotIn;
	std::string quadOut;
	std::string quadIn;
	std::string demoOut;
	std::string cquadOut;
	std::string cquadIn;
	std::string texturedOut;
	std::string texturedIn;
	std::string texturedVpks;
	std::string openPath;
	std::string mountVpks;
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
		else if ( a == "--textured" && i + 3 < argc )
		{
			texturedOut = argv[++i];
			texturedIn = argv[++i];
			texturedVpks = argv[++i];
		}
		else if ( a == "--open" && i + 1 < argc )
		{
			openPath = argv[++i];
		}
		else if ( a == "--mount" && i + 1 < argc )
		{
			mountVpks = argv[++i];
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
	if ( !texturedOut.empty() )
	{
		return RenderTexturedScreenshot( texturedIn, texturedOut, width, height, texturedVpks );
	}

	AppState st;
	st.openOnStart = openPath;
	st.mountOnStart = mountVpks;
	return RunApp( &st, argv );
}
