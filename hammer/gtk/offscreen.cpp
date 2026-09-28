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
//			simulated editor input (--demo). --textured waits for the core's
//			material families (RFC 0016 K4) and fails saying so.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/adapters/render/viewport_renderer.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/presenters/editor_workspace.h"
#include "render/composition/render_core.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
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
	    hammer::presenters::WorkspaceServices{ &codec, &store, nullptr, nullptr, nullptr } };

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

	bool Compose( const char *tag )
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
		auto made = hammer::render_adapter::ViewportRenderer::Create( *binding->device );
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
int RenderTexturedScreenshot(
    const std::string &, const std::string &, int, int, const std::string & )
{
	std::fprintf( stderr, "textured: textured previews wait for the render core's material "
	                      "families (RFC 0016 K4)\n" );
	return 8;
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
