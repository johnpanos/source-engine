//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Private, versioned endpoint of the native Vulkan render provider for
//          presentation bridges (RFC 0001 "Window and render interop", R16).
//
//          Portable code sees only render::IRenderBackendProvider. A bridge that
//          joins a window system to this provider is constructed by the
//          composition root with the typed VulkanRenderBackend, and reaches a
//          device's native objects only through VulkanDeviceEndpoint, which
//          FindDevice returns for devices this provider created (nullptr for any
//          other device, so a foreign device is rejected structurally).
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_NATIVE_H
#define SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_NATIVE_H

#include "vulkan_render_backend.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

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
	// every later submission (needs Vulkan 1.2 timeline semaphores).
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

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_RENDER_BACKEND_NATIVE_H
