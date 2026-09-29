// render.pass.output (RFC 0016, render.output.v1): exposure, the tone map and
// the output encoding, from the scene texel under the pixel to the target.
// A debug view (RFC 0014 "Post-processing is bypassed") skips exposure and
// the tone map and gets the encoding alone.
#version 450

#include "../../shaders/common/tone_map.glsl"
#include "../../shaders/common/color_encoding.glsl"

layout( set = 3, binding = 0 ) uniform texture2D sceneTexture;
layout( set = 3, binding = 1 ) uniform sampler sceneSampler;

layout( push_constant ) uniform Constants
{
	vec4 params; // x: exposure; y: scene peak; z: headroom
	uvec4 modes; // x: the encoding (kOutputEncoding*); y: 1 to tone map, 0 for a debug view
} constants;

layout( location = 0 ) out vec4 outColor;

void main()
{
	vec3 color =
	    texelFetch( sampler2D( sceneTexture, sceneSampler ), ivec2( gl_FragCoord.xy ), 0 ).rgb;
#if defined( SEEDED_DEBUG_VIEW_TONE_MAPPED )
	const bool toneMap = true;
#else
	const bool toneMap = constants.modes.y != 0u;
#endif
	if ( toneMap )
	{
#if defined( SEEDED_HEADROOM_IGNORED )
		color = OutputToneMap( color * constants.params.x, constants.params.y, 1.0 );
#else
		color = OutputToneMap( color * constants.params.x, constants.params.y, constants.params.z );
#endif
	}
	outColor = vec4( OutputEncode( color, constants.modes.x ), 1.0 );
}
