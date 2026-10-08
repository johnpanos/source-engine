// render.pass.visibility (RFC 0016 K8): the client's pixel visibility proxy,
// already in clip space (render/pass/visibility/visibility.h).
#version 450

layout( location = 0 ) in vec4 clipPosition;

void main()
{
	gl_Position = clipPosition;
}
