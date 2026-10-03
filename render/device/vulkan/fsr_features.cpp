//========= Copyright Valve Corporation, All rights reserved. ============//
#include "fsr_features.h"
#include <algorithm>
#include <cstring>
namespace render::device::vulkan
{
bool FsrFeatures::Enable( VkPhysicalDevice physical, VkPhysicalDeviceFeatures2 &head,
    std::vector<const char *> &extensions )
{
#ifndef RENDER_FSR411
	(void)physical;
	(void)head;
	(void)extensions;
	return false;
#else
	// This initial lab composition owns the feature chain. Host composition
	// must negotiate its own chain before product integration.
	if ( head.pNext )
		return false;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties( physical, &properties );
	if ( properties.apiVersion < VK_API_VERSION_1_3 )
		return false;
	std::uint32_t count = 0;
	if ( vkEnumerateDeviceExtensionProperties( physical, nullptr, &count, nullptr ) != VK_SUCCESS )
		return false;
	std::vector<VkExtensionProperties> available( count );
	if ( vkEnumerateDeviceExtensionProperties( physical, nullptr, &count, available.data() ) !=
	     VK_SUCCESS )
		return false;
	const char *wanted[] = { VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
	    VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME,
	    VK_VALVE_SHADER_MIXED_FLOAT_DOT_PRODUCT_EXTENSION_NAME };
	for ( const char *name : wanted )
		if ( std::none_of( available.begin(), available.end(),
		         [name]( const auto &entry )
		         {
			         return std::strcmp( name, entry.extensionName ) == 0;
		         } ) )
			return false;
	f11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	derivatives.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR;
	mixed.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MIXED_FLOAT_DOT_PRODUCT_FEATURES_VALVE;
	VkPhysicalDeviceFeatures2 query{};
	query.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	query.pNext = &f11;
	f11.pNext = &f12;
	f12.pNext = &f13;
	f13.pNext = &derivatives;
	derivatives.pNext = &mixed;
	vkGetPhysicalDeviceFeatures2( physical, &query );
	if ( !f11.storageBuffer16BitAccess || !f12.shaderFloat16 || !f12.shaderInt8 ||
	     !query.features.shaderInt16 || !query.features.shaderStorageImageWriteWithoutFormat ||
	     !f13.shaderIntegerDotProduct || !derivatives.computeDerivativeGroupLinear ||
	     !mixed.shaderMixedFloatDotProductFloat16AccFloat32 )
		return false;
	// Enable only the required shader features, plus the core's own merge.
	f12 = {};
	f12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	f12.shaderFloat16 = f12.shaderInt8 = VK_TRUE;
	f13 = {};
	f13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	f13.shaderIntegerDotProduct = VK_TRUE;
	f11 = {};
	f11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	f11.storageBuffer16BitAccess = VK_TRUE;
	derivatives.computeDerivativeGroupQuads = VK_FALSE;
	mixed.shaderMixedFloatDotProductFloat16AccFloat16 = VK_FALSE;
	mixed.shaderMixedFloatDotProductBFloat16Acc = VK_FALSE;
	mixed.shaderMixedFloatDotProductFloat8AccFloat32 = VK_FALSE;
	head.features.shaderInt16 = VK_TRUE;
	head.features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
	head.pNext = &f11;
	f11.pNext = &f12;
	f12.pNext = &f13;
	f13.pNext = &derivatives;
	derivatives.pNext = &mixed;
	extensions.insert( extensions.end(), std::begin( wanted ), std::end( wanted ) );
	return true;
#endif
}
}
