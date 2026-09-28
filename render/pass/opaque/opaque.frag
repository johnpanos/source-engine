// render.pass.opaque (RFC 0016 K5): the instance's color, until material
// families (K4) supply the pipelines.
#version 450

layout( location = 0 ) flat in vec4 color;
layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = color;
}
