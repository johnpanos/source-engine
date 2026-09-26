#version 450
// WorldTwoTextureBlend's pixel stage: a port of
// stdshaders/worldtwotextureblend_ps2x.fxc (ps20b). Combos
// (fxctmp9/WorldTwoTextureBlend_ps20b.inc): static CONVERT_TO_SRGB (16, always
// 0 here), DETAILTEXTURE (32), BUMPMAP (64), VERTEXCOLOR (128), SELFILLUM
// (256), DIFFUSEBUMPMAP (512), DETAIL_ALPHA_MASK_BASE_TEXTURE (1024),
// FLASHLIGHT (2048), SEAMLESS (4096), FLASHLIGHTDEPTHFILTERMODE (8192);
// dynamic WRITEWATERFOGTODESTALPHA (1), PIXELFOGTYPE (2),
// WRITE_DEPTH_TO_DESTALPHA (4), FLASHLIGHTSHADOWS (8). The flashlight combos
// are not ported: this backend never draws in flashlight mode
// (InFlashlightMode), so FLASHLIGHT, FLASHLIGHTSHADOWS and a nonzero
// FLASHLIGHTDEPTHFILTERMODE are never selected; such a pass is discarded.
// @legacy program=worldtwotextureblend ps=worldtwotextureblend_ps20b
//         vs=lightmappedgeneric_vs20 vert=lightmappedgeneric_vs20
//         samplers=0:2d,1:2d,3:2d,4:2d
#include "legacy_ps.glsl"
#include "legacy_bumpbasis.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D LightmapSampler;    // s1
layout( set = 0, binding = 2 ) uniform sampler2D DetailSampler;      // s3
layout( set = 0, binding = 3 ) uniform sampler2D BumpmapSampler;     // s4

layout( location = 0 ) in vec4 baseTexCoord;
layout( location = 1 ) in vec4 detailOrBumpTexCoord;
layout( location = 2 ) in vec4 lightmapTexCoord1And2;
layout( location = 3 ) in vec4 lightmapTexCoord3;
layout( location = 4 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vertexColor;

#define g_SelfIllumTint PS_C( 7 )
#define g_EyePos PS_C( 10 )
#define g_FogParams PS_C( 11 )
const float g_OverbrightFactor = 2.0;

void main()
{
	const bool bDetailTexture = STATIC_PS_COMBO( 32, 2 ) != 0;
	const bool bBumpmap = STATIC_PS_COMBO( 64, 2 ) != 0;
	const bool bSelfIllum = STATIC_PS_COMBO( 256, 2 ) != 0;
	const bool bDiffuseBumpmap = STATIC_PS_COMBO( 512, 2 ) != 0;
	const bool bDetailAlphaMaskBaseTexture = STATIC_PS_COMBO( 1024, 2 ) != 0;
	const bool bFlashlight = STATIC_PS_COMBO( 2048, 2 ) != 0;
	const int WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 4, 2 );
	if ( bFlashlight )
		discard;

	vec3 lightmapColor1 = vec3( 1.0 );
	vec3 lightmapColor2 = vec3( 1.0 );
	vec3 lightmapColor3 = vec3( 1.0 );
	if ( bBumpmap && bDiffuseBumpmap )
	{
		vec2 bumpCoord1, bumpCoord2, bumpCoord3;
		ComputeBumpedLightmapCoordinates(
		    lightmapTexCoord1And2, lightmapTexCoord3.xy, bumpCoord1, bumpCoord2, bumpCoord3 );
		lightmapColor1 = tex2D( 1, LightmapSampler, bumpCoord1 ).rgb;
		lightmapColor2 = tex2D( 1, LightmapSampler, bumpCoord2 ).rgb;
		lightmapColor3 = tex2D( 1, LightmapSampler, bumpCoord3 ).rgb;
	}
	else
	{
		const vec2 bumpCoord1 =
		    ComputeLightmapCoordinates( lightmapTexCoord1And2, lightmapTexCoord3.xy );
		lightmapColor1 = tex2D( 1, LightmapSampler, bumpCoord1 ).rgb;
	}

	vec4 detailColor = vec4( 1.0 );
	if ( bDetailTexture )
		detailColor = tex2D( 2, DetailSampler, detailOrBumpTexCoord.xy );

	vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord.xy );
	if ( bDetailAlphaMaskBaseTexture )
	{
		// This is what WorldTwoTextureBlend_DX6 does.
		baseColor.rgb = saturate(
		    saturate( baseColor.rgb * 2.0 ) * detailColor.a + ( 1.0 - detailColor.a ) );
		baseColor.rgb *= detailColor.rgb;
	}
	else
	{
		baseColor.rgb = mix( baseColor.rgb, detailColor.rgb, detailColor.a );
	}

	vec3 normal = vec3( 0.0, 0.0, 1.0 );
	if ( bBumpmap )
		normal = 2.0 * tex2D( 3, BumpmapSampler, detailOrBumpTexCoord.xy ).rgb - 1.0;

	vec3 albedo = baseColor.rgb;
	float alpha = 1.0;
	if ( !bSelfIllum )
		alpha *= baseColor.a;

	// The vertex color contains the modulation color + vertex color combined
	albedo *= vertexColor.rgb;
	alpha *= vertexColor.a;

	vec3 diffuseLighting;
	if ( bBumpmap && bDiffuseBumpmap )
	{
		const float dot1 = saturate( dot( normal, bumpBasis[0] ) );
		const float dot2 = saturate( dot( normal, bumpBasis[1] ) );
		const float dot3 = saturate( dot( normal, bumpBasis[2] ) );
		const float sum = dot1 + dot2 + dot3;
		diffuseLighting = dot1 * lightmapColor1 + dot2 * lightmapColor2 + dot3 * lightmapColor3;
		diffuseLighting *= 1.0 / sum;
	}
	else
	{
		diffuseLighting = lightmapColor1;
	}
	diffuseLighting *= g_OverbrightFactor;

	vec3 diffuseComponent = albedo * diffuseLighting;
	if ( bSelfIllum )
	{
		const vec3 selfIllumComponent = g_SelfIllumTint.rgb * albedo;
		diffuseComponent = mix( diffuseComponent, selfIllumComponent, baseColor.a );
	}
	const vec3 result = diffuseComponent;

	const float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );
	if ( WRITEWATERFOGTODESTALPHA != 0 && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		alpha = fogFactor;

	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
