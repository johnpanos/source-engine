#version 450
// VortWarp_DX9's pixel stage: a port of stdshaders/vortwarp_ps2x.fxc (ps20b).
// Combos (fxctmp9/vortwarp_ps20b.inc): static CONVERT_TO_SRGB (80, always 0
// here), BASETEXTURE (160), CUBEMAP (320), DIFFUSELIGHTING (640),
// NORMALMAPALPHAENVMAPMASK (1280), HALFLAMBERT (2560), FLASHLIGHT (5120),
// TRANSLUCENT (10240); dynamic WRITEWATERFOGTODESTALPHA (1), PIXELFOGTYPE (2),
// WARPINGIN (4), AMBIENT_LIGHT (8), NUM_LIGHTS (16, 0..4). DIFFUSELIGHTING
// normalizes the normal through the signed normalization cube map (s5).
// @legacy program=vortwarp ps=vortwarp_ps20b vs=vortwarp_vs20 vert=vortwarp_vs20
//         samplers=0:2d,1:cube,2:2d,3:2d,5:cube,6:2d,7:2d
#include "legacy_ps.glsl"
#include "legacy_ps_lighting.glsl"
#include "legacy_flashlight.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;  // s0
layout( set = 0, binding = 1 ) uniform samplerCube EnvmapSampler;     // s1
layout( set = 0, binding = 2 ) uniform sampler2D FlowMapSampler;      // s2
layout( set = 0, binding = 3 ) uniform sampler2D BumpmapSampler;      // s3
layout( set = 0, binding = 4 ) uniform samplerCube NormalizeSampler;  // s5
layout( set = 0, binding = 5 ) uniform sampler2D SelfIllumMapSampler; // s6
layout( set = 0, binding = 6 ) uniform sampler2D FlashlightSampler;   // s7

layout( location = 0 ) in vec4 baseTexCoord2_tangentSpaceVertToEyeVectorXY;
layout( location = 1 ) in vec4 lightAtten;
layout( location = 2 ) in vec4 worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ;
layout( location = 3 ) in vec3 tangentSpaceTranspose0;
layout( location = 4 ) in vec3 tangentSpaceTranspose1;
layout( location = 5 ) in vec3 tangentSpaceTranspose2;
layout( location = 6 ) in vec4 worldPos_projPosZ;

#define g_EnvmapTint PS_C( 0 )
#define g_DiffuseModulation PS_C( 1 )
#define g_EnvmapContrast ( PS_C( 2 ).xyz )
#define g_EnvmapSaturation ( PS_C( 3 ).xyz )
#define g_SelfIllumTint PS_C( 4 )
#define g_EyePos ( PS_C( 20 ).xyz )
#define g_FogParams PS_C( 21 )
#define g_FlashlightAttenuationFactors PS_C( 22 )
#define g_FlashlightPos ( PS_C( 23 ).xyz )
#define g_Time ( PS_C( 22 ).x )

// common_ps_fxc.h's NormalizeWithCubemap and CalcReflectionVectorUnnormalized.
vec3 NormalizeWithCubemap( vec3 v )
{
	return texCUBE( 4, NormalizeSampler, v ).xyz;
}
vec3 CalcReflectionVectorUnnormalized( vec3 normal, vec3 eyeVector )
{
	return ( 2.0 * ( dot( normal, eyeVector ) ) * normal ) - ( dot( normal, normal ) * eyeVector );
}

void main()
{
	const bool bBaseTexture = STATIC_PS_COMBO( 160, 2 ) != 0;
	const bool bCubemap = STATIC_PS_COMBO( 320, 2 ) != 0;
	const bool bDiffuseLighting = STATIC_PS_COMBO( 640, 2 ) != 0;
	const bool bNormalMapAlphaEnvmapMask = STATIC_PS_COMBO( 1280, 2 ) != 0;
	const bool bHalfLambert = STATIC_PS_COMBO( 2560, 2 ) != 0;
	const bool bFlashlight = STATIC_PS_COMBO( 5120, 2 ) != 0;
	const bool TRANSLUCENT = STATIC_PS_COMBO( 10240, 2 ) != 0;
	const bool WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );
	const bool WARPINGIN = DYNAMIC_PS_COMBO( 4, 2 ) != 0;
	const bool bAmbientLight = DYNAMIC_PS_COMBO( 8, 2 ) != 0;
	const int nNumLights = DYNAMIC_PS_COMBO( 16, 5 );

	const vec2 baseTexCoord = baseTexCoord2_tangentSpaceVertToEyeVectorXY.xy;
	// mul( tangentSpaceTranspose, v ) over the column registers.
	#define TANGENT_TO_WORLD( v ) ( ( v ).x * tangentSpaceTranspose0 + ( v ).y * tangentSpaceTranspose1 + ( v ).z * tangentSpaceTranspose2 )

	vec4 baseColor = vec4( 1.0 );
	if ( bBaseTexture )
		baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord );

	float specularFactor = 1.0;
	vec4 normalTexel = tex2D( 3, BumpmapSampler, baseTexCoord );
	vec3 tangentSpaceNormal = 2.0 * normalTexel.xyz - 1.0;

	if ( bNormalMapAlphaEnvmapMask )
		specularFactor = normalTexel.a;

	vec3 diffuseLighting = vec3( 1.0 );
	if ( bDiffuseLighting )
	{
		vec3 worldSpaceNormal = TANGENT_TO_WORLD( tangentSpaceNormal );
		worldSpaceNormal = NormalizeWithCubemap( worldSpaceNormal );
		diffuseLighting = PixelShaderDoLighting( worldPos_projPosZ.xyz, worldSpaceNormal, vec3( 0.0 ),
		    false, bAmbientLight, lightAtten, 5, nNumLights, 13, bHalfLambert, false, 0.0, false, 0,
		    BaseTextureSampler );
	}

	vec3 albedo = vec3( 1.0 );
	float alpha = 1.0;
	if ( bBaseTexture )
	{
		albedo *= baseColor.rgb;
		alpha *= baseColor.a;
	}

	// If we only have specularity, assume that we want a black diffuse component, and
	// get alpha from the envmapmask
	if ( !bBaseTexture && bCubemap )
	{
		diffuseLighting = vec3( 0.0 );
		if ( bNormalMapAlphaEnvmapMask )
			alpha *= specularFactor;
	}

	if ( bFlashlight )
	{
		vec3 worldSpaceNormal = TANGENT_TO_WORLD( tangentSpaceNormal );
		// mul( float4( worldPos, 1 ), g_FlashlightWorldToTexture ), c24..c27 by column.
		vec4 flashlightSpacePosition = vec4( worldPos_projPosZ.xyz, 1.0 ) *
		                               mat4( PS_C( 24 ), PS_C( 25 ), PS_C( 26 ), PS_C( 27 ) );
		diffuseLighting = DoFlashlight( g_FlashlightPos, worldPos_projPosZ.xyz, flashlightSpacePosition,
		    worldSpaceNormal, g_FlashlightAttenuationFactors.xyz, g_FlashlightAttenuationFactors.w, 6,
		    FlashlightSampler, true );
	}

	diffuseLighting *= g_DiffuseModulation.rgb;
	alpha *= g_DiffuseModulation.a;

	vec3 diffuseComponent = albedo * diffuseLighting;

	vec4 flowmapSample = tex2D( 2, FlowMapSampler, baseTexCoord );
	if ( !bFlashlight )
		flowmapSample.xy += vec2( 0.11, 0.124 ) * vec2( g_Time );
	vec4 selfIllumSample = tex2D( 5, SelfIllumMapSampler, flowmapSample.xy );

	diffuseComponent.xyz += albedo * g_SelfIllumTint.xyz * selfIllumSample.xyz;

	vec3 specularLighting = vec3( 0.0 );
	if ( bCubemap && !bFlashlight )
	{
		// If we've *only* specified a cubemap, blow off the diffuse component
		if ( !bBaseTexture && !bDiffuseLighting && !bFlashlight )
			diffuseComponent = vec3( 0.0 );

		vec3 worldSpaceNormal = TANGENT_TO_WORLD( tangentSpaceNormal );

		vec3 reflectVect = CalcReflectionVectorUnnormalized(
		    worldSpaceNormal, worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ.xyz );

		specularLighting = ENV_MAP_SCALE * texCUBE( 1, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;
		specularLighting *= g_EnvmapTint.rgb;
		vec3 specularLightingSquared = specularLighting * specularLighting;
		specularLighting = mix( specularLighting, specularLightingSquared, g_EnvmapContrast );
		vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
		specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
	}

	vec3 result = diffuseComponent + specularLighting;

	float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );
	if ( WRITEWATERFOGTODESTALPHA && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		alpha = fogFactor;

	// write alpha where the vortigaunt is so that we can selectivly draw pixels when refracting
	if ( !TRANSLUCENT )
		alpha = WARPINGIN ? 0.0 : 1.0;

	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
