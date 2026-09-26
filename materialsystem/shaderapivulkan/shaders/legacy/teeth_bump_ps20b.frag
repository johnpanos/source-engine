#version 450
// Teeth_DX9's pixel stage with a bump map: a port of
// stdshaders/teeth_bump_ps2x.fxc (ps20b). Combos (fxctmp9/teeth_bump_ps20b.inc): static
// CONVERT_TO_SRGB (40, always 0 here); dynamic PIXELFOGTYPE (1), NUM_LIGHTS
// (2, 0..4), AMBIENT_LIGHT (10), WRITE_DEPTH_TO_DESTALPHA (20). The ps20b build
// normalizes with math, so the normalization cube map (s2) is not read.
// @legacy program=teeth_bump ps=teeth_bump_ps20b vs=teeth_bump_vs20 vert=teeth_bump_vs20
//         samplers=0:2d,1:2d
#include "legacy_ps.glsl"
#include "legacy_ps_lighting.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D BumpTextureSampler; // s1

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec4 worldVertToEyeVector_Darkening;
layout( location = 2 ) in vec3 tangentSpaceTranspose0;
layout( location = 3 ) in vec3 tangentSpaceTranspose1;
layout( location = 4 ) in vec3 tangentSpaceTranspose2;
layout( location = 5 ) in vec4 worldPos_projPosZ;
layout( location = 6 ) in vec2 lightAtten01;
layout( location = 7 ) in vec2 lightAtten23;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )
#define worldVertToEyeVector ( worldVertToEyeVector_Darkening.xyz )
#define fDarkening ( worldVertToEyeVector_Darkening.w )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int nNumLights = DYNAMIC_PS_COMBO( 2, 5 );
	const bool bAmbientLight = DYNAMIC_PS_COMBO( 10, 2 ) != 0;
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 20, 2 );

	vec4 vLightAtten = vec4( lightAtten01, lightAtten23 );
	vec4 baseSample = tex2D( 0, BaseTextureSampler, baseTexCoord );

	vec3 worldSpaceNormal, tangentSpaceNormal = vec3( 0.0, 0.0, 1.0 );
	float fSpecExp = g_EyePos_SpecExponent.w;

	vec4 normalTexel = tex2D( 1, BumpTextureSampler, baseTexCoord );
	tangentSpaceNormal = 2.0 * normalTexel.xyz - 1.0;
	// mul( tangentSpaceTranspose, tangentSpaceNormal ) over the column registers.
	worldSpaceNormal = normalize( tangentSpaceNormal.x * tangentSpaceTranspose0 +
	    tangentSpaceNormal.y * tangentSpaceTranspose1 + tangentSpaceNormal.z * tangentSpaceTranspose2 );

	// If the exponent passed in as a constant is zero, use the value from the map as the exponent
	if ( fSpecExp == 0.0 )
		fSpecExp = 1.0 * ( 1.0 - normalTexel.w ) + 150.0 * normalTexel.w;

	const vec3 worldPos = worldPos_projPosZ.xyz;

	// Summation of diffuse illumination from all local lights
	vec3 diffuseLighting = PixelShaderDoLighting( worldPos, worldSpaceNormal, vec3( 0.0 ), false,
	    bAmbientLight, vLightAtten, PSREG_AMBIENT_CUBE, nNumLights, PSREG_LIGHT_INFO_ARRAY, true,
	    false, 0.0, false, 0, BaseTextureSampler );

	// Summation of specular from all local lights
	vec3 vDummy, specularLighting;
	PixelShaderDoSpecularLighting( worldPos, worldSpaceNormal, fSpecExp, normalize( worldVertToEyeVector ),
	    vLightAtten, nNumLights, PSREG_LIGHT_INFO_ARRAY, false, 1.0, false, 0, BaseTextureSampler, 1.0,
	    false, 1.0, specularLighting, vDummy );

	// Specular plus diffuse, all darkened as a function of mouth openness
	vec3 result = ( specularLighting * baseSample.a + baseSample.rgb * diffuseLighting ) * fDarkening;

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( vec4( result, 1.0 ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
