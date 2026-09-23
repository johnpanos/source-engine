//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Read packaged KTX2 images into owned, backend-neutral mip data.
//
//=============================================================================//

#include "texturecontainer/texture_image.h"

#include <ktx.h>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>

namespace texturecontainer
{
namespace
{

constexpr std::size_t kMaxContainerBytes = 512U * 1024U * 1024U;
constexpr std::uint32_t kMaxDimension = 16384;
constexpr std::uint32_t kMaxLevels = 16;
constexpr std::size_t kHeaderBytes = 80;
constexpr std::size_t kLevelIndexBytes = 24;

std::uint32_t ReadLittle32( std::span<const std::byte> input, std::size_t offset )
{
	std::uint32_t result = 0;
	for ( std::size_t i = 0; i < 4; ++i )
		result |= std::uint32_t( std::to_integer<unsigned char>( input[offset + i] ) ) << ( i * 8 );
	return result;
}

std::uint64_t ReadLittle64( std::span<const std::byte> input, std::size_t offset )
{
	std::uint64_t result = 0;
	for ( std::size_t i = 0; i < 8; ++i )
		result |= std::uint64_t( std::to_integer<unsigned char>( input[offset + i] ) ) << ( i * 8 );
	return result;
}

foundation::Expected<void, ReadError> Preflight( std::span<const std::byte> encoded )
{
	// KTX2 fixed header and level index, read only to bound libktx allocations.
	// Full container validation remains with the pinned library below.
	constexpr unsigned char magic[12] = {
	    0xab, 0x4b, 0x54, 0x58, 0x20, 0x32, 0x30, 0xbb, 0x0d, 0x0a, 0x1a, 0x0a };
	if ( encoded.size() < kHeaderBytes || std::memcmp( encoded.data(), magic, sizeof( magic ) ) )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	const std::uint32_t width = ReadLittle32( encoded, 20 );
	const std::uint32_t height = ReadLittle32( encoded, 24 );
	const std::uint32_t levels = ReadLittle32( encoded, 40 );
	if ( width == 0 || height == 0 || width > kMaxDimension || height > kMaxDimension ||
	     levels == 0 || levels > kMaxLevels )
		return foundation::MakeUnexpected( ReadError::UnsupportedTopology );
	if ( encoded.size() < kHeaderBytes + levels * kLevelIndexBytes )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( ReadLittle32( encoded, 44 ) != KTX_SS_NONE )
		return foundation::MakeUnexpected( ReadError::UnsupportedSupercompression );
	for ( std::uint32_t level = 0; level < levels; ++level )
	{
		const std::size_t index = kHeaderBytes + level * kLevelIndexBytes;
		const std::uint64_t bytes = ReadLittle64( encoded, index + 8 );
		const std::uint64_t uncompressed = ReadLittle64( encoded, index + 16 );
		if ( bytes > kMaxContainerBytes || uncompressed > kMaxContainerBytes )
			return foundation::MakeUnexpected( ReadError::TooLarge );
		if ( bytes != uncompressed )
			return foundation::MakeUnexpected( ReadError::InvalidImageSize );
	}
	return foundation::Expected<void, ReadError>::Ok();
}

std::optional<PixelFormat> DescribeFormat( std::uint32_t vkFormat )
{
	switch ( vkFormat )
	{
	case VK_FORMAT_R8G8B8A8_UNORM:
		return PixelFormat::Rgba8Unorm;
	case VK_FORMAT_R8G8B8A8_SRGB:
		return PixelFormat::Rgba8Srgb;
	case VK_FORMAT_B8G8R8A8_UNORM:
		return PixelFormat::Bgra8Unorm;
	case VK_FORMAT_B8G8R8A8_SRGB:
		return PixelFormat::Bgra8Srgb;
	case VK_FORMAT_R8_UNORM:
		return PixelFormat::R8Unorm;
	case VK_FORMAT_R8G8_UNORM:
		return PixelFormat::Rg8Unorm;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return PixelFormat::Rgba16Float;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		return PixelFormat::Rgb9e5Float;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		return PixelFormat::Bc1Unorm;
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		return PixelFormat::Bc1Srgb;
	case VK_FORMAT_BC2_UNORM_BLOCK:
		return PixelFormat::Bc2Unorm;
	case VK_FORMAT_BC2_SRGB_BLOCK:
		return PixelFormat::Bc2Srgb;
	case VK_FORMAT_BC3_UNORM_BLOCK:
		return PixelFormat::Bc3Unorm;
	case VK_FORMAT_BC3_SRGB_BLOCK:
		return PixelFormat::Bc3Srgb;
	case VK_FORMAT_BC4_UNORM_BLOCK:
		return PixelFormat::Bc4Unorm;
	case VK_FORMAT_BC5_UNORM_BLOCK:
		return PixelFormat::Bc5Unorm;
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		return PixelFormat::Bc6hUfloat;
	case VK_FORMAT_BC7_SRGB_BLOCK:
		return PixelFormat::Bc7Srgb;
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
		return PixelFormat::Astc4x4Srgb;
	case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
		return PixelFormat::Astc4x4Float;
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		return PixelFormat::Etc2RgbaSrgb;
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		return PixelFormat::EacR11Unorm;
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		return PixelFormat::EacRg11Unorm;
	default:
		return std::nullopt;
	}
}

std::size_t ExpectedLevelSize( PixelFormat format, std::uint32_t width, std::uint32_t height )
{
	const BlockLayout layout = LayoutForPixelFormat( format );
	const std::size_t blocksWide = ( width + layout.width - 1 ) / layout.width;
	const std::size_t blocksHigh = ( height + layout.height - 1 ) / layout.height;
	return blocksWide * blocksHigh * layout.bytes;
}

} // namespace

foundation::Expected<TextureImage, ReadError> ReadKtx2Image( std::span<const std::byte> encoded )
{
	if ( encoded.empty() )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( encoded.size() > kMaxContainerBytes )
		return foundation::MakeUnexpected( ReadError::TooLarge );
	const auto preflight = Preflight( encoded );
	if ( !preflight )
		return foundation::MakeUnexpected( preflight.Error() );

	ktxTexture2 *raw = nullptr;
	const KTX_error_code status =
	    ktxTexture2_CreateFromMemory( reinterpret_cast<const ktx_uint8_t *>( encoded.data() ),
	        encoded.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &raw );
	std::unique_ptr<ktxTexture2, decltype( &ktxTexture2_Destroy )> texture(
	    raw, &ktxTexture2_Destroy );
	if ( status != KTX_SUCCESS || raw == nullptr )
		return foundation::MakeUnexpected( ReadError::InvalidContainer );
	if ( texture->numDimensions != 2 || texture->baseWidth == 0 || texture->baseHeight == 0 ||
	     texture->baseWidth > kMaxDimension || texture->baseHeight > kMaxDimension ||
	     texture->baseDepth != 1 || texture->numLayers != 1 || texture->numFaces != 1 ||
	     texture->isArray || texture->isCubemap || texture->numLevels == 0 ||
	     texture->numLevels > kMaxLevels || texture->generateMipmaps )
		return foundation::MakeUnexpected( ReadError::UnsupportedTopology );

	if ( texture->orientation.x != KTX_ORIENT_X_RIGHT ||
	     texture->orientation.y != KTX_ORIENT_Y_DOWN )
		return foundation::MakeUnexpected( ReadError::UnsupportedOrientation );
	if ( texture->supercompressionScheme != KTX_SS_NONE )
		return foundation::MakeUnexpected( ReadError::UnsupportedSupercompression );

	const std::optional<PixelFormat> format = DescribeFormat( texture->vkFormat );
	if ( !format )
		return foundation::MakeUnexpected( ReadError::UnsupportedFormat );
	if ( texture->dataSize > kMaxContainerBytes || texture->pData == nullptr )
		return foundation::MakeUnexpected( ReadError::TooLarge );

	TextureImage result{ *format, {} };
	result.levels.reserve( texture->numLevels );
	for ( std::uint32_t level = 0; level < texture->numLevels; ++level )
	{
		const std::uint32_t width = std::max( 1U, texture->baseWidth >> level );
		const std::uint32_t height = std::max( 1U, texture->baseHeight >> level );
		const std::size_t expected = ExpectedLevelSize( *format, width, height );
		ktx_size_t offset = 0;
		if ( ktxTexture2_GetImageOffset( texture.get(), level, 0, 0, &offset ) != KTX_SUCCESS ||
		     offset > texture->dataSize || expected > texture->dataSize - offset ||
		     ktxTexture_GetImageSize( ktxTexture( texture.get() ), level ) != expected )
			return foundation::MakeUnexpected( ReadError::InvalidImageSize );

		ImageLevel imageLevel;
		imageLevel.width = width;
		imageLevel.height = height;
		imageLevel.bytes.resize( expected );
		std::memcpy( imageLevel.bytes.data(), texture->pData + offset, expected );
		result.levels.push_back( std::move( imageLevel ) );
	}
	return result;
}

} // namespace texturecontainer
