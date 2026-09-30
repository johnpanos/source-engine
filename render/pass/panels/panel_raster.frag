// render.pass.panels (RFC 0016): the texture modulated by the quad's color,
// in gamma space as the legacy 2D path blends (the image is an unorm target;
// the surface reads it through its sRGB view). The pipeline blends.
#version 450

layout( set = 2, binding = 0 ) uniform texture2D sourceTexture;
layout( set = 2, binding = 1 ) uniform sampler sourceSampler;

layout( push_constant ) uniform Constants
{
	layout( row_major ) mat4 toClip;
	vec4 params; // x: 1 when the texture's alpha is not read; y: 1 to premultiply (additive)
}
constants;

layout( location = 0 ) in vec2 vertexUv;
layout( location = 1 ) in vec4 vertexColor;
layout( location = 0 ) out vec4 outColor;

void main()
{
	vec4 texel = texture( sampler2D( sourceTexture, sourceSampler ), vertexUv );
	if ( constants.params.x > 0.5 )
		texel.a = 1.0;
	vec4 color = texel * vertexColor;
	if ( constants.params.y > 0.5 )
		color.rgb *= color.a;
	outColor = color;
}
