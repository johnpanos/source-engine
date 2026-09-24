//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BSP2 HDR lightmap layer upload.
//
//===========================================================================//

#include "vulkan_world_lightmap.h"

#include "vulkan_device.h"

namespace render_vulkan
{

bool UploadWorldLightmapLayers( CVulkanContext &context,
    const world_mesh_gpu::WorldLightmapUploadRequest &request, std::string *error )
{
	using world_mesh_gpu::WorldLightmapRole;
	const auto fail = [&]( const char *message )
	{
		if ( error )
			*error = message;
		return false;
	};
	if ( !context.WorldMeshResident() )
		return fail( "WMSH LMAP requires a resident world mesh" );
	if ( request.width == 0 || request.height == 0 || request.width > 16384 ||
	     request.height > 16384 || request.layerCount == 0 ||
	     request.layerCount > world_mesh_gpu::kWorldLightmapMaxUploadLayers ||
	     request.roles[0] != WorldLightmapRole::Total )
		return fail( "WMSH LMAP layers are malformed (layer 0 must be the total page)" );
	int handles[world_mesh_gpu::kWorldLightmapMaxUploadLayers] = { -1, -1, -1 };
	const size_t bytes = size_t( request.width ) * request.height * 8;
	for ( uint32_t i = 0; i < request.layerCount; ++i )
	{
		const uint32_t role = static_cast<uint32_t>( request.roles[i] );
		std::string detail;
		int handle = -1;
		if ( !request.layers[i] || role >= world_mesh_gpu::kWorldLightmapMaxUploadLayers ||
		     handles[role] >= 0 ||
		     ( handle = context.CreateManagedTexture( int( request.width ), int( request.height ),
		           VK_FORMAT_R16G16B16A16_SFLOAT, &detail ) ) < 0 ||
		     !context.UploadManagedTexture( handle,
		         static_cast<const uint8_t *>( request.layers[i] ), bytes, &detail ) )
		{
			if ( handle >= 0 )
				context.DestroyManagedTexture( handle );
			for ( int &created : handles )
				if ( created >= 0 )
					context.DestroyManagedTexture( created );
			if ( error )
				*error = "WMSH LMAP layer upload failed: " + detail;
			return false;
		}
		context.SetManagedTextureSamplerState( handle, CVulkanContext::kSamplerClampU |
		                                                   CVulkanContext::kSamplerClampV |
		                                                   CVulkanContext::kSamplerLinear );
		handles[role] = handle;
	}
	context.SetWorldLightmapHandles( handles[0], handles[1], handles[2] );
	return true;
}

} // namespace render_vulkan
