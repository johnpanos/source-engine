//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.presentation.sdl3-d3d12 (RFC 0024 X4): the shared
//			render.presentation.v1 suite against the SDL3-Direct3D 12 bridge
//			over render_d3d12's render.backend.v1 provider, with real windows,
//			plus native pixel checks: two windows on one device present their
//			own colors (multi-view), a capture distinguishes them (negative
//			control), and after a window's aspect change the swapchain follows
//			its drawable.
//
//			Windows-only; built with MinGW-w64 and run under Wine with
//			vkd3d-proton inside a private headless compositor
//			(tools/render/d3d12_lane.py suite --session), never on the
//			user's desktop. Reports one checks-v1 record.
//
//=============================================================================//

#include "conformance/render_presentation_conformance.h"
#include "../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "../../render/bridge/sdl3-d3d12/sdl3_d3d12_presentation.h"
#include "../../render/device/d3d12/backend_v1/render_backend_v1.h"
#include "render/device/d3d12/host_device.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace render;

namespace
{

struct Color
{
	float r, g, b;
};

class Sdl3D3d12Harness : public conformance::IPresentationHarness
{
public:
	Sdl3D3d12Harness( platform_sdl3::Sdl3RenderSurfaces &surfaces,
	    render_d3d12::Sdl3D3d12PresentationBridge &bridge, render_d3d12::D3d12RenderBackend &provider )
	    : m_Surfaces( surfaces ), m_Bridge( bridge ), m_Provider( provider )
	{
	}

	~Sdl3D3d12Harness() override
	{
		if ( m_Rtv )
			m_Rtv->Release();
	}

	IRenderSurface *CreateSurface( RenderExtent extent ) override
	{
		SDL_Window *window = SDL_CreateWindow( "rfc0024-presentation", int( extent.width ),
		    int( extent.height ), SDL_WINDOW_RESIZABLE );
		if ( !window )
			return nullptr;
		SDL_SyncWindow( window );
		IRenderSurface *surface = m_Surfaces.Adopt( window );
		// Wine maps the window asynchronously; the surface is handed out once
		// it shows a drawable (driven by window events, bounded).
		WaitFor( [surface] { return surface->GetDrawableExtent().IsPresentable(); } );
		m_Windows[surface] = window;
		return surface;
	}

	bool ResizeSurface( IRenderSurface &surface, RenderExtent extent ) override
	{
		SDL_Window *window = m_Windows[&surface];
		if ( !window || !SDL_SetWindowSize( window, int( extent.width ), int( extent.height ) ) )
			return false;
		SDL_SyncWindow( window );
		WaitFor( [&surface, extent] { return surface.GetDrawableExtent() == extent; } );
		return true;
	}

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
		WaitFor( [&surface, visible]
		    { return surface.GetDrawableExtent().IsPresentable() == visible; } );
	}

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

	std::uint32_t GetMaxSurfaces() const override { return 5; }
	// A minimized window under Wine's X11 driver restores programmatically.
	bool CanToggleVisibility() const override { return true; }
	std::uint32_t GetNativeLifetimeViolations() const override { return m_Violations; }

	bool HoldGpuCompletion( IRenderDevice &device ) override
	{
		render_d3d12::D3d12DeviceEndpoint *ep = m_Provider.FindDevice( device );
		return ep && ep->HoldCompletion();
	}
	void ReleaseGpuCompletion( IRenderDevice &device ) override
	{
		if ( render_d3d12::D3d12DeviceEndpoint *ep = m_Provider.FindDevice( device ) )
			ep->ReleaseCompletion();
	}
	bool IsGpuWorkPending( IRenderDevice &device ) override
	{
		render_d3d12::D3d12DeviceEndpoint *ep = m_Provider.FindDevice( device );
		return ep && ep->HasIncompleteGpuWork();
	}

	// Clears the back buffer (which rests in COMMON) to the frame's color.
	void DrawFrame( IRenderDevice &device, IRenderCommandContext &context,
	    RenderResourceHandle backBuffer, std::uint32_t frameIndex ) override
	{
		context.RecordUse( backBuffer );
		render_d3d12::D3d12DeviceEndpoint *ep = m_Provider.FindDevice( device );
		ID3D12Resource *texture = ep ? ep->Texture( backBuffer ) : nullptr;
		if ( !texture )
			return;
		if ( !m_Rtv || m_RtvDevice != ep->NativeDevice() )
		{
			if ( m_Rtv )
				m_Rtv->Release();
			D3D12_DESCRIPTOR_HEAP_DESC heap{};
			heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			heap.NumDescriptors = 1;
			m_Rtv = nullptr;
			ep->NativeDevice()->CreateDescriptorHeap(
			    &heap, IID_ID3D12DescriptorHeap, reinterpret_cast<void **>( &m_Rtv ) );
			m_RtvDevice = ep->NativeDevice();
		}
		Color c = m_FixedColor;
		if ( !m_UseFixedColor )
			c = Color{ ( frameIndex % 3 ) / 2.0f, 0.25f, 1.0f - ( frameIndex % 3 ) / 2.0f };
		ID3D12Device *native = ep->NativeDevice();
		ID3D12DescriptorHeap *rtv = m_Rtv;
		ep->RecordNative( context,
		    [native, rtv, texture, c]( ID3D12GraphicsCommandList &list )
		    {
			    D3D12_RESOURCE_BARRIER barrier{};
			    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			    barrier.Transition.pResource = texture;
			    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			    list.ResourceBarrier( 1, &barrier );
			    const D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv->GetCPUDescriptorHandleForHeapStart();
			    native->CreateRenderTargetView( texture, nullptr, handle );
			    const float color[4] = { c.r, c.g, c.b, 1.0f };
			    list.ClearRenderTargetView( handle, color, 0, nullptr );
			    std::swap( barrier.Transition.StateBefore, barrier.Transition.StateAfter );
			    list.ResourceBarrier( 1, &barrier );
		    } );
	}

	void UseColor( Color c )
	{
		m_FixedColor = c;
		m_UseFixedColor = true;
	}

	// Pumps window events until done() holds or two seconds pass.
	template <typename Done> void WaitFor( Done done )
	{
		const std::uint64_t deadline = SDL_GetTicks() + 2000;
		Pump();
		while ( !done() && SDL_GetTicks() < deadline )
		{
			SDL_Event event;
			if ( SDL_WaitEventTimeout( &event, 20 ) )
				m_Surfaces.HandleEvent( event );
			Pump();
		}
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
	render_d3d12::Sdl3D3d12PresentationBridge &m_Bridge;
	render_d3d12::D3d12RenderBackend &m_Provider;
	std::map<IRenderSurface *, SDL_Window *> m_Windows;
	std::uint32_t m_Violations = 0;
	Color m_FixedColor = { 0, 0, 0 };
	bool m_UseFixedColor = false;
	ID3D12DescriptorHeap *m_Rtv = nullptr;
	ID3D12Device *m_RtvDevice = nullptr;
};

unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool ok, const char *id, const std::string &detail )
{
	++g_Checks;
	if ( !ok )
	{
		++g_Failures;
		std::printf( "FAIL %s: %s\n", id, detail.c_str() );
	}
}

bool Near( std::uint8_t v, float expected )
{
	return std::abs( int( v ) - int( expected * 255.0f + 0.5f ) ) <= 3;
}

bool PresentAndCapture( IRenderDevice &device, IRenderPresentation &p,
    render_d3d12::Sdl3D3d12PresentationBridge &bridge, Sdl3D3d12Harness &harness, Color color,
    std::vector<std::uint8_t> *pixels, std::uint32_t *w, std::uint32_t *h )
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

bool UniformColor( const std::vector<std::uint8_t> &pixels, std::uint32_t w, std::uint32_t h, Color c )
{
	if ( pixels.size() < std::size_t( w ) * h * 4 || w == 0 || h == 0 )
		return false;
	const std::uint32_t xs[] = { 0, w / 2, w - 1 };
	const std::uint32_t ys[] = { 0, h / 2, h - 1 };
	for ( std::uint32_t y : ys )
		for ( std::uint32_t x : xs )
		{
			const std::uint8_t *px = &pixels[( std::size_t( y ) * w + x ) * 4];
			if ( !Near( px[0], c.r ) || !Near( px[1], c.g ) || !Near( px[2], c.b ) )
				return false;
		}
	return true;
}

// Real images in two windows on one device, including after a window resize.
// Each presentation's back buffer is sized from its window; the capture is the
// region presented, so it matches the drawable.
void CheckPixels( render_d3d12::D3d12RenderBackend &provider,
    render_d3d12::Sdl3D3d12PresentationBridge &bridge, Sdl3D3d12Harness &harness )
{
	IRenderDevice *device = provider.CreateDevice( RenderDeviceRequest(), nullptr );
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
	std::vector<std::uint8_t> pixels;
	std::uint32_t w = 0, h = 0;
	const Color red = { 1.0f, 0.0f, 0.0f }, blue = { 0.0f, 0.0f, 1.0f },
	            green = { 0.0f, 1.0f, 0.0f };

	bool okA = pa && PresentAndCapture( *device, *pa, bridge, harness, red, &pixels, &w, &h );
	Check( okA && UniformColor( pixels, w, h, red ) && RenderExtent{ w, h } == a->GetDrawableExtent(),
	    "pixels.window_a", "window A presents its red back buffer at its drawable size" );
	bool okB = pb && PresentAndCapture( *device, *pb, bridge, harness, blue, &pixels, &w, &h );
	Check( okB && UniformColor( pixels, w, h, blue ) && RenderExtent{ w, h } == b->GetDrawableExtent(),
	    "pixels.window_b", "window B presents its own blue back buffer (multi-view)" );
	Check( !UniformColor( pixels, w, h, red ), "pixels.negative_control",
	    "a blue capture must not match red" );

	harness.ResizeSurface( *a, RenderExtent{ 240, 360 } );
	if ( pa )
		pa->ResizeTo( a->GetDrawableExtent() );
	okA = pa && PresentAndCapture( *device, *pa, bridge, harness, green, &pixels, &w, &h );
	Check( okA && UniformColor( pixels, w, h, green ) && RenderExtent{ w, h } == a->GetDrawableExtent(),
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

// The render core's device (the endpoint's hosted port device) draws a frame
// into a presentation's back buffer, imported as a port texture whose home
// usage is kExternal, and the bridge presents it: render_lab's presenting
// path (render/lab/app) on D3D12. The captured swapchain shows the port's
// clear color; a second frame in another color shows the import is not a
// stale copy.
void CheckCorePresents( render_d3d12::D3d12RenderBackend &provider,
    render_d3d12::Sdl3D3d12PresentationBridge &bridge, Sdl3D3d12Harness &harness )
{
	namespace device = render::device;
	IRenderDevice *backend = provider.CreateDevice( RenderDeviceRequest(), nullptr );
	render_d3d12::D3d12DeviceEndpoint *endpoint = backend ? provider.FindDevice( *backend ) : nullptr;
	device::IRenderDevice2 *port = endpoint ? endpoint->Port() : nullptr;
	IRenderSurface *surface = harness.CreateSurface( RenderExtent{ 256, 192 } );
	IRenderPresentation *p =
	    port && surface ? bridge.CreatePresentation( *backend, *surface, RenderPresentationConfig(), nullptr )
	                    : nullptr;
	Check( port != nullptr && p != nullptr, "core.setup",
	    "the endpoint's port device and a presentation are created" );
	std::string step;
	auto frame = [&]( Color color ) -> bool
	{
		RenderResourceHandle back = kInvalidResource;
		if ( !p || p->BeginFrame( &back ) != RenderPresentStatus::kOk )
			return step = "BeginFrame", false;
		const RenderExtent extent = p->GetExtent();
		device::TextureDesc desc;
		desc.format = device::Format::kRGBA8Unorm;
		desc.width = extent.width;
		desc.height = extent.height;
		desc.usages = { device::ResourceUsage::kColorAttachment, device::ResourceUsage::kExternal };
		auto imported = device::d3d12::ImportTexture(
		    *port, endpoint->Texture( back ), desc, device::ResourceUsage::kExternal );
		if ( !imported )
			return step = "ImportTexture status " + std::to_string( int( imported.Error().status ) ), false;
		auto encoder = port->BeginEncoder( device::QueueKind::kGraphics );
		if ( !encoder )
			return step = "BeginEncoder", false;
		device::CommandEncoder &e = encoder.Value();
		e.TransitionTexture( imported.Value(), device::ResourceUsage::kExternal,
		    device::ResourceUsage::kColorAttachment );
		const device::ColorAttachment attachments[] = { { imported.Value(), device::LoadOp::kClear,
			device::StoreOp::kStore, { color.r, color.g, color.b, 1.0f }, {} } };
		device::RenderingDesc rendering;
		rendering.colors = attachments;
		rendering.width = extent.width;
		rendering.height = extent.height;
		e.BeginRendering( rendering );
		e.EndRendering();
		e.TransitionTexture( imported.Value(), device::ResourceUsage::kColorAttachment,
		    device::ResourceUsage::kExternal );
		auto token = port->Submit( device::QueueKind::kGraphics, { &e, 1 }, {} );
		if ( !token )
			return step = "Submit status " + std::to_string( int( token.Error().status ) ), false;
		// The port's work is on the bridge's queue before the present's copy.
		bridge.RequestCapture( *p );
		if ( p->Present() != RenderPresentStatus::kOk )
			return step = "Present", false;
		(void)port->Release( imported.Value(), token.Value() );
		std::vector<std::uint8_t> pixels;
		std::uint32_t w = 0, h = 0;
		if ( !bridge.ReadCapture( *p, &pixels, &w, &h ) )
			return step = "ReadCapture", false;
		if ( !UniformColor( pixels, w, h, color ) )
		{
			step = "pixel (0,0) " + std::to_string( pixels.empty() ? -1 : pixels[0] ) + "," +
			       std::to_string( pixels.size() > 1 ? pixels[1] : -1 ) + "," +
			       std::to_string( pixels.size() > 2 ? pixels[2] : -1 );
			return false;
		}
		return true;
	};
	const Color magenta = { 1.0f, 0.0f, 1.0f }, cyan = { 0.0f, 1.0f, 1.0f };
	const bool first = frame( magenta );
	Check( first, "core.presents",
	    "a frame the core's device clears into the imported back buffer is presented (" + step +
	        ")" );
	const bool second = frame( cyan );
	Check( second, "core.presents-next-frame",
	    "the next frame shows the core's new color (no stale import) (" + step + ")" );
	if ( p )
		bridge.DestroyPresentation( p );
	if ( port )
	{
		(void)port->WaitIdle();
		(void)port->Poll();
		Check( port->LiveResourceCount() == 0, "core.imports-released",
		    "the port device holds no import after its release" );
	}
	if ( surface )
	{
		harness.DestroyWindow( *surface );
		harness.DestroySurface( surface );
	}
	if ( backend )
	{
		while ( backend->PollCompletion() > 0 )
		{
		}
		Check( bridge.ReleaseDevice( *backend ), "core.release_device", "bridge releases the device" );
		provider.DestroyDevice( backend );
	}
}

} // namespace

int main( int, char ** )
{
	// A deadlock fails the suite instead of hanging the lane (the SDL3-Vulkan
	// test's alarm).
	std::thread( []
	    {
		    std::this_thread::sleep_for( std::chrono::seconds( 240 ) );
		    std::printf( "FAIL render.presentation.sdl3-d3d12: watchdog expired (deadlock)\n" );
		    std::fflush( stdout );
		    std::_Exit( 1 );
	    } )
	    .detach();
	if ( !SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::printf( "FAIL render.presentation.sdl3-d3d12: no display (%s)\n", SDL_GetError() );
		return testing::ReportConformance( 1, 1 );
	}
	std::string error;
	std::unique_ptr<render_d3d12::D3d12RenderBackend> provider =
	    render_d3d12::MakeD3d12RenderBackend( &error );
	if ( !provider )
	{
		std::printf( "FAIL render.presentation.sdl3-d3d12: %s\n", error.c_str() );
		SDL_Quit();
		return testing::ReportConformance( 1, 1 );
	}
	std::printf( "INFO render.presentation.sdl3-d3d12: video driver '%s'\n", SDL_GetCurrentVideoDriver() );
	{
		platform_sdl3::Sdl3RenderSurfaces surfaces;
		render_d3d12::Sdl3D3d12PresentationBridge bridge( *provider, surfaces, 4 );
		Sdl3D3d12Harness harness( surfaces, bridge, *provider );
		conformance::Report report;
		conformance::RunPresentationConformance( *provider, bridge, harness, report );
		for ( const conformance::CheckResult &c : report.checks )
		{
			++g_Checks;
			if ( !c.ok )
			{
				++g_Failures;
				std::printf( "FAIL %s: %s\n", c.id.c_str(), c.detail.c_str() );
			}
		}
		for ( const conformance::CheckResult &c : report.skipped )
			std::printf( "SKIP %s: %s\n", c.id.c_str(), c.detail.c_str() );
		std::printf( "INFO render.presentation.sdl3-d3d12: shared suite %d check(s), %d failure(s)\n",
		    int( report.checks.size() ), report.FailureCount() );
		Check( report.checks.size() >= 40, "suite.ran", "the shared suite ran its checks" );
		CheckPixels( *provider, bridge, harness );
		CheckCorePresents( *provider, bridge, harness );
	}
	Check( provider->GetLiveDeviceCount() == 0, "lifetime.no_leaked_devices", "all devices destroyed" );
	provider.reset();
	SDL_Quit();
	return testing::ReportConformance( g_Checks, g_Failures );
}
