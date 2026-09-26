#version 450
// FilmDust_dx9's pixel stage: a port of stdshaders/filmdust_ps20.fxc (ps_2_0,
// no combos), the dust texture dotted with the channel selector, as grey.
// @legacy program=filmdust ps=filmdust_ps20 vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FilmDustSampler; // s0

layout( location = 0 ) in vec2 grainTexCoord;

#define vChannelSelect PS_C( 0 )

void main()
{
	vec4 noise = vec4( dot( tex2D( 0, FilmDustSampler, grainTexCoord ), vChannelSelect ) );
	LegacyWrite( vec4( noise.x, noise.y, noise.z, 1.0 ) );
}
