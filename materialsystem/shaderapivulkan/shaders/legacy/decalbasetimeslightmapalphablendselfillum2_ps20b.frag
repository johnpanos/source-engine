#version 450
// DecalBaseTimesLightmapAlphaBlendSelfIllum's second pass: a port of
// stdshaders/decalbasetimeslightmapalphablendselfillum2_ps2x.fxc (ps20b), the
// self-illumination texture alpha blended over the decal. Combos
// (fxctmp9/decalbasetimeslightmapalphablendselfillum2_ps20b.inc): static
// CONVERT_TO_SRGB (2, always 0 here); dynamic PIXELFOGTYPE (1).
// @legacy program=decalbasetimeslightmapalphablendselfillum2
//         ps=decalbasetimeslightmapalphablendselfillum2_ps20b vs=lightmappedgeneric_vs20
//         vert=lightmappedgeneric_vs20 samplers=0:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec4 vTexCoord0;
layout( location = 4 ) in vec4 worldPos_projPosZ;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	const vec4 result = tex2D( 0, BaseTextureSampler, vTexCoord0.xy );

	const float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( result, fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_LINEAR ) );
}
