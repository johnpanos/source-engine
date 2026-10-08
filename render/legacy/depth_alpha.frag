// render.legacy-frontend's depth-alpha copy (RFC 0016 K9): destination alpha as
// D3D9 PC holds it for the frame copies soft particles read
// (WRITE_DEPTH_TO_DESTALPHA, DepthToDestAlpha): the opaque scene's projected z
// over the dest-alpha depth range, from the depth the scene drew with, into the
// copy's alpha only (the pipeline writes alpha alone). A copy keeps its texels'
// place, so the depth texel under each pixel is the one to read.
#version 450

layout( set = 3, binding = 0 ) uniform texture2D depthTexture;
layout( set = 3, binding = 1 ) uniform sampler depthSampler;

layout( push_constant ) uniform Constants
{
	// The draw projection's z column (row-vector matrices): projected z =
	// view z * zScale + zOffset, w = view z * wScale + wOffset.
	vec4 projection; // zScale, zOffset, wScale, wOffset
	vec4 range;      // x: 1 / the dest-alpha depth range
} constants;

layout( location = 0 ) out vec4 outColor;

void main()
{
	const float depth =
	    texelFetch( sampler2D( depthTexture, depthSampler ), ivec2( gl_FragCoord.xy ), 0 ).r;
	const vec4 p = constants.projection;
	// depth = projected z / w; solve for view z, then projected z.
	const float viewZ = ( p.y - depth * p.w ) / ( depth * p.z - p.x );
	const float projected = viewZ * p.x + p.y;
#if defined( SEEDED_IGNORES_RANGE )
	outColor = vec4( 0.0, 0.0, 0.0, clamp( projected, 0.0, 1.0 ) );
#else
	outColor = vec4( 0.0, 0.0, 0.0, clamp( projected * constants.range.x, 0.0, 1.0 ) );
#endif
}
