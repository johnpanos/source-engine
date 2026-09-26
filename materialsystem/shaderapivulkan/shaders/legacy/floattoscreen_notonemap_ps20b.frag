#version 450
// A port of stdshaders/floattoscreen_notonemap_ps2x.fxc (ps20b,
// HDR_TYPE_FLOAT), the frame as is (its luminance is computed and unused);
// floattoscreen and screenspace_general select it by $pixshader
// (dev/copyfullframefb_vanilla). Combos (fxctmp9/floattoscreen_notonemap_ps20b.inc):
// static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=floattoscreen_notonemap ps=floattoscreen_notonemap_ps20b
//         vs=screenspaceeffect_vs20 vert=screenspaceeffect_vs20 samplers=0:2d
//         flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FBSampler; // s0

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec4 fbSample = tex2D( 0, FBSampler, texCoord );
	LegacyWrite( FinalOutput( fbSample, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
