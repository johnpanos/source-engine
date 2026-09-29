// The render core's color encodings (RFC 0016): the one GLSL copy of the sRGB
// transfer function a shader applies itself, for a target without an sRGB
// view (the output encoding frame term). The same piecewise curve an sRGB
// attachment applies in hardware.

#ifndef RENDER_SHADERS_COMMON_COLOR_ENCODING_GLSL
#define RENDER_SHADERS_COMMON_COLOR_ENCODING_GLSL

vec3 LinearToSrgb( vec3 c )
{
	c = clamp( c, 0.0, 1.0 );
	return mix( c * 12.92, 1.055 * pow( c, vec3( 1.0 / 2.4 ) ) - 0.055, step( 0.0031308, c ) );
}

#endif
