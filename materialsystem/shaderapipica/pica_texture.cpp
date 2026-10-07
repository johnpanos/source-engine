//========= Copyright Valve Corporation, All rights reserved. ============//
//
// PICA200 texture encoders and decoders (see pica_texture.h).
//
//=============================================================================//

#include "pica_texture.h"

#include <algorithm>
#include <climits>
#include <cstring>

namespace pica
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

std::uint32_t PackTexel( TexFormat format, const std::uint8_t *p )
{
	const int r = p[0], g = p[1], b = p[2], a = p[3];
	switch ( format )
	{
	case TexFormat::kRGBA8:
		return std::uint32_t( r ) << 24 | std::uint32_t( g ) << 16 | std::uint32_t( b ) << 8 | std::uint32_t( a );
	case TexFormat::kRGB8:
		return std::uint32_t( r ) << 16 | std::uint32_t( g ) << 8 | std::uint32_t( b );
	case TexFormat::kRGBA5551:
		return ( r >> 3 ) << 11 | ( g >> 3 ) << 6 | ( b >> 3 ) << 1 | ( a >> 7 );
	case TexFormat::kRGB565:
		return ( r >> 3 ) << 11 | ( g >> 2 ) << 5 | ( b >> 3 );
	case TexFormat::kRGBA4:
		return ( r >> 4 ) << 12 | ( g >> 4 ) << 8 | ( b >> 4 ) << 4 | ( a >> 4 );
	case TexFormat::kLA8:
		return std::uint32_t( r ) << 8 | std::uint32_t( a );
	case TexFormat::kL8:
		return std::uint32_t( r );
	default:
		return 0;
	}
}

void UnpackTexel( TexFormat format, std::uint32_t v, std::uint8_t *p )
{
	auto expand = []( std::uint32_t value, int bits ) {
		return std::uint8_t( ( value << ( 8 - bits ) ) | ( value >> ( 2 * bits - 8 > 0 ? 2 * bits - 8 : 0 ) ) );
	};
	switch ( format )
	{
	case TexFormat::kRGBA8:
		p[0] = v >> 24, p[1] = v >> 16, p[2] = v >> 8, p[3] = v;
		break;
	case TexFormat::kRGB8:
		p[0] = v >> 16, p[1] = v >> 8, p[2] = v, p[3] = 255;
		break;
	case TexFormat::kRGBA5551:
		p[0] = expand( ( v >> 11 ) & 31, 5 ), p[1] = expand( ( v >> 6 ) & 31, 5 ),
		p[2] = expand( ( v >> 1 ) & 31, 5 ), p[3] = ( v & 1 ) ? 255 : 0;
		break;
	case TexFormat::kRGB565:
		p[0] = expand( ( v >> 11 ) & 31, 5 ), p[1] = expand( ( v >> 5 ) & 63, 6 ),
		p[2] = expand( v & 31, 5 ), p[3] = 255;
		break;
	case TexFormat::kRGBA4:
		p[0] = ( ( v >> 12 ) & 15 ) * 17, p[1] = ( ( v >> 8 ) & 15 ) * 17,
		p[2] = ( ( v >> 4 ) & 15 ) * 17, p[3] = ( v & 15 ) * 17;
		break;
	case TexFormat::kLA8:
		p[0] = p[1] = p[2] = std::uint8_t( v >> 8 ), p[3] = std::uint8_t( v );
		break;
	case TexFormat::kL8:
		p[0] = p[1] = p[2] = std::uint8_t( v ), p[3] = 255;
		break;
	default:
		break;
	}
}

} // namespace

int BitsPerTexel( TexFormat format )
{
	switch ( format )
	{
	case TexFormat::kRGBA8: return 32;
	case TexFormat::kRGB8: return 24;
	case TexFormat::kRGBA5551:
	case TexFormat::kRGB565:
	case TexFormat::kRGBA4:
	case TexFormat::kLA8: return 16;
	case TexFormat::kL8:
	case TexFormat::kETC1A4: return 8;
	case TexFormat::kETC1: return 4;
	}
	return 0;
}

std::size_t LevelBytes( TexFormat format, int width, int height )
{
	return std::size_t( width ) * height * BitsPerTexel( format ) / 8;
}

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

bool EncodeLevel( TexFormat format, const std::uint8_t *rgba, int width, int height,
	std::vector<std::uint8_t> &out )
{
	if ( !ValidSize( width, height ) || BitsPerTexel( format ) == 0 )
		return false;
	out.assign( LevelBytes( format, width, height ), 0 );
	if ( format == TexFormat::kETC1 || format == TexFormat::kETC1A4 )
	{
		const bool alpha = format == TexFormat::kETC1A4;
		const int blockBytes = alpha ? 16 : 8;
		std::size_t at = 0;
		for ( int ty = 0; ty < height; ty += 8 )
			for ( int tx = 0; tx < width; tx += 8 )
				for ( int b = 0; b < 4; ++b )
				{
					const int bx = tx + ( b & 1 ) * 4;
					const int by = ty + ( b >> 1 ) * 4;
					std::uint8_t texels[4][4][4];
					std::uint64_t alphaWord = 0;
					for ( int y = 0; y < 4; ++y )
						for ( int x = 0; x < 4; ++x )
						{
							std::memcpy( texels[y][x], rgba + ( ( height - 1 - ( by + y ) ) * width + bx + x ) * 4, 4 );
							alphaWord |= std::uint64_t( texels[y][x][3] >> 4 ) << ( ( x * 4 + y ) * 4 );
						}
					if ( alpha )
					{
						PutLE( out, at, alphaWord, 8 );
						at += 8;
					}
					PutLE( out, at, EncodeEtc1Block( texels ), 8 );
					at += blockBytes - ( alpha ? 8 : 0 );
				}
		return true;
	}
	const int bytes = BitsPerTexel( format ) / 8;
	for ( int y = 0; y < height; ++y )
		for ( int x = 0; x < width; ++x )
			PutLE( out, std::size_t( TiledIndex( x, y, width ) ) * bytes,
				PackTexel( format, rgba + ( ( height - 1 - y ) * width + x ) * 4 ), bytes );
	return true;
}

bool DecodeLevel( TexFormat format, const std::uint8_t *data, int width, int height,
	std::vector<std::uint8_t> &rgba )
{
	if ( !ValidSize( width, height ) || BitsPerTexel( format ) == 0 )
		return false;
	rgba.assign( std::size_t( width ) * height * 4, 0 );
	if ( format == TexFormat::kETC1 || format == TexFormat::kETC1A4 )
	{
		const bool alpha = format == TexFormat::kETC1A4;
		const std::uint8_t *at = data;
		for ( int ty = 0; ty < height; ty += 8 )
			for ( int tx = 0; tx < width; tx += 8 )
				for ( int b = 0; b < 4; ++b )
				{
					const int bx = tx + ( b & 1 ) * 4;
					const int by = ty + ( b >> 1 ) * 4;
					std::uint64_t alphaWord = ~0ull;
					if ( alpha )
					{
						alphaWord = GetLE( at, 8 );
						at += 8;
					}
					std::uint8_t texels[4][4][4];
					DecodeEtc1Block( GetLE( at, 8 ), texels );
					at += 8;
					for ( int y = 0; y < 4; ++y )
						for ( int x = 0; x < 4; ++x )
						{
							std::uint8_t *p = &rgba[( ( height - 1 - ( by + y ) ) * width + bx + x ) * 4];
							std::memcpy( p, texels[y][x], 3 );
							p[3] = std::uint8_t( ( ( alphaWord >> ( ( x * 4 + y ) * 4 ) ) & 15 ) * 17 );
						}
				}
		return true;
	}
	const int bytes = BitsPerTexel( format ) / 8;
	for ( int y = 0; y < height; ++y )
		for ( int x = 0; x < width; ++x )
			UnpackTexel( format,
				std::uint32_t( GetLE( data + std::size_t( TiledIndex( x, y, width ) ) * bytes, bytes ) ),
				&rgba[( ( height - 1 - y ) * width + x ) * 4] );
	return true;
}

} // namespace pica
