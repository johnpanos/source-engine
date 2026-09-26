#version 450
// TreeLeaf's pixel stage: a port of stdshaders/treeleaf_ps2x.fxc (ps20b), the
// base texture times the vertex lighting (alpha tested GREATER 0.5 by the
// shader). Combos (fxctmp9/treeleaf_ps20b.inc): static CONVERT_TO_SRGB (1,
// always 0 here).
// @legacy program=treeleaf ps=treeleaf_ps20b vs=treeleaf_vs20 vert=treeleaf_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 texCoord0;
layout( location = 8 ) in vec3 color;

void main()
{
	const vec4 baseTex = tex2D( 0, BaseTextureSampler, texCoord0 );
	LegacyWrite(
	    FinalOutput( baseTex * vec4( color, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
