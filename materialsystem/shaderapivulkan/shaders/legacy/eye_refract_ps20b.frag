#version 450
// EyeRefract_dx9's pixel stage: a port of stdshaders/eye_refract_ps2x.fxc
// (ps20b). Combos (fxctmp9/eye_refract_ps20b.inc): static FLASHLIGHT (10),
// LIGHTWARPTEXTURE (20), SPHERETEXKILLCOMBO (40), RAYTRACESPHERE (80),
// FLASHLIGHTDEPTHFILTERMODE (160); dynamic NUM_LIGHTS (1, 0..4),
// FLASHLIGHTSHADOWS (5). FLASHLIGHTSHADOWS is never selected here (the
// backend has no shadow depth textures, so the flashlight's depth texture is
// never bound) and FLASHLIGHTDEPTHFILTERMODE only filters those shadows.
// @legacy program=eye_refract ps=eye_refract_ps20b vs=eye_refract_vs20 vert=eye_refract_vs20
//         samplers=0:2d,1:2d,2:cube,3:2d,4:2d,5:2d
#include "legacy_ps.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tCorneaSampler;              // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tIrisSampler;                // s1
layout( set = 0, binding = 2 ) uniform samplerCube g_tEyeReflectionCubemapSampler; // s2
layout( set = 0, binding = 3 ) uniform sampler2D g_tEyeAmbientOcclSampler;      // s3
layout( set = 0, binding = 4 ) uniform sampler2D g_tLightwarpSampler;           // s4
layout( set = 0, binding = 5 ) uniform sampler2D g_tFlashlightCookieSampler;    // s5

layout( location = 0 ) in vec4 vAmbientOcclUv_fallbackCorneaUv;
layout( location = 1 ) in vec4 cVertexLightIn; // w is used for the flashlight pass
layout( location = 2 ) in vec4 vTangentViewVectorIn;
layout( location = 3 ) in vec4 vWorldPosition_ProjPosZ;
layout( location = 4 ) in vec3 vWorldNormalIn;
layout( location = 5 ) in vec3 vWorldTangentIn;
layout( location = 6 ) in vec4 vLightFalloffCosine01;
layout( location = 7 ) in vec4 vLightFalloffCosine23;
layout( location = 8 ) in vec3 vWorldBinormalIn; // COLOR0

#define g_vPackedConst0 PS_C( 0 )
#define g_flDilationFactor ( g_vPackedConst0.x )
#define g_flGlossiness ( g_vPackedConst0.y )
#define g_flAverageAmbient ( g_vPackedConst0.z )
#define g_flCorneaBumpStrength ( g_vPackedConst0.w )
#define g_vEyeOrigin ( PS_C( 1 ).xyz )
#define g_vIrisProjectionU PS_C( 2 )
#define g_vIrisProjectionV PS_C( 3 )
#define g_vCameraPosition PS_C( 4 )
#define g_cAmbientOcclColor ( PS_C( 5 ).xyz )
#define g_vPackedConst6 PS_C( 6 )
#define g_flEyeballRadius ( g_vPackedConst6.y )
#define g_flParallaxStrength ( g_vPackedConst6.w )
#define g_vFlashlightAttenuationFactors PS_C( 7 )
#define g_vFlashlightPos ( PS_C( 8 ).xyz )
#define g_ShaderControls PS_C( 10 )
#define g_fPixelFogType ( g_ShaderControls.x )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )

// Ray sphere intersect returns distance along ray to intersection
float IntersectRaySphere( vec3 cameraPos, vec3 ray, vec3 sphereCenter, float sphereRadius )
{
	vec3 dst = cameraPos.xyz - sphereCenter.xyz;
	float B = dot( dst, ray );
	float C = dot( dst, dst ) - ( sphereRadius * sphereRadius );
	float D = B * B - C;
	return ( D > 0.0 ) ? ( -B - sqrt( D ) ) : 0.0;
}

// Calculate both types of Fog and lerp to get result
float CalcPixelFogFactorConst(
    float fPixelFogType, vec4 fogParams, float flEyePosZ, float flWorldPosZ, float flProjPosZ )
{
	float fRangeFog = CalcRangeFog( flProjPosZ, fogParams.x, fogParams.z, fogParams.w );
	float fHeightFog = CalcWaterFogAlpha( fogParams.y, flEyePosZ, flWorldPosZ, flProjPosZ, fogParams.w );
	return mix( fRangeFog, fHeightFog, fPixelFogType );
}

// Blend both types of Fog and lerp to get result
vec3 BlendPixelFogConst( vec3 vShaderColor, float pixelFogFactor, vec3 vFogColor, float fPixelFogType )
{
	pixelFogFactor = saturate( pixelFogFactor );
	vec3 fRangeResult = mix( vShaderColor.rgb, vFogColor.rgb, pixelFogFactor * pixelFogFactor );
	vec3 fHeightResult = mix( vShaderColor.rgb, vFogColor.rgb, saturate( pixelFogFactor ) );
	return mix( fRangeResult, fHeightResult, fPixelFogType );
}

vec4 FinalOutputConst( vec4 vShaderColor, float pixelFogFactor, float fPixelFogType, int iTONEMAP_SCALE_TYPE )
{
	vec4 result = vShaderColor;
	if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_LINEAR )
		result.rgb *= LINEAR_LIGHT_SCALE;
	else if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_GAMMA )
		result.rgb *= GAMMA_LIGHT_SCALE;
	result.rgb = BlendPixelFogConst( result.rgb, pixelFogFactor, g_LinearFogColor.rgb, fPixelFogType );
	return result;
}

void main()
{
	const bool bFlashlight = STATIC_PS_COMBO( 10, 2 ) != 0;
	const bool bDoDiffuseWarp = STATIC_PS_COMBO( 20, 2 ) != 0;
	const bool bRayCastTexKill = STATIC_PS_COMBO( 40, 2 ) != 0;
	const bool bRayCast = STATIC_PS_COMBO( 80, 2 ) != 0;
	// Flashlight is considered one light, otherwise, use numlights combo
	const int nNumLights = bFlashlight ? 1 : DYNAMIC_PS_COMBO( 1, 5 );

	vec4 cVertexLight = cVertexLightIn;
	float flFlashlightNDotL = vTangentViewVectorIn.w;
	vec4 vFlashlightTexCoord = vec4( 0.0 );
	if ( bFlashlight )
	{
		vFlashlightTexCoord.xyzw = cVertexLight.xyzw; // This was hidden in this interpolator
		cVertexLight.rgba = vec4( 0.0 );
	}

	// Interpolated vectors
	vec3 vWorldNormal = vWorldNormalIn.xyz;
	vec3 vWorldTangent = vWorldTangentIn.xyz;
	vec3 vWorldBinormal = ( vWorldBinormalIn.xyz * 2.0 ) - 1.0;

	vec3 vTangentViewVector = vTangentViewVectorIn.xyz;

	// World position
	vec3 vWorldPosition = vWorldPosition_ProjPosZ.xyz;

	// World view vector to pixel
	vec3 vWorldViewVector = normalize( vWorldPosition.xyz - g_vCameraPosition.xyz );

	// TF NPR lighting
	if ( bDoDiffuseWarp && !bFlashlight )
	{
		// Replace the interpolated vertex light
		if ( nNumLights > 0 )
		{
			vec3 cWarpedLight = 2.0 * tex1D( 4, g_tLightwarpSampler, vLightFalloffCosine01.z ).rgb;
			cVertexLight.rgb += vLightFalloffCosine01.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 0 ) * cWarpedLight.rgb;
		}
		if ( nNumLights > 1 )
		{
			vec3 cWarpedLight = 2.0 * tex1D( 4, g_tLightwarpSampler, vLightFalloffCosine01.w ).rgb;
			cVertexLight.rgb += vLightFalloffCosine01.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 1 ) * cWarpedLight.rgb;
		}
		if ( nNumLights > 2 )
		{
			vec3 cWarpedLight = 2.0 * tex1D( 4, g_tLightwarpSampler, vLightFalloffCosine23.z ).rgb;
			cVertexLight.rgb += vLightFalloffCosine23.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 2 ) * cWarpedLight.rgb;
		}
		if ( nNumLights > 3 )
		{
			vec3 cWarpedLight = 2.0 * tex1D( 4, g_tLightwarpSampler, vLightFalloffCosine23.w ).rgb;
			cVertexLight.rgb += vLightFalloffCosine23.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 3 ) * cWarpedLight.rgb;
		}
	}

	// Ray cast against sphere representing eyeball to reduce artifacts from
	// non-spherical morphed eye geometry
	if ( bRayCast )
	{
		float fSphereRayCastDistance = IntersectRaySphere(
		    g_vCameraPosition.xyz, vWorldViewVector.xyz, g_vEyeOrigin.xyz, g_flEyeballRadius );
		vWorldPosition.xyz = g_vCameraPosition.xyz + ( vWorldViewVector.xyz * fSphereRayCastDistance );
		if ( fSphereRayCastDistance == 0.0 )
		{
			if ( bRayCastTexKill )
				discard; // texkill to get a better silhouette
			vWorldPosition.xyz = g_vEyeOrigin.xyz + ( vWorldNormal.xyz * g_flEyeballRadius );
		}
	}

	// Generate sphere and cornea uv's
	vec2 vCorneaUv; // Note: Cornea texture is a cropped version of the iris texture
	vCorneaUv.x = dot( g_vIrisProjectionU, vec4( vWorldPosition, 1.0 ) );
	vCorneaUv.y = dot( g_vIrisProjectionV, vec4( vWorldPosition, 1.0 ) );
	vec2 vSphereUv = ( vCorneaUv.xy * 0.5 ) + 0.25;

	// Hacked parallax mapping on iris
	float fIrisOffset = tex2D( 0, g_tCorneaSampler, vCorneaUv.xy ).b;

	vec2 vParallaxVector =
	    ( ( vTangentViewVector.xy * fIrisOffset * g_flParallaxStrength ) / ( 1.0 - vTangentViewVector.z ) );
	vParallaxVector.x = -vParallaxVector.x; // Need to flip x...not sure why.

	vec2 vIrisUv = vSphereUv.xy - vParallaxVector.xy;

	// Note: We fetch from this texture twice right now with different uv's for the color and alpha
	vec2 vCorneaNoiseUv = vSphereUv.xy + ( vParallaxVector.xy * 0.5 );
	float fCorneaNoise = tex2D( 1, g_tIrisSampler, vCorneaNoiseUv.xy ).a;

	// Cornea normal
	vec3 vCorneaTangentNormal = vec3( 0.0, 0.0, 1.0 );
	vec4 vCorneaSample = tex2D( 0, g_tCorneaSampler, vCorneaUv.xy );
	vCorneaTangentNormal.xy = vCorneaSample.rg - 0.5; // Note: This scales the bump to 50% strength

	// Scale strength of normal
	vCorneaTangentNormal.xy *= g_flCorneaBumpStrength;

	// Add in surface noise and imperfections (NOTE: This should be baked into the normal map!)
	vCorneaTangentNormal.xy += fCorneaNoise * 0.1;

	// Normalize tangent vector
	vCorneaTangentNormal.xyz = normalize( vCorneaTangentNormal.xyz );

	// Transform into world space
	vec3 vCorneaWorldNormal = Vec3TangentToWorldNormalized(
	    vCorneaTangentNormal.xyz, vWorldNormal.xyz, vWorldTangent.xyz, vWorldBinormal.xyz );

	// Flashlight
	vec3 vFlashlightVector = vec3( 0.0 );
	vec3 cFlashlightColorFalloff = vec3( 0.0 );
	if ( bFlashlight )
	{
		// Flashlight vector
		vFlashlightVector.xyz = normalize( g_vFlashlightPos.xyz - vWorldPosition_ProjPosZ.xyz );

		// Distance attenuation for flashlight and to fade out shadow over distance
		vec3 vDelta = g_vFlashlightPos.xyz - vWorldPosition_ProjPosZ.xyz;
		float flDistSquared = dot( vDelta, vDelta );
		float flDist = sqrt( flDistSquared );
		float flFlashlightAttenuation = dot( g_vFlashlightAttenuationFactors.xyz,
		    vec3( 1.0, 1.0 / flDist, 1.0 / flDistSquared ) );

		// Flashlight cookie
		vec3 vProjCoords = vFlashlightTexCoord.xyz / vFlashlightTexCoord.w;
		vec3 cFlashlightCookieColor = tex2D( 5, g_tFlashlightCookieSampler, vProjCoords.xy ).rgb;

		// Flashlight color intensity (needs to be multiplied by global flashlight color later)
		cFlashlightColorFalloff.rgb = flFlashlightAttenuation * cFlashlightCookieColor.rgb;

		// Add this into the interpolated lighting (no light warp for the flashlight)
		cVertexLight.rgb += cFlashlightColorFalloff.rgb * cFlashlightColor.rgb * flFlashlightNDotL;
	}

	// Dilate pupil
	vIrisUv.xy -= 0.5; // Center around (0,0)
	float fPupilCenterToBorder = saturate( length( vIrisUv.xy ) / 0.2 ); // Note: 0.2 is the uv radius of the iris
	float fPupilDilateFactor = g_flDilationFactor; // This value should be between 0-1
	vIrisUv.xy *= mix( 1.0, fPupilCenterToBorder, saturate( fPupilDilateFactor ) * 2.5 - 1.25 );
	vIrisUv.xy += 0.5;

	// Iris color
	vec4 cIrisColor = tex2D( 1, g_tIrisSampler, vIrisUv.xy );

	// Iris lighting highlights
	vec3 cIrisLighting = vec3( 0.0 );

	// Mask off everything but the iris pixels
	float fIrisHighlightMask = tex2D( 0, g_tCorneaSampler, vCorneaUv.xy ).a;

	// Generate the normal
	vec3 vIrisTangentNormal = vCorneaTangentNormal.xyz;
	vIrisTangentNormal.xy *= -2.5; // I'm not normalizing on purpose

	for ( int j = 0; j < nNumLights; j++ )
	{
		// World light vector
		vec3 vWorldLightVector;
		if ( ( j == 0 ) && bFlashlight )
			vWorldLightVector = vFlashlightVector.xyz;
		else
			vWorldLightVector = PixelShaderGetLightVector(
			    vWorldPosition_ProjPosZ.xyz, PSREG_LIGHT_INFO_ARRAY, j );

		// Tangent light vector
		vec3 vTangentLightVector = Vec3WorldToTangent(
		    vWorldLightVector.xyz, vWorldNormal.xyz, vWorldTangent.xyz, vWorldBinormal.xyz );

		// Adjust the tangent light vector to generate the iris lighting
		vec3 tmpv = -vTangentLightVector.xyz;
		tmpv.xy *= -0.5; // Flatten tangent view
		tmpv.z = max( tmpv.z, 0.5 ); // Clamp z of tangent view to help maintain highlight
		tmpv.xyz = normalize( tmpv.xyz );

		// Core iris lighting math
		float fIrisFacing = pow( abs( dot( vIrisTangentNormal.xyz, tmpv.xyz ) ), 6.0 ) * 0.5;

		// Cone of darkness to darken iris highlights when light falls behind eyeball past a certain point
		float flConeOfDarkness = pow( 1.0 - saturate( ( -vTangentLightVector.z - 0.25 ) / 0.75 ), 4.0 );

		// Tint by iris color and cone of darkness
		vec3 cIrisLightingTmp = vec3( fIrisFacing * fIrisHighlightMask * flConeOfDarkness );

		// Attenuate by light color and light falloff
		if ( ( j == 0 ) && bFlashlight )
			cIrisLightingTmp.rgb *= cFlashlightColorFalloff.rgb * cFlashlightColor.rgb;
		else if ( j == 0 )
			cIrisLightingTmp.rgb *= vLightFalloffCosine01.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 0 );
		else if ( j == 1 )
			cIrisLightingTmp.rgb *= vLightFalloffCosine01.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 1 );
		else if ( j == 2 )
			cIrisLightingTmp.rgb *= vLightFalloffCosine23.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 2 );
		else
			cIrisLightingTmp.rgb *= vLightFalloffCosine23.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 3 );

		// Sum into final variable
		cIrisLighting.rgb += cIrisLightingTmp.rgb;
	}

	// Add slight view dependent iris lighting based on ambient light intensity
	cIrisLighting.rgb += saturate( dot( vIrisTangentNormal.xyz, -vTangentViewVector.xyz ) ) *
	                     g_flAverageAmbient * fIrisHighlightMask * 0.5;

	// Ambient occlusion
	vec3 cAmbientOcclFromTexture = tex2D( 3, g_tEyeAmbientOcclSampler, vAmbientOcclUv_fallbackCorneaUv.xy ).rgb;
	vec3 cAmbientOcclColor = mix( g_cAmbientOcclColor, vec3( 1.0 ), cAmbientOcclFromTexture.rgb ); // Color the ambient occlusion
	cVertexLight.rgb *= cAmbientOcclColor.rgb;

	// Reflection from cube map
	vec3 vCorneaReflectionVector = reflect( vWorldViewVector.xyz, vCorneaWorldNormal.xyz );
	vec3 cReflection = g_flGlossiness * texCUBE( 2, g_tEyeReflectionCubemapSampler, vCorneaReflectionVector.xyz ).rgb;

	// Hack: Only add in half of the env map for the flashlight pass. This looks reasonable.
	if ( bFlashlight )
		cReflection.rgb *= 0.5;

	// Glint specular highlights
	vec3 cSpecularHighlights = vec3( 0.0 );
	if ( bFlashlight )
	{
		cSpecularHighlights.rgb += pow( saturate( dot( vCorneaReflectionVector.xyz, vFlashlightVector.xyz ) ), 128.0 ) *
		                           cFlashlightColorFalloff.rgb * cFlashlightColor.rgb;
	}
	else // no flashlight
	{
		const vec3 wp = vWorldPosition_ProjPosZ.xyz;
		if ( nNumLights > 0 )
			cSpecularHighlights.rgb += pow( saturate( dot( vCorneaReflectionVector.xyz, PixelShaderGetLightVector( wp, PSREG_LIGHT_INFO_ARRAY, 0 ) ) ), 128.0 ) *
			                           vLightFalloffCosine01.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 0 );
		if ( nNumLights > 1 )
			cSpecularHighlights.rgb += pow( saturate( dot( vCorneaReflectionVector.xyz, PixelShaderGetLightVector( wp, PSREG_LIGHT_INFO_ARRAY, 1 ) ) ), 128.0 ) *
			                           vLightFalloffCosine01.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 1 );
		if ( nNumLights > 2 )
			cSpecularHighlights.rgb += pow( saturate( dot( vCorneaReflectionVector.xyz, PixelShaderGetLightVector( wp, PSREG_LIGHT_INFO_ARRAY, 2 ) ) ), 128.0 ) *
			                           vLightFalloffCosine23.x * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 2 );
		if ( nNumLights > 3 )
			cSpecularHighlights.rgb += pow( saturate( dot( vCorneaReflectionVector.xyz, PixelShaderGetLightVector( wp, PSREG_LIGHT_INFO_ARRAY, 3 ) ) ), 128.0 ) *
			                           vLightFalloffCosine23.y * PixelShaderGetLightColor( PSREG_LIGHT_INFO_ARRAY, 3 );
	}

	// Combine terms
	vec4 result;

	// Unlit iris, pupil, and sclera color
	result.rgb = cIrisColor.rgb;

	// Add in slight cornea noise to help define raised cornea layer for close-ups
	result.rgb += fCorneaNoise * 0.1;

	// Diffuse light (Vertex lighting + extra iris caustic lighting)
	result.rgb *= cVertexLight.rgb + cIrisLighting.rgb;

	// Environment map
	result.rgb += cReflection.rgb * cVertexLight.rgb;

	// Local light glints
	result.rgb += cSpecularHighlights.rgb;

	// Set alpha to 1.0 by default
	result.a = 1.0;

	float fogFactor = CalcPixelFogFactorConst( g_fPixelFogType, g_FogParams, g_vCameraPosition.z,
	    vWorldPosition_ProjPosZ.z, vWorldPosition_ProjPosZ.w );
	LegacyWrite( FinalOutputConst( result, fogFactor, g_fPixelFogType, TONEMAP_SCALE_LINEAR ) );
}
