//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Format-checked, all-mip upload of a container-neutral image.
//
//=============================================================================//

#include "vulkan_texture_image.h"

#include "vulkan_device.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>

namespace render_vulkan
{
namespace
{

std::optional<VkFormat> VulkanFormat( texturecontainer::PixelFormat format )
{
	using texturecontainer::PixelFormat;
	switch ( format )
	{
	case PixelFormat::Rgba8Unorm:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case PixelFormat::Rgba8Srgb:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case PixelFormat::Bgra8Unorm:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case PixelFormat::Bgra8Srgb:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case PixelFormat::R8Unorm:
		return VK_FORMAT_R8_UNORM;
	case PixelFormat::Rg8Unorm:
		return VK_FORMAT_R8G8_UNORM;
	case PixelFormat::Rgba16Float:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case PixelFormat::Rgb9e5Float:
		return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
	case PixelFormat::Bc1Unorm:
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case PixelFormat::Bc1Srgb:
		return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case PixelFormat::Bc2Unorm:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case PixelFormat::Bc2Srgb:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case PixelFormat::Bc3Unorm:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case PixelFormat::Bc3Srgb:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case PixelFormat::Bc4Unorm:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case PixelFormat::Bc5Unorm:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case PixelFormat::Bc6hUfloat:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case PixelFormat::Bc7Unorm:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case PixelFormat::Bc7Srgb:
		return VK_FORMAT_BC7_SRGB_BLOCK;
	case PixelFormat::Astc4x4Unorm:
		return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
	case PixelFormat::Astc4x4Srgb:
		return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
	case PixelFormat::Astc4x4Float:
		return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
	case PixelFormat::Etc2RgbaUnorm:
		return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	case PixelFormat::Etc2RgbaSrgb:
		return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
	case PixelFormat::EacR11Unorm:
		return VK_FORMAT_EAC_R11_UNORM_BLOCK;
	case PixelFormat::EacRg11Unorm:
		return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
	}
	return std::nullopt;
}

bool ValidLevels( const texturecontainer::TextureImage &image )
{
	if ( image.levels.empty() || image.levels.size() > 32 )
		return false;
	const auto &base = image.levels.front();
	if ( base.width == 0 || base.height == 0 ||
	     base.width > static_cast<std::uint32_t>( std::numeric_limits<int>::max() ) ||
	     base.height > static_cast<std::uint32_t>( std::numeric_limits<int>::max() ) )
		return false;
	const texturecontainer::BlockLayout layout =
	    texturecontainer::LayoutForPixelFormat( image.format );
	if ( layout.width == 0 || layout.height == 0 || layout.bytes == 0 )
		return false;
	std::uint32_t fullChain = 1;
	for ( std::uint32_t size = std::max( base.width, base.height ); size > 1; size >>= 1 )
		++fullChain;
	if ( image.levels.size() > fullChain )
		return false;
	for ( std::size_t index = 0; index < image.levels.size(); ++index )
	{
		const auto &level = image.levels[index];
		if ( level.width != std::max( 1U, base.width >> index ) ||
		     level.height != std::max( 1U, base.height >> index ) )
			return false;
		const std::size_t blocksWide =
		    ( std::size_t( level.width ) + layout.width - 1 ) / layout.width;
		const std::size_t blocksHigh =
		    ( std::size_t( level.height ) + layout.height - 1 ) / layout.height;
		if ( blocksWide > std::numeric_limits<std::size_t>::max() / blocksHigh / layout.bytes ||
		     level.bytes.size() != blocksWide * blocksHigh * layout.bytes )
			return false;
	}
	return true;
}

} // namespace

foundation::Expected<int, TextureImageUploadFailure> CreateManagedTextureImage(
    CVulkanContext &context, const texturecontainer::TextureImage &image )
{
	const std::optional<VkFormat> format = VulkanFormat( image.format );
	if ( !format )
		return foundation::MakeUnexpected( TextureImageUploadFailure{
		    TextureImageUploadError::UnsupportedFormat, "unknown pixel format" } );
	if ( !ValidLevels( image ) )
		return foundation::MakeUnexpected( TextureImageUploadFailure{
		    TextureImageUploadError::InvalidImage, "incomplete or incorrectly sized mip chain" } );

	std::string detail;
	const int handle = context.CreateManagedTexture( static_cast<int>( image.levels[0].width ),
	    static_cast<int>( image.levels[0].height ), *format, &detail, 0,
	    static_cast<std::uint32_t>( image.levels.size() ) );
	if ( handle < 0 )
		return foundation::MakeUnexpected( TextureImageUploadFailure{
		    TextureImageUploadError::CreateFailed, std::move( detail ) } );
	if ( context.ManagedTextureMipLevels( handle ) != image.levels.size() )
	{
		context.DestroyManagedTexture( handle );
		return foundation::MakeUnexpected( TextureImageUploadFailure{
		    TextureImageUploadError::CreateFailed, "device shortened the requested mip chain" } );
	}
	for ( std::size_t level = 0; level < image.levels.size(); ++level )
	{
		const auto &bytes = image.levels[level].bytes;
		if ( !context.UploadManagedTexture( handle,
		         reinterpret_cast<const std::uint8_t *>( bytes.data() ), bytes.size(), &detail,
		         static_cast<std::uint32_t>( level ) ) )
		{
			context.DestroyManagedTexture( handle );
			return foundation::MakeUnexpected( TextureImageUploadFailure{
			    TextureImageUploadError::UploadFailed, std::move( detail ) } );
		}
	}
	return handle;
}

} // namespace render_vulkan
