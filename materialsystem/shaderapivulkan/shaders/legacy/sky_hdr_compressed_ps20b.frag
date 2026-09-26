#version 450
// Sky_HDR_DX9's pixel stage for $hdrcompressedtexture0..2: a port of
// stdshaders/sky_hdr_compressed_ps2x.fxc (ps20b). The shader returns constant
// red (its three exposure samples are computed but unused, and fxc dropped
// them), scaled by LINEAR_LIGHT_SCALE. Combos
// (fxctmp9/sky_hdr_compressed_ps20b.inc): static CONVERT_TO_SRGB (2, always 0
// here); dynamic WRITE_DEPTH_TO_DESTALPHA (1).
// @legacy program=sky_hdr_compressed ps=sky_hdr_compressed_ps20b vs=sky_vs20
//         vert=sky_vs20 samplers=
#include "legacy_ps.glsl"

void main()
{
	const int WRITE_DEPTH_TO_DESTALPHA = DYNAMIC_PS_COMBO( 1, 2 );

	// This is never fogged.
	LegacyWrite( FinalOutput( vec4( 1.0, 0.0, 0.0, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE,
	    TONEMAP_SCALE_LINEAR, WRITE_DEPTH_TO_DESTALPHA != 0, 1e20 ) );
}
