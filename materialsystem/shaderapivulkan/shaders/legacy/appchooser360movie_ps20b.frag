#version 450
// A port of stdshaders/appchooser360movie_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws with $x360appchooser: the texture decoded from sRGB
// in shader math, times the vertex color (TEXCOORD3), with its alpha.
// @legacy program=appchooser360movie ps=appchooser360movie_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;
layout( location = 3 ) in vec4 vColor;

void main()
{
	vec4 vTextureColor = tex2D( 0, TexSampler, baseTexCoord );
	vTextureColor.r = SrgbGammaToLinear( vTextureColor.r );
	vTextureColor.g = SrgbGammaToLinear( vTextureColor.g );
	vTextureColor.b = SrgbGammaToLinear( vTextureColor.b );

	vec4 result;
	result.rgb = vTextureColor.rgb * vColor.rgb;
	result.a = vColor.a;

	LegacyWrite( result );
}
