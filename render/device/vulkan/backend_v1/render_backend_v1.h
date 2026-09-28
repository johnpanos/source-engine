//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.backend.v1 (public/render/render_backend.h, RFC 0001
//			render.contracts) over render.device.vulkan (RFC 0016 K1). The
//			lifetime subset of the port keeps its shared suite
//			(unittests/rendertest) and its presentation bridges; since K1 the
//			provider borrows the adapter's instance, devices, memory and
//			timeline (host_device.h), so there is one Vulkan stack: every
//			resource is a VMA allocation of the adapter, and every submission
//			signals the adapter's timeline, whose values are the tokens'
//			completion. It replaces the test-only provider that created its own
//			devices (materialsystem/shaderapivulkan/vulkan_render_backend.cpp).
//
//			Private to the Vulkan family and its bridges: compiled into the
//			programs that use it (the conformance and presentation suites), not
//			into the adapter library, because its interface carries dual-ABI
//			library types (std::string).
//
//			A bridge that joins a window system to this provider reaches a
//			device's native objects only through VulkanDeviceEndpoint, which
//			FindDevice returns for devices this provider created (nullptr for
//			any other device, so a foreign device is rejected structurally).
//
//=============================================================================//

#ifndef RENDER_DEVICE_VULKAN_BACKEND_V1_RENDER_BACKEND_V1_H
#define RENDER_DEVICE_VULKAN_BACKEND_V1_RENDER_BACKEND_V1_H

#include "render/render_backend.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace render::device::vulkan
{
class IHostDevice;
}

namespace render_vulkan
{

struct VulkanProviderOptions
{
	// Instance extensions a bridge needs, e.g. the window system's surface
	// extensions. Creation fails when one is unavailable.
	std::vector<std::string> instanceExtensions;
	// Enable VK_KHR_swapchain on created devices where the adapter supports it.
	bool enableSwapchain = false;
	// Test hook: create a timeline semaphore that can hold GPU completion of
	// every later submission.
	bool enableCompletionGate = false;
};

class VulkanDeviceEndpoint
{
public:
	static const uint32_t kVersion = 1;

	virtual VkInstance Instance() const = 0;
	virtual VkPhysicalDevice PhysicalDevice() const = 0;
	virtual VkDevice Device() const = 0;
	virtual VkQueue Queue() const = 0;
	// The adapter's device behind it: a bridge allocates its own buffers and
	// images there (render/device/vulkan/host_device.h).
	virtual render::device::vulkan::IHostDevice &Host() = 0;
	virtual uint32_t QueueFamily() const = 0;
	virtual bool SwapchainEnabled() const = 0;

	// A device-memory image registered as a device resource, so its lifetime
	// follows DestroyResourceWhenComplete like any other resource.
	virtual render::RenderResourceHandle CreateImage(
	    uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage ) = 0;
	virtual VkImage Image( render::RenderResourceHandle handle ) const = 0;

	// The recording command buffer behind a context from CreateCommandContext.
	virtual VkCommandBuffer CommandBuffer( render::IRenderCommandContext &context ) const = 0;

	// Submit that also waits 'wait' at 'waitStage' and signals 'signal' (either
	// may be VK_NULL_HANDLE). Ordered with Submit; returns a device token.
	virtual render::IRenderCompletionToken *SubmitWithSemaphores(
	    render::IRenderCommandContext &context, VkSemaphore wait, VkPipelineStageFlags waitStage,
	    VkSemaphore signal ) = 0;

	// Test hooks (need VulkanProviderOptions::enableCompletionGate). While held,
	// every later submission waits on the GPU for the release. HasIncompleteGpuWork
	// polls fences without blocking.
	virtual bool HoldCompletion() = 0;
	virtual void ReleaseCompletion() = 0;
	virtual bool HasIncompleteGpuWork() const = 0;

protected:
	~VulkanDeviceEndpoint() = default;
};

class VulkanRenderBackend : public render::IRenderBackendProvider
{
public:
	virtual VkInstance Instance() const = 0;
	virtual VulkanDeviceEndpoint *FindDevice( render::IRenderDevice &device ) = 0;
};

std::unique_ptr<VulkanRenderBackend> MakeVulkanRenderBackend(
    const VulkanProviderOptions &options, std::string *outError );

// The provider with default options. Returns nullptr and fills *outError when
// no usable Vulkan device is available (a harness then reports a skip).
std::unique_ptr<render::IRenderBackendProvider> MakeVulkanRenderBackend( std::string *outError );

} // namespace render_vulkan

#endif // RENDER_DEVICE_VULKAN_BACKEND_V1_RENDER_BACKEND_V1_H
