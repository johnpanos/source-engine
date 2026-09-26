#version 450
// A port of stdshaders/eyeglint_vs20.fxc (EyeGlint_dx9's vertex stage): the
// position passes through untransformed (studiorender draws the glint quad in
// clip space), with TEXCOORD0, TEXCOORD1 and the three-component TEXCOORD2.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 tc;
layout( location = 1 ) out vec2 glintCenter;
layout( location = 2 ) out vec3 glintColor;

void main()
{
	gl_Position = vec4( inWorldPos, 1.0 );
	gl_ClipDistance[0] = dot( pc.clipPlanes[0], gl_Position );
	gl_ClipDistance[1] = dot( pc.clipPlanes[1], gl_Position );
	tc = inTexCoord0;
	glintCenter = inTexCoord1;
	glintColor = inExtra.xyz; // TEXCOORD2
}
