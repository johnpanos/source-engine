#version 450
// A port of stdshaders/motion_blur_vs20.fxc: the position passes through
// untransformed (DrawScreenSpaceRectangle's clip-space quad), with TEXCOORD0.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vUv0;

void main()
{
	gl_Position = vec4( inWorldPos, 1.0 );
	gl_ClipDistance[0] = dot( pc.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( pc.clipPlanes[1], gl_Position );
	vUv0 = inTexCoord0;
}
