#version 450
// A port of stdshaders/eyes_flashlight_vs20.fxc (Eyes_dx9's flashlight pass).
// Combos (fxctmp9/eyes_flashlight_vs20.inc): dynamic COMPRESSED_VERTS (1),
// SKINNING (2), DOWATERFOG (4). The vertex record already applied the flex
// deltas, the skinning and the vertex decompression; the fixed-function fog
// output is unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_flashlight_vs.glsl"

layout( location = 0 ) out vec4 spotTexCoord;
layout( location = 1 ) out vec2 baseTexCoord;
layout( location = 3 ) out vec2 irisTexCoord;
layout( location = 4 ) out vec3 vertAtten;
layout( location = 5 ) out vec3 worldPosOut;
layout( location = 7 ) out vec3 projPosXYZ;

#define cLightPosition VS_C( 48 )
#define cSpotlightProj1 VS_C( 49 )
#define cSpotlightProj2 VS_C( 50 )
#define cSpotlightProj3 VS_C( 51 )
#define cSpotlightProj4 VS_C( 52 )
#define cFlashlighAtten VS_C( 53 ) // const, linear, quadratic & farZ
#define cIrisProjectionU VS_C( 56 )
#define cIrisProjectionV VS_C( 57 )

void main()
{
	vec3 worldPos = inWorldPos;
	vec3 worldNormal = normalize( inWorldNormal );

	// Transform into projection space
	vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;
	projPosXYZ = projPos.xyz;
	worldPosOut = worldPos.xyz;

	// Base texture coordinates
	baseTexCoord = inTexCoord0;

	// Spotlight texture coordinates
	spotTexCoord.x = dot( cSpotlightProj1, vec4( worldPos, 1.0 ) );
	spotTexCoord.y = dot( cSpotlightProj2, vec4( worldPos, 1.0 ) );
	spotTexCoord.z = dot( cSpotlightProj3, vec4( worldPos, 1.0 ) );
	spotTexCoord.w = dot( cSpotlightProj4, vec4( worldPos, 1.0 ) );

	// Compute vector to light
	vec3 vWorldPosToLightVector = cLightPosition.xyz - worldPos;

	vec3 vDistAtten = vec3( 1.0 );
	vDistAtten.z = dot( vWorldPosToLightVector, vWorldPosToLightVector ); // distsquared
	vDistAtten.y = inversesqrt( vDistAtten.z );                           // 1 / dist

	float flDist = vDistAtten.z * vDistAtten.y; // dist
	vDistAtten.z = 1.0 / vDistAtten.z;          // 1 / distsquared

	float fFarZ = cFlashlighAtten.w;

	float endFalloffFactor = RemapValClamped_01( flDist, fFarZ, 0.6 * fFarZ );
	vertAtten.xyz = vec3( endFalloffFactor * dot( vDistAtten, cFlashlighAtten.xyz ) );

	vertAtten *= dot( normalize( vWorldPosToLightVector ), worldNormal );

	irisTexCoord.x = dot( cIrisProjectionU, vec4( worldPos, 1.0 ) );
	irisTexCoord.y = dot( cIrisProjectionV, vec4( worldPos, 1.0 ) );
}
