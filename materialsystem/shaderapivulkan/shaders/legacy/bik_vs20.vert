#version 450
// A port of stdshaders/bik_vs20.fxc (Bik's vertex stage). Combos
// (fxctmp9/bik_vs20.inc): dynamic DOWATERFOG (1); vs_2_0 leaves water fog to
// the pixel shader (fog 1).
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec4 worldPos_projPosZ;
layout( location = 9 ) out vec4 fogFactorW;

void main()
{
	const bool DOWATERFOG = DYNAMIC_VS_COMBO( 1, 2 ) != 0;
	const vec3 worldPos = inWorldPos;
	const vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;

	fogFactorW = vec4( 0.0 );
	fogFactorW.w = DOWATERFOG ? 1.0 : RangeFog( projPos.xyz );

	worldPos_projPosZ = vec4( worldPos, projPos.z );
	baseTexCoord = inTexCoord0;
}
