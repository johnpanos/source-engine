//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The headless window system and the headless presentation bridge
//			(RFC 0001 rank 9 "headless-null" pair). HeadlessSurfaces models windows
//			with sizes, visibility, native-surface replacement and destruction, but
//			no display. The bridge presents from any device through the portable
//			device contract alone: back buffers are device resources and retirement
//			rides the device's ordered completion tokens, so the bridge proves the
//			presentation obligations without a GPU.
//
//			Injecting one HeadlessBridgeDefect makes the bridge violate exactly one
//			render.presentation.v1 obligation; the sensitivity main requires the
//			shared suite to detect each one.
//
//=============================================================================//

#ifndef RENDERTEST_HEADLESS_PRESENTATION_H
#define RENDERTEST_HEADLESS_PRESENTATION_H

#include "../conformance/render_presentation_conformance.h"
#include "render/render_presentation.h"

#include <memory>
#include <vector>

namespace rendertest
{

class HeadlessSurface;

// The headless window system. Owns its surfaces; the composition root (or test)
// owns this object and keeps it alive longer than every bridge using it.
class HeadlessSurfaces
{
public:
	HeadlessSurfaces();
	~HeadlessSurfaces();

	render::IRenderSurface *Create( render::RenderExtent extent );
	void Resize( render::IRenderSurface &surface, render::RenderExtent extent );
	void SetVisible( render::IRenderSurface &surface, bool visible );
	void ReleaseNativeSurface( render::IRenderSurface &surface );
	void RestoreNativeSurface( render::IRenderSurface &surface );
	void DestroyWindow( render::IRenderSurface &surface );
	void Destroy( render::IRenderSurface *surface );

	// -- Private endpoint for the headless bridge --
	// True when 'surface' is a live surface this window system created.
	bool Owns( const render::IRenderSurface &surface ) const;
	// A presentation reports holding (or dropping) a native binding on a surface.
	void SetNativeBinding( const render::IRenderSurface &surface, bool bound );
	uint32_t GetNativeLifetimeViolations() const { return m_Violations; }

private:
	HeadlessSurface *Find( const render::IRenderSurface &surface ) const;

	std::vector<HeadlessSurface *> m_Surfaces;
	uint32_t m_Violations;
};

// One flag per render.presentation.v1 obligation. A correct bridge leaves every
// field false.
struct HeadlessBridgeDefects
{
	bool recycleOnResize = false;			// completion.retired_until_complete
	bool resizeLosesDevice = false;			// resize.keeps_device
	bool fatalOnZeroSize = false;			// zero.backbuffer_suspends
	bool ignoreSurfaceDestroyed = false;	// loss.window_destroyed
	bool ignoreSurfaceUnavailable = false;	// surface.unavailable_suspends
	bool sharedExtent = false;				// multi.independent_resize
	bool acceptForeignDevice = false;		// create.foreign_device
	bool allowSecondPresentation = false;	// create.surface_busy
	bool leakOnDestroy = false;				// lifetime.device_resources_baseline
	bool keepListenerAttached = false;		// lifetime.surface_detached
	bool noLimit = false;					// multi.over_limit
	bool keepNativeAfterRelease = false;	// lifetime.native_released_before_window
	bool resizeDuringFrame = false;			// resize.rejected_during_frame
	bool releaseDeviceWhileLive = false;	// lifetime.release_device_order
};

// Builds the bridge joining 'surfaces' to 'provider'. Both must outlive it.
std::unique_ptr<render::IRenderPresentationBridgeFactory> MakeHeadlessPresentationBridge(
	render::IRenderBackendProvider &provider, HeadlessSurfaces &surfaces,
	uint32_t maxPresentations = 4, HeadlessBridgeDefects defects = HeadlessBridgeDefects{} );

// Drives HeadlessSurfaces for the shared suite. A null device completes nothing
// until PollCompletion, so GPU completion is inherently held.
class HeadlessHarness : public render::conformance::IPresentationHarness
{
public:
	explicit HeadlessHarness( HeadlessSurfaces &surfaces ) : m_Surfaces( surfaces ) {}

	render::IRenderSurface *CreateSurface( render::RenderExtent extent ) override;
	bool ResizeSurface( render::IRenderSurface &surface, render::RenderExtent extent ) override;
	void SetSurfaceVisible( render::IRenderSurface &surface, bool visible ) override;
	void ReleaseNativeSurface( render::IRenderSurface &surface ) override;
	void RestoreNativeSurface( render::IRenderSurface &surface ) override;
	void DestroyWindow( render::IRenderSurface &surface ) override;
	void DestroySurface( render::IRenderSurface *surface ) override;
	uint32_t GetMaxSurfaces() const override { return 8; }
	uint32_t GetNativeLifetimeViolations() const override;
	bool HoldGpuCompletion( render::IRenderDevice &device ) override;
	void ReleaseGpuCompletion( render::IRenderDevice &device ) override;
	bool IsGpuWorkPending( render::IRenderDevice &device ) override;

private:
	HeadlessSurfaces &m_Surfaces;
	uint64_t m_HeldAt = 0;
	bool m_Held = false;
};

} // namespace rendertest

#endif // RENDERTEST_HEADLESS_PRESENTATION_H
