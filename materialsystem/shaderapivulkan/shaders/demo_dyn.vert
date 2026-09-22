#version 450
// Dynamic-mesh vertex shader: applies the model->projection transform the
// material system supplies in vertex-shader constant registers c0-c3 (mat4),
// exactly like a real Source vertex shader. Identity leaves geometry in place.
layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec3 inColor;
layout( location = 0 ) out vec3 fragColor;
layout( push_constant ) uniform Constants { mat4 mvp; vec4 color; } consts;
void main()
{
	gl_Position = consts.mvp * vec4( inPos, 1.0 );
	fragColor = inColor;
}
