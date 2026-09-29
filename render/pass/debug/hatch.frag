// render.pass.debug (RFC 0014): the not-applicable hatch over the whole
// target, debug_view.glsl's one definition of it. A target without an sRGB
// view takes the encoded value (kEncode), as the programs' outputs do.
#version 450

#include "../../shaders/common/color_encoding.glsl"
#include "../../shaders/common/debug_view.glsl"

layout( constant_id = 0 ) const bool kEncode = false;

layout( location = 0 ) out vec4 outColor;

void main()
{
	const vec3 hatch = DebugHatch();
	outColor = vec4( kEncode ? LinearToSrgb( hatch ) : hatch, 1.0 );
}
