#version 450
// showz's pixel stage: a port of stdshaders/showz_ps2x.fxc (ps20b), a depth
// texture's red (or alpha with DEPTH_IN_ALPHA) raised to g_Parameters.x, as
// grey. Combos (fxctmp9/showz_ps20b.inc): static DEPTH_IN_ALPHA (1),
// CONVERT_TO_SRGB (2, always 0 here).
// @legacy program=showz ps=showz_ps20b vs=showz_vs20 vert=showz_vs20 samplers=0:2d
//         flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D DepthTextureSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

#define g_Parameters PS_C( 0 )

void main()
{
	const bool DEPTH_IN_ALPHA = STATIC_PS_COMBO( 1, 2 ) != 0;

	float fDepth = DEPTH_IN_ALPHA ? tex2D( 0, DepthTextureSampler, baseTexCoord ).a
	                              : tex2D( 0, DepthTextureSampler, baseTexCoord ).r;

	fDepth = HlslPow( fDepth, g_Parameters.x );

	LegacyWrite( FinalOutput( vec4( fDepth, fDepth, fDepth, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_NONE ) );
}
