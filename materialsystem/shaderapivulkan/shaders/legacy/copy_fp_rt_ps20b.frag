#version 450
// A port of stdshaders/copy_fp_rt_ps2x.fxc (ps20b), which screenspace_general
// draws (editor/copyalbedo, editor/addlight0): the texture's color with alpha
// 1. Combos (fxctmp9/copy_fp_rt_ps20b.inc): static CONVERT_TO_SRGB (1, always 0
// here).
// @legacy program=copy_fp_rt ps=copy_fp_rt_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D InputTexture; // s0

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec4 inputColor = tex2D( 0, InputTexture, texCoord );
	LegacyWrite( FinalOutput(
	    vec4( inputColor.xyz, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
