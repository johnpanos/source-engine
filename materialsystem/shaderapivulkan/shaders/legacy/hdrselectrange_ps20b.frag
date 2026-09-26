#version 450
// HDRSelectRange's pixel stage: a port of stdshaders/HDRSelectRange_ps2x.fxc
// (ps20b). The shader writes two render targets: COLOR0 the low range (with
// its blend factor fixed at 1, black) and COLOR1 the high range. This backend
// draws into one color attachment, so the port writes COLOR0 only. Combos
// (fxctmp9/HDRSelectRange_ps20b.inc): static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=hdrselectrange ps=hdrselectrange_ps20b vs=hdrselectrange_vs20
//         vert=hdrselectrange_vs20 samplers=0:2d,1:2d flags=object_position
#include "legacy_ps.glsl"
#include "legacy_color.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D LowSampler; // s0
layout( set = 0, binding = 1 ) uniform sampler2D HiSampler;  // s1

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec3 lowColor = GammaToLinear( tex2D( 0, LowSampler, texCoord ).rgb );
	vec3 hiColor = GammaToLinear( tex2D( 1, HiSampler, texCoord ).rgb );

	vec4 lowOut;
	lowOut.a = 1.0;
	vec4 hiOut;
	hiOut.a = 1.0;

	float blendFactor = 1.0;

	lowOut.rgb = LinearToGamma( lowColor * ( 1.0 - blendFactor ) );
	hiOut.rgb = LinearToGamma( hiColor * ( blendFactor ) );
	// output.color[1] = hiOut: the second render target.
	LegacyWrite( lowOut );
}
