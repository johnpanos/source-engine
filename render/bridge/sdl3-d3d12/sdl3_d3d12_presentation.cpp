//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Direct3D 12 presentation bridge (RFC 0024 X4); see
//			sdl3_d3d12_presentation.h.
//
//=============================================================================//

#include "sdl3_d3d12_presentation.h"

#include "../../../platform/sdl3/render_surface/sdl3_render_surfaces.h"
#include "../../device/d3d12/backend_v1/render_backend_v1.h"

#include <SDL3/SDL.h>
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace render_d3d12
{

using namespace render;

namespace
{

constexpr UINT kSwapchainBuffers = 3;

DXGI_FORMAT BackBufferFormat( RenderColorFormat format )
{
	switch ( format )
	{
	case RenderColorFormat::kRGBA8Unorm:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RenderColorFormat::kRGBA8Srgb:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case RenderColorFormat::kBGRA8Unorm:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case RenderColorFormat::kRGBA16Float:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}

// Flip-model swapchains take no sRGB format: the sRGB back buffer's bytes are
// copied into its UNORM twin, which displays them unchanged.
DXGI_FORMAT SwapchainFormat( RenderColorFormat format )
{
	return format == RenderColorFormat::kRGBA8Srgb ? DXGI_FORMAT_R8G8B8A8_UNORM
	                                               : BackBufferFormat( format );
}

void Fill( RenderCreateError *error, RenderCreateStatus status, const char *message )
{
	if ( !error )
		return;
	error->status = status;
	std::snprintf( error->message, sizeof( error->message ), "%s", message );
}

// D3D12_BRIDGE_TRACE=1: native failures on stderr, for diagnosis.
void Trace( const char *what, HRESULT result )
{
	if ( std::getenv( "D3D12_BRIDGE_TRACE" ) )
		std::fprintf( stderr, "render.bridge.sdl3-d3d12: %s failed (0x%08lx)\n", what,
		    static_cast<unsigned long>( result ) );
}

HWND WindowHandle( SDL_Window *window )
{
	return static_cast<HWND>( SDL_GetPointerProperty(
	    SDL_GetWindowProperties( window ), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );
}

void Barrier( ID3D12GraphicsCommandList &list, ID3D12Resource *resource,
    D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after )
{
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	list.ResourceBarrier( 1, &barrier );
}

} // namespace

// A swapchain kept after its presentation, until its queue is idle. It
// listens to its surface in the presentation's place, so it is released
// before the window's native surface goes (the contract's "no later than
// the surface's next native release").
struct ParkedSwapchain final : public IRenderSurfaceListener
{
	ParkedSwapchain( Sdl3D3d12PresentationBridge *bridge_, IDXGISwapChain3 *swapchain_,
	    D3d12DeviceEndpoint *endpoint_, IRenderDevice *device_, IRenderSurface *surface_,
	    std::uint64_t lastUse_ )
	    : bridge( bridge_ ), swapchain( swapchain_ ), endpoint( endpoint_ ), device( device_ ),
	      surface( surface_ ), lastUse( lastUse_ )
	{
	}
	void OnNativeSurfaceReleasing() override { bridge->CollectParked( true, surface, nullptr ); }

	Sdl3D3d12PresentationBridge *bridge;
	IDXGISwapChain3 *swapchain;
	D3d12DeviceEndpoint *endpoint;
	IRenderDevice *device;
	IRenderSurface *surface;
	std::uint64_t lastUse;
	bool listening = false;
};

class Sdl3D3d12Presentation final : public IRenderPresentation, public IRenderSurfaceListener
{
public:
	Sdl3D3d12Presentation( Sdl3D3d12PresentationBridge &bridge, IRenderDevice &device,
	    D3d12DeviceEndpoint &endpoint, IRenderSurface &surface,
	    const RenderPresentationConfig &config, RenderExtent extent )
	    : m_Bridge( bridge ), m_Device( device ), m_Endpoint( endpoint ), m_Surface( surface ),
	      m_Config( config ), m_Extent( extent ), m_Generation( surface.GetGeneration() )
	{
	}

	~Sdl3D3d12Presentation() override { ReleaseReadback(); }

	IRenderDevice &Device() const { return m_Device; }
	IRenderSurface &Surface() const { return m_Surface; }

	// Builds the swapchain; false (with *error) when the window cannot show the
	// requested range.
	bool BuildSwapchain( RenderCreateError *error )
	{
		SDL_Window *window = m_Bridge.Surfaces().NativeWindow( m_Surface );
		HWND hwnd = window ? WindowHandle( window ) : nullptr;
		if ( !hwnd )
		{
			Fill( error, RenderCreateStatus::kSurfaceIncompatible, "the window has no HWND" );
			return false;
		}
		// A destroyed presentation's swapchain on this window must go first:
		// one flip-model swapchain per window.
		m_Bridge.CollectParked( true, &m_Surface, nullptr );
		const RenderExtent drawable = m_Surface.GetDrawableExtent();
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = std::max( 1u, drawable.width );
		desc.Height = std::max( 1u, drawable.height );
		desc.Format = SwapchainFormat( m_Config.format );
		desc.SampleDesc.Count = 1;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = kSwapchainBuffers;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Scaling = DXGI_SCALING_STRETCH;
		IDXGISwapChain1 *created = nullptr;
		const HRESULT result = m_Endpoint.Factory()->CreateSwapChainForHwnd(
		    m_Endpoint.Queue(), hwnd, &desc, nullptr, nullptr, &created );
		if ( FAILED( result ) || !created )
		{
			Trace( "CreateSwapChainForHwnd", result );
			Fill( error, RenderCreateStatus::kSurfaceIncompatible,
			    "CreateSwapChainForHwnd failed" );
			return false;
		}
		created->QueryInterface( IID_IDXGISwapChain3, reinterpret_cast<void **>( &m_Swapchain ) );
		created->Release();
		m_Endpoint.Factory()->MakeWindowAssociation( hwnd, DXGI_MWA_NO_ALT_ENTER );
		if ( m_Config.dynamicRange == RenderDynamicRange::kExtendedLinear )
		{
			UINT support = 0;
			const DXGI_COLOR_SPACE_TYPE scRgb = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
			if ( FAILED( m_Swapchain->CheckColorSpaceSupport( scRgb, &support ) ) ||
			     !( support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT ) ||
			     FAILED( m_Swapchain->SetColorSpace1( scRgb ) ) )
			{
				m_Swapchain->Release();
				m_Swapchain = nullptr;
				Fill( error, RenderCreateStatus::kSurfaceIncompatible,
				    "the display cannot show scRGB (extended-linear) output" );
				return false;
			}
		}
		m_SwapchainExtent = { desc.Width, desc.Height };
		m_Generation = m_Surface.GetGeneration();
		return true;
	}

	// -- IRenderPresentation --

	RenderExtent GetExtent() const override { return m_Extent; }

	bool ResizeTo( RenderExtent extent ) override
	{
		if ( m_FrameOpen )
			return false;
		CollectRetired();
		if ( extent != m_Extent )
		{
			RetireBackBuffer();
			m_Extent = extent;
		}
		return true;
	}

	RenderPresentStatus BeginFrame( RenderResourceHandle *outBackBuffer ) override
	{
		if ( outBackBuffer )
			*outBackBuffer = kInvalidResource;
		if ( m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		CollectRetired();
		if ( m_Device.GetState() == RenderDeviceState::kDeviceLost ||
		     m_Device.GetState() == RenderDeviceState::kFatal ||
		     m_Surface.GetStatus() == RenderSurfaceStatus::kDestroyed )
			return RenderPresentStatus::kLost;
		if ( m_Surface.GetStatus() != RenderSurfaceStatus::kAvailable ||
		     !m_Extent.IsPresentable() || !m_Surface.GetDrawableExtent().IsPresentable() )
			return RenderPresentStatus::kSuspended;
		// A replaced native surface (or one released and restored) needs a new
		// swapchain.
		if ( !m_Swapchain || m_Surface.GetGeneration() != m_Generation )
		{
			ReleaseSwapchain( true );
			if ( !BuildSwapchain( nullptr ) )
				return RenderPresentStatus::kRecoverable;
		}
		if ( m_BackBuffer == kInvalidResource )
		{
			m_BackBuffer = m_Endpoint.CreateTexture(
			    m_Extent.width, m_Extent.height, BackBufferFormat( m_Config.format ) );
			if ( m_BackBuffer == kInvalidResource )
				return RenderPresentStatus::kRecoverable;
		}
		m_FrameOpen = true;
		if ( outBackBuffer )
			*outBackBuffer = m_BackBuffer;
		return RenderPresentStatus::kOk;
	}

	RenderPresentStatus Present() override
	{
		if ( !m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		m_FrameOpen = false;
		if ( m_Surface.GetStatus() == RenderSurfaceStatus::kDestroyed ||
		     m_Device.GetState() != RenderDeviceState::kAvailable )
			return RenderPresentStatus::kLost;
		// The swapchain follows the window's drawable size, once the GPU is
		// done with its buffers.
		const RenderExtent drawable = m_Surface.GetDrawableExtent();
		if ( drawable.IsPresentable() &&
		     ( drawable.width != m_SwapchainExtent.width ||
		         drawable.height != m_SwapchainExtent.height ) )
		{
			if ( !m_Endpoint.WaitFor( m_SwapchainLastUse ) ||
			     FAILED( m_Swapchain->ResizeBuffers( kSwapchainBuffers, drawable.width,
			         drawable.height, SwapchainFormat( m_Config.format ), 0 ) ) )
				return RenderPresentStatus::kRecoverable;
			m_SwapchainExtent = drawable;
		}
		ID3D12Resource *target = nullptr;
		if ( FAILED( m_Swapchain->GetBuffer( m_Swapchain->GetCurrentBackBufferIndex(),
		         IID_ID3D12Resource, reinterpret_cast<void **>( &target ) ) ) )
			return RenderPresentStatus::kRecoverable;
		ID3D12Resource *source = m_Endpoint.Texture( m_BackBuffer );
		const UINT width = std::min( m_Extent.width, m_SwapchainExtent.width );
		const UINT height = std::min( m_Extent.height, m_SwapchainExtent.height );
		const bool capture = m_CaptureRequested && m_Config.format != RenderColorFormat::kRGBA16Float;
		if ( capture )
			PrepareReadback( width, height );
		IRenderCompletionToken *token = m_Endpoint.SubmitRecorded(
		    [&]( ID3D12GraphicsCommandList &list )
		    {
			    Barrier( list, source, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE );
			    Barrier( list, target, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST );
			    D3D12_TEXTURE_COPY_LOCATION to{};
			    to.pResource = target;
			    to.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			    D3D12_TEXTURE_COPY_LOCATION from{};
			    from.pResource = source;
			    from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			    const D3D12_BOX box{ 0, 0, 0, width, height, 1 };
			    list.CopyTextureRegion( &to, 0, 0, 0, &from, &box );
			    if ( capture )
			    {
				    Barrier( list, target, D3D12_RESOURCE_STATE_COPY_DEST,
				        D3D12_RESOURCE_STATE_COPY_SOURCE );
				    D3D12_TEXTURE_COPY_LOCATION out{};
				    out.pResource = m_Readback;
				    out.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				    out.PlacedFootprint.Footprint.Format = SwapchainFormat( m_Config.format );
				    out.PlacedFootprint.Footprint.Width = width;
				    out.PlacedFootprint.Footprint.Height = height;
				    out.PlacedFootprint.Footprint.Depth = 1;
				    out.PlacedFootprint.Footprint.RowPitch = m_ReadbackPitch;
				    to.SubresourceIndex = 0;
				    list.CopyTextureRegion( &out, 0, 0, 0, &to, &box );
				    Barrier( list, target, D3D12_RESOURCE_STATE_COPY_SOURCE,
				        D3D12_RESOURCE_STATE_PRESENT );
			    }
			    else
			    {
				    Barrier( list, target, D3D12_RESOURCE_STATE_COPY_DEST,
				        D3D12_RESOURCE_STATE_PRESENT );
			    }
			    Barrier( list, source, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON );
		    },
		    &m_BackBuffer, 1 );
		target->Release();
		if ( !token )
			return RenderPresentStatus::kRecoverable;
		m_SwapchainLastUse = m_Endpoint.TokenValue( *token );
		if ( capture )
		{
			m_CaptureRequested = false;
			m_CaptureValue = m_SwapchainLastUse;
			m_CaptureExtent = { width, height };
		}
		const HRESULT presented = m_Swapchain->Present( m_Config.vsync ? 1 : 0, 0 );
		// The present is queued after the copy's signal: the swapchain's last
		// use is a signal after it, so releasing a parked swapchain never waits
		// on a present still queued (behind a held gate, for one).
		if ( IRenderCompletionToken *after = m_Endpoint.SubmitRecorded( {}, nullptr, 0 ) )
			m_SwapchainLastUse = m_Endpoint.TokenValue( *after );
		if ( presented == DXGI_ERROR_DEVICE_REMOVED || presented == DXGI_ERROR_DEVICE_RESET )
			return RenderPresentStatus::kLost;
		if ( FAILED( presented ) )
		{
			Trace( "Present", presented );
			ReleaseSwapchain( true );
			return RenderPresentStatus::kRecoverable;
		}
		return RenderPresentStatus::kOk;
	}

	RenderPresentStatus CancelFrame() override
	{
		if ( !m_FrameOpen )
			return RenderPresentStatus::kInvalidSequence;
		m_FrameOpen = false;
		return RenderPresentStatus::kOk;
	}

	std::size_t GetPendingRetirementCount() const override { return m_Retired.size(); }

	void CollectRetired() override
	{
		m_Device.CollectCompletedDestructions();
		m_Retired.erase( std::remove_if( m_Retired.begin(), m_Retired.end(),
		                     [this]( const Retired &r )
		                     {
			                     return !m_Device.IsResourceLive( r.handle );
		                     } ),
		    m_Retired.end() );
	}

	RenderDynamicRangeState GetDynamicRange() const override
	{
		RenderDynamicRangeState state;
		state.range = m_Config.dynamicRange;
		return state;
	}

	// -- IRenderSurfaceListener --

	void OnNativeSurfaceReleasing() override { ReleaseSwapchain( true ); }

	// -- For the bridge --

	// Retires the back buffer and hands the swapchain to the bridge.
	void Retire()
	{
		m_FrameOpen = false;
		RetireBackBuffer();
		m_Surface.DetachListener( *this ); // first: the parked swapchain listens next
		if ( m_Swapchain )
		{
			auto *parked = new ParkedSwapchain( &m_Bridge, m_Swapchain, &m_Endpoint, &m_Device,
			    &m_Surface, m_SwapchainLastUse );
			m_Swapchain = nullptr;
			m_Bridge.Park( parked );
		}
	}

	bool HasSwapchain() const { return m_Swapchain != nullptr; }

	void RequestCapture() { m_CaptureRequested = true; }

	bool ReadCapture( std::vector<std::uint8_t> *outRgba, std::uint32_t *outWidth,
	    std::uint32_t *outHeight )
	{
		if ( !m_Readback || m_CaptureValue == 0 || !m_Endpoint.WaitFor( m_CaptureValue ) )
			return false;
		void *mapped = nullptr;
		const D3D12_RANGE range{ 0, SIZE_T( m_ReadbackPitch ) * m_CaptureExtent.height };
		if ( FAILED( m_Readback->Map( 0, &range, &mapped ) ) )
			return false;
		const bool bgra = m_Config.format == RenderColorFormat::kBGRA8Unorm;
		outRgba->resize( std::size_t( m_CaptureExtent.width ) * m_CaptureExtent.height * 4 );
		for ( std::uint32_t y = 0; y < m_CaptureExtent.height; ++y )
		{
			const auto *row = static_cast<const std::uint8_t *>( mapped ) + y * m_ReadbackPitch;
			std::uint8_t *out = outRgba->data() + std::size_t( y ) * m_CaptureExtent.width * 4;
			std::memcpy( out, row, std::size_t( m_CaptureExtent.width ) * 4 );
			for ( std::uint32_t x = 0; bgra && x < m_CaptureExtent.width; ++x )
				std::swap( out[x * 4], out[x * 4 + 2] );
		}
		const D3D12_RANGE none{ 0, 0 };
		m_Readback->Unmap( 0, &none );
		*outWidth = m_CaptureExtent.width;
		*outHeight = m_CaptureExtent.height;
		return true;
	}

private:
	struct Retired
	{
		RenderResourceHandle handle;
	};

	void RetireBackBuffer()
	{
		if ( m_BackBuffer == kInvalidResource )
			return;
		// An empty submission after every use of the back buffer: its token
		// completes only after they do (one queue, in order).
		if ( IRenderCompletionToken *token = m_Endpoint.SubmitRecorded( {}, &m_BackBuffer, 1 ) )
		{
			m_Device.DestroyResourceWhenComplete( m_BackBuffer, *token );
			m_Retired.push_back( { m_BackBuffer } );
		}
		m_BackBuffer = kInvalidResource;
	}

	// Releasing a swapchain drains its queue (see CollectParked): with work
	// held on the GPU it is parked instead, and released at a later
	// collection.
	void ReleaseSwapchain( bool wait )
	{
		if ( !m_Swapchain )
			return;
		if ( wait && m_Endpoint.WaitFor( m_Endpoint.LastSubmitted() ) )
		{
			m_Swapchain->Release();
			m_Swapchain = nullptr;
			return;
		}
		m_Bridge.Park( new ParkedSwapchain(
		    &m_Bridge, m_Swapchain, &m_Endpoint, &m_Device, &m_Surface, m_SwapchainLastUse ) );
		m_Swapchain = nullptr;
	}

	void PrepareReadback( UINT width, UINT height )
	{
		const UINT pitch = ( width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1 ) &
		                   ~UINT( D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1 );
		const UINT64 bytes = UINT64( pitch ) * height;
		if ( m_Readback && m_ReadbackBytes >= bytes )
		{
			m_ReadbackPitch = pitch;
			return;
		}
		ReleaseReadback();
		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_READBACK;
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = bytes;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		if ( SUCCEEDED( m_Endpoint.NativeDevice()->CreateCommittedResource( &heap,
		         D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		         IID_ID3D12Resource, reinterpret_cast<void **>( &m_Readback ) ) ) )
		{
			m_ReadbackBytes = bytes;
			m_ReadbackPitch = pitch;
		}
	}

	void ReleaseReadback()
	{
		if ( m_Readback )
		{
			(void)m_Endpoint.WaitFor( m_CaptureValue );
			m_Readback->Release();
		}
		m_Readback = nullptr;
		m_ReadbackBytes = 0;
	}

	Sdl3D3d12PresentationBridge &m_Bridge;
	IRenderDevice &m_Device;
	D3d12DeviceEndpoint &m_Endpoint;
	IRenderSurface &m_Surface;
	RenderPresentationConfig m_Config;
	RenderExtent m_Extent;
	RenderExtent m_SwapchainExtent;
	std::uint64_t m_Generation;
	IDXGISwapChain3 *m_Swapchain = nullptr;
	std::uint64_t m_SwapchainLastUse = 0;
	RenderResourceHandle m_BackBuffer = kInvalidResource;
	std::vector<Retired> m_Retired;
	bool m_FrameOpen = false;
	bool m_CaptureRequested = false;
	ID3D12Resource *m_Readback = nullptr;
	UINT64 m_ReadbackBytes = 0;
	UINT m_ReadbackPitch = 0;
	std::uint64_t m_CaptureValue = 0;
	RenderExtent m_CaptureExtent;
};

// Bridge --------------------------------------------------------------------------

Sdl3D3d12PresentationBridge::Sdl3D3d12PresentationBridge( D3d12RenderBackend &provider,
    platform_sdl3::Sdl3RenderSurfaces &surfaces, std::uint32_t maxPresentations )
    : m_Provider( provider ), m_Surfaces( surfaces ), m_Max( maxPresentations )
{
}

Sdl3D3d12PresentationBridge::~Sdl3D3d12PresentationBridge()
{
	for ( Sdl3D3d12Presentation *presentation : m_Live )
	{
		presentation->Retire();
		delete presentation;
	}
	m_Live.clear();
	CollectParked( true, nullptr, nullptr );
}

RenderPresentationPairId Sdl3D3d12PresentationBridge::GetPairId() const
{
	RenderPresentationPairId pair;
	pair.windowSystem = "sdl3";
	pair.renderBackend = "d3d12";
	return pair;
}

IRenderPresentation *Sdl3D3d12PresentationBridge::CreatePresentation( IRenderDevice &device,
    IRenderSurface &surface, const RenderPresentationConfig &config, RenderCreateError *error )
{
	if ( error )
		*error = RenderCreateError();
	D3d12DeviceEndpoint *endpoint = m_Provider.OwnsDevice( device ) ? m_Provider.FindDevice( device )
	                                                                : nullptr;
	if ( !endpoint )
	{
		Fill( error, RenderCreateStatus::kForeignObject, "the device is not render_d3d12's" );
		return nullptr;
	}
	if ( !m_Surfaces.Owns( surface ) )
	{
		Fill( error, RenderCreateStatus::kForeignObject, "the surface is not platform_sdl3's" );
		return nullptr;
	}
	if ( config.dynamicRange == RenderDynamicRange::kExtendedLinear &&
	     config.format != RenderColorFormat::kRGBA16Float )
	{
		Fill( error, RenderCreateStatus::kInvalidConfig,
		    "kExtendedLinear needs a kRGBA16Float back buffer" );
		return nullptr;
	}
	if ( surface.GetStatus() == RenderSurfaceStatus::kDestroyed )
	{
		Fill( error, RenderCreateStatus::kSurfaceLost, "the window is destroyed" );
		return nullptr;
	}
	if ( device.GetState() != RenderDeviceState::kAvailable )
	{
		Fill( error, RenderCreateStatus::kDeviceUnavailable, "the device is not available" );
		return nullptr;
	}
	if ( m_Live.size() >= m_Max )
	{
		Fill( error, RenderCreateStatus::kTooManyPresentations, "the presentation limit is reached" );
		return nullptr;
	}
	const RenderExtent extent =
	    config.extent.IsPresentable() ? config.extent : surface.GetDrawableExtent();
	auto *presentation =
	    new Sdl3D3d12Presentation( *this, device, *endpoint, surface, config, extent );
	// A destroyed presentation's swapchain on this window goes first (it holds
	// the surface's listener and the window's one flip-model swapchain).
	CollectParked( true, &surface, nullptr );
	if ( !surface.AttachListener( *presentation ) )
	{
		delete presentation;
		Fill( error, RenderCreateStatus::kSurfaceBusy, "the surface has a presentation" );
		return nullptr;
	}
	if ( surface.GetStatus() == RenderSurfaceStatus::kAvailable &&
	     !presentation->BuildSwapchain( error ) )
	{
		surface.DetachListener( *presentation );
		delete presentation;
		return nullptr;
	}
	m_Live.push_back( presentation );
	return presentation;
}

void Sdl3D3d12PresentationBridge::DestroyPresentation( IRenderPresentation *presentation )
{
	Sdl3D3d12Presentation *mine = presentation ? Find( *presentation ) : nullptr;
	if ( !mine )
		return;
	m_Live.erase( std::find( m_Live.begin(), m_Live.end(), mine ) );
	IRenderSurface &surface = mine->Surface();
	mine->Retire();
	delete mine;
	// Releasing a swapchain drains its queue under vkd3d-proton whatever the
	// bridge does, so waiting for the device first changes nothing but the
	// order; it keeps the surface's listener slot free. Only a held queue
	// (WaitFor returns at once) leaves the swapchain parked, listening to the
	// surface in the presentation's place.
	CollectParked( true, &surface, nullptr );
	CollectParked( false, nullptr, nullptr );
}

bool Sdl3D3d12PresentationBridge::ReleaseDevice( IRenderDevice &device )
{
	for ( const Sdl3D3d12Presentation *presentation : m_Live )
	{
		if ( &presentation->Device() == &device )
			return false;
	}
	CollectParked( true, nullptr, &device );
	return true;
}

std::size_t Sdl3D3d12PresentationBridge::NativeSurfaceCount( const IRenderSurface &surface ) const
{
	std::size_t count = 0;
	for ( const Sdl3D3d12Presentation *presentation : m_Live )
		count += &presentation->Surface() == &surface && presentation->HasSwapchain() ? 1 : 0;
	for ( const ParkedSwapchain *parked : m_Parked )
		count += parked->surface == &surface ? 1 : 0;
	return count;
}

bool Sdl3D3d12PresentationBridge::RequestCapture( IRenderPresentation &presentation )
{
	Sdl3D3d12Presentation *mine = Find( presentation );
	if ( mine )
		mine->RequestCapture();
	return mine != nullptr;
}

bool Sdl3D3d12PresentationBridge::ReadCapture( IRenderPresentation &presentation,
    std::vector<std::uint8_t> *outRgba, std::uint32_t *outWidth, std::uint32_t *outHeight )
{
	Sdl3D3d12Presentation *mine = Find( presentation );
	return mine && mine->ReadCapture( outRgba, outWidth, outHeight );
}

void Sdl3D3d12PresentationBridge::Park( ParkedSwapchain *parked )
{
	// A presentation parking during its own listener callback is still
	// attached; the parked entry listens once the surface is free.
	parked->listening = parked->surface->AttachListener( *parked );
	m_Parked.push_back( parked );
}

void Sdl3D3d12PresentationBridge::CollectParked(
    bool wait, const IRenderSurface *onlySurface, IRenderDevice *onlyDevice )
{
	for ( auto it = m_Parked.begin(); it != m_Parked.end(); )
	{
		ParkedSwapchain *parked = *it;
		const bool selected = ( !onlySurface || parked->surface == onlySurface ) &&
		                      ( !onlyDevice || parked->device == onlyDevice );
		// Releasing a swapchain waits for its whole queue (vkd3d-proton drains
		// it), so a parked one goes only once the device has no incomplete
		// work; a held queue keeps it parked until a later collection.
		const bool idle = !parked->endpoint->HasIncompleteGpuWork() ||
		                  ( wait && parked->endpoint->WaitFor( parked->endpoint->LastSubmitted() ) );
		const bool done = selected && idle;
		if ( !done )
		{
			++it;
			continue;
		}
		parked->swapchain->Release();
		if ( parked->listening )
			parked->surface->DetachListener( *parked );
		delete parked;
		it = m_Parked.erase( it );
	}
}

Sdl3D3d12Presentation *Sdl3D3d12PresentationBridge::Find( IRenderPresentation &presentation ) const
{
	for ( Sdl3D3d12Presentation *mine : m_Live )
	{
		if ( static_cast<IRenderPresentation *>( mine ) == &presentation )
			return mine;
	}
	return nullptr;
}

} // namespace render_d3d12
