#version 450
// A port of stdshaders/teeth_flashlight_vs20.fxc (Teeth_DX9's flashlight
// pass). Combos (fxctmp9/teeth_flashlight_vs20.inc): static INTRO (8); dynamic
// COMPRESSED_VERTS (1), DOWATERFOG (2), SKINNING (4). The vertex record
// already applied the skinning and the vertex decompression; its world normal
// is the model normal rotated, the HLSL's normalize( normal ) rotated for the
// unit-length normals models carry. The fixed-function fog output is unused.
#include "legacy_vs.glsl"
#include "legacy_flashlight_vs.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec2 baseTexCoord;
layout( location = 1 ) out vec4 spotTexCoord;
layout( location = 2 ) out vec3 vertAtten;
layout( location = 3 ) out vec4 vProjPos;
layout( location = 4 ) out vec3 worldPosOut;

#define cFlashlightPosition VS_C( 48 )
#define cSpotlightProj1 VS_C( 49 )
#define cSpotlightProj2 VS_C( 50 )
#define cSpotlightProj3 VS_C( 51 )
#define cSpotlightProj4 VS_C( 52 )
#define cFlashlighAtten VS_C( 53 ) // const, linear, quadratic & farZ
#define cTeethLighting VS_C( 56 )
#define const4 VS_C( 57 )
#define g_Time ( const4.w )
#define modelOrigin ( const4.xyz )

void main()
{
	const bool INTRO = STATIC_VS_COMBO( 8, 2 ) != 0;

	vec3 worldPos = inWorldPos;
	vec3 worldNormal = inWorldNormal;
	if ( INTRO )
	{
		vec3 dummy = vec3( 0.0 );
		WorldSpaceVertexProcess( g_Time, modelOrigin, worldPos, worldNormal, dummy, dummy );
	}

	// Transform into projection space
	vec4 projPos = LegacyProject( worldPos );
	gl_Position = projPos;
	worldPosOut = worldPos.xyz;
	vProjPos = projPos;

	// Spotlight texture coordinates
	spotTexCoord.x = dot( cSpotlightProj1, vec4( worldPos, 1.0 ) );
	spotTexCoord.y = dot( cSpotlightProj2, vec4( worldPos, 1.0 ) );
	spotTexCoord.z = dot( cSpotlightProj3, vec4( worldPos, 1.0 ) );
	spotTexCoord.w = dot( cSpotlightProj4, vec4( worldPos, 1.0 ) );

	// Compute vector to light
	vec3 vWorldPosToLightVector = cFlashlightPosition.xyz - worldPos;

	vec3 vDistAtten = vec3( 1.0 );
	vDistAtten.z = dot( vWorldPosToLightVector, vWorldPosToLightVector );
	vDistAtten.y = inversesqrt( vDistAtten.z );

	float flDist = vDistAtten.z * vDistAtten.y; // Distance to light
	vDistAtten.z = 1.0 / vDistAtten.z;          // 1 / distsquared

	float fFarZ = cFlashlighAtten.w;

	float NdotL = saturate( dot( worldNormal, normalize( vWorldPosToLightVector ) ) );

	float endFalloffFactor = RemapValClamped_01( flDist, fFarZ, 0.6 * fFarZ );

	// Final attenuation from flashlight only...
	float linearAtten = NdotL * dot( vDistAtten, cFlashlighAtten.xyz ) * endFalloffFactor;

	// Forward vector
	vec3 vForward = cTeethLighting.xyz;
	float fIllumFactor = cTeethLighting.w;

	// Modulate flashlight by mouth darkening
	vertAtten = vec3( linearAtten * fIllumFactor * saturate( dot( worldNormal, vForward ) ) );

	baseTexCoord = inTexCoord0;
}
