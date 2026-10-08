//========= Copyright Valve Corporation, All rights reserved. ============//
//
// ETC1 for the 3DS shader API's textures: the encoder that turns the
// material system's RGBA8888 mips into render.device.v2's kETC1Rgb and
// kETC1A4 blocks (clause D40: 4x4 blocks in raster order, row 0 at the top;
// kETC1Rgb words in the specification's byte order; kETC1A4 a little-endian
// word of 4-bit alpha, texel (x, y) at bit 4(4x + y), then the ETC1 word
// little-endian). The device tiles them for the GPU. Portable: a host suite
// checks it against an independent decoder.
//
//=============================================================================//

#ifndef PICA_TEXTURE_H
#define PICA_TEXTURE_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace pica
{

// Encodes one level as kETC1Rgb (alpha false) or kETC1A4 blocks. rgba is
// width*height RGBA8 texels, row-major, row 0 at the top; width and height
// are powers of two from 8 to 1024. Returns false (out untouched) otherwise.
bool EncodeEtc1Level(
    bool alpha, const std::uint8_t *rgba, int width, int height, std::vector<std::uint8_t> &out );

// Decodes one level back to row-major RGBA8 (the oracle's counterpart).
bool DecodeEtc1Level(
    bool alpha, const std::uint8_t *data, int width, int height, std::vector<std::uint8_t> &rgba );

// Packs row-major RGBA8 into render.device.v2's kRGBA4Unorm (clause D42):
// one little-endian 16-bit word per texel, R in bits 12-15 down to A in 0-3,
// each channel rounded to the nearest of 16 levels.
void PackRgba4Level( const std::uint8_t *rgba, int width, int height, std::vector<std::uint8_t> &out );

// True when any texel's alpha is below 255.
bool HasAlpha( const std::uint8_t *rgba, int width, int height );

// Box-filters row-major RGBA8 from (srcW, srcH) to (dstW, dstH); each
// destination texel averages the source texels its footprint covers.
void Resample( const std::uint8_t *src, int srcW, int srcH, std::uint8_t *dst, int dstW, int dstH );

// The largest power of two not above n (n >= 1).
int FloorPow2( int n );

} // namespace pica

#endif // PICA_TEXTURE_H
