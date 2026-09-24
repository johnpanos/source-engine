//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What the material system asks of a Vulkan adapter: its identity
//          (for dxsupport.cfg), memory, and the capabilities the Video options
//          offer. One owner for these facts, whether they come from the live
//          device or from a probe made before any window exists.
//
//===========================================================================//

#ifndef VULKAN_ADAPTER_H
#define VULKAN_ADAPTER_H

#include <cstdint>
#include <string>
#include <vulkan/vulkan.h>

namespace render_vulkan
{
struct VulkanAdapterCaps
{
	bool valid = false;
	std::string name;
	uint32_t vendorId = 0;
	uint32_t deviceId = 0;
	uint32_t driverVersion = 0;
	VkPhysicalDeviceType deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
	// The largest device-local heap: what a D3D9 driver reports as texture
	// memory. (A UMA device reports its shared pool.)
	uint64_t largestDeviceLocalHeapBytes = 0;
	// Sample counts a back buffer of the queried color format and its
	// depth/stencil buffer can both use (VkSampleCountFlags; always has 1).
	uint32_t backBufferSampleMask = VK_SAMPLE_COUNT_1_BIT;
	// The depth/stencil format every depth buffer uses, and whether it can be
	// sampled with linear filtering (a hardware-PCF shadow depth texture).
	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	bool depthSampleable = false;
};

// Device-type ranking shared by device selection and the probe: discrete,
// then integrated, then virtual GPUs; the first-listed device wins a tie.
int ScorePhysicalDeviceType( VkPhysicalDeviceType type );

// The depth/stencil format depth buffers use: D24S8 as D3D9 creates, else D32S8.
// VK_FORMAT_UNDEFINED when neither is a depth/stencil attachment format.
VkFormat SelectDepthStencilFormat( VkPhysicalDevice device );

// The caps of `device` for back buffers of `colorFormat`.
void QueryVulkanAdapterCaps(
    VkPhysicalDevice device, VkFormat colorFormat, VulkanAdapterCaps *outCaps );

// Before a window exists (the material system asks at Init, before SetMode):
// a surface-less instance picks the best-ranked device with a graphics queue
// and the swapchain extension, reports its caps for B8G8R8A8_UNORM back
// buffers, and is destroyed. The live device can differ where a device cannot
// present to the window; callers then prefer the live device's caps.
bool ProbeVulkanAdapter( VulkanAdapterCaps *outCaps, std::string *outError );
} // namespace render_vulkan

#endif // VULKAN_ADAPTER_H
