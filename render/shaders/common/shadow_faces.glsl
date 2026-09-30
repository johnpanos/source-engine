// render.shadows.v1: the visibility of a point from a light shadowed by
// several perspective tiles (the faces of a cube or hemicube). The includer
// includes shadow_sample.glsl first and declares its tile list as
// `shadowTiles` (ShadowTile records) before including this file.
#ifndef SHADOW_FACES_GLSL
#define SHADOW_FACES_GLSL

// A light shadowed by `count` perspective tiles from its position (the faces
// of a cube or hemicube, each wide enough that its filter stays inside): the
// tile whose view holds the point nearest its centre.
float ShadowFacesVisibility( texture2D atlas, sampler pointSampler, int first, int count,
    vec3 world, vec3 normal, float size, float rotation )
{
	int best = -1;
	float bestExtent = 2.0;
	for ( int i = 0; i < count; ++i )
	{
		const ShadowTile tile = shadowTiles[first + i];
		const vec4 h = tile.viewProjection * vec4( world, 1.0 );
		if ( !( h.w > 0.0 ) )
			continue;
		const vec2 ndc = h.xy / h.w;
		const float extent = max( abs( ndc.x ), abs( ndc.y ) );
		if ( extent < bestExtent )
		{
			bestExtent = extent;
			best = first + i;
		}
	}
	if ( best < 0 || bestExtent > 1.0 )
		return 1.0;
	return ShadowVisibilitySoft( atlas, pointSampler, shadowTiles[best], world, normal, size, rotation );
}

#endif // SHADOW_FACES_GLSL
