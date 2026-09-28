#version 450
// render.pass.shadows caster depth (RFC 0016 K7, render.shadows.v1): each
// caster drawn into its view's atlas tile. The draw group holds one clip
// matrix per (view, caster) pair, the view's world to clip times the
// caster's world matrix (composed in double on the CPU), selected by the
// draw's first instance. The viewport places the view in its tile; the
// pipeline has no fragment stage (depth only). Matrices are row-major with
// column vectors, as render.math stores them.

layout( location = 0 ) in vec3 position;

layout( std430, set = 3, binding = 0, row_major ) readonly buffer Casters
{
	mat4 clipFromObject[];
};

void main()
{
	gl_Position = clipFromObject[gl_InstanceIndex] * vec4( position, 1.0 );
}
