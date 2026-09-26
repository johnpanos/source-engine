#version 450
// A port of stdshaders/constant_color_ps2x.fxc (ps20b), which
// screenspace_general draws (dev/no_pixel_write): constant green. Combos
// (fxctmp9/constant_color_ps20b.inc): static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=constant_color ps=constant_color_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 flags=object_position
#include "legacy_ps.glsl"

void main()
{
	LegacyWrite( FinalOutput(
	    vec4( 0.0, 1.0, 0.0, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
