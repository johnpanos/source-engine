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
	// The view's fog (common_ps_fxc.h CalcPixelFogFactor, BlendPixelFog):
	// color with its type in w (-1 none, 0 range, 1 height), parameters
	// (range: start / range, water z, max density, 1 / range; height: 0,
	// water z, 1, 1 / range), and the eye's world z in misc.x.
	vec4 fogColor;
	vec4 fogParams;
	vec4 fogMisc;
} frame;
layout( set = 2, binding = 0 ) uniform Material
{
	vec4 tint;  // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $alphatest, z: $alphatestreference, w: 1 when lighting is one (unlit)
	vec4 state; // x: 1 when fully opaque (height fog's factor is the output alpha), y: gamma vertex color (vertex stage)
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
layout( location = 3 ) in vec2 fogDepth; // the clip-space z (D3D9's projPos.z) and world z
layout( location = 0 ) out vec4 outColor;

float FogFactor()
{
	const float type = frame.fogColor.w;
	if ( type < -0.5 )
		return 0.0;
	const float projZ = fogDepth.x;
	if ( type < 0.5 )
		return clamp( min( frame.fogParams.z, projZ * frame.fogParams.w - frame.fogParams.x ), 0.0,
		    1.0 );
	const float depthFromWater = frame.fogParams.y - fogDepth.y;
	const float depthFromEye = frame.fogMisc.x - fogDepth.y;
	const float f = clamp( depthFromWater * ( 1.0 / depthFromEye ), 0.0, 1.0 );
	return clamp( f * projZ * frame.fogParams.w, 0.0, 1.0 );
}

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
	// The view's fog, after the tone-mapping scale (its color is scaled too):
	// range fog squares its factor; a fully opaque surface under height fog
	// writes the factor to alpha.
	const float fogType = frame.fogColor.w;
	if ( fogType > -0.5 )
	{
		const float factor = FogFactor();
		if ( fogType > 0.5 && material.state.x != 0.0 )
			alpha = factor;
		lit = mix( lit, frame.fogColor.rgb, fogType < 0.5 ? factor * factor : factor );
	}
	if ( frame.light.z != 0.0 )
		lit = LinearToSrgb( lit );
	outColor = vec4( lit, alpha );
}
