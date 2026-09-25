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
		static const char *const kRoleNames[] = { "LMAP total", "LMAP direct", "LMAP indirect" };
		context.NameManagedTexture( handle, kRoleNames[role] );
		context.SetManagedTextureSamplerState( handle, CVulkanContext::kSamplerClampU |
		                                                   CVulkanContext::kSamplerClampV |
		                                                   CVulkanContext::kSamplerLinear );
		handles[role] = handle;
	}
	context.SetWorldLightmapHandles( handles[0], handles[1], handles[2] );
	return true;
}

bool UploadWorldShadowField( CVulkanContext &context,
    const world_mesh_gpu::ShadowFieldUploadRequest &request, std::string *error )
{
	const auto fail = [&]( const std::string &message )
	{
		if ( error )
			*error = message;
		return false;
	};
	if ( !request.distances )
	{
		context.SetShadowField( -1, nullptr, 0.0f, nullptr );
		return true;
	}
	if ( !context.WorldMeshResident() )
		return fail( "the shadow field requires a resident world mesh" );
	const uint32_t edge = context.MaxVolumeTextureDimension();
	if ( !request.distances || !( request.voxel > 0.0f ) || request.dims[0] < 2 ||
	     request.dims[1] < 2 || request.dims[2] < 2 || request.dims[0] > edge ||
	     request.dims[1] > edge || request.dims[2] > edge )
		return fail( "shadow field upload request is malformed or exceeds the device's volumes" );
	std::string detail;
	const int field = context.CreateManagedTexture( int( request.dims[0] ), int( request.dims[1] ),
	    VK_FORMAT_R16_SFLOAT, &detail, 0, 1, VK_FORMAT_UNDEFINED, false, request.dims[2] );
	if ( field < 0 ||
	     !context.UploadManagedTexture( field,
	         reinterpret_cast<const uint8_t *>( request.distances ),
	         size_t( request.dims[0] ) * request.dims[1] * request.dims[2] * 2, &detail ) )
	{
		if ( field >= 0 )
			context.DestroyManagedTexture( field );
		return fail( "shadow field upload failed: " + detail );
	}
	context.NameManagedTexture( field, "SDFV shadow field" );
	// Trilinear, clamped on every axis (a volume's W follows U).
	context.SetManagedTextureSamplerState( field, CVulkanContext::kSamplerClampU |
	                                                  CVulkanContext::kSamplerClampV |
	                                                  CVulkanContext::kSamplerLinear );
	context.SetShadowField( field, request.origin, request.voxel, request.dims );
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
	context.NameManagedTexture( atlas, "PRBV probe atlas" );
	context.NameManagedTexture( grids, "PRBV grid table" );
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
		context.NameManagedTexture( delta, "PRBV change atlas" );
		context.SetManagedTextureSamplerState( delta, CVulkanContext::kSamplerClampU |
		                                                  CVulkanContext::kSamplerClampV |
		                                                  CVulkanContext::kSamplerLinear );
	}
	context.SetProbeVolumeHandles( atlas, grids, request.gridCount );
	context.SetProbeDeltaHandle( delta );
	return true;
}

} // namespace render_vulkan
