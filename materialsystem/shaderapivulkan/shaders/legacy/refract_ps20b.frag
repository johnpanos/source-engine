#version 450
// Refract_DX90's pixel stage: a port of stdshaders/refract_ps2x.fxc (ps20b).
// Combos (fxctmp9/Refract_ps20b.inc): static CONVERT_TO_SRGB (4, 0 here), BLUR
// (8), FADEOUTONSILHOUETTE (16), CUBEMAP (32), REFRACTTINTTEXTURE (64), MASKED
// (128), COLORMODULATE (256), SECONDARY_NORMAL (512), NORMAL_DECODE_MODE and
// SHADER_SRGB_READ (1024, both 0 off OS X); dynamic PIXELFOGTYPE (1),
// WRITE_DEPTH_TO_DESTALPHA (2).
// @legacy program=refract ps=refract_ps20b vs=refract_vs20 vert=refract_vs20
//         samplers=1:2d,2:2d,3:2d,4:cube,5:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D NormalSampler2;     // s1
layout( set = 0, binding = 1 ) uniform sampler2D RefractSampler;     // s2
layout( set = 0, binding = 2 ) uniform sampler2D NormalSampler;      // s3
layout( set = 0, binding = 3 ) uniform samplerCube EnvmapSampler;    // s4
layout( set = 0, binding = 4 ) uniform sampler2D RefractTintSampler; // s5

layout( location = 0 ) in vec4 vBumpTexCoord;
layout( location = 1 ) in vec3 vTangentVertToEyeVector;
layout( location = 2 ) in vec3 vWorldNormal;
layout( location = 3 ) in vec3 vWorldTangent;
layout( location = 4 ) in vec3 vWorldBinormal;
layout( location = 5 ) in vec3 vRefractXYW;
layout( location = 6 ) in vec3 vWorldViewVector;
layout( location = 7 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 ColorModulate;

#define g_EnvmapTint ( PS_C( 0 ).xyz )
#define g_RefractTint ( PS_C( 1 ).xyz )
#define g_EnvmapContrast ( PS_C( 2 ).xyz )
#define g_EnvmapSaturation ( PS_C( 3 ).xyz )
#define g_RefractScale ( PS_C( 5 ).x )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

const float g_BlurFraction = 1.0 / 512.0;
const float g_HalfBlurFraction = 0.5 * g_BlurFraction;

// common_fxc.h's Vec3TangentToWorld and CalcReflectionVectorUnnormalized.
vec3 Vec3TangentToWorld( vec3 iTangentVector, vec3 iWorldNormal, vec3 iWorldTangent,
    vec3 iWorldBinormal )
{
	return iTangentVector.x * iWorldTangent + iTangentVector.y * iWorldBinormal +
	       iTangentVector.z * iWorldNormal;
}
vec3 CalcReflectionVectorUnnormalized( vec3 normal, vec3 eyeVector )
{
	return ( 2.0 * dot( normal, eyeVector ) ) * normal - dot( normal, normal ) * eyeVector;
}

void main()
{
	const int BLUR = STATIC_PS_COMBO( 8, 2 );
	const bool FADEOUTONSILHOUETTE = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool CUBEMAP = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool REFRACTTINTTEXTURE = STATIC_PS_COMBO( 64, 2 ) != 0;
	const bool MASKED = STATIC_PS_COMBO( 128, 2 ) != 0;
	const bool COLORMODULATE = STATIC_PS_COMBO( 256, 2 ) != 0;
	const bool SECONDARY_NORMAL = STATIC_PS_COMBO( 512, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 2, 2 );

	vec3 result;
	float pixelFogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );

	float blend = 1.0;
	if ( FADEOUTONSILHOUETTE )
	{
		blend = saturate( dot( -vWorldViewVector.xyz, vWorldNormal.xyz ) );
		blend = blend * blend * blend;
	}

	// Decompress normal
	vec4 vNormal = DecompressNormal( 2, NormalSampler, vBumpTexCoord.xy );
	if ( SECONDARY_NORMAL )
	{
		vec3 vNormal2 = DecompressNormal( 0, NormalSampler2, vBumpTexCoord.wz ).xyz;
		vNormal.xyz = normalize( vNormal.xyz + vNormal2.xyz );
	}

	vec3 refractTintColor = g_RefractTint;
	if ( REFRACTTINTTEXTURE )
		refractTintColor = 2.0 * g_RefractTint * tex2D( 4, RefractTintSampler, vBumpTexCoord.xy ).rgb;
	if ( COLORMODULATE )
		refractTintColor *= ColorModulate.rgb;

	// Perform division by W only once
	float ooW = 1.0 / vRefractXYW.z;

	// Compute coordinates for sampling refraction
	vec2 vRefractTexCoordNoWarp = vRefractXYW.xy * ooW;
	vec2 vRefractTexCoord = vNormal.xy;
	float scale = vNormal.a * g_RefractScale;
	if ( COLORMODULATE )
		scale *= ColorModulate.a;
	vRefractTexCoord *= scale;
	vRefractTexCoord += vRefractTexCoordNoWarp;

	if ( BLUR == 1 )
	{
		// Nine taps as four bilinear ones (the polyphase kernel).
		vec2 upper_2x2_loc = vRefractTexCoord.xy - vec2( g_HalfBlurFraction, g_HalfBlurFraction );
		vec2 right_1x2_loc = vRefractTexCoord.xy + vec2( g_BlurFraction, -g_HalfBlurFraction );
		vec2 lower_2x1_loc = vRefractTexCoord.xy + vec2( -g_HalfBlurFraction, g_BlurFraction );
		vec2 singleton_loc = vRefractTexCoord.xy + vec2( g_BlurFraction, g_BlurFraction );
		result = tex2D( 1, RefractSampler, upper_2x2_loc ).rgb * 0.4444444;
		result += tex2D( 1, RefractSampler, right_1x2_loc ).rgb * 0.2222222;
		result += tex2D( 1, RefractSampler, lower_2x1_loc ).rgb * 0.2222222;
		result += tex2D( 1, RefractSampler, singleton_loc ).rgb * 0.1111111;
		vec3 unblurredColor = tex2D( 1, RefractSampler, vRefractTexCoordNoWarp.xy ).rgb;
		result = mix( unblurredColor, result * refractTintColor, blend );
	}
	else if ( MASKED )
	{
		vec4 fMaskedResult = tex2D( 1, RefractSampler, vRefractTexCoord.xy );
		LegacyWrite( FinalOutput( fMaskedResult, pixelFogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE ) );
		return;
	}
	else
	{
		vec3 colorWarp = tex2D( 1, RefractSampler, vRefractTexCoord.xy ).rgb;
		vec3 colorNoWarp = tex2D( 1, RefractSampler, vRefractTexCoordNoWarp.xy ).rgb;
		colorWarp *= refractTintColor;
		result = mix( colorNoWarp, colorWarp, blend );
	}

	if ( CUBEMAP )
	{
		float specularFactor = vNormal.a;
		vec3 worldSpaceNormal =
		    Vec3TangentToWorld( vNormal.xyz, vWorldNormal, vWorldTangent, vWorldBinormal );
		vec3 reflectVect = CalcReflectionVectorUnnormalized( worldSpaceNormal, vTangentVertToEyeVector );
		vec3 specularLighting = texCUBE( 3, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;
		specularLighting *= g_EnvmapTint;
		vec3 specularLightingSquared = specularLighting * specularLighting;
		specularLighting = mix( specularLighting, specularLightingSquared, g_EnvmapContrast );
		vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
		specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
		result += specularLighting;
	}

	float resultAlpha = COLORMODULATE ? ColorModulate.a * vNormal.a : vNormal.a;
	LegacyWrite( FinalOutput( vec4( result, resultAlpha ), pixelFogFactor, PIXELFOGTYPE,
	    TONEMAP_SCALE_NONE, WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
