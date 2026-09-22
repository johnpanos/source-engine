#version 450
// Textured dynamic-mesh vertex shader: applies the material mvp (c0-c3) and
// passes the texture coordinate. Reads pos (loc 0) and uv (loc 2) from the
// shared pos+color+uv vertex; ignores the color at loc 1.
layout( location = 0 ) in vec3 inPos;
layout( location = 2 ) in vec2 inUv;
layout( location = 0 ) out vec2 fragUv;
layout( push_constant ) uniform Constants { mat4 mvp; vec4 color; } consts;
void main()
{
	gl_Position = consts.mvp * vec4( inPos, 1.0 );
	fragUv = inUv;
}
