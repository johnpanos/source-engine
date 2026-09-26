#version 450
// ShatteredGlass' pixel stage: a port of stdshaders/shatteredglass_ps2x.fxc
// (ps20b). Base times detail, lit "somewhat unlit" by the lightmap
// (g_OverbrightFactor's unlit factor), plus the Fresnel-weighted cube map
// reflection masked by the envmap mask and/or the base alpha. Combos
// (fxctmp9/ShatteredGlass_ps20b.inc): static CONVERT_TO_SRGB (4, always 0
// here), CUBEMAP (8), VERTEXCOLOR (16), ENVMAPMASK (32), BASEALPHAENVMAPMASK
// (64), HDRTYPE (128); dynamic HDRENABLED (1, unused by the shader),
// PIXELFOGTYPE (2). The eye vector and normal are normalized through the
// normalization cube map (s6), as the HLSL does.
// @legacy program=shatteredglass ps=shatteredglass_ps20b vs=shatteredglass_vs20
//         vert=shatteredglass_vs20 samplers=0:2d,1:2d,2:cube,3:2d,5:2d,6:cube
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;  // s0
layout( set = 0, binding = 1 ) uniform sampler2D LightmapSampler;     // s1
layout( set = 0, binding = 2 ) uniform samplerCube EnvmapSampler;     // s2
layout( set = 0, binding = 3 ) uniform sampler2D DetailSampler;       // s3
layout( set = 0, binding = 4 ) uniform sampler2D EnvmapMaskSampler;   // s5
layout( set = 0, binding = 5 ) uniform samplerCube NormalizeSampler;  // s6

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec2 detailTexCoord;
layout( location = 2 ) in vec2 lightmapTexCoord;
layout( location = 4 ) in vec4 worldPos_projPosZ;
layout( location = 5 ) in vec3 worldSpaceNormalIn;
layout( location = 8 ) in vec4 vertexColor;

#define g_EnvmapTint PS_C( 0 )
#define g_DiffuseModulation PS_C( 1 )
#define g_EnvmapContrast PS_C( 2 )
#define g_EnvmapSaturation PS_C( 3 )
#define g_FresnelReflection PS_C( 4 )
#define g_EyePos PS_C( 5 )
#define g_OverbrightFactor PS_C( 6 )
#define g_FogParams PS_C( 12 )

vec3 NormalizeWithCubemap( vec3 v )
{
	return texCUBE( 5, NormalizeSampler, v ).xyz;
}

void main()
{
	const bool bCubemap = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool bVertexColor = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool bEnvmapMask = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool bBaseAlphaEnvmapMask = STATIC_PS_COMBO( 64, 2 ) != 0;
	const int HDRTYPE = STATIC_PS_COMBO( 128, 3 );
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );

	const vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord );
	const vec4 detailColor = tex2D( 3, DetailSampler, detailTexCoord );
	const vec3 lightmapColor = tex2D( 1, LightmapSampler, lightmapTexCoord ).rgb;

	vec3 specularFactor = vec3( 1.0 );
	if ( bEnvmapMask )
		specularFactor = tex2D( 4, EnvmapMaskSampler, detailTexCoord ).xyz;
	if ( bBaseAlphaEnvmapMask )
		specularFactor *= 1.0 - baseColor.a; // this blows!

	vec3 diffuseLighting = lightmapColor;
	diffuseLighting *= g_DiffuseModulation.rgb;
	diffuseLighting *= LIGHT_MAP_SCALE;

	vec3 albedo = baseColor.rgb;
	float alpha = 1.0;
	if ( !bBaseAlphaEnvmapMask )
		alpha *= baseColor.a;

	albedo *= detailColor.rgb;
	alpha *= detailColor.a;

	// vertex alpha is ignored if vertexcolor isn't set.
	if ( bVertexColor )
	{
		albedo *= vertexColor.rgb;
		alpha *= vertexColor.a;
	}

	vec3 specularLighting = vec3( 0.0 );
	if ( bCubemap )
	{
		vec3 worldVertToEyeVector = g_EyePos.xyz - worldPos_projPosZ.xyz;
		worldVertToEyeVector = NormalizeWithCubemap( worldVertToEyeVector );
		// CalcReflectionVectorUnnormalized
		const vec3 n = worldSpaceNormalIn;
		const vec3 reflectVect =
		    2.0 * dot( n, worldVertToEyeVector ) * n - dot( n, n ) * worldVertToEyeVector;

		// Calc Fresnel factor
		const vec3 worldSpaceNormal = NormalizeWithCubemap( worldSpaceNormalIn );
		float fresnel = 1.0 - dot( worldSpaceNormal, worldVertToEyeVector );
		fresnel = pow( fresnel, 5.0 );
		fresnel = fresnel * g_FresnelReflection.b + g_FresnelReflection.a;

		specularLighting = texCUBE( 2, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;
		specularLighting *= g_EnvmapTint.rgb;
		if ( HDRTYPE == 0 ) // HDR_TYPE_NONE
		{
			const vec3 specularLightingSquared = specularLighting * specularLighting;
			specularLighting = mix( specularLighting, specularLightingSquared, g_EnvmapContrast.rgb );
			const vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
			specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation.rgb );
		}
		specularLighting *= fresnel;
	}

	// Do it somewhat unlit
	const vec3 result =
	    albedo * ( g_OverbrightFactor.z * diffuseLighting + g_OverbrightFactor.y ) + specularLighting;

	const float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
