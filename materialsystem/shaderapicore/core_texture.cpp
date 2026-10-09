//========= Copyright Valve Corporation, All rights reserved. ============//
//
// ETC1 encoders and decoders for render.device.pica (see core_texture.h).
//
//=============================================================================//

#include "core_texture.h"

#include <algorithm>
#include <climits>
#include <cstring>

namespace corefacade
{

namespace
{

bool ValidSize( int width, int height )
{
	auto pow2 = []( int n ) { return n >= 8 && n <= 1024 && ( n & ( n - 1 ) ) == 0; };
	return pow2( width ) && pow2( height );
}

int Clamp255( int v )
{
	return v < 0 ? 0 : ( v > 255 ? 255 : v );
}

// ETC1 intensity modifier tables (Khronos ETC1 specification, table 3.17.2).
const int kEtcModifiers[8][4] = {
	{ 2, 8, -2, -8 },
	{ 5, 17, -5, -17 },
	{ 9, 29, -9, -29 },
	{ 13, 42, -13, -42 },
	{ 18, 60, -18, -60 },
	{ 24, 80, -24, -80 },
	{ 33, 106, -33, -106 },
	{ 47, 183, -47, -183 },
};

// An ETC1 sub-block: 2x4 (vertical split, flip 0) or 4x2 (flip 1) texels.
bool InSubBlock( int x, int y, int sub, bool flip )
{
	return flip ? ( ( y < 2 ) == ( sub == 0 ) ) : ( ( x < 2 ) == ( sub == 0 ) );
}

// Encodes one 4x4 block (texels[y][x] RGB) in individual mode (4-bit base
// colors), choosing the flip and per-sub-block table that minimize the
// squared error. Returns the 64-bit word in specification bit order.
std::uint64_t EncodeEtc1Block( const std::uint8_t texels[4][4][4] )
{
	std::uint64_t best = 0;
	long long bestError = LLONG_MAX;
	for ( int flipIndex = 0; flipIndex < 2; ++flipIndex )
	{
		const bool flip = flipIndex != 0;
		std::uint64_t word = flip ? ( 1ull << 32 ) : 0;
		long long error = 0;
		int base4[2][3];
		int tables[2];
		std::uint32_t indices = 0; // MSB plane in bits 16..31, LSB plane in bits 0..15
		for ( int sub = 0; sub < 2; ++sub )
		{
			int sum[3] = { 0, 0, 0 };
			for ( int y = 0; y < 4; ++y )
				for ( int x = 0; x < 4; ++x )
					if ( InSubBlock( x, y, sub, flip ) )
						for ( int c = 0; c < 3; ++c )
							sum[c] += texels[y][x][c];
			int base[3];
			for ( int c = 0; c < 3; ++c )
			{
				base4[sub][c] = Clamp255( ( sum[c] + 4 ) / 8 ) * 15 / 255;
				// The 4-bit value expands by bit replication.
				base[c] = base4[sub][c] * 17;
			}
			long long subBest = LLONG_MAX;
			int subTable = 0;
			std::uint32_t subIndices = 0;
			for ( int table = 0; table < 8; ++table )
			{
				long long subError = 0;
				std::uint32_t tableIndices = 0;
				for ( int y = 0; y < 4; ++y )
					for ( int x = 0; x < 4; ++x )
					{
						if ( !InSubBlock( x, y, sub, flip ) )
							continue;
						long long texelBest = LLONG_MAX;
						int texelIndex = 0;
						for ( int index = 0; index < 4; ++index )
						{
							long long e = 0;
							for ( int c = 0; c < 3; ++c )
							{
								int d = Clamp255( base[c] + kEtcModifiers[table][index] ) - texels[y][x][c];
								e += (long long)d * d;
							}
							if ( e < texelBest )
							{
								texelBest = e;
								texelIndex = index;
							}
						}
						subError += texelBest;
						// Texel bit position: x * 4 + y (column-major).
						const int bit = x * 4 + y;
						tableIndices |= std::uint32_t( texelIndex & 1 ) << bit;
						tableIndices |= std::uint32_t( texelIndex >> 1 ) << ( bit + 16 );
					}
				if ( subError < subBest )
				{
					subBest = subError;
					subTable = table;
					subIndices = tableIndices;
				}
			}
			error += subBest;
			tables[sub] = subTable;
			indices |= subIndices;
		}
		// High word: R1 R2 G1 G2 B1 B2 (4 bits each), table1, table2, diff, flip.
		word |= std::uint64_t( base4[0][0] ) << 60 | std::uint64_t( base4[1][0] ) << 56;
		word |= std::uint64_t( base4[0][1] ) << 52 | std::uint64_t( base4[1][1] ) << 48;
		word |= std::uint64_t( base4[0][2] ) << 44 | std::uint64_t( base4[1][2] ) << 40;
		word |= std::uint64_t( tables[0] ) << 37 | std::uint64_t( tables[1] ) << 34;
		word |= indices;
		if ( error < bestError )
		{
			bestError = error;
			best = word;
		}
	}
	return best;
}

void DecodeEtc1Block( std::uint64_t word, std::uint8_t out[4][4][4] )
{
	const bool diff = ( word >> 33 ) & 1;
	const bool flip = ( word >> 32 ) & 1;
	int base[2][3];
	if ( diff )
	{
		for ( int c = 0; c < 3; ++c )
		{
			const int shift = 59 - c * 8;
			int b5 = ( word >> shift ) & 31;
			int d = ( word >> ( shift - 3 ) ) & 7;
			if ( d >= 4 )
				d -= 8;
			const int b52 = b5 + d;
			base[0][c] = ( b5 << 3 ) | ( b5 >> 2 );
			base[1][c] = ( ( b52 & 31 ) << 3 ) | ( ( b52 & 31 ) >> 2 );
		}
	}
	else
	{
		for ( int c = 0; c < 3; ++c )
		{
			const int shift = 60 - c * 8;
			base[0][c] = ( ( word >> shift ) & 15 ) * 17;
			base[1][c] = ( ( word >> ( shift - 4 ) ) & 15 ) * 17;
		}
	}
	const int tables[2] = { int( ( word >> 37 ) & 7 ), int( ( word >> 34 ) & 7 ) };
	for ( int y = 0; y < 4; ++y )
		for ( int x = 0; x < 4; ++x )
		{
			const int sub = InSubBlock( x, y, 0, flip ) ? 0 : 1;
			const int bit = x * 4 + y;
			const int index = int( ( word >> bit ) & 1 ) | int( ( ( word >> ( bit + 16 ) ) & 1 ) << 1 );
			for ( int c = 0; c < 3; ++c )
				out[y][x][c] = std::uint8_t( Clamp255( base[sub][c] + kEtcModifiers[tables[sub]][index] ) );
			out[y][x][3] = 255;
		}
}

void PutLE( std::vector<std::uint8_t> &out, std::size_t at, std::uint64_t value, int bytes )
{
	for ( int i = 0; i < bytes; ++i )
		out[at + i] = std::uint8_t( value >> ( 8 * i ) );
}

std::uint64_t GetLE( const std::uint8_t *in, int bytes )
{
	std::uint64_t value = 0;
	for ( int i = 0; i < bytes; ++i )
		value |= std::uint64_t( in[i] ) << ( 8 * i );
	return value;
}

} // namespace

bool HasAlpha( const std::uint8_t *rgba, int width, int height )
{
	for ( int i = 0; i < width * height; ++i )
		if ( rgba[i * 4 + 3] != 255 )
			return true;
	return false;
}

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
