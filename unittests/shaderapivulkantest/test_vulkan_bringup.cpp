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
#include "sdl3/sdl3_vulkan_surface_host.h"
#include "render/render_gamma_ramp.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

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

bool PresentAndCapture(
    CVulkanContext &ctx, float r, float g, float b, std::string *err, bool presented = false )
{
	ctx.SetClearColor( r, g, b, 1.0f );
	// Present a few frames so the swapchain cycles through its images.
	for ( int i = 0; i < 4; ++i )
	{
		bool skip = false;
		if ( i == 3 )
		{
			if ( presented )
				ctx.RequestPresentedCapture();
			else
				ctx.RequestCapture();
		}
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

	std::string err;
	std::unique_ptr<render_vulkan::IVulkanSurfaceHost> host =
	    render_vulkan::MakeSdl3LegacySurfaceHost( window, &err );
	CVulkanContext ctx;
	if ( !host || !ctx.Init( *host, config, &err ) )
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
	// Exercise map-scoped WMSH buffer ownership on a real device. A failed
	// replacement must leave the previous pair resident, while explicit release
	// and context shutdown both retire them after GPU work completes.
	uint8_t worldVertices[3 * 40] = {};
	const uint32_t worldIndices[3] = { 0, 1, 2 };
	for ( size_t i = 0; i < sizeof( worldVertices ); ++i )
		worldVertices[i] = static_cast<uint8_t>( i * 37 + 11 );
	uint8_t readVertices[sizeof( worldVertices )] = {};
	uint32_t readIndices[3] = {};
	Check( !ctx.WorldMeshResident(), "world mesh starts absent" );
	Check( !ctx.UploadWorldMesh(
	           nullptr, sizeof( worldVertices ), worldIndices, sizeof( worldIndices ), &err ),
	    "world mesh rejects a missing vertex section" );
	Check( !ctx.WorldMeshResident(), "failed first upload leaves no world mesh" );
	Check( ctx.UploadWorldMesh(
	           worldVertices, sizeof( worldVertices ), worldIndices, sizeof( worldIndices ), &err ),
	    "world mesh uploads device-local vertex and index buffers" );
	Check( ctx.WorldMeshResident(), "world mesh pair is resident after upload" );
	Check( ctx.ReadWorldMeshBytes(
	           readVertices, sizeof( readVertices ), readIndices, sizeof( readIndices ), &err ),
	    "world mesh device buffers read back" );
	Check( std::memcmp( readVertices, worldVertices, sizeof( readVertices ) ) == 0 &&
	           std::memcmp( readIndices, worldIndices, sizeof( readIndices ) ) == 0,
	    "world mesh GPU bytes match both authored sections" );
	Check( readVertices[0] != 0, "readback oracle rejects a zero-filled vertex buffer" );
	worldVertices[0] = 1;
	Check( ctx.UploadWorldMesh(
	           worldVertices, sizeof( worldVertices ), worldIndices, sizeof( worldIndices ), &err ),
	    "world mesh replacement uploads after prior GPU work" );
	Check( !ctx.UploadWorldMesh(
	           worldVertices, sizeof( worldVertices ), nullptr, sizeof( worldIndices ), &err ),
	    "world mesh rejects a missing index section" );
	Check( ctx.WorldMeshResident(), "failed replacement retains the prior world mesh" );
	Check( ctx.ReadWorldMeshBytes(
	           readVertices, sizeof( readVertices ), readIndices, sizeof( readIndices ), &err ) &&
	           std::memcmp( readVertices, worldVertices, sizeof( readVertices ) ) == 0,
	    "replacement bytes survive a failed upload" );

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
	Check( ctx.WorldMeshResident(), "world mesh survives presentation resize" );
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

		// A back buffer of the video mode's size, smaller than the window (D3D9's
		// BackBufferWidth/Height), is presented scaled over the whole drawable.
		// The clear is blue now: a back buffer copied 1:1 into the top-left would
		// leave the window's far corner holding an earlier frame's red.
		int dw = 0, dh = 0;
		ctx.GetPresentExtent( dw, dh );
		if ( !ctx.SetBackBufferSize( dw / 2, dh / 2, &err ) )
		{
			std::fprintf( stderr, "SetBackBufferSize failed: %s\n", err.c_str() );
			++g_failures;
		}
		int bw = 0, bh = 0;
		ctx.GetSwapchainExtent( bw, bh );
		Check( bw == dw / 2 && bh == dh / 2, "back buffer takes the requested mode size" );
		if ( PresentAndCapture( ctx, 0.0f, 0.0f, 1.0f, &err, true ) )
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw == 0 && ch == 0 )
				std::fprintf( stderr, "note: this surface cannot read its presented images\n" );
			else
			{
				Check( cw == dw && ch == dh, "presented image covers the window's drawable" );
				if ( cw == dw && ch == dh && !px.empty() )
				{
					const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
					const uint8_t *far = &px[( size_t( ch - 2 ) * cw + ( cw - 2 ) ) * 4];
					Check( PixelClose( center, 0, 255, 0, 255, 2 ),
					    "scaled present: the triangle is at the window's center" );
					Check( PixelClose( far, 0, 0, 255, 255, 2 ),
					    "scaled present: the window's far corner is the blue clear" );
				}
			}
		}
		else
		{
			std::fprintf( stderr, "present/capture (scaled) failed: %s\n", err.c_str() );
			++g_failures;
		}
		if ( !ctx.SetBackBufferSize( 0, 0, &err ) )
			++g_failures;
		ctx.GetSwapchainExtent( bw, bh );
		Check( bw == dw && bh == dh, "a 0 x 0 back buffer follows the drawable again" );
		ctx.SetDrawDemoTriangle( false );
	}

	// Vsync (render.present-policy.v1): the request applies at the next frame
	// with exactly one swapchain rebuild. Without vsync the mode is immediate
	// when the surface offers it, then mailbox, then FIFO; with vsync it is FIFO.
	{
		const std::vector<VkPresentModeKHR> &offered = ctx.SurfacePresentModes();
		const auto offers = [&]( VkPresentModeKHR mode ) {
			return std::find( offered.begin(), offered.end(), mode ) != offered.end();
		};
		std::fprintf( stderr, "surface present modes:" );
		for ( VkPresentModeKHR mode : offered )
			std::fprintf( stderr, " %d", static_cast<int>( mode ) );
		std::fprintf( stderr, "\n" );
		Check( offers( VK_PRESENT_MODE_FIFO_KHR ), "the surface offers FIFO (Vulkan guarantee)" );
		Check( ctx.VSyncRequested() && ctx.PresentMode() == VK_PRESENT_MODE_FIFO_KHR,
		    "the default request is vsync, presented FIFO" );

		const VkPresentModeKHR expectedOff = offers( VK_PRESENT_MODE_IMMEDIATE_KHR )
		                                         ? VK_PRESENT_MODE_IMMEDIATE_KHR
		                                     : offers( VK_PRESENT_MODE_MAILBOX_KHR )
		                                         ? VK_PRESENT_MODE_MAILBOX_KHR
		                                         : VK_PRESENT_MODE_FIFO_KHR;
		const uint64_t generation = ctx.SwapchainGeneration();
		ctx.RequestVSync( false );
		Check( ctx.SwapchainGeneration() == generation, "a vsync request waits for the next frame" );
		if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			Check( ctx.SwapchainGeneration() == generation + 1,
			    "vsync off rebuilt the swapchain exactly once" );
			Check( ctx.PresentMode() == expectedOff, "vsync off selects the policy's present mode" );
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			Check( cw > 0 && ch > 0 && !px.empty() &&
			           PixelClose( &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4], 255, 0, 0, 255, 2 ),
			    "frames render after the present-mode change" );
		}
		else
		{
			std::fprintf( stderr, "present (vsync off) failed: %s\n", err.c_str() );
			++g_failures;
		}
		ctx.RequestVSync( false );
		const uint64_t offGeneration = ctx.SwapchainGeneration();
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
			++g_failures;
		Check( ctx.SwapchainGeneration() == offGeneration,
		    "repeating the same vsync request does not rebuild the swapchain" );
		ctx.RequestVSync( true );
		if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
			++g_failures;
		Check( ctx.SwapchainGeneration() == offGeneration + 1 &&
		           ctx.PresentMode() == VK_PRESENT_MODE_FIFO_KHR,
		    "vsync on rebuilds once and returns to FIFO" );
	}

	// Monitor gamma (render.gamma-ramp.v1) applied at present: the presented
	// image is the back buffer through the ramp; back-buffer reads (ReadPixels)
	// are unchanged; an 8-bit identity ramp keeps the plain blit.
	{
		ctx.RequestVSync( true );
		const auto centerOf = []( const std::vector<uint8_t> &px, int cw, int ch ) {
			return &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
		};
		const auto rampOf = []( float gamma, bool tv ) {
			render::GammaRampParams params;
			params.gamma = gamma;
			params.tvEnabled = tv;
			render::GammaRamp16 ramp;
			render::BuildGammaRamp16( params, ramp );
			return ramp;
		};
		struct Case
		{
			float gamma;
			bool tv;
			const char *name;
		};
		const Case cases[] = { { 1.6f, false, "gamma 1.6" }, { 2.6f, false, "gamma 2.6" },
			{ 2.2f, true, "TV range" } };
		for ( const Case &c : cases )
		{
			const render::GammaRamp16 ramp = rampOf( c.gamma, c.tv );
			ctx.PublishGammaRamp( ramp );
			uint8_t stored[3] = {};
			bool haveStored = false;
			if ( PresentAndCapture( ctx, 0.5f, 0.25f, 0.75f, &err ) )
			{
				int cw = 0, ch = 0;
				const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
				if ( cw > 0 && ch > 0 && !px.empty() )
				{
					std::memcpy( stored, centerOf( px, cw, ch ), 3 );
					haveStored = true;
				}
			}
			Check( haveStored && std::abs( int( stored[0] ) - 128 ) <= 1 &&
			           std::abs( int( stored[1] ) - 64 ) <= 1 &&
			           std::abs( int( stored[2] ) - 191 ) <= 1,
			    "back-buffer reads stay pre-gamma (hardware-ramp semantics)" );
			const uint64_t gammaPresents = ctx.GammaPresentCount();
			if ( haveStored && PresentAndCapture( ctx, 0.5f, 0.25f, 0.75f, &err, true ) )
			{
				int cw = 0, ch = 0;
				const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
				if ( cw == 0 && ch == 0 )
					std::fprintf( stderr, "note: this surface cannot read its presented images\n" );
				else
				{
					const uint8_t *p = centerOf( px, cw, ch );
					bool matches = true, identityMatches = true;
					for ( int channel = 0; channel < 3; ++channel )
					{
						const int expected = render::GammaRampEntryTo8Bit( ramp[stored[channel]] );
						matches = matches && std::abs( int( p[channel] ) - expected ) <= 1;
						identityMatches =
						    identityMatches && std::abs( int( p[channel] ) - stored[channel] ) <= 1;
					}
					std::fprintf( stderr, "%s: stored %d %d %d presented %d %d %d\n", c.name,
					    stored[0], stored[1], stored[2], p[0], p[1], p[2] );
					Check( ctx.GammaPresentActive() && ctx.GammaPresentCount() > gammaPresents,
					    "a non-identity ramp presents through the gamma pass" );
					Check( matches, "presented pixels are the back buffer through the ramp" );
					Check( !identityMatches,
					    "negative control: the presented pixels differ from an unramped present" );
				}
			}
			else
			{
				std::fprintf( stderr, "present/capture (%s) failed: %s\n", c.name, err.c_str() );
				++g_failures;
			}
		}

		// Scaled present through the ramp: a half-size back buffer covers the
		// drawable, every pixel ramped.
		int dw = 0, dh = 0;
		ctx.GetPresentExtent( dw, dh );
		const render::GammaRamp16 ramp = rampOf( 1.6f, false );
		ctx.PublishGammaRamp( ramp );
		if ( ctx.SetBackBufferSize( dw / 2, dh / 2, &err ) &&
		     PresentAndCapture( ctx, 0.5f, 0.5f, 0.5f, &err, true ) )
		{
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw == dw && ch == dh && !px.empty() )
			{
				const int expected = render::GammaRampEntryTo8Bit( ramp[128] );
				const uint8_t *far = &px[( size_t( ch - 2 ) * cw + ( cw - 2 ) ) * 4];
				Check( std::abs( int( centerOf( px, cw, ch )[0] ) - expected ) <= 1 &&
				           std::abs( int( far[0] ) - expected ) <= 1,
				    "a scaled present applies the ramp over the whole drawable" );
			}
		}
		else
		{
			std::fprintf( stderr, "scaled gamma present failed: %s\n", err.c_str() );
			++g_failures;
		}
		ctx.SetBackBufferSize( 0, 0, &err );

		// The default gamma is an 8-bit identity: the plain blit presents.
		ctx.PublishGammaRamp( rampOf( 2.2f, false ) );
		const uint64_t gammaPresents = ctx.GammaPresentCount();
		if ( !PresentAndCapture( ctx, 0.5f, 0.25f, 0.75f, &err ) )
			++g_failures;
		Check( !ctx.GammaPresentActive() && ctx.GammaPresentCount() == gammaPresents,
		    "the identity ramp (gamma 2.2) presents by blit" );
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
	// Draw packed WMSH corners from the persistent device-local buffers, through
	// the ordinary textured material fragment stage. This is an independent
	// pixel oracle for the new vertex format and indexed draw command.
	std::memset( worldVertices, 0, sizeof( worldVertices ) );
	const float worldPositions[3][3] = {
	    { -0.6f, -0.6f, 0.1f }, { 0.6f, -0.6f, 0.1f }, { 0.0f, 0.6f, 0.1f } };
	for ( int vertex = 0; vertex < 3; ++vertex )
	{
		uint8_t *corner = worldVertices + vertex * 40;
		std::memcpy( corner, worldPositions[vertex], sizeof( worldPositions[vertex] ) );
		corner[20] = 1; // tangent handedness; normal oct zero is +Z
	}
	Check( ctx.UploadWorldMesh(
	           worldVertices, sizeof( worldVertices ), worldIndices, sizeof( worldIndices ), &err ),
	    "packed world triangle uploads" );
	Check( ctx.InitDynamicMesh( &err ), "world mesh material pipeline initializes" );
	ctx.SelectDynamicShader( CVulkanContext::kDynShaderTextured );
	ctx.SelectDynamicColorSpace( 0 );
	const float magenta[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	ctx.SetDynamicModulation( magenta );
	ctx.BindManagedTexture( -1 );
	Check( !ctx.QueueWorldMeshBatch( 1, 3 ), "world draw rejects an index range past the buffer" );
	Check( ctx.QueueWorldMeshBatch( 0, 3 ), "world draw queues an indexed material batch" );
	if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
	{
		int cw = 0, ch = 0;
		const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
		if ( cw > 0 && ch > 0 && !px.empty() )
		{
			const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
			const uint8_t *corner = &px[0];
			Check( PixelClose( center, 255, 0, 255, 255, 3 ),
			    "WMSH indexed draw colors the center magenta" );
			Check( PixelClose( corner, 255, 0, 0, 255, 3 ),
			    "WMSH indexed draw leaves the clear outside its triangle" );
		}
		else
			Check( false, "WMSH draw captured a nonempty frame" );
	}
	else
		Check( false, "WMSH draw submitted and captured" );
	ctx.ClearDynamicQueue();
	ctx.ReleaseWorldMesh();
	Check( !ctx.WorldMeshResident(), "map unload releases world mesh buffers" );
	Check( !ctx.ReadWorldMeshBytes(
	           readVertices, sizeof( readVertices ), readIndices, sizeof( readIndices ), &err ),
	    "released world mesh cannot be read back" );
	ctx.ReleaseWorldMesh();
	Check( !ctx.WorldMeshResident(), "world mesh release is idempotent" );
	Check( ctx.UploadWorldMesh(
	           worldVertices, sizeof( worldVertices ), worldIndices, sizeof( worldIndices ), &err ),
	    "world mesh can upload after release" );

	if ( ctx.ValidationEnabled() )
		Check( ctx.ValidationErrorCount() == 0, "no validation errors/warnings during the run" );

	ctx.Shutdown();
	Check( !ctx.IsValid(), "context is invalid after Shutdown" );
	Check( !ctx.WorldMeshResident(), "device shutdown releases world mesh buffers" );

	SDL_DestroyWindow( window );
	SDL_Quit();

	std::fprintf(
	    stderr, "native Vulkan bring-up: %d checks, %d failures\n", g_checks, g_failures );
	return g_failures == 0 ? 0 : 1;
}
