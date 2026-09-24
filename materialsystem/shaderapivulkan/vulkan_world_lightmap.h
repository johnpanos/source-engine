//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Checked BSP2 lightmap layer upload into the native world context.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H
#define SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H

#include "render/world_mesh_upload.h"

#include <string>

namespace render_vulkan
{

class CVulkanContext;

// Uploads every layer of a validated LMAP (linear RGBA16F, one page per role)
// as map-scoped images, then publishes them together; on failure nothing is
// published and the previous map's layers stay bound.
bool UploadWorldLightmapLayers( CVulkanContext &context,
    const world_mesh_gpu::WorldLightmapUploadRequest &request, std::string *error );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H
