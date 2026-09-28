// render.material family `unlit` (RFC 0016 K4): UnlitGeneric's claimed
// subset. The draw constants carry the draw's world-to-clip matrix (row-major
// with column vectors, as render.math stores them).
#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec2 uv0;
layout( location = 2 ) in vec4 vertexColor;

layout( push_constant ) uniform Draw
{
	layout( row_major ) mat4 toClip;
} draw;

layout( location = 0 ) out vec2 uv;
layout( location = 1 ) out vec4 color;

void main()
{
	gl_Position = draw.toClip * vec4( position, 1.0 );
	uv = uv0;
	// Vertex colors are gamma-space, like the port's GammaToLinear (pow 2.2).
	color = vec4( pow( vertexColor.rgb, vec3( 2.2 ) ), vertexColor.a );
}
