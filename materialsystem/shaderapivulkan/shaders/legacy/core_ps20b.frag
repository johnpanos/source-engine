#version 450
// Core_DX90's pixel stage: a port of stdshaders/core_ps2x.fxc (ps20b). Combos
// (fxctmp9/core_ps20b.inc): static CONVERT_TO_SRGB (2, always 0 here),
// CUBEMAP (4), FLOWMAP (8), CORECOLORTEXTURE (16), REFRACT (32); dynamic
// PIXELFOGTYPE (1). Pass 0 refracts the frame copy (or $basetexture), pass 1
// adds the cube map reflection.
// @legacy program=core ps=core_ps20b vs=core_vs20 vert=core_vs20
//         samplers=2:2d,3:2d,4:cube,6:2d,7:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D RefractSampler;   // s2
layout( set = 0, binding = 1 ) uniform sampler2D NormalSampler;    // s3
layout( set = 0, binding = 2 ) uniform samplerCube EnvmapSampler;  // s4
layout( set = 0, binding = 3 ) uniform sampler2D FlowmapSampler;   // s6
layout( set = 0, binding = 4 ) uniform sampler2D CoreColorSampler; // s7

layout( location = 0 ) in vec2 vBumpTexCoordIn;
layout( location = 1 ) in vec3 vWorldVertToEyeVector;
layout( location = 2 ) in vec3 tangentSpaceTranspose0;
layout( location = 3 ) in vec3 tangentSpaceTranspose1;
layout( location = 4 ) in vec3 tangentSpaceTranspose2;
layout( location = 5 ) in vec3 vRefractXYW;
layout( location = 6 ) in vec3 projNormal;
layout( location = 7 ) in vec4 worldPos_projPosZ;

#define g_EnvmapTint ( PS_C( 0 ).xyz )
#define g_RefractTint ( PS_C( 1 ).xyz )
#define g_EnvmapContrast ( PS_C( 2 ).xyz )
#define g_EnvmapSaturation ( PS_C( 3 ).xyz )
#define g_RefractScale ( PS_C( 5 ).xy )
#define g_Time ( PS_C( 6 ).x )
#define g_FlowScrollRate ( PS_C( 7 ).xy )
#define g_EyePos ( PS_C( 8 ).xyz )
#define g_CoreColorTexCoordOffset ( PS_C( 9 ).x )
#define g_FogParams PS_C( 11 )

float LengthThroughSphere(
    vec3 vecRayOrigin, vec3 vecRayDelta, vec3 vecSphereCenter, float flRadius, out float alpha )
{
	vec3 vecSphereToRay;
	vecSphereToRay = vecRayOrigin - vecSphereCenter;

	float a = dot( vecRayDelta, vecRayDelta );
	float b = 2.0 * dot( vecSphereToRay, vecRayDelta );
	float c = dot( vecSphereToRay, vecSphereToRay ) - flRadius * flRadius;
	float flDiscrim = b * b - 4.0 * a * c;

	float hack = flDiscrim;
	flDiscrim = sqrt( flDiscrim );
	float oo2a = 0.5 / a;

	// replacing the if's above because if's in hlsl are bad.....
	float fHackGreaterThanZero = step( 0.0, hack );
	alpha = fHackGreaterThanZero;
	return ( fHackGreaterThanZero * ( abs( flDiscrim ) * 2.0 * oo2a ) );
}

// common_fxc.h's CalcReflectionVectorUnnormalized.
vec3 CalcReflectionVectorUnnormalized( vec3 normal, vec3 eyeVector )
{
	return ( 2.0 * ( dot( normal, eyeVector ) ) * normal ) - ( dot( normal, normal ) * eyeVector );
}

void main()
{
	const bool CUBEMAP = STATIC_PS_COMBO( 4, 2 ) != 0;
	const bool FLOWMAP = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool CORECOLORTEXTURE = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool REFRACT = STATIC_PS_COMBO( 32, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	vec3 result = vec3( 0.0 );
	float blend = 1.0;
	vec2 vBumpTexCoord = vBumpTexCoordIn;

	float sphereAlpha = 0.0;
	float normalizedLengthThroughSphere = 0.0;
	if ( FLOWMAP )
	{
		// hack
		vec3 g_SphereCenter = vec3( 2688.0, 12139.0, 5170.0 );
		float g_SphereDiameter = 430.0;

		vec3 tmp = worldPos_projPosZ.xyz - g_SphereCenter;
		float hackRadius = 1.05 * sqrt( dot( tmp, tmp ) );

		float lengthThroughSphere = LengthThroughSphere( g_EyePos,
		    normalize( worldPos_projPosZ.xyz - g_EyePos ), g_SphereCenter, hackRadius, sphereAlpha );

		normalizedLengthThroughSphere = lengthThroughSphere / g_SphereDiameter;

		vec3 hackWorldSpaceNormal = normalize( worldPos_projPosZ.xyz - g_SphereCenter );
		vec3 realFuckingNormal = abs( hackWorldSpaceNormal );
		hackWorldSpaceNormal = 0.5 * ( hackWorldSpaceNormal + 1.0 );

		vBumpTexCoord.xy = realFuckingNormal.z * tex2D( 3, FlowmapSampler, hackWorldSpaceNormal.xy ).xy;
		vBumpTexCoord.xy += realFuckingNormal.y * tex2D( 3, FlowmapSampler, hackWorldSpaceNormal.xz ).xy;
		vBumpTexCoord.xy += realFuckingNormal.x * tex2D( 3, FlowmapSampler, hackWorldSpaceNormal.yz ).xy;
		vBumpTexCoord.xy += g_Time * g_FlowScrollRate;
	}

	// Load normal and expand range
	vec4 vNormalSample = tex2D( 1, NormalSampler, vBumpTexCoord );
	vec3 tangentSpaceNormal = vNormalSample.xyz * 2.0 - 1.0;

	vec3 refractTintColor = g_RefractTint;

	// Perform division by W only once
	float ooW = 1.0 / vRefractXYW.z;

	// Compute coordinates for sampling refraction
	vec2 vRefractTexCoordNoWarp = vRefractXYW.xy * ooW;
	vec2 vRefractTexCoord = tangentSpaceNormal.xy;
	float scale = vNormalSample.a * g_RefractScale.x;
	if ( FLOWMAP )
		scale *= normalizedLengthThroughSphere;
	vRefractTexCoord *= scale;
	vec2 hackOffset = vRefractTexCoord;
	vRefractTexCoord += vRefractTexCoordNoWarp;

	vec3 colorWarp = tex2D( 0, RefractSampler, vRefractTexCoord.xy ).rgb;
	vec3 colorNoWarp = tex2D( 0, RefractSampler, vRefractTexCoordNoWarp.xy ).rgb;

	colorWarp *= refractTintColor;
	if ( REFRACT )
		result = mix( colorNoWarp, colorWarp, blend );

	if ( CUBEMAP )
	{
		float specularFactor = vNormalSample.a;

		// mul( tangentSpaceTranspose, tangentSpaceNormal ) over the column registers.
		vec3 worldSpaceNormal = tangentSpaceNormal.x * tangentSpaceTranspose0 +
		                        tangentSpaceNormal.y * tangentSpaceTranspose1 +
		                        tangentSpaceNormal.z * tangentSpaceTranspose2;

		vec3 reflectVect = CalcReflectionVectorUnnormalized( worldSpaceNormal, vWorldVertToEyeVector );
		vec3 specularLighting = texCUBE( 2, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;
		specularLighting *= g_EnvmapTint;
		vec3 specularLightingSquared = specularLighting * specularLighting;
		specularLighting = mix( specularLighting, specularLightingSquared, g_EnvmapContrast );
		vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
		specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
		result += specularLighting;
	}

	vec4 rgba;
	if ( CORECOLORTEXTURE && FLOWMAP )
	{
		vec4 coreColorTexel = tex2D( 4, CoreColorSampler,
		    hackOffset + vec2( normalizedLengthThroughSphere, g_CoreColorTexCoordOffset ) );
		rgba = vec4( mix( result, coreColorTexel.rgb, coreColorTexel.a ), sphereAlpha );
	}
	else
	{
		rgba = vec4( result, vNormalSample.a );
	}

	float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( rgba, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE ) );
}
