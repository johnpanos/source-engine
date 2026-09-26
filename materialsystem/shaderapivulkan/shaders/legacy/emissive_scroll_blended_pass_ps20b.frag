#version 450
// VertexLitGeneric's $emissiveblendenabled pass: a port of
// stdshaders/emissive_scroll_blended_pass_ps2x.fxc (ps20b): the base times an
// emissive texture read through a scrolled flow map, added over the model.
// Combos (fxctmp9/emissive_scroll_blended_pass_ps20b.inc): static
// CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=emissive_scroll_blended_pass ps=emissive_scroll_blended_pass_ps20b
//         vs=emissive_scroll_blended_pass_vs20 vert=emissive_scroll_blended_pass_vs20
//         samplers=0:2d,1:2d,2:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tBaseSampler;      // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tFlowSampler;      // s1
layout( set = 0, binding = 2 ) uniform sampler2D g_tSelfIllumSampler; // s2

layout( location = 0 ) in vec2 vTexCoord0;

#define g_flBlendStrength ( PS_C( 0 ).x )
#define g_flTime ( PS_C( 0 ).y )
#define g_vEmissiveScrollVector ( PS_C( 1 ).xy )
#define g_cSelfIllumTint ( PS_C( 2 ).rgb )

void main()
{
	// Color texture
	vec4 cBaseColor = tex2D( 0, g_tBaseSampler, vTexCoord0 );

	// Fetch from dudv map and then fetch from emissive texture with new uv's & scroll
	vec4 vFlowValue = tex2D( 1, g_tFlowSampler, vTexCoord0 );
	vec2 vEmissiveTexCoord = vFlowValue.xy + ( g_vEmissiveScrollVector * g_flTime );
	vec4 cEmissiveColor = tex2D( 2, g_tSelfIllumSampler, vEmissiveTexCoord );

	vec4 result;
	result.rgb = cBaseColor.rgb * cEmissiveColor.rgb * g_cSelfIllumTint;
	result.rgb *= g_flBlendStrength;
	// Alpha 0 so it doesn't change dest alpha.
	result.a = 0.0;

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR ) );
}
