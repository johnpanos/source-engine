// render.pass.lines (RFC 0016): the vertex color, alpha-blended by the
// pipeline.
#version 450

layout( location = 0 ) in vec4 vertexColor;
layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = vertexColor;
}
