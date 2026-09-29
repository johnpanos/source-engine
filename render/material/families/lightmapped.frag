// render.material family `lightmapped` (RFC 0016 K4): the base texture times
// the lightmap times the material tint, with the vertex color and alpha when
// the material asks for them and alpha test; the non-bumped path of the
// lightmappedgeneric port, in linear light. Blending is pipeline state.
#version 450

#include "../../shaders/common/color_encoding.glsl"

layout( set = 0, binding = 0 ) uniform Frame
{
	// x: the lightmap scale for how the pages encode light (2^2.2 for LDR
	// gamma pages, 16 for integer-HDR pages); y: the output's linear scale
	// (the frame's tone-mapping scale, 1 without HDR); z: 1 when the target
	// has no sRGB view and the shader encodes the output itself.
	vec4 light;
} frame;
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference, w: 1 when lighting is one (unlit)
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;
// The lightmap page is the draw's: surfaces of one material share pages
// with others.
layout( set = 3, binding = 0 ) uniform texture2D lightmap;
layout( set = 3, binding = 1 ) uniform sampler lightmapSampler;

layout( location = 0 ) in vec2 baseUv;
layout( location = 1 ) in vec2 lightmapUv;
layout( location = 2 ) in vec4 color;
layout( location = 0 ) out vec4 outColor;

void main()
{
	const vec4 base = texture( sampler2D( baseTexture, baseSampler ), baseUv );
	const vec3 light = texture( sampler2D( lightmap, lightmapSampler ), lightmapUv ).rgb;
	vec3 albedo = base.rgb;
	// The port's vertex fast path (no texture transform, no detail): the
	// vertex color replaces the modulation alpha, which otherwise applies
	// $alpha a second time.
	float alpha = base.a * material.tint.a;
	if ( material.flags.x != 0.0 )
	{
		albedo *= color.rgb;
		alpha *= color.a;
	}
	else
	{
		alpha *= material.tint.a;
	}
	if ( material.flags.y != 0.0 && alpha < material.flags.z )
		discard;
	// Unlit is this term with the lighting fixed at one.
	const vec3 lighting = material.flags.w != 0.0 ? vec3( 1.0 ) : light * frame.light.x;
	vec3 lit = albedo * lighting * material.tint.rgb * frame.light.y;
	if ( frame.light.z != 0.0 )
		lit = LinearToSrgb( lit );
	outColor = vec4( lit, alpha );
}
