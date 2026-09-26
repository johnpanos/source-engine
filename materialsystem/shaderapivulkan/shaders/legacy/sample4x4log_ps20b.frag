#version 450
// Sample4x4's pixel stage ($pixshader sample4x4log): a port of
// stdshaders/sample4x4log_ps2x.fxc (ps20b), the average log luminance of four
// taps, alpha AlphaConst. Combos (fxctmp9/sample4x4log_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=sample4x4log ps=sample4x4log_ps20b vs=downsample_vs20 vert=downsample_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 coordTap0;
layout( location = 1 ) in vec2 coordTap1;
layout( location = 2 ) in vec2 coordTap2;
layout( location = 3 ) in vec2 coordTap3;

#define AlphaConst ( PS_C( 0 ).x )
#define LOG_EPSILON 0.000001

float logluminance( vec3 color )
{
	return log( 0.2125 * color.x + 0.7154 * color.y + 0.0721 * color.z + LOG_EPSILON );
}

void main()
{
	// Four bilinear taps average sixteen texels.
	vec3 s0 = tex2D( 0, TexSampler, coordTap0 ).rgb;
	vec3 s1 = tex2D( 0, TexSampler, coordTap1 ).rgb;
	vec3 s2 = tex2D( 0, TexSampler, coordTap2 ).rgb;
	vec3 s3 = tex2D( 0, TexSampler, coordTap3 ).rgb;
	float loglum0 = logluminance( s0 );
	float loglum1 = logluminance( s1 );
	float loglum2 = logluminance( s2 );
	float loglum3 = logluminance( s3 );

	LegacyWrite( FinalOutput( vec4( 0.25 * ( loglum0 + loglum1 + loglum2 + loglum3 ), 0.0, 0.5,
	                              AlphaConst ),
	    0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
