#version 450
// hsl_filmgrain_pass2's pixel stage: a port of
// stdshaders/hsl_filmgrain_pass2_ps2x.fxc (ps20b): the HSL input image back to
// RGB. Combos (fxctmp9/hsl_filmgrain_pass2_ps20b.inc): static CONVERT_TO_SRGB
// (1, always 0 here).
// @legacy program=hsl_filmgrain_pass2 ps=hsl_filmgrain_pass2_ps20b vs=filmgrain_vs20
//         vert=filmgrain_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D InputSampler; // s0

layout( location = 0 ) in vec2 inputImageCoords;

void main()
{
	LegacyWrite( FinalOutput( HSLtoRGB( tex2D( 0, InputSampler, inputImageCoords ) ), 0.0,
	    PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
