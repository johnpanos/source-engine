//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 provider descriptors (RFC 0016, RFC 0001
//			composition). Each adapter's public header returns one typed
//			descriptor; the composition root chooses among the descriptors it
//			links. No filename, string registry or native handle crosses here.
//
//=============================================================================//

#ifndef RENDER_DEVICE_PROVIDER_H
#define RENDER_DEVICE_PROVIDER_H

#include "render/device/device.h"
#include "render/device/facts.h"

#include <memory>
#include <string_view>

namespace render::device
{

struct DeviceRequest
{
	// Creation fails with kUnsupported, naming nothing silently, when the
	// device lacks one of these.
	CapabilitySet required;
	// Enable the API's validation (Vulkan layers, GL debug output) when present.
	bool validation = false;
};

using CreateDeviceFn = DeviceResult<std::unique_ptr<IRenderDevice2>> ( * )(
    const DeviceRequest &request );

struct DeviceProviderDescriptor
{
	std::string_view id; // "null", "vulkan", "gl", "gles"
	CreateDeviceFn create = nullptr;
};

} // namespace render::device

#endif // RENDER_DEVICE_PROVIDER_H
