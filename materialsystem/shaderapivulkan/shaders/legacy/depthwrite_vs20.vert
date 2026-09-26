#version 450
// A port of stdshaders/depthwrite_vs20.fxc (DepthWrite's vertex stage). Combos
// (fxctmp9/depthwrite_vs20.inc): static ONLY_PROJECT_POSITION (4, always 0 on
// PC), COLOR_DEPTH (4); dynamic COMPRESSED_VERTS (1), SKINNING (2), which the
// vertex record already applied with the flex deltas.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 texCoord;
layout( location = 1 ) out vec4 vWorldPos_projPosZ;

void main()
{
	const bool COLOR_DEPTH = STATIC_VS_COMBO( 4, 2 ) != 0;

	vec4 vProjPos = LegacyProject( inWorldPos );
	gl_Position = vProjPos;

	texCoord = inTexCoord0;

	vWorldPos_projPosZ = vec4( 0.0 );
	if ( COLOR_DEPTH )
	{
		vWorldPos_projPosZ.z = vProjPos.z;
		vWorldPos_projPosZ.w = vProjPos.w;
	}
}
