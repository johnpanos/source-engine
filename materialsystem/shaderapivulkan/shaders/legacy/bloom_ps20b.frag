#version 450
// Bloom's pixel stage: a port of stdshaders/Bloom_ps2x.fxc (ps20b). Combos
// (fxctmp9/Bloom_ps20b.inc): static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=bloom ps=bloom_ps20b vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20
//         samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FBSampler;   // s0
layout( set = 0, binding = 1 ) uniform sampler2D BlurSampler; // s1

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec4 fbSample = tex2D( 0, FBSampler, texCoord );
	vec4 blurSample = tex2D( 1, BlurSampler, texCoord );

	LegacyWrite( FinalOutput(
	    vec4( fbSample.rgb + blurSample.rgb * blurSample.a * MAX_HDR_OVERBRIGHT, 1.0 ), 0.0,
	    PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
