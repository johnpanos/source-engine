#version 450
// VertexLitGeneric's $phong pixel stage: a port of stdshaders/skin_ps20b.fxc.
// Combos (fxctmp9/skin_ps20b.inc): static CUBEMAP (80), SELFILLUM (160),
// SELFILLUMFRESNEL (320), FLASHLIGHT (640), LIGHTWARPTEXTURE (1280),
// PHONGWARPTEXTURE (2560), WRINKLEMAP (5120), DETAIL_BLEND_MODE (10240, 0..6),
// DETAILTEXTURE (71680), RIMLIGHT (143360), FLASHLIGHTDEPTHFILTERMODE (286720,
// 0..2), FASTPATH_NOBUMP (860160), BLENDTINTBYBASEALPHA (1720320);
// CONVERT_TO_SRGB is 0..0. Dynamic WRITEWATERFOGTODESTALPHA (1), PIXELFOGTYPE
// (2), NUM_LIGHTS (4, 0..4), WRITE_DEPTH_TO_DESTALPHA (20), FLASHLIGHTSHADOWS
// (40). FLASHLIGHT, FLASHLIGHTDEPTHFILTERMODE and FLASHLIGHTSHADOWS are not
// ported: this backend never draws a flashlight pass (InFlashlightMode is false
// and it reports no shadow filter mode), and the shader API keeps the
// FLASHLIGHT snapshots on the native skin family, which reports them.
// The wrinkle weight (skin_vs20's projPos_fWrinkleWeight.w) is zero on this
// backend: the vertex record carries no flex stream (see skin_vs20.vert).
// The samplers D3D9 may read as sRGB (s0, s8, s9, s10, s13) take bindings 0..4;
// the shader API can ask the shader to decode only bindings 0..7, so the
// others, which skin_dx9_helper.cpp never reads as sRGB, are read as stored.
// @legacy program=skin ps=skin_ps20b vs=skin_vs20 vert=skin_vs20
//         samplers=0:2d,8:cube,9:2d,10:2d,13:2d,1:2d,2:2d,3:2d,7:2d,11:2d,12:2d,14:2d
#include "legacy_ps.glsl"
#include "legacy_combine.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler;    // s0, selfillum in alpha
layout( set = 0, binding = 1 ) uniform samplerCube EnvmapSampler;       // s8
layout( set = 0, binding = 2 ) uniform sampler2D WrinkleSampler;        // s9: compression base
layout( set = 0, binding = 3 ) uniform sampler2D StretchSampler;        // s10: expansion base
layout( set = 0, binding = 4 ) uniform sampler2D DetailSampler;         // s13
layout( set = 0, binding = 5 ) uniform sampler2D SpecularWarpSampler;   // s1
layout( set = 0, binding = 6 ) uniform sampler2D DiffuseWarpSampler;    // s2: lighting warp
layout( set = 0, binding = 7 ) uniform sampler2D NormalMapSampler;      // s3, spec mask in alpha
layout( set = 0, binding = 8 ) uniform sampler2D SpecExponentSampler;   // s7
layout( set = 0, binding = 9 ) uniform sampler2D NormalWrinkleSampler;  // s11: compression normal
layout( set = 0, binding = 10 ) uniform sampler2D NormalStretchSampler; // s12: expansion normal
layout( set = 0, binding = 11 ) uniform sampler2D SelfIllumMaskSampler; // s14

// Bindings 8 and up: never read as sRGB.
#define tex2DStored( s, uv ) texture( ( s ), ( uv ) )

layout( location = 0 ) in vec4 baseTexCoordDetailTexCoord; // xy=base zw=detail
layout( location = 1 ) in vec3 lightAtten; // lights 0..2 (light 3 in worldPos_atten3.w)
layout( location = 2 ) in vec3 worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ;
// tangentSpaceTranspose (float3x3 at TEXCOORD3..5): the interpolants hold its
// columns, the world tangent S, tangent T and normal.
layout( location = 3 ) in vec3 tangentSpaceTransposeColumn0;
layout( location = 4 ) in vec3 tangentSpaceTransposeColumn1;
layout( location = 5 ) in vec3 tangentSpaceTransposeColumn2;
layout( location = 6 ) in vec4 worldPos_atten3;
layout( location = 7 ) in vec4 projPos_fWrinkleWeight;

#define g_SelfIllumTint_and_DetailBlendFactor PS_C( PSREG_SELFILLUMTINT )
#define g_SelfIllumScaleBiasExpBrightness PS_C( PSREG_SELFILLUM_SCALE_BIAS_EXP )
#define g_DiffuseModulation PS_C( PSREG_DIFFUSE_MODULATION )
// w controls spec mask
#define g_EnvmapTint_ShadowTweaks PS_C( PSREG_ENVMAP_TINT_SHADOW_TWEAKS )
#define cAmbientCubeReg PSREG_AMBIENT_CUBE
// x is envmap fresnel ... w is selfillummask control
#define g_EnvMapFresnel PS_C( PSREG_ENVMAP_FRESNEL_SELFILLUMMASK )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
// On non-flashlight pass, x has rim mask control
#define g_FlashlightAttenuationFactors_RimMask PS_C( PSREG_FLASHLIGHT_ATTENUATION )
#define g_FlashlightPos_RimBoost PS_C( PSREG_FLASHLIGHT_POSITION_RIM_BOOST )
// xyz are fresnel, w is specular boost
#define g_FresnelSpecParams PS_C( PSREG_FRESNEL_SPEC_PARAMS )
// xyz are specular tint color, w is rim power
#define g_SpecularRimParams PS_C( PSREG_SPEC_RIM_PARAMS )
#define cLightInfoReg PSREG_LIGHT_INFO_ARRAY
// PSREG_CONSTANT_27: x is basemap alpha phong mask, z is tint overlay amount,
// w controls "INVERTPHONGMASK"
#define g_ShaderControls PS_C( 27 )

#define g_fRimBoost ( g_FlashlightPos_RimBoost.w )
#define g_FresnelRanges ( g_FresnelSpecParams.xyz )
#define g_SpecularBoost ( g_FresnelSpecParams.w )
#define g_SpecularTint ( g_SpecularRimParams.xyz )
#define g_RimExponent ( g_SpecularRimParams.w )
#define g_RimMaskControl ( g_FlashlightAttenuationFactors_RimMask.x )
#define g_SelfIllumMaskControl ( g_EnvMapFresnel.w )
#define g_fBaseMapAlphaPhongMask ( g_ShaderControls.x )
#define g_fTintReplacementControl ( g_ShaderControls.z )
#define g_fInvertPhongMask ( g_ShaderControls.w )

void main()
{
	const bool bCubemap = STATIC_PS_COMBO( 80, 2 ) != 0;
	const bool bSelfIllum = STATIC_PS_COMBO( 160, 2 ) != 0;
	const bool SELFILLUMFRESNEL = STATIC_PS_COMBO( 320, 2 ) != 0;
	const bool bDoDiffuseWarp = STATIC_PS_COMBO( 1280, 2 ) != 0;
	const bool bDoSpecularWarp = STATIC_PS_COMBO( 2560, 2 ) != 0;
	const bool bWrinkleMap = STATIC_PS_COMBO( 5120, 2 ) != 0;
	const int DETAIL_BLEND_MODE = STATIC_PS_COMBO( 10240, 7 );
	const bool DETAILTEXTURE = STATIC_PS_COMBO( 71680, 2 ) != 0;
	const bool bDoRimLighting = STATIC_PS_COMBO( 143360, 2 ) != 0;
	const bool FASTPATH_NOBUMP = STATIC_PS_COMBO( 860160, 2 ) != 0;
	const bool bBlendTintByBaseAlpha = STATIC_PS_COMBO( 1720320, 2 ) != 0;
	const bool WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 1, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 2, 2 );
	const int nNumLights = DYNAMIC_PS_COMBO( 4, 5 );
	const bool WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 20, 2 ) != 0;

	// Unpacking for convenience
	const float fWrinkleWeight = projPos_fWrinkleWeight.w;
	const vec3 vProjPos = projPos_fWrinkleWeight.xyz;
	const vec3 vWorldPos = worldPos_atten3.xyz;
	const float atten3 = worldPos_atten3.w;

	const vec4 vLightAtten = vec4( lightAtten, atten3 );

	// One of these two is zero while the other is in the 0..1 range; with the
	// texture amount they sum to one.
	const float flWrinkleAmount = saturate( -fWrinkleWeight );
	const float flStretchAmount = saturate( fWrinkleWeight );
	const float flTextureAmount = 1.0 - flWrinkleAmount - flStretchAmount;

	const vec2 baseTexCoord = baseTexCoordDetailTexCoord.xy;
	vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord );
	if ( bWrinkleMap )
	{
		const vec4 wrinkleColor = tex2D( 2, WrinkleSampler, baseTexCoord );
		const vec4 stretchColor = tex2D( 3, StretchSampler, baseTexCoord );

		// Apply wrinkle blend to only RGB.  Alpha comes from the base texture
		baseColor.rgb = ( flTextureAmount * baseColor + flWrinkleAmount * wrinkleColor +
		                    flStretchAmount * stretchColor )
		                    .rgb;
	}

	vec4 detailColor = vec4( 0.0 );
	if ( DETAILTEXTURE )
	{
		detailColor = tex2D( 4, DetailSampler, baseTexCoordDetailTexCoord.zw );
		baseColor = TextureCombine(
		    baseColor, detailColor, DETAIL_BLEND_MODE, g_SelfIllumTint_and_DetailBlendFactor.w );
	}

	const float fogFactor = CalcPixelFogFactor(
	    PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z, vWorldPos.z, vProjPos.z );

	const vec3 vEyeDir = normalize( worldVertToEyeVectorXYZ_tangentSpaceVertToEyeVectorZ.xyz );
	const vec3 vRimAmbientCubeColor = PixelShaderAmbientLight( vEyeDir, cAmbientCubeReg );

	vec3 worldSpaceNormal, tangentSpaceNormal;
	float fSpecMask = 1.0;
	vec4 normalTexel = tex2D( 7, NormalMapSampler, baseTexCoord );

	if ( bWrinkleMap )
	{
		const vec4 wrinkleNormal = tex2DStored( NormalWrinkleSampler, baseTexCoord );
		const vec4 stretchNormal = tex2DStored( NormalStretchSampler, baseTexCoord );
		normalTexel = flTextureAmount * normalTexel + flWrinkleAmount * wrinkleNormal +
		              flStretchAmount * stretchNormal;
	}

	if ( !FASTPATH_NOBUMP )
	{
		tangentSpaceNormal =
		    mix( 2.0 * normalTexel.xyz - 1.0, vec3( 0.0, 0.0, 1.0 ), g_fBaseMapAlphaPhongMask );
		fSpecMask = mix( normalTexel.a, baseColor.a, g_fBaseMapAlphaPhongMask );
	}
	else
	{
		tangentSpaceNormal = vec3( 0.0, 0.0, 1.0 );
		fSpecMask = baseColor.a;
	}

	// We need a normal if we're doing any lighting:
	// mul( tangentSpaceTranspose, tangentSpaceNormal ).
	worldSpaceNormal = normalize( tangentSpaceTransposeColumn0 * tangentSpaceNormal.x +
	                              tangentSpaceTransposeColumn1 * tangentSpaceNormal.y +
	                              tangentSpaceTransposeColumn2 * tangentSpaceNormal.z );

	const float fFresnelRanges = Fresnel( worldSpaceNormal, vEyeDir, g_FresnelRanges );
	const float fRimFresnel = Fresnel4( worldSpaceNormal, vEyeDir );

	// Break down reflect so that we can share dot(worldSpaceNormal,vEyeDir) with fresnel terms
	const vec3 vReflect = 2.0 * worldSpaceNormal * dot( worldSpaceNormal, vEyeDir ) - vEyeDir;

	// Summation of diffuse illumination from all local lights (half-Lambert,
	// ambient always).
	const vec3 diffuseLighting = PixelShaderDoLighting( vWorldPos, worldSpaceNormal, vec3( 0.0 ),
	    false, true, vLightAtten, cAmbientCubeReg, nNumLights, cLightInfoReg, true, false, 1.0,
	    bDoDiffuseWarp, 6, DiffuseWarpSampler );

	vec3 envMapColor = vec3( 0.0 );
	if ( bCubemap )
	{
		// Mask is either normal map alpha or base map alpha
		float fEnvMapMask;
		if ( SELFILLUMFRESNEL ) // This is to match the 2.0 version of vertexlitgeneric
			fEnvMapMask = mix( baseColor.a, g_fInvertPhongMask, g_EnvmapTint_ShadowTweaks.w );
		else
			fEnvMapMask = mix( baseColor.a, fSpecMask, g_EnvmapTint_ShadowTweaks.w );

		envMapColor = ( ENV_MAP_SCALE * mix( 1.0, fFresnelRanges, g_EnvMapFresnel.x ) *
		                  mix( fEnvMapMask, 1.0 - fEnvMapMask, g_fInvertPhongMask ) ) *
		              texCUBE( 1, EnvmapSampler, vReflect ).rgb * g_EnvmapTint_ShadowTweaks.xyz;
	}

	vec3 specularLighting = vec3( 0.0 );
	vec3 rimLighting = vec3( 0.0 );

	vec3 vSpecularTint = vec3( 1.0 );
	float fRimMask = 0.0;
	float fSpecExp = 1.0;

	if ( !FASTPATH_NOBUMP )
	{
		const vec4 vSpecExpMap = tex2DStored( SpecExponentSampler, baseTexCoord );

		fRimMask = mix( 1.0, vSpecExpMap.a, g_RimMaskControl ); // Select rim mask

		// If the exponent passed in as a constant is zero, use the value from the map
		// as the exponent
		fSpecExp = ( g_EyePos_SpecExponent.w >= 0.0 ) ? g_EyePos_SpecExponent.w
		                                               : ( 1.0 + 149.0 * vSpecExpMap.r );

		// If constant tint is negative, tint with albedo, based upon scalar tint map
		vSpecularTint = mix( vec3( 1.0 ), baseColor.rgb, vSpecExpMap.g );
		vSpecularTint = ( g_SpecularTint.r >= 0.0 ) ? g_SpecularTint.rgb : vSpecularTint;
	}
	else
	{
		fSpecExp = max( g_EyePos_SpecExponent.w, 0.0 );
	}

	vec3 albedo = baseColor.rgb;

	// Summation of specular from all local lights besides the flashlight
	PixelShaderDoSpecularLighting( vWorldPos, worldSpaceNormal, fSpecExp, vEyeDir, vLightAtten,
	    nNumLights, cLightInfoReg, false, 1.0, bDoSpecularWarp, 5, SpecularWarpSampler,
	    fFresnelRanges, bDoRimLighting, g_RimExponent, specularLighting, rimLighting );

	// If we didn't already apply Fresnel to specular warp, modulate the specular
	if ( !bDoSpecularWarp )
		fSpecMask *= fFresnelRanges;

	// Modulate with spec mask, boost and tint
	specularLighting *= fSpecMask * g_SpecularBoost;

	if ( bBlendTintByBaseAlpha )
	{
		vec3 tintedColor = albedo * g_DiffuseModulation.rgb;
		tintedColor = mix( tintedColor, g_DiffuseModulation.rgb, g_fTintReplacementControl );
		albedo = mix( albedo, tintedColor, baseColor.a );
	}
	else
	{
		albedo = albedo * g_DiffuseModulation.rgb;
	}

	vec3 diffuseComponent = albedo * diffuseLighting;
	if ( bSelfIllum )
	{
		if ( SELFILLUMFRESNEL )
		{
			// A Fresnel term based on the vertex normal (not the per-pixel
			// normal!) to help fake an internal glow look.
			const vec3 vVertexNormal = normalize( vec3( tangentSpaceTransposeColumn2.x,
			    tangentSpaceTransposeColumn2.y, tangentSpaceTransposeColumn2.z ) );
			const float flSelfIllumFresnel =
			    ( HlslPow( saturate( dot( vVertexNormal.xyz, vEyeDir.xyz ) ),
			          g_SelfIllumScaleBiasExpBrightness.z ) *
			        g_SelfIllumScaleBiasExpBrightness.x ) +
			    g_SelfIllumScaleBiasExpBrightness.y;
			const vec3 selfIllumComponent = g_SelfIllumTint_and_DetailBlendFactor.rgb * albedo *
			                                g_SelfIllumScaleBiasExpBrightness.w;
			diffuseComponent = mix( diffuseComponent, selfIllumComponent,
			    baseColor.a * saturate( flSelfIllumFresnel ) );
		}
		else
		{
			vec3 vSelfIllumMask = tex2DStored( SelfIllumMaskSampler, baseTexCoord ).rgb;
			vSelfIllumMask = mix( baseColor.aaa, vSelfIllumMask, g_SelfIllumMaskControl );
			diffuseComponent = mix( diffuseComponent,
			    g_SelfIllumTint_and_DetailBlendFactor.rgb * albedo, vSelfIllumMask );
		}

		diffuseComponent = max( vec3( 0.0 ), diffuseComponent );
	}

	if ( DETAILTEXTURE )
	{
		diffuseComponent = TextureCombinePostLighting( diffuseComponent, detailColor,
		    DETAIL_BLEND_MODE, g_SelfIllumTint_and_DetailBlendFactor.w );
	}

	if ( bDoRimLighting )
	{
		const float fRimMultiply = fRimMask * fRimFresnel; // both unit range: [0, 1]

		// Add in rim light modulated with tint, mask and traditional Fresnel (not using
		// Fresnel ranges)
		rimLighting *= fRimMultiply;

		// Fold rim lighting into specular term by using the max so that we don't really
		// add light twice...
		specularLighting = max( specularLighting, rimLighting );

		// Add in view-ray lookup from ambient cube
		specularLighting +=
		    ( vRimAmbientCubeColor * g_fRimBoost ) * saturate( fRimMultiply * worldSpaceNormal.z );
	}

	const vec3 result = specularLighting * vSpecularTint + envMapColor + diffuseComponent;

	float alpha;
	if ( WRITEWATERFOGTODESTALPHA && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
	{
		alpha = fogFactor;
	}
	else
	{
		alpha = g_DiffuseModulation.a;
		if ( !bSelfIllum && !bBlendTintByBaseAlpha )
			alpha = mix( baseColor.a * alpha, alpha, g_fBaseMapAlphaPhongMask );
	}

	const bool bWriteDepthToAlpha = WRITE_DEPTH_TO_DESTALPHA && !WRITEWATERFOGTODESTALPHA;

	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    bWriteDepthToAlpha, vProjPos.z ) );
}
