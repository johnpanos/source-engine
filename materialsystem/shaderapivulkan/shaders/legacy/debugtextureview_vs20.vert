#version 450
// A port of stdshaders/DebugTextureView_vs20.fxc (its COMPRESSED_VERTS combo
// the shader API already decoded): the position through cModelViewProj, with
// TEXCOORD0.
#include "legacy_screen_vs.glsl"

layout( location = 0 ) out vec2 vBaseTexCoords;

void main()
{
	gl_Position = LegacyClipSpace( MulModelViewProj( vec4( inWorldPos, 1.0 ) ) );
	vBaseTexCoords = inTexCoord0;
}
