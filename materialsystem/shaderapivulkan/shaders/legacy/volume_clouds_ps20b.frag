#version 450
// VolumeClouds_dx9's pixel stage: a port of stdshaders/volume_clouds_ps2x.fxc
// (ps20b: ten parallax layers, from the highest to the lowest, each the three
// rotating textures' alpha-weighted sum blended over the layers above).
// Combos (fxctmp9/volume_clouds_ps20b.inc): static CONVERT_TO_SRGB (1, always
// 0 here).
// @legacy program=volume_clouds ps=volume_clouds_ps20b vs=volume_clouds_vs20
//         vert=volume_clouds_vs20 samplers=0:2d,1:2d,2:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D g_tInnerSampler;  // s0
layout( set = 0, binding = 1 ) uniform sampler2D g_tMiddleSampler; // s1
layout( set = 0, binding = 2 ) uniform sampler2D g_tOuterSampler;  // s2

layout( location = 0 ) in vec4 v2DTangentViewVector01;
layout( location = 1 ) in vec4 vUv01;
layout( location = 2 ) in vec4 v2DTangentViewVector2_vUv2;

void main()
{
	vec4 vFinalColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	const float flNumLayers = 10.0;

	for ( float j = flNumLayers - 1.0; j >= 0.0; j -= 1.0 ) // From hightest to lowest layer
	{
		const vec4 vInnerTexel = tex2D( 0, g_tInnerSampler,
		    saturate( vUv01.xy + v2DTangentViewVector01.xy * 0.005 * j ) );
		const vec4 vMiddleTexel = tex2D( 1, g_tMiddleSampler,
		    saturate( vUv01.wz + v2DTangentViewVector01.wz * 0.005 * j ) );
		const vec4 vOuterTexel = tex2D( 2, g_tOuterSampler,
		    saturate( v2DTangentViewVector2_vUv2.wz + v2DTangentViewVector2_vUv2.xy * 0.005 * j ) );

		vec4 vThisTexel;
		vThisTexel.rgb = ( vInnerTexel.rgb * vInnerTexel.a ) + ( vMiddleTexel.rgb * vMiddleTexel.a ) +
		                 ( vOuterTexel.rgb * vOuterTexel.a );
		vThisTexel.a =
		    1.0 - ( ( 1.0 - vOuterTexel.a ) * ( 1.0 - vMiddleTexel.a ) * ( 1.0 - vInnerTexel.a ) );

		// 5.0 and 0.8625 are magic numbers that look good with the current textures
		const float flBlendValue = saturate(
		    pow( vThisTexel.a, mix( 5.0, 0.8625, saturate( j / ( flNumLayers - 1.0 ) ) ) ) );

		vFinalColor.rgb = vThisTexel.rgb + ( vFinalColor.rgb * ( 1.0 - flBlendValue ) );
		vFinalColor.a *= 1.0 - flBlendValue; // Dest alpha scalar
	}

	vec4 result;
	result.rgb = vFinalColor.rgb;
	result.a = 1.0 - vFinalColor.a;

	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR ) );
}
