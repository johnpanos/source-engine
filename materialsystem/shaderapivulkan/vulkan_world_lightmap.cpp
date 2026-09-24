//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BSP2 HDR lightmap KTX2 validation and native upload.
//
//===========================================================================//

#include "vulkan_world_lightmap.h"

#include "vulkan_device.h"
#ifdef RFC0008_KTX_READER
#include "vulkan_texture_image.h"
#include "texturecontainer/texture_image.h"
#endif

#include <span>

namespace render_vulkan
{

bool UploadWorldLightmapKtx2( CVulkanContext &context, const void *bytes, std::size_t size,
    std::uint32_t *width, std::uint32_t *height, std::string *error )
{
#ifdef RFC0008_KTX_READER
	if ( !context.WorldMeshResident() || !bytes || size < 80 || size > 256ull * 1024 * 1024 )
	{
		if ( error )
			*error = "WMSH LMAP input or map residency invalid";
		return false;
	}
	const auto encoded = std::span( static_cast<const std::byte *>( bytes ), size );
	const auto image = texturecontainer::ReadKtx2Image( encoded );
	if ( !image || image.Value().format != texturecontainer::PixelFormat::Rgba16Float ||
	     image.Value().levels.size() != 1 )
	{
		if ( error )
			*error = "WMSH LMAP KTX2 requires one linear RGBA16F page";
		return false;
	}
	const auto upload = CreateManagedTextureImage( context, image.Value() );
	if ( !upload )
	{
		if ( error )
			*error = upload.Error().detail;
		return false;
	}
	context.SetManagedTextureSamplerState( upload.Value(), CVulkanContext::kSamplerClampU |
	                                                           CVulkanContext::kSamplerClampV |
	                                                           CVulkanContext::kSamplerLinear );
	context.SetWorldLightmapHandle( upload.Value() );
	if ( width )
		*width = image.Value().levels[0].width;
	if ( height )
		*height = image.Value().levels[0].height;
	return true;
#else
	(void)context;
	(void)bytes;
	(void)size;
	(void)width;
	(void)height;
	if ( error )
		*error = "WMSH LMAP requires the pinned KTX reader profile";
	return false;
#endif
}

} // namespace render_vulkan
