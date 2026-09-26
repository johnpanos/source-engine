#version 450
// A port of stdshaders/Refract_vs20.fxc. Combos (fxctmp9/Refract_vs20.inc):
// static MODEL (4), COLORMODULATE (8); dynamic COMPRESSED_VERTS (1) and
// SKINNING (2), which the shader API's vertex record already applied (a
// skinned model's fog and eye vector use its skinned position, where the HLSL
// uses mul( v.vPos, cModel[0] )).
#include "legacy_vs.glsl"

layout( location = 0 ) out vec4 vBumpTexCoord;
layout( location = 1 ) out vec3 vTangentEyeVect;
layout( location = 2 ) out vec3 vWorldNormal;
layout( location = 3 ) out vec3 vWorldTangent;
layout( location = 4 ) out vec3 vWorldBinormal;
layout( location = 5 ) out vec3 vRefractXYW;
layout( location = 6 ) out vec3 vWorldViewVector;
layout( location = 7 ) out vec4 worldPos_projPosZ;
layout( location = 8 ) out vec4 vColor;
layout( location = 9 ) out vec4 fogFactorW;

#define cBumpTexCoordTransform_0 VS_C( 49 )
#define cBumpTexCoordTransform_1 VS_C( 50 )
#define cBumpTexCoordTransform_2 VS_C( 51 )
#define cBumpTexCoordTransform_3 VS_C( 52 )

void main()
{
	const bool MODEL = STATIC_VS_COMBO( 4, 2 ) != 0;
	const bool COLORMODULATE = STATIC_VS_COMBO( 8, 2 ) != 0;

	vColor = COLORMODULATE ? vec4( inColor, inAlpha ) : vec4( 0.0 );

	const vec3 worldPos = inWorldPos;
	const vec3 worldNormal = inWorldNormal;
	const vec3 worldTangentS = inWorldTangentS.xyz;
	// SkinPositionNormalAndTangentSpace: T from the normal, S and the TANGENT w
	// for a model; the brush's TANGENTT stream otherwise.
	const vec3 worldTangentT =
	    MODEL ? cross( worldNormal, worldTangentS ) * LegacyTangentSign() : LegacyTangentT();

	// Projected position
	vec4 vProjPos = LegacyProject( worldPos );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( worldPos, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	worldPos_projPosZ = vec4( worldPos, vProjPos.z );

	// Map projected position to the refraction texture
	vec2 vRefractPos = vec2( vProjPos.x, -vProjPos.y );
	vRefractPos = ( vRefractPos + vProjPos.w ) * 0.5;
	vRefractXYW = vec3( vRefractPos.x, vRefractPos.y, vProjPos.w );

	fogFactorW = vec4( RangeFog( vProjPos.xyz ) );

	// Eye vector
	const vec3 vWorldEyeVect = normalize( cEyePos - worldPos );
	vWorldViewVector = -vWorldEyeVect;

	// Transform to the tangent space
	vTangentEyeVect = vec3( dot( vWorldEyeVect, worldTangentS ),
	    dot( vWorldEyeVect, worldTangentT ), dot( vWorldEyeVect, worldNormal ) );

	// Bump coordinates (note wz, not zw)
	const vec4 uv = vec4( inTexCoord0, 0.0, 1.0 );
	vBumpTexCoord.x = dot( uv, cBumpTexCoordTransform_0 );
	vBumpTexCoord.y = dot( uv, cBumpTexCoordTransform_1 );
	vBumpTexCoord.w = dot( uv, cBumpTexCoordTransform_2 );
	vBumpTexCoord.z = dot( uv, cBumpTexCoordTransform_3 );

	vWorldNormal = normalize( worldNormal );
	vWorldTangent = worldTangentS;
	vWorldBinormal = worldTangentT;
}
