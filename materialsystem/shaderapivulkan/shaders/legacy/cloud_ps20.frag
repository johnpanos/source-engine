#version 450
// Cloud_dx9's pixel stage: a port of stdshaders/cloud_ps20.fxc (ps20, no
// combos). The base texture times the cloud alpha texture, with alpha scaled
// by a smoothstep of the vertex fog factor; the color is scaled by
// LINEAR_LIGHT_SCALE and never fogged.
// @legacy program=cloud ps=cloud_ps20 vs=cloud_vs20 vert=cloud_vs20
//         samplers=0:2d,1:2d
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D BaseTextureSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D CloudAlphaSampler;  // s1

layout( location = 0 ) in vec2 baseCoords;
layout( location = 1 ) in vec2 cloudAlphaCoords;
layout( location = 2 ) in float fogFactorIn;

void main()
{
	vec4 vBase = tex2D( 0, BaseTextureSampler, baseCoords );
	vec4 vCloudAlpha = tex2D( 1, CloudAlphaSampler, cloudAlphaCoords );

	float fogFactor = 2.0 * smoothstep( 0.3, 0.6, fogFactorIn );

	vec4 result = vBase * vCloudAlpha;
	result.a *= fogFactor;

	LegacyWrite( FinalOutput( result, 1.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_LINEAR ) );
}
