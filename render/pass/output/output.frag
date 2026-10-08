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
	vec4 extent; // xy: the target's extent in pixels; z: 1 to add the bloom;
	             // w: the motion blur's longest vector in screen fractions
	vec4 motion; // the motion blur: global vector xy, falling and roll intensities
} constants;

layout( location = 0 ) out vec4 outColor;

// The finished frame at a target pixel's center: exposure, the tone map and
// the bloom, in linear light.
vec3 Shade( vec2 pixel )
{
	// The texel under the pixel, or, when the extents differ, the scene
	// filtered at the pixel's center (the sampler is then linear).
	vec3 color = constants.modes.z != 0u
	                 ? texture( sampler2D( sceneTexture, sceneSampler ),
	                       pixel / constants.extent.xy ).rgb
	                 : texelFetch( sampler2D( sceneTexture, sceneSampler ),
	                       ivec2( pixel ), 0 ).rgb;
#if defined( SEEDED_DEBUG_VIEW_TONE_MAPPED )
	const bool toneMap = true;
#else
	const bool toneMap = constants.modes.y != 0u;
#endif
#if defined( SEEDED_BLOOM_BEFORE_TONE_MAP )
	if ( constants.extent.z != 0.0 )
		color = OutputLinearFromSrgb( OutputSrgbExtended( color ) + texture( sampler2D( bloomTexture,
		    bloomSampler ), pixel / constants.extent.xy ).rgb );
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
		    texture( sampler2D( bloomTexture, bloomSampler ), pixel / constants.extent.xy ).rgb );
#endif
	return color;
}

// The finished frame as the legacy frame buffer held it (sRGB encoded),
// filtered bilinearly at uv and clamped to the edge, as motion_blur_ps2x read
// _rt_FullFrameFB.
vec3 Frame( vec2 uv )
{
	const vec2 position = uv * constants.extent.xy - 0.5;
	const vec2 base = floor( position );
	const vec2 f = position - base;
	const vec2 last = constants.extent.xy - 1.0;
	vec3 taps[4];
	for ( int i = 0; i < 4; ++i )
	{
		const vec2 texel = clamp( base + vec2( i & 1, i >> 1 ), vec2( 0.0 ), last );
#if defined( SEEDED_MOTION_BLUR_LINEAR )
		taps[i] = Shade( texel + 0.5 );
#else
		taps[i] = OutputSrgbExtended( Shade( texel + 0.5 ) );
#endif
	}
	return mix( mix( taps[0], taps[1], f.x ), mix( taps[2], taps[3], f.x ), f.y );
}

// MotionBlur's pixel stage (motion_blur_ps2x.fxc): the global, falling and
// roll vectors, clamped in length, sampled 7, 11 or 15 times by the frame's
// height, averaged in the frame's encoding.
vec3 MotionBlur()
{
	const vec2 uv = gl_FragCoord.xy / constants.extent.xy;
	vec2 falling = uv * 2.0 - 1.0;
	vec2 roll = vec2( falling.y, -falling.x ) * constants.motion.w;
	vec2 global = vec2( constants.motion.x, -constants.motion.y );
	falling *= dot( falling, falling ) * -abs( constants.motion.z );
	vec2 blur = global + falling + roll;
	if ( length( blur ) > constants.extent.w )
		blur = normalize( blur ) * constants.extent.w;
	const int samples = constants.extent.y >= 1080.0 ? 15 : constants.extent.y >= 720.0 ? 11 : 7;
	const vec2 step = blur / float( samples - 1 );
	vec3 sum = vec3( 0.0 );
	for ( int i = 0; i < samples; ++i )
		sum += Frame( uv + step * float( i ) );
#if defined( SEEDED_MOTION_BLUR_LINEAR )
	return sum / float( samples );
#else
	return OutputLinearFromSrgb( sum / float( samples ) );
#endif
}

void main()
{
	const bool blurred = constants.motion != vec4( 0.0 );
	vec3 color = blurred ? MotionBlur() : Shade( gl_FragCoord.xy );
	if ( constants.modes.x == kOutputEncodingLinear )
		color *= constants.params.w;
	outColor = vec4( OutputEncode( color, constants.modes.x ), 1.0 );
}
