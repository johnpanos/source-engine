//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Checked BSP2 KTX2 lightmap upload into the native world context.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H
#define SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace render_vulkan
{

class CVulkanContext;

bool UploadWorldLightmapKtx2( CVulkanContext &context, const void *bytes, std::size_t size,
    std::uint32_t *width, std::uint32_t *height, std::string *error );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H
