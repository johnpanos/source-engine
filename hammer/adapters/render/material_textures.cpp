//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer/adapters/render/material_textures.h.
//
//=============================================================================//

#include "material_textures.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace hammer::render_adapter
{

namespace
{

// Linear light in 1/65535 steps: the sRGB decode of a byte, and the nearest
// sRGB byte of a step. The step is 20 times finer than the darkest byte's
// (1 / 255 / 12.92), so the tables round as the exact curves do except
// within 1e-5 of a byte boundary; sums of steps stay in integers.
constexpr std::uint32_t kSteps = 65535;

const std::array<std::uint32_t, 256> &DecodeTable()
{
	static const std::array<std::uint32_t, 256> table = []
	{
		std::array<std::uint32_t, 256> out{};
		for ( int i = 0; i < 256; ++i )
		{
			const double c = i / 255.0;
			const double l = c <= 0.04045 ? c / 12.92 : std::pow( ( c + 0.055 ) / 1.055, 2.4 );
			out[std::size_t( i )] = std::uint32_t( std::lround( l * kSteps ) );
		}
		return out;
	}();
	return table;
}

const std::vector<std::uint8_t> &EncodeTable()
{
	static const std::vector<std::uint8_t> table = []
	{
		std::vector<std::uint8_t> out( kSteps + 1 );
		for ( std::uint32_t i = 0; i <= kSteps; ++i )
		{
			const double l = double( i ) / kSteps;
			const double c = l <= 0.0031308 ? l * 12.92 : 1.055 * std::pow( l, 1.0 / 2.4 ) - 0.055;
			out[i] = std::uint8_t( std::lround( std::clamp( c, 0.0, 1.0 ) * 255.0 ) );
		}
		return out;
	}();
	return table;
}

// The source span of output texel 'x' on an axis of 'source' texels
// reduced to 'target' (see BuildMipChain).
void Span( std::uint32_t x, std::uint32_t source, std::uint32_t target, std::uint32_t &first,
    std::uint32_t &last )
{
	first = std::min( 2 * x, source - 1 );
	last = x + 1 == target ? source - 1 : std::min( 2 * x + 1, source - 1 );
}

MipLevel Reduce( const MipLevel &above )
{
	const std::array<std::uint32_t, 256> &decode = DecodeTable();
	const std::vector<std::uint8_t> &encode = EncodeTable();
	MipLevel level;
	level.width = std::max( above.width / 2, 1u );
	level.height = std::max( above.height / 2, 1u );
	level.rgba.resize( std::size_t( level.width ) * level.height * 4 );
	const std::size_t pitch = std::size_t( above.width ) * 4;
	for ( std::uint32_t y = 0; y < level.height; ++y )
	{
		std::uint32_t y0 = 0;
		std::uint32_t y1 = 0;
		Span( y, above.height, level.height, y0, y1 );
		std::uint8_t *out = level.rgba.data() + std::size_t( y ) * level.width * 4;
		for ( std::uint32_t x = 0; x < level.width; ++x, out += 4 )
		{
			std::uint32_t x0 = 0;
			std::uint32_t x1 = 0;
			Span( x, above.width, level.width, x0, x1 );
			std::uint32_t color[3] = {};
			std::uint32_t alpha = 0;
			for ( std::uint32_t sy = y0; sy <= y1; ++sy )
			{
				const std::uint8_t *texel = above.rgba.data() + sy * pitch + std::size_t( x0 ) * 4;
				for ( std::uint32_t sx = x0; sx <= x1; ++sx, texel += 4 )
				{
					color[0] += decode[texel[0]];
					color[1] += decode[texel[1]];
					color[2] += decode[texel[2]];
					alpha += texel[3];
				}
			}
			const std::uint32_t count = ( x1 - x0 + 1 ) * ( y1 - y0 + 1 );
			for ( int c = 0; c < 3; ++c )
				out[c] = encode[( color[c] + count / 2 ) / count];
			out[3] = std::uint8_t( ( alpha + count / 2 ) / count );
		}
	}
	return level;
}

} // namespace

std::vector<MipLevel> BuildMipChain( const MaterialImage &image )
{
	std::vector<MipLevel> chain;
	if ( image.width == 0 || image.height == 0 ||
	     image.rgba.size() != std::size_t( image.width ) * image.height * 4 )
	{
		return chain;
	}
	chain.push_back( { image.width, image.height, image.rgba } );
	while ( chain.back().width > 1 || chain.back().height > 1 )
	{
		chain.push_back( Reduce( chain.back() ) );
	}
	return chain;
}

} // namespace hammer::render_adapter
