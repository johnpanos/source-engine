#version 450
// UnlitGeneric and VertexLitGeneric's pixel stage: a port of
// stdshaders/vertexlit_and_unlit_generic_ps2x.fxc (ps20b). Combos
// (fxctmp9/vertexlit_and_unlit_generic_ps20b.inc): static DETAILTEXTURE (6),
// CUBEMAP (12), DIFFUSELIGHTING (24), ENVMAPMASK (48), BASEALPHAENVMAPMASK (96),
// SELFILLUM (192), VERTEXCOLOR (384), FLASHLIGHT (768), SELFILLUM_ENVMAPMASK_ALPHA
// (1536), DETAIL_BLEND_MODE (3072, 0..9), SEAMLESS_BASE (30720), SEAMLESS_DETAIL
// (61440), DISTANCEALPHA (122880), DISTANCEALPHAFROMDETAIL (245760), SOFT_MASK
// (491520), OUTLINE (983040), OUTER_GLOW (1966080), FLASHLIGHTDEPTHFILTERMODE
// (3932160, 0..2), DEPTHBLEND (11796480), BLENDTINTBYBASEALPHA (23592960),
// SRGB_INPUT_ADAPTER (47185920), CUBEMAP_SPHERE_LEGACY (94371840); dynamic
// LIGHTING_PREVIEW (1, 0..2) and FLASHLIGHTSHADOWS (3). FLASHLIGHT and
// FLASHLIGHTSHADOWS are never selected here (the backend reports no flashlight
// mode), and LIGHTING_PREVIEW only in Hammer's lighting preview, whose mode 2
// writes multiple render targets: mode 2 draws its color target only.
// @legacy program=vertexlit_and_unlit_generic ps=vertexlit_and_unlit_generic_ps20b
//         vs=vertexlit_and_unlit_generic_vs20 vert=vertexlit_and_unlit_generic_vs20
//         samplers=0:2d,1:cube,1:2d,2:2d,4:2d,10:2d,11:2d flags=object_position_extra
#include "legacy_ps.glsl"
#include "legacy_combine.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;     // s0
layout( set = 0, binding = 1 ) uniform samplerCube EnvmapSampler;        // s1
layout( set = 0, binding = 2 ) uniform sampler2D EnvmapSphereSampler;    // s1 (sphere map)
layout( set = 0, binding = 3 ) uniform sampler2D DetailSampler;          // s2
layout( set = 0, binding = 4 ) uniform sampler2D EnvmapMaskSampler;      // s4
layout( set = 0, binding = 5 ) uniform sampler2D DepthSampler;           // s10
layout( set = 0, binding = 6 ) uniform sampler2D SelfIllumMaskSampler;   // s11

layout( location = 0 ) in vec3 baseTexCoord;   // xy, or xyz with SEAMLESS_BASE
layout( location = 1 ) in vec3 detailTexCoord; // xy, or xyz with SEAMLESS_DETAIL
layout( location = 2 ) in vec4 color;          // vertex color (from lighting or unlit)
layout( location = 3 ) in vec3 worldVertToEyeVector;
layout( location = 4 ) in vec3 worldSpaceNormal;
layout( location = 6 ) in vec4 projPos;
layout( location = 7 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec3 SeamlessWeights;

#define g_EnvmapTint_TintReplaceFactor PS_C( 0 )
#define g_DiffuseModulation PS_C( 1 )
#define g_EnvmapContrast_ShadowTweaks PS_C( 2 )
#define g_EnvmapSaturation_SelfIllumMask PS_C( 3 )
#define g_SelfIllumTint_and_BlendFactor PS_C( 4 )
#define g_GlowParameters PS_C( 5 )
#define g_GlowColor PS_C( 6 )
#define g_DistanceAlphaParams PS_C( 7 )
#define g_OutlineColor PS_C( 8 )
#define g_OutlineParams PS_C( 9 )
#define g_DetailTint ( PS_C( 10 ).xyz )
#define g_ShaderControls PS_C( 12 )
#define g_DepthFeatheringConstants PS_C( 13 )
#define g_EyePos PS_C( 20 )
#define g_FogParams PS_C( 21 )

#define g_SelfIllumTint ( g_SelfIllumTint_and_BlendFactor.xyz )
#define g_DetailBlendFactor ( g_SelfIllumTint_and_BlendFactor.w )
#define g_EnvmapSaturation ( g_EnvmapSaturation_SelfIllumMask.xyz )
#define g_SelfIllumMaskControl ( g_EnvmapSaturation_SelfIllumMask.w )
#define GLOW_UV_OFFSET ( g_GlowParameters.xy )
#define OUTER_GLOW_MIN_DVALUE ( g_GlowParameters.z )
#define OUTER_GLOW_MAX_DVALUE ( g_GlowParameters.w )
#define SOFT_MASK_MAX ( g_DistanceAlphaParams.x )
#define SOFT_MASK_MIN ( g_DistanceAlphaParams.y )
#define g_fPixelFogType ( g_ShaderControls.x )
#define g_fWriteDepthToAlpha ( g_ShaderControls.y )
#define g_fWriteWaterFogToDestAlpha ( g_ShaderControls.z )
#define g_fVertexAlpha ( g_ShaderControls.w )

vec3 CalcReflectionVectorUnnormalized( vec3 normal, vec3 eyeVector )
{
	return ( 2.0 * dot( normal, eyeVector ) ) * normal - dot( normal, normal ) * eyeVector;
}

// The shader's own fog: range and height fog selected by a constant.
float CalcPixelFogFactorConst(
    float fPixelFogType, vec4 fogParams, float flEyePosZ, float flWorldPosZ, float flProjPosZ )
{
	float flDepthBelowWater = fPixelFogType * fogParams.y - flWorldPosZ;
	float flDepthBelowEye = fPixelFogType * flEyePosZ - flWorldPosZ;
	float frac = ( flDepthBelowEye == 0.0 ) ? 1.0 : saturate( flDepthBelowWater / flDepthBelowEye );
	return saturate( min( fogParams.z, flProjPosZ * fogParams.w * frac - fogParams.x ) );
}
vec3 BlendPixelFogConst( vec3 vShaderColor, float pixelFogFactor, vec3 vFogColor, float fPixelFogType )
{
	pixelFogFactor = mix( pixelFogFactor * pixelFogFactor, pixelFogFactor, fPixelFogType );
	return mix( vShaderColor.rgb, vFogColor.rgb, pixelFogFactor );
}
vec4 FinalOutputConst( vec4 vShaderColor, float pixelFogFactor, float fPixelFogType,
    float fWriteDepthToDestAlpha, float flProjZ )
{
	vec4 result = vShaderColor;
	result.rgb *= LINEAR_LIGHT_SCALE; // TONEMAP_SCALE_LINEAR
	result.a = mix( result.a, DepthToDestAlpha( flProjZ ), fWriteDepthToDestAlpha );
	result.rgb = BlendPixelFogConst( result.rgb, pixelFogFactor, g_LinearFogColor.rgb, fPixelFogType );
	return result;
}

void main()
{
	const bool DETAILTEXTURE = STATIC_PS_COMBO( 6, 2 ) != 0;
	const bool CUBEMAP = STATIC_PS_COMBO( 12, 2 ) != 0;
	const bool DIFFUSELIGHTING = STATIC_PS_COMBO( 24, 2 ) != 0;
	const bool ENVMAPMASK = STATIC_PS_COMBO( 48, 2 ) != 0;
	const bool BASEALPHAENVMAPMASK = STATIC_PS_COMBO( 96, 2 ) != 0;
	const bool SELFILLUM = STATIC_PS_COMBO( 192, 2 ) != 0;
	const bool VERTEXCOLOR = STATIC_PS_COMBO( 384, 2 ) != 0;
	const bool SELFILLUM_ENVMAPMASK_ALPHA = STATIC_PS_COMBO( 1536, 2 ) != 0;
	const int DETAIL_BLEND_MODE = STATIC_PS_COMBO( 3072, 10 );
	const bool SEAMLESS_BASE = STATIC_PS_COMBO( 30720, 2 ) != 0;
	const bool SEAMLESS_DETAIL = STATIC_PS_COMBO( 61440, 2 ) != 0;
	const bool DISTANCEALPHA = STATIC_PS_COMBO( 122880, 2 ) != 0;
	const bool DISTANCEALPHAFROMDETAIL = STATIC_PS_COMBO( 245760, 2 ) != 0;
	const bool SOFT_MASK = STATIC_PS_COMBO( 491520, 2 ) != 0;
	const bool OUTLINE = STATIC_PS_COMBO( 983040, 2 ) != 0;
	const bool OUTER_GLOW = STATIC_PS_COMBO( 1966080, 2 ) != 0;
	const bool DEPTHBLEND = STATIC_PS_COMBO( 11796480, 2 ) != 0;
	const bool BLENDTINTBYBASEALPHA = STATIC_PS_COMBO( 23592960, 2 ) != 0;
	const bool SRGB_INPUT_ADAPTER = STATIC_PS_COMBO( 47185920, 2 ) != 0;
	const bool CUBEMAP_SPHERE_LEGACY = STATIC_PS_COMBO( 94371840, 2 ) != 0;
	const int LIGHTING_PREVIEW = DYNAMIC_PS_COMBO( 1, 3 );

	vec4 baseColor;
	if ( SEAMLESS_BASE )
	{
		baseColor = SeamlessWeights.x * tex2D( 0, BaseTextureSampler, baseTexCoord.yz ) +
		            SeamlessWeights.y * tex2D( 0, BaseTextureSampler, baseTexCoord.zx ) +
		            SeamlessWeights.z * tex2D( 0, BaseTextureSampler, baseTexCoord.xy );
	}
	else
	{
		baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord.xy );
		if ( SRGB_INPUT_ADAPTER )
			baseColor.rgb = GammaToLinear( baseColor.rgb );
	}

	float distAlphaMask = baseColor.a;
	vec4 detailColor = vec4( 0.0 );
	if ( DETAILTEXTURE )
	{
		if ( SEAMLESS_DETAIL )
		{
			detailColor = SeamlessWeights.x * tex2D( 3, DetailSampler, detailTexCoord.yz ) +
			              SeamlessWeights.y * tex2D( 3, DetailSampler, detailTexCoord.zx ) +
			              SeamlessWeights.z * tex2D( 3, DetailSampler, detailTexCoord.xy );
		}
		else
		{
			detailColor = tex2D( 3, DetailSampler, detailTexCoord.xy );
		}
		detailColor.rgb *= g_DetailTint;
		if ( DISTANCEALPHA && DISTANCEALPHAFROMDETAIL )
		{
			distAlphaMask = detailColor.a;
			detailColor.a = 1.0; // make tcombine treat as 1.0
		}
		baseColor = TextureCombine( baseColor, detailColor, DETAIL_BLEND_MODE, g_DetailBlendFactor );
	}

	if ( DISTANCEALPHA )
	{
		if ( OUTLINE )
		{
			vec4 oFactors = smoothstep( g_OutlineParams.xyzw, g_OutlineParams.wzyx, vec4( distAlphaMask ) );
			baseColor = mix( baseColor, g_OutlineColor, oFactors.x * oFactors.y );
		}
		float mskUsed;
		if ( SOFT_MASK )
		{
			mskUsed = smoothstep( SOFT_MASK_MIN, SOFT_MASK_MAX, distAlphaMask );
			baseColor.a *= mskUsed;
		}
		else
		{
			mskUsed = distAlphaMask >= 0.5 ? 1.0 : 0.0;
			if ( DETAILTEXTURE )
				baseColor.a *= mskUsed;
			else
				baseColor.a = mskUsed;
		}
		if ( OUTER_GLOW )
		{
			vec4 glowTexel = DISTANCEALPHAFROMDETAIL
			                     ? tex2D( 3, DetailSampler, detailTexCoord.xy + GLOW_UV_OFFSET )
			                     : tex2D( 0, BaseTextureSampler, baseTexCoord.xy + GLOW_UV_OFFSET );
			vec4 glowc =
			    g_GlowColor * smoothstep( OUTER_GLOW_MIN_DVALUE, OUTER_GLOW_MAX_DVALUE, glowTexel.a );
			baseColor = mix( glowc, baseColor, mskUsed );
		}
	}

	vec3 specularFactor = vec3( 1.0 );
	vec4 envmapMaskTexel = vec4( 0.0 );
	if ( ENVMAPMASK )
	{
		envmapMaskTexel = tex2D( 4, EnvmapMaskSampler, baseTexCoord.xy );
		specularFactor *= envmapMaskTexel.xyz;
	}
	if ( BASEALPHAENVMAPMASK )
		specularFactor *= 1.0 - baseColor.a;

	vec3 diffuseLighting = vec3( 1.0 );
	if ( DIFFUSELIGHTING || VERTEXCOLOR && !( VERTEXCOLOR && DIFFUSELIGHTING ) )
		diffuseLighting = color.rgb;

	vec3 albedo = baseColor.rgb;
	if ( BLENDTINTBYBASEALPHA )
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
	if ( !BASEALPHAENVMAPMASK && !SELFILLUM && !BLENDTINTBYBASEALPHA )
		alpha *= baseColor.a;

	if ( VERTEXCOLOR && DIFFUSELIGHTING )
		albedo *= color.rgb;

	alpha = mix( alpha, alpha * color.a, g_fVertexAlpha );

	vec3 diffuseComponent = albedo * diffuseLighting;
	if ( DETAILTEXTURE )
		diffuseComponent = TextureCombinePostLighting(
		    diffuseComponent, detailColor, DETAIL_BLEND_MODE, g_DetailBlendFactor );

	vec3 specularLighting = vec3( 0.0 );
	if ( SELFILLUM_ENVMAPMASK_ALPHA )
	{
		// alpha 0 - 0.125 lerps diffuse to self-illumination; 0.125 - 1.0 glows.
		vec3 selfIllumComponent = g_SelfIllumTint * albedo;
		float Adj_Alpha = 8.0 * envmapMaskTexel.a;
		diffuseComponent =
		    ( max( 0.0, 1.0 - Adj_Alpha ) * diffuseComponent ) + Adj_Alpha * selfIllumComponent;
	}
	else if ( SELFILLUM )
	{
		vec3 vSelfIllumMask = tex2D( 6, SelfIllumMaskSampler, baseTexCoord.xy ).rgb;
		vSelfIllumMask = mix( vec3( baseColor.a ), vSelfIllumMask, g_SelfIllumMaskControl );
		diffuseComponent = mix( diffuseComponent, g_SelfIllumTint * albedo, vSelfIllumMask );
	}

	if ( CUBEMAP )
	{
		if ( CUBEMAP_SPHERE_LEGACY )
		{
			vec3 reflectVect =
			    normalize( CalcReflectionVectorUnnormalized( worldSpaceNormal, worldVertToEyeVector ) );
			specularLighting = 0.5 * tex2D( 2, EnvmapSphereSampler, reflectVect.xy ).rgb *
			                   g_DiffuseModulation.rgb * diffuseLighting;
		}
		else
		{
			vec3 reflectVect = CalcReflectionVectorUnnormalized( worldSpaceNormal, worldVertToEyeVector );
			specularLighting = ENV_MAP_SCALE * texCUBE( 1, EnvmapSampler, reflectVect ).rgb;
			specularLighting *= specularFactor;
			specularLighting *= g_EnvmapTint_TintReplaceFactor.rgb;
			vec3 specularLightingSquared = specularLighting * specularLighting;
			specularLighting =
			    mix( specularLighting, specularLightingSquared, g_EnvmapContrast_ShadowTweaks.xyz );
			vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
			specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
		}
	}

	vec3 result = diffuseComponent + specularLighting;

	if ( LIGHTING_PREVIEW == 1 )
	{
		float dotprod = 0.7 + 0.25 * dot( worldSpaceNormal, normalize( vec3( 1.0, 2.0, -0.5 ) ) );
		LegacyWrite( FinalOutput( vec4( dotprod * albedo.xyz, alpha ), 0.0, PIXEL_FOG_TYPE_NONE,
		    TONEMAP_SCALE_LINEAR ) );
		return;
	}
	if ( LIGHTING_PREVIEW == 2 )
	{
		LegacyWrite( FinalOutput( vec4( albedo.xyz, alpha ), 0.0, PIXEL_FOG_TYPE_NONE,
		    TONEMAP_SCALE_NONE ) );
		return;
	}

	if ( DEPTHBLEND )
	{
		vec2 vScreenPos = vec2( projPos.x, -projPos.y );
		vScreenPos = ( vScreenPos + projPos.w ) * 0.5;
		alpha *= DepthFeathering( 5, DepthSampler, vScreenPos / projPos.w, projPos.w - projPos.z,
		    projPos.w, g_DepthFeatheringConstants );
	}

	float fogFactor = CalcPixelFogFactorConst(
	    g_fPixelFogType, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, projPos.z );
	alpha = mix( alpha, fogFactor, g_fWriteWaterFogToDestAlpha );
	LegacyWrite( FinalOutputConst( vec4( result.rgb, alpha ), fogFactor, g_fPixelFogType,
	    g_fWriteDepthToAlpha, projPos.z ) );
}
