#version 450
// A port of stdshaders/screenspaceeffect_vs20.fxc, the vertex stage of the
// screen-space effects (screenspace_general and the post-processing shaders).
// Combos (fxctmp9/screenspaceeffect_vs20.inc): static X360APPCHOOSER (1): the
// position through cModelViewProj and the vertex color in TEXCOORD3; without
// it the position passes through as is.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec2 ZeroTexCoord;
layout( location = 2 ) out vec2 bloomTexCoord;
layout( location = 3 ) out vec4 vColor;

#define Texel_Sizes VS_C( 48 )

void main()
{
	const bool X360APPCHOOSER = STATIC_VS_COMBO( 1, 2 ) != 0;

	vec4 projPos = vec4( inWorldPos, 1.0 );
	baseTexCoord = inTexCoord0;
	ZeroTexCoord = vec2( 0.0 );
	bloomTexCoord = inTexCoord0 + Texel_Sizes.zw;
	vColor = vec4( 0.0 );
	if ( X360APPCHOOSER )
	{
		vColor = vec4( inColor, inAlpha );
		projPos = MulModelViewProj( projPos );
	}
	gl_Position = LegacyClipSpace( projPos );
}
