// render.pass.output (RFC 0016, render.output.v1): exposure, the tone map and
// the output encoding, from the scene texel under the pixel to the target.
// A debug view (RFC 0014 "Post-processing is bypassed") skips exposure and
// the tone map and gets the encoding alone.
#version 450

#include "../../shaders/common/tone_map.glsl"
#include "../../shaders/common/color_encoding.glsl"

layout( set = 3, binding = 0 ) uniform texture2D sceneTexture;
layout( set = 3, binding = 1 ) uniform sampler sceneSampler;
// render.pass.post's bloom (display-referred), or the scene again when absent.
layout( set = 3, binding = 2 ) uniform texture2D bloomTexture;
layout( set = 3, binding = 3 ) uniform sampler bloomSampler; // bilinear

layout( push_constant ) uniform Constants
{
	vec4 params; // x: exposure; y: scene peak; z: headroom
	uvec4 modes; // x: the encoding (kOutputEncoding*); y: 1 to tone map, 0 for a debug view;
	             // z: 1 when the scene's extent differs from the target's
	vec4 extent; // xy: the target's extent in pixels; z: 1 to add the bloom
} constants;

layout( location = 0 ) out vec4 outColor;

void main()
{
	// The texel under the pixel, or, when the extents differ, the scene
	// filtered at the pixel's center (the sampler is then linear).
	vec3 color = constants.modes.z != 0u
	                 ? texture( sampler2D( sceneTexture, sceneSampler ),
	                       gl_FragCoord.xy / constants.extent.xy ).rgb
	                 : texelFetch( sampler2D( sceneTexture, sceneSampler ),
	                       ivec2( gl_FragCoord.xy ), 0 ).rgb;
#if defined( SEEDED_DEBUG_VIEW_TONE_MAPPED )
	const bool toneMap = true;
#else
	const bool toneMap = constants.modes.y != 0u;
#endif
#if defined( SEEDED_BLOOM_BEFORE_TONE_MAP )
	if ( constants.extent.z != 0.0 )
		color = OutputLinearFromSrgb( OutputSrgbExtended( color ) + texture( sampler2D( bloomTexture,
		    bloomSampler ), gl_FragCoord.xy / constants.extent.xy ).rgb );
#endif
	if ( toneMap )
	{
#if defined( SEEDED_HEADROOM_IGNORED )
		color = OutputToneMap( color * constants.params.x, constants.params.y, 1.0 );
#else
		color = OutputToneMap( color * constants.params.x, constants.params.y, constants.params.z );
#endif
	}
#if !defined( SEEDED_BLOOM_BEFORE_TONE_MAP )
	// RFC 0016 K8 "Post and screen effects": the bloom is added to the
	// tone-mapped frame in its sRGB encoding, as engine_post and bloomadd
	// added it to the legacy frame buffer.
	if ( constants.extent.z != 0.0 )
		color = OutputLinearFromSrgb( OutputSrgbExtended( color ) +
		    texture( sampler2D( bloomTexture, bloomSampler ), gl_FragCoord.xy / constants.extent.xy ).rgb );
#endif
	if ( constants.modes.x == kOutputEncodingLinear )
		color *= constants.params.w;
	outColor = vec4( OutputEncode( color, constants.modes.x ), 1.0 );
}
