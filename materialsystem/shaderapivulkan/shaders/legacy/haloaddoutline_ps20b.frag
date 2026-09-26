#version 450
// A port of stdshaders/haloaddoutline_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws by $pixshader: the largest of four texture taps
// two texels (c4, the base texture's texel size) diagonally away; alpha the
// largest channel. (It declares samplers 1..3 and reads none.)
// @legacy program=haloaddoutline ps=haloaddoutline_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

#define g_vPixelSize PS_C( 4 )

void main()
{
	vec2 vOffset = 2.0 * g_vPixelSize.xy;

	// The largest of four offset samples.
	vec4 result;
	const vec2 uv = baseTexCoord.xy;
	result.rgba = tex2D( 0, TexSampler, uv + vec2( vOffset.x, vOffset.y ) );
	result.rgba = max( result.rgba, tex2D( 0, TexSampler, uv + vec2( vOffset.x, -vOffset.y ) ) );
	result.rgba = max( result.rgba, tex2D( 0, TexSampler, uv + vec2( -vOffset.x, vOffset.y ) ) );
	result.rgba = max( result.rgba, tex2D( 0, TexSampler, uv + vec2( -vOffset.x, -vOffset.y ) ) );

	float flLuminance = max( result.r, max( result.g, result.b ) );
	result.a = flLuminance;

	LegacyWrite( result );
}
