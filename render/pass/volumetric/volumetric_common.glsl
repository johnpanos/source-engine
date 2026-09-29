// render.pass.volumetric (RFC 0016, the participating-media term): what every
// stage reads. See public/render/pass/volumetric/volumetric.h.
//
// VolumetricParamsGpu (binding 0) and the grid's slice depths (binding 1),
// in the dispatch's own group (the draw role).

layout( std140, set = 3, binding = 0, row_major ) uniform VolumetricParams
{
	uvec4 dims;   // froxels x, y, slices, samplesXY
	uvec4 counts; // lights, volumes, projectors, samplesDepth
	vec4 screen;  // width, height, tile size in pixels, density scale
	vec4 slicing; // sliceScale, sliceBias, nearZ, farZ
	vec4 rays;    // the view ray at z = -1 at the top-left corner (xy), its change across the screen (zw)
	vec4 depth;   // projection rows[2].z, rows[2].w
	mat4 viewToWorld;
	vec4 eye;
	vec4 fog0; // global sigma_t, height sigma_t, height falloff, base height
	vec4 fog1; // global and height albedo, anisotropy
}
params;

// ClusterGrid::sliceDepths: slices + 1 view distances.
layout( std430, set = 3, binding = 1 ) readonly buffer SliceDepths
{
	float sliceDepth[];
};

// The view-space point at distance 1 under a pixel position (x right, y down,
// in pixels; positions outside the screen extend the same rays).
vec3 ViewRay( vec2 pixel )
{
	const vec2 at = pixel / params.screen.xy;
	return vec3( params.rays.xy + params.rays.zw * at, -1.0 );
}

// The slice holding a view distance, as ClusterGrid's SliceOfDepth, then
// corrected against the slice depths so sliceDepth[s] <= d < sliceDepth[s + 1]
// (the formula's rounding may differ from the depths by one).
uint SliceOf( float distance )
{
	const int slices = int( params.dims.z );
	int s = int( floor( log( max( distance, 1e-30 ) ) * params.slicing.x + params.slicing.y ) );
#ifdef SEEDED_SLICE_OFF_BY_ONE
	s += 1;
#endif
	s = clamp( s, 0, slices - 1 );
#ifndef SEEDED_SLICE_OFF_BY_ONE
	if ( s > 0 && distance < sliceDepth[s] )
		--s;
	if ( s < slices - 1 && distance >= sliceDepth[s + 1] )
		++s;
#endif
	return uint( s );
}
