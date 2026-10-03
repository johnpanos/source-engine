#version 450
// D24: compare before filtering, equality and clamp-to-edge. A four-column
// depth fixture holds 0.25, 0.5, 0.75 and 1.0 on every row.
layout( set = 2, binding = 0 ) uniform texture2D image;
layout( set = 2, binding = 1 ) uniform sampler comparison;
layout( location = 0 ) out vec4 outColor;
void main()
{
	const vec2 uv = gl_FragCoord.xy / 4.0;
	outColor = vec4(
	    textureLod( sampler2DShadow( image, comparison ), vec3( uv + vec2( 0.125, 0 ), 0.5 ), 0 ),
	    textureLod( sampler2DShadow( image, comparison ), vec3( uv, 0.5 ), 0 ),
	    textureLod( sampler2DShadow( image, comparison ), vec3( -1, -1, 0.5 ), 0 ), 1 );
}
