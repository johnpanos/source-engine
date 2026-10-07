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

#include <cstdint>

namespace render::device::vulkan
{

class IHostDeviceFactory;

// The adapter's factory of host devices; lives as long as the process.
const IHostDeviceFactory &HostDeviceFactory( bool fsr411 = false );

// The identity of a physical device, as the root's legacy device facade
// reports it (RFC 0016 legacy device facade, F1).
struct HostAdapterIdentity
{
	char name[256] = {};
	std::uint32_t vendorId = 0;
	std::uint32_t deviceId = 0;
	std::uint32_t driverVersion = 0;
	std::uint64_t deviceLocalBytes = 0; // the largest device-local heap
};

// The adapter `factory` creates host devices on: the adapter of the last
// device it created or, before any, the adapter its selection ranks first
// without a surface (a later device differs only when that adapter cannot
// present to the window). Adapter 0 is the only one. False, with *out
// cleared, for another index or without a Vulkan adapter; the probe runs
// once per factory.
bool DescribeHostAdapter(
    const IHostDeviceFactory &factory, int adapter, HostAdapterIdentity *out );

} // namespace render::device::vulkan

// Exported by the native Vulkan shader backend (shaderapivulkan). The root
// binds the factory before the material system sets a video mode; without
// one, the backend's device creation fails with a named error.
extern "C" void NativeVulkanShaderBackend_BindDeviceFactory(
    const render::device::vulkan::IHostDeviceFactory *factory );

#endif // RENDER_DEVICE_VULKAN_HOST_BINDING_H
