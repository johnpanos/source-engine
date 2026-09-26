#version 450
// floattoscreen's pixel stage (and floattoscreen_vanilla's, whose static shader
// is this one): a port of stdshaders/floattoscreen_ps2x.fxc (ps20b,
// HDR_TYPE_FLOAT), the frame scaled by cLightScale. Combos
// (fxctmp9/floattoscreen_ps20b.inc): static CONVERT_TO_SRGB (1, always 0 here).
// @legacy program=floattoscreen ps=floattoscreen_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D FBSampler; // s0

layout( location = 0 ) in vec2 texCoord;

void main()
{
	vec4 fbSample = tex2D( 0, FBSampler, texCoord ) * vec4( cLightScale.xyz, 1.0 );
	LegacyWrite( FinalOutput( fbSample, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
