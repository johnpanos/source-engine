#version 450
// Cloak_DX90's pixel stage: a port of stdshaders/cloak_ps2x.fxc (ps20b).
// Combos (fxctmp9/cloak_ps20b.inc): static CONVERT_TO_SRGB (20, always 0
// here), LIGHTWARPTEXTURE (40); dynamic PIXELFOGTYPE (1),
// WRITEWATERFOGTODESTALPHA (2), NUM_LIGHTS (4, 0..4). The pass never enables
// sampler 4 (SpecExponentSampler), so it reads D3D9's no-texture ( 0, 0, 0, 1 );
// the ps20b build does not read the normalization cube map (s5).
// @legacy program=cloak ps=cloak_ps20b vs=cloak_vs20 vert=cloak_vs20
//         samplers=0:2d,1:2d,2:2d,3:2d,4:2d
#include "legacy_ps.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseSampler;        // s0
layout( set = 0, binding = 1 ) uniform sampler2D DiffuseWarpSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D RefractSampler;     // s2
layout( set = 0, binding = 3 ) uniform sampler2D NormalSampler;      // s3
layout( set = 0, binding = 4 ) uniform sampler2D SpecExponentSampler; // s4

layout( location = 0 ) in vec2 vBaseTexCoord;
layout( location = 1 ) in vec3 tangentSpaceTranspose0;
layout( location = 2 ) in vec3 tangentSpaceTranspose1;
layout( location = 3 ) in vec3 tangentSpaceTranspose2;
layout( location = 4 ) in vec3 worldPos;
layout( location = 5 ) in vec3 projPos;
layout( location = 6 ) in vec4 lightAtten;

// float3x3 g_ViewProj : c0..c2, register-packed by column: row n is
// ( c0[n], c1[n], c2[n] ).
#define g_ViewProjRow( n ) vec3( PS_C( 0 )[n], PS_C( 1 )[n], PS_C( 2 )[n] )
#define g_CloakControl ( PS_C( 3 ).xy ) // { refract amount, cloak, ?, ? }
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_FresnelSpecParams PS_C( PSREG_FRESNEL_SPEC_PARAMS )
#define g_SpecularRimParams PS_C( PSREG_SPEC_RIM_PARAMS )
#define g_FresnelRanges ( g_FresnelSpecParams.xyz )
#define g_SpecularBoost ( g_FresnelSpecParams.w )
#define g_SpecularTint ( g_SpecularRimParams.xyz )
#define g_RimExponent ( g_SpecularRimParams.w )

// 8 2D Poisson offsets (designed to use .xy and .wz swizzles (not .zw)
const vec4 gPoissonOffset[4] = vec4[4]( vec4( -0.0876, 0.9703, 0.5651, 0.4802 ),
    vec4( 0.1851, 0.1580, -0.0617, -0.2616 ), vec4( -0.5477, -0.6603, 0.0711, -0.5325 ),
    vec4( -0.0751, -0.8954, 0.4054, 0.6384 ) );

void main()
{
	const bool bDoDiffuseWarp = STATIC_PS_COMBO( 40, 2 ) != 0;
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const bool WRITEWATERFOGTODESTALPHA = DYNAMIC_PS_COMBO( 2, 2 ) != 0;
	const int nNumLights = DYNAMIC_PS_COMBO( 4, 5 );

	vec3 vSpecular = vec3( 0.0 );

	// Base color
	vec4 albedo = tex2D( 0, BaseSampler, vBaseTexCoord );

	// Load normal and expand range
	vec4 vNormalSample = tex2D( 3, NormalSampler, vBaseTexCoord );
	vec3 tangentSpaceNormal = 2.0 * vNormalSample.xyz - 1.0;

	// We need a world space normal if we're doing any lighting
	vec3 vWorldNormal = normalize( tangentSpaceNormal.x * tangentSpaceTranspose0 +
	    tangentSpaceNormal.y * tangentSpaceTranspose1 + tangentSpaceNormal.z * tangentSpaceTranspose2 );
	vec3 vWorldEyeDir = normalize( g_EyePos_SpecExponent.xyz - worldPos );

	// Vanilla 1-(N.V) fresnel term used later in transition lerp
	float fresnel = 1.0 - saturate( dot( vWorldNormal, vWorldEyeDir ) );

	// Summation of diffuse illumination from all local lights
	vec3 diffuseLighting = PixelShaderDoLighting( worldPos, vWorldNormal, vec3( 0.0 ), false, true,
	    lightAtten, PSREG_AMBIENT_CUBE, nNumLights, PSREG_LIGHT_INFO_ARRAY, true, false, 1.0,
	    bDoDiffuseWarp, 1, DiffuseWarpSampler );

	// Transform world space normal into clip space and project
	vec2 vProjNormal;
	vProjNormal.x = dot( vWorldNormal, g_ViewProjRow( 0 ) ); // 1st row
	vProjNormal.y = dot( vWorldNormal, g_ViewProjRow( 1 ) ); // 2nd row

	// Compute coordinates for sampling refraction
	vec2 vRefractTexCoordNoWarp = projPos.xy / projPos.z;
	vec2 vRefractTexCoord = vProjNormal.xy;
	float scale = mix( g_CloakControl.x, 0.0, g_CloakControl.y );
	vRefractTexCoord *= scale;
	vRefractTexCoord += vRefractTexCoordNoWarp;

	// Blur by scalable Poisson filter
	float fBlurAmount = mix( 0.05, 0.0, g_CloakControl.y );
	vec3 vRefract = tex2D( 2, RefractSampler, vRefractTexCoord ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[0].xy * fBlurAmount ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[0].wz * fBlurAmount ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[1].xy * fBlurAmount ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[1].wz * fBlurAmount ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[2].xy * fBlurAmount ).rgb;
	vRefract += tex2D( 2, RefractSampler, vRefractTexCoord + gPoissonOffset[2].wz * fBlurAmount ).rgb;
	vRefract /= 7.0;

	vec3 rimLighting = vec3( 0.0 );
	vec3 specularLighting = vec3( 0.0 );
	float fSpecExp = g_EyePos_SpecExponent.w;
	float fSpecMask = vNormalSample.a;

	vec4 vSpecExpMap = tex2D( 4, SpecExponentSampler, vBaseTexCoord );
	float fSpecExpMap = vSpecExpMap.r;
	float fRimMask = 1.0;
	vec3 vSpecularTint;

	// If the exponent passed in as a constant is zero, use the value from the map as the exponent
	if ( fSpecExp == 0.0 )
		fSpecExp = 1.0 - fSpecExpMap + 150.0 * fSpecExpMap;

	// If constant tint is negative, tint with albedo, based upon scalar tint map
	if ( g_SpecularTint.r == -1.0 )
		vSpecularTint = mix( vec3( 1.0 ), albedo.rgb, vSpecExpMap.g );
	else
		vSpecularTint = g_SpecularTint.rgb;

	// Fresnel to match regular specular lighting
	float fFresnelRanges = Fresnel( vWorldNormal, vWorldEyeDir, g_FresnelRanges );

	// Summation of specular from all local lights besides the flashlight
	PixelShaderDoSpecularLighting( worldPos, vWorldNormal, fSpecExp, vWorldEyeDir, lightAtten,
	    nNumLights, PSREG_LIGHT_INFO_ARRAY, false, 1.0, false, 0, BaseSampler, 1.0, true,
	    g_RimExponent, specularLighting, rimLighting );

	// Modulate with spec mask, boost, tint and fresnel ranges
	specularLighting *= fSpecMask * g_SpecularBoost * fFresnelRanges * vSpecularTint;

	float fRimFresnel = Fresnel4( vWorldNormal, vWorldEyeDir );

	// Add in rim light modulated with tint, mask and traditional Fresnel (not using Fresnel ranges)
	rimLighting *= vSpecularTint * fRimMask * fRimFresnel;

	// Fold rim lighting into specular term by using the max so that we don't really add light twice...
	specularLighting = max( specularLighting, rimLighting );

	// Add in view-ray lookup from ambient cube
	specularLighting += fRimFresnel * fRimMask * vSpecularTint *
	                    PixelShaderAmbientLight( vWorldEyeDir, PSREG_AMBIENT_CUBE ) *
	                    saturate( dot( vWorldNormal, vec3( 0.0, 0.0, 1.0 ) ) );

	float tintLerpFactor = saturate( mix( 1.0, fresnel - 1.1, saturate( g_CloakControl.y ) ) );
	tintLerpFactor = smoothstep( 0.4, 0.425, tintLerpFactor );
	vec3 vTintedRefract = mix( vRefract, albedo.rgb * vRefract, 0.7 );
	vRefract = mix( vRefract, vTintedRefract, tintLerpFactor );

	vSpecular = specularLighting * smoothstep( 0.98, 0.8, saturate( g_CloakControl.y ) );

	// Blend refraction component with diffusely lit model
	float diffuseLerpFactor = saturate( mix( 1.0, fresnel - 1.35, saturate( g_CloakControl.y ) ) );
	diffuseLerpFactor = smoothstep( 0.4, 0.425, diffuseLerpFactor );

	vec3 fDiffuse = mix( vRefract, albedo.rgb * diffuseLighting, diffuseLerpFactor );
	vec3 result = fDiffuse + vSpecular;

	float alpha = 1.0;

	// Emulate LinearColorToHDROutput() when uncloaked
	result = mix( result.xyz * LINEAR_LIGHT_SCALE, result, saturate( g_CloakControl.y ) );

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z, worldPos.z, projPos.z );

	if ( WRITEWATERFOGTODESTALPHA && PIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT )
		alpha = fogFactor;

	LegacyWrite( FinalOutput( vec4( result, alpha ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE ) );
}
