#version 450
// A port of stdshaders/eye_refract_vs20.fxc (EyeRefract_dx9's vertex stage).
// Combos (fxctmp9/eye_refract_vs20.inc): static INTRO (160), HALFLAMBERT (320),
// FLASHLIGHT (640), LIGHTWARPTEXTURE (1280); dynamic COMPRESSED_VERTS (1),
// SKINNING (2), DOWATERFOG (4), DYNAMIC_LIGHT (8), STATIC_LIGHT (16),
// NUM_LIGHTS (32, 0..4). The vertex record already applied the flex deltas,
// the skinning and the vertex decompression; the fixed-function fog output is
// unused (pixel fog).
#include "legacy_vs.glsl"
#include "legacy_vs_lighting.glsl"
#include "legacy_vortwarp_vs.glsl"

layout( location = 0 ) out vec4 vAmbientOcclUv_fallbackCorneaUv;
layout( location = 1 ) out vec4 cVertexLight;
layout( location = 2 ) out vec4 vTangentViewVector;
layout( location = 3 ) out vec4 vWorldPosition_ProjPosZ;
layout( location = 4 ) out vec3 vWorldNormalOut;
layout( location = 5 ) out vec3 vWorldTangentOut;
layout( location = 6 ) out vec4 vLightFalloffCosine01;
layout( location = 7 ) out vec4 vLightFalloffCosine23;
layout( location = 8 ) out vec3 vWorldBinormalOut; // COLOR0

#define g_cEyeOrigin ( VS_C( 48 ).xyz )
#define g_vIrisProjectionU VS_C( 50 )
#define g_vIrisProjectionV VS_C( 51 )
#define g_vFlashlightPosition VS_C( 52 )
#define g_vConst4 VS_C( 53 )
#define g_vModelOrigin ( g_vConst4.xyz )
#define g_flTime ( g_vConst4.w )
#define g_vFlashlightMatrixRow1 VS_C( 54 )
#define g_vFlashlightMatrixRow2 VS_C( 55 )
#define g_vFlashlightMatrixRow3 VS_C( 56 )
#define g_vFlashlightMatrixRow4 VS_C( 57 )

// common_fxc.h's Vec3WorldToTangent( Normalized ).
vec3 Vec3WorldToTangent( vec3 iWorldVector, vec3 iWorldNormal, vec3 iWorldTangent, vec3 iWorldBinormal )
{
	vec3 vTangentVector;
	vTangentVector.x = dot( iWorldVector.xyz, iWorldTangent.xyz );
	vTangentVector.y = dot( iWorldVector.xyz, iWorldBinormal.xyz );
	vTangentVector.z = dot( iWorldVector.xyz, iWorldNormal.xyz );
	return vTangentVector.xyz;
}
vec3 Vec3WorldToTangentNormalized(
    vec3 iWorldVector, vec3 iWorldNormal, vec3 iWorldTangent, vec3 iWorldBinormal )
{
	return normalize( Vec3WorldToTangent( iWorldVector, iWorldNormal, iWorldTangent, iWorldBinormal ) );
}

void main()
{
	const bool INTRO = STATIC_VS_COMBO( 160, 2 ) != 0;
	const bool g_bHalfLambert = STATIC_VS_COMBO( 320, 2 ) != 0;
	const bool bFlashlight = STATIC_VS_COMBO( 640, 2 ) != 0;
	const bool bDoDiffuseWarp = STATIC_VS_COMBO( 1280, 2 ) != 0;
	const bool bDynamicLight = DYNAMIC_VS_COMBO( 8, 2 ) != 0;
	const bool bStaticLight = DYNAMIC_VS_COMBO( 16, 2 ) != 0;
	const int nNumLights = DYNAMIC_VS_COMBO( 32, 5 );

	vec3 vWorldPosition = inWorldPos;

	// Note: I'm relying on the iris projection vector math not changing or this will break
	vec3 vEyeSocketUpVector = normalize( -g_vIrisProjectionV.xyz );
	vec3 vEyeSocketLeftVector = normalize( -g_vIrisProjectionU.xyz );

	if ( INTRO )
	{
		vec3 dummy = vec3( 0.0 );
		WorldSpaceVertexProcess( g_flTime, g_vModelOrigin, vWorldPosition, dummy, dummy, dummy );
	}

	vWorldPosition_ProjPosZ.xyz = vWorldPosition.xyz;

	// Transform into projection space
	vec4 vProjPos = LegacyProject( vWorldPosition );
	gl_Position = vProjPos;
	vProjPos.z = dot( vec4( vWorldPosition, 1.0 ), VS_C( 13 ) ); // cViewProjZ
	vWorldPosition_ProjPosZ.w = vProjPos.z;

	// Normal = (Pos - Eye origin)
	vec3 vWorldNormal = normalize( vWorldPosition.xyz - g_cEyeOrigin.xyz );
	vWorldNormalOut = vWorldNormal;

	vec3 vWorldTangent = normalize( cross( vEyeSocketUpVector.xyz, vWorldNormal.xyz ) );
	vWorldTangentOut = vWorldTangent;

	vec3 vWorldBinormal = normalize( cross( vWorldNormal.xyz, vWorldTangent.xyz ) );
	vWorldBinormalOut = vWorldBinormal.xyz * 0.5 + 0.5;

	vec3 vWorldViewVector = normalize( vWorldPosition.xyz - cEyePos.xyz );
	vTangentViewVector.xyz = Vec3WorldToTangentNormalized(
	    vWorldViewVector.xyz, vWorldNormal.xyz, vWorldTangent.xyz, vWorldBinormal.xyz );

	float vNormalDotSideVec = -dot( vWorldNormal, vEyeSocketLeftVector ) * 0.5;
	vec3 vBentWorldNormal = normalize( vNormalDotSideVec * vEyeSocketLeftVector + vWorldNormal );

	// Compute vertex lighting
	cVertexLight.a = 0.0; // Only used for flashlight pass
	cVertexLight.rgb = DoLightingUnrolled( vWorldPosition, vBentWorldNormal, vec3( 0.0 ),
	    bStaticLight, bDynamicLight, g_bHalfLambert, nNumLights );

	// Only interpolate ambient light for TF NPR lighting
	if ( bDoDiffuseWarp )
	{
		if ( bDynamicLight )
			cVertexLight.rgb = AmbientLight( vBentWorldNormal.xyz );
		else
			cVertexLight.rgb = vec3( 0.0 );
	}

	// Light falloff and cosine terms for the four local lights (filled in
	// whatever the light count; the pixel shader reads the ones it has).
	vLightFalloffCosine01.x = VertexAttenInternal( vWorldPosition.xyz, 0 );
	vLightFalloffCosine01.y = VertexAttenInternal( vWorldPosition.xyz, 1 );
	vLightFalloffCosine01.z = CosineTermInternal( vWorldPosition.xyz, vWorldNormal.xyz, 0, g_bHalfLambert );
	vLightFalloffCosine01.w = CosineTermInternal( vWorldPosition.xyz, vWorldNormal.xyz, 1, g_bHalfLambert );

	vLightFalloffCosine23.x = VertexAttenInternal( vWorldPosition.xyz, 2 );
	vLightFalloffCosine23.y = VertexAttenInternal( vWorldPosition.xyz, 3 );
	vLightFalloffCosine23.z = CosineTermInternal( vWorldPosition.xyz, vWorldNormal.xyz, 2, g_bHalfLambert );
	vLightFalloffCosine23.w = CosineTermInternal( vWorldPosition.xyz, vWorldNormal.xyz, 3, g_bHalfLambert );

	// Texture coordinates set by artists for ambient occlusion
	vAmbientOcclUv_fallbackCorneaUv.xy = inTexCoord0.xy;

	// Cornea uv for ps.2.0 fallback
	vec2 vCorneaUv;
	vCorneaUv.x = dot( g_vIrisProjectionU, vec4( vWorldPosition, 1.0 ) );
	vCorneaUv.y = dot( g_vIrisProjectionV, vec4( vWorldPosition, 1.0 ) );
	vAmbientOcclUv_fallbackCorneaUv.wz = vCorneaUv.xy;

	// Step on the vertex light interpolator for the flashlight tex coords
	vTangentViewVector.w = 0.0;
	if ( bFlashlight )
	{
		cVertexLight.x = dot( g_vFlashlightMatrixRow1.xyzw, vec4( vWorldPosition, 1.0 ) );
		cVertexLight.y = dot( g_vFlashlightMatrixRow2.xyzw, vec4( vWorldPosition, 1.0 ) );
		cVertexLight.z = dot( g_vFlashlightMatrixRow3.xyzw, vec4( vWorldPosition, 1.0 ) );
		cVertexLight.w = dot( g_vFlashlightMatrixRow4.xyzw, vec4( vWorldPosition, 1.0 ) );

		// Flashlight N.L with modified normal
		vTangentViewVector.w = saturate( dot( vBentWorldNormal.xyz,
		    normalize( g_vFlashlightPosition.xyz - vWorldPosition.xyz ) ) );
	}
}
