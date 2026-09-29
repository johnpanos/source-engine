//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The C++ reference of render.output.v1 (RFC 0016 "Output"): the
//			tone map and output encodings of render.pass.output, in double
//			precision, sharing no code with the GLSL (tone_map.glsl,
//			color_encoding.glsl). SMPTE ST 2084 constants and the ITU-R
//			BT.2390 EETF are written from their standards.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_PASS_OUTPUT_ORACLE_H
#define RENDERTEST_CORE_PASS_OUTPUT_ORACLE_H

#include <algorithm>
#include <array>
#include <cmath>

namespace rendertest::output
{

using Rgb = std::array<double, 3>;

constexpr double kWhiteNits = 203.0; // ITU-R BT.2408 reference white

// ST 2084: cd/m^2 to PQ.
inline double Pq( double nits )
{
	const double m1 = 0.1593017578125, m2 = 78.84375;
	const double c1 = 0.8359375, c2 = 18.8515625, c3 = 18.6875;
	const double y = std::pow( std::clamp( nits / 10000.0, 0.0, 1.0 ), m1 );
	return std::pow( ( c1 + c2 * y ) / ( 1.0 + c3 * y ), m2 );
}

// ST 2084: PQ to cd/m^2.
inline double PqInverse( double pq )
{
	const double m1 = 0.1593017578125, m2 = 78.84375;
	const double c1 = 0.8359375, c2 = 18.8515625, c3 = 18.6875;
	const double p = std::pow( std::clamp( pq, 0.0, 1.0 ), 1.0 / m2 );
	return 10000.0 * std::pow( std::max( p - c1, 0.0 ) / ( c2 - c3 * p ), 1.0 / m1 );
}

// BT.2390's knee start for a normalized target peak.
inline double KneeStart( double maxLum )
{
	return 1.5 * maxLum - 0.5;
}

// The mapped value of one magnitude (multiples of white) whose scene peak is
// `peak`, for a display showing `headroom`; valid for headroom < peak.
inline double MapMagnitude( double value, double peak, double headroom )
{
	const double source = Pq( peak * kWhiteNits );
	const double maxLum = Pq( headroom * kWhiteNits ) / source;
	const double ks = KneeStart( maxLum );
	const double e1 = Pq( value * kWhiteNits ) / source;
	if ( e1 < ks )
		return value;
	const double t = ( e1 - ks ) / ( 1.0 - ks );
	const double hermite = ( 2 * t * t * t - 3 * t * t + 1 ) * ks +
	                       ( t * t * t - 2 * t * t + t ) * ( 1 - ks ) +
	                       ( -2 * t * t * t + 3 * t * t ) * maxLum;
	return PqInverse( hermite * source ) / kWhiteNits;
}

// The value (multiples of white) where compression starts.
inline double KneeValue( double peak, double headroom )
{
	const double source = Pq( peak * kWhiteNits );
	const double ks = KneeStart( Pq( headroom * kWhiteNits ) / source );
	return PqInverse( ks * source ) / kWhiteNits;
}

// Exposure, the per-channel clip to the peak, and the hue-keeping EETF.
inline Rgb ToneMap( Rgb in, double exposure, double peak, double headroom )
{
	Rgb c;
	for ( int i = 0; i < 3; ++i )
		c[i] = std::clamp( in[i] * exposure, 0.0, peak );
	if ( headroom >= peak )
		return c;
	const double m = std::max( { c[0], c[1], c[2] } );
	if ( m <= 0.0 )
		return c;
	const double scale = MapMagnitude( m, peak, headroom ) / m;
	return { c[0] * scale, c[1] * scale, c[2] * scale };
}

// The sRGB curve (IEC 61966-2-1), clipped to [0, 1].
inline double SrgbEncode( double linear )
{
	const double c = std::clamp( linear, 0.0, 1.0 );
	return c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow( c, 1.0 / 2.4 ) - 0.055;
}

} // namespace rendertest::output

#endif // RENDERTEST_CORE_PASS_OUTPUT_ORACLE_H
