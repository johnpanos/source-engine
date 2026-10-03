// render.shadows.v1: the visibility of a point from a light shadowed by
// several perspective tiles (the faces of a cube or hemicube). The includer
// includes shadow_sample.glsl first and declares its tile list as
// `shadowTiles` (ShadowTile records) before including this file.
#ifndef SHADOW_FACES_GLSL
#define SHADOW_FACES_GLSL

float ShadowFacesVisibility( texture2D atlas, sampler pointSampler, sampler comparisonSampler,
    int first, int count, vec3 world, vec3 normal, float size, float rotation );

// RuntimeShadowLayout::kWorldCube: select the dominant world axis in the
// planner's fixed face order. Equal axes choose X, then Y, then Z, just as
// the generic search chooses its first face on a tie. Area lights retain
// their oriented hemicube search below.
float ShadowWorldCubeVisibility( texture2D atlas, sampler pointSampler, sampler comparisonSampler,
    int first, vec3 fromLight, vec3 world, vec3 normal, float size, float rotation )
{
	const vec3 magnitude = abs( fromLight );
	const int axis = magnitude.x >= magnitude.y && magnitude.x >= magnitude.z ? 0
	                 : magnitude.y >= magnitude.z                              ? 1
	                                                                           : 2;
	if ( magnitude[axis] == 0.0 )
		return 1.0;
	int face = axis * 2 + ( fromLight[axis] >= 0.0 ? 0 : 1 );
#ifdef SEEDED_SHADOW_CUBE_NEXT
	face = ( face + 1 ) % 6;
#endif
	return ShadowVisibilitySoft( atlas, pointSampler, comparisonSampler, shadowTiles[first + face],
	    world, normal, size, rotation );
}

// A light shadowed by `count` perspective tiles from its position (the faces
// of a cube or hemicube, each wide enough that its filter stays inside): the
// tile whose view holds the point nearest its centre.
float ShadowFacesVisibility( texture2D atlas, sampler pointSampler, sampler comparisonSampler, int first, int count,
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
	return ShadowVisibilitySoft( atlas, pointSampler, comparisonSampler, shadowTiles[best], world, normal, size, rotation );
}

#endif // SHADOW_FACES_GLSL
