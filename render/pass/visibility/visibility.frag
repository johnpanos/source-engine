// render.pass.visibility (RFC 0016 K8): the proxy writes no color (the
// pipeline masks it); only its samples count, under an occlusion query.
#version 450

layout( location = 0 ) out vec4 outColor;

void main()
{
	outColor = vec4( 0.0 );
}
