#version 450
// DebugMRTTexture's pixel stage: a port of stdshaders/debugmrttexture_ps2x.fxc
// (ps20b), the texture of sampler 0 (MRTINDEX 0) or 1 (MRTINDEX 1, which the
// shader never binds) as is. Combos (fxctmp9/debugmrttexture_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here), MRTINDEX (2).
// @legacy program=debugmrttexture ps=debugmrttexture_ps20b vs=debugmrttexture_vs20
//         vert=debugmrttexture_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler1; // s0
layout( set = 0, binding = 1 ) uniform sampler2D BaseTextureSampler2; // s1

layout( location = 0 ) in vec2 baseTexCoord;

void main()
{
	const int MRTINDEX = STATIC_PS_COMBO( 2, 2 );

	vec4 result = MRTINDEX == 0 ? tex2D( 0, BaseTextureSampler1, baseTexCoord )
	                            : tex2D( 1, BaseTextureSampler2, baseTexCoord );

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
