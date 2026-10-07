//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: BC1-BC7 decoding to uncompressed texels (bcdec).
//
//=============================================================================//

#include "texturecontainer/block_decode.h"

#define BCDECDEF static inline
#define BCDEC_IMPLEMENTATION
#include "bcdec.h"

#include <algorithm>
#include <cstring>

namespace texturecontainer
{

std::optional<PixelFormat> DecodedBlockFormat( PixelFormat format ) noexcept
{
	switch ( format )
	{
	case PixelFormat::Bc1Unorm:
	case PixelFormat::Bc2Unorm:
	case PixelFormat::Bc3Unorm:
	case PixelFormat::Bc7Unorm:
		return PixelFormat::Rgba8Unorm;
	case PixelFormat::Bc1Srgb:
	case PixelFormat::Bc2Srgb:
	case PixelFormat::Bc3Srgb:
	case PixelFormat::Bc7Srgb:
		return PixelFormat::Rgba8Srgb;
	case PixelFormat::Bc4Unorm:
		return PixelFormat::R8Unorm;
	case PixelFormat::Bc5Unorm:
		return PixelFormat::Rg8Unorm;
	case PixelFormat::Bc6hUfloat:
		return PixelFormat::Rgba16Float;
	default:
		return std::nullopt;
	}
}

std::optional<TextureImage> DecodeBlockImage( const TextureImage &image )
{
	const std::optional<PixelFormat> to = DecodedBlockFormat( image.format );
	if ( !to )
		return std::nullopt;
	const BlockLayout from = LayoutForPixelFormat( image.format );
	const std::size_t texel = LayoutForPixelFormat( *to ).bytes;
	TextureImage out;
	out.format = *to;
	out.levels.reserve( image.levels.size() );
	for ( const ImageLevel &level : image.levels )
	{
		const std::size_t across = ( std::size_t( level.width ) + 3 ) / 4;
		const std::size_t down = ( std::size_t( level.height ) + 3 ) / 4;
		if ( level.bytes.size() != across * down * from.bytes )
			return std::nullopt;
		ImageLevel decoded;
		decoded.width = level.width;
		decoded.height = level.height;
		decoded.bytes.resize( std::size_t( level.width ) * level.height * texel );
		const unsigned char *block = reinterpret_cast<const unsigned char *>( level.bytes.data() );
		for ( std::size_t by = 0; by < down; ++by )
			for ( std::size_t bx = 0; bx < across; ++bx, block += from.bytes )
			{
				// One 4 x 4 block of texels, row pitch 4 texels.
				unsigned char texels[4 * 4 * 8] = {};
				const int pitch = int( 4 * texel );
				switch ( image.format )
				{
				case PixelFormat::Bc1Unorm:
				case PixelFormat::Bc1Srgb:
					bcdec_bc1( block, texels, pitch );
					break;
				case PixelFormat::Bc2Unorm:
				case PixelFormat::Bc2Srgb:
					bcdec_bc2( block, texels, pitch );
					break;
				case PixelFormat::Bc3Unorm:
				case PixelFormat::Bc3Srgb:
					bcdec_bc3( block, texels, pitch );
					break;
				case PixelFormat::Bc4Unorm:
					bcdec_bc4( block, texels, pitch );
					break;
				case PixelFormat::Bc5Unorm:
					bcdec_bc5( block, texels, pitch );
					break;
				case PixelFormat::Bc7Unorm:
				case PixelFormat::Bc7Srgb:
					bcdec_bc7( block, texels, pitch );
					break;
				case PixelFormat::Bc6hUfloat:
				{
					// RGB halves, then alpha 1.
					std::uint16_t rgb[16 * 3];
					bcdec_bc6h_half( block, rgb, 4 * 3, 0 );
					for ( int t = 0; t < 16; ++t )
					{
						const std::uint16_t rgba[4] = {
						    rgb[t * 3], rgb[t * 3 + 1], rgb[t * 3 + 2], 0x3c00u };
						std::memcpy( texels + t * 8, rgba, sizeof( rgba ) );
					}
					break;
				}
				default:
					return std::nullopt;
				}
				for ( std::size_t y = 0; y < 4 && by * 4 + y < level.height; ++y )
				{
					const std::size_t columns =
					    std::min<std::size_t>( 4, level.width - bx * 4 ) * texel;
					std::memcpy(
					    decoded.bytes.data() + ( ( by * 4 + y ) * level.width + bx * 4 ) * texel,
					    texels + y * 4 * texel, columns );
				}
			}
		out.levels.push_back( std::move( decoded ) );
	}
	return out;
}

} // namespace texturecontainer
