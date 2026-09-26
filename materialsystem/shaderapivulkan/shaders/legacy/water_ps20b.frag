#version 450
// Water_DX90's expensive pixel stage: a port of stdshaders/water_ps2x.fxc and
// water_ps2x_helper.h's DrawWater (ps20b). The bump-perturbed reflection and
// refraction (optionally a 5x5 blurred refraction) blended by a Fresnel term,
// the refraction fogged toward the water fog color by the refraction's depth
// alpha (above water) or the projected depth (below), plus the base texture
// times the bumped lightmaps. Combos (fxctmp9/water_ps20b.inc): static
// CONVERT_TO_SRGB (4, always 0 here), BASETEXTURE (8), MULTITEXTURE (16),
// REFLECT (32), REFRACT (64), ABOVEWATER (128), BLURRY_REFRACT (256),
// NORMAL_DECODE_MODE (512, always 0); dynamic PIXELFOGTYPE (1),
// WRITE_DEPTH_TO_DESTALPHA (2).
// @legacy program=water ps=water_ps20b vs=water_vs20 vert=water_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d,4:2d
#include "legacy_ps.glsl"
#include "legacy_bumpbasis.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D RefractSampler;     // s0
layout( set = 0, binding = 1 ) uniform sampler2D BaseTextureSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D ReflectSampler;     // s2
layout( set = 0, binding = 3 ) uniform sampler2D LightmapSampler;    // s3
layout( set = 0, binding = 4 ) uniform sampler2D NormalSampler;      // s4

layout( location = 0 ) in vec2 vBumpTexCoord;
layout( location = 1 ) in vec3 vTangentEyeVect;
layout( location = 2 ) in vec4 vReflectXY_vRefractYX;
layout( location = 3 ) in float W;
layout( location = 4 ) in vec4 vProjPos;
layout( location = 6 ) in vec4 vExtraBumpTexCoord_lightmapTexCoord1And2;
layout( location = 7 ) in vec4 lightmapTexCoord3;

#define vRefractTint PS_C( 1 )
#define vReflectTint PS_C( 4 )
#define g_ReflectRefractScale PS_C( 5 ) // xy - reflect scale, zw - refract scale
#define g_WaterFogColor PS_C( 6 )
#define g_WaterFogParams PS_C( 7 )
#define g_PixelFogParams PS_C( 8 )
#define g_WaterFogStart ( g_WaterFogParams.x )
#define g_WaterFogEndMinusStart ( g_WaterFogParams.y )
#define g_Reflect_OverBright ( g_WaterFogParams.z )

void main()
{
	const bool BASETEXTURE = STATIC_PS_COMBO( 8, 2 ) != 0;
	const bool MULTITEXTURE = STATIC_PS_COMBO( 16, 2 ) != 0;
	const bool bReflect = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool bRefract = STATIC_PS_COMBO( 64, 2 ) != 0;
	const bool ABOVEWATER = STATIC_PS_COMBO( 128, 2 ) != 0;
	const bool BLURRY_REFRACT = STATIC_PS_COMBO( 256, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 2, 2 );

	vec4 vNormal;
	if ( MULTITEXTURE )
	{
		const vec4 vExtraBumpTexCoord = vExtraBumpTexCoord_lightmapTexCoord1And2;
		vNormal = tex2D( 4, NormalSampler, vBumpTexCoord );
		const vec4 vNormal1 = tex2D( 4, NormalSampler, vExtraBumpTexCoord.xy );
		const vec4 vNormal2 = tex2D( 4, NormalSampler, vExtraBumpTexCoord.zw );
		vNormal = 0.33 * ( vNormal + vNormal1 + vNormal2 );
		vNormal.xyz = 2.0 * vNormal.xyz - 1.0;
	}
	else
	{
		vNormal = DecompressNormal( 4, NormalSampler, vBumpTexCoord );
	}

	// Perform division by W only once
	const float ooW = 1.0 / W;

	const vec2 unwarpedRefractTexCoord = vReflectXY_vRefractYX.wz * ooW;

	// We don't actually have valid depth values in alpha when we are underwater
	// looking out, so just set to farthest value.
	float waterFogDepthValue = 1.0;
	if ( ABOVEWATER )
		waterFogDepthValue = tex2D( 0, RefractSampler, unwarpedRefractTexCoord ).a;

	vec4 reflectRefractScale = g_ReflectRefractScale;
	if ( !BASETEXTURE && !BLURRY_REFRACT )
		reflectRefractScale *= waterFogDepthValue;

	// vectorize the dependent UV calculations (reflect = .xy, refract = .wz)
	const vec4 vN = vec4( vNormal.xy, vNormal.y, vNormal.x );
	vec4 vDependentTexCoords = vN * vNormal.a * reflectRefractScale;
	vDependentTexCoords += ( vReflectXY_vRefractYX * ooW );
	const vec2 vReflectTexCoord = vDependentTexCoords.xy;
	const vec2 vRefractTexCoord = vDependentTexCoords.wz;

	vec4 vReflectColor = tex2D( 2, ReflectSampler, vReflectTexCoord );
	vec4 vRefractColor;
	if ( BLURRY_REFRACT )
	{
		// Sample reflection and refraction
		const vec2 ddx1 = vec2( 0.005, 0.0 );
		const vec2 ddy1 = vec2( 0.0, 0.005 );
		vRefractColor = vec4( 0.0 );
		// genwaterloop.pl's unrolled 5x5 box, x outer.
		for ( int ix = -2; ix <= 2; ix++ )
		{
			for ( int iy = -2; iy <= 2; iy++ )
			{
				vRefractColor += tex2D( 0, RefractSampler,
				    vRefractTexCoord + float( ix ) * ddx1 + float( iy ) * ddy1 );
			}
		}
		const float sumweights = 25.0;
		vRefractColor *= ( 1.0 / sumweights );
		vReflectColor *= g_Reflect_OverBright;
		vReflectColor *= vReflectTint;
		vRefractColor *= vRefractTint;
		// Don't mess with this in the underwater case since we don't really
		// have depth values there. Get the blurred depth value for fog.
		if ( ABOVEWATER )
			waterFogDepthValue = vRefractColor.a;
	}
	else
	{
		vReflectColor *= vReflectTint;
		vRefractColor = tex2D( 0, RefractSampler, vRefractTexCoord );
		// get the depth value from the refracted sample to be used for fog.
		if ( ABOVEWATER )
			waterFogDepthValue = tex2D( 0, RefractSampler, vRefractTexCoord ).a;
	}

	const vec3 vEyeVect = normalize( vTangentEyeVect );

	// Fresnel term
	const float fNdotV = saturate( dot( vEyeVect, vNormal.xyz ) );
	float fFresnel = pow( 1.0 - fNdotV, 5.0 );

	// fFresnel == 1.0f means full reflection
	if ( !BASETEXTURE )
		fFresnel *= saturate( ( waterFogDepthValue - 0.05 ) * 20.0 );

	// blend between refraction and fog color.
	if ( ABOVEWATER )
	{
		vRefractColor = mix( vRefractColor, g_WaterFogColor * LINEAR_LIGHT_SCALE,
		    saturate( waterFogDepthValue - 0.05 ) );
	}
	else
	{
		const float waterFogFactor =
		    saturate( ( vProjPos.z - g_WaterFogStart ) / g_WaterFogEndMinusStart );
		vRefractColor = mix( vRefractColor, g_WaterFogColor * LINEAR_LIGHT_SCALE, waterFogFactor );
	}

	vec4 baseSample = vec4( 0.0 );
	vec3 diffuseComponent = vec3( 0.0 );
	if ( BASETEXTURE )
	{
		baseSample = tex2D( 1, BaseTextureSampler, vBumpTexCoord.xy );
		vec2 bumpCoord1, bumpCoord2, bumpCoord3;
		ComputeBumpedLightmapCoordinates( vExtraBumpTexCoord_lightmapTexCoord1And2,
		    lightmapTexCoord3.xy, bumpCoord1, bumpCoord2, bumpCoord3 );
		const vec3 lightmapColor1 = tex2D( 3, LightmapSampler, bumpCoord1 ).rgb;
		const vec3 lightmapColor2 = tex2D( 3, LightmapSampler, bumpCoord2 ).rgb;
		const vec3 lightmapColor3 = tex2D( 3, LightmapSampler, bumpCoord3 ).rgb;

		vec3 dp;
		dp.x = saturate( dot( vNormal.xyz, bumpBasis[0] ) );
		dp.y = saturate( dot( vNormal.xyz, bumpBasis[1] ) );
		dp.z = saturate( dot( vNormal.xyz, bumpBasis[2] ) );
		dp *= dp;

		vec3 diffuseLighting = dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
		const float sum = dot( dp, vec3( 1.0 ) );
		diffuseLighting *= LIGHT_MAP_SCALE / sum;
		diffuseComponent = baseSample.rgb * diffuseLighting;
	}

	vec4 result;
	if ( bReflect && bRefract )
	{
		result = mix( vRefractColor, vReflectColor, fFresnel );
	}
	else if ( bReflect )
	{
		if ( BASETEXTURE )
			result = vec4( diffuseComponent, 1.0 ) + vReflectColor * fFresnel * baseSample.a;
		else
			result = vReflectColor;
	}
	else if ( bRefract )
	{
		result = vRefractColor;
	}
	else
	{
		result = vec4( 0.0 );
	}

	float fogFactor = 0.0;
	if ( PIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE )
		fogFactor =
		    CalcRangeFog( vProjPos.z, g_PixelFogParams.x, g_PixelFogParams.z, g_PixelFogParams.w );

	LegacyWrite( FinalOutput( vec4( result.rgb, 1.0 ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE,
	    WRITE_DEPTH_TO_DESTALPHA != 0, vProjPos.z ) );
}
