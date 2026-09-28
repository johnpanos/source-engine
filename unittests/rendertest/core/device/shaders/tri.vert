#version 450
// render.device.v2 suite fixture (test_shaders.h kFullScreenVertex): a
// full-screen triangle at clip z 0.25. Embedded as built by the pinned compiler
// with --target-env=vulkan1.1 -O; tools/render/shader_toolchain.py check
// rebuilds and compares it.

void main()
{
	vec2 p = vec2( ( gl_VertexIndex << 1 ) & 2, gl_VertexIndex & 2 );
	gl_Position = vec4( p * 2.0 - 1.0, 0.25, 1.0 );
}
