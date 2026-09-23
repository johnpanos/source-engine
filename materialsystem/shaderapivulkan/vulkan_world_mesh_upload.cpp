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

bool CVulkanWorldMeshUpload::UploadLightmapKtx2( const void *bytes, size_t size )
{
	std::string error;
	uint32_t width = 0;
	uint32_t height = 0;
	if ( !UploadWorldLightmapKtx2( m_context, bytes, size, &width, &height, &error ) )
	{
		Warning( "[NativeVulkan] WMSH LMAP rejected: %s\n", error.c_str() );
		return false;
	}
	Msg( "[NativeVulkan] WMSH LMAP ready (%u x %u, linear RGBA16F)\n", width, height );
	return true;
}

bool CVulkanWorldMeshUpload::DrawBatch( uint32_t firstIndex, uint32_t indexCount )
{
	return m_context.WorldMeshResident() && indexCount && m_drawBatch &&
	       m_drawBatch( firstIndex, indexCount );
}

void CVulkanWorldMeshUpload::Release()
{
	m_context.ReleaseWorldMesh();
}

bool CVulkanWorldMeshUpload::IsResident() const
{
	return m_context.WorldMeshResident();
}

} // namespace render_vulkan
