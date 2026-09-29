//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless offscreen render path for the GTK Hammer shell (RFC 0002;
//			RFC 0016 "Editor viewports"). No window server is needed: it
//			composes the RFC 0016 render core (Vulkan device, no legacy
//			backend), opens a map through the SAME
//			hammer::presenters::EditorWorkspace the window uses (strict VMF
//			codec, render snapshot, framed cameras), draws it with the SAME
//			hammer::render_adapter::ViewportRenderer, and writes a binary PPM.
//			Forms: a single 3D view (--screenshot), the classic 2x2 quad of
//			camera/top/front/side views (--quad), and a quad of a map built by
//			simulated editor input (--demo). --textured draws the 3D view with
//			the materials' base textures from the given VPKs, through the
//			core's unlit material family (RFC 0016 K4).
//
//=============================================================================//

#include "catalog_models.h"
#include "catalog_textures.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/adapters/render/viewport_service.h"
#include "hammer/app/session_commands.h"
#include "platform/runners/manual_task_runner.h"
#include "platform/runners/thread_task_runner.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/presenters/editor_workspace.h"
#include "render/composition/render_core.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{

using hammer::viewport::ViewKind;

bool WritePpm( const std::string &path, int w, int h, const std::vector<std::uint8_t> &rgb )
{
	std::ofstream os( path, std::ios::binary );
	if ( !os )
	{
		return false;
	}
	os << "P6\n" << w << " " << h << "\n255\n";
	os.write(
	    reinterpret_cast<const char *>( rgb.data() ), static_cast<std::streamsize>( rgb.size() ) );
	return os.good();
}

// Blits a WxH RGB tile into a (2W)x(2H) composite at tile (tx,ty).
void BlitTile( std::vector<std::uint8_t> &dst, int fullW, const std::vector<std::uint8_t> &tile,
    int w, int h, int tx, int ty )
{
	for ( int y = 0; y < h; ++y )
	{
		for ( int x = 0; x < w; ++x )
		{
			const std::size_t s = ( static_cast<std::size_t>( y ) * w + x ) * 3;
			const std::size_t dx = static_cast<std::size_t>( tx * w + x );
			const std::size_t dy = static_cast<std::size_t>( ty * h + y );
			const std::size_t d = ( dy * fullW + dx ) * 3;
			dst[d + 0] = tile[s + 0];
			dst[d + 1] = tile[s + 1];
			dst[d + 2] = tile[s + 2];
		}
	}
}

// A workspace over the strict codec and the disk store (no builder, no
// catalogs): the editor's own document, snapshot and cameras.
struct Editor
{
	hammer::formats::VmfMapCodec codec;
	hammer::adapters::platform::DiskFileStore store;
	hammer::presenters::EditorWorkspace workspace{
	    hammer::presenters::WorkspaceServices{ &codec, &store, nullptr, nullptr, nullptr, {} } };

	void SetSize( int width, int height )
	{
		for ( ViewKind kind :
		    { ViewKind::Camera3D, ViewKind::Top, ViewKind::Front, ViewKind::Side } )
		{
			workspace.SetViewportSize( kind, width, height );
		}
	}

	// Opens 'path' with every camera 'width' x 'height' (so opening frames them).
	bool Open( const std::string &path, int width, int height, const char *tag )
	{
		SetSize( width, height );
		if ( !workspace.Open( path ) )
		{
			std::fprintf( stderr, "%s: failed to open %s: %s\n", tag, path.c_str(),
			    workspace.Status().Message().c_str() );
			return false;
		}
		return true;
	}
};

// The render core and the viewport renderer over its device, composed the way
// the window composes them.
struct Viewports
{
	struct CoreDeleter
	{
		void operator()( RenderCore *core ) const { RenderCore_Destroy( core ); }
	};
	std::unique_ptr<RenderCore, CoreDeleter> core;
	std::unique_ptr<hammer::render_adapter::ViewportRenderer> renderer;

	bool Compose( const char *tag, hammer::render_adapter::IMaterialTextures *textures = nullptr,
	    hammer::render_adapter::IModelSource *models = nullptr )
	{
		RenderCoreConfig config;
		config.device = "vulkan";
		config.features = "";
		config.legacyBackend = nullptr;
		RenderCoreResult result;
		core.reset( RenderCore_Create( &config, &result ) );
		const RenderCoreBinding *binding = core ? RenderCore_GetBinding( core.get() ) : nullptr;
		if ( !binding || !binding->device )
		{
			std::fprintf( stderr, "%s: the render core did not compose: %s\n", tag,
			    result.message[0] ? result.message : "no render device" );
			return false;
		}
		auto made =
		    hammer::render_adapter::ViewportRenderer::Create( *binding->device, textures, models );
		if ( !made )
		{
			std::fprintf( stderr, "%s: the viewport renderer did not start\n", tag );
			return false;
		}
		renderer = std::move( made ).Value();
		return true;
	}

	~Viewports() { renderer.reset(); } // before the core's device
};

// Draws one view of the workspace and returns it as top-down RGB.
std::vector<std::uint8_t> RenderView( hammer::render_adapter::ViewportRenderer &renderer,
    hammer::presenters::EditorWorkspace &ws, ViewKind kind, int width, int height )
{
	if ( !renderer.SetScene( ws.Snapshot(), ws.SnapshotRevision() ) )
	{
		return {};
	}
	hammer::render_adapter::ViewRequest request;
	request.kind = kind;
	if ( kind == ViewKind::Camera3D )
	{
		request.camera3D = &ws.Camera3DView();
	}
	else
	{
		request.camera2D = &ws.Camera2DFor( kind );
		request.grid = ws.GridLines( kind );
	}
	request.overlay = ws.Overlay( kind );
	request.pixelWidth = static_cast<std::uint32_t>( width );
	request.pixelHeight = static_cast<std::uint32_t>( height );
	auto pixels = renderer.RenderAndWait( request );
	if ( !pixels )
	{
		return {};
	}
	std::vector<std::uint8_t> rgb( std::size_t( width ) * height * 3 );
	for ( std::size_t i = 0, n = std::size_t( width ) * height; i < n; ++i )
	{
		rgb[i * 3 + 0] = pixels.Value().rgba[i * 4 + 0];
		rgb[i * 3 + 1] = pixels.Value().rgba[i * 4 + 1];
		rgb[i * 3 + 2] = pixels.Value().rgba[i * 4 + 2];
	}
	return rgb;
}

// Renders the workspace's 3D view to 'outPpm'.
int RenderSingle( hammer::presenters::EditorWorkspace &ws, const std::string &outPpm, int width,
    int height, const char *tag )
{
	Viewports views;
	if ( !views.Compose( tag ) )
	{
		return 4;
	}
	const std::vector<std::uint8_t> rgb =
	    RenderView( *views.renderer, ws, ViewKind::Camera3D, width, height );
	if ( rgb.empty() )
	{
		std::fprintf( stderr, "%s: the view did not render\n", tag );
		return 6;
	}
	if ( !WritePpm( outPpm, width, height, rgb ) )
	{
		std::fprintf( stderr, "%s: failed to write %s\n", tag, outPpm.c_str() );
		return 7;
	}
	std::printf( "%s: wrote %s (%dx%d), %zu solids, %u triangles, %zu entities\n", tag,
	    outPpm.c_str(), width, height, ws.Snapshot().solids.size(),
	    views.renderer->Scene().triangles, ws.Snapshot().entities.size() );
	return 0;
}

// Renders the classic 2x2 quad (camera / top / front / side) to a PPM: camera
// top-left, top(X/Y) top-right, front(X/Z) bottom-left, side(Y/Z) bottom-right,
// matching the on-screen layout.
int RenderQuadOf( hammer::presenters::EditorWorkspace &ws, const std::string &outPpm, int tileW,
    int tileH, const char *tag )
{
	Viewports views;
	if ( !views.Compose( tag ) )
	{
		return 4;
	}
	const int fullW = tileW * 2;
	const int fullH = tileH * 2;
	std::vector<std::uint8_t> composite( static_cast<std::size_t>( fullW ) * fullH * 3, 20 );
	struct Tile
	{
		ViewKind kind;
		int tx;
		int ty;
	};
	const Tile tiles[] = {
	    { ViewKind::Camera3D, 0, 0 },
	    { ViewKind::Top, 1, 0 },
	    { ViewKind::Front, 0, 1 },
	    { ViewKind::Side, 1, 1 },
	};
	for ( const Tile &t : tiles )
	{
		const std::vector<std::uint8_t> tile =
		    RenderView( *views.renderer, ws, t.kind, tileW, tileH );
		if ( tile.empty() )
		{
			std::fprintf( stderr, "%s: a view did not render\n", tag );
			return 6;
		}
		BlitTile( composite, fullW, tile, tileW, tileH, t.tx, t.ty );
	}
	if ( !WritePpm( outPpm, fullW, fullH, composite ) )
	{
		std::fprintf( stderr, "%s: failed to write %s\n", tag, outPpm.c_str() );
		return 7;
	}
	std::printf( "%s: wrote %s (%dx%d), %zu solids, %zu entities\n", tag, outPpm.c_str(), fullW,
	    fullH, ws.Snapshot().solids.size(), ws.Snapshot().entities.size() );
	return 0;
}

} // namespace

int RenderScreenshot( const std::string &vmfPath, const std::string &outPpm, int width, int height )
{
	if ( width <= 0 || height <= 0 )
	{
		std::fprintf( stderr, "screenshot: invalid size %dx%d\n", width, height );
		return 2;
	}
	Editor editor;
	if ( !editor.Open( vmfPath, width, height, "screenshot" ) )
	{
		return 3;
	}
	return RenderSingle( editor.workspace, outPpm, width, height, "screenshot" );
}

// The textured preview needs the render core's material families (RFC 0016
// K4: render.material over formats::MaterialCatalog's texels); until they land
// this refuses rather than render an untextured view under a textured name.
int RenderTexturedScreenshot( const std::string &vmfPath, const std::string &outPpm, int width,
    int height, const std::string &vpkList, const std::string &eye )
{
	const char *tag = "textured";
	if ( width <= 0 || height <= 0 )
	{
		std::fprintf( stderr, "%s: invalid size %dx%d\n", tag, width, height );
		return 2;
	}
	std::string errors;
	std::unique_ptr<hammer::gtk::CatalogTextures> textures =
	    hammer::gtk::CatalogTextures::Open( vpkList, errors );
	std::unique_ptr<hammer::gtk::CatalogModels> models =
	    hammer::gtk::CatalogModels::Open( vpkList, errors );
	if ( !textures || !models )
	{
		std::fprintf( stderr, "%s: no VPK mounted: %s\n", tag, errors.c_str() );
		return 5;
	}
	Editor editor;
	// Instances look up their files beside the map, then in the mounted
	// game's instance path and HAMMER_INSTANCE_ROOTS (comma-separated).
	const char *extraRoots = std::getenv( "HAMMER_INSTANCE_ROOTS" );
	editor.workspace.SetInstanceRoots(
	    hammer::gtk::GameInstanceRoots( vpkList, extraRoots ? extraRoots : "" ) );
	if ( !editor.Open( vmfPath, width, height, tag ) )
	{
		return 3;
	}
	if ( !eye.empty() )
	{
		// --eye X,Y,Z,TX,TY,TZ: the camera at X,Y,Z looking at TX,TY,TZ.
		double v[6] = {};
		if ( std::sscanf( eye.c_str(), "%lf,%lf,%lf,%lf,%lf,%lf", &v[0], &v[1], &v[2], &v[3], &v[4],
		         &v[5] ) != 6 ||
		     !editor.workspace.Camera3DView().SetPosition(
		         mapgeometry::Vec3d( v[0], v[1], v[2] ) ) ||
		     !editor.workspace.Camera3DView().LookAt( mapgeometry::Vec3d( v[3], v[4], v[5] ) ) )
		{
			std::fprintf( stderr, "%s: --eye wants X,Y,Z,TX,TY,TZ: %s\n", tag, eye.c_str() );
			return 2;
		}
	}
	Viewports views;
	if ( !views.Compose( tag, textures.get(), models.get() ) )
	{
		return 4;
	}
	const std::vector<std::uint8_t> rgb =
	    RenderView( *views.renderer, editor.workspace, ViewKind::Camera3D, width, height );
	if ( rgb.empty() )
	{
		std::fprintf( stderr, "%s: the view did not render\n", tag );
		return 6;
	}
	if ( !WritePpm( outPpm, width, height, rgb ) )
	{
		std::fprintf( stderr, "%s: failed to write %s\n", tag, outPpm.c_str() );
		return 7;
	}
	const hammer::render_adapter::SceneStats &scene = views.renderer->Scene();
	std::size_t placed = 0;
	for ( const hammer::viewport::InstanceDraw &instance : editor.workspace.Snapshot().instances )
	{
		placed += instance.status == hammer::ports::InstanceStatus::Placed ? 1 : 0;
	}
	std::printf( "%s: wrote %s (%dx%d), %u triangles, %u of %u materials textured, "
	             "%u without a texture, %u draws, %u model entities (%u drawn as markers, "
	             "%u model files), %zu of %zu instances placed\n",
	    tag, outPpm.c_str(), width, height, scene.triangles, scene.textures,
	    scene.textures + scene.missingTextures, scene.missingTextures,
	    views.renderer->LastView().drawn, scene.modelEntities + scene.missingModels,
	    scene.missingModels, scene.models, placed, editor.workspace.Snapshot().instances.size() );
	views.renderer.reset(); // it borrows the textures and models
	return 0;
}

int RenderQuad( const std::string &vmfPath, const std::string &outPpm, int tileW, int tileH )
{
	if ( tileW <= 0 || tileH <= 0 )
	{
		std::fprintf( stderr, "quad: invalid size %dx%d\n", tileW, tileH );
		return 2;
	}
	Editor editor;
	if ( !editor.Open( vmfPath, tileW, tileH, "quad" ) )
	{
		return 3;
	}
	return RenderQuadOf( editor.workspace, outPpm, tileW, tileH, "quad" );
}

// Builds a small map by driving the EditorWorkspace with SIMULATED input (the
// events the window sends from gestures and keys), selects one brush, and
// renders the result. A visual companion to the UI-driven conformance test.
int RenderWorkspaceDemo( const std::string &outPpm, int tileW, int tileH )
{
	if ( tileW <= 0 || tileH <= 0 )
	{
		std::fprintf( stderr, "demo: invalid size %dx%d\n", tileW, tileH );
		return 2;
	}
	Editor editor;
	hammer::presenters::EditorWorkspace &ws = editor.workspace;
	editor.SetSize( tileW, tileH );
	if ( !ws.New() )
	{
		std::fprintf( stderr, "demo: %s\n", ws.Status().Message().c_str() );
		return 3;
	}

	namespace tools = hammer::tools;
	auto pointer = [&]( ViewKind view, tools::PointerPhase phase, double u, double v )
	{
		const hammer::viewport::Camera2D &camera = ws.Camera2DFor( view );
		const hammer::viewport::ScreenPoint p = camera.PlaneToScreen( { u, v } );
		tools::PointerEvent event;
		event.phase = phase;
		event.button = phase == tools::PointerPhase::Move ? tools::PointerButton::None
		                                                  : tools::PointerButton::Left;
		event.x = p.x;
		event.y = p.y;
		event.view = view;
		ws.OnPointer( view, event );
	};
	auto drag = [&]( ViewKind view, double u0, double v0, double u1, double v1 )
	{
		pointer( view, tools::PointerPhase::Down, u0, v0 );
		pointer( view, tools::PointerPhase::Move, u1, v1 );
		pointer( view, tools::PointerPhase::Up, u1, v1 );
	};
	auto enter = [&]()
	{
		ws.OnKey( ViewKind::Top, tools::KeyEvent::Press( tools::Key::Enter ) );
	};

	// An L-shaped floor plan (one grid step tall), then a tower whose height is
	// drawn in the front view, all via simulated Block drags.
	ws.RunAction( "tools.block" );
	drag( ViewKind::Top, 0, 0, 384, 128 );
	enter();
	drag( ViewKind::Top, 0, 128, 128, 384 );
	enter();
	drag( ViewKind::Top, 256, 256, 448, 448 );
	drag( ViewKind::Front, 256, 320, 448, 0 );
	enter();

	// Select the tower (Selection tool click) so it renders highlighted.
	ws.RunAction( "tools.selection" );
	pointer( ViewKind::Top, tools::PointerPhase::Down, 350, 350 );
	pointer( ViewKind::Top, tools::PointerPhase::Up, 350, 350 );

	std::printf( "demo: built %zu brushes via simulated input, %zu selected\n",
	    ws.Session().Document().SolidIds().size(), ws.Session().CurrentSelection().objects.size() );
	ws.FrameDocument();
	return RenderQuadOf( ws, outPpm, tileW, tileH, "demo" );
}

// The edit-to-pixels measurement of quality/budgets/hammer-viewport-v1.json:
// the window's own path (EditorWorkspace, an immutable scene copy per scene
// change, ViewportService on a render thread, replies on the host sequence),
// timed from issuing one edit to all four views' new frames arriving back.
// Writes the samples in milliseconds as JSON.
int RenderEditBudget( const std::string &vmfPath, const std::string &outJson, int width, int height,
    int warmup, int edits, const std::string &vpkList, bool external )
{
	if ( width <= 0 || height <= 0 || edits <= 0 || warmup < 0 )
	{
		std::fprintf( stderr, "viewport-budget: invalid arguments\n" );
		return 2;
	}
	Editor editor;
	hammer::presenters::EditorWorkspace &ws = editor.workspace;
	if ( !vpkList.empty() )
	{
		// With the game mounted its instances draw too, as in the window.
		const char *extraRoots = std::getenv( "HAMMER_INSTANCE_ROOTS" );
		ws.SetInstanceRoots(
		    hammer::gtk::GameInstanceRoots( vpkList, extraRoots ? extraRoots : "" ) );
	}
	if ( !editor.Open( vmfPath, width, height, "viewport-budget" ) )
	{
		return 3;
	}
	const std::vector<hammer::scene::ObjectId> solids = ws.Session().Document().SolidIds();
	if ( solids.empty() )
	{
		std::fprintf( stderr, "viewport-budget: the map has no solid\n" );
		return 3;
	}
	const std::string id =
	    std::to_string( hammer::app::SessionCommands::ScriptId( solids.front() ) );
	if ( !ws.Commands().Execute( "select", { { "ids", id } } ) )
	{
		std::fprintf( stderr, "viewport-budget: could not select solid %s\n", id.c_str() );
		return 3;
	}

	Viewports views;
	if ( !views.Compose( "viewport-budget" ) )
	{
		return 4;
	}
	RenderCoreBinding const *binding = RenderCore_GetBinding( views.core.get() );
	views.renderer.reset(); // the service owns the renderer on its render thread
	platform::VirtualClock clock;
	platform::ManualTaskRunner host( clock );
	std::vector<double> samples;
	bool failed = false;
	{
		// The textured preview when VPKs are given, as the window does once
		// assets are mounted.
		std::unique_ptr<hammer::gtk::CatalogTextures> textures;
		std::unique_ptr<hammer::gtk::CatalogModels> models;
		if ( !vpkList.empty() )
		{
			std::string errors;
			textures = hammer::gtk::CatalogTextures::Open( vpkList, errors );
			models = hammer::gtk::CatalogModels::Open( vpkList, errors );
			if ( !textures || !models )
			{
				std::fprintf( stderr, "viewport-budget: no VPK mounted: %s\n", errors.c_str() );
				return 5;
			}
		}
		platform::ThreadTaskRunner render( "hammer-render" );
		hammer::render_adapter::ViewportService service(
		    *binding->device, render, host, std::move( textures ), std::move( models ) );
		const ViewKind kinds[] = {
		    ViewKind::Camera3D, ViewKind::Top, ViewKind::Front, ViewKind::Side };
		for ( int round = 0; round < warmup + edits && !failed; ++round )
		{
			const auto start = std::chrono::steady_clock::now();
			const double step = ( round % 2 ) ? -16.0 : 16.0; // back and forth
			if ( !ws.Commands().Execute(
			         "move_selection", { { "delta", std::to_string( step ) + " 0 0" } } ) )
			{
				std::fprintf( stderr, "viewport-budget: the edit failed\n" );
				failed = true;
				break;
			}
			auto scene = std::make_shared<const hammer::viewport::RenderSnapshot>( ws.Snapshot() );
			int arrived = 0;
			for ( ViewKind kind : kinds )
			{
				hammer::render_adapter::ViewJob job;
				job.kind = kind;
				if ( kind == ViewKind::Camera3D )
				{
					job.camera3D = ws.Camera3DView();
				}
				else
				{
					job.camera2D = ws.Camera2DFor( kind );
					job.grid = ws.GridLines( kind );
				}
				job.overlay = ws.Overlay( kind );
				job.pixelWidth = static_cast<std::uint32_t>( width );
				job.pixelHeight = static_cast<std::uint32_t>( height );
				job.external = external;
				if ( !service.Submit( scene, ws.SnapshotRevision(), std::move( job ),
				         [&arrived, &failed, &service, external](
				             hammer::render_adapter::ViewportService::Result result )
				         {
					         ++arrived;
					         failed = failed || !result || ( external && !result.Value().external );
					         // The window returns a dmabuf frame when GTK drops it;
					         // here it is shown at once.
					         if ( result && result.Value().external )
						         (void)service.ReturnFrame( result.Value().external->lease );
				         } ) )
				{
					failed = true;
				}
			}
			// The host sequence waits for its replies as the GTK loop would.
			const auto deadline = start + std::chrono::seconds( 10 );
			while ( arrived < 4 && !failed && std::chrono::steady_clock::now() < deadline )
			{
				host.RunUntilIdle();
				if ( arrived < 4 )
				{
					std::this_thread::sleep_for( std::chrono::microseconds( 50 ) );
				}
			}
			if ( arrived < 4 )
			{
				failed = true;
				break;
			}
			const double ms = std::chrono::duration<double, std::milli>(
			    std::chrono::steady_clock::now() - start )
			                      .count();
			if ( round >= warmup )
			{
				samples.push_back( ms );
			}
		}
	}
	if ( failed )
	{
		std::fprintf( stderr, "viewport-budget: a view did not render\n" );
		return 6;
	}
	FILE *file = std::fopen( outJson.c_str(), "w" );
	if ( !file )
	{
		std::fprintf( stderr, "viewport-budget: failed to write %s\n", outJson.c_str() );
		return 7;
	}
	std::fprintf( file,
	    "{\"schema\": \"hammer-viewport-samples/v1\", \"map\": \"%s\", \"width\": %d, "
	    "\"height\": %d, \"views\": 4, \"warmup_edits\": %d, \"solids\": %zu, "
	    "\"triangles_note\": \"edit-to-pixels in milliseconds\", \"samples_ms\": [",
	    vmfPath.c_str(), width, height, warmup, solids.size() );
	for ( std::size_t i = 0; i < samples.size(); ++i )
	{
		std::fprintf( file, "%s%.4f", i ? ", " : "", samples[i] );
	}
	std::fprintf( file, "]}\n" );
	std::fclose( file );
	std::printf( "viewport-budget: wrote %s (%zu samples)\n", outJson.c_str(), samples.size() );
	return 0;
}
