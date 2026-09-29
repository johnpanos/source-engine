#version 450
// A port of stdshaders/black_vs20.fxc (Black's vertex stage). Combos
// (fxctmp9/black_vs20.inc): dynamic COMPRESSED_VERTS (1), DOWATERFOG (2),
// SKINNING (4), which the shader API's vertex record already applied. The FOG
// output only serves ps20's hardware fog; the ps20b stage fogs per pixel.
#include "legacy_vs.glsl"

layout( location = 7 ) out vec4 worldPos_projPosZ;

void main()
{
	const vec3 worldPos = inWorldPos;
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );
}
