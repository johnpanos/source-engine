#version 450
// DepthWrite's pixel stage (drawn when the material alpha-tests or writes depth
// as color): a port of stdshaders/depthwrite_ps2x.fxc (ps20b). Combos
// (fxctmp9/depthwrite_ps20b.inc): static COLOR_DEPTH (2); dynamic ALPHACLIP (1).
// @legacy program=depthwrite ps=depthwrite_ps20b vs=depthwrite_vs20 vert=depthwrite_vs20
//         samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 texCoord0;
layout( location = 1 ) in vec4 vWorldPos_projPosZ;

#define g_AlphaThreshold ( PS_C( 0 ).x )
#define g_vNearFarPlanes ( PS_C( 1 ).xy )
#define g_flNearPlane ( g_vNearFarPlanes.x )
#define g_flFarPlane ( g_vNearFarPlanes.y )

void main()
{
	const bool COLOR_DEPTH = STATIC_PS_COMBO( 2, 2 ) != 0;
	const bool ALPHACLIP = DYNAMIC_PS_COMBO( 1, 2 ) != 0;

	vec4 color = vec4( 1.0, 0.0, 0.0, 1.0 ); // opaque alpha....the color doesn't matter for this shader

	if ( ALPHACLIP )
	{
		color = tex2D( 0, BaseTextureSampler, texCoord0 );
		if ( color.a - g_AlphaThreshold < 0.0 )
			discard;
	}

	if ( COLOR_DEPTH )
		LegacyWrite( vec4( vWorldPos_projPosZ.w / g_flFarPlane, 0.0, 0.0, 1.0 ) );
	else
		LegacyWrite( color );
}
