//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The SDL3-Vulkan bridge's presentation for the desktop core shader
//			API (RFC 0016 K9, R91 cutover part f): the composition root owns
//			one. Before any window exists it makes a render.backend.v1 Vulkan
//			device with swapchain support and SDL3's instance extensions,
//			whose port (render.device.v2) the render core borrows
//			(RenderCoreConfig::borrowedDevice). Each frame it opens the
//			bridge's presentation of the engine's window, hands the back
//			buffer to the caller as a port texture to draw into, submits that
//			work and presents.
//
//			Thread: the thread the core shader API ends its frames on. Free
//			of SDL and Vulkan types; errors are copied into caller buffers.
//
//=============================================================================//

#ifndef RENDER_BRIDGE_SDL3_VULKAN_CORE_PRESENTER_H
#define RENDER_BRIDGE_SDL3_VULKAN_CORE_PRESENTER_H

#include "render/device/encoder.h"
#include "render/device/resources.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace render::device
{
class IRenderDevice2;
}

namespace render_vulkan
{

class ICorePresenter
{
public:
	virtual ~ICorePresenter() = default;

	// The device the core borrows; it lives as long as this presenter.
	virtual render::device::IRenderDevice2 &Port() = 0;

	// What the caller draws into: the back buffer as a port texture in
	// `usage` (it leaves it there), `width` x `height` in `format`.
	struct Target
	{
		render::device::TextureId texture;
		render::device::Format format = render::device::Format::kUnknown;
		render::device::ResourceUsage usage = render::device::ResourceUsage::kExternal;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};
	// Records the frame's copy into `encoder`, which the presenter submits.
	using Record = bool ( * )(
	    void *user, render::device::CommandEncoder &encoder, const Target &target );

	// One frame of `width` x `height` for the engine's window (the legacy
	// window reference IShaderAPI::SetMode receives), presented with or
	// without vsync as the video mode asks: the presentation is made on the
	// first call, resized when the frame's size changes and made again when
	// vsync changes. On
	// success *submitted is the submission that holds the recorded work.
	// False when nothing was shown: kSuspended (no window yet, minimized)
	// with error empty, or a failure with the reason.
	virtual bool PresentFrame( void *legacyWindowRef, std::uint32_t width, std::uint32_t height,
	    bool vsync, Record record, void *user, render::device::CompletionToken *submitted,
	    char *error, std::size_t errorSize ) = 0;

	// Tests and evidence: the next presented frame, as the swapchain holds
	// it, written to `path` as a binary PPM after it completes. False when no
	// frame was presented or the file could not be written.
	virtual bool CaptureNextPresented( const char *path ) = 0;
};

// Null, with the reason, when SDL3 has no Vulkan window system here (the
// offscreen driver) or no adapter qualifies. fsr411: the device takes the FSR
// 4.1.1 features the temporal upscaler needs (-fsr); unavailable fails creation.
std::unique_ptr<ICorePresenter> CreateSdl3CorePresenter(
    bool validation, bool fsr411, char *error, std::size_t errorSize );

} // namespace render_vulkan

#endif // RENDER_BRIDGE_SDL3_VULKAN_CORE_PRESENTER_H
