#version 450
// Fragment color comes entirely from a uniform (constant) buffer, so reading
// the rendered pixel back proves the constant reached the shader.
layout( location = 0 ) out vec4 outColor;
layout( binding = 0 ) uniform Constants { vec4 color; } consts;
void main()
{
	outColor = consts.color;
}
