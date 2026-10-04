// Extended sRGB transfers for floating point storage. Values above white survive.
#ifndef RENDER_COLOR_TRANSFER_GLSL
#define RENDER_COLOR_TRANSFER_GLSL

vec3 OutputLinearFromSrgb( vec3 rgb )
{
	rgb = max( rgb, vec3( 0.0 ) );
	return mix( rgb / 12.92, pow( ( rgb + 0.055 ) / 1.055, vec3( 2.4 ) ),
	    step( 0.04045, rgb ) );
}

vec3 OutputSrgbExtended( vec3 rgb )
{
	rgb = max( rgb, vec3( 0.0 ) );
	return mix( rgb * 12.92, 1.055 * pow( rgb, vec3( 1.0 / 2.4 ) ) - 0.055,
	    step( 0.0031308, rgb ) );
}

#endif
