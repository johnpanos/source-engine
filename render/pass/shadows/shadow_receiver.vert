#version 450
// render.pass.shadows receiver pass (RFC 0016 K7): receivers drawn from the
// camera. The view group holds the camera and the light (shadow_receiver.frag),
// the draw group each receiver's world matrix, selected by the draw's first
// instance. Matrices are row-major with column vectors.

layout( location = 0 ) in vec3 position;

struct ShadowTile
{
	mat4 viewProjection;
	vec4 transform;
	vec4 bounds;
	vec4 params;
};

layout( std140, set = 1, binding = 0, row_major ) uniform Receiver
{
	mat4 viewProjection;
	mat4 view;
	vec4 lightPositionKind;
	vec4 lightAxisCos;
	vec4 lightRange;
	vec4 cascadeSplits;
	ShadowTile tiles[4];
}
receiver;

layout( std430, set = 3, binding = 0, row_major ) readonly buffer Receivers
{
	mat4 world[];
};

layout( location = 0 ) out vec3 worldPosition;
layout( location = 1 ) out float viewDistance;

void main()
{
	const vec4 p = world[gl_InstanceIndex] * vec4( position, 1.0 );
	worldPosition = p.xyz;
	viewDistance = -( receiver.view * p ).z;
	gl_Position = receiver.viewProjection * p;
}
