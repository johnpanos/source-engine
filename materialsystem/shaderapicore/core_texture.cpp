//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Texture size helpers of the core shader API (see core_texture.h).
//
//=============================================================================//

#include "core_texture.h"

#include <algorithm>
#include <cstdint>

namespace corefacade
{

int FloorPow2( int n )
{
	int p = 1;
	while ( p * 2 <= n )
		p *= 2;
	return p;
}

void Resample( const std::uint8_t *src, int srcW, int srcH, std::uint8_t *dst, int dstW, int dstH )
{
	for ( int y = 0; y < dstH; ++y )
	{
		const int y0 = y * srcH / dstH;
		const int y1 = std::max( y0 + 1, ( y + 1 ) * srcH / dstH );
		for ( int x = 0; x < dstW; ++x )
		{
			const int x0 = x * srcW / dstW;
			const int x1 = std::max( x0 + 1, ( x + 1 ) * srcW / dstW );
			unsigned sum[4] = { 0, 0, 0, 0 };
			for ( int sy = y0; sy < y1; ++sy )
				for ( int sx = x0; sx < x1; ++sx )
					for ( int c = 0; c < 4; ++c )
						sum[c] += src[( sy * srcW + sx ) * 4 + c];
			const unsigned count = unsigned( ( y1 - y0 ) * ( x1 - x0 ) );
			for ( int c = 0; c < 4; ++c )
				dst[( y * dstW + x ) * 4 + c] = std::uint8_t( ( sum[c] + count / 2 ) / count );
		}
	}
}

} // namespace corefacade
