//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless offscreen render path for the GTK Hammer shell (RFC 0002).
//			Creates an OpenGL 3.3-core context with EGL (no window server needed),
//			renders a loaded VMF into an offscreen framebuffer with the SAME
//			hammergtk::Renderer the interactive viewport uses, and writes the result
//			as a binary PPM. This is what lets the 3D preview be verified
//			automatically: "same pixels, no window".
//
//=============================================================================//

#include "hammer/geometry/brush.h"

#include "renderer.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <epoxy/egl.h>
#include <epoxy/gl.h>

// The offscreen path is self-contained and free of GTK, but loads through the
// same editor core + geometry bridge the interactive viewport uses.
#include "hammer/app/editor_document.h"
#include "posix_file_store.h"

namespace
{

bool LoadSceneForScreenshot( const std::string &path, hammer::geometry::WorldScene &scene,
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

bool WritePpm( const std::string &path, int w, int h, const std::vector<std::uint8_t> &rgb )
{
	std::ofstream os( path, std::ios::binary );
	if ( !os )
	{
		return false;
	}
	os << "P6\n" << w << " " << h << "\n255\n";
	os.write( reinterpret_cast<const char *>( rgb.data() ),
	          static_cast<std::streamsize>( rgb.size() ) );
	return os.good();
}

} // namespace

// Renders 'vmfPath' to 'outPpm' at WxH. Returns 0 on success, non-zero on failure.
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
		std::fprintf( stderr, "screenshot: failed to load %s: %s\n", vmfPath.c_str(), error.c_str() );
		return 3;
	}

	// --- EGL surfaceless/pbuffer context -----------------------------------
	EGLDisplay dpy = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( dpy == EGL_NO_DISPLAY )
	{
		std::fprintf( stderr, "screenshot: no EGL display\n" );
		return 4;
	}
	EGLint major = 0;
	EGLint minor = 0;
	if ( !eglInitialize( dpy, &major, &minor ) )
	{
		std::fprintf( stderr, "screenshot: eglInitialize failed\n" );
		return 4;
	}

	const EGLint configAttribs[] = {
	    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	    EGL_RED_SIZE,     8,               EGL_GREEN_SIZE,      8,
	    EGL_BLUE_SIZE,    8,               EGL_ALPHA_SIZE,      8,
	    EGL_DEPTH_SIZE,   24,              EGL_NONE,
	};
	EGLConfig config;
	EGLint numConfigs = 0;
	if ( !eglChooseConfig( dpy, configAttribs, &config, 1, &numConfigs ) || numConfigs == 0 )
	{
		std::fprintf( stderr, "screenshot: no suitable EGL config\n" );
		eglTerminate( dpy );
		return 4;
	}

	const EGLint pbufferAttribs[] = { EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE };
	EGLSurface surface = eglCreatePbufferSurface( dpy, config, pbufferAttribs );
	if ( surface == EGL_NO_SURFACE )
	{
		std::fprintf( stderr, "screenshot: eglCreatePbufferSurface failed\n" );
		eglTerminate( dpy );
		return 4;
	}

	if ( !eglBindAPI( EGL_OPENGL_API ) )
	{
		std::fprintf( stderr, "screenshot: eglBindAPI(OpenGL) failed\n" );
		eglTerminate( dpy );
		return 4;
	}

	const EGLint contextAttribs[] = {
	    EGL_CONTEXT_MAJOR_VERSION,       3,
	    EGL_CONTEXT_MINOR_VERSION,       3,
	    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
	    EGL_NONE,
	};
	EGLContext ctx = eglCreateContext( dpy, config, EGL_NO_CONTEXT, contextAttribs );
	if ( ctx == EGL_NO_CONTEXT )
	{
		std::fprintf( stderr, "screenshot: eglCreateContext(3.3 core) failed\n" );
		eglDestroySurface( dpy, surface );
		eglTerminate( dpy );
		return 4;
	}

	if ( !eglMakeCurrent( dpy, surface, surface, ctx ) )
	{
		std::fprintf( stderr, "screenshot: eglMakeCurrent failed\n" );
		eglDestroyContext( dpy, ctx );
		eglDestroySurface( dpy, surface );
		eglTerminate( dpy );
		return 4;
	}

	// --- Offscreen FBO ------------------------------------------------------
	GLuint colorTex = 0;
	glGenTextures( 1, &colorTex );
	glBindTexture( GL_TEXTURE_2D, colorTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	GLuint depthRb = 0;
	glGenRenderbuffers( 1, &depthRb );
	glBindRenderbuffer( GL_RENDERBUFFER, depthRb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height );

	GLuint fbo = 0;
	glGenFramebuffers( 1, &fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0 );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb );

	if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		std::fprintf( stderr, "screenshot: framebuffer incomplete\n" );
		eglMakeCurrent( dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		eglDestroyContext( dpy, ctx );
		eglDestroySurface( dpy, surface );
		eglTerminate( dpy );
		return 5;
	}

	// --- Render with the shared renderer -----------------------------------
	{
		hammergtk::Renderer renderer;
		if ( !renderer.Init( error ) )
		{
			std::fprintf( stderr, "screenshot: renderer init failed: %s\n", error.c_str() );
			return 6;
		}
		renderer.SetScene( scene );
		renderer.Render( width, height );
		glFinish();

		std::vector<std::uint8_t> rgba( static_cast<std::size_t>( width ) * height * 4 );
		glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data() );

		// Flip vertically (GL origin is bottom-left) and drop alpha.
		std::vector<std::uint8_t> rgb( static_cast<std::size_t>( width ) * height * 3 );
		for ( int y = 0; y < height; ++y )
		{
			const int srcRow = height - 1 - y;
			for ( int x = 0; x < width; ++x )
			{
				const std::size_t s = ( static_cast<std::size_t>( srcRow ) * width + x ) * 4;
				const std::size_t d = ( static_cast<std::size_t>( y ) * width + x ) * 3;
				rgb[d + 0] = rgba[s + 0];
				rgb[d + 1] = rgba[s + 1];
				rgb[d + 2] = rgba[s + 2];
			}
		}

		const bool wrote = WritePpm( outPpm, width, height, rgb );
		if ( !wrote )
		{
			std::fprintf( stderr, "screenshot: failed to write %s\n", outPpm.c_str() );
		}
		else
		{
			std::printf( "screenshot: wrote %s (%dx%d), %d solids, %d triangles\n", outPpm.c_str(),
			             width, height, renderer.SolidCount(), renderer.TriangleCount() );
		}

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		glDeleteFramebuffers( 1, &fbo );
		glDeleteRenderbuffers( 1, &depthRb );
		glDeleteTextures( 1, &colorTex );

		if ( !wrote )
		{
			eglMakeCurrent( dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			eglDestroyContext( dpy, ctx );
			eglDestroySurface( dpy, surface );
			eglTerminate( dpy );
			return 7;
		}
	}

	eglMakeCurrent( dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroyContext( dpy, ctx );
	eglDestroySurface( dpy, surface );
	eglTerminate( dpy );
	return 0;
}
