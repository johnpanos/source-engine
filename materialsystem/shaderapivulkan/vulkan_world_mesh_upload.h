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

class CVulkanContext;

class CVulkanWorldMeshUpload final : public world_mesh_gpu::IWorldMeshUpload
{
public:
	using DrawMaterialBatch = bool ( * )( uint32_t firstIndex, uint32_t indexCount );

	CVulkanWorldMeshUpload( CVulkanContext &context, DrawMaterialBatch drawBatch )
	    : m_context( context ), m_drawBatch( drawBatch )
	{
	}

	bool Upload( const world_mesh_gpu::WorldMeshUploadRequest &request ) override;
	bool UploadLightmap( const world_mesh_gpu::WorldLightmapUploadRequest &request ) override;
	bool UploadProbeVolume( const world_mesh_gpu::ProbeVolumeUploadRequest &request ) override;
	bool UploadShadowField( const world_mesh_gpu::ShadowFieldUploadRequest &request ) override;
	bool DrawBatch( uint32_t firstIndex, uint32_t indexCount ) override;
	void Release() override;
	bool IsResident() const override;

private:
	CVulkanContext &m_context;
	DrawMaterialBatch m_drawBatch;
	bool m_probeVolumeReported = false; // this map's first PRBV upload was logged
};

} // namespace render_vulkan

#endif // SHADERAPIVULKAN_VULKAN_WORLD_MESH_UPLOAD_H
