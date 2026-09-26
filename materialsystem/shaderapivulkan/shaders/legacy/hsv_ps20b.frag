#version 450
// HSV's pixel stage: a port of stdshaders/hsv_ps2x.fxc (ps20b), the frame's
// largest channel as grey. Combos (fxctmp9/hsv_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=hsv ps=hsv_ps20b vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

void main()
{
	vec4 baseColor = tex2D( 0, BaseTextureSampler, baseTexCoord );
	float maxValue = max( baseColor.r, baseColor.g );
	maxValue = max( baseColor.b, maxValue );
	vec4 result = vec4( maxValue );
	result.a = 1.0;
	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
