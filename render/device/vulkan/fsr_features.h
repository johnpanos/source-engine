//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_DEVICE_VULKAN_FSR_FEATURES_H
#define RENDER_DEVICE_VULKAN_FSR_FEATURES_H
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
namespace render::device::vulkan
{
struct FsrFeatures
{
#ifdef RENDER_FSR411
	VkPhysicalDeviceVulkan11Features f11{};
	VkPhysicalDeviceVulkan12Features f12{};
	VkPhysicalDeviceVulkan13Features f13{};
	VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR derivatives{};
	VkPhysicalDeviceShaderMixedFloatDotProductFeaturesVALVE mixed{};
#endif
	bool Enable( VkPhysicalDevice physical, VkPhysicalDeviceFeatures2 &head,
	    std::vector<const char *> &extensions );
};
}
#endif
