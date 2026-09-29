#version 450
// Black's pixel stage: a port of stdshaders/black_ps2x.fxc (ps20b). Black,
// fogged to the fog color (FinalOutput blends by the squared range fog factor).
// Combos (fxctmp9/black_ps20b.inc): static CONVERT_TO_SRGB (2, always 0 here);
// dynamic PIXELFOGTYPE (1).
// @legacy program=black ps=black_ps20b vs=black_vs20 vert=black_vs20
#include "legacy_ps.glsl"

layout( location = 7 ) in vec4 worldPos_projPosZ;

#define g_FogParams PS_C( PSREG_FOG_PARAMS )
#define g_EyePos_SpecExponent PS_C( PSREG_EYEPOS_SPEC_EXPONENT )

void main()
{
	const int PIXELFOGTYPE = DYNAMIC_PS_COMBO( 1, 2 );

	float fogFactor = CalcPixelFogFactor( PIXELFOGTYPE, g_FogParams, g_EyePos_SpecExponent.z,
	    worldPos_projPosZ.z, worldPos_projPosZ.w );
	LegacyWrite( FinalOutput( vec4( 0.0, 0.0, 0.0, 1.0 ), fogFactor, PIXELFOGTYPE, TONEMAP_SCALE_NONE ) );
}
