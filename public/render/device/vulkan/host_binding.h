//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.vulkan: how the composition root lends the Vulkan
//			adapter to the legacy native Vulkan backend (RFC 0016 K1, "One
//			Vulkan stack"). The adapter creates the device the backend borrows
//			(render/device/vulkan/host_device.h, private to the Vulkan family);
//			its code is linked once, into the root, and the backend's module
//			receives only this factory. Names no Vulkan, SDL or other native
//			type (CAP007).
//
//=============================================================================//

#ifndef RENDER_DEVICE_VULKAN_HOST_BINDING_H
#define RENDER_DEVICE_VULKAN_HOST_BINDING_H

namespace render::device::vulkan
{

class IHostDeviceFactory;

// The adapter's factory of host devices; lives as long as the process.
const IHostDeviceFactory &HostDeviceFactory( bool fsr411 = false );

} // namespace render::device::vulkan

// Exported by the native Vulkan shader backend (shaderapivulkan). The root
// binds the factory before the material system sets a video mode; without
// one, the backend's device creation fails with a named error.
extern "C" void NativeVulkanShaderBackend_BindDeviceFactory(
    const render::device::vulkan::IHostDeviceFactory *factory );

#endif // RENDER_DEVICE_VULKAN_HOST_BINDING_H
