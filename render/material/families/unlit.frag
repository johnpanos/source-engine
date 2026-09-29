// render.material family `unlit` (RFC 0016 K4): the base texture times the
// material color and alpha, times the vertex color and alpha when the
// material asks for them, with alpha test; the arithmetic of the
// vertexlit_and_unlit_generic port without lighting, in the port's gamma
// space. Blending is pipeline state (UnlitFamily::BlendFor). The debug views
// (RFC 0014) come from debug_view.glsl.
#version 450

#include "../../shaders/common/debug_view.glsl"

layout( set = 2, binding = 0 ) uniform Material
{
	vec4 color; // rgb: $color, a: $alpha
	vec4 flags; // x: $vertexcolor, y: $vertexalpha, z: $alphatest, w: $alphatestreference
} material;
layout( set = 2, binding = 1 ) uniform texture2D baseTexture;
layout( set = 2, binding = 2 ) uniform sampler baseSampler;

layout( location = 0 ) in vec2 uv;
layout( location = 1 ) in vec4 color;
layout( location = 0 ) out vec4 outColor;

void main()
{
	vec4 result = texture( sampler2D( baseTexture, baseSampler ), uv ) * material.color;
	if ( material.flags.x != 0.0 )
		result.rgb *= color.rgb;
	if ( material.flags.y != 0.0 )
		result.a *= color.a;
	if ( material.flags.z != 0.0 && result.a < material.flags.w )
		discard;
	if ( DebugViewActive() )
	{
		DebugInputs inputs = DebugInputsNone();
		inputs.mask = kDebugHasAlbedo | kDebugHasUv0 | kDebugHasVertexColor;
		inputs.albedo = result.rgb;
		inputs.uv0 = uv;
		inputs.vertexColor = color;
		inputs.final = result.rgb;
		outColor = DebugViewOutput( inputs );
		return;
	}
	outColor = result;
}
