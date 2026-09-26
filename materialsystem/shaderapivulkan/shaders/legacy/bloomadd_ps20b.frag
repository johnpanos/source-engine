#version 450
// A port of stdshaders/bloomadd_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws (dev/bloomadd): the texture with alpha 1.
// @legacy program=bloomadd ps=bloomadd_ps20b vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

void main()
{
	vec4 result = tex2D( 0, TexSampler, baseTexCoord );
	result.a = 1.0;
	LegacyWrite( result );
}
