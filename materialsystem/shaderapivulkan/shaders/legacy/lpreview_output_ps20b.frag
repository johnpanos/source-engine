#version 450
// A port of stdshaders/lpreview_output_ps2x.fxc (ps20b), which
// screenspace_general draws (editor/sample_result_*): the albedo (sampler 1)
// or the input (sampler 0) by the flags texture's red. Combos
// (fxctmp9/lpreview_output_ps20b.inc): static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=lpreview_output ps=lpreview_output_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:2d,2:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D InputTexture; // s0
layout( set = 0, binding = 1 ) uniform sampler2D Albedo;       // s1
layout( set = 0, binding = 2 ) uniform sampler2D Flags;        // s2

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec3 flags_check = tex2D( 2, Flags, texCoord ).rgb;
	vec3 inputColor = tex2D( 0, InputTexture, texCoord ).rgb;
	vec3 albedo = tex2D( 1, Albedo, texCoord ).rgb;
	inputColor = mix( albedo, inputColor, flags_check.r );
	LegacyWrite( FinalOutput(
	    vec4( inputColor.xyz, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
