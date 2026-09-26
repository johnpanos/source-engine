#version 450
// accumbuff5sample's pixel stage: a port of stdshaders/accumbuff5sample_ps2x.fxc
// (ps20b): four textures weighted by weights.x and a fifth by weights.y.
// Combos (fxctmp9/accumbuff5sample_ps20b.inc): static CONVERT_TO_SRGB (1,
// always 0 here).
// @legacy program=accumbuff5sample ps=accumbuff5sample_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:2d,2:2d,3:2d,4:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler0; // s0
layout( set = 0, binding = 1 ) uniform sampler2D TexSampler1; // s1
layout( set = 0, binding = 2 ) uniform sampler2D TexSampler2; // s2
layout( set = 0, binding = 3 ) uniform sampler2D TexSampler3; // s3
layout( set = 0, binding = 4 ) uniform sampler2D TexSampler4; // s4

layout( location = 0 ) in vec2 texCoord;

#define weights PS_C( 0 )

void main()
{
	vec4 sample0 = tex2D( 0, TexSampler0, texCoord );
	vec4 sample1 = tex2D( 1, TexSampler1, texCoord );
	vec4 sample2 = tex2D( 2, TexSampler2, texCoord );
	vec4 sample3 = tex2D( 3, TexSampler3, texCoord );
	vec4 sample4 = tex2D( 4, TexSampler4, texCoord );

	LegacyWrite( FinalOutput( weights.x * sample0 + weights.x * sample1 + weights.x * sample2 +
	        weights.x * sample3 + weights.y * sample4,
	    0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
