#version 450
// Sample4x4's pixel stage ($pixshader sample4x4_blend): a port of
// stdshaders/sample4x4_blend_ps2x.fxc (ps20b), the exponent of the average of
// four taps with alpha 0.01 (Sample4x4_Blend blends it). Combos
// (fxctmp9/sample4x4_blend_ps20b.inc): static CONVERT_TO_SRGB (1, always 0
// here).
// @legacy program=sample4x4_blend ps=sample4x4_blend_ps20b vs=downsample_vs20 vert=downsample_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 coordTap0;
layout( location = 1 ) in vec2 coordTap1;
layout( location = 2 ) in vec2 coordTap2;
layout( location = 3 ) in vec2 coordTap3;


void main()
{
	// Four bilinear taps average sixteen texels.
	vec3 s0 = tex2D( 0, TexSampler, coordTap0 ).rgb;
	vec3 s1 = tex2D( 0, TexSampler, coordTap1 ).rgb;
	vec3 s2 = tex2D( 0, TexSampler, coordTap2 ).rgb;
	vec3 s3 = tex2D( 0, TexSampler, coordTap3 ).rgb;
	LegacyWrite( FinalOutput( vec4( exp( 0.25 * ( s0 + s1 + s2 + s3 ) ), 0.01 ), 0.0,
	    PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
