#version 450
// render.cull suite fixture (test_cull_vulkan.cpp): a coloured box vertex,
// projected by the draw constants' view-projection rows (clip = M * p).

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 color;

layout( push_constant ) uniform Constants
{
	vec4 rows[4];
} constants;

layout( location = 0 ) out vec3 outColor;

void main()
{
	const vec4 p = vec4( position, 1.0 );
	gl_Position = vec4( dot( constants.rows[0], p ), dot( constants.rows[1], p ),
	    dot( constants.rows[2], p ), dot( constants.rows[3], p ) );
	outColor = color;
}
