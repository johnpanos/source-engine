#version 450
// A port of stdshaders/aftershock_vs20.fxc (Aftershock_dx9's vertex stage).
// Combos (fxctmp9/aftershock_vs20.inc): dynamic COMPRESSED_VERTS (1), SKINNING
// (2), which the vertex record already applied.
#include "legacy_vs.glsl"

layout( location = 0 ) out vec3 vWorldNormalOut;
layout( location = 1 ) out vec3 vWorldTangentOut;
layout( location = 2 ) out vec3 vWorldBinormalOut;
layout( location = 3 ) out vec3 vProjPosForRefract;
layout( location = 4 ) out vec3 vWorldViewVectorOut;
layout( location = 5 ) out vec4 vUv0Out;
layout( location = 6 ) out vec4 vUv1Out;
layout( location = 7 ) out vec2 vUvGroundNoise;

#define g_flTime ( VS_C( 48 ).x )
#define cBaseTexCoordTransform_0 VS_C( 49 )
#define cBaseTexCoordTransform_1 VS_C( 50 )

void main()
{
	// SkinPositionNormalAndTangentSpace: T = cross( N, S ) * TANGENT.w.
	vec3 vWorldPosition = inWorldPos;
	vec3 vWorldNormal = inWorldNormal;
	vec3 vWorldTangent = inWorldTangentS.xyz;
	vec3 vWorldBinormal = cross( vWorldNormal, vWorldTangent ) * LegacyTangentSign();
	vWorldNormal.xyz = normalize( vWorldNormal.xyz );
	vWorldTangent.xyz = normalize( vWorldTangent.xyz );
	vWorldBinormal.xyz = normalize( vWorldBinormal.xyz );

	vWorldNormalOut = vWorldNormal;
	vWorldTangentOut = vWorldTangent;
	vWorldBinormalOut = vWorldBinormal;

	// Transform into projection space
	vec4 vProjPosition = LegacyProject( vWorldPosition );
	gl_Position = vProjPosition;

	// Map projected position to the refraction texture
	vec2 vRefractPos;
	vRefractPos.x = vProjPosition.x;
	vRefractPos.y = -vProjPosition.y; // Invert Y
	vRefractPos = ( vRefractPos + vProjPosition.w ) * 0.5;
	vProjPosForRefract = vec3( vRefractPos.x, vRefractPos.y, vProjPosition.w );

	// View vector
	vWorldViewVectorOut = normalize( vWorldPosition.xyz - cEyePos.xyz );

	// Texture coordinates: dot( float2 uv, float4 row ) uses the row's xy.
	vec2 vBaseUv;
	vBaseUv.x = dot( inTexCoord0.xy, cBaseTexCoordTransform_0.xy );
	vBaseUv.y = dot( inTexCoord0.xy, cBaseTexCoordTransform_1.xy );

	// Bump layer 0
	vec2 vUv0 = vBaseUv.xy;
	vec2 vUv0Scroll = vBaseUv.xy * 3.0;
	vUv0Scroll.y -= g_flTime * 0.1;
	vUv0Out.xy = vUv0.xy;
	vUv0Out.wz = vUv0Scroll.xy;

	// Bump layer 1
	vec2 vUv1 = vBaseUv.xy * 8.0;
	vec2 vUv1Scroll = vBaseUv.xy * 16.0;
	vUv1Scroll.y -= g_flTime * 0.1;
	vUv1Out.xy = vUv1.xy;
	vUv1Out.wz = vUv1Scroll.xy;

	// Ground noise
	vUvGroundNoise.xy = vBaseUv.xy;
	vUvGroundNoise.x *= 3.5;
	vUvGroundNoise.y *= 0.2105;
	vUvGroundNoise.y -= g_flTime * 0.04;
}
