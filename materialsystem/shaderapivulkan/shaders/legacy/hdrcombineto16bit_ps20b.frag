#version 450
// HDRCombineTo16Bit's pixel stage: a port of stdshaders/HDRCombineTo16Bit_ps2x.fxc
// (ps20b): the low and high range images (gamma 2.2) merged, scaled down by
// MAX_HDR_OVERBRIGHT. Combos (fxctmp9/HDRCombineTo16Bit_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=hdrcombineto16bit ps=hdrcombineto16bit_ps20b vs=hdrcombineto16bit_vs20
//         vert=hdrcombineto16bit_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D LowSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D HiSampler;  // s1

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec4 lowColor = tex2D( 0, LowSampler, texCoord );
	vec3 hiColor = tex2D( 1, HiSampler, texCoord ).rgb;

	lowColor.rgb = GammaToLinear( lowColor.rgb );
	hiColor.rgb = GammaToLinear( hiColor.rgb );

	vec4 result = vec4(
	    ( 1.0 / MAX_HDR_OVERBRIGHT ) * max( lowColor.xyz, hiColor.xyz * MAX_HDR_OVERBRIGHT ),
	    lowColor.a );
	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
