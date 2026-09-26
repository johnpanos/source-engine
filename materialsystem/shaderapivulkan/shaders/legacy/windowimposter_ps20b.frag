#version 450
// WindowImposter_DX90's pixel stage: a port of
// stdshaders/windowimposter_ps2x.fxc (ps20b), the cube map along the
// eye-to-vertex vector times the modulation color. Combos
// (fxctmp9/windowimposter_ps20b.inc): static CONVERT_TO_SRGB (2, always 0
// here); dynamic PIXELFOGTYPE (1).
// @legacy program=windowimposter ps=windowimposter_ps20b vs=windowimposter_vs20
//         vert=windowimposter_vs20 samplers=0:cube
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform samplerCube EnvmapSampler; // s0

layout( location = 0 ) in vec3 eyeToVertVector;
layout( location = 7 ) in vec4 worldPos_projPosZ;
layout( location = 8 ) in vec4 vertexColor;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	vec4 color;
	color.xyz = ENV_MAP_SCALE * texCUBE( 0, EnvmapSampler, eyeToVertVector ).xyz;
	color.a = 1.0;
	color *= vertexColor;

	const float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( color, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
