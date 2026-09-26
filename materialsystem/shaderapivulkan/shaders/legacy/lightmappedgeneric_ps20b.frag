#version 450
// LightmappedGeneric's pixel stage (also WorldVertexTransition's, with
// BASETEXTURE2): a port of stdshaders/lightmappedgeneric_ps2x.fxc (ps20b), the
// body in lightmappedgeneric_ps2_3_x.h with common_lightmappedgeneric_fxc.h.
// Combos (fxctmp9/lightmappedgeneric_ps20b.inc): static MASKEDBLENDING (96),
// BASETEXTURE2 (192), DETAILTEXTURE (384), BUMPMAP (768, 0..2; 2 is ssbump),
// BUMPMAP2 (2304), CUBEMAP (4608), ENVMAPMASK (9216), BASEALPHAENVMAPMASK
// (18432), SELFILLUM (36864), NORMALMAPALPHAENVMAPMASK (73728), DIFFUSEBUMPMAP
// (147456), BASETEXTURENOENVMAP (294912), BASETEXTURE2NOENVMAP (589824),
// WARPLIGHTING (1179648), FANCY_BLENDING (2359296), RELIEF_MAPPING (always 0)
// and SEAMLESS (4718592), OUTLINE (9437184), SOFTEDGES (18874368), BUMPMASK
// (37748736), NORMAL_DECODE_MODE and NORMALMASK_DECODE_MODE (always 0, NONE)
// and DETAIL_BLEND_MODE (75497472, 0..11); dynamic FASTPATHENVMAPCONTRAST (1),
// FASTPATH (2), WRITEWATERFOGTODESTALPHA (4), PIXELFOGTYPE (8),
// LIGHTING_PREVIEW (16, 0..2) and WRITE_DEPTH_TO_DESTALPHA (48). LIGHTING_PREVIEW
// is Hammer's lighting preview; its mode 2 writes four render targets and this
// stage draws the color target only. FLASHLIGHT is an Xbox combo.
//
// The samplers the material may read as sRGB (s0, s1, s2, s7, s12) take the
// first slots; the linear ones follow. Slot 8 (the warp texture) is read with
// texture(): the sRGB decode bits of the push block cover slots 0..7 only, and
// LightmappedGeneric never reads s6 as sRGB.
// @legacy program=lightmappedgeneric ps=lightmappedgeneric_ps20b
//         vs=lightmappedgeneric_vs20 vert=lightmappedgeneric_vs20
//         samplers=0:2d,1:2d,2:cube,7:2d,12:2d,3:2d,4:2d,5:2d,6:2d,8:2d
#include "legacy_ps.glsl"
#include "legacy_bumpbasis.glsl"
#include "legacy_combine.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;     // s0
layout( set = 0, binding = 1 ) uniform sampler2D LightmapSampler;        // s1
layout( set = 0, binding = 2 ) uniform samplerCube EnvmapSampler;        // s2
layout( set = 0, binding = 3 ) uniform sampler2D BaseTextureSampler2;    // s7
layout( set = 0, binding = 4 ) uniform sampler2D DetailSampler;          // s12
layout( set = 0, binding = 5 ) uniform sampler2D BlendModulationSampler; // s3
layout( set = 0, binding = 6 ) uniform sampler2D BumpmapSampler;         // s4
layout( set = 0, binding = 7 ) uniform sampler2D BumpmapSampler2;        // s5 (or EnvmapMaskSampler)
layout( set = 0, binding = 8 ) uniform sampler2D WarpLightingSampler;    // s6
layout( set = 0, binding = 9 ) uniform sampler2D BumpMaskSampler;        // s8

layout( location = 0 ) in vec4 baseTexCoord; // xy, or SEAMLESS's SeamlessTexCoord xyz
layout( location = 1 ) in vec4 detailOrBumpAndEnvmapMaskTexCoord;
layout( location = 2 ) in vec4 lightmapTexCoord1And2;
layout( location = 3 ) in vec4 lightmapTexCoord3;
layout( location = 4 ) in vec4 worldPos_projPosZ;
layout( location = 5 ) in vec3 tangentSpaceTranspose0;
layout( location = 6 ) in vec3 tangentSpaceTranspose1;
layout( location = 7 ) in vec3 tangentSpaceTranspose2;
layout( location = 8 ) in vec4 vertexColor;
layout( location = 9 ) in vec4 vertexBlendX_fogFactorW;

#define g_EnvmapTint PS_C( 0 )
#define g_OutlineParams PS_C( 2 )
#define OUTLINE_MIN_VALUE0 ( g_OutlineParams.x )
#define OUTLINE_MIN_VALUE1 ( g_OutlineParams.y )
#define OUTLINE_MAX_VALUE0 ( g_OutlineParams.z )
#define OUTLINE_MAX_VALUE1 ( g_OutlineParams.w )
#define OUTLINE_COLOR PS_C( 3 )
#define g_EdgeSoftnessParms PS_C( 4 )
#define SOFT_MASK_MIN ( g_EdgeSoftnessParms.x )
#define SOFT_MASK_MAX ( g_EdgeSoftnessParms.y )
#define g_EnvmapContrastReg PS_C( 2 )
#define g_EnvmapSaturationReg PS_C( 3 )
#define g_FresnelReflectionReg PS_C( 4 )
#define g_SelfIllumTintReg PS_C( 7 )
#define g_DetailTint_and_BlendFactor PS_C( 8 )
#define g_DetailTint ( g_DetailTint_and_BlendFactor.rgb )
#define g_DetailBlendFactor ( g_DetailTint_and_BlendFactor.w )
#define g_EyePos PS_C( 10 )
#define g_FogParams PS_C( 11 )
#define g_TintValuesAndLightmapScale PS_C( 12 )
#define g_flAlpha2 ( g_TintValuesAndLightmapScale.w )

// HLSL's smoothstep as fxc compiles it: t = saturate( ( x - a ) * rcp( b - a ) ),
// t * t * ( 3 - 2 t ), defined for any edge order (OUTLINE and SOFTEDGES pass
// them descending).
float HlslSmoothstep( float a, float b, float x )
{
	const float t = saturate( ( x - a ) * ( 1.0 / ( b - a ) ) );
	return t * t * ( 3.0 - 2.0 * t );
}

// common_lightmappedgeneric_fxc.h's GetBaseTextureAndNormal: with SEAMLESS the
// three planar projections weighted by the vertex color, else one read.
void GetBaseTextureAndNormal( bool SEAMLESS, bool bBase2, bool bBump, vec3 coords, vec3 vWeights,
    out vec4 vResultBase, out vec4 vResultBase2, out vec4 vResultBump )
{
	vResultBase = vec4( 0.0 );
	vResultBase2 = vec4( 0.0 );
	vResultBump = vec4( 0.0 );
	if ( !bBump )
		vResultBump = vec4( 0.0, 0.0, 1.0, 1.0 );

	if ( SEAMLESS )
	{
		vResultBase += vWeights.x * tex2D( 0, BaseTextureSampler, coords.zy );
		if ( bBase2 )
			vResultBase2 += vWeights.x * tex2D( 3, BaseTextureSampler2, coords.zy );
		if ( bBump )
			vResultBump += vWeights.x * tex2D( 6, BumpmapSampler, coords.zy );

		vResultBase += vWeights.y * tex2D( 0, BaseTextureSampler, coords.xz );
		if ( bBase2 )
			vResultBase2 += vWeights.y * tex2D( 3, BaseTextureSampler2, coords.xz );
		if ( bBump )
			vResultBump += vWeights.y * tex2D( 6, BumpmapSampler, coords.xz );

		vResultBase += vWeights.z * tex2D( 0, BaseTextureSampler, coords.xy );
		if ( bBase2 )
			vResultBase2 += vWeights.z * tex2D( 3, BaseTextureSampler2, coords.xy );
		if ( bBump )
			vResultBump += vWeights.z * tex2D( 6, BumpmapSampler, coords.xy );
	}
	else
	{
		vResultBase = tex2D( 0, BaseTextureSampler, coords.xy );
		if ( bBase2 )
			vResultBase2 = tex2D( 3, BaseTextureSampler2, coords.xy );
		if ( bBump )
			vResultBump = tex2D( 6, BumpmapSampler, coords.xy );
	}
}

// mul( float4 vNormal, float3x3 tangentSpaceTranspose ): the vector truncated to
// xyz, the rows S, T, N.
vec3 TangentToWorld( vec4 vNormal )
{
	return vNormal.x * tangentSpaceTranspose0 + vNormal.y * tangentSpaceTranspose1 +
	       vNormal.z * tangentSpaceTranspose2;
}

void main()
{
	const bool MASKEDBLENDING = STATIC_PS_COMBO( 96, 2 ) != 0;
	const bool bBaseTexture2 = STATIC_PS_COMBO( 192, 2 ) != 0;
	const bool bDetailTexture = STATIC_PS_COMBO( 384, 2 ) != 0;
	const int BUMPMAP = STATIC_PS_COMBO( 768, 3 );
	const bool BUMPMAP2 = STATIC_PS_COMBO( 2304, 2 ) != 0;
	const bool bCubemap = STATIC_PS_COMBO( 4608, 2 ) != 0;
	const bool bEnvmapMask = STATIC_PS_COMBO( 9216, 2 ) != 0;
	const bool bBaseAlphaEnvmapMask = STATIC_PS_COMBO( 18432, 2 ) != 0;
	const bool bSelfIllum = STATIC_PS_COMBO( 36864, 2 ) != 0;
	const bool bNormalMapAlphaEnvmapMask = STATIC_PS_COMBO( 73728, 2 ) != 0;
	const bool bDiffuseBumpmap = STATIC_PS_COMBO( 147456, 2 ) != 0;
	const bool bBaseTextureNoEnvmap = STATIC_PS_COMBO( 294912, 2 ) != 0;
	const bool bBaseTexture2NoEnvmap = STATIC_PS_COMBO( 589824, 2 ) != 0;
	const bool WARPLIGHTING = STATIC_PS_COMBO( 1179648, 2 ) != 0;
	const bool FANCY_BLENDING = STATIC_PS_COMBO( 2359296, 2 ) != 0;
	const bool SEAMLESS = STATIC_PS_COMBO( 4718592, 2 ) != 0;
	const bool OUTLINE = STATIC_PS_COMBO( 9437184, 2 ) != 0;
	const bool SOFTEDGES = STATIC_PS_COMBO( 18874368, 2 ) != 0;
	const bool BUMPMASK = STATIC_PS_COMBO( 37748736, 2 ) != 0;
	const int DETAIL_BLEND_MODE = STATIC_PS_COMBO( 75497472, 12 );
	const bool FASTPATHENVMAPCONTRAST = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const bool FASTPATH = DYNAMIC_PS_COMBO( 2, 2 ) != 0;
	const bool WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 4, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 8, 2 );
	const int LIGHTING_PREVIEW = DYNAMIC_PS_COMBO( 16, 3 );
	const bool WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 48, 2 ) != 0;
	const bool bBumpmap = BUMPMAP != 0;

	// USE_FAST_PATH (SEAMLESS forces it): the envmap contrast, saturation,
	// fresnel and self-illumination tint are compile-time constants.
	const bool USE_FAST_PATH = SEAMLESS || FASTPATH;
	vec3 g_EnvmapContrast = vec3( FASTPATHENVMAPCONTRAST ? 1.0 : 0.0 );
	vec3 g_EnvmapSaturation = vec3( 1.0 );
	float g_FresnelReflection = 1.0;
	float g_OneMinusFresnelReflection = 0.0;
	vec4 g_SelfIllumTint = vec4( 1.0 );
	if ( !USE_FAST_PATH )
	{
		g_EnvmapContrast = g_EnvmapContrastReg.rgb;
		g_EnvmapSaturation = g_EnvmapSaturationReg.rgb;
		g_FresnelReflection = g_FresnelReflectionReg.a;
		g_OneMinusFresnelReflection = g_FresnelReflectionReg.b;
		g_SelfIllumTint = g_SelfIllumTintReg;
	}

	vec4 baseColor = vec4( 0.0 );
	vec4 baseColor2 = vec4( 0.0 );
	vec4 vNormal = vec4( 0.0, 0.0, 1.0, 1.0 );
	const vec3 baseTexCoords = SEAMLESS ? baseTexCoord.xyz : vec3( baseTexCoord.xy, 0.0 );

	GetBaseTextureAndNormal( SEAMLESS, bBaseTexture2, bBumpmap || bNormalMapAlphaEnvmapMask,
	    baseTexCoords, vertexColor.rgb, baseColor, baseColor2, vNormal );

	if ( BUMPMAP == 1 ) // not ssbump
		vNormal.xyz = vNormal.xyz * 2.0 - 1.0;

	vec3 lightmapColor1 = vec3( 1.0 );
	vec3 lightmapColor2 = vec3( 1.0 );
	vec3 lightmapColor3 = vec3( 1.0 );
	if ( LIGHTING_PREVIEW == 0 )
	{
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
	}

	vec2 detailTexCoord = vec2( 0.0 );
	vec2 bumpmapTexCoord = detailOrBumpAndEnvmapMaskTexCoord.xy;
	vec2 bumpmap2TexCoord = vec2( 0.0 );
	if ( bDetailTexture )
	{
		detailTexCoord = detailOrBumpAndEnvmapMaskTexCoord.xy;
		bumpmapTexCoord = baseTexCoord.xy;
	}
	else if ( BUMPMASK )
	{
		bumpmap2TexCoord = detailOrBumpAndEnvmapMaskTexCoord.wz;
	}
	const vec2 envmapMaskTexCoord = detailOrBumpAndEnvmapMaskTexCoord.wz;

	vec4 detailColor = vec4( 1.0 );
	if ( bDetailTexture )
		detailColor = vec4( g_DetailTint, 1.0 ) * tex2D( 4, DetailSampler, detailTexCoord );

	if ( OUTLINE || SOFTEDGES )
	{
		const float distAlphaMask = baseColor.a;
		if ( OUTLINE )
		{
			if ( ( distAlphaMask >= OUTLINE_MIN_VALUE0 ) && ( distAlphaMask <= OUTLINE_MAX_VALUE1 ) )
			{
				float oFactor = 1.0;
				if ( distAlphaMask <= OUTLINE_MIN_VALUE1 )
					oFactor = HlslSmoothstep( OUTLINE_MIN_VALUE0, OUTLINE_MIN_VALUE1, distAlphaMask );
				else
					oFactor = HlslSmoothstep( OUTLINE_MAX_VALUE1, OUTLINE_MAX_VALUE0, distAlphaMask );
				baseColor = mix( baseColor, OUTLINE_COLOR, oFactor );
			}
		}
		if ( SOFTEDGES )
			baseColor.a *= HlslSmoothstep( SOFT_MASK_MAX, SOFT_MASK_MIN, distAlphaMask );
		else
			baseColor.a *= distAlphaMask >= 0.5 ? 1.0 : 0.0;
	}

	if ( LIGHTING_PREVIEW == 2 )
		baseColor.xyz = GammaToLinear( baseColor.xyz );

	float blendedAlpha = baseColor.a;

	float blendfactor = MASKEDBLENDING ? 0.5 : vertexBlendX_fogFactorW.r;

	if ( bBaseTexture2 )
	{
		if ( !bSelfIllum && PIXELFOGTYPE != PIXEL_FOG_TYPE_HEIGHT && FANCY_BLENDING )
		{
			const vec4 modt = tex2D( 5, BlendModulationSampler, lightmapTexCoord3.zw );
			if ( MASKEDBLENDING )
			{
				blendfactor = modt.g;
			}
			else
			{
				const float minb = saturate( modt.g - modt.r );
				const float maxb = saturate( modt.g + modt.r );
				blendfactor = HlslSmoothstep( minb, maxb, blendfactor );
			}
		}
		baseColor.rgb = mix( baseColor.rgb, baseColor2.rgb, blendfactor );
		blendedAlpha = mix( baseColor.a, baseColor2.a, blendfactor );
	}

	vec3 specularFactor = vec3( 1.0 );
	vec4 vNormalMask = vec4( 0.0, 0.0, 1.0, 1.0 );
	if ( bBumpmap )
	{
		if ( bBaseTextureNoEnvmap )
			vNormal.a = 0.0;

		if ( BUMPMAP2 )
		{
			const vec2 b2TexCoord = BUMPMASK ? bumpmap2TexCoord : bumpmapTexCoord;

			vec4 vNormal2;
			if ( BUMPMAP == 2 )
				vNormal2 = tex2D( 7, BumpmapSampler2, b2TexCoord );
			else
				vNormal2 = DecompressNormal( 7, BumpmapSampler2, b2TexCoord );

			if ( bBaseTexture2NoEnvmap )
				vNormal2.a = 0.0;

			if ( BUMPMASK )
			{
				const vec3 vNormal1 =
				    DecompressNormal( 6, BumpmapSampler, detailOrBumpAndEnvmapMaskTexCoord.xy ).xyz;
				vNormal.xyz = normalize( vNormal1 + vNormal2.xyz );

				// Third normal map, at the base coordinates.
				vNormalMask = DecompressNormal( 9, BumpMaskSampler, baseTexCoord.xy );

				vNormal.xyz = mix( vNormalMask.xyz, vNormal.xyz, vNormalMask.a );
				specularFactor = vec3( vNormalMask.a );
			}
			else
			{
				if ( FANCY_BLENDING && bNormalMapAlphaEnvmapMask )
					vNormal = mix( vNormal, vNormal2, blendfactor );
				else
					vNormal.xyz = mix( vNormal.xyz, vNormal2.xyz, blendfactor );
			}
		}

		if ( bNormalMapAlphaEnvmapMask )
			specularFactor *= vNormal.a;
	}
	else if ( bNormalMapAlphaEnvmapMask )
	{
		specularFactor *= vNormal.a;
	}

	if ( !BUMPMAP2 && bEnvmapMask )
		specularFactor *= tex2D( 7, BumpmapSampler2, envmapMaskTexCoord ).xyz;

	if ( bBaseAlphaEnvmapMask )
		specularFactor *= 1.0 - blendedAlpha; // Reversing alpha blows!

	vec4 albedo = vec4( 1.0 );
	float alpha = 1.0;
	albedo *= baseColor;
	if ( !bBaseAlphaEnvmapMask && !bSelfIllum )
		alpha *= baseColor.a;

	if ( bDetailTexture )
		albedo = TextureCombine( albedo, detailColor, DETAIL_BLEND_MODE, g_DetailBlendFactor );

	// The vertex color contains the modulation color + vertex color combined
	if ( !SEAMLESS )
		albedo.xyz *= vertexColor.rgb;
	alpha *= vertexColor.a * g_flAlpha2;

	vec3 diffuseLighting;
	if ( bBumpmap && bDiffuseBumpmap )
	{
		if ( BUMPMAP == 2 )
		{
			// ssbump
			diffuseLighting = vNormal.x * lightmapColor1 + vNormal.y * lightmapColor2 +
			                  vNormal.z * lightmapColor3;
			diffuseLighting *= g_TintValuesAndLightmapScale.rgb;

			// The normal for the reflection.
			vNormal.xyz = normalize(
			    bumpBasis[0] * vNormal.x + bumpBasis[1] * vNormal.y + bumpBasis[2] * vNormal.z );
		}
		else
		{
			vec3 dp;
			dp.x = saturate( dot( vNormal.xyz, bumpBasis[0] ) );
			dp.y = saturate( dot( vNormal.xyz, bumpBasis[1] ) );
			dp.z = saturate( dot( vNormal.xyz, bumpBasis[2] ) );
			dp *= dp;

			if ( DETAIL_BLEND_MODE == TCOMBINE_SSBUMP_BUMP )
				dp *= 2.0 * detailColor.rgb;
			diffuseLighting = dp.x * lightmapColor1 + dp.y * lightmapColor2 + dp.z * lightmapColor3;
			const float sum = dot( dp, vec3( 1.0 ) );
			diffuseLighting *= g_TintValuesAndLightmapScale.rgb / sum;
		}
	}
	else
	{
		diffuseLighting = lightmapColor1 * g_TintValuesAndLightmapScale.rgb;
	}

	if ( WARPLIGHTING && !SEAMLESS )
	{
		const float len = 0.5 * length( diffuseLighting );
		diffuseLighting *= 2.0 * texture( WarpLightingSampler, vec2( len, 0.0 ) ).rgb;
	}

	vec3 worldSpaceNormal = vec3( 0.0 );
	if ( bCubemap || LIGHTING_PREVIEW != 0 )
		worldSpaceNormal = TangentToWorld( vNormal );

	vec3 diffuseComponent = albedo.xyz * diffuseLighting;

	if ( bSelfIllum )
	{
		const vec3 selfIllumComponent = g_SelfIllumTint.rgb * albedo.xyz;
		diffuseComponent = mix( diffuseComponent, selfIllumComponent, baseColor.a );
	}

	vec3 specularLighting = vec3( 0.0 );
	if ( bCubemap )
	{
		const vec3 worldVertToEyeVector = g_EyePos.xyz - worldPos_projPosZ.xyz;
		// CalcReflectionVectorUnnormalized.
		const vec3 reflectVect = 2.0 * dot( worldSpaceNormal, worldVertToEyeVector ) * worldSpaceNormal -
		                         dot( worldSpaceNormal, worldSpaceNormal ) * worldVertToEyeVector;

		// Fresnel factor; fxc expands pow( x, 5 ) to products, keeping the sign.
		const vec3 eyeVect = normalize( worldVertToEyeVector );
		float fresnel = 1.0 - dot( worldSpaceNormal, eyeVect );
		const float fresnel2 = fresnel * fresnel;
		fresnel = fresnel * ( fresnel2 * fresnel2 );
		fresnel = fresnel * g_OneMinusFresnelReflection + g_FresnelReflection;

		specularLighting = ENV_MAP_SCALE * texCUBE( 2, EnvmapSampler, reflectVect ).rgb;
		specularLighting *= specularFactor;

		specularLighting *= g_EnvmapTint.rgb;
		if ( !FANCY_BLENDING )
		{
			const vec3 specularLightingSquared = specularLighting * specularLighting;
			specularLighting = mix( specularLighting, specularLightingSquared, g_EnvmapContrast );
			const vec3 greyScale = vec3( dot( specularLighting, vec3( 0.299, 0.587, 0.114 ) ) );
			specularLighting = mix( greyScale, specularLighting, g_EnvmapSaturation );
		}
		specularLighting *= fresnel;
	}

	const vec3 result = diffuseComponent + specularLighting;

	if ( LIGHTING_PREVIEW == 1 )
	{
		const float dotprod =
		    0.7 + 0.25 * dot( worldSpaceNormal, normalize( vec3( 1.0, 2.0, -0.5 ) ) );
		LegacyWrite( FinalOutput(
		    vec4( dotprod * albedo.xyz, alpha ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
		return;
	}
	if ( LIGHTING_PREVIEW == 2 )
	{
		// LPREVIEW_PS_OUT's color target (COLOR0).
		LegacyWrite(
		    FinalOutput( vec4( albedo.xyz, alpha ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
		return;
	}

	const bool bWriteDepthToAlpha = WRITE_DEPTH_TO_DESTALPHA && !WRITEWATERFOGTODESTALPHA;

	const float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos.z, worldPos_projPosZ.z, worldPos_projPosZ.w );

	if ( WRITEWATERFOGTODESTALPHA && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		alpha = fogFactor;

	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    bWriteDepthToAlpha, worldPos_projPosZ.w ) );
}
