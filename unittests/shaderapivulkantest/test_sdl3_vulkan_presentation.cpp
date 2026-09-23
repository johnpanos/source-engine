//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Native oracle for render.presentation.v1 on the SDL3-Vulkan pair
//          (RFC 0001 rank 9, roadmap R16). Runs the SAME shared presentation
//          suite as the headless pair against real SDL3 windows and the native
//          Vulkan provider, with GPU completion genuinely held by a timeline
//          semaphore, then checks real presented pixels in two windows.
//
//          Without a display or Vulkan device it reports an explicit skip
//          (exit 77), never a pass. A watchdog turns a deadlock into a failure.
//
//          Build/run: python3 waf build --targets=render_presentation_sdl3_vulkan_conformance
//
//===========================================================================//

#include "../rendertest/conformance/render_presentation_conformance.h"
#include "../../materialsystem/shaderapivulkan/sdl3/sdl3_vulkan_presentation.h"
#include "../../materialsystem/shaderapivulkan/vulkan_render_backend_native.h"
#include "../../platform/sdl3/render_surface/sdl3_render_surfaces.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unistd.h>

using namespace render;

namespace
{

void OnWatchdog( int )
{
	const char message[] = "FAIL render.presentation.sdl3_vulkan: watchdog expired (deadlock)\n";
	(void)!write( 2, message, sizeof( message ) - 1 );
	_exit( 1 );
}

struct Color
{
	float r, g, b;
};

class Sdl3VulkanHarness : public conformance::IPresentationHarness
{
public:
	Sdl3VulkanHarness( platform_sdl3::Sdl3RenderSurfaces &surfaces,
	    render_vulkan::Sdl3VulkanPresentationBridge &bridge,
	    render_vulkan::VulkanRenderBackend &provider )
	    : m_Surfaces( surfaces ), m_Bridge( bridge ), m_Provider( provider )
	{
	}

	IRenderSurface *CreateSurface( RenderExtent extent ) override
	{
		SDL_Window *window = SDL_CreateWindow( "r16-presentation", static_cast<int>( extent.width ),
		    static_cast<int>( extent.height ), SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE );
		if ( !window )
			return nullptr;
		SDL_SyncWindow( window );
		Pump();
		IRenderSurface *surface = m_Surfaces.Adopt( window );
		m_Windows[surface] = window;
		return surface;
	}

	bool ResizeSurface( IRenderSurface &surface, RenderExtent extent ) override
	{
		SDL_Window *window = m_Windows[&surface];
		if ( !window || !SDL_SetWindowSize( window, static_cast<int>( extent.width ),
		                    static_cast<int>( extent.height ) ) )
			return false;
		SDL_SyncWindow( window );
		Pump();
		return true;
	}

	// Minimize and restore. Hiding is deliberately not used: on Wayland (mutter,
	// Mesa WSI) hiding a window whose swapchain presented in FIFO mode leaves the
	// hide commit latched behind the FIFO barrier of a surface that never
	// repaints, and re-showing it is a protocol error that closes the connection.
	// A minimized window reports a zero drawable where the platform exposes it.
	void SetSurfaceVisible( IRenderSurface &surface, bool visible ) override
	{
		SDL_Window *window = m_Windows[&surface];
		if ( !window )
			return;
		if ( visible )
			SDL_RestoreWindow( window );
		else
			SDL_MinimizeWindow( window );
		SDL_SyncWindow( window );
		Pump();
	}

	// Through the lifecycle events a mobile platform delivers, so the window
	// system's event mapping is exercised, not just its direct API.
	void ReleaseNativeSurface( IRenderSurface & ) override
	{
		PushLifecycle( SDL_EVENT_DID_ENTER_BACKGROUND );
	}
	void RestoreNativeSurface( IRenderSurface & ) override
	{
		PushLifecycle( SDL_EVENT_WILL_ENTER_FOREGROUND );
	}

	void DestroyWindow( IRenderSurface &surface ) override
	{
		SDL_Window *window = m_Windows[&surface];
		if ( !window )
			return;
		m_Surfaces.InvalidateWindow( surface );
		// The contract: nothing native built on the window survives its release.
		if ( m_Bridge.NativeSurfaceCount( surface ) != 0 )
			++m_Violations;
		SDL_DestroyWindow( window );
		m_Windows[&surface] = nullptr;
		Pump();
	}

	void DestroySurface( IRenderSurface *surface ) override
	{
		m_Windows.erase( surface );
		m_Surfaces.Destroy( surface );
	}

	uint32_t GetMaxSurfaces() const override { return 5; }
	bool CanToggleVisibility() const override
	{
		const char *driver = SDL_GetCurrentVideoDriver();
		return !driver || std::strcmp( driver, "wayland" ) != 0;
	}
	uint32_t GetNativeLifetimeViolations() const override { return m_Violations; }

	bool HoldGpuCompletion( IRenderDevice &device ) override
	{
		render_vulkan::VulkanDeviceEndpoint *ep = m_Provider.FindDevice( device );
		return ep && ep->HoldCompletion();
	}

	void ReleaseGpuCompletion( IRenderDevice &device ) override
	{
		if ( render_vulkan::VulkanDeviceEndpoint *ep = m_Provider.FindDevice( device ) )
			ep->ReleaseCompletion();
	}

	bool IsGpuWorkPending( IRenderDevice &device ) override
	{
		render_vulkan::VulkanDeviceEndpoint *ep = m_Provider.FindDevice( device );
		return ep && ep->HasIncompleteGpuWork();
	}

	void DrawFrame( IRenderDevice &device, IRenderCommandContext &context,
	    RenderResourceHandle backBuffer, uint32_t frameIndex ) override
	{
		context.RecordUse( backBuffer );
		render_vulkan::VulkanDeviceEndpoint *ep = m_Provider.FindDevice( device );
		if ( !ep )
			return;
		Color c = m_FixedColor;
		if ( !m_UseFixedColor )
			c = Color{ ( frameIndex % 3 ) / 2.0f, 0.25f, 1.0f - ( frameIndex % 3 ) / 2.0f };
		VkClearColorValue value = {};
		value.float32[0] = c.r;
		value.float32[1] = c.g;
		value.float32[2] = c.b;
		value.float32[3] = 1.0f;
		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;
		// The back buffer rests in GENERAL (the bridge's endpoint contract).
		vkCmdClearColorImage( ep->CommandBuffer( context ), ep->Image( backBuffer ),
		    VK_IMAGE_LAYOUT_GENERAL, &value, 1, &range );
	}

	void UseColor( Color c )
	{
		m_FixedColor = c;
		m_UseFixedColor = true;
	}

	void Pump()
	{
		SDL_PumpEvents();
		SDL_Event event;
		while ( SDL_PollEvent( &event ) )
			m_Surfaces.HandleEvent( event );
	}

private:
	void PushLifecycle( SDL_EventType type )
	{
		SDL_Event event = {};
		event.type = type;
		SDL_PushEvent( &event );
		Pump();
	}

	platform_sdl3::Sdl3RenderSurfaces &m_Surfaces;
	render_vulkan::Sdl3VulkanPresentationBridge &m_Bridge;
	render_vulkan::VulkanRenderBackend &m_Provider;
	std::map<IRenderSurface *, SDL_Window *> m_Windows;
	uint32_t m_Violations = 0;
	Color m_FixedColor = { 0, 0, 0 };
	bool m_UseFixedColor = false;
};

int g_Checks = 0;
int g_Failures = 0;

void Check( bool ok, const char *id, const std::string &detail )
{
	++g_Checks;
	if ( !ok )
	{
		++g_Failures;
		std::printf( "FAIL %s: %s\n", id, detail.c_str() );
	}
}

bool Near( uint8_t v, float expected )
{
	return std::abs( int( v ) - int( expected * 255.0f + 0.5f ) ) <= 3;
}

// Presents one frame of a solid color and reads the presented swapchain image.
bool PresentAndCapture( IRenderDevice &device, IRenderPresentation &p,
    render_vulkan::Sdl3VulkanPresentationBridge &bridge, Sdl3VulkanHarness &harness, Color color,
    std::vector<uint8_t> *pixels, uint32_t *w, uint32_t *h )
{
	harness.UseColor( color );
	for ( int attempt = 0; attempt < 4; ++attempt )
	{
		RenderResourceHandle back = kInvalidResource;
		const RenderPresentStatus begin = p.BeginFrame( &back );
		if ( begin == RenderPresentStatus::kRecoverable )
			continue;
		if ( begin != RenderPresentStatus::kOk )
			return false;
		IRenderCommandContext *context = device.CreateCommandContext();
		harness.DrawFrame( device, *context, back, 0 );
		device.Submit( *context );
		bridge.RequestCapture( p );
		const RenderPresentStatus status = p.Present();
		if ( status == RenderPresentStatus::kOk )
			return bridge.ReadCapture( p, pixels, w, h );
		if ( status != RenderPresentStatus::kRecoverable )
			return false;
	}
	return false;
}

bool UniformColor( const std::vector<uint8_t> &pixels, uint32_t w, uint32_t h, Color c )
{
	if ( pixels.size() < size_t( w ) * h * 4 || w == 0 || h == 0 )
		return false;
	const uint32_t xs[] = { 0, w / 2, w - 1 };
	const uint32_t ys[] = { 0, h / 2, h - 1 };
	for ( uint32_t y : ys )
		for ( uint32_t x : xs )
		{
			const uint8_t *px = &pixels[( size_t( y ) * w + x ) * 4];
			if ( !Near( px[0], c.r ) || !Near( px[1], c.g ) || !Near( px[2], c.b ) )
				return false;
		}
	return true;
}

// Real images in two windows on one device, including after a window resize.
void CheckPixels( render_vulkan::VulkanRenderBackend &provider,
    render_vulkan::Sdl3VulkanPresentationBridge &bridge, Sdl3VulkanHarness &harness )
{
	RenderDeviceRequest request;
	IRenderDevice *device = provider.CreateDevice( request, nullptr );
	IRenderSurface *a = harness.CreateSurface( RenderExtent{ 320, 200 } );
	IRenderSurface *b = harness.CreateSurface( RenderExtent{ 200, 320 } );
	if ( !device || !a || !b )
	{
		Check( false, "pixels.setup", "could not create a device and two windows" );
		return;
	}
	IRenderPresentation *pa =
	    bridge.CreatePresentation( *device, *a, RenderPresentationConfig(), nullptr );
	IRenderPresentation *pb =
	    bridge.CreatePresentation( *device, *b, RenderPresentationConfig(), nullptr );
	std::vector<uint8_t> pixels;
	uint32_t w = 0, h = 0;
	const Color red = { 1.0f, 0.0f, 0.0f }, blue = { 0.0f, 0.0f, 1.0f },
	            green = { 0.0f, 1.0f, 0.0f };

	bool okA = pa && PresentAndCapture( *device, *pa, bridge, harness, red, &pixels, &w, &h );
	Check(
	    okA && UniformColor( pixels, w, h, red ) && RenderExtent{ w, h } == a->GetDrawableExtent(),
	    "pixels.window_a", "window A presents its red back buffer at its drawable size" );
	bool okB = pb && PresentAndCapture( *device, *pb, bridge, harness, blue, &pixels, &w, &h );
	Check(
	    okB && UniformColor( pixels, w, h, blue ) && RenderExtent{ w, h } == b->GetDrawableExtent(),
	    "pixels.window_b", "window B presents its own blue back buffer (multi-view)" );

	// Negative control: the capture really distinguishes colors.
	Check( !UniformColor( pixels, w, h, red ), "pixels.negative_control",
	    "a blue capture must not match red" );

	harness.ResizeSurface( *a, RenderExtent{ 240, 360 } );
	okA = pa && PresentAndCapture( *device, *pa, bridge, harness, green, &pixels, &w, &h );
	Check( okA && UniformColor( pixels, w, h, green ) &&
	           RenderExtent{ w, h } == a->GetDrawableExtent(),
	    "pixels.after_window_resize",
	    "after an aspect change the swapchain follows the drawable (" + std::to_string( w ) + "x" +
	        std::to_string( h ) + ")" );

	if ( pa )
		bridge.DestroyPresentation( pa );
	if ( pb )
		bridge.DestroyPresentation( pb );
	harness.DestroyWindow( *a );
	harness.DestroyWindow( *b );
	harness.DestroySurface( a );
	harness.DestroySurface( b );
	while ( device->PollCompletion() > 0 )
	{
	}
	Check( bridge.ReleaseDevice( *device ), "pixels.release_device", "bridge releases the device" );
	provider.DestroyDevice( device );
}

} // namespace

int main()
{
	std::signal( SIGALRM, OnWatchdog );
	alarm( 180 );

	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::printf( "SKIP render.presentation.sdl3_vulkan: no display (%s)\n", SDL_GetError() );
		return 77;
	}
	render_vulkan::VulkanProviderOptions options;
	std::string error;
	if ( !render_vulkan::Sdl3VulkanInstanceExtensions( &options.instanceExtensions, &error ) )
	{
		std::printf( "SKIP render.presentation.sdl3_vulkan: %s\n", error.c_str() );
		SDL_Quit();
		return 77;
	}
	options.enableSwapchain = true;
	options.enableCompletionGate = true;
	std::unique_ptr<render_vulkan::VulkanRenderBackend> provider =
	    render_vulkan::MakeVulkanRenderBackend( options, &error );
	if ( !provider )
	{
		std::printf( "SKIP render.presentation.sdl3_vulkan: %s\n", error.c_str() );
		SDL_Quit();
		return 77;
	}
	std::printf(
	    "render.presentation.sdl3_vulkan: video driver '%s'\n", SDL_GetCurrentVideoDriver() );

	int status = 0;
	{
		platform_sdl3::Sdl3RenderSurfaces surfaces;
		render_vulkan::Sdl3VulkanPresentationBridge bridge( *provider, surfaces, 4 );
		Sdl3VulkanHarness harness( surfaces, bridge, *provider );

		conformance::Report report;
		const bool passed =
		    conformance::RunPresentationConformance( *provider, bridge, harness, report );
		for ( const conformance::CheckResult &c : report.checks )
			if ( !c.ok )
				std::printf( "FAIL %s: %s\n", c.id.c_str(), c.detail.c_str() );
		for ( const conformance::CheckResult &c : report.skipped )
			std::printf( "SKIP %s: %s\n", c.id.c_str(), c.detail.c_str() );
		std::printf(
		    "render.presentation.sdl3_vulkan: shared suite %d check(s), %d failure(s) -> %s\n",
		    static_cast<int>( report.checks.size() ), report.FailureCount(),
		    passed ? "PASS" : "FAIL" );
		if ( !passed || report.checks.size() < 40 )
			status = 1;

		CheckPixels( *provider, bridge, harness );
		std::printf( "render.presentation.sdl3_vulkan: native pixel checks %d, failure(s) %d\n",
		    g_Checks, g_Failures );
		if ( g_Failures != 0 || g_Checks < 5 )
			status = 1;
	}
	Check( provider->GetLiveDeviceCount() == 0, "lifetime.no_leaked_devices",
	    "all devices destroyed" );
	provider.reset();
	SDL_Quit();
	std::printf(
	    "render.presentation.sdl3_vulkan: %s\n", status == 0 && g_Failures == 0 ? "PASS" : "FAIL" );
	return status == 0 && g_Failures == 0 ? 0 : 1;
}
