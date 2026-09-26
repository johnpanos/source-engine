#version 450
// Water_DX90's cheap pixel stage: a port of stdshaders/watercheap_ps2x.fxc
// (ps20b). The environment cube map reflected about the bumped world normal,
// tinted, with a Fresnel (or constant) factor: blended by the Fresnel and
// distance factor (faded at the shore by the refraction's depth alpha), or
// opaque over the water fog color. Combos (fxctmp9/WaterCheap_ps20b.inc):
// static CONVERT_TO_SRGB (4, always 0 here), MULTITEXTURE (8), FRESNEL (16),
// BLEND (32), REFRACTALPHA (64), HDRTYPE (128), NORMAL_DECODE_MODE (384,
// always 0); dynamic HDRENABLED (1), PIXELFOGTYPE (2). The opaque combos
// normalize the eye vector through the normalization cube map (s6).
// @legacy program=watercheap ps=watercheap_ps20b vs=watercheap_vs20 vert=watercheap_vs20
//         samplers=0:cube,1:2d,2:2d,6:cube
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform samplerCube EnvmapSampler;   // s0
layout( set = 0, binding = 1 ) uniform sampler2D NormalMapSampler;  // s1
layout( set = 0, binding = 2 ) uniform sampler2D RefractSampler;    // s2
layout( set = 0, binding = 3 ) uniform samplerCube NormalizeSampler; // s6

layout( location = 0 ) in vec2 normalMapTexCoord;
layout( location = 1 ) in vec3 worldSpaceEyeVect;
layout( location = 2 ) in vec3 tangentSpaceTranspose0;
layout( location = 3 ) in vec3 tangentSpaceTranspose1;
layout( location = 4 ) in vec3 tangentSpaceTranspose2;
layout( location = 5 ) in vec4 vRefract_W_ProjZ;
layout( location = 6 ) in vec4 vExtraBumpTexCoord;

#define g_WaterFogColor PS_C( 0 )
#define g_CheapWaterParams PS_C( 1 )
#define g_ReflectTint PS_C( 2 )
#define g_PixelFogParams PS_C( 3 )
#define g_CheapWaterStart ( g_CheapWaterParams.x )
#define g_CheapWaterEnd ( g_CheapWaterParams.y )
#define g_CheapWaterDeltaRecip ( g_CheapWaterParams.z )
#define g_CheapWaterStartDivDelta ( g_CheapWaterParams.w )

void main()
{
	const bool MULTITEXTURE = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool FRESNEL = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool bBlend = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool REFRACTALPHA = STATIC_PS_COMBO( 64, 2 ) != 0;
	const int HDRTYPE = STATIC_PS_COMBO( 128, 3 );
	const bool HDRENABLED = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );

	vec3 vNormal;
	if ( MULTITEXTURE )
	{
		vNormal = tex2D( 1, NormalMapSampler, normalMapTexCoord ).xyz;
		const vec3 vNormal1 = tex2D( 1, NormalMapSampler, vExtraBumpTexCoord.xy ).xyz;
		const vec3 vNormal2 = tex2D( 1, NormalMapSampler, vExtraBumpTexCoord.zw ).xyz;
		vNormal = 0.33 * ( vNormal + vNormal1 + vNormal2 );
		vNormal = 2.0 * vNormal - 1.0;
	}
	else
	{
		vNormal = DecompressNormal( 1, NormalMapSampler, normalMapTexCoord ).xyz;
	}

	// mul( vNormal, tangentSpaceTranspose )
	const vec3 worldSpaceNormal = vNormal.x * tangentSpaceTranspose0 +
	                              vNormal.y * tangentSpaceTranspose1 +
	                              vNormal.z * tangentSpaceTranspose2;
	vec3 worldSpaceEye;
	float flWorldSpaceDist = 1.0;
	if ( bBlend )
	{
		worldSpaceEye = worldSpaceEyeVect;
		flWorldSpaceDist = length( worldSpaceEye );
		worldSpaceEye /= flWorldSpaceDist;
	}
	else
	{
		worldSpaceEye = texCUBE( 3, NormalizeSampler, worldSpaceEyeVect ).xyz; // NormalizeWithCubemap
	}

	// CalcReflectionVectorUnnormalized
	const vec3 reflectVect = 2.0 * dot( worldSpaceNormal, worldSpaceEye ) * worldSpaceNormal -
	                         dot( worldSpaceNormal, worldSpaceNormal ) * worldSpaceEye;
	vec3 specularLighting = ENV_MAP_SCALE * texCUBE( 0, EnvmapSampler, reflectVect ).xyz;
	specularLighting *= g_ReflectTint.rgb;

	float flFresnelFactor;
	if ( FRESNEL )
	{
		float flDotResult = dot( worldSpaceEye, worldSpaceNormal );
		flDotResult = 1.0 - max( 0.0, flDotResult );
		flFresnelFactor = flDotResult * flDotResult;
		flFresnelFactor *= flFresnelFactor;
		flFresnelFactor *= flDotResult;
	}
	else
	{
		flFresnelFactor = g_ReflectTint.a;
	}

	float flAlpha;
	if ( bBlend )
	{
		const float flReflectAmount =
		    saturate( flWorldSpaceDist * g_CheapWaterDeltaRecip - g_CheapWaterStartDivDelta );
		flAlpha = saturate( flFresnelFactor + flReflectAmount );
		if ( REFRACTALPHA )
		{
			// Perform division by W only once
			const float ooW = 1.0 / vRefract_W_ProjZ.z;
			const vec2 unwarpedRefractTexCoord = vRefract_W_ProjZ.xy * ooW;
			const float fogDepthValue = tex2D( 2, RefractSampler, unwarpedRefractTexCoord ).a;
			// Fade on the border between the water and land.
			flAlpha *= saturate( ( fogDepthValue - 0.05 ) * 20.0 );
		}
	}
	else
	{
		flAlpha = 1.0;
		if ( HDRTYPE == 0 || !HDRENABLED )
			specularLighting = mix( g_WaterFogColor.rgb, specularLighting, flFresnelFactor );
		else
			specularLighting =
			    mix( pow( g_WaterFogColor.rgb, vec3( 2.2 ) ), specularLighting, flFresnelFactor );
	}

	float fogFactor = 0.0;
	if ( PIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE )
		fogFactor = CalcRangeFog(
		    vRefract_W_ProjZ.w, g_PixelFogParams.x, g_PixelFogParams.z, g_PixelFogParams.w );

	LegacyWrite(
	    FinalOutput( vec4( specularLighting, flAlpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
