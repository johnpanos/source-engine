#version 450
// render.device.v2 suite fixture (test_device_vulkan.cpp kSampledFragment): the
// bound image at this pixel. Embedded as built by the pinned compiler with
// --target-env=vulkan1.1 -O; tools/render/shader_toolchain.py check rebuilds
// and compares it.

layout( set = 2, binding = 0 ) uniform texture2D image;
layout( set = 2, binding = 1 ) uniform sampler point;

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = texture( sampler2D( image, point ),
	                    gl_FragCoord.xy / vec2( textureSize( sampler2D( image, point ), 0 ) ) );
}
