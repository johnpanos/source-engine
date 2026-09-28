//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: GTK4 + libadwaita desktop shell for Hammer (RFC 0002,
//			linux-gtk-desktop). This is the delivery-target host: it presents the
//			classic Hammer layout (menu, toolbar, tool palette, the four viewports,
//			object bar, status bar) over the strict headless editor core.
//
//			It is a THIN HOST over hammer::presenters::EditorWorkspace, which owns
//			the one document and undo history (EditSession), the editor settings,
//			the named command catalog (SessionCommands), the tools, the viewport
//			cameras and their navigation, the render snapshot and the action
//			catalog. This file only:
//
//			  * turns GTK pointer, key and scroll events into tools:: events and
//			    hands them to the workspace (keys go through the ActionCatalog's
//			    shortcuts first, as in every host);
//			  * draws the workspace's snapshot, grid lines and tool overlay through
//			    the workspace's cameras (renderer.h);
//			  * fulfils host requests the workspace cannot: file dialogs, running
//			    the map, and F9's build OFF the UI thread (MapBuildQueue);
//			  * builds its menus from the ActionCatalog.
//
//			Files go through the strict VMF codec (hammer::formats::VmfMapCodec)
//			over the DiskFileStore. All GTK/GDK/GL native detail is confined here.
//			A headless "--screenshot/--quad" path (offscreen.cpp) shares the
//			renderer for windowless verification.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_byte_store.h"
#include "../../platform/posix/tool_process_provider.h"
#include "../../platform/runners/thread_task_runner.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/adapters/platform/tool_process_map_builder.h"
#include "hammer/app/map_build_queue.h"
#include "hammer/formats/material_catalog.h"
#include "hammer/formats/search_path_assets.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/formats/vpk_archive.h"
#include "hammer/presenters/editor_workspace.h"
#include "hammer/tools/block_tool.h"
#include "hammer/tools/entity_tool.h"
#include "hammer/tools/selection_tool.h"
#ifdef HAMMER_KTX_PREVIEW
#include "hammer/adapters/source/ktx2_preview.h"
#endif

#include "glib_task_runner.h"
#include "renderer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <adwaita.h>
#include <epoxy/gl.h>
#include <gtk/gtk.h>

namespace
{

constexpr const char *kAppId = "com.valvesoftware.HammerGtk";

using hammer::viewport::ViewKind;
namespace tools = hammer::tools;

struct AppState;

struct Viewport
{
	AppState *app = nullptr;
	hammergtk::Renderer renderer;
	ViewKind kind = ViewKind::Camera3D;
	const char *label = "";
	GtkGLArea *area = nullptr;
	GtkWidget *caption = nullptr; // OSD label, updated when Tab cycles the 2D view
	bool glReady = false;
	bool uploaded = false; // the renderer holds the current scene

	tools::PointerButton button = tools::PointerButton::None; // held, while dragging
	double startX = 0.0; // press position (logical widget pixels)
	double startY = 0.0;
	double cursorX = 0.0;
	double cursorY = 0.0;
	double pinchPrev = 1.0;
};

// What the renderers were last given: the snapshot revision and the selection
// its highlight flags came from. A change of either re-uploads the scene.
struct SceneKey
{
	std::uint64_t revision = ~std::uint64_t( 0 );
	hammer::app::Selection selection;
	const void *catalog = nullptr;

	friend bool operator==( const SceneKey &, const SceneKey & ) = default;
};

struct AppState
{
	AppState( std::string builds, bool publish, std::string lighting )
	    : buildsRoot( std::move( builds ) ), publishBuilds( publish ),
	      buildLighting( std::move( lighting ) )
	{
	}

	// Where F9 writes its build records, whether it publishes the map for
	// ./play, and the export profile the lighting back end lights it with
	// (empty: vrad's lighting). Fixed by the composition root (--builds,
	// --no-publish, --lighting).
	const std::string buildsRoot;
	const bool publishBuilds;
	const std::string buildLighting;

	// Services the workspace borrows: the strict VMF codec, the disk store and
	// the map builder over the platform tool-process provider (the editor runs
	// from the repository root). Declared before the workspace, which borrows
	// them and so must go first.
	hammer::formats::VmfMapCodec codec;
	hammer::adapters::platform::DiskFileStore store;
	std::unique_ptr<platform::IToolProcessProvider> processes =
	    platform::CreatePosixToolProcessProvider();
	hammer::adapters::platform::ToolProcessMapBuilder builder{ *processes, ".", buildsRoot,
	    []( const std::string &path )
	    {
		    return path;
	    } };
	hammer::presenters::EditorWorkspace workspace{
	    hammer::presenters::WorkspaceServices{ &codec, &store, &builder, nullptr, nullptr } };

	// F9 compiles off the UI thread: the queue runs the builder on its own
	// thread and replies on the GTK main loop. Declaration order is teardown
	// order in reverse: the queue goes first, then the build thread joins (a
	// running compile finishes and may still post its reply), then the UI
	// runner drops that reply, and only then the builder.
	hammer::gtk::GlibTaskRunner uiRunner;
	platform::ThreadTaskRunner buildThread{ "hammer-build" };
	hammer::app::MapBuildQueue builds{ builder, buildThread, uiRunner };

	std::array<Viewport, 4> viewports;
	Viewport *hovered = nullptr; // the view keys go to when focus is elsewhere
	SceneKey sceneKey;
	guint flyTick = 0;    // frame-clock tick while the camera flies
	gint64 flyPrevUs = 0; // last tick time, 0 = uninitialised

	std::string openOnStart;
	bool startMaximized = false; // --maximized: window at the screen origin
	std::string mountOnStart;    // comma-separated _dir.vpk paths to mount at launch

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
	GtkToggleButton *entityBtn = nullptr;
	bool suppressToolSignal = false;

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
// Presentation refresh: the workspace's state onto the widgets.
// ---------------------------------------------------------------------------

void SetHelp( AppState *st, const std::string &text )
{
	if ( st->helpLabel )
	{
		gtk_label_set_text( st->helpLabel, text.c_str() );
	}
}

std::string ActiveToolName( AppState *st )
{
	const tools::ITool *active = st->workspace.Tools().Active();
	return active ? std::string( active->Name() ) : std::string();
}

std::string ToolDisplayName( const std::string &tool )
{
	if ( tool == tools::BlockTool::kName )
		return "Block";
	if ( tool == tools::EntityTool::kName )
		return "Entity";
	if ( tool == tools::SelectionTool::kName )
		return "Select";
	std::string name = tool;
	if ( !name.empty() )
		name[0] = static_cast<char>( std::toupper( static_cast<unsigned char>( name[0] ) ) );
	return name;
}

// Syncs the menu actions' enabled and checked state with the catalog.
void UpdateActions( AppState *st );

void UpdateChrome( AppState *st )
{
	hammer::presenters::EditorWorkspace &ws = st->workspace;
	if ( st->objectsCount )
	{
		char buf[128];
		char sel[48] = { 0 };
		const std::size_t n = ws.Session().CurrentSelection().objects.size();
		if ( n > 0 )
		{
			std::snprintf( sel, sizeof( sel ), "  ·  %zu selected", n );
		}
		std::snprintf(
		    buf, sizeof( buf ), "%zu brush(es)%s", ws.Session().Document().SolidIds().size(), sel );
		gtk_label_set_text( st->objectsCount, buf );
	}
	if ( st->snapLabel )
	{
		char buf[64];
		std::snprintf( buf, sizeof( buf ), "Grid: %g  ·  %s", ws.Settings().gridSize,
		    ToolDisplayName( ActiveToolName( st ) ).c_str() );
		gtk_label_set_text( st->snapLabel, buf );
	}
	if ( st->window )
	{
		const std::string &path = ws.MapPath();
		const std::string base =
		    path.empty() ? std::string( "untitled" ) : path.substr( path.find_last_of( '/' ) + 1 );
		const std::string title = "Hammer - [" + base + ( ws.Session().IsModified() ? " *]" : "]" );
		gtk_window_set_title( GTK_WINDOW( st->window ), title.c_str() );
	}
	st->suppressToolSignal = true;
	const std::string tool = ActiveToolName( st );
	if ( st->selectBtn )
		gtk_toggle_button_set_active( st->selectBtn, tool == tools::SelectionTool::kName );
	if ( st->blockBtn )
		gtk_toggle_button_set_active( st->blockBtn, tool == tools::BlockTool::kName );
	if ( st->entityBtn )
		gtk_toggle_button_set_active( st->entityBtn, tool == tools::EntityTool::kName );
	st->suppressToolSignal = false;
	UpdateActions( st );
}

// Logical size of a viewport's widget, which is the size its camera works in.
void ViewSize( const Viewport &vp, int &w, int &h )
{
	w = gtk_widget_get_width( GTK_WIDGET( vp.area ) );
	h = gtk_widget_get_height( GTK_WIDGET( vp.area ) );
}

// Tells the workspace this viewport's size before it interprets an event or
// draws (two panes may show the same kind after Tab).
void SyncViewSize( Viewport &vp )
{
	if ( !vp.area )
		return;
	int w = 0;
	int h = 0;
	ViewSize( vp, w, h );
	vp.app->workspace.SetViewportSize( vp.kind, w, h );
}

// Redraws every viewport; re-uploads the scene to those whose renderer is
// behind the workspace's snapshot, selection or material catalog.
void RefreshScene( AppState *st )
{
	SceneKey key{ st->workspace.SnapshotRevision(), st->workspace.Session().CurrentSelection(),
	    st->catalog.get() };
	if ( !( key == st->sceneKey ) )
	{
		st->sceneKey = key;
		for ( Viewport &vp : st->viewports )
		{
			vp.uploaded = false;
		}
	}
	for ( Viewport &vp : st->viewports )
	{
		if ( vp.area )
		{
			gtk_gl_area_queue_render( vp.area );
		}
	}
	UpdateChrome( st );
}

// Applies what the workspace reported for one input event: its status message
// and a redraw.
void Apply( AppState *st, const hammer::presenters::InputOutcome &outcome );

// ---------------------------------------------------------------------------
// Game-asset mounting and material preview.
// ---------------------------------------------------------------------------

// Nearest-neighbour downscale of an RGBA image into a swatch-sized preview stored
// on the AppState, so the (Cairo) object-bar swatch can paint the real texture.
void SetCurrentMaterial( AppState *st, const std::string &name )
{
	st->currentMaterial = name;
	// The current material IS the editor settings' face material, which
	// apply_material uses and new geometry gets. One owner, one update.
	if ( auto set = st->workspace.Commands().Execute( "set_material", { { "material", name } } );
	    !set )
	{
		SetHelp( st, "set_material: " + set.Error().detail );
	}

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

	RefreshScene( st );
	char msg[128];
	std::snprintf( msg, sizeof( msg ), "Mounted %zu archive(s); %zu materials available",
	    st->archives.size(), count );
	SetHelp( st, msg );
	return count;
}

// ---------------------------------------------------------------------------
// File operations: the workspace's Open/Save over the strict codec.
// ---------------------------------------------------------------------------

void DoOpen( AppState *st, const std::string &path )
{
	if ( !st->workspace.Open( path ) )
	{
		SetHelp( st, st->workspace.Status().Message() );
		return;
	}
	RefreshScene( st );
	SetHelp( st, "Opened " + path );
}

void DoSave( AppState *st, std::string path )
{
	if ( !st->workspace.Save( path ) )
	{
		SetHelp( st, st->workspace.Status().Message() );
		return;
	}
	UpdateChrome( st );
	SetHelp( st, "Saved " + path );
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
	if ( st->workspace.MapPath().empty() )
	{
		ActionSaveAs( action, param, user_data );
	}
	else
	{
		DoSave( st, st->workspace.MapPath() );
	}
}

// ---- Map build (F9) -------------------------------------------------------

std::string MapNameOf( const std::string &path )
{
	std::string map = path.substr( path.find_last_of( '/' ) + 1 );
	map = map.substr( 0, map.rfind( '.' ) );
	for ( char &c : map )
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	return map;
}

void RunMap( const std::string &map )
{
	const gchar *argv[] = { "./play", map.c_str(), nullptr };
	g_spawn_async( nullptr, const_cast<gchar **>( argv ), nullptr, G_SPAWN_DEFAULT, nullptr,
	    nullptr, nullptr, nullptr );
}

// The catalog's map.build / map.build_and_run, run the way this host must: it
// saves on this thread, then compiles off it and publishes the map (./play
// <map>); 'run' also launches it (Source 2's "load in engine after building").
// The editor stays usable while the map compiles.
void StartBuild( AppState *st, bool run )
{
	if ( st->builds.Busy() )
	{
		SetHelp( st, "A build is already running" );
		return;
	}
	const std::string path = st->workspace.MapPath().empty()
	                             ? std::string( "quality-results/hammer-builds/untitled.vmf" )
	                             : st->workspace.MapPath();
	if ( !st->workspace.Save( path ) )
	{
		SetHelp( st, "build_map: " + st->workspace.Status().Message() );
		return;
	}
	UpdateChrome( st );
	const std::string map = MapNameOf( path );
	const hammer::app::BuildStart started =
	    st->builds.Start( hammer::ports::MapBuildRequest{ path, false, st->publishBuilds,
	                                                      st->buildLighting },
	        [st, map, run]( const hammer::ports::MapBuildResult &result )
	        {
		        if ( !result.ok )
		        {
			        SetHelp( st, "build_map: " + result.status + ": " + result.detail );
			        return;
		        }
		        if ( !st->publishBuilds )
		        {
			        SetHelp( st, "Built " + map + " (not published)" );
			        return;
		        }
		        SetHelp( st,
		            "Built " + map + ( run ? "; launching ./play " + map : "; ./play " + map ) );
		        if ( run )
		        {
			        RunMap( map );
		        }
	        } );
	SetHelp( st, started == hammer::app::BuildStart::kStarted
	                 ? "Building " + map + " ..."
	                 : std::string( "Build unavailable" ) );
}

// ---- Actions ----------------------------------------------------------------

void ActionOpen( GSimpleAction *, GVariant *, gpointer user_data );
void ActionSaveAs( GSimpleAction *, GVariant *, gpointer user_data );
void OpenTextureWindow( AppState *st );

void Apply( AppState *st, const hammer::presenters::InputOutcome &outcome )
{
	if ( !outcome.status.empty() )
	{
		SetHelp( st, outcome.status );
	}
	if ( outcome.hostRequest == "open_dialog" )
	{
		ActionOpen( nullptr, nullptr, st );
	}
	else if ( outcome.hostRequest == "save_dialog" )
	{
		ActionSaveAs( nullptr, nullptr, st );
	}
	else if ( outcome.hostRequest == "run_map" && !st->workspace.MapPath().empty() )
	{
		RunMap( MapNameOf( st->workspace.MapPath() ) );
	}
	else if ( outcome.hostRequest == "properties" )
	{
		SetHelp( st, "Properties: not in this shell yet" );
	}
	if ( outcome.actionId == "tools.face" )
	{
		// Legacy Shift+A: the face tool comes with the Texture Application window.
		OpenTextureWindow( st );
	}
	if ( outcome.redraw || outcome.handled || !outcome.actionId.empty() )
	{
		RefreshScene( st );
	}
}

// Runs a catalog action as a menu, toolbar or palette button would. The build
// actions run asynchronously here (StartBuild) instead of in the workspace.
void RunCatalogAction( AppState *st, const std::string &id )
{
	if ( id == "map.build" || id == "map.build_and_run" )
	{
		StartBuild( st, id == "map.build_and_run" );
		return;
	}
	Apply( st, st->workspace.RunAction( id ) );
}

// A catalog id as a GAction name ("edit.undo" -> "act-edit-undo").
std::string ActionNameOf( const std::string &id )
{
	std::string name = "act-" + id;
	for ( char &c : name )
	{
		if ( c == '.' || c == '_' )
			c = '-';
	}
	return name;
}

struct CatalogAction
{
	AppState *st;
	std::string id;
	GSimpleAction *action;
};

std::vector<std::unique_ptr<CatalogAction>> &CatalogActions()
{
	static std::vector<std::unique_ptr<CatalogAction>> actions;
	return actions;
}

void OnCatalogAction( GSimpleAction *, GVariant *, gpointer user_data )
{
	const CatalogAction *a = static_cast<const CatalogAction *>( user_data );
	RunCatalogAction( a->st, a->id );
}

void UpdateActions( AppState *st )
{
	static std::uint64_t seen = ~std::uint64_t( 0 );
	hammer::presenters::ActionCatalog &catalog = st->workspace.Actions();
	if ( catalog.Revision() == seen )
		return;
	seen = catalog.Revision();
	for ( const auto &a : CatalogActions() )
	{
		g_simple_action_set_enabled( a->action, catalog.IsEnabled( a->id ) );
		if ( const std::optional<bool> checked = catalog.IsChecked( a->id ) )
		{
			g_simple_action_set_state( a->action, g_variant_new_boolean( *checked ) );
		}
	}
}

void ActionNew( GSimpleAction *, GVariant *, gpointer user_data )
{
	RunCatalogAction( static_cast<AppState *>( user_data ), "file.new" );
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
		SyncViewSize( vp );
	}
	st->workspace.FrameDocument();
	RefreshScene( st );
}

void SetTool( AppState *st, const char *actionId )
{
	RunCatalogAction( st, actionId );
	const std::string tool = ActiveToolName( st );
	SetHelp(
	    st, tool == tools::BlockTool::kName
	            ? "Block tool (Shift+B): drag in a 2D view (again in another view for the height), "
	              "Enter to create, F to hollow"
	        : tool == tools::EntityTool::kName
	            ? "Entity tool (Shift+E): click in a view to place " +
	                  st->workspace.Settings().entityClass
	            : "Selection tool (Shift+S): click to select, drag to move" );
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
	vp->glReady = true;
	vp->uploaded = false;
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
	AppState *st = vp->app;
	hammer::presenters::EditorWorkspace &ws = st->workspace;
	if ( !vp->glReady )
	{
		return TRUE;
	}
	if ( !vp->uploaded )
	{
		vp->renderer.SetMaterialCatalog( st->catalog.get() );
		vp->renderer.SetSnapshot( ws.Snapshot() );
		vp->uploaded = true;
	}
	SyncViewSize( *vp );
	const int scale = gtk_widget_get_scale_factor( GTK_WIDGET( area ) );
	const int w = gtk_widget_get_width( GTK_WIDGET( area ) ) * scale;
	const int h = gtk_widget_get_height( GTK_WIDGET( area ) ) * scale;
	const tools::OverlayList overlay = ws.Overlay( vp->kind );
	if ( vp->kind == ViewKind::Camera3D )
	{
		vp->renderer.Render3D( ws.Camera3DView(), overlay, w, h );
	}
	else
	{
		vp->renderer.Render2D(
		    ws.Camera2DFor( vp->kind ), ws.GridLines( vp->kind ), overlay, w, h );
	}
	return TRUE;
}

// ---- Input translation -------------------------------------------------------

tools::Modifiers ModifiersOf( GdkModifierType state )
{
	std::uint8_t bits = 0;
	if ( state & GDK_SHIFT_MASK )
		bits |= tools::kShift;
	if ( state & GDK_CONTROL_MASK )
		bits |= tools::kCtrl;
	if ( state & GDK_ALT_MASK )
		bits |= tools::kAlt;
	return tools::Mods( bits );
}

tools::PointerButton ButtonOf( guint button )
{
	switch ( button )
	{
	case GDK_BUTTON_PRIMARY:
		return tools::PointerButton::Left;
	case GDK_BUTTON_MIDDLE:
		return tools::PointerButton::Middle;
	case GDK_BUTTON_SECONDARY:
		return tools::PointerButton::Right;
	default:
		return tools::PointerButton::None;
	}
}

std::optional<tools::KeyEvent> KeyEventOf( guint keyval, GdkModifierType state, bool press )
{
	tools::KeyEvent event;
	event.phase = press ? tools::KeyPhase::Press : tools::KeyPhase::Release;
	event.modifiers = ModifiersOf( state );
	using K = tools::Key;
	switch ( keyval )
	{
	case GDK_KEY_Return:
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
		event.key = K::Enter;
		return event;
	case GDK_KEY_Escape:
		event.key = K::Escape;
		return event;
	case GDK_KEY_Delete:
	case GDK_KEY_KP_Delete:
		event.key = K::Delete;
		return event;
	case GDK_KEY_BackSpace:
		event.key = K::Backspace;
		return event;
	case GDK_KEY_Left:
	case GDK_KEY_KP_Left:
		event.key = K::Left;
		return event;
	case GDK_KEY_Right:
	case GDK_KEY_KP_Right:
		event.key = K::Right;
		return event;
	case GDK_KEY_Up:
	case GDK_KEY_KP_Up:
		event.key = K::Up;
		return event;
	case GDK_KEY_Down:
	case GDK_KEY_KP_Down:
		event.key = K::Down;
		return event;
	case GDK_KEY_Page_Up:
		event.key = K::PageUp;
		return event;
	case GDK_KEY_Page_Down:
		event.key = K::PageDown;
		return event;
	case GDK_KEY_Home:
		event.key = K::Home;
		return event;
	case GDK_KEY_End:
		event.key = K::End;
		return event;
	case GDK_KEY_Tab:
	case GDK_KEY_ISO_Left_Tab:
		event.key = K::Tab;
		return event;
	case GDK_KEY_space:
		event.key = K::Space;
		return event;
	default:
		break;
	}
	if ( keyval >= GDK_KEY_F1 && keyval <= GDK_KEY_F12 )
	{
		event.key = static_cast<K>( static_cast<int>( K::F1 ) + ( keyval - GDK_KEY_F1 ) );
		return event;
	}
	const guint32 ch = gdk_keyval_to_unicode( gdk_keyval_to_lower( keyval ) );
	if ( ch > 0x20 && ch < 0x7f )
	{
		event.key = K::Character;
		event.character = static_cast<char>( std::tolower( static_cast<int>( ch ) ) );
		return event;
	}
	return std::nullopt; // modifiers alone, dead keys, non-ASCII
}

void UpdateCoords( Viewport *vp )
{
	AppState *st = vp->app;
	if ( !st->coordLabel || vp->kind == ViewKind::Camera3D )
	{
		return;
	}
	const hammer::viewport::Camera2D &camera = st->workspace.Camera2DFor( vp->kind );
	const hammer::viewport::PlanePoint p = camera.ScreenToPlane( vp->cursorX, vp->cursorY );
	const hammer::viewport::ViewAxes axes = camera.Axes();
	const char *letters = "xyz";
	char buf[96];
	std::snprintf(
	    buf, sizeof( buf ), "%c %.0f  %c %.0f", letters[axes.u], p.u, letters[axes.v], p.v );
	gtk_label_set_text( st->coordLabel, buf );
}

const char *GdkCursorName( tools::Cursor cursor )
{
	switch ( cursor )
	{
	case tools::Cursor::Crosshair:
		return "crosshair";
	case tools::Cursor::Move:
		return "move";
	case tools::Cursor::ResizeN:
	case tools::Cursor::ResizeS:
		return "ns-resize";
	case tools::Cursor::ResizeE:
	case tools::Cursor::ResizeW:
		return "ew-resize";
	case tools::Cursor::ResizeNE:
	case tools::Cursor::ResizeSW:
		return "nesw-resize";
	case tools::Cursor::ResizeNW:
	case tools::Cursor::ResizeSE:
		return "nwse-resize";
	case tools::Cursor::Rotate:
		return "grab";
	case tools::Cursor::Forbidden:
		return "not-allowed";
	case tools::Cursor::Hand:
		return "pointer";
	case tools::Cursor::Text:
		return "text";
	case tools::Cursor::Default:
	default:
		return nullptr;
	}
}

void UpdateCursor( Viewport *vp )
{
	const char *name =
	    GdkCursorName( vp->app->workspace.CursorFor( vp->kind, vp->cursorX, vp->cursorY ) );
	gtk_widget_set_cursor_from_name( GTK_WIDGET( vp->area ), name );
}

void SendPointer( Viewport *vp, tools::PointerPhase phase, tools::PointerButton button, double x,
    double y, GdkModifierType state )
{
	AppState *st = vp->app;
	SyncViewSize( *vp );
	tools::PointerEvent event;
	event.phase = phase;
	event.button = button;
	event.modifiers = ModifiersOf( state );
	event.x = x;
	event.y = y;
	event.view = vp->kind;
	event.timestampMs = static_cast<std::uint64_t>( g_get_monotonic_time() / 1000 );
	Apply( st, st->workspace.OnPointer( vp->kind, event ) );
	vp->cursorX = x;
	vp->cursorY = y;
	UpdateCoords( vp );
	UpdateCursor( vp );
}

// ---- Camera fly (held W/A/S/D/E/Q, integrated per frame) ---------------------

gboolean OnFlyTick( GtkWidget *, GdkFrameClock *clock, gpointer user_data );

// Runs the frame-clock tick exactly while the camera controller has fly keys
// held, so the clock idles when nothing moves.
void UpdateFlyTick( AppState *st )
{
	const bool flying = st->workspace.CameraControl().Flying();
	if ( flying && st->flyTick == 0 && st->window )
	{
		st->flyPrevUs = 0;
		st->flyTick = gtk_widget_add_tick_callback( st->window, OnFlyTick, st, nullptr );
	}
	else if ( !flying && st->flyTick != 0 )
	{
		gtk_widget_remove_tick_callback( st->window, st->flyTick );
		st->flyTick = 0;
	}
}

gboolean OnFlyTick( GtkWidget *, GdkFrameClock *clock, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	const gint64 now = gdk_frame_clock_get_frame_time( clock );
	if ( st->flyPrevUs != 0 )
	{
		// Long stalls are clamped, like MFC's ProcessInput.
		const double dt = std::min( static_cast<double>( now - st->flyPrevUs ) / 1.0e6, 0.25 );
		if ( dt > 0.0 )
		{
			Apply( st, st->workspace.Advance( dt ) );
		}
	}
	st->flyPrevUs = now;
	return G_SOURCE_CONTINUE;
}

// ---- Keys ----------------------------------------------------------------------

void CycleView2D( Viewport *vp );

// One key event for 'vp' (the focused view, or the hovered one when focus is
// elsewhere). Host keys first: Tab cycles a 2D pane's view, and the catalog's
// build chords run the asynchronous build. Everything else goes to the
// workspace (catalog shortcuts, camera keys, then the active tool).
gboolean HandleKey( AppState *st, Viewport *vp, guint keyval, GdkModifierType state, bool press )
{
	if ( !vp )
	{
		vp = st->hovered ? st->hovered : &st->viewports[1];
	}
	const std::optional<tools::KeyEvent> event = KeyEventOf( keyval, state, press );
	if ( !event )
	{
		return FALSE;
	}
	if ( press )
	{
		if ( event->key == tools::Key::Tab && !event->modifiers.Any() &&
		     vp->kind != ViewKind::Camera3D )
		{
			CycleView2D( vp );
			return TRUE;
		}
		const std::string chord = hammer::presenters::EditorWorkspace::ChordOf( *event );
		if ( const hammer::presenters::ActionSpec *spec =
		         hammer::presenters::ActionCatalog::FindByShortcut( chord ) )
		{
			if ( spec->id == "map.build" || spec->id == "map.build_and_run" )
			{
				RunCatalogAction( st, spec->id );
				return TRUE;
			}
		}
	}
	SyncViewSize( *vp );
	const hammer::presenters::InputOutcome outcome = st->workspace.OnKey( vp->kind, *event );
	Apply( st, outcome );
	UpdateFlyTick( st );
	return outcome.handled ? TRUE : FALSE;
}

gboolean OnViewKeyPressed(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	return HandleKey( vp->app, vp, keyval, state, true );
}

void OnViewKeyReleased(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	HandleKey( vp->app, vp, keyval, state, false );
}

// Keys that reach the window while no viewport has focus (a palette button,
// the object bar) go to the hovered view. A focused viewport already had them.
bool ViewportHasFocus( AppState *st )
{
	GtkWidget *focus = st->window ? gtk_root_get_focus( GTK_ROOT( st->window ) ) : nullptr;
	for ( const Viewport &vp : st->viewports )
	{
		if ( focus && focus == GTK_WIDGET( vp.area ) )
			return true;
	}
	return false;
}

gboolean OnWindowKeyPressed(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	return ViewportHasFocus( st ) ? FALSE : HandleKey( st, nullptr, keyval, state, true );
}

void OnWindowKeyReleased(
    GtkEventControllerKey *, guint keyval, guint, GdkModifierType state, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( !ViewportHasFocus( st ) )
	{
		HandleKey( st, nullptr, keyval, state, false );
	}
}

void OnWindowActive( GObject *window, GParamSpec *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( !gtk_window_is_active( GTK_WINDOW( window ) ) )
	{
		Apply( st, st->workspace.OnFocusLost() );
		UpdateFlyTick( st );
	}
}

// Tab cycles a 2D pane's orientation Top -> Front -> Side -> Top, like MFC's
// draw-type cycle. Each orientation has its own workspace camera.
void CycleView2D( Viewport *vp )
{
	struct ViewDef
	{
		ViewKind kind;
		const char *label;
	};
	static const ViewDef order[] = {
	    { ViewKind::Top, "top (x/y)" },
	    { ViewKind::Front, "front (x/z)" },
	    { ViewKind::Side, "side (y/z)" },
	};
	int cur = 0;
	for ( int i = 0; i < 3; ++i )
	{
		if ( order[i].kind == vp->kind )
		{
			cur = i;
		}
	}
	const ViewDef &next = order[( cur + 1 ) % 3];
	vp->kind = next.kind;
	vp->label = next.label;
	SyncViewSize( *vp );
	if ( vp->caption )
	{
		gtk_label_set_text( GTK_LABEL( vp->caption ), next.label );
	}
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( vp->area ), GTK_ACCESSIBLE_PROPERTY_LABEL, next.label, -1 );
	UpdateCoords( vp );
	gtk_gl_area_queue_render( vp->area );
}

// ---- Pointer ---------------------------------------------------------------------

// Any button: press, drag and release become Down, Move and Up for the
// workspace, which offers them to the camera controller, then the active tool.
void OnDragBegin( GtkGestureDrag *gesture, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->button = ButtonOf( gtk_gesture_single_get_current_button( GTK_GESTURE_SINGLE( gesture ) ) );
	if ( vp->button == tools::PointerButton::None )
	{
		return; // extra mouse buttons are not editor input
	}
	vp->startX = x;
	vp->startY = y;
	gtk_widget_grab_focus( GTK_WIDGET( vp->area ) );
	SendPointer( vp, tools::PointerPhase::Down, vp->button, x, y,
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( gesture ) ) );
}

void OnDragUpdate( GtkGestureDrag *gesture, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->button == tools::PointerButton::None )
	{
		return;
	}
	SendPointer( vp, tools::PointerPhase::Move, tools::PointerButton::None, vp->startX + offX,
	    vp->startY + offY,
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( gesture ) ) );
}

void OnDragEnd( GtkGestureDrag *gesture, double offX, double offY, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	const tools::PointerButton button = vp->button;
	if ( button == tools::PointerButton::None )
	{
		return; // cancelled (OnDragCancel), or a button the tools do not use
	}
	vp->button = tools::PointerButton::None;
	SendPointer( vp, tools::PointerPhase::Up, button, vp->startX + offX, vp->startY + offY,
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( gesture ) ) );
}

// A drag GTK cancels (a popover or grab took the pointer) ends no gesture with
// a release: the workspace cancels its tool and camera drags as on focus loss.
void OnDragCancel( GtkGesture *, GdkEventSequence *, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->button == tools::PointerButton::None )
	{
		return;
	}
	vp->button = tools::PointerButton::None;
	Apply( vp->app, vp->app->workspace.OnFocusLost() );
	UpdateFlyTick( vp->app );
}

// Hover (no button held): tools show where a click would act.
void OnMotion( GtkEventControllerMotion *motion, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->button != tools::PointerButton::None )
	{
		return; // the drag gesture reports these
	}
	SendPointer( vp, tools::PointerPhase::Move, tools::PointerButton::None, x, y,
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( motion ) ) );
}

// Wheel notches go to the workspace (2D zoom about the cursor, 3D dolly).
// Touchpad scrolling pans a 2D view (Ctrl: zooms) and dollies the 3D view;
// pinches zoom. Those drive the workspace cameras' own navigation functions.
gboolean OnScroll( GtkEventControllerScroll *ctrl, double dx, double dy, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	AppState *st = vp->app;
	SyncViewSize( *vp );
	const GdkModifierType state =
	    gtk_event_controller_get_current_event_state( GTK_EVENT_CONTROLLER( ctrl ) );
	if ( gtk_event_controller_scroll_get_unit( ctrl ) == GDK_SCROLL_UNIT_WHEEL )
	{
		tools::WheelEvent wheel;
		wheel.steps = -dy;
		wheel.x = vp->cursorX;
		wheel.y = vp->cursorY;
		wheel.view = vp->kind;
		wheel.modifiers = ModifiersOf( state );
		Apply( st, st->workspace.OnWheel( vp->kind, wheel ) );
	}
	else if ( vp->kind == ViewKind::Camera3D )
	{
		st->workspace.Camera3DView().Fly( -dy * 4.0, dx * 4.0, 0.0 );
		RefreshScene( st );
	}
	else if ( state & GDK_CONTROL_MASK )
	{
		st->workspace.Camera2DFor( vp->kind )
		    .ZoomAt( vp->cursorX, vp->cursorY, std::pow( 1.02, -dy ) );
		RefreshScene( st );
	}
	else
	{
		st->workspace.Camera2DFor( vp->kind ).PanPixels( -dx, -dy );
		RefreshScene( st );
	}
	UpdateCoords( vp );
	return TRUE;
}

void OnZoomBegin( GtkGesture *, GdkEventSequence *, gpointer user_data )
{
	static_cast<Viewport *>( user_data )->pinchPrev = 1.0;
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
	double cx = vp->cursorX;
	double cy = vp->cursorY;
	gtk_gesture_get_bounding_box_center( GTK_GESTURE( zoom ), &cx, &cy );
	SyncViewSize( *vp );
	if ( vp->kind == ViewKind::Camera3D )
	{
		vp->app->workspace.Camera3DView().Fly( ( delta - 1.0 ) * 512.0, 0.0, 0.0 );
	}
	else
	{
		vp->app->workspace.Camera2DFor( vp->kind ).ZoomAt( cx, cy, delta );
	}
	RefreshScene( vp->app );
}

// A hovered viewport takes keyboard focus (MFC makes the view active on
// mouse-move) and is where window-level keys go.
void OnViewEnter( GtkEventControllerMotion *, double, double, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	vp->app->hovered = vp;
	gtk_widget_grab_focus( GTK_WIDGET( vp->area ) );
}

void OnPopoverClosed( GtkPopover *popover, gpointer )
{
	gtk_widget_unparent( GTK_WIDGET( popover ) );
}

// Right-click context menu. Faithful to MFC: the 2D views get a popup (a
// selection menu when something is selected, otherwise the default view menu);
// the 3D camera view has none (its right button looks around).
void OnContextClick( GtkGestureClick *, int, double x, double y, gpointer user_data )
{
	Viewport *vp = static_cast<Viewport *>( user_data );
	if ( vp->kind == ViewKind::Camera3D )
	{
		return;
	}
	auto item = []( GMenu *menu, const char *label, const std::string &id )
	{
		const std::string action = "app." + ActionNameOf( id );
		g_menu_append( menu, label, action.c_str() );
	};
	GMenu *model = g_menu_new();
	if ( !vp->app->workspace.Session().CurrentSelection().Empty() )
	{
		item( model, "Delete", "edit.delete" );
		item( model, "Group", "tools.group" );
		GMenu *toolsMenu = g_menu_new();
		item( toolsMenu, "Selection Tool", "tools.selection" );
		item( toolsMenu, "Block Tool", "tools.block" );
		g_menu_append_section( model, nullptr, G_MENU_MODEL( toolsMenu ) );
		g_object_unref( toolsMenu );
	}
	else
	{
		item( model, "Block Tool", "tools.block" );
		item( model, "Selection Tool", "tools.selection" );
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

GtkWidget *MakeViewport( AppState *st, int index, ViewKind kind, const char *label )
{
	Viewport &vp = st->viewports[index];
	vp.app = st;
	vp.kind = kind;
	vp.label = label;

	// An interactive region with its own pointer and key handling: the
	// "application" role, named for assistive technology (and the UI-driven
	// conformance test, which finds each view by this name).
	GtkWidget *glarea = GTK_WIDGET( g_object_new(
	    GTK_TYPE_GL_AREA, "accessible-role", GTK_ACCESSIBLE_ROLE_APPLICATION, nullptr ) );
	vp.area = GTK_GL_AREA( glarea );
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( glarea ), GTK_ACCESSIBLE_PROPERTY_LABEL, label, -1 );
	gtk_gl_area_set_allowed_apis( GTK_GL_AREA( glarea ), GDK_GL_API_GL );
	gtk_gl_area_set_required_version( GTK_GL_AREA( glarea ), 3, 3 );
	gtk_gl_area_set_has_depth_buffer( GTK_GL_AREA( glarea ), TRUE );
	gtk_widget_set_hexpand( glarea, TRUE );
	gtk_widget_set_vexpand( glarea, TRUE );
	// Focusable so a hovered view receives key events.
	gtk_widget_set_focusable( glarea, TRUE );
	g_signal_connect( glarea, "realize", G_CALLBACK( OnGlRealize ), &vp );
	g_signal_connect( glarea, "unrealize", G_CALLBACK( OnGlUnrealize ), &vp );
	g_signal_connect( glarea, "render", G_CALLBACK( OnGlRender ), &vp );

	// Every button: the workspace routes it (camera navigation, then the tool).
	GtkGesture *drag = gtk_gesture_drag_new();
	gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( drag ), 0 );
	g_signal_connect( drag, "drag-begin", G_CALLBACK( OnDragBegin ), &vp );
	g_signal_connect( drag, "drag-update", G_CALLBACK( OnDragUpdate ), &vp );
	g_signal_connect( drag, "drag-end", G_CALLBACK( OnDragEnd ), &vp );
	g_signal_connect( drag, "cancel", G_CALLBACK( OnDragCancel ), &vp );
	gtk_widget_add_controller( glarea, GTK_EVENT_CONTROLLER( drag ) );

	// Wheel, touchpad scroll (kinetic) and pinch.
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
	gtk_widget_add_controller( glarea, motion );

	GtkEventController *viewKeys = gtk_event_controller_key_new();
	g_signal_connect( viewKeys, "key-pressed", G_CALLBACK( OnViewKeyPressed ), &vp );
	g_signal_connect( viewKeys, "key-released", G_CALLBACK( OnViewKeyReleased ), &vp );
	gtk_widget_add_controller( glarea, viewKeys );

	// Right click: context menu (2D views).
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
	gtk_widget_set_can_target( caption, FALSE );
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
		SetTool( st, "tools.selection" );
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
		SetTool( st, "tools.block" );
	}
}

void OnEntityToggled( GtkToggleButton *btn, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( !st->suppressToolSignal && gtk_toggle_button_get_active( btn ) )
	{
		SetTool( st, "tools.entity" );
	}
}

// The Entity tool's class palette (Source 2's categories); P1 offers the two a
// playable room needs.
const char *const kEntityClasses[] = { "info_player_start", "light", nullptr };

void OnEntityClassChanged( GObject *dropdown, GParamSpec *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	const guint index = gtk_drop_down_get_selected( GTK_DROP_DOWN( dropdown ) );
	if ( index < G_N_ELEMENTS( kEntityClasses ) - 1 )
	{
		if ( auto set = st->workspace.Commands().Execute(
		         "set_entity_class", { { "classname", kEntityClasses[index] } } );
		    !set )
		{
			SetHelp( st, "set_entity_class: " + set.Error().detail );
		}
		else if ( ActiveToolName( st ) == tools::EntityTool::kName )
		{
			SetHelp( st, std::string( "Entity tool (Shift+E): click in a view to place " ) +
			                 kEntityClasses[index] );
		}
	}
}

void Label( GtkWidget *widget, const char *label )
{
	gtk_accessible_update_property(
	    GTK_ACCESSIBLE( widget ), GTK_ACCESSIBLE_PROPERTY_LABEL, label, -1 );
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
	gtk_widget_set_tooltip_text( select, "Selection Tool (Shift+S)" );
	Label( select, "Selection Tool" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( select ), TRUE );
	st->selectBtn = GTK_TOGGLE_BUTTON( select );
	g_signal_connect( select, "toggled", G_CALLBACK( OnSelectToggled ), st );
	gtk_box_append( GTK_BOX( palette ), select );

	GtkWidget *block = gtk_toggle_button_new();
	gtk_button_set_child(
	    GTK_BUTTON( block ), gtk_image_new_from_icon_name( "view-grid-symbolic" ) );
	gtk_widget_set_tooltip_text( block, "Block Tool (Shift+B; F makes a room)" );
	Label( block, "Block Tool" );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( block ), GTK_TOGGLE_BUTTON( select ) );
	st->blockBtn = GTK_TOGGLE_BUTTON( block );
	g_signal_connect( block, "toggled", G_CALLBACK( OnBlockToggled ), st );
	gtk_box_append( GTK_BOX( palette ), block );

	GtkWidget *entity = gtk_toggle_button_new();
	gtk_button_set_child(
	    GTK_BUTTON( entity ), gtk_image_new_from_icon_name( "insert-object-symbolic" ) );
	gtk_widget_set_tooltip_text( entity, "Entity Tool (Shift+E): click a surface in the 3D view" );
	Label( entity, "Entity Tool" );
	gtk_toggle_button_set_group( GTK_TOGGLE_BUTTON( entity ), GTK_TOGGLE_BUTTON( select ) );
	st->entityBtn = GTK_TOGGLE_BUTTON( entity );
	g_signal_connect( entity, "toggled", G_CALLBACK( OnEntityToggled ), st );
	gtk_box_append( GTK_BOX( palette ), entity );

	GtkWidget *classes = gtk_drop_down_new_from_strings( kEntityClasses );
	gtk_widget_set_tooltip_text( classes, "Entity class to place" );
	Label( classes, "Entity class" );
	g_signal_connect( classes, "notify::selected", G_CALLBACK( OnEntityClassChanged ), st );
	gtk_box_append( GTK_BOX( palette ), classes );

	// Remaining classic tools (laid out; not yet functional).
	struct Tool
	{
		const char *icon;
		const char *tip;
	};
	const Tool rest[] = {
	    { "zoom-in-symbolic", "Magnify" },
	    { "camera-photo-symbolic", "Camera" },
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
	    { "edit-undo-symbolic", "Undo", "app.act-edit-undo" },
	    { "edit-redo-symbolic", "Redo", "app.act-edit-redo" },
	    { "edit-delete-symbolic", "Delete", "app.act-edit-delete" },
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
	    GTK_PANED( topPane ), MakeViewport( st, 0, ViewKind::Camera3D, "camera" ) );
	gtk_paned_set_end_child(
	    GTK_PANED( topPane ), MakeViewport( st, 1, ViewKind::Top, "top (x/y)" ) );
	gtk_paned_set_resize_start_child( GTK_PANED( topPane ), TRUE );
	gtk_paned_set_resize_end_child( GTK_PANED( topPane ), TRUE );

	GtkWidget *bottomPane = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
	gtk_paned_set_start_child(
	    GTK_PANED( bottomPane ), MakeViewport( st, 2, ViewKind::Front, "front (x/z)" ) );
	gtk_paned_set_end_child(
	    GTK_PANED( bottomPane ), MakeViewport( st, 3, ViewKind::Side, "side (y/z)" ) );
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

// The menu bar is the ActionCatalog's categories in order, one item per action
// (app.act-*), plus the host's own entries (Save As, assets, Reset Views, Quit).
GMenu *MakeMenuModel()
{
	GMenu *bar = g_menu_new();
	for ( const std::string &category : hammer::presenters::ActionCatalog::Categories() )
	{
		GMenu *menu = g_menu_new();
		GMenu *main = g_menu_new();
		for ( const hammer::presenters::ActionSpec *spec :
		    hammer::presenters::ActionCatalog::InCategory( category ) )
		{
			const std::string action = "app." + ActionNameOf( spec->id );
			g_menu_append( main, spec->label.c_str(), action.c_str() );
		}
		g_menu_append_section( menu, nullptr, G_MENU_MODEL( main ) );
		g_object_unref( main );
		if ( category == "File" )
		{
			GMenu *more = g_menu_new();
			g_menu_append( more, "Save As…", "app.saveas" );
			g_menu_append( more, "Mount Game Assets…", "app.mount-assets" );
			g_menu_append( more, "Texture Application…", "app.browse-materials" );
			g_menu_append_section( menu, nullptr, G_MENU_MODEL( more ) );
			g_object_unref( more );
			GMenu *end = g_menu_new();
			g_menu_append( end, "Quit", "app.quit" );
			g_menu_append_section( menu, nullptr, G_MENU_MODEL( end ) );
			g_object_unref( end );
		}
		else if ( category == "View" )
		{
			GMenu *more = g_menu_new();
			g_menu_append( more, "Reset Views", "app.reset-views" );
			g_menu_append_section( menu, nullptr, G_MENU_MODEL( more ) );
			g_object_unref( more );
		}
		g_menu_append_submenu( bar, category.c_str(), G_MENU_MODEL( menu ) );
		g_object_unref( menu );
	}
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

	// Keys that reach the window with no viewport focused go to the hovered view.
	GtkEventController *keys = gtk_event_controller_key_new();
	g_signal_connect( keys, "key-pressed", G_CALLBACK( OnWindowKeyPressed ), st );
	g_signal_connect( keys, "key-released", G_CALLBACK( OnWindowKeyReleased ), st );
	gtk_widget_add_controller( window, keys );
	g_signal_connect( window, "notify::is-active", G_CALLBACK( OnWindowActive ), st );

	gtk_window_set_child( GTK_WINDOW( window ), root );

	if ( !st->mountOnStart.empty() )
	{
		MountAssets( st, st->mountOnStart );
	}

	if ( !st->openOnStart.empty() )
	{
		DoOpen( st, st->openOnStart );
	}
	else
	{
		// A fresh map, which also frames the cameras on the empty map.
		if ( !st->workspace.New() )
		{
			SetHelp( st, st->workspace.Status().Message() );
		}
	}
	RefreshScene( st );

	if ( st->startMaximized )
	{
		gtk_window_maximize( GTK_WINDOW( window ) );
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
	// The Face tool applies the active material with a right click, as the
	// legacy texture tool does.
	st->workspace.RunAction( "tools.face" );
	RefreshScene( st );
	SetHelp( st, std::string( "Active material: " ) + name +
	                 "  ·  right-click a face to apply, or use Apply to Selection" );
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

// Apply the active material to the selection (the catalog's apply_material).
void OnApplyToSelection( GtkButton *, gpointer user_data )
{
	AppState *st = static_cast<AppState *>( user_data );
	if ( st->workspace.Session().CurrentSelection().Empty() )
	{
		SetHelp( st, "Select a brush first, then Apply to Selection" );
		return;
	}
	const std::uint64_t before = st->workspace.Session().Revision();
	RunCatalogAction( st, "tools.apply_material" );
	SetHelp( st, st->workspace.Session().Revision() != before
	                 ? "Applied " + st->currentMaterial + " to the selection"
	                 : std::string( "The selection already uses that material" ) );
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
	    { "reset-views", ActionResetViews, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "mount-assets", ActionMountAssets, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	    { "browse-materials", ActionBrowseMaterials, nullptr, nullptr, nullptr, { 0, 0, 0 } },
	};
	g_action_map_add_action_entries( G_ACTION_MAP( app ), entries, G_N_ELEMENTS( entries ), st );

	// One action per catalog entry; toggles carry their checked state. Their
	// shortcuts are the catalog's and reach the workspace through the key
	// handlers, so they get no GTK accelerators.
	for ( const hammer::presenters::ActionSpec &spec : hammer::presenters::ActionCatalog::Specs() )
	{
		const std::string name = ActionNameOf( spec.id );
		const bool toggle = st->workspace.Actions().IsChecked( spec.id ).has_value();
		GSimpleAction *action = toggle ? g_simple_action_new_stateful(
		                                     name.c_str(), nullptr, g_variant_new_boolean( FALSE ) )
		                               : g_simple_action_new( name.c_str(), nullptr );
		auto &entry = CatalogActions().emplace_back(
		    std::make_unique<CatalogAction>( CatalogAction{ st, spec.id, action } ) );
		g_signal_connect( action, "activate", G_CALLBACK( OnCatalogAction ), entry.get() );
		g_action_map_add_action( G_ACTION_MAP( app ), G_ACTION( action ) );
		g_object_unref( action ); // the map holds it
	}

	struct Accel
	{
		const char *action;
		const char *accel;
	};
	const Accel accels[] = {
	    { "app.saveas", "<Control><Shift>s" },
	    { "app.quit", "<Control>q" },
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
int RenderWorkspaceDemo( const std::string &outPpm, int tileW, int tileH );
int RenderTexturedScreenshot( const std::string &vmfPath, const std::string &outPpm, int width,
    int height, const std::string &vpkList );

int main( int argc, char **argv )
{
	std::string screenshotOut;
	std::string screenshotIn;
	std::string quadOut;
	std::string quadIn;
	std::string demoOut;
	std::string texturedOut;
	std::string texturedIn;
	std::string texturedVpks;
	std::string openPath;
	bool maximized = false;
	std::string buildsRoot = "quality-results/hammer-builds";
	bool publishBuilds = true;
	std::string buildLighting;
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
		else if ( a == "--maximized" )
		{
			maximized = true;
		}
		else if ( a == "--builds" && i + 1 < argc )
		{
			buildsRoot = argv[++i];
		}
		else if ( a == "--no-publish" )
		{
			publishBuilds = false;
		}
		else if ( a == "--lighting" && i + 1 < argc )
		{
			buildLighting = argv[++i];
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
			std::printf( "Usage: hammer_gtk [--open MAP.vmf] [--maximized] [--builds DIR] "
			             "[--no-publish] [--lighting PROFILE]\n"
			             "       hammer_gtk --screenshot OUT.ppm MAP.vmf [--width W --height H]\n"
			             "       hammer_gtk --quad OUT.ppm MAP.vmf [--width W --height H]\n"
			             "       hammer_gtk --demo OUT.ppm [--width W --height H]\n" );
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
		return RenderWorkspaceDemo( demoOut, width / 2, height / 2 );
	}
	if ( !texturedOut.empty() )
	{
		return RenderTexturedScreenshot( texturedIn, texturedOut, width, height, texturedVpks );
	}

	AppState st( buildsRoot, publishBuilds, buildLighting );
	st.openOnStart = openPath;
	st.startMaximized = maximized;
	st.mountOnStart = mountVpks;
	return RunApp( &st, argv );
}
