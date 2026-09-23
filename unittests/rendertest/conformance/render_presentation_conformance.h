//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared presentation-bridge conformance suite (RFC 0001 rank 9,
//			roadmap R16; verified under RFC 0005 Q-PRESENTATION). One suite runs
//			against every bridge that claims render.presentation.v1: the headless
//			bridge over the null provider, and the SDL3-Vulkan bridge over real
//			windows and a real GPU. A harness drives the window-system side (create,
//			resize, hide, take away and restore the native surface, destroy the
//			window) and can hold GPU completion, so "storage is recycled only after
//			GPU completion" is tested against genuinely incomplete work.
//
//			The suite never sees a native handle. It records structured results
//			and never aborts, so a sensitivity run can require a broken bridge to
//			fail the specific obligation it violates.
//
//=============================================================================//

#ifndef RENDERTEST_RENDER_PRESENTATION_CONFORMANCE_H
#define RENDERTEST_RENDER_PRESENTATION_CONFORMANCE_H

#include "render_backend_conformance.h"
#include "render/render_presentation.h"

namespace render
{
namespace conformance
{

// The window-system side of one bridge pair, as a test sees it.
class IPresentationHarness
{
public:
	virtual ~IPresentationHarness() = default;

	// Opens a window with this drawable size and returns its surface, or nullptr.
	virtual IRenderSurface *CreateSurface( RenderExtent extent ) = 0;

	// Resizes the window. Returns false when the window system refused.
	virtual bool ResizeSurface( IRenderSurface &surface, RenderExtent extent ) = 0;

	// Hides (zero drawable extent) or shows the window.
	virtual void SetSurfaceVisible( IRenderSurface &surface, bool visible ) = 0;

	// Takes the native surface away (as a mobile platform does in the background)
	// and later provides a new one.
	virtual void ReleaseNativeSurface( IRenderSurface &surface ) = 0;
	virtual void RestoreNativeSurface( IRenderSurface &surface ) = 0;

	// Destroys the window; the surface object stays valid until DestroySurface.
	virtual void DestroyWindow( IRenderSurface &surface ) = 0;
	virtual void DestroySurface( IRenderSurface *surface ) = 0;

	// Maximum windows the harness can open at once.
	virtual uint32_t GetMaxSurfaces() const = 0;

	// Windows destroyed while a presentation still held a native object built on
	// them. A conforming bridge releases native objects in the surface listener
	// callback, so this stays zero.
	virtual uint32_t GetNativeLifetimeViolations() const = 0;

	// While held, no submission made after HoldGpuCompletion completes on the GPU
	// until ReleaseGpuCompletion. IsGpuWorkPending reports, without blocking,
	// whether held work is still genuinely incomplete. Return false from Hold when
	// the pair cannot hold completion; the suite then fails the delayed-completion
	// obligations rather than passing them vacuously.
	virtual bool HoldGpuCompletion( IRenderDevice &device ) = 0;
	virtual void ReleaseGpuCompletion( IRenderDevice &device ) = 0;
	virtual bool IsGpuWorkPending( IRenderDevice &device ) = 0;

	// Records frame content into the back buffer. Optional; the default draws
	// nothing and only records the resource use.
	virtual void DrawFrame( IRenderDevice &device, IRenderCommandContext &context,
		RenderResourceHandle backBuffer, uint32_t frameIndex )
	{
		( void )device;
		( void )backBuffer;
		( void )frameIndex;
		context.RecordUse( backBuffer );
	}
};

// Runs the full suite. 'provider' must be the bridge's render provider; the
// suite creates and destroys its own devices. Returns report.Passed().
bool RunPresentationConformance( IRenderBackendProvider &provider,
	IRenderPresentationBridgeFactory &bridge, IPresentationHarness &harness, Report &report );

} // namespace conformance
} // namespace render

#endif // RENDERTEST_RENDER_PRESENTATION_CONFORMANCE_H
