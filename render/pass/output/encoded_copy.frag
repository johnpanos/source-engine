// render.pass.output's encoded copy: a frame that already holds its output
// encoding (the core shader API's colour target, 8-bit display values) into
// a presentation's back buffer of another colour format, texel for texel at
// the same place. The values are not decoded or encoded again; alpha is 1,
// as an opaque presentation shows it.
#version 450

layout( set = 3, binding = 0 ) uniform texture2D sourceTexture;
layout( set = 3, binding = 1 ) uniform sampler sourceSampler;

layout( location = 0 ) out vec4 outColor;

void main()
{
	const vec3 encoded =
	    texelFetch( sampler2D( sourceTexture, sourceSampler ), ivec2( gl_FragCoord.xy ), 0 ).rgb;
#if defined( SEEDED_ENCODED_COPY_SWAPS_RED_BLUE )
	outColor = vec4( encoded.bgr, 1.0 );
#else
	outColor = vec4( encoded, 1.0 );
#endif
}
