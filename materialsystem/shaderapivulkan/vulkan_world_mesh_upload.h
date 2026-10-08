//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Map-scoped WMSH service adapter for the native Vulkan provider.
//
//===========================================================================//

#ifndef SHADERAPIVULKAN_VULKAN_WORLD_MESH_UPLOAD_H
#define SHADERAPIVULKAN_VULKAN_WORLD_MESH_UPLOAD_H

#include "render/world_mesh_upload.h"

namespace render_vulkan
{

// The render core's world stage draws the map's WMSH from its own copy
// (RFC 0016 K5/K12); this device keeps no GPU copy of the mesh, lightmap,
// probe volume, shadow field or reflection probes. It validates the mesh
// request, records that the map is resident (the engine keeps the map's WMSH
// bytes for the stage only while it is), and draws no batch: the engine then
// draws its brush batches, which reach the core like any other mesh.
class CVulkanWorldMeshUpload final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override;
	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override;
	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override;
	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override;
	bool UploadReflectionProbes(
	    const world_mesh_gpu::ReflectionProbesUploadRequest &request ) override;
	bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) override;
	void Release() override;
	bool IsResident() const override;

private:
	bool m_resident = false;
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_WORLD_MESH_UPLOAD_H
