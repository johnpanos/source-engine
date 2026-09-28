//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 external images (RFC 0016; port clause D18): a
//			texture whose memory another API imports, such as a toolkit that
//			composites it without a copy (the Hammer editor's GTK viewports,
//			GdkDmabufTexture). An adapter that can export claims
//			Capability::kExternalImages, and IRenderDevice2::ExternalImages
//			is non-null then.
//
//			The description of the memory is one plane in the Linux dmabuf
//			convention (a file descriptor, a DRM fourcc and modifier, the
//			plane's offset and stride), carried as opaque integers so that no
//			portable module names a platform type. The texture is an ordinary
//			port texture: it is recorded, transitioned and released as any
//			other. Before another API reads it, the last use in a submission
//			must be ResourceUsage::kExternal, and the reader must wait for that
//			submission's token. The next use in the port starts from
//			kUndefined. The texture must not be released while the other API
//			still reads it.
//
//			D18: an adapter that claims the capability exports an image whose
//			memory, read through its description (mapped at offset, rows
//			stride bytes apart), equals what the port reads back from the
//			texture; an adapter that does not claim it returns nullptr.
//
//=============================================================================//

#ifndef RENDER_DEVICE_EXTERNAL_IMAGES_H
#define RENDER_DEVICE_EXTERNAL_IMAGES_H

#include "foundation/expected.h"
#include "render/device/errors.h"
#include "render/device/resources.h"

#include <cstdint>

namespace render::device
{

struct ExternalImage
{
	TextureId texture;
	std::int64_t handle = -1;   // a dmabuf file descriptor; the caller owns it (CloseHandle)
	std::uint32_t fourcc = 0;   // DRM_FORMAT_* of the plane
	std::uint64_t modifier = 0; // DRM_FORMAT_MOD_* (0 is linear)
	std::uint32_t offset = 0;   // bytes to the first row
	std::uint32_t stride = 0;   // bytes between rows
};

class IExternalImages
{
public:
	virtual ~IExternalImages() = default;
	// Creates a 2D texture of one mip and layer, not multisampled, whose
	// usages include kExternal, and exports its memory. Formats: kRGBA8Unorm,
	// kRGBA8Srgb, kBGRA8Unorm, kBGRA8Srgb; anything else fails
	// kUnsupported, and a description breaking those rules
	// kInvalidDescription.
	virtual foundation::Expected<ExternalImage, DeviceError> CreateExported(
	    const TextureDesc &desc ) = 0;
	// Closes a handle CreateExported returned (the platform's close), so that
	// portable owners of exported images name no platform call.
	virtual void CloseHandle( std::int64_t handle ) = 0;
};

} // namespace render::device

#endif // RENDER_DEVICE_EXTERNAL_IMAGES_H
