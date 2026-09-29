// render.material family `vertexlit` (RFC 0016 K4): the base texture times
// the material color and the interpolated vertex lighting, with $alpha times
// the base alpha and alpha test; the vertexlit_and_unlit_generic_ps20b port's
// DIFFUSELIGHTING path in linear light. Blending is pipeline state.
// The debug views (RFC 0014) come from debug_view.glsl: the vertex lighting
// mixes the ambient cube and the lights, so no light term is separable, and
// the furnace takes albedo 1 under a uniform radiance of 1.
#version 450

#include "../../shaders/common/debug_view.glsl"

layout( set = 2, binding = 0 ) uniform Material
{
	vec4 color; // rgb: $color, linear; a: $alpha
	vec4 flags; // x: $halflambert, y: $alphatest, z: $alphatestreference
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;

layout( location = 0 ) in vec2 uv;
layout( location = 1 ) in vec3 diffuseLighting;
layout( location = 0 ) out vec4 outColor;

void main()
{
	const vec4 baseColor = texture( sampler2D( baseTexture, baseSampler ), uv );
	const vec3 albedo = DebugFurnace() ? vec3( 1.0 ) : baseColor.rgb * material.color.rgb;
	const float alpha = material.color.a * baseColor.a;
	if ( material.flags.y != 0.0 && alpha < material.flags.z )
		discard;
	const vec3 color = albedo * ( DebugFurnace() ? vec3( 1.0 ) : diffuseLighting );
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasAo | kDebugHasUv0;
		inputs.albedo = albedo;
		inputs.uv0 = uv;
		inputs.final = color;
		outColor = DebugViewOutput( inputs );
		return;
	}
	outColor = vec4( color, alpha );
}
