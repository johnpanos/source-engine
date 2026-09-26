#version 450
// VertexLitGeneric and UnlitGeneric's bumped pixel stage ($bumpmap or
// $lightwarptexture without $phong): a port of
// stdshaders/vertexlit_and_unlit_generic_bump_ps2x.fxc (ps20b). Combos
// (fxctmp9/vertexlit_and_unlit_generic_bump_ps20b.inc): static CUBEMAP (20),
// DIFFUSELIGHTING (40), LIGHTWARPTEXTURE (80), SELFILLUM (160),
// SELFILLUMFRESNEL (320), NORMALMAPALPHAENVMAPMASK (640), HALFLAMBERT (1280),
// FLASHLIGHT (2560), DETAILTEXTURE (5120), DETAIL_BLEND_MODE (10240, 0..6),
// FLASHLIGHTDEPTHFILTERMODE (71680, 0..2), BLENDTINTBYBASEALPHA (215040);
// dynamic NUM_LIGHTS (1, 0..4), AMBIENT_LIGHT (5), FLASHLIGHTSHADOWS (10).
// FLASHLIGHT, FLASHLIGHTDEPTHFILTERMODE and FLASHLIGHTSHADOWS are not ported:
// this backend never draws a flashlight pass (InFlashlightMode is false and it
// reports no shadow filter mode), so those passes are never drawn.
// @legacy program=vertexlit_and_unlit_generic_bump ps=vertexlit_and_unlit_generic_bump_ps20b
//         vs=vertexlit_and_unlit_generic_bump_vs20 vert=vertexlit_and_unlit_generic_bump_vs20
//         samplers=0:2d,1:cube,2:2d,3:2d,9:2d
#include "legacy_ps.glsl"
#include "legacy_combine.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform samplerCube EnvmapSampler;    // s1
layout( set = 0, binding = 2 ) uniform sampler2D DetailSampler;      // s2
layout( set = 0, binding = 3 ) uniform sampler2D BumpmapSampler;     // s3
layout( set = 0, binding = 4 ) uniform sampler2D DiffuseWarpSampler; // s9

layout( location = 0 ) in vec4 baseTexCoord2_tangentSpaceVertToEyeVectorXY;
layout( location = 1 ) in vec3 lightAtten;
layout( location = 2 ) in vec4 worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ;
layout( location = 3 ) in vec3 vWorldNormal;
layout( location = 4 ) in vec4 vWorldTangent;
layout( location = 5 ) in vec4 vProjPos;
layout( location = 6 ) in vec4 worldPos_projPosZ;
layout( location = 7 ) in vec3 detailTexCoord_atten3;

#define g_EnvmapTint_TintReplaceFactor PS_C( 0 )
#define g_DiffuseModulation PS_C( 1 )
#define g_EnvmapContrast_ShadowTweaks PS_C( 2 )
#define g_EnvmapSaturation ( PS_C( 3 ).xyz )
#define g_SelfIllumTint_and_BlendFactor PS_C( 4 )
#define cAmbientCubeReg 5
#define g_SelfIllumScaleBiasExpBrightness PS_C( 11 )
#define g_ShaderControls PS_C( 12 )
#define cLightInfoReg 13
#define g_EyePos ( PS_C( 20 ).xyz )
#define g_FogParams PS_C( 21 )

#define g_SelfIllumTint ( g_SelfIllumTint_and_BlendFactor.rgb )
#define g_DetailBlendFactor ( g_SelfIllumTint_and_BlendFactor.w )
#define g_fPixelFogType ( g_ShaderControls.x )
#define g_fWriteDepthToAlpha ( g_ShaderControls.y )
#define g_fWriteWaterFogToDestAlpha ( g_ShaderControls.z )

vec3 CalcReflectionVectorUnnormalized( vec3 normal, vec3 eyeVector )
{
	return ( 2.0 * dot( normal, eyeVector ) ) * normal - dot( normal, normal ) * eyeVector;
}

// Calculate both types of Fog and lerp to get result
float CalcPixelFogFactorConst(
    float fPixelFogType, vec4 fogParams, float flEyePosZ, float flWorldPosZ, float flProjPosZ )
{
	const float fRangeFog = CalcRangeFog( flProjPosZ, fogParams.x, fogParams.z, fogParams.w );
	const float fHeightFog =
	    CalcWaterFogAlpha( fogParams.y, flEyePosZ, flWorldPosZ, flProjPosZ, fogParams.w );
	return mix( fRangeFog, fHeightFog, fPixelFogType );
}

// Blend both types of Fog and lerp to get result
vec3 BlendPixelFogConst(
    vec3 vShaderColor, float pixelFogFactor, vec3 vFogColor, float fPixelFogType )
{
	pixelFogFactor = saturate( pixelFogFactor );
	// squaring the factor will get the middle range mixing closer to hardware fog
	const vec3 fRangeResult =
	    mix( vShaderColor.rgb, vFogColor.rgb, pixelFogFactor * pixelFogFactor );
	const vec3 fHeightResult = mix( vShaderColor.rgb, vFogColor.rgb, saturate( pixelFogFactor ) );
	return mix( fRangeResult, fHeightResult, fPixelFogType );
}

vec4 FinalOutputConst( vec4 vShaderColor, float pixelFogFactor, float fPixelFogType,
    int iTONEMAP_SCALE_TYPE, float fWriteDepthToDestAlpha, float flProjZ )
{
	vec4 result = vShaderColor;
	if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_LINEAR )
		result.rgb *= LINEAR_LIGHT_SCALE;
	else if ( iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_GAMMA )
		result.rgb *= GAMMA_LIGHT_SCALE;

	result.a = mix( result.a, DepthToDestAlpha( flProjZ ), fWriteDepthToDestAlpha );

	result.rgb =
	    BlendPixelFogConst( result.rgb, pixelFogFactor, g_LinearFogColor.rgb, fPixelFogType );
	// SRGBOutput is the identity (CONVERT_TO_SRGB is 0 on this backend).
	return result;
}

void main()
{
	const bool bCubemap = STATIC_PS_COMBO( 20, 2 ) != 0;
	const bool bDiffuseLighting = STATIC_PS_COMBO( 40, 2 ) != 0;
	const bool bDoDiffuseWarp = STATIC_PS_COMBO( 80, 2 ) != 0;
	const bool bSelfIllum = STATIC_PS_COMBO( 160, 2 ) != 0;
	const bool bSelfIllumFresnel = STATIC_PS_COMBO( 320, 2 ) != 0;
	const bool bNormalMapAlphaEnvmapMask = STATIC_PS_COMBO( 640, 2 ) != 0;
	const bool bHalfLambert = STATIC_PS_COMBO( 1280, 2 ) != 0;
	const bool bDetailTexture = STATIC_PS_COMBO( 5120, 2 ) != 0;
	const int DETAIL_BLEND_MODE = STATIC_PS_COMBO( 10240, 7 );
	const bool bBlendTintByBaseAlpha = STATIC_PS_COMBO( 215040, 2 ) != 0;
	const int nNumLights = DYNAMIC_PS_COMBO( 1, 5 );
	const bool bAmbientLight = DYNAMIC_PS_COMBO( 5, 2 ) != 0;

	const vec3 vWorldBinormal = cross( vWorldNormal.xyz, vWorldTangent.xyz ) * vWorldTangent.w;

	// Unpack four light attenuations
	const vec4 vLightAtten = vec4( lightAtten, detailTexCoord_atten3.z );

	vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord2_tangentSpaceVertToEyeVectorXY.xy );

	vec4 detailColor = vec4( 0.0 );
	if ( bDetailTexture )
	{
		detailColor = tex2D( 2, DetailSampler, detailTexCoord_atten3.xy );
		baseColor =
		    TextureCombine( baseColor, detailColor, DETAIL_BLEND_MODE, g_DetailBlendFactor );
	}

	float specularFactor = 1.0;
	const vec4 normalTexel =
	    tex2D( 3, BumpmapSampler, baseTexCoord2_tangentSpaceVertToEyeVectorXY.xy );
	const vec3 tangentSpaceNormal = normalTexel.xyz * 2.0 - 1.0;

	if ( bNormalMapAlphaEnvmapMask )
		specularFactor = normalTexel.a;

	vec3 diffuseLighting = vec3( 1.0 );

	vec3 worldSpaceNormal = vec3( 0.0, 0.0, 1.0 );
	if ( bDiffuseLighting || bCubemap || bSelfIllumFresnel )
	{
		// Vec3TangentToWorld, then normalized (ps_2_b).
		worldSpaceNormal = tangentSpaceNormal.x * vWorldTangent.xyz;
		worldSpaceNormal += tangentSpaceNormal.y * vWorldBinormal;
		worldSpaceNormal += tangentSpaceNormal.z * vWorldNormal;
		worldSpaceNormal = normalize( worldSpaceNormal );
	}

	if ( bDiffuseLighting )
	{
		diffuseLighting = PixelShaderDoLighting( worldPos_projPosZ.xyz, worldSpaceNormal,
		    vec3( 0.0 ), false, bAmbientLight, vLightAtten, cAmbientCubeReg, nNumLights,
		    cLightInfoReg, bHalfLambert, false, 1.0, bDoDiffuseWarp, 4, DiffuseWarpSampler );
	}

	vec3 albedo = baseColor.rgb;
	if ( bBlendTintByBaseAlpha )
	{
		vec3 tintedColor = albedo * g_DiffuseModulation.rgb;
		tintedColor = mix( tintedColor, g_DiffuseModulation.rgb, g_EnvmapTint_TintReplaceFactor.w );
		albedo = mix( albedo, tintedColor, baseColor.a );
	}
	else
	{
		albedo = albedo * g_DiffuseModulation.rgb;
	}

	float alpha = g_DiffuseModulation.a;
	if ( !bSelfIllum && !bBlendTintByBaseAlpha )
		alpha *= baseColor.a;

	vec3 diffuseComponent = albedo * diffuseLighting;

	if ( bSelfIllum )
	{
		if ( bSelfIllumFresnel )
		{
			// A fresnel term based on the vertex normal (not the per-pixel normal!)
			// to help fake an internal glow look.
			const vec3 vVertexNormal = normalize( vWorldNormal.xyz );
			const vec3 vEyeDir =
			    normalize( worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ.xyz );
			const float flSelfIllumFresnel =
			    ( HlslPow( saturate( dot( vVertexNormal.xyz, vEyeDir ) ),
			          g_SelfIllumScaleBiasExpBrightness.z ) *
			        g_SelfIllumScaleBiasExpBrightness.x ) +
			    g_SelfIllumScaleBiasExpBrightness.y;

			const vec3 selfIllumComponent =
			    g_SelfIllumTint * albedo * g_SelfIllumScaleBiasExpBrightness.w;
			diffuseComponent = mix( diffuseComponent, selfIllumComponent,
			    baseColor.a * saturate( flSelfIllumFresnel ) );
		}
		else
		{
			const vec3 selfIllumComponent = g_SelfIllumTint * albedo;
			diffuseComponent = mix( diffuseComponent, selfIllumComponent, baseColor.a );
		}
	}

	vec3 specularLighting = vec3( 0.0 );
	if ( bCubemap )
	{
		const vec3 reflectVect = CalcReflectionVectorUnnormalized(
		    worldSpaceNormal, worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ.xyz );

		specularLighting = ENV_MAP_SCALE * texCUBE( 1, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;
		specularLighting *= g_EnvmapTint_TintReplaceFactor.rgb;
		const vec3 specularLightingSquared = specularLighting * specularLighting;
		specularLighting =
		    mix( specularLighting, specularLightingSquared, g_EnvmapContrast_ShadowTweaks.xyz );
		const vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
		specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
	}

	const vec3 result = diffuseComponent + specularLighting;

	const float fogFactor = CalcPixelFogFactorConst(
	    g_fPixelFogType, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );

	// Use the fog factor if it's height fog
	alpha = mix( alpha, fogFactor, g_fPixelFogType * g_fWriteWaterFogToDestAlpha );

	LegacyWrite( FinalOutputConst( vec4( result.rgb, alpha ), fogFactor, g_fPixelFogType,
	    TONEMAP_SCALE_LINEAR, g_fWriteDepthToAlpha, worldPos_projPosZ.w ) );
}
