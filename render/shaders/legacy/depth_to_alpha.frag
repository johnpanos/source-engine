#version 450
// Destination alpha as D3D9 PC holds it for the frame copies soft particles
// read (WRITE_DEPTH_TO_DESTALPHA, common_ps_fxc.h DepthToDestAlpha): the opaque
// scene's projected z over the dest-alpha depth range. Native passes do not
// write depth into alpha, so a copy of the frame gets it here from the copied
// depth buffer, written into the copy's alpha only. With MULTISAMPLE it reads a
// multisampled back buffer's depth and averages its samples' values, as D3D9's
// resolve averages the samples' destination alpha.
#ifdef MULTISAMPLE
layout( set = 0, binding = 0 ) uniform sampler2DMS u_depth;
#else
layout( set = 0, binding = 0 ) uniform sampler2D u_depth;
#endif
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

// The dest-alpha value of one depth sample.
float DestAlpha( float depth )
{
	// depth = projected z / w; solve for view z, then projected z.
	float viewZ = ( pc.projection.y - depth * pc.projection.w ) /
	              ( depth * pc.projection.z - pc.projection.x );
	float projZ = viewZ * pc.projection.x + pc.projection.y;
	return clamp( projZ * pc.invRange, 0.0, 1.0 );
}

void main()
{
	vec2 uv = pc.source.xy + v_uv * pc.source.zw;
#ifdef MULTISAMPLE
	ivec2 texel = ivec2( uv * vec2( textureSize( u_depth ) ) );
	int samples = textureSamples( u_depth );
	float alpha = 0.0;
	for ( int s = 0; s < samples; ++s )
		alpha += DestAlpha( texelFetch( u_depth, texel, s ).r );
	alpha /= float( samples );
#else
	float alpha = DestAlpha( texture( u_depth, uv ).r );
#endif
	o_color = vec4( 0.0, 0.0, 0.0, alpha );
}
