#version 450
// A port of stdshaders/haloadd1d_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws by $pixshader: the dimmed texture's channels
// remapped through the ramp textures of samplers 1..3 (read at ( v, v ),
// v the channel to the power 0.45) and summed; alpha the largest channel.
// @legacy program=haloadd1d ps=haloadd1d_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d,1:2d,2:2d,3:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D TexRed;     // s1
layout( set = 0, binding = 2 ) uniform sampler2D TexGreen;   // s2
layout( set = 0, binding = 3 ) uniform sampler2D TexBlue;    // s3

layout( location = 0 ) in vec2 baseTexCoord;

#define g_flDimValue ( PS_C( 0 ).x )

void main()
{
	vec4 result = tex2D( 0, TexSampler, baseTexCoord );

	// Scale by the dim value before computing the luminance below.
	result.rgb *= saturate( g_flDimValue );

	// tex2D of a scalar coordinate reads at ( v, v ).
	vec4 vRed = tex2D( 1, TexRed, vec2( HlslPow( result.r, 0.45 ) ) );
	vec4 vGreen = tex2D( 2, TexGreen, vec2( HlslPow( result.g, 0.45 ) ) );
	vec4 vBlue = tex2D( 3, TexBlue, vec2( HlslPow( result.b, 0.45 ) ) );
	result.rgb = vRed.rgb + vGreen.rgb + vBlue.rgb;

	float flLuminance = max( result.r, max( result.g, result.b ) );
	result.a = flLuminance;

	LegacyWrite( result );
}
