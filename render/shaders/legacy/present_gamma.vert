#version 450
// Present-time monitor gamma (render.gamma-ramp.v1): one triangle covering the
// swapchain image; v_uv spans the back buffer 0..1 over the visible area.
layout( location = 0 ) out vec2 v_uv;

void main()
{
	vec2 corner = vec2( ( gl_VertexIndex << 1 ) & 2, gl_VertexIndex & 2 );
	v_uv = corner;
	gl_Position = vec4( corner * 2.0 - 1.0, 0.0, 1.0 );
}
