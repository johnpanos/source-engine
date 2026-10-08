//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Vulkan adapter identity and capability queries (vulkan_adapter.h).
//
//===========================================================================//

#include "vulkan_adapter.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace render_vulkan
{
VkFormat SelectDepthStencilFormat( VkPhysicalDevice device )
{
	for ( VkFormat candidate : { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT } )
	{
		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties( device, candidate, &fp );
		if ( fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
			return candidate;
	}
	return VK_FORMAT_UNDEFINED;
}

static uint32_t ImageSampleCounts(
    VkPhysicalDevice device, VkFormat format, VkImageUsageFlags usage )
{
	VkImageFormatProperties properties = {};
	if ( format == VK_FORMAT_UNDEFINED ||
	     vkGetPhysicalDeviceImageFormatProperties( device, format, VK_IMAGE_TYPE_2D,
	         VK_IMAGE_TILING_OPTIMAL, usage, 0, &properties ) != VK_SUCCESS )
		return VK_SAMPLE_COUNT_1_BIT;
	return properties.sampleCounts;
}

void QueryVulkanAdapterCaps(
    VkPhysicalDevice device, VkFormat colorFormat, VulkanAdapterCaps *outCaps )
{
	VulkanAdapterCaps caps;
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( device, &properties );
	caps.valid = true;
	caps.name = properties.deviceName;
	caps.vendorId = properties.vendorID;
	caps.deviceId = properties.deviceID;
	caps.driverVersion = properties.driverVersion;
	caps.deviceType = properties.deviceType;

	VkPhysicalDeviceMemoryProperties memory = {};
	vkGetPhysicalDeviceMemoryProperties( device, &memory );
	for ( uint32_t i = 0; i < memory.memoryHeapCount; ++i )
	{
		if ( memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
			caps.largestDeviceLocalHeapBytes =
			    std::max<uint64_t>( caps.largestDeviceLocalHeapBytes, memory.memoryHeaps[i].size );
	}

	caps.depthFormat = SelectDepthStencilFormat( device );
	const VkSampleCountFlags limits = properties.limits.framebufferColorSampleCounts &
	                                  properties.limits.framebufferDepthSampleCounts &
	                                  properties.limits.framebufferStencilSampleCounts;
	caps.backBufferSampleMask =
	    ( limits &
	        ImageSampleCounts( device, colorFormat,
	            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT ) &
	        ImageSampleCounts(
	            device, caps.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) ) |
	    VK_SAMPLE_COUNT_1_BIT;

	if ( caps.depthFormat != VK_FORMAT_UNDEFINED )
	{
		VkFormatProperties fp = {};
		vkGetPhysicalDeviceFormatProperties( device, caps.depthFormat, &fp );
		const VkFormatFeatureFlags needed = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
		                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		                                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
		caps.depthSampleable = ( fp.optimalTilingFeatures & needed ) == needed;
	}
	*outCaps = caps;
}
} // namespace render_vulkan
