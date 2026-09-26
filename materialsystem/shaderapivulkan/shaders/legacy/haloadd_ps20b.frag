#version 450
// A port of stdshaders/haloadd_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws by $pixshader: the texture dimmed by c0.x, its
// largest channel to the power 0.8 as alpha (for a ONE / INVSRCALPHA blend).
// @legacy program=haloadd ps=haloadd_ps20b vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20
//         samplers=0:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

#define g_flDimValue ( PS_C( 0 ).x )

void main()
{
	vec4 result = tex2D( 0, TexSampler, baseTexCoord );

	// Scale by the dim value before computing the luminance below.
	result.rgb *= saturate( g_flDimValue );

	float flLuminance = max( result.r, max( result.g, result.b ) );
	result.a = HlslPow( flLuminance, 0.8 );

	LegacyWrite( result );
}
