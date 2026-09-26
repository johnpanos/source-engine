#version 450
// FilmGrain_dx9's pixel stage: a port of stdshaders/filmgrain_ps20.fxc (ps_2_0,
// no combos), the grain texture scaled by vNoiseScale, its alpha the blend
// factor FilmGrain's ONE / SRC_ALPHA blend uses.
// @legacy program=filmgrain ps=filmgrain_ps20 vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FilmGrainSampler; // s0

layout( location = 0 ) in vec2 grainTexCoord;

#define vNoiseScale PS_C( 0 )

void main()
{
	vec4 noise = tex2D( 0, FilmGrainSampler, grainTexCoord );

	noise.w = noise.x * ( 1.0 - vNoiseScale.w ) + vNoiseScale.w;
	noise.xyz *= vNoiseScale.xyz;

	LegacyWrite( vec4( noise.x, noise.y, noise.z, noise.w ) );
}
