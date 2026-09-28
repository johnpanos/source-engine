// render.pass.opaque (RFC 0016 K5): scene draws. The view group holds the
// view-projection, the draw group each drawn instance's record (world matrix
// and color), selected by the draw's first instance. Matrices are row-major
// with column vectors, as render.math stores them.
#version 450

layout( location = 0 ) in vec3 position;

layout( set = 1, binding = 0, row_major ) uniform View { mat4 viewProjection; };

struct Instance
{
	mat4 world;
	vec4 color;
};
layout( std430, set = 3, binding = 0, row_major ) readonly buffer Instances { Instance instances[]; };

layout( location = 0 ) flat out vec4 color;

void main()
{
	const Instance instance = instances[gl_InstanceIndex];
	gl_Position = viewProjection * ( instance.world * vec4( position, 1.0 ) );
	color = instance.color;
}
