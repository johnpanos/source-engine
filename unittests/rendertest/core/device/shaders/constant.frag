// render.device.v2 D16 fixture: the draw constants' color.
#version 450

layout( push_constant ) uniform Constants
{
	vec4 color;
} constants;

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = constants.color;
}
