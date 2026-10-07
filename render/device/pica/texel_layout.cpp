//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: How render.device.pica stores a texture (texel_layout.h).
//
//=============================================================================//

#include "texel_layout.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace render::device::pica
{
namespace
{

bool PowerOfTwo( std::uint32_t value )
{
	return value != 0 && ( value & ( value - 1 ) ) == 0;
}

std::uint32_t Word( const std::byte *at )
{
	return std::to_integer<std::uint32_t>( at[0] ) | std::to_integer<std::uint32_t>( at[1] ) << 8 |
	       std::to_integer<std::uint32_t>( at[2] ) << 16 |
	       std::to_integer<std::uint32_t>( at[3] ) << 24;
}

void PutWord( std::byte *at, std::uint32_t value )
{
	for ( int i = 0; i < 4; ++i )
		at[i] = std::byte( value >> ( 8 * i ) );
}

std::uint8_t Unorm8( float value )
{
	const float v = std::isnan( value ) ? 0.0f : std::clamp( value, 0.0f, 1.0f );
	return std::uint8_t( std::lround( v * 255.0f ) );
}

constexpr std::uint32_t kDepthMax = 0xFFFFFF;

} // namespace

bool Storable( Format format )
{
	switch ( format )
	{
	case Format::kRGBA8Unorm:
	case Format::kBGRA8Unorm:
	case Format::kR8Unorm:
	case Format::kD24UnormS8:
	case Format::kETC1Rgb:
	case Format::kETC1A4:
		return true;
	default:
		return false;
	}
}

std::uint32_t StoredBlockBytes( Format format )
{
	return format == Format::kETC1Rgb ? 8u : format == Format::kETC1A4 ? 16u : 0u;
}

namespace
{

// Offset of the 4x4 block at block coordinates (bx, by) of a level stored
// storedWidth texels wide: tiles row-major, a tile's four blocks in Z order.
std::size_t BlockOffset(
    std::uint32_t bx, std::uint32_t by, std::uint32_t storedWidth, std::uint32_t blockBytes )
{
	const std::uint32_t tile = ( by / 2 ) * ( storedWidth / kTile ) + bx / 2;
	const std::uint32_t inTile = ( bx & 1 ) | ( ( by & 1 ) << 1 );
	return ( std::size_t( tile ) * 4 + inTile ) * blockBytes;
}

void Reverse8( const std::byte *from, std::byte *to )
{
	for ( int i = 0; i < 8; ++i )
		to[i] = from[7 - i];
}

// One block, port bytes -> stored bytes or back (ETC1's colour word is the
// only part whose byte order differs; it is its own inverse).
void ConvertBlock( Format format, const std::byte *from, std::byte *to )
{
	if ( format == Format::kETC1Rgb )
		Reverse8( from, to );
	else
		std::memcpy( to, from, 16 );
}

std::uint32_t Blocks( std::uint32_t texels )
{
	return ( texels + 3 ) / 4;
}

} // namespace

bool LayoutOf( Format format, std::uint32_t width, std::uint32_t height, std::uint32_t mipLevels,
    TextureLayout &out )
{
	out = {};
	if ( !Storable( format ) || width == 0 || height == 0 || mipLevels == 0 ||
	     mipLevels > TextureLayout::kMaxLevels )
		return false;
	std::uint64_t offset = 0;
	bool sampleable = PowerOfTwo( width ) && PowerOfTwo( height );
	for ( std::uint32_t level = 0; level < mipLevels; ++level )
	{
		LevelLayout &l = out.levels[level];
		l.width = std::max<std::uint32_t>( 1u, width >> level );
		l.height = std::max<std::uint32_t>( 1u, height >> level );
		l.storedWidth = RoundToTile( l.width );
		l.storedHeight = RoundToTile( l.height );
		l.offset = offset;
		const std::uint64_t texels = std::uint64_t( l.storedWidth ) * l.storedHeight;
		l.bytes = StoredBlockBytes( format ) ? texels / 16 * StoredBlockBytes( format )
		                                     : texels * kStoredTexelBytes;
		offset += l.bytes;
		// A power-of-two level 0 under a tile is stretched over it (texels
		// only; a block format has no texel to repeat): one sampled level.
		if ( level == 0 && sampleable && !StoredBlockBytes( format ) &&
		     ( l.width < kTile || l.height < kTile ) )
		{
			l.stretchX = l.storedWidth / l.width;
			l.stretchY = l.storedHeight / l.height;
			out.sampledLevels = 1;
			sampleable = false;
			continue;
		}
		sampleable = sampleable && l.width >= kTile && l.height >= kTile;
		if ( sampleable )
			out.sampledLevels = level + 1;
	}
	out.levelCount = mipLevels;
	out.bytes = offset;
	return true;
}

std::uint32_t PortTexelBytes( Format format )
{
	return format == Format::kR8Unorm ? 1u : 4u;
}

std::uint32_t StoreTexel( Format format, std::span<const std::byte> port )
{
	auto byte = [&]( std::size_t i )
	{
		return std::to_integer<std::uint32_t>( port[i] );
	};
	switch ( format )
	{
	case Format::kRGBA8Unorm:
		return byte( 0 ) << 24 | byte( 1 ) << 16 | byte( 2 ) << 8 | byte( 3 );
	case Format::kBGRA8Unorm:
		return byte( 2 ) << 24 | byte( 1 ) << 16 | byte( 0 ) << 8 | byte( 3 );
	case Format::kR8Unorm:
		return byte( 0 ) << 24 | 0xFFu;
	case Format::kD24UnormS8:
	{
		float depth = 0.0f;
		std::memcpy( &depth, port.data(), sizeof( depth ) );
		const float d = std::isnan( depth ) ? 0.0f : std::clamp( depth, 0.0f, 1.0f );
		return std::uint32_t( std::lround( double( d ) * kDepthMax ) );
	}
	default:
		return 0;
	}
}

void LoadTexel( Format format, std::uint32_t stored, std::span<std::byte> port )
{
	auto channel = [&]( int shift )
	{
		return std::byte( stored >> shift );
	};
	switch ( format )
	{
	case Format::kRGBA8Unorm:
		port[0] = channel( 24 );
		port[1] = channel( 16 );
		port[2] = channel( 8 );
		port[3] = channel( 0 );
		break;
	case Format::kBGRA8Unorm:
		port[0] = channel( 8 );
		port[1] = channel( 16 );
		port[2] = channel( 24 );
		port[3] = channel( 0 );
		break;
	case Format::kR8Unorm:
		port[0] = channel( 24 );
		break;
	case Format::kD24UnormS8:
	{
		const float depth = float( double( stored & kDepthMax ) / kDepthMax );
		std::memcpy( port.data(), &depth, sizeof( depth ) );
		break;
	}
	default:
		break;
	}
}

std::uint32_t ClearWord( Format format, const float ( &rgba )[4] )
{
	const std::byte port[4] = { std::byte( Unorm8( rgba[0] ) ), std::byte( Unorm8( rgba[1] ) ),
	    std::byte( Unorm8( rgba[2] ) ), std::byte( Unorm8( rgba[3] ) ) };
	// R8 keeps red alone, as its stored form does.
	return StoreTexel( format == Format::kBGRA8Unorm ? Format::kRGBA8Unorm : format, port );
}

std::uint32_t DepthClearWord( float depth, std::uint8_t stencil )
{
	std::byte port[4];
	std::memcpy( port, &depth, sizeof( depth ) );
	return StoreTexel( Format::kD24UnormS8, port ) | std::uint32_t( stencil ) << 24;
}

void CopyIn( Format format, const LevelLayout &level, std::byte *stored, std::uint32_t x,
    std::uint32_t y, std::uint32_t width, std::uint32_t height, const std::byte *rows )
{
	if ( const std::uint32_t blockBytes = StoredBlockBytes( format ) )
	{
		const std::uint32_t wide = Blocks( width ), tall = Blocks( height );
		for ( std::uint32_t by = 0; by < tall; ++by )
			for ( std::uint32_t bx = 0; bx < wide; ++bx )
				ConvertBlock( format, rows + ( std::size_t( by ) * wide + bx ) * blockBytes,
				    stored + BlockOffset( x / 4 + bx, y / 4 + by, level.storedWidth, blockBytes ) );
		return;
	}
	const std::uint32_t texel = PortTexelBytes( format );
	for ( std::uint32_t row = 0; row < height; ++row )
	{
		for ( std::uint32_t column = 0; column < width; ++column )
		{
			const std::byte *from = rows + ( std::size_t( row ) * width + column ) * texel;
			const std::uint32_t word = StoreTexel( format, { from, texel } );
			// A stretched level repeats the texel over its block.
			for ( std::uint32_t sy = 0; sy < level.stretchY; ++sy )
				for ( std::uint32_t sx = 0; sx < level.stretchX; ++sx )
				{
					std::byte *to = stored + std::size_t( TiledIndex(
					                             ( x + column ) * level.stretchX + sx,
					                             ( y + row ) * level.stretchY + sy,
					                             level.storedWidth ) ) *
					                             kStoredTexelBytes;
					// A depth copy writes depth and keeps the stencil already stored.
					PutWord( to, format == Format::kD24UnormS8 ? word | ( Word( to ) & ~kDepthMax )
					                                           : word );
				}
		}
	}
}

void CopyOut( Format format, const LevelLayout &level, const std::byte *stored, std::uint32_t x,
    std::uint32_t y, std::uint32_t width, std::uint32_t height, std::byte *rows )
{
	if ( const std::uint32_t blockBytes = StoredBlockBytes( format ) )
	{
		const std::uint32_t wide = Blocks( width ), tall = Blocks( height );
		for ( std::uint32_t by = 0; by < tall; ++by )
			for ( std::uint32_t bx = 0; bx < wide; ++bx )
				ConvertBlock( format,
				    stored + BlockOffset( x / 4 + bx, y / 4 + by, level.storedWidth, blockBytes ),
				    rows + ( std::size_t( by ) * wide + bx ) * blockBytes );
		return;
	}
	const std::uint32_t texel = PortTexelBytes( format );
	for ( std::uint32_t row = 0; row < height; ++row )
	{
		for ( std::uint32_t column = 0; column < width; ++column )
		{
			const std::byte *from =
			    stored + std::size_t( TiledIndex( ( x + column ) * level.stretchX,
			                             ( y + row ) * level.stretchY, level.storedWidth ) ) *
			                 kStoredTexelBytes;
			std::byte *to = rows + ( std::size_t( row ) * width + column ) * texel;
			LoadTexel( format, Word( from ), { to, texel } );
		}
	}
}

} // namespace render::device::pica

namespace render::device::pica
{

void CopyBetween( Format format, const LevelLayout &fromLevel, const std::byte *from,
    const LevelLayout &toLevel, std::byte *to, std::uint32_t x, std::uint32_t y,
    std::uint32_t width, std::uint32_t height )
{
	if ( const std::uint32_t blockBytes = StoredBlockBytes( format ) )
	{
		for ( std::uint32_t by = y / 4; by < Blocks( y + height ); ++by )
			for ( std::uint32_t bx = x / 4; bx < Blocks( x + width ); ++bx )
				std::memcpy( to + BlockOffset( bx, by, toLevel.storedWidth, blockBytes ),
				    from + BlockOffset( bx, by, fromLevel.storedWidth, blockBytes ), blockBytes );
		return;
	}
	for ( std::uint32_t row = y; row < y + height; ++row )
		for ( std::uint32_t column = x; column < x + width; ++column )
			std::memcpy( to + std::size_t( TiledIndex( column, row, toLevel.storedWidth ) ) *
			                      kStoredTexelBytes,
			    from + std::size_t( TiledIndex( column, row, fromLevel.storedWidth ) ) *
			               kStoredTexelBytes,
			    kStoredTexelBytes );
}

} // namespace render::device::pica
