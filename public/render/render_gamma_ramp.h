//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The monitor gamma ramp the material system requests with
//          SetHardwareGammaRamp (render.gamma-ramp.v1).
//
//          One owner for the curve: the D3D9 device hands it to the hardware
//          ramp, and providers without a window-system ramp (SDL3, Vulkan)
//          apply it when presenting.
//
//===========================================================================//

#ifndef RENDER_GAMMA_RAMP_H
#define RENDER_GAMMA_RAMP_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace render
{
// mat_monitorgamma and the mat_monitorgamma_tv_* convars.
struct GammaRampParams
{
	float gamma = 2.2f;
	float tvRangeMin = 16.0f;
	float tvRangeMax = 255.0f;
	float tvExponent = 2.5f;
	bool tvEnabled = false;
};

// 256 entries mapping an 8-bit frame-buffer value to a 16-bit output value, the
// layout of D3DGAMMARAMP's channels. The ramp is the same for all channels.
using GammaRamp16 = std::array<uint16_t, 256>;

// The frame buffer is gamma 2.2; the user exponent is applied relative to it.
// The TV adjustment re-encodes for the TV exponent, then scales and biases
// into [tvRangeMin, tvRangeMax] / 255. Values truncate to 16 bits, as the D3D9
// device always has (CShaderDeviceDx8::SetHardwareGammaRamp).
inline void BuildGammaRamp16( const GammaRampParams &params, GammaRamp16 &out )
{
	for ( int i = 0; i < 256; ++i )
	{
		const float input = float( i ) / 255.0f;
		float correction = std::pow( input, params.gamma / 2.2f );
		correction = std::clamp( correction, 0.0f, 1.0f );
		if ( params.tvEnabled )
		{
			correction = std::pow( correction, 2.2f / params.tvExponent );
			correction = std::clamp( correction, 0.0f, 1.0f );
			correction = ( correction * ( params.tvRangeMax - params.tvRangeMin ) / 255.0f ) +
			             ( params.tvRangeMin / 255.0f );
			correction = std::clamp( correction, 0.0f, 1.0f );
		}
		out[i] = static_cast<uint16_t>( static_cast<int>( correction * 65535.0f ) );
	}
}

// The 8-bit output a presenter writes for a ramp entry (round to nearest).
[[nodiscard]] constexpr int GammaRampEntryTo8Bit( uint16_t value )
{
	return ( static_cast<int>( value ) * 255 + 32767 ) / 65535;
}

// True when presenting through the ramp cannot change any 8-bit value. The
// default gamma of 2.2 is such a ramp even though its 16-bit entries are not
// exactly i * 257 (truncation).
[[nodiscard]] inline bool IsIdentityAt8Bit( const GammaRamp16 &ramp )
{
	for ( int i = 0; i < 256; ++i )
	{
		if ( GammaRampEntryTo8Bit( ramp[i] ) != i )
			return false;
	}
	return true;
}
} // namespace render

#endif // RENDER_GAMMA_RAMP_H
