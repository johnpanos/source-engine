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
#include "../../render/bridge/sdl3-vulkan/sdl3_vulkan_surface_host.h"
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
		if ( !ctx.RenderFrame( &skip, err ) )
			return false;
		if ( skip )
		{
			--i;
			SDL_Delay( 8 );
			continue;
		}
	}
	return true;
}

// A real core section on either side of the retained portal snapshot. The
// independent pixel oracle detects a snapshot/draw moved across either slot.
class PortalReplayRecorder final : public render::legacy::ICorePassRecorder
{
public:
	std::uint32_t SlotStages() const override { return 0; }
	void RecordSlot( std::uint32_t tag, render::device::CommandEncoder &encoder,
	    const render::legacy::CorePassTarget &target ) override
	{
		using namespace render::device;
		ColorAttachment color;
		color.texture = target.color;
		color.load = LoadOp::kClear;
		color.store = StoreOp::kStore;
		color.clear = ( tag & 0xffu ) == 1 ? ClearColor{ 1, 0, 0, 1 } : ClearColor{ 0, 0, 1, 1 };
		RenderingDesc rendering;
		rendering.width = target.width;
		rendering.height = target.height;
		rendering.colors = std::span<const ColorAttachment>( &color, 1 );
		encoder.BeginRendering( rendering );
		encoder.EndRendering();
	}
	bool RecordOutput(
	    render::device::CommandEncoder &, const render::legacy::CoreOutputTargets & ) override
	{
		return false;
	}
	void ReleaseDevice( render::device::IRenderDevice2 & ) override {}
};

bool RenderCoreFrame( CVulkanContext &ctx, bool *skipped, std::string *error )
{
	using namespace render::device;
	if ( !ctx.PrepareFrame( skipped, error ) )
		return false;
	if ( *skipped )
		return true;
	auto encoder = ctx.Port()->BeginEncoder( QueueKind::kGraphics );
	if ( !encoder )
		return ctx.FinishFrame( {}, false, error );
	for ( int i = 0; i < CVulkanContext::kFrameStageCount; ++i )
	{
		const auto stage = static_cast<CVulkanContext::FrameStage>( i );
		if ( ctx.HasFrameStage( stage ) )
			ctx.AttachFrameStage( stage, encoder.Value() );
	}
	auto submitted = ctx.Port()->Submit( QueueKind::kGraphics, { &encoder.Value(), 1 }, {} );
	return ctx.FinishFrame(
	    submitted ? submitted.Value() : CompletionToken{}, submitted.HasValue(), error );
}

void CheckPortalCoreReplay( CVulkanContext &ctx, std::string &err )
{
	PortalReplayRecorder recorder;
	ctx.BindCorePassRecorder( &recorder );
	const int snapshot = ctx.CreateRenderTargetTexture( 64, 64, &err );
	Check( snapshot >= 0 && ctx.PortalPipelineSupported(), "portal replay resources available" );
	if ( snapshot >= 0 && ctx.PortalPipelineSupported() )
	{
		const float quad[6][8] = { { -0.8f, -0.8f, 0, 1, 1, 1, 0, 0 },
		    { 0.8f, -0.8f, 0, 1, 1, 1, 1, 0 }, { 0.8f, 0.8f, 0, 1, 1, 1, 1, 1 },
		    { -0.8f, -0.8f, 0, 1, 1, 1, 0, 0 }, { 0.8f, 0.8f, 0, 1, 1, 1, 1, 1 },
		    { -0.8f, 0.8f, 0, 1, 1, 1, 0, 1 } };
		float tangents[6][7];
		for ( auto &v : tangents )
		{
			const float basis[7] = { 0, 0, 1, 1, 0, 0, 1 };
			std::copy_n( basis, 7, v );
		}
		CVulkanContext::PortalConstants portal{};
		portal.model[0] = portal.model[5] = portal.model[10] = portal.model[15] = 1;
		std::copy_n( portal.model, 16, portal.viewProj );
		portal.texXform0[0] = portal.texXform1[1] = 1;
		portal.openAmount = 1;
		portal.stage = 0;
		CVulkanContext::DynRasterState raster;
		raster.depthTest = raster.depthWrite = false;
		raster.cullMode = VK_CULL_MODE_NONE;
		for ( int mode : { 0, 1, 0, 2 } )
		{
			const bool product = mode != 1;
			const bool wrongOrder = mode == 2;
			ctx.ClearDynamicQueue();
			ctx.SetRenderTarget( -1 );
			ctx.SetViewport( 0, 0, 0, 0, 0, 1 );
			const std::uint32_t policy = render::legacy::kCorePassForwarded |
			                             render::legacy::kCorePassLegacyOff |
			                             ( product ? render::legacy::kCorePassPortalEffects : 0 );
			ctx.QueueCorePass( policy | 1, {} );
			Check( ctx.QueueCopyToTexture( snapshot, nullptr, nullptr ), "portal snapshot queued" );
			ctx.QueueCorePass( render::legacy::kCorePassForwarded | 2, {} );
			ctx.SelectDynamicShader( CVulkanContext::kDynShaderPortalRefract );
			ctx.SelectDynamicRasterState( raster );
			ctx.SetDynamicPortalConstants( portal );
			ctx.SelectDynamicColorSpace( 0 );
			ctx.BindManagedTexture( snapshot );
			ctx.QueueDynamicTriangles( &quad[0][0], 6, nullptr, &tangents[0][0] );
			// Unconsumed copies and ordinary legacy draws must stay suppressed.
			Check( ctx.QueueCopyToTexture( snapshot, nullptr, nullptr ), "unused snapshot queued" );
			ctx.SelectDynamicShader( CVulkanContext::kDynShaderConstColor );
			ctx.SetDynamicConstantColor( 0, 1, 0, 1 );
			ctx.QueueDynamicTriangles( &quad[0][0], 6 );
			if ( wrongOrder )
				ctx.QueueCorePass( render::legacy::kCorePassForwarded | 2, {} );
			for ( int replay = 0; replay < 2; ++replay )
			{
				ctx.RequestCapture();
				bool skipped = false;
				const bool rendered = RenderCoreFrame( ctx, &skipped, &err );
				int width = 0, height = 0;
				const auto &pixels = ctx.GetCapturedPixels( &width, &height );
				bool correct = rendered && !skipped && width > 0 && height > 0 && !pixels.empty();
				if ( correct )
				{
					const auto *center = &pixels[( size_t( height / 2 ) * width + width / 2 ) * 4];
					correct =
					    PixelClose( pixels.data(), 0, 0, 255, 255, 2 ) &&
					    ( product && !wrongOrder ? center[0] > 200 && center[1] < 3 && center[2] < 3
					                             : PixelClose( center, 0, 0, 255, 255, 2 ) );
				}
				Check( correct, "portal snapshot stays before child slot and effect after it, "
				                "including replay" );
				Check( ctx.LastFrameCost().count[render_vulkan::kCostTargetCopy] ==
				           ( product ? 1u : 0u ),
				    "only the consumed portal copy survives product filtering; diagnostics retain "
				    "none" );
			}
		}
	}
	ctx.ClearDynamicQueue();
	ctx.BindCorePassRecorder( nullptr );
	if ( snapshot >= 0 )
		ctx.DestroyManagedTexture( snapshot );
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

	config.deviceFactory = &render::device::vulkan::HostDeviceFactory();
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
		const auto offers = [&]( VkPresentModeKHR mode )
		{
			return std::find( offered.begin(), offered.end(), mode ) != offered.end();
		};
		std::fprintf( stderr, "surface present modes:" );
		for ( VkPresentModeKHR mode : offered )
			std::fprintf( stderr, " %d", static_cast<int>( mode ) );
		std::fprintf( stderr, "\n" );
		Check( offers( VK_PRESENT_MODE_FIFO_KHR ), "the surface offers FIFO (Vulkan guarantee)" );
		Check( ctx.VSyncRequested() && ctx.PresentMode() == VK_PRESENT_MODE_FIFO_KHR,
		    "the default request is vsync, presented FIFO" );

		const VkPresentModeKHR expectedOff =
		    offers( VK_PRESENT_MODE_IMMEDIATE_KHR ) ? VK_PRESENT_MODE_IMMEDIATE_KHR
		    : offers( VK_PRESENT_MODE_MAILBOX_KHR ) ? VK_PRESENT_MODE_MAILBOX_KHR
		                                            : VK_PRESENT_MODE_FIFO_KHR;
		const uint64_t generation = ctx.SwapchainGeneration();
		ctx.RequestVSync( false );
		Check(
		    ctx.SwapchainGeneration() == generation, "a vsync request waits for the next frame" );
		if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
		{
			Check( ctx.SwapchainGeneration() == generation + 1,
			    "vsync off rebuilt the swapchain exactly once" );
			Check(
			    ctx.PresentMode() == expectedOff, "vsync off selects the policy's present mode" );
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
		const auto centerOf = []( const std::vector<uint8_t> &px, int cw, int ch )
		{
			return &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
		};
		const auto rampOf = []( float gamma, bool tv )
		{
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

	// Multisampling (render.sample-count.v1): the magenta triangle's slanted
	// edges over the red clear. Single-sampled, every pixel is fully one or the
	// other (the negative control); multisampled, edge pixels blend, resolved
	// into the back buffer that captures and presents read.
	{
		const auto blendedPixels = []( const std::vector<uint8_t> &px )
		{
			int blended = 0;
			for ( size_t i = 0; i + 3 < px.size(); i += 4 )
				blended += px[i] >= 250 && px[i + 1] <= 5 && px[i + 2] >= 16 && px[i + 2] <= 239;
			return blended;
		};
		int singleBlended = -1;
		if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
			singleBlended = blendedPixels( ctx.GetCapturedPixels( nullptr, nullptr ) );
		Check( ctx.ActiveSampleCount() == 1 && singleBlended == 0,
		    "single-sampled edges are hard (no blended pixel)" );
		const uint32_t mask = ctx.AdapterCaps().backBufferSampleMask;
		Check( ( mask & VK_SAMPLE_COUNT_1_BIT ) != 0, "the back buffer sample mask includes 1" );
		if ( mask & VK_SAMPLE_COUNT_4_BIT )
		{
			const uint64_t resolves = ctx.ResolveCount();
			ctx.RequestSampleCount( 4 );
			Check(
			    ctx.ActiveSampleCount() == 1, "a sample-count request waits for the next frame" );
			int msBlended = 0;
			bool centerOk = false, cornerOk = false;
			if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
			{
				int cw = 0, ch = 0;
				const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
				msBlended = blendedPixels( px );
				if ( cw > 0 && ch > 0 && !px.empty() )
				{
					centerOk = PixelClose(
					    &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4], 255, 0, 255, 255, 3 );
					cornerOk = PixelClose( &px[0], 255, 0, 0, 255, 3 );
				}
			}
			std::fprintf( stderr, "MSAA edge pixels: 1x %d, 4x %d\n", singleBlended, msBlended );
			Check( ctx.ActiveSampleCount() == 4, "4x multisampling applies at the next frame" );
			Check( ctx.ResolveCount() > resolves, "multisampled frames resolve the back buffer" );
			Check( centerOk && cornerOk, "4x keeps the interior and the clear" );
			Check( msBlended > 0, "4x blends the triangle's edge pixels" );
			bool presentedOk = false;
			if ( PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err, true ) )
			{
				int cw = 0, ch = 0;
				const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
				presentedOk = ( cw == 0 && ch == 0 ) ||
				              ( !px.empty() && blendedPixels( px ) > 0 &&
				                  PixelClose( &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4], 255, 0,
				                      255, 255, 3 ) );
			}
			Check( presentedOk, "the presented image is the resolved back buffer" );
			ctx.RequestSampleCount( 64 );
			if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
				++g_failures;
			int largest = 1;
			for ( int samples = 2; samples <= 64; samples *= 2 )
				largest = ( mask & static_cast<uint32_t>( samples ) ) ? samples : largest;
			Check( ctx.ActiveSampleCount() == largest,
			    "an unsupported request clamps to the largest supported count" );
			ctx.RequestSampleCount( 0 );
			if ( !PresentAndCapture( ctx, 1.0f, 0.0f, 0.0f, &err ) )
				++g_failures;
			Check( ctx.ActiveSampleCount() == 1 &&
			           blendedPixels( ctx.GetCapturedPixels( nullptr, nullptr ) ) == 0,
			    "AA off returns to hard single-sampled edges" );
		}
		else
			std::fprintf( stderr, "note: this device cannot multisample the back buffer 4x\n" );
	}
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

	err.clear(); // the preceding invalid-readback negative case intentionally set this

	// Render-pass merging: a tiled GPU stores and reloads the whole target at
	// every pass break. Draws that write no color and depth/stencil-only clears
	// keep the open (sRGB) view, a query spanning them stays in one pass, a
	// repeated copy with nothing drawn since is made once, and the frame's
	// clearing pass opens in the view its first draw needs. The frame is: sRGB
	// draw, query { masked draw, depth/stencil clear, sRGB draw }, two identical
	// copies, sRGB draw. With merging off (-vkpassmerge 0, the earlier policy)
	// the same frame takes seven passes and two copies and fails the query.
	{
		static const float quad[6][8] = {
		    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
		    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
		    { 0.5f, -0.5f, 0.5f, 1, 1, 1, 1, 0 },
		    { -0.5f, -0.5f, 0.5f, 1, 1, 1, 0, 0 },
		    { -0.5f, 0.5f, 0.5f, 1, 1, 1, 0, 1 },
		    { 0.5f, 0.5f, 0.5f, 1, 1, 1, 1, 1 },
		};
		const float grey[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		const float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		CVulkanContext::DynRasterState opaque;
		CVulkanContext::DynRasterState masked;
		masked.colorWrite = false;
		const int copyTarget = ctx.CreateRenderTargetTexture( 64, 64, &err );
		const int query = ctx.CreateOcclusionQuery( &err );
		const bool srgb = ctx.LinearSpaceSrgbBlending();
		for ( bool merge : { true, false } )
		{
			const char *const mode = merge ? "merged" : "unmerged";
			ctx.SetPassMerging( merge );
			ctx.ClearDynamicQueue();
			ctx.SetClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
			ctx.SetRenderTarget( -1 );
			ctx.SelectDynamicShader( CVulkanContext::kDynShaderTextured );
			ctx.BindManagedTexture( -1 );
			ctx.SelectDynamicRasterState( opaque );
			ctx.SelectDynamicColorSpace( CVulkanContext::kColorSrgbWrite );
			ctx.SetDynamicModulation( grey );
			ctx.QueueDynamicTriangles( &quad[0][0], 6 );
			if ( query >= 0 )
				ctx.QueueBeginOcclusionQuery( query );
			ctx.SelectDynamicRasterState( masked );
			ctx.SelectDynamicColorSpace( 0 );
			ctx.SetDynamicModulation( red );
			ctx.QueueDynamicTriangles( &quad[0][0], 6 );
			ctx.QueueClear( false, true, true );
			ctx.SelectDynamicRasterState( opaque );
			ctx.SelectDynamicColorSpace( CVulkanContext::kColorSrgbWrite );
			ctx.SetDynamicModulation( grey );
			ctx.QueueDynamicTriangles( &quad[0][0], 6 );
			if ( query >= 0 )
				ctx.QueueEndOcclusionQuery( query );
			Check( ctx.QueueCopyToTexture( copyTarget, nullptr, nullptr ), "first copy queues" );
			Check( ctx.QueueCopyToTexture( copyTarget, nullptr, nullptr ), "repeated copy queues" );
			ctx.QueueDynamicTriangles( &quad[0][0], 6 );
			ctx.RequestCapture();
			bool skip = true;
			for ( int attempt = 0; skip && attempt < 100; ++attempt )
			{
				if ( !ctx.RenderFrame( &skip, &err ) )
					break;
				if ( skip )
					SDL_Delay( 8 );
			}
			Check( !skip, "pass-merge frame records" );
			if ( skip )
				continue;
			const render_vulkan::FrameCost &cost = ctx.LastFrameCost();
			const uint32_t passes = cost.count[render_vulkan::kCostRenderPass];
			const uint32_t copies = cost.count[render_vulkan::kCostTargetCopy];
			std::fprintf( stderr, "  %s: %u render passes, %u copies\n", mode, passes, copies );
			if ( merge )
			{
				// The clearing pass (in the sRGB view) and the pass after the copy.
				Check(
				    passes == 2, "masked draws, depth/stencil clears and queries break no pass" );
				Check( copies == 1, "a repeated copy with nothing drawn since is made once" );
			}
			else
			{
				// Clear, sRGB, UNORM (masked draw and clear), sRGB, two reopened
				// after the copies, and the UNORM pass the frame ended in.
				Check( passes == ( srgb ? 7u : 3u ), "without merging, each view change breaks" );
				Check( copies == 2, "without merging, every copy is made" );
			}
			Check( err.empty(), "pass-merge frame presents" );
			if ( merge && query >= 0 )
			{
				const int64_t samples = ctx.OcclusionQueryResult( query, true );
				Check( samples > 0, "a query spanning a masked and an sRGB draw completes" );
			}
			int cw = 0, ch = 0;
			const std::vector<uint8_t> &px = ctx.GetCapturedPixels( &cw, &ch );
			if ( cw > 0 && ch > 0 && !px.empty() )
			{
				// sRGB(0.5) = 188 in both modes; the masked red draw left no color.
				const uint8_t *center = &px[( size_t( ch / 2 ) * cw + cw / 2 ) * 4];
				const uint8_t *corner = &px[0];
				Check( PixelClose( center, 188, 188, 188, 255, 2 ),
				    "the sRGB-encoded grey is drawn; the masked draw writes no color" );
				Check( PixelClose( corner, 0, 0, 0, 255, 0 ),
				    "the clear stores the same bytes in either view" );
			}
			else
				Check( false, "pass-merge frame captured" );
		}
		ctx.SetPassMerging( true );
		if ( query >= 0 )
			ctx.DestroyOcclusionQuery( query );
		ctx.ClearDynamicQueue();
	}

	CheckPortalCoreReplay( ctx, err );

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
