//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless offscreen render path for the GTK Hammer shell (RFC 0002).
//			Creates an OpenGL 3.3-core context with EGL (no window server needed),
//			renders a loaded VMF into an offscreen framebuffer with the SAME
//			hammergtk::Renderer the interactive viewports use, and writes a binary
//			PPM. Two forms: a single 3D view (--screenshot) and the classic 2x2
//			quad of camera/top/front/side views (--quad). This is what lets the
//			viewports be verified automatically: "same pixels, no window".
//
//=============================================================================//

#include "hammer/geometry/brush.h"

#include "renderer.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <epoxy/egl.h>
#include <epoxy/gl.h>

#include "hammer/app/editor_document.h"
#include "posix_file_store.h"

namespace
{

bool LoadSceneForScreenshot(
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

} // namespace

int RenderScreenshot( const std::string &vmfPath, const std::string &outPpm, int width, int height )
{
	if ( width <= 0 || height <= 0 )
	{
		std::fprintf( stderr, "screenshot: invalid size %dx%d\n", width, height );
		return 2;
	}
	hammer::geometry::WorldScene scene;
	std::string error;
	if ( !LoadSceneForScreenshot( vmfPath, scene, error ) )
	{
		std::fprintf(
		    stderr, "screenshot: failed to load %s: %s\n", vmfPath.c_str(), error.c_str() );
		return 3;
	}

	EglContext egl = CreateEgl( width, height );
	if ( !egl.ok )
	{
		std::fprintf( stderr, "screenshot: EGL context creation failed\n" );
		DestroyEgl( egl );
		return 4;
	}

	GLuint fbo = 0;
	GLuint colorTex = 0;
	GLuint depthRb = 0;
	if ( !CreateFbo( width, height, fbo, colorTex, depthRb ) )
	{
		std::fprintf( stderr, "screenshot: framebuffer incomplete\n" );
		DestroyEgl( egl );
		return 5;
	}

	int rc = 0;
	{
		hammergtk::Renderer renderer;
		if ( !renderer.Init( error ) )
		{
			std::fprintf( stderr, "screenshot: renderer init failed: %s\n", error.c_str() );
			rc = 6;
		}
		else
		{
			renderer.SetScene( scene );
			renderer.Render( width, height );
			glFinish();
			const std::vector<std::uint8_t> rgb = ReadRgbTopDown( width, height );
			if ( !WritePpm( outPpm, width, height, rgb ) )
			{
				std::fprintf( stderr, "screenshot: failed to write %s\n", outPpm.c_str() );
				rc = 7;
			}
			else
			{
				std::printf( "screenshot: wrote %s (%dx%d), %d solids, %d triangles\n",
				    outPpm.c_str(), width, height, renderer.SolidCount(),
				    renderer.TriangleCount() );
			}
		}
	}

	glDeleteFramebuffers( 1, &fbo );
	glDeleteRenderbuffers( 1, &depthRb );
	glDeleteTextures( 1, &colorTex );
	DestroyEgl( egl );
	return rc;
}

// Renders the classic 2x2 viewport quad (camera / top / front / side) to a single
// PPM: camera top-left, top(X/Y) top-right, front(X/Z) bottom-left, side(Y/Z)
// bottom-right, matching the on-screen layout.
int RenderQuad( const std::string &vmfPath, const std::string &outPpm, int tileW, int tileH )
{
	if ( tileW <= 0 || tileH <= 0 )
	{
		std::fprintf( stderr, "quad: invalid size %dx%d\n", tileW, tileH );
		return 2;
	}
	hammer::geometry::WorldScene scene;
	std::string error;
	if ( !LoadSceneForScreenshot( vmfPath, scene, error ) )
	{
		std::fprintf( stderr, "quad: failed to load %s: %s\n", vmfPath.c_str(), error.c_str() );
		return 3;
	}

	EglContext egl = CreateEgl( tileW, tileH );
	if ( !egl.ok )
	{
		std::fprintf( stderr, "quad: EGL context creation failed\n" );
		DestroyEgl( egl );
		return 4;
	}

	GLuint fbo = 0;
	GLuint colorTex = 0;
	GLuint depthRb = 0;
	if ( !CreateFbo( tileW, tileH, fbo, colorTex, depthRb ) )
	{
		std::fprintf( stderr, "quad: framebuffer incomplete\n" );
		DestroyEgl( egl );
		return 5;
	}

	int rc = 0;
	{
		hammergtk::Renderer renderer;
		if ( !renderer.Init( error ) )
		{
			std::fprintf( stderr, "quad: renderer init failed: %s\n", error.c_str() );
			rc = 6;
		}
		else
		{
			renderer.SetScene( scene );

			const int fullW = tileW * 2;
			const int fullH = tileH * 2;
			std::vector<std::uint8_t> composite(
			    static_cast<std::size_t>( fullW ) * fullH * 3, 20 );

			struct Tile
			{
				hammergtk::ViewMode mode;
				int tx;
				int ty;
			};
			const Tile tiles[] = {
			    { hammergtk::ViewMode::Perspective, 0, 0 },
			    { hammergtk::ViewMode::Top, 1, 0 },
			    { hammergtk::ViewMode::Front, 0, 1 },
			    { hammergtk::ViewMode::Side, 1, 1 },
			};
			for ( const Tile &t : tiles )
			{
				renderer.SetViewMode( t.mode );
				renderer.FrameScene();
				renderer.Render( tileW, tileH );
				glFinish();
				const std::vector<std::uint8_t> tile = ReadRgbTopDown( tileW, tileH );
				BlitTile( composite, fullW, tile, tileW, tileH, t.tx, t.ty );
			}

			if ( !WritePpm( outPpm, fullW, fullH, composite ) )
			{
				std::fprintf( stderr, "quad: failed to write %s\n", outPpm.c_str() );
				rc = 7;
			}
			else
			{
				std::printf( "quad: wrote %s (%dx%d), %d solids\n", outPpm.c_str(), fullW, fullH,
				    renderer.SolidCount() );
			}
		}
	}

	glDeleteFramebuffers( 1, &fbo );
	glDeleteRenderbuffers( 1, &depthRb );
	glDeleteTextures( 1, &colorTex );
	DestroyEgl( egl );
	return rc;
}
