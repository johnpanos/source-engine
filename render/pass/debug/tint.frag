// render.pass.debug (RFC 0014): a flat color blended over the whole target
// (cl_render_debug_legacy 1's magenta over what the core did not draw). The
// color is linear; a target without an sRGB view takes it encoded (kEncode).
#version 450

#include "../../shaders/common/color_encoding.glsl"

layout( constant_id = 0 ) const bool kEncode = false;

layout( push_constant ) uniform Tint
{
	vec4 color; // rgb linear, a the blend weight
} tint;

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = vec4( kEncode ? LinearToSrgb( tint.color.rgb ) : tint.color.rgb, tint.color.a );
}
