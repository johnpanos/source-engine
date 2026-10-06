#version 450
// render.cull suite fixture (test_cull_vulkan.cpp): the box's colour.

layout( location = 0 ) in vec3 color;
layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = vec4( color, 1.0 );
}
