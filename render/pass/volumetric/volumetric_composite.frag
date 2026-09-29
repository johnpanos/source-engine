#version 450
// render.pass.volumetric composite stage (RFC 0016, the participating-media
// term): per pixel, front to back along the pixel's own ray through the
// froxel slices up to the scene depth, energy conserving (Hillaire 2015,
// after Wronski 2014). A slice's in-scattering S and extinction sigma_t are
// the inject stage's froxel averages, filtered bilinearly across froxel
// columns at the slice's centre (no filtering across slices); its ray length
// d is its depth extent (the last one ends at the surface) times the pixel's
// ray length at distance 1:
//
//     L += T S ( 1 - exp( -sigma_t d ) ) / sigma_t,   T *= exp( -sigma_t d )
//
// (S d where sigma_t is 0). Marching per pixel keeps the transmittance exact
// for a medium uniform across a froxel column, the screen's edges included,
// where a column integration at the column's centre would extrapolate its ray
// length. Written as ( L, 1 - T ) under premultiplied blending, rgb only: the
// frame becomes dst T + L, and at density zero ( 0, 0 ) leaves it bitwise
// unchanged.
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

void main()
{
	const vec2 pixel = gl_FragCoord.xy;
	const float depth = texelFetch( sampler2D( sceneDepth, pointSampler ), ivec2( pixel ), 0 ).r;
	const float distance = params.depth.y / ( depth + params.depth.x );
	const float rayLength = length( ViewRay( pixel ) );
	const uint last = SliceOf( distance );
	// Froxel column centres sit at ( column + 0.5 ) * tile pixels.
	const vec2 uv = pixel / ( params.screen.z * vec2( params.dims.xy ) );
	const float slices = float( params.dims.z );

	vec3 light = vec3( 0.0 );
	float transmittance = 1.0;
	for ( uint s = 0u; s <= last; ++s )
	{
		const vec4 slice = textureLod(
		    sampler3D( injected, linearSampler ), vec3( uv, ( float( s ) + 0.5 ) / slices ), 0.0 );
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
	outColor = vec4( light, 1.0 - transmittance );
}
