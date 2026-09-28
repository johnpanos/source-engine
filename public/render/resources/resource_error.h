//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.resources errors (RFC 0016).
//
//=============================================================================//

#ifndef RENDER_RESOURCES_RESOURCE_ERROR_H
#define RENDER_RESOURCES_RESOURCE_ERROR_H

#include "render/device/errors.h"

#include <cstdint>

namespace render::resources
{

enum class ResourceStatus : std::uint8_t
{
	kSizeMismatch = 1, // the bytes do not match the description
	kUnknownName,
	kDevice // the device refused; see device
};

struct ResourceError
{
	ResourceStatus status = ResourceStatus::kDevice;
	device::DeviceError device;
};

} // namespace render::resources

#endif // RENDER_RESOURCES_RESOURCE_ERROR_H
