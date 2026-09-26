#version 450
// A port of stdshaders/sfm_combine_vs20.fxc: the position passes through, with
// TEXCOORD0 and TEXCOORD1.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec2 bloomTexCoord;

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );
	baseTexCoord = inTexCoord0;
	bloomTexCoord = inTexCoord1;
}
