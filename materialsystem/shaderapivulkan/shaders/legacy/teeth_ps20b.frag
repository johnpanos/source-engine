#version 450
// Teeth_DX9's pixel stage without a bump map: a port of
// stdshaders/teeth_ps2x.fxc (ps20b). Combos (fxctmp9/teeth_ps20b.inc): static
// CONVERT_TO_SRGB (4, always 0 here); dynamic PIXELFOGTYPE (1),
// WRITE_DEPTH_TO_DESTALPHA (2).
// @legacy program=teeth ps=teeth_ps20b vs=teeth_vs20 vert=teeth_vs20
//         samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec3 vertAtten;
layout( location = 7 ) in vec4 worldPos_projPosZ;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 2, 2 );

	vec4 baseSample = tex2D( 0, BaseTextureSampler, baseTexCoord );

	vec4 result;
	result.xyz = baseSample.xyz * vertAtten;
	result.a = baseSample.a;

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( result, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR,
	    WRITE_DEPTH_TO_DESTALPHA != 0, worldPos_projPosZ.w ) );
}
