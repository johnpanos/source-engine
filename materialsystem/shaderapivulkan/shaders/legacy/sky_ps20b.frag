#version 450
// Sky_HDR_DX9's (and Sky_DX9's) uncompressed pixel stage: a port of
// stdshaders/sky_ps2x.fxc (ps20b), the base texture scaled by InputScale and
// LINEAR_LIGHT_SCALE, never fogged. Combos (fxctmp9/sky_ps20b.inc): static
// CONVERT_TO_SRGB (2, always 0 here); dynamic WRITE_DEPTH_TO_DESTALPHA (1),
// which writes a depth that saturates.
// @legacy program=sky ps=sky_ps20b vs=sky_vs20 vert=sky_vs20 samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

#define InputScale PS_C( 0 )

void main()
{
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );

	vec4 color = tex2D( 0, BaseTextureSampler, baseTexCoord );
	color.rgb *= InputScale.rgb;

	// This is never fogged.
	LegacyWrite( FinalOutput( color, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR,
	    WRITE_DEPTH_TO_DESTALPHA != 0, 1e20 ) );
}
