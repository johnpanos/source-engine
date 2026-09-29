// The render core's color encodings (RFC 0016): the one GLSL copy of the sRGB
// transfer function a shader applies itself, for a target without an sRGB
// view (the output encoding frame term). The same piecewise curve an sRGB
// attachment applies in hardware.
//
// OutputEncode is render.output.v1's output encoding (render.pass.output), by
// the target's kind:
//   kOutputEncodingSrgb      an 8-bit UNORM target shown in the standard range:
//                            the sRGB curve, clipped to [0, 1];
//   kOutputEncodingHardware  an 8-bit sRGB-view target: linear values clipped to
//                            [0, 1], which the attachment encodes;
//   kOutputEncodingLinear    a half-float target shown in the extended linear
//                            range (scRGB: linear Rec. 709, 1.0 = SDR white):
//                            the linear values, negatives clipped to 0.

#ifndef RENDER_SHADERS_COMMON_COLOR_ENCODING_GLSL
#define RENDER_SHADERS_COMMON_COLOR_ENCODING_GLSL

vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

const uint kOutputEncodingSrgb = 0u;
const uint kOutputEncodingHardware = 1u;
const uint kOutputEncodingLinear = 2u;

vec3 OutputEncode( vec3 linear, uint encoding )
{
#if defined( SEEDED_SRGB_ON_LINEAR )
	if ( encoding == kOutputEncodingLinear )
		return LinearToSrgb( linear );
#endif
	if ( encoding == kOutputEncodingLinear )
		return max( linear, vec3( 0.0 ) );
	if ( encoding == kOutputEncodingHardware )
		return clamp( linear, 0.0, 1.0 );
	return LinearToSrgb( linear );
}

#endif
