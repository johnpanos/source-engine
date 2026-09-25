//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BSP2 HDR lightmap layer and PRBV probe volume upload.
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
		     !context.UploadManagedTexture(
		         handle, static_cast<const uint8_t *>( request.layers[i] ), bytes, &detail ) )
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

bool UploadWorldProbeVolume( CVulkanContext &context,
    const world_mesh_gpu::ProbeVolumeUploadRequest &request, std::string *error )
{
	const auto fail = [&]( const std::string &message )
	{
		if ( error )
			*error = message;
		return false;
	};
	// Six RGBA32F texels per grid (mapcontainer::kProbeGridTableFloats).
	const uint32_t kTableFloats = 24;
	if ( !context.WorldMeshResident() )
		return fail( "PRBV requires a resident world mesh" );
	if ( !request.atlas || !request.gridTable || request.atlasWidth == 0 ||
	     request.atlasHeight == 0 || request.atlasWidth > 16384 || request.atlasHeight > 16384 ||
	     request.gridCount == 0 || request.gridCount > 16 || request.tableFloats != kTableFloats )
		return fail( "PRBV upload request is malformed" );
	std::string detail;
	const int atlas = context.CreateManagedTexture( int( request.atlasWidth ),
	    int( request.atlasHeight ), VK_FORMAT_R16G16B16A16_SFLOAT, &detail );
	if ( atlas < 0 ||
	     !context.UploadManagedTexture( atlas, static_cast<const uint8_t *>( request.atlas ),
	         size_t( request.atlasWidth ) * request.atlasHeight * 8, &detail ) )
	{
		if ( atlas >= 0 )
			context.DestroyManagedTexture( atlas );
		return fail( "PRBV atlas upload failed: " + detail );
	}
	const int grids = context.CreateManagedTexture(
	    int( kTableFloats / 4 ), int( request.gridCount ), VK_FORMAT_R32G32B32A32_SFLOAT, &detail );
	if ( grids < 0 || !context.UploadManagedTexture( grids,
	                      reinterpret_cast<const uint8_t *>( request.gridTable ),
	                      size_t( request.gridCount ) * kTableFloats * sizeof( float ), &detail ) )
	{
		if ( grids >= 0 )
			context.DestroyManagedTexture( grids );
		context.DestroyManagedTexture( atlas );
		return fail( "PRBV grid table upload failed: " + detail );
	}
	// Clamped bilinear filtering is the C++ sampler's Bilinear(); the table
	// and the state texels are read with texelFetch.
	context.SetManagedTextureSamplerState( atlas, CVulkanContext::kSamplerClampU |
	                                                  CVulkanContext::kSamplerClampV |
	                                                  CVulkanContext::kSamplerLinear );
	context.SetManagedTextureSamplerState(
	    grids, CVulkanContext::kSamplerClampU | CVulkanContext::kSamplerClampV );
	int delta = -1;
	if ( request.deltaAtlas )
	{
		delta = context.CreateManagedTexture( int( request.atlasWidth ), int( request.atlasHeight ),
		    VK_FORMAT_R16G16B16A16_SFLOAT, &detail );
		if ( delta < 0 || !context.UploadManagedTexture( delta,
		                      static_cast<const uint8_t *>( request.deltaAtlas ),
		                      size_t( request.atlasWidth ) * request.atlasHeight * 8, &detail ) )
		{
			if ( delta >= 0 )
				context.DestroyManagedTexture( delta );
			context.DestroyManagedTexture( grids );
			context.DestroyManagedTexture( atlas );
			return fail( "PRBV change atlas upload failed: " + detail );
		}
		context.SetManagedTextureSamplerState( delta, CVulkanContext::kSamplerClampU |
		                                                  CVulkanContext::kSamplerClampV |
		                                                  CVulkanContext::kSamplerLinear );
	}
	context.SetProbeVolumeHandles( atlas, grids, request.gridCount );
	context.SetProbeDeltaHandle( delta );
	return true;
}

} // namespace render_vulkan
