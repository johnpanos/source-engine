//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: How render.device.pica stores a texture (RFC 0026 decisions 4 and
//			7). Portable: no 3DS SDK, so the host suite checks it.
//
//			Every texture is the GPU's tiled layout (3dbrew "GPU/Textures"):
//			8x8 tiles, tiles row-major, texels inside a tile in Morton order
//			with x in the low bit. The port's row 0 (the top) is the first
//			stored row, as the GPU writes a render target's top row; the GPU
//			samples it at t = 1, so PVS1 programs pass t = 1 - v (artifacts.h).
//
//			A level's storage is its extent rounded up to whole tiles. Levels
//			follow each other; the GPU addresses level n at the offset the
//			levels before it take, which equals this layout while each side
//			stays a power of two of at least 8 (Sampleable). Smaller levels are
//			stored, copied and read back, but never sampled: the device limits
//			the sampled levels to those (a named limit of the PICA200).
//
//			Stored texels, by port format:
//			  kRGBA8Unorm, kBGRA8Unorm  GPU RGBA8: the word 0xRRGGBBAA
//			  kR8Unorm                  GPU RGBA8 with G = B = 0, A = 255 (the
//			                            GPU's L8 would sample (r, r, r, 1))
//			  kD24UnormS8               depth in bits 0-23, stencil in 24-31;
//			                            copied to and from buffers as the
//			                            port's 32-bit float depth (D13)
//			  kETC1Rgb, kETC1A4         the GPU's ETC1 and ETC1A4: each 8x8 tile
//			                            holds its four 4x4 blocks in Z order;
//			                            an ETC1 word is stored little-endian
//			                            (the port's is the specification's
//			                            byte order, D40); ETC1A4 is the same
//			                            16 bytes as the port's
//
//=============================================================================//

#ifndef RENDER_DEVICE_PICA_TEXEL_LAYOUT_H
#define RENDER_DEVICE_PICA_TEXEL_LAYOUT_H

#include "render/device/resources.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace render::device::pica
{

inline constexpr std::uint32_t kTile = 8;

// Offset of texel (x, y) inside its 8x8 tile, in texels (Morton order).
constexpr std::uint32_t MortonOffset( std::uint32_t x, std::uint32_t y )
{
	return ( x & 1 ) | ( ( y & 1 ) << 1 ) | ( ( x & 2 ) << 1 ) | ( ( y & 2 ) << 2 ) |
	       ( ( x & 4 ) << 2 ) | ( ( y & 4 ) << 3 );
}

// Index of texel (x, y) of a level stored storedWidth texels wide (a multiple
// of 8), in texels.
constexpr std::uint32_t TiledIndex( std::uint32_t x, std::uint32_t y, std::uint32_t storedWidth )
{
	return ( ( y / kTile ) * ( storedWidth / kTile ) + x / kTile ) * kTile * kTile +
	       MortonOffset( x % kTile, y % kTile );
}

constexpr std::uint32_t RoundToTile( std::uint32_t extent )
{
	return ( extent + kTile - 1 ) / kTile * kTile;
}

// The formats a PICA texture can hold (RFC 0026 decision 4).
bool Storable( Format format );
// Bytes of one stored texel of the uncompressed formats.
inline constexpr std::uint32_t kStoredTexelBytes = 4;
// Bytes of a stored 4x4 block of an ETC format (0 for the others).
std::uint32_t StoredBlockBytes( Format format );

struct LevelLayout
{
	std::uint32_t width = 0; // the port's extent
	std::uint32_t height = 0;
	std::uint32_t storedWidth = 0; // whole tiles
	std::uint32_t storedHeight = 0;
	std::uint64_t offset = 0; // from the texture's first byte
	std::uint64_t bytes = 0;
	// A sampleable texture smaller than a tile (a 1x1 neutral texture) is
	// stored stretched over the whole tile, each texel repeated stretchX by
	// stretchY times, so the GPU (8x8 at least) samples the same texels at
	// every coordinate. 1 for every other level.
	std::uint32_t stretchX = 1;
	std::uint32_t stretchY = 1;
};

struct TextureLayout
{
	static constexpr std::uint32_t kMaxLevels = 11; // 1024 down to 1
	LevelLayout levels[kMaxLevels];
	std::uint32_t levelCount = 0;
	std::uint64_t bytes = 0;
	// Levels the GPU can sample: from 0 while each side is a power of two of
	// at least 8 (0: the texture cannot be sampled at all).
	std::uint32_t sampledLevels = 0;
};

// The layout of a 2D texture of one layer; false when it cannot be stored.
bool LayoutOf( Format format, std::uint32_t width, std::uint32_t height, std::uint32_t mipLevels,
    TextureLayout &out );

// One texel, port bytes <-> stored word (little-endian in memory).
std::uint32_t StoreTexel( Format format, std::span<const std::byte> port );
void LoadTexel( Format format, std::uint32_t stored, std::span<std::byte> port );
// The port's buffer bytes per texel of a copy (4 for every storable format
// but kR8Unorm, 1).
std::uint32_t PortTexelBytes( Format format );

// A clear colour as the stored word of a colour format.
std::uint32_t ClearWord( Format format, const float ( &rgba )[4] );
// A depth/stencil clear as the stored word.
std::uint32_t DepthClearWord( float depth, std::uint8_t stencil );

// Copies a region between a level's tiled storage and the port's linear
// rows (row 0 at the top, rowBytes = width * PortTexelBytes), or for an ETC
// format its blocks in raster order (x, y, width and height whole blocks or
// reaching the level's edge, as the port's copy rules allow).
void CopyIn( Format format, const LevelLayout &level, std::byte *stored, std::uint32_t x,
    std::uint32_t y, std::uint32_t width, std::uint32_t height, const std::byte *rows );
void CopyOut( Format format, const LevelLayout &level, const std::byte *stored, std::uint32_t x,
    std::uint32_t y, std::uint32_t width, std::uint32_t height, std::byte *rows );
// A region between two levels of one format, texel or block aligned as above.
void CopyBetween( Format format, const LevelLayout &fromLevel, const std::byte *from,
    const LevelLayout &toLevel, std::byte *to, std::uint32_t x, std::uint32_t y,
    std::uint32_t width, std::uint32_t height );

} // namespace render::device::pica

#endif // RENDER_DEVICE_PICA_TEXEL_LAYOUT_H
