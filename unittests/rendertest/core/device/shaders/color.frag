#version 450
// render.device.v2 suite fixture (test_shaders.h kColorFragment): the material
// block's color. Embedded as built by the pinned compiler with --target-
// env=vulkan1.1 -O; tools/render/shader_toolchain.py check rebuilds and
// compares it.

layout( set = 2, binding = 0 ) uniform Material
{
	vec4 color;
} material;

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = material.color;
}
