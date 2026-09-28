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
#include <memory>
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
	// The device render.device.vulkan created (RFC 0016 K1): the suites use
	// the adapter's instance, device, queue and allocator, as the product does.
	std::unique_ptr<render::device::vulkan::IHostDevice> host;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physical = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	VkCommandPool pool = VK_NULL_HANDLE;
	uint32_t family = 0;
	bool validation = false;
	ComputeCaps queried;
	DeviceFeatureChain chain;
	std::vector<const char *> extensions;
	std::string name;
};

inline bool CreateDevice( Device *d )
{
	uint32_t layers = 0;
	vkEnumerateInstanceLayerProperties( &layers, nullptr );
	std::vector<VkLayerProperties> layer( layers );
	vkEnumerateInstanceLayerProperties( &layers, layer.data() );
	for ( const VkLayerProperties &l : layer )
		d->validation |= std::strcmp( l.layerName, "VK_LAYER_KHRONOS_validation" ) == 0;
	render::device::vulkan::HostDeviceRequest request;
	request.applicationName = "render.compute";
	request.validation = d->validation;
	request.messageCallback = &OnMessage;
	request.user = d;
	// The compute foundation's feature chain on the device the adapter chose.
	request.describeDevice = []( void *user, VkPhysicalDevice physical, uint32_t family,
	                             render::device::vulkan::HostDeviceFeatures *out )
	{
		Device *device = static_cast<Device *>( user );
		device->queried = QueryComputeCaps( physical, family );
		device->extensions.clear();
		device->chain.Build(
		    physical, device->queried, VkPhysicalDeviceFeatures(), &device->extensions );
		out->features = device->chain.Head();
		out->extensions = device->extensions.data();
		out->extensionCount = uint32_t( device->extensions.size() );
	};
	char error[256] = {};
	d->host = render::device::vulkan::HostDeviceFactory().Create( request, error, sizeof( error ) );
	if ( !d->host )
	{
		std::fprintf( stderr, "headless_vulkan: %s\n", error );
		return false;
	}
	const render::device::vulkan::HostDeviceInfo &info = d->host->Info();
	d->instance = info.instance;
	d->physical = info.physical;
	d->device = info.device;
	d->queue = info.graphicsQueue;
	d->family = info.graphicsFamily;
	d->validation = info.validation;
	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties( d->physical, &properties );
	d->name = properties.deviceName;
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
	d.host.reset(); // the adapter destroys the device and the instance
	d.device = VK_NULL_HANDLE;
	d.instance = VK_NULL_HANDLE;
}

} // namespace headless_vulkan

#endif // HEADLESS_VULKAN_H
