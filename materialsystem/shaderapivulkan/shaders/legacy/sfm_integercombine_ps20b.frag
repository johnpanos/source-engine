#version 450
// sfm_integercombine_shader's pixel stage: a port of
// stdshaders/sfm_integercombine_ps2x.fxc (ps20b), the original plus the blurred
// image times the bloom amount. Combos (fxctmp9/sfm_integercombine_ps20b.inc):
// static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=sfm_integercombine ps=sfm_integercombine_ps20b vs=sfm_combine_vs20
//         vert=sfm_combine_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler0; // s0
layout( set = 0, binding = 1 ) uniform sampler2D TexSampler1; // s1

layout( location = 0 ) in vec2 tc0;
layout( location = 1 ) in vec2 tc1;

#define bloomamount PS_C( 0 )

void main()
{
	vec4 c0 = tex2D( 0, TexSampler0, tc0 );
	vec4 c1 = tex2D( 1, TexSampler1, tc1 );

	LegacyWrite( FinalOutput( c0 + bloomamount.xxxx * c1, 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_NONE ) );
}
