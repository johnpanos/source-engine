//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Small sampling image of an opaque procedural RGB/BGR texture. Rebuilt from
// the same CPU pixels as an upload; no video identity or renderer dependency.
// RGB is averaged in linear light, stored as gamma bytes for ITexture's frozen
// GetLowResColorSample API. Sixteen stratified samples per cell bound the work.
//
//=============================================================================//
#ifndef PROCEDURAL_TEXTURE_SAMPLE_H
#define PROCEDURAL_TEXTURE_SAMPLE_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

namespace procedural_texture_sample
{

constexpr int kDimension = 64;
constexpr int kBytes = kDimension * kDimension * 3;

inline bool Build( std::span<const std::uint8_t> pixels, int width, int height, int stride,
    bool bgr, std::span<std::uint8_t> output )
{
	if ( width <= 0 || height <= 0 || stride <= 0 ||
	     std::size_t( width ) * 3 > std::size_t( stride ) ||
	     std::size_t( height ) > pixels.size() / stride || output.size() != kBytes )
		return false;
	static const auto linear = []
	{
		std::array<float, 256> values;
		for ( int i = 0; i < 256; ++i )
		{
			const float v = float( i ) / 255.0f;
			values[i] = v <= 0.04045f ? v / 12.92f : std::pow( ( v + 0.055f ) / 1.055f, 2.4f );
		}
		return values;
	}();
	for ( int y = 0; y < kDimension; ++y )
	{
		for ( int x = 0; x < kDimension; ++x )
		{
			float sum[3] = {};
			for ( int sy = 0; sy < 4; ++sy )
			{
				const int py = std::min( height - 1,
				    int( ( std::int64_t( y * 8 + sy * 2 + 1 ) * height ) / ( kDimension * 8 ) ) );
				for ( int sx = 0; sx < 4; ++sx )
				{
					const int px =
					    std::min( width - 1, int( ( std::int64_t( x * 8 + sx * 2 + 1 ) * width ) /
					                              ( kDimension * 8 ) ) );
					const auto at = std::size_t( py ) * stride + std::size_t( px ) * 3;
					for ( int k = 0; k < 3; ++k )
						sum[k] += linear[pixels[at + ( bgr ? 2 - k : k )]];
				}
			}
			for ( int k = 0; k < 3; ++k )
			{
				const float v = sum[k] / 16.0f;
				const float gamma =
				    v <= 0.0031308f ? v * 12.92f : 1.055f * std::pow( v, 1.0f / 2.4f ) - 0.055f;
				output[( y * kDimension + x ) * 3 + k] =
				    std::uint8_t( std::clamp( std::lround( gamma * 255.0f ), 0l, 255l ) );
			}
		}
	}
	return true;
}

} // namespace procedural_texture_sample

#endif
