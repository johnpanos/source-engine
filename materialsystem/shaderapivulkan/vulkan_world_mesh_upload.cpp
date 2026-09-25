//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Validate WMSH uploads and retain map-scoped GPU resources.
//
//===========================================================================//

#include "vulkan_world_mesh_upload.h"

#include "vulkan_device.h"
#include "vulkan_world_lightmap.h"
#include "tier0/dbg.h"

#include <string>

namespace render_vulkan
{

bool CVulkanWorldMeshUpload::Upload( const world_mesh_gpu::WorldMeshUploadRequest &request )
{
	if ( !request.vertexCount || !request.indexCount ||
	     request.vertexBytes != size_t( request.vertexCount ) * 40 ||
	     request.indexBytes != size_t( request.indexCount ) * sizeof( uint32_t ) )
		return false;
	std::string error;
	if ( !m_context.UploadWorldMesh(
	         request.vertices, request.vertexBytes, request.indices, request.indexBytes, &error ) )
	{
		Warning( "[NativeVulkan] WMSH upload failed: %s\n", error.c_str() );
		return false;
	}
	return true;
}

bool CVulkanWorldMeshUpload::UploadLightmap(
    const world_mesh_gpu::WorldLightmapUploadRequest &request )
{
	std::string error;
	if ( !UploadWorldLightmapLayers( m_context, request, &error ) )
	{
		Warning( "[NativeVulkan] WMSH LMAP rejected: %s\n", error.c_str() );
		return false;
	}
	Msg( "[NativeVulkan] WMSH LMAP ready (%u x %u, linear RGBA16F, %u layer%s)\n", request.width,
	    request.height, request.layerCount, request.layerCount == 1 ? "" : "s" );
	return true;
}

bool CVulkanWorldMeshUpload::UploadProbeVolume(
    const world_mesh_gpu::ProbeVolumeUploadRequest &request )
{
	std::string error;
	if ( !UploadWorldProbeVolume( m_context, request, &error ) )
	{
		Warning( "[NativeVulkan] PRBV rejected: %s\n", error.c_str() );
		return false;
	}
	// Producers republish the volume as often as every frame: report its
	// first upload per map, not each update.
	if ( !m_probeVolumeReported )
		Msg( "[NativeVulkan] PRBV ready (%u x %u atlas, %u grid%s, per-pixel model sampling %s)\n",
		    request.atlasWidth, request.atlasHeight, request.gridCount,
		    request.gridCount == 1 ? "" : "s",
		    m_context.ProbeVolumeSamplingSupported() ? "on" : "unavailable" );
	m_probeVolumeReported = true;
	return true;
}

bool CVulkanWorldMeshUpload::UploadShadowField(
    const world_mesh_gpu::ShadowFieldUploadRequest &request )
{
	std::string error;
	if ( !UploadWorldShadowField( m_context, request, &error ) )
	{
		Warning( "[NativeVulkan] SDF shadow field rejected: %s\n", error.c_str() );
		return false;
	}
	Msg( "[NativeVulkan] SDF shadow field ready (%u x %u x %u, %.1f-unit voxels)\n",
	    request.dims[0], request.dims[1], request.dims[2], request.voxel );
	return true;
}

bool CVulkanWorldMeshUpload::DrawBatch( uint32_t firstIndex, uint32_t indexCount )
{
	return m_context.WorldMeshResident() && indexCount && m_drawBatch &&
	       m_drawBatch( firstIndex, indexCount );
}

void CVulkanWorldMeshUpload::Release()
{
	m_probeVolumeReported = false;
	m_context.ReleaseWorldMesh();
}

bool CVulkanWorldMeshUpload::IsResident() const
{
	return m_context.WorldMeshResident();
}

} // namespace render_vulkan
