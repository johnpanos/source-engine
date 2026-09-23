//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Private endpoint between the native Vulkan presentation core
//          (CVulkanContext) and a pair-specific window bridge (RFC 0001
//          "Window and render interop", roadmap R16).
//
//          The core owns the instance, device and swapchain; the window system
//          owns the window. Everything the core needs from the window --
//          required instance extensions, creating and destroying the
//          VkSurfaceKHR, the drawable size in pixels, and whether the platform
//          currently provides a native surface -- goes through this interface.
//          Only a bridge (e.g. sdl3/sdl3_vulkan_surface_host.cpp) implements it,
//          so the core includes no window-system header.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_SURFACE_HOST_H
#define SHADERAPIVULKAN_VULKAN_SURFACE_HOST_H

#ifdef _WIN32
#pragma once
#endif

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <vector>

namespace render_vulkan
{

class IVulkanSurfaceHost
{
public:
	virtual ~IVulkanSurfaceHost() = default;

	// Instance extensions the window system needs to create surfaces.
	virtual bool GetInstanceExtensions(
	    std::vector<const char *> *outExtensions, std::string *outError ) const = 0;

	// Creates a surface for the window's current native surface. The caller
	// destroys it with DestroySurface before destroying 'instance'.
	virtual bool CreateSurface(
	    VkInstance instance, VkSurfaceKHR *outSurface, std::string *outError ) = 0;
	virtual void DestroySurface( VkInstance instance, VkSurfaceKHR surface ) = 0;

	// Drawable size in pixels (zero while minimized).
	virtual void GetDrawableSize( int *outWidth, int *outHeight ) const = 0;

	// False while the platform provides no native surface (a backgrounded
	// Android activity). Always true where a window keeps one surface for life.
	virtual bool IsNativeSurfaceAvailable() const = 0;

	// Changes whenever the platform replaces the window's native surface; a
	// surface created before the change must be rebuilt.
	virtual uint64_t GetNativeSurfaceGeneration() const = 0;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_SURFACE_HOST_H
