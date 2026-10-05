// render.pass.post (RFC 0016 K8 "Post and screen effects"): the engine's
// bloom chain, ported from the native backend's screenspace_post.frag
// (Downsample_nohdr_ps2x.fxc and BlurFilter_ps2x.fxc). Mode 0 reads the
// linear scene and writes display-referred bloom; modes 1 and 2 blur it.
#version 450

#include "../../shaders/common/color_transfer.glsl"

layout( set = 3, binding = 0 ) uniform texture2D sourceTexture;
layout( set = 3, binding = 1 ) uniform sampler sourceSampler; // bilinear, clamped

layout( push_constant ) uniform Constants
{
	vec4 tint;   // r_bloomtint r, g, b and exponent
	vec4 params; // x exposure, y bloom amount, zw the source's texel size
	uvec4 modes; // x: 0 downsample, 1 blur x, 2 blur y
} constants;

layout( location = 0 ) out vec4 outColor;

// The frame buffer the legacy chain read: the scene exposed, clipped to SDR
// white and sRGB encoded.
vec3 Encoded( ivec2 texel )
{
	const ivec2 size = textureSize( sampler2D( sourceTexture, sourceSampler ), 0 );
	const vec3 linear =
	    texelFetch( sampler2D( sourceTexture, sourceSampler ), clamp( texel, ivec2( 0 ), size - 1 ), 0 )
	        .rgb;
#if defined( SEEDED_POST_LINEAR_SHAPE )
	return clamp( linear * constants.params.x, 0.0, 1.0 );
#else
	return OutputSrgbExtended( clamp( linear * constants.params.x, 0.0, 1.0 ) );
#endif
}

// Downsample_nohdr's Shape (SRGB_ADAPTER 0): luminance against the tint,
// times the pixel raised to the tint's exponent.
vec3 Shape( vec3 pixel )
{
	const float lum = dot( pixel, constants.tint.rgb );
	return pow( pixel, vec3( constants.tint.w ) ) * lum;
}

// One bilinear tap at the shared corner of a 2x2 block, in encoded values.
vec3 Tap( ivec2 corner )
{
	return ( Encoded( corner ) + Encoded( corner + ivec2( 1, 0 ) ) + Encoded( corner + ivec2( 0, 1 ) ) +
	           Encoded( corner + ivec2( 1, 1 ) ) ) *
	       0.25;
}

vec3 Downsample()
{
	const ivec2 base = ivec2( gl_FragCoord.xy ) * 4;
	return ( Shape( Tap( base ) ) + Shape( Tap( base + ivec2( 2, 0 ) ) ) +
	           Shape( Tap( base + ivec2( 0, 2 ) ) ) + Shape( Tap( base + ivec2( 2, 2 ) ) ) ) *
	       0.25;
}

const float kWeights[7] = float[7]( 0.2013, 0.2185, 0.0821, 0.0461, 0.0262, 0.0162, 0.0102 );
const float kOffsets[7] = float[7]( 0.0, 1.3366, 3.4295, 5.4264, 7.4359, 9.4436, 11.4401 );

vec3 Blur( vec2 axis )
{
	const vec2 uv = gl_FragCoord.xy * constants.params.zw;
	vec3 color = texture( sampler2D( sourceTexture, sourceSampler ), uv ).rgb * kWeights[0];
	for ( int tap = 1; tap < 7; ++tap )
	{
		const vec2 offset = axis * kOffsets[tap];
		color += ( texture( sampler2D( sourceTexture, sourceSampler ), uv + offset ).rgb +
		             texture( sampler2D( sourceTexture, sourceSampler ), uv - offset ).rgb ) *
		         kWeights[tap];
	}
	return color;
}

void main()
{
	vec3 color;
	if ( constants.modes.x == 0u )
		color = Downsample();
	else if ( constants.modes.x == 1u )
		color = Blur( vec2( constants.params.z, 0.0 ) );
	else
	{
		// BlurFilterY steps by 1 / width: the legacy quirk the port keeps.
#if defined( SEEDED_POST_BLUR_Y_HEIGHT_STEP )
		color = Blur( vec2( 0.0, constants.params.w ) ) * constants.params.y;
#else
		color = Blur( vec2( 0.0, constants.params.z ) ) * constants.params.y;
#endif
	}
	outColor = vec4( color, 1.0 );
}
