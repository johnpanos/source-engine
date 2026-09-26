#version 450
// Modulate_DX9's pixel stage: a port of stdshaders/modulate_ps2x.fxc (ps20b).
// Combos (fxctmp9/modulate_ps20b.inc): static CONVERT_TO_SRGB (4, always 0
// here); dynamic PIXELFOGTYPE (1), WRITE_DEPTH_TO_DESTALPHA (2).
// @legacy program=modulate ps=modulate_ps20b vs=unlitgeneric_vs20 vert=unlitgeneric_vs20
//         samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 vTexCoord0;
layout( location = 7 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vColor;

#define g_WhiteGrayMix PS_C( 0 )
#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 2, 2 );

	vec4 textureColor = tex2D( 0, BaseTextureSampler, vTexCoord0 );

	vec4 resultColor = saturate( textureColor * vColor );
	resultColor.rgb = mix( g_WhiteGrayMix.rgb, resultColor.rgb, resultColor.a );

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( resultColor, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE,
	    WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
