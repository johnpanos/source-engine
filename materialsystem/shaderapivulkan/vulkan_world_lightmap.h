//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Checked BSP2 lightmap layer and probe volume upload into the native
//          world context.
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

// Uploads a validated PRBV probe volume's atlas and grid table as map-scoped
// images, then publishes them together; on failure nothing is published and
// the previous map's volume stays bound.
// RFC 0011 G9: the map's SDFV distances as an R16F volume the world's
// direct lights are shadowed with.
bool UploadWorldShadowField( CVulkanContext &context,
    const world_mesh_gpu::ShadowFieldUploadRequest &request, std::string *error );
bool UploadWorldProbeVolume( CVulkanContext &context,
    const world_mesh_gpu::ProbeVolumeUploadRequest &request, std::string *error );
// R50-PARALLAX: the map's RPRB reflection probes in their GPU form, published
// as one map-scoped RGBA16F texture (vulkan_world_reflection_probes.cpp). A
// request without texels removes them; on failure the previous map's probes
// stay bound.
bool UploadWorldReflectionProbes( CVulkanContext &context,
    const world_mesh_gpu::ReflectionProbesUploadRequest &request, std::string *error );

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_WORLD_LIGHTMAP_H
