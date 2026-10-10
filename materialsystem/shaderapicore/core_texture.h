//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Texture size helpers of the core shader API: the box filter and power-of-two
// rounding the material system's texture uploads use.
//
//=============================================================================//

#ifndef CORE_TEXTURE_H
#define CORE_TEXTURE_H

#include <cstdint>

namespace corefacade
{

// Box-filters row-major RGBA8 from (srcW, srcH) to (dstW, dstH); each
// destination texel averages the source texels its footprint covers.
void Resample( const std::uint8_t *src, int srcW, int srcH, std::uint8_t *dst, int dstW, int dstH );

// The largest power of two not above n (n >= 1).
int FloorPow2( int n );

} // namespace corefacade

#endif // CORE_TEXTURE_H
