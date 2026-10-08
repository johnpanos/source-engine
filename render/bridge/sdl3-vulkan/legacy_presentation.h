//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Vulkan bridge's presentation state for the legacy
//			backend's window (RFC 0016 legacy device facade, F3). The
//			composition root owns one; the native Vulkan backend borrows it
//			(NativeVulkanShaderBackend_BindPresentation) and keeps no copy of
//			what it owns:
//
//			- the window's surface host (the pair's only SDL code), opened for
//			  the engine's window before the device is created;
//			- the video mode's presentation request: back-buffer size, vsync
//			  and sample count, with a revision the backend's swapchain
//			  follows;
//			- the gamma ramp presents apply (render.gamma-ramp.v1), with a
//			  revision;
//			- the mode-change callbacks, and whether a change is pending.
//
//			Thread: the render sequence, except DispatchModeChange (the main
//			thread). Free of SDL types; errors are copied into caller buffers
//			(the backend and the root may use different libstdc++ ABIs).
//
//=============================================================================//

#ifndef RENDER_BRIDGE_SDL3_VULKAN_LEGACY_PRESENTATION_H
#define RENDER_BRIDGE_SDL3_VULKAN_LEGACY_PRESENTATION_H

#include "render/render_gamma_ramp.h"

#include <cstddef>
#include <cstdint>

namespace render_vulkan
{

class IVulkanSurfaceHost;

class ILegacyPresentation
{
public:
	struct Mode
	{
		int width = 0; // 0 x 0 follows the window
		int height = 0;
		bool vsync = true;
		int samples = 1;
	};

	// The surface host for the engine's window: made on the first call, the
	// same host afterwards. Null with the reason (always terminated).
	virtual IVulkanSurfaceHost *Open(
	    void *legacyWindowRef, char *error, std::size_t errorSize ) = 0;
	virtual IVulkanSurfaceHost *SurfaceHost() const = 0;
	// Destroys the surface host; nothing presents to it any more.
	virtual void Close() = 0;

	// The video mode's presentation request; every change bumps the revision.
	virtual void RequestMode( const Mode &mode ) = 0;
	virtual Mode GetMode() const = 0;
	virtual std::uint64_t ModeRevision() const = 0;

	// The gamma ramp presents apply; every change bumps the revision.
	virtual void SetGammaRamp( const render::GammaRamp16 &ramp ) = 0;
	// False until a ramp was set.
	virtual bool GetGammaRamp( render::GammaRamp16 *ramp, std::uint64_t *revision ) const = 0;

	// The material system's mode-change callbacks. ModeChanged marks a
	// change; DispatchModeChange runs the callbacks once for it.
	virtual void AddModeChangeCallback( void ( *callback )() ) = 0;
	virtual void RemoveModeChangeCallback( void ( *callback )() ) = 0;
	virtual void ModeChanged() = 0;
	virtual bool DispatchModeChange() = 0;

protected:
	~ILegacyPresentation() = default;
};

// The root's presentation for one legacy window (render.bridge.sdl3-vulkan).
ILegacyPresentation *CreateSdl3LegacyPresentation();
void DestroySdl3LegacyPresentation( ILegacyPresentation *presentation );

} // namespace render_vulkan

// Exported by the native Vulkan shader backend (shaderapivulkan). The root
// binds its presentation before the material system sets a video mode.
extern "C" void NativeVulkanShaderBackend_BindPresentation(
    render_vulkan::ILegacyPresentation *presentation );

#endif // RENDER_BRIDGE_SDL3_VULKAN_LEGACY_PRESENTATION_H
