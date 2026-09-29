// The render core's tone map (RFC 0016 "Output", render.output.v1): the one
// GLSL copy. Scene values are linear, in multiples of SDR reference white.
//
// OutputToneMap clips each channel to the scene's peak (the brightest value
// the scene is graded to), then maps max(R, G, B) from [0, scenePeak] onto
// [0, headroom] with the ITU-R BT.2390 EETF (the Hermite knee in the
// SMPTE ST 2084 (PQ) domain, black level zero), scaling the three channels
// by one factor so hue is kept. When the display shows at least the scene's
// peak (headroom >= scenePeak) it is the clip alone: at SDR (peak 1,
// headroom 1) it is exactly the legacy clip to [0, 1].
//
// Reference white is 203 cd/m^2 (ITU-R BT.2408) for the PQ domain; a scene
// peak is at most 10000 / 203 (PQ's range), so the knee start is never
// below 0.37 for any headroom of at least 1.
//
// The C++ reference, which shares no code with this file, is
// unittests/rendertest/core/pass/output/output_oracle.h.

#ifndef RENDER_SHADERS_COMMON_TONE_MAP_GLSL
#define RENDER_SHADERS_COMMON_TONE_MAP_GLSL

const float kOutputReferenceWhiteNits = 203.0;

// SMPTE ST 2084 inverse EOTF: cd/m^2 in [0, 10000] to a PQ value in [0, 1].
float OutputPqEncode( float nits )
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 2523.0 / 4096.0 * 128.0;
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 2413.0 / 4096.0 * 32.0;
	const float c3 = 2392.0 / 4096.0 * 32.0;
	const float y = pow( clamp( nits / 10000.0, 0.0, 1.0 ), m1 );
	return pow( ( c1 + c2 * y ) / ( 1.0 + c3 * y ), m2 );
}

// SMPTE ST 2084 EOTF: a PQ value in [0, 1] to cd/m^2.
float OutputPqDecode( float pq )
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 2523.0 / 4096.0 * 128.0;
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 2413.0 / 4096.0 * 32.0;
	const float c3 = 2392.0 / 4096.0 * 32.0;
	const float p = pow( clamp( pq, 0.0, 1.0 ), 1.0 / m2 );
	return 10000.0 * pow( max( p - c1, 0.0 ) / ( c2 - c3 * p ), 1.0 / m1 );
}

// One value through the BT.2390 EETF: the value in multiples of SDR white,
// the scene peak's PQ value and the target's normalized PQ peak maxLum < 1.
// Below the knee the value is returned unchanged (no PQ round trip).
float OutputMapPeak( float value, float srcPq, float maxLum )
{
	const float e1 = OutputPqEncode( value * kOutputReferenceWhiteNits ) / srcPq;
#if defined( SEEDED_KNEE_AT_PEAK )
	const float knee = maxLum;
#else
	const float knee = 1.5 * maxLum - 0.5;
#endif
	if ( e1 < knee )
		return value;
	const float t = ( e1 - knee ) / ( 1.0 - knee );
	const float t2 = t * t;
	const float t3 = t2 * t;
	const float e2 = ( 2.0 * t3 - 3.0 * t2 + 1.0 ) * knee + ( t3 - 2.0 * t2 + t ) * ( 1.0 - knee ) +
	                 ( -2.0 * t3 + 3.0 * t2 ) * maxLum;
	return OutputPqDecode( e2 * srcPq ) / kOutputReferenceWhiteNits;
}

vec3 OutputToneMap( vec3 rgb, float scenePeak, float headroom )
{
	rgb = clamp( rgb, 0.0, scenePeak );
#if !defined( SEEDED_ALWAYS_COMPRESS )
	if ( headroom >= scenePeak )
		return rgb;
#endif
	const float srcPq = OutputPqEncode( scenePeak * kOutputReferenceWhiteNits );
	const float maxLum = OutputPqEncode( headroom * kOutputReferenceWhiteNits ) / srcPq;
#if defined( SEEDED_PER_CHANNEL )
	return vec3( OutputMapPeak( rgb.r, srcPq, maxLum ), OutputMapPeak( rgb.g, srcPq, maxLum ),
	    OutputMapPeak( rgb.b, srcPq, maxLum ) );
#else
	const float peak = max( rgb.r, max( rgb.g, rgb.b ) );
	if ( peak <= 0.0 )
		return rgb;
	return rgb * ( OutputMapPeak( peak, srcPq, maxLum ) / peak );
#endif
}

#endif
