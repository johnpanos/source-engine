//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless offscreen render path for the GTK Hammer shell (RFC 0002).
//			Creates an OpenGL 3.3-core context with EGL (no window server needed),
//			opens a map through the SAME hammer::presenters::EditorWorkspace the
//			window uses (strict VMF codec, render snapshot, framed cameras), draws
//			it with the SAME hammergtk::Renderer, and writes a binary PPM. Forms:
//			a single 3D view (--screenshot), the classic 2x2 quad of camera/top/
//			front/side views (--quad), a textured 3D view (--textured), and a quad
//			of a map built by simulated editor input (--demo). This is what lets
//			the viewports be verified automatically: "same pixels, no window".
//
//=============================================================================//

#include "renderer.h"

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/adapters/platform/disk_file_store.h"
#include "hammer/formats/material_catalog.h"
#include "hammer/formats/search_path_assets.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/formats/vpk_archive.h"
#include "hammer/presenters/editor_workspace.h"
#ifdef HAMMER_KTX_PREVIEW
#include "hammer/adapters/source/ktx2_preview.h"
#endif

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <epoxy/egl.h>
#include <epoxy/gl.h>

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

// EGL state for an offscreen OpenGL 3.3-core context.
struct EglContext
{
	EGLDisplay dpy = EGL_NO_DISPLAY;
	EGLSurface surface = EGL_NO_SURFACE;
	EGLContext ctx = EGL_NO_CONTEXT;
	bool ok = false;
};

EglContext CreateEgl( int width, int height )
{
	EglContext e;
	e.dpy = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( e.dpy == EGL_NO_DISPLAY )
	{
		return e;
	}
	EGLint major = 0;
	EGLint minor = 0;
	if ( !eglInitialize( e.dpy, &major, &minor ) )
	{
		return e;
	}
	const EGLint configAttribs[] = {
	    EGL_SURFACE_TYPE,
	    EGL_PBUFFER_BIT,
	    EGL_RENDERABLE_TYPE,
	    EGL_OPENGL_BIT,
	    EGL_RED_SIZE,
	    8,
	    EGL_GREEN_SIZE,
	    8,
	    EGL_BLUE_SIZE,
	    8,
	    EGL_ALPHA_SIZE,
	    8,
	    EGL_DEPTH_SIZE,
	    24,
	    EGL_NONE,
	};
	EGLConfig config;
	EGLint numConfigs = 0;
	if ( !eglChooseConfig( e.dpy, configAttribs, &config, 1, &numConfigs ) || numConfigs == 0 )
	{
		return e;
	}
	const EGLint pbufferAttribs[] = { EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE };
	e.surface = eglCreatePbufferSurface( e.dpy, config, pbufferAttribs );
	if ( e.surface == EGL_NO_SURFACE || !eglBindAPI( EGL_OPENGL_API ) )
	{
		return e;
	}
	const EGLint contextAttribs[] = {
	    EGL_CONTEXT_MAJOR_VERSION,
	    3,
	    EGL_CONTEXT_MINOR_VERSION,
	    3,
	    EGL_CONTEXT_OPENGL_PROFILE_MASK,
	    EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
	    EGL_NONE,
	};
	e.ctx = eglCreateContext( e.dpy, config, EGL_NO_CONTEXT, contextAttribs );
	if ( e.ctx == EGL_NO_CONTEXT )
	{
		return e;
	}
	e.ok = eglMakeCurrent( e.dpy, e.surface, e.surface, e.ctx );
	return e;
}

void DestroyEgl( EglContext &e )
{
	if ( e.dpy != EGL_NO_DISPLAY )
	{
		eglMakeCurrent( e.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		if ( e.ctx != EGL_NO_CONTEXT )
		{
			eglDestroyContext( e.dpy, e.ctx );
		}
		if ( e.surface != EGL_NO_SURFACE )
		{
			eglDestroySurface( e.dpy, e.surface );
		}
		eglTerminate( e.dpy );
	}
}

// Creates a color+depth FBO of WxH and leaves it bound. Returns the object names.
bool CreateFbo( int w, int h, GLuint &fbo, GLuint &colorTex, GLuint &depthRb )
{
	glGenTextures( 1, &colorTex );
	glBindTexture( GL_TEXTURE_2D, colorTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glGenRenderbuffers( 1, &depthRb );
	glBindRenderbuffer( GL_RENDERBUFFER, depthRb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h );

	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0 );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb );
	return glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE;
}

// Reads the bound framebuffer as top-down RGB (GL origin is bottom-left).
std::vector<std::uint8_t> ReadRgbTopDown( int w, int h )
{
	std::vector<std::uint8_t> rgba( static_cast<std::size_t>( w ) * h * 4 );
	glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data() );
	std::vector<std::uint8_t> rgb( static_cast<std::size_t>( w ) * h * 3 );
	for ( int y = 0; y < h; ++y )
	{
		const int srcRow = h - 1 - y;
		for ( int x = 0; x < w; ++x )
		{
			const std::size_t s = ( static_cast<std::size_t>( srcRow ) * w + x ) * 4;
			const std::size_t d = ( static_cast<std::size_t>( y ) * w + x ) * 3;
			rgb[d + 0] = rgba[s + 0];
			rgb[d + 1] = rgba[s + 1];
			rgb[d + 2] = rgba[s + 2];
		}
	}
	return rgb;
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

// Draws one view of the workspace into the bound framebuffer and reads it back.
std::vector<std::uint8_t> RenderView( hammergtk::Renderer &renderer,
    hammer::presenters::EditorWorkspace &ws, ViewKind kind, int width, int height )
{
	const hammer::tools::OverlayList overlay = ws.Overlay( kind );
	if ( kind == ViewKind::Camera3D )
	{
		renderer.Render3D( ws.Camera3DView(), overlay, width, height );
	}
	else
	{
		renderer.Render2D( ws.Camera2DFor( kind ), ws.GridLines( kind ), overlay, width, height );
	}
	glFinish();
	return ReadRgbTopDown( width, height );
}

// An EGL context with a bound WxH framebuffer, released on scope exit.
struct OffscreenTarget
{
	EglContext egl;
	GLuint fbo = 0;
	GLuint colorTex = 0;
	GLuint depthRb = 0;
	int status = 0; // 0 = ready, else the exit code

	OffscreenTarget( int width, int height, const char *tag )
	{
		egl = CreateEgl( width, height );
		if ( !egl.ok )
		{
			std::fprintf( stderr, "%s: EGL context creation failed\n", tag );
			status = 4;
			return;
		}
		if ( !CreateFbo( width, height, fbo, colorTex, depthRb ) )
		{
			std::fprintf( stderr, "%s: framebuffer incomplete\n", tag );
			status = 5;
		}
	}
	~OffscreenTarget()
	{
		if ( egl.ok )
		{
			glDeleteFramebuffers( 1, &fbo );
			glDeleteRenderbuffers( 1, &depthRb );
			glDeleteTextures( 1, &colorTex );
		}
		DestroyEgl( egl );
	}
};

// Renders the workspace's 3D view to 'outPpm'.
int RenderSingle( hammer::presenters::EditorWorkspace &ws,
    hammer::formats::MaterialCatalog *catalog, const std::string &outPpm, int width, int height,
    const char *tag )
{
	OffscreenTarget target( width, height, tag );
	if ( target.status )
	{
		return target.status;
	}
	hammergtk::Renderer renderer;
	std::string error;
	if ( !renderer.Init( error ) )
	{
		std::fprintf( stderr, "%s: renderer init failed: %s\n", tag, error.c_str() );
		return 6;
	}
	renderer.SetMaterialCatalog( catalog );
	renderer.SetSnapshot( ws.Snapshot() );
	const std::vector<std::uint8_t> rgb =
	    RenderView( renderer, ws, ViewKind::Camera3D, width, height );
	if ( !WritePpm( outPpm, width, height, rgb ) )
	{
		std::fprintf( stderr, "%s: failed to write %s\n", tag, outPpm.c_str() );
		return 7;
	}
	std::printf( "%s: wrote %s (%dx%d), %d solids, %d triangles, %d entities\n", tag,
	    outPpm.c_str(), width, height, renderer.SolidCount(), renderer.TriangleCount(),
	    renderer.EntityCount() );
	return 0;
}

// Renders the classic 2x2 quad (camera / top / front / side) to a PPM: camera
// top-left, top(X/Y) top-right, front(X/Z) bottom-left, side(Y/Z) bottom-right,
// matching the on-screen layout.
int RenderQuadOf( hammer::presenters::EditorWorkspace &ws, const std::string &outPpm, int tileW,
    int tileH, const char *tag )
{
	OffscreenTarget target( tileW, tileH, tag );
	if ( target.status )
	{
		return target.status;
	}
	hammergtk::Renderer renderer;
	std::string error;
	if ( !renderer.Init( error ) )
	{
		std::fprintf( stderr, "%s: renderer init failed: %s\n", tag, error.c_str() );
		return 6;
	}
	renderer.SetSnapshot( ws.Snapshot() );
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
		BlitTile( composite, fullW, RenderView( renderer, ws, t.kind, tileW, tileH ), tileW, tileH,
		    t.tx, t.ty );
	}
	if ( !WritePpm( outPpm, fullW, fullH, composite ) )
	{
		std::fprintf( stderr, "%s: failed to write %s\n", tag, outPpm.c_str() );
		return 7;
	}
	std::printf( "%s: wrote %s (%dx%d), %d solids, %d entities\n", tag, outPpm.c_str(), fullW,
	    fullH, renderer.SolidCount(), renderer.EntityCount() );
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
	return RenderSingle( editor.workspace, nullptr, outPpm, width, height, "screenshot" );
}

// Renders a single 3D view of a VMF with materials mounted from one or more VPK
// archives, so the screenshot shows real Source textures. 'vpkList' is a
// comma-separated list of _dir.vpk paths (later ones are lower priority). This is
// the headless evidence path for texture loading reaching the frontend renderer.
int RenderTexturedScreenshot( const std::string &vmfPath, const std::string &outPpm, int width,
    int height, const std::string &vpkList )
{
	if ( width <= 0 || height <= 0 )
	{
		std::fprintf( stderr, "textured: invalid size %dx%d\n", width, height );
		return 2;
	}
	Editor editor;
	if ( !editor.Open( vmfPath, width, height, "textured" ) )
	{
		return 3;
	}

	// Mount the VPKs into an ordered asset search path, then a material catalog.
	hammer::adapters::platform::DiskByteStore byteStore;
	hammer::formats::SearchPathAssets assets;
	std::vector<std::unique_ptr<hammer::formats::VpkArchive>> archives;
	std::size_t start = 0;
	while ( start <= vpkList.size() )
	{
		std::size_t comma = vpkList.find( ',', start );
		std::string path =
		    vpkList.substr( start, comma == std::string::npos ? std::string::npos : comma - start );
		if ( !path.empty() )
		{
			std::string err;
			auto vpk = hammer::formats::VpkArchive::Open( byteStore, path, err );
			if ( !vpk )
			{
				std::fprintf(
				    stderr, "textured: cannot mount %s: %s\n", path.c_str(), err.c_str() );
				return 3;
			}
			std::printf(
			    "textured: mounted %s (%zu entries)\n", path.c_str(), vpk->Entries().size() );
			assets.AddProvider( vpk.get() );
			archives.push_back( std::move( vpk ) );
		}
		if ( comma == std::string::npos )
			break;
		start = comma + 1;
	}
#ifdef HAMMER_KTX_PREVIEW
	hammer::formats::MaterialCatalog catalog(
	    assets, &hammer::adapters::source::DecodeKtx2Preview );
#else
	hammer::formats::MaterialCatalog catalog( assets );
#endif
	return RenderSingle( editor.workspace, &catalog, outPpm, width, height, "textured" );
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
