#version 450
// LightmappedReflective_DX90's pixel stage: a port of
// stdshaders/lightmappedreflective_ps2x.fxc (ps20b). The bump-perturbed
// reflection and refraction textures blended by a Fresnel term, plus the base
// texture times the bumped lightmaps, masked by the envmap mask. Combos
// (fxctmp9/lightmappedreflective_ps20b.inc): static CONVERT_TO_SRGB (4, always
// 0 here), BASETEXTURE (8), REFLECT (16), REFRACT (32), ENVMAPMASK (64);
// dynamic PIXELFOGTYPE (1), WRITE_DEPTH_TO_DESTALPHA (2).
// @legacy program=lightmappedreflective ps=lightmappedreflective_ps20b
//         vs=lightmappedreflective_vs20 vert=lightmappedreflective_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d,4:2d,6:2d
#include "legacy_ps.glsl"
#include "legacy_bumpbasis.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D RefractSampler;     // s0
layout( set = 0, binding = 1 ) uniform sampler2D BaseTextureSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D ReflectSampler;     // s2
layout( set = 0, binding = 3 ) uniform sampler2D LightmapSampler;    // s3
layout( set = 0, binding = 4 ) uniform sampler2D NormalSampler;      // s4
layout( set = 0, binding = 5 ) uniform sampler2D EnvMapMaskSampler;  // s6

layout( location = 0 ) in vec4 vBumpTexCoordXY_vTexCoordXY;
layout( location = 1 ) in vec3 vTangentEyeVect;
layout( location = 2 ) in vec4 vReflectXY_vRefractYX;
layout( location = 3 ) in float W;
layout( location = 4 ) in vec4 vProjPos;
layout( location = 6 ) in vec4 lightmapTexCoord1And2;
layout( location = 7 ) in vec4 lightmapTexCoord3;

#define vRefractTint PS_C( 1 )
#define g_FresnelConstants PS_C( 3 )
#define vReflectTint PS_C( 4 )
#define g_ReflectRefractScale PS_C( 5 ) // xy - reflect scale, zw - refract scale
#define g_PixelFogParams PS_C( 8 )

void main()
{
	const bool BASETEXTURE = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool g_bReflect = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool g_bRefract = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool ENVMAPMASK = STATIC_PS_COMBO( 64, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 2, 2 );

	// Load normal and expand range; the normalize is of all four components.
	const vec4 vNormalSample = tex2D( 4, NormalSampler, vBumpTexCoordXY_vTexCoordXY.xy );
	const vec3 vNormal = normalize( vNormalSample * 2.0 - 1.0 ).xyz;

	// Perform division by W only once
	const float ooW = 1.0 / W;

	// vectorize the dependent UV calculations (reflect = .xy, refract = .wz)
	const vec4 vN = vec4( vNormal.xy, vNormal.y, vNormal.x );
	vec4 vDependentTexCoords = vN * vNormalSample.a * g_ReflectRefractScale;
	vDependentTexCoords += ( vReflectXY_vRefractYX * ooW );
	const vec2 vReflectTexCoord = vDependentTexCoords.xy;
	const vec2 vRefractTexCoord = vDependentTexCoords.wz;

	// Sample reflection and refraction
	vec4 vReflectColor = tex2D( 2, ReflectSampler, vReflectTexCoord );
	vec4 vRefractColor = tex2D( 0, RefractSampler, vRefractTexCoord );
	vReflectColor *= vReflectTint;
	vRefractColor *= vRefractTint;

	const vec3 vEyeVect = normalize( vTangentEyeVect );

	// Fresnel term
	const float fNdotV = saturate( dot( vEyeVect, vNormal ) );
	const float fFresnelScalar =
	    g_FresnelConstants.x * pow( 1.0 - fNdotV, g_FresnelConstants.y ) + g_FresnelConstants.z;
	const vec4 fFresnel = vec4( fFresnelScalar );

	vec4 baseSample = vec4( 0.0 );
	vec3 diffuseComponent = vec3( 0.0 );
	if ( BASETEXTURE )
	{
		baseSample = tex2D( 1, BaseTextureSampler, vBumpTexCoordXY_vTexCoordXY.zw );
		vec2 bumpCoord1, bumpCoord2, bumpCoord3;
		ComputeBumpedLightmapCoordinates(
		    lightmapTexCoord1And2, lightmapTexCoord3.xy, bumpCoord1, bumpCoord2, bumpCoord3 );
		const vec3 lightmapColor1 = tex2D( 3, LightmapSampler, bumpCoord1 ).rgb;
		const vec3 lightmapColor2 = tex2D( 3, LightmapSampler, bumpCoord2 ).rgb;
		const vec3 lightmapColor3 = tex2D( 3, LightmapSampler, bumpCoord3 ).rgb;

		vec3 dp;
		dp.x = saturate( dot( vNormal, bumpBasis[0] ) );
		dp.y = saturate( dot( vNormal, bumpBasis[1] ) );
		dp.z = saturate( dot( vNormal, bumpBasis[2] ) );
		dp *= dp;

		vec3 diffuseLighting =
		    dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
		const float sum = dot( dp, vec3( 1.0 ) );
		diffuseLighting *= LIGHT_MAP_SCALE / sum;
		diffuseComponent = baseSample.rgb * diffuseLighting;
	}

	vec4 flMask = vec4( 1.0 );
	if ( ENVMAPMASK )
		flMask = tex2D( 5, EnvMapMaskSampler, vBumpTexCoordXY_vTexCoordXY.zw );

	vec4 result;
	float flAlpha = 1.0;
	if ( g_bReflect && g_bRefract )
	{
		result = mix( vRefractColor, vReflectColor, fFresnel ) * flMask;
		if ( BASETEXTURE )
		{
			result += vec4( diffuseComponent, 1.0 );
			flAlpha = baseSample.a;
		}
	}
	else if ( g_bReflect )
	{
		if ( BASETEXTURE )
		{
			result = vec4( diffuseComponent, 1.0 ) + vReflectColor * flMask;
			flAlpha = baseSample.a;
		}
		else
		{
			result = vReflectColor;
		}
	}
	else if ( g_bRefract )
	{
		if ( BASETEXTURE )
		{
			result = vec4( diffuseComponent, 1.0 ) + vRefractColor * flMask;
			flAlpha = baseSample.a;
		}
		else
		{
			result = vRefractColor;
		}
	}
	else
	{
		if ( BASETEXTURE )
		{
			result = vec4( diffuseComponent, 1.0 );
			flAlpha = baseSample.a;
		}
		else
		{
			result = vec4( 0.0 );
		}
	}

	float fogFactor = 0.0;
	if ( PIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE )
		fogFactor =
		    CalcRangeFog( vProjPos.z, g_PixelFogParams.x, g_PixelFogParams.z, g_PixelFogParams.w );

	LegacyWrite( FinalOutput( vec4( result.rgb, flAlpha ), fogFactor, PIXELFOGTYPE,
	    TONEMAP_SCALE_NONE, WRITE_DEPTH_TO_DESTALPHA != 0, vProjPos.z ) );
}
