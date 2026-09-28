// render.pass.lines (RFC 0016): world- or screen-space lines and flat-colored
// triangles. The draw constants carry the draw's to-clip matrix (the view's
// world-to-clip, or pixels-to-clip for screen space; row-major with column
// vectors, as render.math stores them) and a clip-space depth offset that
// pulls depth-tested lines toward the eye.
#version 450

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec4 color;

layout( push_constant ) uniform Constants
{
	layout( row_major ) mat4 toClip;
	vec4 params; // x: clip-space z offset toward the eye; y: 1 on an sRGB target
} constants;

layout( location = 0 ) out vec4 vertexColor;

void main()
{
	vec4 clip = constants.toClip * vec4( position, 1.0 );
	clip.z -= constants.params.x;
	gl_Position = clip;
	// Display colors, decoded to linear light for an sRGB target (the exact
	// sRGB curve, so the target's encode gives the written bytes back).
	vec3 linear = mix( color.rgb / 12.92, pow( ( color.rgb + 0.055 ) / 1.055, vec3( 2.4 ) ),
	    step( vec3( 0.04045 ), color.rgb ) );
	vertexColor = vec4( constants.params.y > 0.5 ? linear : color.rgb, color.a );
}
