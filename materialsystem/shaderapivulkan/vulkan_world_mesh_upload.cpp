//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Validate WMSH uploads; the render core's world stage owns the
// map's GPU resources.
//
//===========================================================================//

#include "vulkan_world_mesh_upload.h"

namespace render_vulkan
{

bool CVulkanWorldMeshUpload::Upload( const world_mesh_gpu::WorldMeshUploadRequest &request )
{
	if ( !request.vertices || !request.indices || !request.vertexCount || !request.indexCount ||
	     request.vertexBytes != size_t( request.vertexCount ) * 40 ||
	     request.indexBytes != size_t( request.indexCount ) * sizeof( uint32_t ) )
		return false;
	m_resident = true;
	return true;
}

bool CVulkanWorldMeshUpload::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	(void)request;
	return m_resident;
}

bool CVulkanWorldMeshUpload::UploadProbeVolume(
    const world_mesh_gpu::ProbeVolumeUploadRequest &request )
{
	(void)request;
	return m_resident;
}

bool CVulkanWorldMeshUpload::UploadShadowField(
    const world_mesh_gpu::ShadowFieldUploadRequest &request )
{
	(void)request;
	return m_resident;
}

bool CVulkanWorldMeshUpload::UploadReflectionProbes(
    const world_mesh_gpu::ReflectionProbesUploadRequest &request )
{
	(void)request;
	return m_resident;
}

bool CVulkanWorldMeshUpload::DrawBatch( uint32_t firstIndex, uint32_t indexCount )
{
	(void)firstIndex;
	(void)indexCount;
	return false;
}

void CVulkanWorldMeshUpload::Release()
{
	m_resident = false;
}

bool CVulkanWorldMeshUpload::IsResident() const
{
	return m_resident;
}

} // namespace render_vulkan
