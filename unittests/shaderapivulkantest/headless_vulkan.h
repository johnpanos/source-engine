//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A headless Vulkan device for the native GPU suites that need no
//          window (RFC 0011 G5/G6): the first graphics-capable adapter,
//          created through the compute foundation's VkPhysicalDeviceFeatures2
//          chain, the validation layer when present, and fenced submissions.
//          The same code runs on Linux and on Android from /data/local/tmp.
//
//===========================================================================//

#ifndef HEADLESS_VULKAN_H
#define HEADLESS_VULKAN_H

#include "../../materialsystem/shaderapivulkan/vulkan_compute.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace headless_vulkan
{
using namespace render_vulkan;

inline unsigned long g_validationMessages = 0;

inline VKAPI_ATTR VkBool32 VKAPI_CALL OnMessage( VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *data, void * )
{
	if ( severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
	{
		++g_validationMessages;
		std::fprintf( stderr, "validation: %s\n", data->pMessage );
	}
	return VK_FALSE;
}

struct Device
{
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	VkCommandPool pool = VK_NULL_HANDLE;
	uint32_t family = 0;
	bool validation = false;
	ComputeCaps queried;
	DeviceFeatureChain chain;
	std::string name;
};

inline bool CreateDevice( Device *d )
{
	uint32_t apiVersion = VK_API_VERSION_1_1;
	auto enumerateVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
	    vkGetInstanceProcAddr( VK_NULL_HANDLE, "vkEnumerateInstanceVersion" ) );
	if ( enumerateVersion )
		enumerateVersion( &apiVersion );
	uint32_t layers = 0;
	vkEnumerateInstanceLayerProperties( &layers, nullptr );
	std::vector<VkLayerProperties> layer( layers );
	vkEnumerateInstanceLayerProperties( &layers, layer.data() );
	for ( const VkLayerProperties &l : layer )
		d->validation |= std::strcmp( l.layerName, "VK_LAYER_KHRONOS_validation" ) == 0;
	VkApplicationInfo app = {};
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName = "render.compute";
	app.apiVersion = apiVersion >= VK_API_VERSION_1_2 ? VK_API_VERSION_1_2 : VK_API_VERSION_1_1;
	const char *layerName = "VK_LAYER_KHRONOS_validation";
	const char *debugExtension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	VkInstanceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.pApplicationInfo = &app;
	if ( d->validation )
	{
		info.enabledLayerCount = 1;
		info.ppEnabledLayerNames = &layerName;
		info.enabledExtensionCount = 1;
		info.ppEnabledExtensionNames = &debugExtension;
	}
	if ( vkCreateInstance( &info, nullptr, &d->instance ) != VK_SUCCESS )
		return false;
	if ( d->validation )
	{
		auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr( d->instance, "vkCreateDebugUtilsMessengerEXT" ) );
		VkDebugUtilsMessengerCreateInfoEXT messenger = {};
		messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messenger.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messenger.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		messenger.pfnUserCallback = &OnMessage;
		if ( create )
			create( d->instance, &messenger, nullptr, &d->messenger );
	}
	uint32_t count = 0;
	vkEnumeratePhysicalDevices( d->instance, &count, nullptr );
	std::vector<VkPhysicalDevice> devices( count );
	vkEnumeratePhysicalDevices( d->instance, &count, devices.data() );
	for ( VkPhysicalDevice physical : devices )
	{
		uint32_t families = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( physical, &families, nullptr );
		std::vector<VkQueueFamilyProperties> family( families );
		vkGetPhysicalDeviceQueueFamilyProperties( physical, &families, family.data() );
		for ( uint32_t f = 0; f < families && d->physical == VK_NULL_HANDLE; ++f )
		{
			if ( family[f].queueFlags & VK_QUEUE_GRAPHICS_BIT )
			{
				d->physical = physical;
				d->family = f;
			}
		}
		if ( d->physical != VK_NULL_HANDLE )
			break;
	}
	if ( d->physical == VK_NULL_HANDLE )
		return false;
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( d->physical, &properties );
	d->name = properties.deviceName;
	d->queried = QueryComputeCaps( d->physical, d->family );
	std::vector<const char *> extensions;
	d->chain.Build( d->physical, d->queried, VkPhysicalDeviceFeatures(), &extensions );
	float priority = 1.0f;
	VkDeviceQueueCreateInfo queue = {};
	queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue.queueFamilyIndex = d->family;
	queue.queueCount = 1;
	queue.pQueuePriorities = &priority;
	VkDeviceCreateInfo device = {};
	device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device.pNext = d->chain.Chain();
	device.queueCreateInfoCount = 1;
	device.pQueueCreateInfos = &queue;
	device.enabledExtensionCount = uint32_t( extensions.size() );
	device.ppEnabledExtensionNames = extensions.data();
	if ( vkCreateDevice( d->physical, &device, nullptr, &d->device ) != VK_SUCCESS )
		return false;
	vkGetDeviceQueue( d->device, d->family, 0, &d->queue );
	VkCommandPoolCreateInfo pool = {};
	pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool.queueFamilyIndex = d->family;
	return vkCreateCommandPool( d->device, &pool, nullptr, &d->pool ) == VK_SUCCESS;
}

// One submission: records `record`, submits with a fence, returns the fence
// (the caller waits); the submission's serial is the caller's counter.
inline VkFence Submit( Device &d, VkCommandBuffer *cmd, const std::function<void( VkCommandBuffer )> &record )
{
	VkCommandBufferAllocateInfo allocate = {};
	allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate.commandPool = d.pool;
	allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate.commandBufferCount = 1;
	vkAllocateCommandBuffers( d.device, &allocate, cmd );
	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( *cmd, &begin );
	record( *cmd );
	vkEndCommandBuffer( *cmd );
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence = VK_NULL_HANDLE;
	vkCreateFence( d.device, &fenceInfo, nullptr, &fence );
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = cmd;
	vkQueueSubmit( d.queue, 1, &submit, fence );
	return fence;
}

inline void Finish( Device &d, VkFence fence, VkCommandBuffer cmd )
{
	vkWaitForFences( d.device, 1, &fence, VK_TRUE, UINT64_MAX );
	vkDestroyFence( d.device, fence, nullptr );
	vkFreeCommandBuffers( d.device, d.pool, 1, &cmd );
}


inline void DestroyDevice( Device &d )
{
	vkDestroyCommandPool( d.device, d.pool, nullptr );
	vkDestroyDevice( d.device, nullptr );
	if ( d.messenger != VK_NULL_HANDLE )
	{
		auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr( d.instance, "vkDestroyDebugUtilsMessengerEXT" ) );
		if ( destroy )
			destroy( d.instance, d.messenger, nullptr );
	}
	vkDestroyInstance( d.instance, nullptr );
}

} // namespace headless_vulkan

#endif // HEADLESS_VULKAN_H
