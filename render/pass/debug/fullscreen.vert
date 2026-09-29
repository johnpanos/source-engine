// render.pass.debug (RFC 0014): one triangle covering the target.
#version 450

void main()
{
	const vec2 corner = vec2( ( gl_VertexIndex << 1 ) & 2, gl_VertexIndex & 2 );
	gl_Position = vec4( corner * 2.0 - 1.0, 0.0, 1.0 );
}
