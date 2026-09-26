#version 450
// A port of stdshaders/core_vs20.fxc (Core_DX90's vertex stage). Combos
// (fxctmp9/core_vs20.inc): static MODEL (4); dynamic COMPRESSED_VERTS (1),
// SKINNING (2). The vertex record already applied the skinning and the vertex
// decompression: a brush's tangent S and T streams (MODEL 0) or a model's
// TANGENT with its sign (MODEL 1). The eye vector and fog read
// mul( v.vPos, cModel[ 0 ] ), which is the record's world position unless
// skinned (a skinned draw uses the skinned position here). The fixed-function
// fog output is unused (pixel fog).
#include "legacy_vs.glsl"

layout( location = 0 ) out vec2 vBumpTexCoord;
layout( location = 1 ) out vec3 vTangentEyeVect;
// float3x3 tangentSpaceTranspose : TEXCOORD2..4, written by column: register n
// holds ( S[n], T[n], N[n] ).
layout( location = 2 ) out vec3 tangentSpaceTranspose0;
layout( location = 3 ) out vec3 tangentSpaceTranspose1;
layout( location = 4 ) out vec3 tangentSpaceTranspose2;
layout( location = 5 ) out vec3 vRefractXYW;
layout( location = 6 ) out vec4 projNormal_screenCoordW;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 9 ) out vec4 fogFactorW; // COLOR1

#define cBumpTexCoordTransform_0 VS_C( 49 )
#define cBumpTexCoordTransform_1 VS_C( 50 )

void main()
{
	const bool g_bModel = STATIC_VS_COMBO( 4, 2 ) != 0;

	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	vec3 worldTangentS = inWorldTangentS.xyz;
	// SkinPositionNormalAndTangentSpace's T = cross( N, S ) * TANGENT.w for a
	// model; a brush's TANGENTT stream.
	vec3 worldTangentT = g_bModel ? cross( worldNormal, worldTangentS ) * LegacyTangentSign() : inExtra.xyz;

	// Projected position
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	// mul( worldNormal, cViewProj ): the float3 against the first three rows.
	projNormal_screenCoordW.x = dot( worldNormal, VS_C( 8 ).xyz );
	projNormal_screenCoordW.y = dot( worldNormal, VS_C( 9 ).xyz );
	projNormal_screenCoordW.z = dot( worldNormal, VS_C( 10 ).xyz );
	projNormal_screenCoordW.w = 0.0;

	// The clip-space z of the D3D9 projection (cViewProj's third column).
	float projPosZ = dot( vec4( worldPos, 1.0 ), VS_C( 10 ) );
	worldPos_projPosZ = vec4( worldPos.xyz, projPosZ );

	// Map projected position to the refraction texture
	vec2 vRefractPos;
	vRefractPos.x = vProjPos.x;
	vRefractPos.y = -vProjPos.y; // invert Y
	vRefractPos = ( vRefractPos + vProjPos.w ) * 0.5;

	// Refraction transform
	vRefractXYW = vec3( vRefractPos.x, vRefractPos.y, vProjPos.w );

	// Compute fog based on the position
	vec3 vWorldPos = worldPos;
	fogFactorW = vec4( RangeFog( vec3( vProjPos.xy, projPosZ ) ) );

	// Eye vector
	vec3 vWorldEyeVect = cEyePos - vWorldPos;
	// Transform to the tangent space
	vTangentEyeVect.x = dot( vWorldEyeVect, worldTangentS );
	vTangentEyeVect.y = dot( vWorldEyeVect, worldTangentT );
	vTangentEyeVect.z = dot( vWorldEyeVect, worldNormal );

	// Tranform bump coordinates
	vBumpTexCoord = DotTexTransform( inTexCoord0, cBumpTexCoordTransform_0, cBumpTexCoordTransform_1 );

	tangentSpaceTranspose0 = vec3( worldTangentS.x, worldTangentT.x, worldNormal.x );
	tangentSpaceTranspose1 = vec3( worldTangentS.y, worldTangentT.y, worldNormal.y );
	tangentSpaceTranspose2 = vec3( worldTangentS.z, worldTangentT.z, worldNormal.z );
}
