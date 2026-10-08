//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.world (RFC 0016 K5): lightmap pages from a stage's layers.
//
//=============================================================================//

#include "world_pass_internal.h"

namespace render::pass::world
{

namespace
{

float LightmapHalfToFloat( std::uint16_t half )
{
	const std::uint32_t sign = std::uint32_t( half & 0x8000u ) << 16;
	std::uint32_t exponent = ( half >> 10 ) & 0x1fu;
	std::uint32_t mantissa = half & 0x3ffu;
	std::uint32_t bits;
	if ( exponent == 0 )
	{
		if ( mantissa == 0 )
			bits = sign;
		else
		{
			// A subnormal: normalize it.
			exponent = 113;
			while ( !( mantissa & 0x400u ) )
			{
				mantissa <<= 1;
				--exponent;
			}
			bits = sign | ( exponent << 23 ) | ( ( mantissa & 0x3ffu ) << 13 );
		}
	}
	else if ( exponent == 31 )
		bits = sign | 0x7f800000u | ( mantissa << 13 );
	else
		bits = sign | ( ( exponent + 112 ) << 23 ) | ( mantissa << 13 );
	float value;
	std::memcpy( &value, &bits, sizeof( value ) );
	return value;
}

// A value in [0, 1] (the gradient page's channels), rounded to nearest.
std::uint16_t LightmapUnitToHalf( float value )
{
	value = std::clamp( value, 0.0f, 1.0f );
	if ( value < 6.103515625e-05f )
		return std::uint16_t( std::lround( value * 16777216.0f ) ); // subnormal
	std::uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	const std::uint32_t exponent = ( bits >> 23 ) - 112;
	std::uint32_t half = ( exponent << 10 ) | ( ( bits >> 13 ) & 0x3ffu );
	if ( bits & 0x1000u )
		++half;
	return std::uint16_t( half );
}

} // namespace

LightmapPages SplitLightmapLayer(
    std::span<const std::byte> layer, std::uint32_t width, std::uint32_t height )
{
	constexpr std::size_t kTexel = 8; // RGBA16F
	LightmapPages pages;
	if ( layer.size() != std::size_t( width ) * height * kTexel )
		return pages; // no page: the caller reports it
	pages.height = height;
	if ( width != 2 * height )
	{
		pages.width = width;
		pages.flat.assign( layer.begin(), layer.end() );
		return pages;
	}
	pages.width = height;
	const std::size_t row = std::size_t( width ) * kTexel;
	const std::size_t half = std::size_t( pages.width ) * kTexel;
	pages.flat.resize( std::size_t( height ) * half );
	pages.gradient.resize( std::size_t( height ) * half );
	for ( std::uint32_t y = 0; y < height; ++y )
	{
		const std::byte *from = layer.data() + std::size_t( y ) * row;
		std::copy( from, from + half, pages.flat.data() + std::size_t( y ) * half );
		// The stored convention: beta / 4 + 0.5 (beta in [-2, 2]), and the
		// sun (the flat texel's alpha in the decoded form) in the gradient's
		// alpha.
		for ( std::uint32_t x = 0; x < pages.width; ++x )
		{
			std::uint16_t flat[4], beta[4], out[4];
			std::memcpy( flat, from + std::size_t( x ) * kTexel, sizeof( flat ) );
			std::memcpy( beta, from + half + std::size_t( x ) * kTexel, sizeof( beta ) );
			for ( int c = 0; c < 3; ++c )
				out[c] = LightmapUnitToHalf( LightmapHalfToFloat( beta[c] ) * 0.25f + 0.5f );
			out[3] = LightmapUnitToHalf( LightmapHalfToFloat( flat[3] ) );
			std::memcpy(
			    pages.gradient.data() + std::size_t( y ) * half + std::size_t( x ) * kTexel, out,
			    sizeof( out ) );
		}
	}
	return pages;
}

LightmapPages BlockLightmapLayer( std::uint32_t width, std::uint32_t height,
    std::span<const std::byte> irradiance, std::span<const std::byte> gradient )
{
	LightmapPages pages;
	const std::uint64_t bytes = device::RegionBytes( Format::kBC6HUfloat, width, height );
	if ( width == 0 || height == 0 || irradiance.size() != bytes || gradient.size() != bytes )
		return pages;
	pages.width = width;
	pages.height = height;
	pages.flatFormat = Format::kBC6HUfloat;
	pages.gradientFormat = Format::kBC7Unorm;
	pages.flat.assign( irradiance.begin(), irradiance.end() );
	pages.gradient.assign( gradient.begin(), gradient.end() );
	return pages;
}

} // namespace render::pass::world
