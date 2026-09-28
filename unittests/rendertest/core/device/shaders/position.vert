#version 450
// render.device.v2 suite fixture (test_device_vulkan.cpp kPositionVertex): a
// vec2 position at clip z 0.5. Embedded as built by the pinned compiler with
// --target-env=vulkan1.1 -O; tools/render/shader_toolchain.py check rebuilds
// and compares it.

layout( location = 0 ) in vec2 position;

void main()
{
	gl_Position = vec4( position, 0.5, 1.0 );
}
