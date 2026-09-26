#version 450
// A port of stdshaders/HDRSelectRange_vs20.fxc: the position passes through (a clip-space
// quad), with TEXCOORD0.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 texCoord;

void main()
{
	gl_Position = LegacyClipSpace( vec4( inWorldPos, 1.0 ) );
	texCoord = inTexCoord0;
}
