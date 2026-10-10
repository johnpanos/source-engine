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

bool EncodeEtc1Level(
    bool alpha, const std::uint8_t *rgba, int width, int height, std::vector<std::uint8_t> &out )
{
	if ( !ValidSize( width, height ) )
		return false;
	const int blockBytes = alpha ? 16 : 8;
	out.assign( std::size_t( width / 4 ) * ( height / 4 ) * blockBytes, 0 );
	std::size_t at = 0;
	for ( int by = 0; by < height; by += 4 )
		for ( int bx = 0; bx < width; bx += 4 )
		{
			std::uint8_t texels[4][4][4];
			std::uint64_t alphaWord = 0;
			for ( int y = 0; y < 4; ++y )
				for ( int x = 0; x < 4; ++x )
				{
					std::memcpy( texels[y][x], rgba + ( ( by + y ) * width + bx + x ) * 4, 4 );
					alphaWord |= std::uint64_t( texels[y][x][3] >> 4 ) << ( ( x * 4 + y ) * 4 );
				}
			const std::uint64_t word = EncodeEtc1Block( texels );
			if ( alpha )
			{
				PutLE( out, at, alphaWord, 8 );
				PutLE( out, at + 8, word, 8 );
			}
			else
				for ( int i = 0; i < 8; ++i ) // the specification's byte order
					out[at + i] = std::uint8_t( word >> ( 56 - 8 * i ) );
			at += blockBytes;
		}
	return true;
}

bool DecodeEtc1Level(
    bool alpha, const std::uint8_t *data, int width, int height, std::vector<std::uint8_t> &rgba )
{
	if ( !ValidSize( width, height ) )
		return false;
	rgba.assign( std::size_t( width ) * height * 4, 0 );
	const int blockBytes = alpha ? 16 : 8;
	const std::uint8_t *at = data;
	for ( int by = 0; by < height; by += 4 )
		for ( int bx = 0; bx < width; bx += 4 )
		{
			std::uint64_t word = 0;
			std::uint64_t alphaWord = 0;
			if ( alpha )
			{
				alphaWord = GetLE( at, 8 );
				word = GetLE( at + 8, 8 );
			}
			else
				for ( int i = 0; i < 8; ++i )
					word = word << 8 | at[i];
			std::uint8_t texels[4][4][4];
			DecodeEtc1Block( word, texels );
			for ( int y = 0; y < 4; ++y )
				for ( int x = 0; x < 4; ++x )
				{
					std::uint8_t *p = &rgba[( std::size_t( by + y ) * width + bx + x ) * 4];
					std::memcpy( p, texels[y][x], 4 );
					if ( alpha )
						p[3] = std::uint8_t( ( ( alphaWord >> ( ( x * 4 + y ) * 4 ) ) & 15 ) * 17 );
				}
			at += blockBytes;
		}
	return true;
}

void PackRgba4Level( const std::uint8_t *rgba, int width, int height, std::vector<std::uint8_t> &out )
{
	const std::size_t texels = std::size_t( width ) * std::size_t( height );
	out.resize( texels * 2 );
	const auto nibble = []( std::uint8_t v ) -> unsigned
	{
		return ( unsigned( v ) * 15 + 127 ) / 255;
	};
	for ( std::size_t i = 0; i < texels; ++i )
	{
		const std::uint8_t *t = rgba + i * 4;
		const unsigned word =
		    nibble( t[0] ) << 12 | nibble( t[1] ) << 8 | nibble( t[2] ) << 4 | nibble( t[3] );
		out[i * 2] = std::uint8_t( word );
		out[i * 2 + 1] = std::uint8_t( word >> 8 );
	}
}

} // namespace corefacade
