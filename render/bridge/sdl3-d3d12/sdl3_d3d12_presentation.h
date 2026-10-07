//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Direct3D 12 presentation bridge (RFC 0024 X4,
//			render.presentation.v1): joins platform_sdl3's surfaces and
//			render_d3d12's render.backend.v1 devices. The only code that sees the
//			window's HWND or a DXGI swapchain.
//
//			A presentation renders into back buffers of its own (device
//			textures of its extent); Present copies the overlapping region into
//			the current buffer of a DXGI flip-model swapchain on the window's
//			HWND and presents, after the frame's work on the same queue. So a
//			resize retires the old back buffer behind a completion token and
//			never waits for the GPU, and the swapchain follows the window's
//			drawable size on its own: it is resized (ResizeBuffers) only after
//			its last use completes. kExtendedLinear is an RGBA16F swapchain in
//			scRGB (DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709); a display that
//			cannot show it fails creation with kSurfaceIncompatible.
//
//			Swapchains of destroyed presentations are parked with the fence
//			value of their last use and released when it completes, before the
//			surface's next native release, or at ReleaseDevice. When SDL takes a
//			window's native surface away, the presentation waits for its
//			swapchain's last use and releases it before returning.
//
//=============================================================================//

#ifndef RENDER_BRIDGE_SDL3_D3D12_PRESENTATION_H
#define RENDER_BRIDGE_SDL3_D3D12_PRESENTATION_H

#include "render/render_presentation.h"

#include <cstdint>
#include <vector>

namespace platform_sdl3
{
class Sdl3RenderSurfaces;
}

namespace render_d3d12
{

class D3d12RenderBackend;
class Sdl3D3d12Presentation;
struct ParkedSwapchain;

class Sdl3D3d12PresentationBridge : public render::IRenderPresentationBridgeFactory
{
public:
	// Both providers must outlive the bridge.
	Sdl3D3d12PresentationBridge( D3d12RenderBackend &provider,
	    platform_sdl3::Sdl3RenderSurfaces &surfaces, std::uint32_t maxPresentations = 4 );
	~Sdl3D3d12PresentationBridge() override;

	Sdl3D3d12PresentationBridge( const Sdl3D3d12PresentationBridge & ) = delete;
	Sdl3D3d12PresentationBridge &operator=( const Sdl3D3d12PresentationBridge & ) = delete;

	render::RenderPresentationPairId GetPairId() const override;
	std::uint32_t GetMaxPresentations() const override { return m_Max; }
	render::IRenderPresentation *CreatePresentation( render::IRenderDevice &device,
	    render::IRenderSurface &surface, const render::RenderPresentationConfig &config,
	    render::RenderCreateError *error ) override;
	void DestroyPresentation( render::IRenderPresentation *presentation ) override;
	std::size_t GetLivePresentationCount() const override { return m_Live.size(); }
	bool ReleaseDevice( render::IRenderDevice &device ) override;

	// -- Native test endpoint --
	// Swapchains alive for 'surface' (a live presentation's and parked ones).
	std::size_t NativeSurfaceCount( const render::IRenderSurface &surface ) const;
	// Copies what the next Present shows into host memory; ReadCapture waits
	// for that frame's completion and returns tightly packed RGBA8 (an 8-bit
	// presentation only).
	bool RequestCapture( render::IRenderPresentation &presentation );
	bool ReadCapture( render::IRenderPresentation &presentation, std::vector<std::uint8_t> *outRgba,
	    std::uint32_t *outWidth, std::uint32_t *outHeight );

	// For presentations.
	void Park( ParkedSwapchain *parked );
	void CollectParked( bool wait, const render::IRenderSurface *onlySurface,
	    render::IRenderDevice *onlyDevice );
	platform_sdl3::Sdl3RenderSurfaces &Surfaces() { return m_Surfaces; }

private:
	Sdl3D3d12Presentation *Find( render::IRenderPresentation &presentation ) const;

	D3d12RenderBackend &m_Provider;
	platform_sdl3::Sdl3RenderSurfaces &m_Surfaces;
	std::uint32_t m_Max;
	std::vector<Sdl3D3d12Presentation *> m_Live;
	std::vector<ParkedSwapchain *> m_Parked;
};

} // namespace render_d3d12

#endif // RENDER_BRIDGE_SDL3_D3D12_PRESENTATION_H
