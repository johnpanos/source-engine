#version 450
// Bik's pixel stage: a port of stdshaders/bik_ps2x.fxc (ps20b), the Y, Cr and
// Cb planes of a Bink frame converted to RGB, fogged. Combos
// (fxctmp9/bik_ps20b.inc): static CONVERT_TO_SRGB (2, always 0 here); dynamic
// PIXELFOGTYPE (1).
// @legacy program=bik ps=bik_ps20b vs=bik_vs20 vert=bik_vs20 samplers=0:2d,1:2d,2:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D YTextureSampler;  // s0
layout( set = 0, binding = 1 ) uniform sampler2D cRTextureSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D cBTextureSampler; // s2

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 1 ) in vec4 worldPos_projPosZ;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	float y = tex2D( 0, YTextureSampler, baseTexCoord.xy ).x;
	float cR = tex2D( 1, cRTextureSampler, baseTexCoord.xy ).x;
	float cB = tex2D( 2, cBTextureSampler, baseTexCoord.xy ).x;

	vec4 c = vec4( y, cR, cB, 1.0 );

	vec4 tor = vec4( 1.164123535, 1.595794678, 0.0, -0.87065506 );
	vec4 tog = vec4( 1.164123535, -0.813476563, -0.391448975, 0.529705048 );
	vec4 tob = vec4( 1.164123535, 0.0, 2.017822266, -1.081668854 );

	vec4 rgba;
	rgba.r = dot( c, tor );
	rgba.g = dot( c, tog );
	rgba.b = dot( c, tob );
	rgba.a = 1.0;

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( rgba, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE ) );
}
