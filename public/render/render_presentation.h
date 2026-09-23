//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Presentation-bridge contract (RFC 0001 "Window and render interop",
//			roadmap R16). A window system supplies opaque surfaces; a render
//			provider supplies devices; a pair-specific bridge selected by the
//			composition root joins exactly one of each and creates presentation
//			objects. Only that bridge may interpret native window or swapchain
//			handles. Neither this header nor render_backend.h exposes one.
//
//			Obligations pinned by render.presentation.v1 (see
//			unittests/rendertest/contracts/render.presentation.v1.md):
//
//			  - A presentation is a child of one device and one surface; the
//			    bridge creates and destroys it, never the device.
//			  - Back-buffer and swapchain storage replaced by a resize or released
//			    by destruction is recycled only after the device's completion token
//			    for its last use reports complete (RFC 0006 M3): never on a CPU
//			    event, a flipped flag or a frame count.
//			  - Resize, orientation change and zero-area suspension never recreate
//			    or lose the device. A surface that is temporarily unavailable
//			    suspends; a destroyed surface is lost; both are non-fatal.
//			  - The window system tells the attached presentation before a native
//			    surface goes away, so no native object outlives its window.
//			  - Destruction order is explicit: presentations, then the bridge's
//			    ReleaseDevice, then the device; windows may close at any point.
//			  - Unsupported pairs, foreign devices/surfaces and a second presentation
//			    on one surface fail structurally during composition.
//
//			Like render_backend.h this header is C++11-compatible and free of
//			native types.
//
//=============================================================================//

#ifndef RENDER_RENDER_PRESENTATION_H
#define RENDER_RENDER_PRESENTATION_H

#include "render/render_backend.h"

namespace render
{

// A drawable extent in pixels. A zero-area extent is legal but non-presentable
// (a minimized or in-transition surface); presentation suspends rather than
// failing fatally in that state.
struct RenderExtent
{
	uint32_t width = 0;
	uint32_t height = 0;

	bool IsPresentable() const { return width > 0 && height > 0; }
	bool operator==( const RenderExtent &o ) const
	{
		return width == o.width && height == o.height;
	}
	bool operator!=( const RenderExtent &o ) const { return !( *this == o ); }
};

enum class RenderColorFormat : uint32_t
{
	kRGBA8Unorm = 0,
	kRGBA8Srgb,
	kBGRA8Unorm,
};

// Requested presentation behavior. A zero 'extent' sizes the back buffer from
// the surface's current drawable extent.
struct RenderPresentationConfig
{
	RenderExtent extent;
	RenderColorFormat format = RenderColorFormat::kRGBA8Unorm;
	bool vsync = true;
};

// Result of a frame operation.
enum class RenderPresentStatus : uint32_t
{
	kOk = 0,
	kSuspended,       // not presentable now (zero area, surface unavailable); retry later
	kRecoverable,     // transient native error; the presentation rebuilt what it needed
	kLost,            // surface destroyed or device lost; destroy the presentation
	kInvalidSequence, // Present without an open frame, or BeginFrame with one open
};

// ---------------------------------------------------------------------------
// Surface (owned by the window system)
// ---------------------------------------------------------------------------

enum class RenderSurfaceStatus : uint32_t
{
	kAvailable = 0,
	kUnavailable, // native surface temporarily absent (e.g. a backgrounded mobile app)
	kDestroyed,   // the window is gone; terminal
};

// Implemented by a presentation. The window system calls it synchronously,
// before the current native surface is released (window destruction, or a
// platform taking the surface away). On return the presentation holds no native
// object tied to that surface.
class IRenderSurfaceListener
{
public:
	virtual void OnNativeSurfaceReleasing() = 0;

protected:
	~IRenderSurfaceListener() = default;
};

// An opaque presentation surface. It exposes only portable size, lifetime and
// state; native handles stay in the window system and its bridges. The surface
// object outlives every presentation attached to it, even after its window is
// destroyed (it then reports kDestroyed).
class IRenderSurface
{
public:
	virtual ~IRenderSurface() = default;

	// Drawable size in pixels. Zero-area while minimized or hidden.
	virtual RenderExtent GetDrawableExtent() const = 0;
	virtual RenderSurfaceStatus GetStatus() const = 0;

	// Changes whenever the window system replaces the underlying native surface,
	// so a presentation knows to rebuild its native objects.
	virtual uint64_t GetGeneration() const = 0;

	// At most one presentation is attached at a time. Attach returns false when
	// another listener is attached; Detach of a non-attached listener is a no-op.
	virtual bool AttachListener( IRenderSurfaceListener &listener ) = 0;
	virtual void DetachListener( IRenderSurfaceListener &listener ) = 0;
};

// ---------------------------------------------------------------------------
// Presentation (child of a device and a surface)
// ---------------------------------------------------------------------------

class IRenderPresentation
{
public:
	virtual ~IRenderPresentation() = default;

	// Back-buffer extent.
	virtual RenderExtent GetExtent() const = 0;

	// Resizes the back buffer (including to zero area) WITHOUT recreating the
	// device. Storage the old size used is retired behind its last completion
	// token. Returns false, changing nothing, while a frame is open.
	virtual bool ResizeTo( RenderExtent extent ) = 0;

	// Opens a frame and returns the device resource to render into. The handle is
	// live until the frame's Present has completed on the GPU. Returns kSuspended
	// (no frame opened) while the back buffer or surface is not presentable.
	virtual RenderPresentStatus BeginFrame( RenderResourceHandle *outBackBuffer ) = 0;

	// Presents the open frame. Work writing the back buffer must already have been
	// submitted to the device; the presentation's own submission is ordered after
	// it. Returns kInvalidSequence with no open frame.
	virtual RenderPresentStatus Present() = 0;

	// Closes the open frame without presenting it (a skipped frame). Work already
	// submitted against the back buffer still completes on the GPU and keeps the
	// back buffer live until it does. Returns kInvalidSequence with no open frame.
	virtual RenderPresentStatus CancelFrame() = 0;

	// Presentation-owned storage (back buffers, native swapchains) waiting for GPU
	// completion before it is released. Reaches zero once the device has
	// completed every submission that used it and the presentation has collected.
	virtual size_t GetPendingRetirementCount() const = 0;

	// Releases retired storage whose completion tokens report complete. Also run
	// by BeginFrame and ResizeTo.
	virtual void CollectRetired() = 0;
};

// ---------------------------------------------------------------------------
// Bridge factory (one per supported window-system / render-backend pair)
// ---------------------------------------------------------------------------

struct RenderPresentationPairId
{
	const char *windowSystem = nullptr;  // e.g. "sdl3", "headless"
	const char *renderBackend = nullptr; // matches RenderBackendId::id
};

class IRenderPresentationBridgeFactory
{
public:
	virtual ~IRenderPresentationBridgeFactory() = default;

	virtual RenderPresentationPairId GetPairId() const = 0;
	virtual uint32_t GetMaxPresentations() const = 0;

	// Creates a presentation for 'surface' on 'device'. Fails with a structured
	// error for a device or surface from another provider (kForeignObject), a
	// surface that already has a presentation (kSurfaceBusy), a destroyed surface
	// (kSurfaceLost), an unusable device (kDeviceUnavailable), a surface the device
	// cannot present to (kSurfaceIncompatible) or the presentation limit
	// (kTooManyPresentations).
	virtual IRenderPresentation *CreatePresentation( IRenderDevice &device, IRenderSurface &surface,
	    const RenderPresentationConfig &config, RenderCreateError *error ) = 0;

	// Retires the presentation's storage behind its completion tokens without
	// waiting for the GPU; the device collects back buffers later. Native objects
	// that must outlive in-flight work (swapchains, and the native surface they
	// were built on) stay with the bridge until that work completes, and are
	// released no later than the surface's next native release or ReleaseDevice.
	virtual void DestroyPresentation( IRenderPresentation *presentation ) = 0;

	virtual size_t GetLivePresentationCount() const = 0;

	// Waits for the device's GPU work and releases every native object the bridge
	// still holds for it. Required before the provider destroys the device.
	// Returns false (releasing nothing) while a live presentation still uses it.
	virtual bool ReleaseDevice( IRenderDevice &device ) = 0;
};

// Composition-time selection: returns the bridge joining exactly this pair, or
// nullptr with kUnsupportedPair. Unsupported pairs never fall back to another.
inline IRenderPresentationBridgeFactory *SelectPresentationBridge(
    IRenderPresentationBridgeFactory *const *bridges, size_t count, const char *windowSystem,
    const char *renderBackend, RenderCreateError *error )
{
	for ( size_t i = 0; bridges != nullptr && i < count; ++i )
	{
		if ( bridges[i] == nullptr )
			continue;
		const RenderPresentationPairId pair = bridges[i]->GetPairId();
		if ( pair.windowSystem && pair.renderBackend && windowSystem && renderBackend &&
		     contract_detail::EqualStrings( pair.windowSystem, windowSystem, 64 ) &&
		     contract_detail::EqualStrings( pair.renderBackend, renderBackend, 64 ) )
			return bridges[i];
	}
	if ( error )
	{
		error->status = RenderCreateStatus::kUnsupportedPair;
		const char text[] = "no presentation bridge joins this window system and backend";
		for ( size_t i = 0; i < sizeof( text ) && i < sizeof( error->message ); ++i )
			error->message[i] = text[i];
	}
	return nullptr;
}

} // namespace render

#endif // RENDER_RENDER_PRESENTATION_H
