#version 450
// Destination alpha as D3D9 PC holds it for the frame copies soft particles
// read (WRITE_DEPTH_TO_DESTALPHA, common_ps_fxc.h DepthToDestAlpha): the opaque
// scene's projected z over the dest-alpha depth range. Native passes do not
// write depth into alpha, so a copy of the frame gets it here from the copied
// depth buffer, written into the copy's alpha only.
layout( set = 0, binding = 0 ) uniform sampler2D u_depth;
layout( push_constant ) uniform DepthToAlpha
{
	// The draw projection's z column (row-vector matrices): projected z =
	// view z * zScale + zOffset, w = view z * wScale + wOffset.
	vec4 projection; // zScale, zOffset, wScale, wOffset
	vec4 source;     // the copied rectangle in the depth image: uv offset, uv scale
	float invRange;  // 1 / the dest-alpha depth range
}
pc;

layout( location = 0 ) in vec2 v_uv;
layout( location = 0 ) out vec4 o_color;

void main()
{
	float depth = texture( u_depth, pc.source.xy + v_uv * pc.source.zw ).r;
	// depth = projected z / w; solve for view z, then projected z.
	float viewZ = ( pc.projection.y - depth * pc.projection.w ) /
	              ( depth * pc.projection.z - pc.projection.x );
	float projZ = viewZ * pc.projection.x + pc.projection.y;
	o_color = vec4( 0.0, 0.0, 0.0, clamp( projZ * pc.invRange, 0.0, 1.0 ) );
}
