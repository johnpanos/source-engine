//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Vulkan bindings of render.present-policy.v1: what a surface offers,
//          and the VkPresentModeKHR for the selected mode. Every native
//          swapchain selects its present mode through these.
//
//===========================================================================//

#ifndef VULKAN_PRESENT_MODE_H
#define VULKAN_PRESENT_MODE_H

#include "render/render_present_policy.h"

#include <cstdint>
#include <vulkan/vulkan.h>

namespace render_vulkan
{
[[nodiscard]] inline render::PresentModeSupport VulkanPresentModeSupport(
    const VkPresentModeKHR *modes, uint32_t count )
{
	render::PresentModeSupport supported;
	for ( uint32_t i = 0; i < count; ++i )
	{
		if ( modes[i] == VK_PRESENT_MODE_MAILBOX_KHR )
			supported.mailbox = true;
		else if ( modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR )
			supported.immediate = true;
	}
	return supported;
}

[[nodiscard]] constexpr VkPresentModeKHR ToVkPresentMode( render::PresentModeKind mode )
{
	switch ( mode )
	{
	case render::PresentModeKind::Mailbox:
		return VK_PRESENT_MODE_MAILBOX_KHR;
	case render::PresentModeKind::Immediate:
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	case render::PresentModeKind::Fifo:
		break;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

// The present mode for a vsync request on a surface offering `modes`.
[[nodiscard]] inline VkPresentModeKHR SelectVulkanPresentMode(
    bool vsync, const VkPresentModeKHR *modes, uint32_t count )
{
	return ToVkPresentMode( render::SelectPresentMode( vsync, VulkanPresentModeSupport( modes, count ) ) );
}

[[nodiscard]] constexpr const char *VulkanPresentModeName( VkPresentModeKHR mode )
{
	switch ( mode )
	{
	case VK_PRESENT_MODE_FIFO_KHR:
		return "fifo";
	case VK_PRESENT_MODE_MAILBOX_KHR:
		return "mailbox";
	case VK_PRESENT_MODE_IMMEDIATE_KHR:
		return "immediate";
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
		return "fifo-relaxed";
	default:
		return "other";
	}
}
} // namespace render_vulkan

#endif // VULKAN_PRESENT_MODE_H
