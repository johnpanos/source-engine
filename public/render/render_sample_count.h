//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Multisample-count policy (render.sample-count.v1).
//
//          A supported-count mask has one bit per power-of-two sample count,
//          with the bit's value equal to the count (the layout of
//          VkSampleCountFlags): bit 1 = 1 sample, bit 4 = 4 samples.
//
//===========================================================================//

#ifndef RENDER_SAMPLE_COUNT_H
#define RENDER_SAMPLE_COUNT_H

#include <cstdint>

namespace render
{
inline constexpr int kMaxSampleCount = 64;

// Whether mat_antialias n can be honoured exactly: n is a multisampled power of
// two present in the mask. 6x (an old D3D9 vendor mode) is never supported.
[[nodiscard]] constexpr bool IsMsaaModeSupported( int samples, uint32_t supportedMask )
{
	if ( samples < 2 || samples > kMaxSampleCount || ( samples & ( samples - 1 ) ) != 0 )
		return false;
	return ( supportedMask & static_cast<uint32_t>( samples ) ) != 0;
}

// The largest supported count not above the request; 1 (no multisampling) when
// the request is below 2 or nothing smaller is supported. Never rounds up.
[[nodiscard]] constexpr int ClampSampleCount( int requested, uint32_t supportedMask )
{
	for ( int samples = kMaxSampleCount; samples >= 2; samples /= 2 )
	{
		if ( samples <= requested && ( supportedMask & static_cast<uint32_t>( samples ) ) != 0 )
			return samples;
	}
	return 1;
}
} // namespace render

#endif // RENDER_SAMPLE_COUNT_H
