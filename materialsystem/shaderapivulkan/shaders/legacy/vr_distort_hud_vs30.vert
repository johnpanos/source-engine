#version 450
// A port of stdshaders/vr_distort_hud_vs20.fxc as compiled for vs_3_0
// (vr_distort_hud_vs30): the position (w 1, the stream being float3) passes
// through, with TEXCOORD0.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 vBaseTexCoord;

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );
	vBaseTexCoord = inTexCoord0;
}
