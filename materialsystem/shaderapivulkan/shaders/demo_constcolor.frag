#version 450
// A material shader parameterized by a shader constant. The shared push-constant
// block is { mat4 mvp; vec4 color; }, so the color is at byte offset 64.
layout( location = 0 ) out vec4 outColor;
layout( push_constant ) uniform Constants { layout( offset = 64 ) vec4 color; } consts;
void main()
{
	outColor = consts.color;
}
