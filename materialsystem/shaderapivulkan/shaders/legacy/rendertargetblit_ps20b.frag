#version 450
// A port of stdshaders/rendertargetblit_ps2x.fxc (ps20b, no combos), which
// screenspace_general draws by $pixshader: the texture as is.
// @legacy program=rendertargetblit ps=rendertargetblit_ps20b vs=screenspaceeffect_vs20
//         vert=screenspaceeffect_vs20 samplers=0:2d flags=object_position
#include "legacy_ps.glsl"

layout( set = 0, binding = 0 ) uniform sampler2D TexSampler; // s0

layout( location = 0 ) in vec2 baseTexCoord;

void main()
{
	vec4 result = tex2D( 0, TexSampler, baseTexCoord );
	LegacyWrite( FinalOutput( result, 0.0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE ) );
}
