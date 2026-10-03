//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_DEVICE_VULKAN_FSR_H
#define RENDER_DEVICE_VULKAN_FSR_H
#include "render/device/temporal.h"
namespace render::device::vulkan
{
// Experimental INT8 FSR 4.1.1. Device must be created with fsr411 enabled.
// Unsupported devices/builds fail before recording. Each call creates a
// separate view history. Assets are the pinned dependency's assets directory.
DeviceResult<std::unique_ptr<ITemporalUpscaler>> CreateFsr411(
    IRenderDevice2 &device, const std::string &assets );
}
#endif
