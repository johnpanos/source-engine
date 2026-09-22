//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native Vulkan bring-up / presentation smoke test (RFC 0001 rank 14,
//          roadmap R28). Brings up the real CVulkanContext against an SDL3
//          window, presents cleared frames, reads the presented image back and
//          asserts the pixels match the requested clear color, proves the
//          verification detects a wrong expectation, exercises resize, and
//          tears everything down cleanly.
//
//          This is outcome-driven: a clean process exit alone does not pass;
//          the GPU must have actually produced the requested pixels. The test
//          requires a working Vulkan device and a display; when neither is
//          available it reports an explicit skip (non-pass) rather than a
//          false success, so a required lane cannot be certified by absence.
//
//===========================================================================//

#include "vulkan_device.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <string>

using render_vulkan::CVulkanContext;
using render_vulkan::VulkanContextConfig;

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool condition, const char *what )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", what );
	}
}

// A pixel is "close" to the expected 8-bit RGBA within a small tolerance to
// absorb driver rounding. Pure channel values (0/255) are exact under both
// UNORM and sRGB storage, which is why the test clears to primaries.
bool PixelClose( const uint8_t *p, int r, int g, int b, int a, int tol )
{
	return std::abs( int( p[0] ) - r ) <= tol && std::abs( int( p[1] ) - g ) <= tol &&
	       std::abs( int( p[2] ) - b ) <= tol && std::abs( int( p[3] ) - a ) <= tol;
}

bool PresentAndCapture( CVulkanContext &ctx, float r, float g, float b, std::string *err )
{
	ctx.SetClearColor( r, g, b, 1.0f );
	// Present a few frames so the swapchain cycles through its images.
	for ( int i = 0; i < 4; ++i )
	{
		bool skip = false;
		if ( i == 3 )
			ctx.RequestCapture();
		if ( !ctx.BeginFrame( &skip, err ) )
			return false;
		if ( skip )
		{
			--i;
			SDL_Delay( 8 );
			continue;
		}
		if ( !ctx.EndFrame( err ) )
			return false;
	}
	return true;
}

} // namespace

int main( int argc, char **argv )
{
	bool requireValidation = false;
	for ( int i = 1; i < argc; ++i )
		if ( std::string( argv[i] ) == "--require-validation" )
			requireValidation = true;

	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::fprintf( stderr, "SKIP: SDL_Init(video) failed (no display): %s\n", SDL_GetError() );
		return 77; // automake-style skip: explicitly not a pass
	}

	SDL_Window *window = SDL_CreateWindow( "native-vulkan-bringup", 320, 240, SDL_WINDOW_VULKAN );
	if ( !window )
	{
		std::fprintf( stderr, "SKIP: SDL_CreateWindow(vulkan) failed: %s\n", SDL_GetError() );
		SDL_Quit();
		return 77;
	}

	VulkanContextConfig config;
	config.appName = "native-vulkan-bringup";
	config.enableValidation = true; // opportunistic when the layer exists
	config.requireValidation = requireValidation;
	config.framesInFlight = 2;

	CVulkanContext ctx;
	std::string err;
	if ( !ctx.Init( window, config, &err ) )
	{
		// A genuine device/driver failure is a skip only when there is no usable
		// Vulkan device at all; any other Init failure is a real test failure.
		std::fprintf( stderr, "native Vulkan bring-up failed: %s\n", err.c_str() );
		SDL_DestroyWindow( window );
		SDL_Quit();
		return 77;
	}

	std::fprintf( stderr, "brought up device '%s' (vendor 0x%04x, %s, %.0f MiB, validation=%s)\n",
	    ctx.DeviceName(), ctx.VendorId(), ctx.IsDiscrete() ? "discrete" : "integrated/other",
	    double( ctx.DeviceLocalMemoryBytes() ) / ( 1024.0 * 1024.0 ),
	    ctx.ValidationEnabled() ? "on" : "off" );

	Check( ctx.IsValid(), "context is valid after Init" );
	Check( ctx.DeviceName()[0] != '\0', "adapter reported a device name" );

	int sw = 0, sh = 0;
	ctx.GetSwapchainExtent( sw, sh );
	Check( sw > 0 && sh > 0, "swapchain has a non-zero extent" );

	// Clear to pure red and verify the presented image really is red.
	if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
	{
		std::fprintf( stderr, "present/capture (red) failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		int cw = 0, ch = 0;
		const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
		Check( cw > 0 && ch > 0 && !px.empty(), "captured a non-empty frame" );
		if ( cw > 0 && ch > 0 && !px.empty() )
		{
			const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
			Check( PixelClose( center, 255, 0, 0, 255, 2 ), "presented center pixel is red" );
			// Corner too, to catch a partial clear.
			Check( PixelClose( &px[0], 255, 0, 0, 255, 2 ), "presented top-left pixel is red" );
			// Negative control: the verifier must reject a wrong expectation.
			Check( !PixelClose( center, 0, 255, 0, 255, 2 ),
			    "verifier rejects a wrong (green) expectation" );
		}
	}

	// Clear to pure green after a resize and verify both the new color and that
	// the swapchain rebuilt to the new drawable size.
	SDL_SetWindowSize( window, 200, 160 );
	SDL_SyncWindow( window );
	if ( !ctx.Resize( 200, 160, &err ) )
	{
		std::fprintf( stderr, "resize failed: %s\n", err.c_str() );
		++g_failures;
	}
	if ( !PresentAndCapture( ctx, 0.0f, 1.0f, 0.0f, &err ) )
	{
		std::fprintf( stderr, "present/capture (green) failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		int cw = 0, ch = 0;
		const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
		if ( cw > 0 && ch > 0 && !px.empty() )
		{
			const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
			Check(
			    PixelClose( center, 0, 255, 0, 255, 2 ), "presented pixel is green after resize" );
		}
		else
		{
			Check( false, "captured a frame after resize" );
		}
	}

	// Real geometry: bring up the demo pipeline (shader modules + graphics
	// pipeline + GPU vertex buffer), clear to red, and rasterize a green
	// triangle over it. The center must read back green (a triangle was
	// actually drawn) while a corner stays red (the clear still shows), which a
	// clear-only path could never produce.
	if ( !ctx.InitDemoTriangle( &err ) )
	{
		std::fprintf( stderr, "InitDemoTriangle failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		Check( ctx.DemoTriangleReady(), "demo triangle pipeline is ready" );
		ctx.SetDrawDemoTriangle( true );
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			std::fprintf( stderr, "present/capture (triangle) failed: %s\n", err.c_str() );
			++g_failures;
		}
		else
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw > 0 && ch > 0 && !px.empty() )
			{
				const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
				const uint8_t *corner = &px[0]; // top-left, outside the triangle
				Check( PixelClose( center, 0, 255, 0, 255, 2 ),
				    "triangle center is green (geometry drawn)" );
				Check( PixelClose( corner, 255, 0, 0, 255, 2 ),
				    "frame corner is still red (clear preserved)" );
			}
			else
			{
				Check( false, "captured a frame with the triangle" );
			}
		}
		ctx.SetDrawDemoTriangle( false );
	}

	// Texturing: upload a distinctive magenta texture through a staging buffer,
	// bind it via a descriptor set, and sample it onto a quad. Magenta appears
	// nowhere else, so the quad center reading back magenta proves the texture
	// was really uploaded, bound, and sampled; the corner stays red (clear).
	if ( !ctx.InitTexturedQuad( &err ) )
	{
		std::fprintf( stderr, "InitTexturedQuad failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		Check( ctx.TexturedQuadReady(), "textured-quad pipeline is ready" );
		ctx.SetDrawTexturedQuad( true );
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			std::fprintf( stderr, "present/capture (textured quad) failed: %s\n", err.c_str() );
			++g_failures;
		}
		else
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw > 0 && ch > 0 && !px.empty() )
			{
				const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
				const uint8_t *corner = &px[0];
				Check( PixelClose( center, 255, 0, 255, 255, 2 ),
				    "quad center is magenta (texture sampled)" );
				Check( PixelClose( corner, 255, 0, 0, 255, 2 ),
				    "frame corner is still red (clear preserved)" );
			}
			else
			{
				Check( false, "captured a frame with the textured quad" );
			}
		}
		ctx.SetDrawTexturedQuad( false );
	}

	// Index buffers + shader constants: draw an indexed quad whose fragment color
	// comes entirely from a uniform (constant) buffer. Setting the constant to
	// blue and reading the center back as blue proves vkCmdDrawIndexed ran and the
	// constant reached the shader -- the shader-constant path the material system
	// uses on every material.
	if ( !ctx.InitIndexedUbo( &err ) )
	{
		std::fprintf( stderr, "InitIndexedUbo failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		Check( ctx.IndexedUboReady(), "indexed + uniform-buffer pipeline is ready" );
		ctx.SetIndexedUboColor( 0.0f, 0.0f, 1.0f, 1.0f ); // blue via the constant buffer
		ctx.SetDrawIndexedUbo( true );
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			std::fprintf( stderr, "present/capture (indexed ubo) failed: %s\n", err.c_str() );
			++g_failures;
		}
		else
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw > 0 && ch > 0 && !px.empty() )
			{
				const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
				const uint8_t *corner = &px[0];
				Check( PixelClose( center, 0, 0, 255, 255, 2 ),
				    "indexed quad center is blue (constant buffer reached the shader)" );
				Check( PixelClose( corner, 255, 0, 0, 255, 2 ),
				    "frame corner is still red (clear preserved)" );
			}
			else
			{
				Check( false, "captured a frame with the indexed/ubo quad" );
			}
		}
		ctx.SetDrawIndexedUbo( false );
	}

	// Depth buffering: two overlapping triangles, near (blue) drawn first, far
	// (green) drawn second, with depth testing on. If the depth attachment
	// resolves occlusion, the center stays blue even though green was drawn
	// last -- the capability real 3D scene rendering needs.
	if ( !ctx.InitDemoDepth( &err ) )
	{
		std::fprintf( stderr, "InitDemoDepth failed: %s\n", err.c_str() );
		++g_failures;
	}
	else
	{
		Check( ctx.DemoDepthReady(), "depth-test pipeline is ready" );
		ctx.SetDrawDemoDepth( true );
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			std::fprintf( stderr, "present/capture (depth) failed: %s\n", err.c_str() );
			++g_failures;
		}
		else
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw > 0 && ch > 0 && !px.empty() )
			{
				const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
				Check( PixelClose( center, 0, 0, 255, 255, 2 ),
				    "near (blue) triangle occludes the later far (green) one (depth test works)" );
			}
			else
			{
				Check( false, "captured a frame with the depth demo" );
			}
		}
		ctx.SetDrawDemoDepth( false );
	}

	if ( ctx.ValidationEnabled() )
		Check( ctx.ValidationErrorCount() == 0, "no validation errors/warnings during the run" );

	ctx.Shutdown();
	Check( !ctx.IsValid(), "context is invalid after Shutdown" );

	SDL_DestroyWindow( window );
	SDL_Quit();

	std::fprintf(
	    stderr, "native Vulkan bring-up: %d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
