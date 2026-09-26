#version 450
// hsl_filmgrain_pass1's pixel stage: a port of
// stdshaders/hsl_filmgrain_pass1_ps2x.fxc (ps20b): the input image in HSL with
// the grain texture's noise added. Combos (fxctmp9/hsl_filmgrain_pass1_ps20b.inc):
// static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=hsl_filmgrain_pass1 ps=hsl_filmgrain_pass1_ps20b vs=filmgrain_vs20
//         vert=filmgrain_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D InputSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D FilmGrain;    // s1

layout( location = 0 ) in vec2 inputImageCoords;
layout( location = 1 ) in vec2 filmGrainCoords;

#define vNoiseScale PS_C( 0 )

void main()
{
	vec3 hsl = RGBtoHSL( tex2D( 0, InputSampler, inputImageCoords ) ).xyz;
	vec3 hslNoise = tex2D( 1, FilmGrain, filmGrainCoords ).xyz * 2.0 - 1.0;

	hsl += hslNoise * vNoiseScale.xyz * vec3( 0.5, 1.0, 1.0 );

	LegacyWrite( FinalOutput( vec4( hsl, 1.0 ), 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
