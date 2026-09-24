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
int ScorePhysicalDeviceType( VkPhysicalDeviceType type )
{
	switch ( type )
	{
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		return 1000;
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		return 500;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		return 100;
	default:
		return 0;
	}
}

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
	        ImageSampleCounts( device, caps.depthFormat,
	            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ) ) |
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

static bool HasGraphicsQueueAndSwapchain( VkPhysicalDevice device )
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
	std::vector<VkQueueFamilyProperties> families( count );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &count, families.data() );
	const bool graphics = std::any_of( families.begin(), families.end(),
	    []( const VkQueueFamilyProperties &f ) { return ( f.queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0; } );
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extCount, nullptr );
	std::vector<VkExtensionProperties> extensions( extCount );
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extCount, extensions.data() );
	const bool swapchain = std::any_of( extensions.begin(), extensions.end(),
	    []( const VkExtensionProperties &e ) {
		    return std::strcmp( e.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) == 0;
	    } );
	return graphics && swapchain;
}

bool ProbeVulkanAdapter( VulkanAdapterCaps *outCaps, std::string *outError )
{
	*outCaps = VulkanAdapterCaps();
	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "Source Native Vulkan adapter probe";
	app.pEngineName = "Source";
	app.apiVersion = VK_API_VERSION_1_1;
	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app;
	VkInstance instance = VK_NULL_HANDLE;
	const VkResult r = vkCreateInstance( &info, nullptr, &instance );
	if ( r != VK_SUCCESS )
	{
		if ( outError )
			*outError = "adapter probe: vkCreateInstance failed (" + std::to_string( r ) + ")";
		return false;
	}
	uint32_t count = 0;
	vkEnumeratePhysicalDevices( instance, &count, nullptr );
	std::vector<VkPhysicalDevice> devices( count );
	vkEnumeratePhysicalDevices( instance, &count, devices.data() );
	VkPhysicalDevice best = VK_NULL_HANDLE;
	int bestScore = -1;
	for ( VkPhysicalDevice device : devices )
	{
		if ( !HasGraphicsQueueAndSwapchain( device ) )
			continue;
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties( device, &properties );
		const int score = ScorePhysicalDeviceType( properties.deviceType );
		if ( score > bestScore )
		{
			bestScore = score;
			best = device;
		}
	}
	if ( best != VK_NULL_HANDLE )
		QueryVulkanAdapterCaps( best, VK_FORMAT_B8G8R8A8_UNORM, outCaps );
	vkDestroyInstance( instance, nullptr );
	if ( best == VK_NULL_HANDLE && outError )
		*outError = "adapter probe: no device with a graphics queue and swapchain support";
	return best != VK_NULL_HANDLE;
}
} // namespace render_vulkan
