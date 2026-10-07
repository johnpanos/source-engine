//========= Copyright Valve Corporation, All rights reserved. ============//
//
// PICA200 (Nintendo 3DS GPU) texture formats: the tiled layouts and the
// encoders the 3DS shader API uses to turn the material system's RGBA8888
// mips into textures the GPU samples. Portable: no 3DS SDK dependency, so a
// host suite (unittests/rendertest/pica) checks it against an independent
// decoder.
//
// Layout facts (3dbrew "GPU/Textures"):
//  * images are stored in 8x8 tiles, tiles row-major; inside a tile texels are
//    in Morton (Z) order, x in the low bit;
//  * texel words are little-endian with the first component in the high bits
//    (RGBA8 is the 32-bit word 0xRRGGBBAA);
//  * ETC1 tiles hold four 4x4 blocks in Z order, each block's 64-bit word
//    stored little-endian (the reverse of the Khronos byte order); ETC1A4
//    prefixes each block with 64 bits of 4-bit alpha, column-major.
// The GPU's t = 0 is the first stored row and t = 1 the last, with t = 1 at
// the image's top for D3D coordinates (v = 0 at the top, v = 1 - t): the
// encoders store the source's bottom row first (proven by tools/n3ds/pica_lab).
//
//=============================================================================//

#ifndef PICA_TEXTURE_H
#define PICA_TEXTURE_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace pica
{

// The values are the GPU's GPU_TEXCOLOR codes.
enum class TexFormat : std::uint8_t
{
	kRGBA8 = 0,
	kRGB8 = 1,
	kRGBA5551 = 2,
	kRGB565 = 3,
	kRGBA4 = 4,
	kLA8 = 5,
	kL8 = 7,
	kETC1 = 12,
	kETC1A4 = 13,
};

// Bits per texel.
int BitsPerTexel( TexFormat format );

// Bytes of one level at width x height (both multiples of 8).
std::size_t LevelBytes( TexFormat format, int width, int height );

// Offset of texel (x, y) inside its 8x8 tile, in texels (Morton order).
inline int MortonOffset( int x, int y )
{
	return ( x & 1 ) | ( ( y & 1 ) << 1 ) | ( ( x & 2 ) << 1 ) | ( ( y & 2 ) << 2 ) |
	       ( ( x & 4 ) << 2 ) | ( ( y & 4 ) << 3 );
}

// Index of texel (x, y) in a tiled image of the given width, in texels.
inline int TiledIndex( int x, int y, int width )
{
	return ( ( y >> 3 ) * ( width >> 3 ) + ( x >> 3 ) ) * 64 + MortonOffset( x & 7, y & 7 );
}

// Encodes one level. rgba is width*height RGBA8 texels, row-major, row 0 at
// the top. width and height are powers of two, at least 8. Returns false
// (out untouched) on invalid sizes.
bool EncodeLevel( TexFormat format, const std::uint8_t *rgba, int width, int height,
	std::vector<std::uint8_t> &out );

// Decodes one level back to row-major RGBA8 (the independent oracle's
// counterpart; also used for capture and debugging).
bool DecodeLevel( TexFormat format, const std::uint8_t *data, int width, int height,
	std::vector<std::uint8_t> &rgba );

// True when any texel's alpha is below 255.
bool HasAlpha( const std::uint8_t *rgba, int width, int height );

// Box-filters row-major RGBA8 from (srcW, srcH) to (dstW, dstH); each
// destination texel averages the source texels its footprint covers.
void Resample( const std::uint8_t *src, int srcW, int srcH, std::uint8_t *dst, int dstW, int dstH );

// The largest power of two not above n (n >= 1).
int FloorPow2( int n );

} // namespace pica

#endif // PICA_TEXTURE_H
