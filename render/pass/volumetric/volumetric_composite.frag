#version 450
// render.pass.volumetric composite stage (RFC 0016, the participating-media
// term): per pixel, front to back along the pixel's own ray through the
// froxel slices up to the scene depth, energy conserving (Hillaire 2015,
// after Wronski 2014). A slice's in-scattering S and extinction sigma_t are
// the inject stage's froxel averages, filtered bilinearly across froxel
// columns at the slice's centre (no filtering across slices); within half a
// froxel of the screen's edge, beyond the outermost column centres, they are
// extrapolated linearly from the two columns nearest the edge (clamped at 0),
// not held. A slice's ray length d is its depth extent (the last one ends at
// the surface) times the pixel's ray length at distance 1:
//
//     L += T S ( 1 - exp( -sigma_t d ) ) / sigma_t,   T *= exp( -sigma_t d )
//
// (S d where sigma_t is 0). Marching per pixel keeps the transmittance exact
// for a medium uniform across a froxel column, the screen's edges included.
// Written as ( L, T ) under transmittance blending (render.device.v2 D21:
// src + dst * a, the destination's alpha kept), so the frame becomes
// dst T + L with T at the target's own precision; at density zero ( 0, 1 )
// leaves it bitwise unchanged.
//
// SEEDED_EXTINCTION_TWICE, and SEEDED_SLICE_OFF_BY_ONE (volumetric_common.glsl's
// SliceOf), build the defective variants render.lab.volumetric's sensitivity
// row must reject; no product uses them.

#include "volumetric_common.glsl"

layout( set = 3, binding = 9 ) uniform sampler linearSampler;
layout( set = 3, binding = 7 ) uniform sampler pointSampler;
layout( set = 3, binding = 10 ) uniform texture3D injected;
layout( set = 3, binding = 12 ) uniform texture2D sceneDepth;

layout( location = 0 ) out vec4 outColor;

vec4 Fetch( vec2 columns, float w )
{
	// Column c's centre is at texture coordinate ( c + 0.5 ) / count.
	return textureLod( sampler3D( injected, linearSampler ),
	    vec3( ( columns + 0.5 ) / vec2( params.dims.xy ), w ), 0.0 );
}

void main()
{
	const vec2 pixel = gl_FragCoord.xy;
	const float depth = texelFetch( sampler2D( sceneDepth, pointSampler ), ivec2( pixel ), 0 ).r;
	const float distance = params.depth.y / ( depth + params.depth.x );
	const float rayLength = length( ViewRay( pixel ) );
	const uint last = SliceOf( distance );
	// The pixel in froxel-column units (column centres at integers), the
	// nearest point inside the centres' span, and how far beyond it the pixel
	// lies (0 inside; at most half a column at the screen's edge).
	const vec2 columns = pixel / params.screen.z - 0.5;
	const vec2 inside = clamp( columns, vec2( 0.0 ), vec2( params.dims.xy ) - 1.0 );
	const vec2 beyond = columns - inside;
	const vec2 inward = vec2( beyond.x > 0.0 ? -1.0 : 1.0, beyond.y > 0.0 ? -1.0 : 1.0 );
	const bool edge = any( notEqual( beyond, vec2( 0.0 ) ) );
	const float slices = float( params.dims.z );

	vec3 light = vec3( 0.0 );
	float transmittance = 1.0;
	for ( uint s = 0u; s <= last; ++s )
	{
		const float w = ( float( s ) + 0.5 ) / slices;
		vec4 slice = Fetch( inside, w );
		if ( edge )
		{
			const vec4 dx = beyond.x != 0.0 && params.dims.x > 1u
			                    ? slice - Fetch( inside + vec2( inward.x, 0.0 ), w )
			                    : vec4( 0.0 );
			const vec4 dy = beyond.y != 0.0 && params.dims.y > 1u
			                    ? slice - Fetch( inside + vec2( 0.0, inward.y ), w )
			                    : vec4( 0.0 );
			slice = max( slice + dx * abs( beyond.x ) + dy * abs( beyond.y ), vec4( 0.0 ) );
		}
		const float end = s < last ? sliceDepth[s + 1u] : distance;
		const float d = max( end - sliceDepth[s], 0.0 ) * rayLength;
		const float sigma = slice.a;
		const float through = exp( -sigma * d );
		const vec3 scattered = sigma > 1e-12 ? slice.rgb * ( ( 1.0 - through ) / sigma )
		                                     : slice.rgb * d;
		light += transmittance * scattered;
#ifdef SEEDED_EXTINCTION_TWICE
		transmittance *= through * through;
#else
		transmittance *= through;
#endif
	}
	outColor = vec4( light, transmittance );
}
