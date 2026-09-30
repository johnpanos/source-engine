// render.pass.panels (RFC 0016; render.world-panel.v1 coatings): a coating
// quad (grime on the screen's face) into the coating target: its color in
// linear light, premultiplied by its alpha, composited by the pipeline's
// premultiplied blend over nothing. The mips kernel reads the result as the
// emission's transmittance ((1 - A.a) + A.rgb) and the face's albedo (A.rgb).
#version 450

layout( set = 2, binding = 0 ) uniform texture2D sourceTexture;
layout( set = 2, binding = 1 ) uniform sampler sourceSampler;

layout( push_constant ) uniform Constants
{
	layout( row_major ) mat4 toClip;
	vec4 params; // unused by coatings
}
constants;

layout( location = 0 ) in vec2 vertexUv;
layout( location = 1 ) in vec4 vertexColor;
layout( location = 0 ) out vec4 outCoating;

vec3 Decode( vec3 c )
{
	return mix( c / 12.92, pow( ( c + 0.055 ) / 1.055, vec3( 2.4 ) ), step( vec3( 0.04045 ), c ) );
}

void main()
{
	const vec4 color = texture( sampler2D( sourceTexture, sourceSampler ), vertexUv ) * vertexColor;
	outCoating = vec4( Decode( color.rgb ) * color.a, color.a );
}
