// The shadow receiver helper (RFC 0016 K7, render.shadows.v1): the
// visibility of a world point from one shadow view's atlas tile.
//
// ShadowTile is ShadowTileGpu (public/render/shadow_tile.h, render.contracts):
// the view's world to clip matrix, the tile transform applied after the
// perspective divide (u = x * scaleU + biasU, v = y * scaleV + biasV), the
// tile viewport's rectangle in atlas texture coordinates, and the receiver
// depth bias and the atlas size in texels. The matrix is stored row-major, so
// a block holding ShadowTile records is declared row_major.
//
// Visibility uses the device's linear depth-comparison sampler: comparison
// precedes the bilinear filter. Blocker search keeps a separate point sampler
// for raw depth. The footprint is clamped inside its tile (including texel
// centres at the edge), so neither operation reads a neighbouring tile.
// Clip depth is 0 near, 1 far; reference <= stored depth is lit.
// A point behind the view, beyond its far plane or outside the tile is lit.
//
// SEEDED_DEPTH_REVERSED builds the defective compare render.shadows.pixels
// must reject; no product uses it.

struct ShadowTile
{
	mat4 viewProjection;
	vec4 transform; // scaleU, biasU, scaleV, biasV
	vec4 bounds;    // u0, v0, u1, v1
	vec4 params;    // depth bias, atlas size in texels, 0, 0
};

// A 2x2 bilinear compare, with the footprint clamped to this tile.
// The comparison sampler filters four independently compared depths.
float ShadowBilinear( texture2D atlas, sampler pointSampler, sampler comparisonSampler,
    ShadowTile tile, vec2 uv, float depth )
{
	const float atlasSize = tile.params.y;
	const vec2 lo = tile.bounds.xy * atlasSize;
	const vec2 hi = tile.bounds.zw * atlasSize - 1.0;
#if defined( REFERENCE_MANUAL_SHADOW_COMPARE ) || defined( SEEDED_SHADOW_UNSTABLE_GATHER )
	// Private immutable-input oracle; never selected by a product.
	const vec2 position = clamp( uv * atlasSize - 0.5, lo, hi );
	const vec2 coordinate = ( position + 0.5 ) / atlasSize;
	const vec2 f = fract( position );
#ifdef SEEDED_SHADOW_UNSTABLE_GATHER
	const vec4 stored = textureGather( sampler2D( atlas, pointSampler ), coordinate, 0 );
#else
	// Integer addressing makes this oracle independent of the texture unit's
	// subtexel footprint rounding. All four texels remain in the same tile.
	const ivec2 base = ivec2( floor( position ) );
	const ivec2 last = ivec2( hi );
	const vec4 stored = vec4(
	    texelFetch( sampler2D( atlas, pointSampler ), min( base + ivec2( 0, 1 ), last ), 0 ).r,
	    texelFetch( sampler2D( atlas, pointSampler ), min( base + ivec2( 1, 1 ), last ), 0 ).r,
	    texelFetch( sampler2D( atlas, pointSampler ), min( base + ivec2( 1, 0 ), last ), 0 ).r,
	    texelFetch( sampler2D( atlas, pointSampler ), base, 0 ).r );
#endif
	const vec4 lit = step( vec4( depth ), stored );
	return mix( mix( lit.w, lit.z, f.x ), mix( lit.x, lit.y, f.x ), f.y );
#else
	// Clamp directly to the tile's texel centres. The atlas planner uses
	// power-of-two extents, so the half-texel bounds are represented exactly.
	const vec2 coordinate = clamp( uv, ( lo + 0.5 ) / atlasSize, ( hi + 0.5 ) / atlasSize );
	const float lit = textureLod( sampler2DShadow( atlas, comparisonSampler ),
	    vec3( coordinate, depth ), 0.0 );
#ifdef SEEDED_DEPTH_REVERSED
	return 1.0 - lit;
#else
	return lit;
#endif
#endif
}

float ShadowVisibility( texture2D atlas, sampler pointSampler, sampler comparisonSampler, ShadowTile tile, vec3 world )
{
	vec4 h = tile.viewProjection * vec4( world, 1.0 );
	if ( !( h.w > 0.0 ) )
		return 1.0;
	vec3 ndc = h.xyz / h.w;
	vec2 uv = vec2( ndc.x * tile.transform.x + tile.transform.y,
	    ndc.y * tile.transform.z + tile.transform.w );
	if ( ndc.z > 1.0 || any( lessThan( uv, tile.bounds.xy ) ) ||
	     any( greaterThan( uv, tile.bounds.zw ) ) )
		return 1.0;
	return ShadowBilinear( atlas, pointSampler, comparisonSampler, tile, uv, ndc.z - tile.params.x );
}

// Soft shadows (RFC 0016 K11: the direct-visibility term for lights with a
// size): percentage-closer soft shadows (Fernando 2005) on one tile. The
// tile's params.z and params.w give its depth mapping: near and far planes
// of a perspective view (params.z > 0), or -1 and the depth range in world
// units of an orthographic one; 0, 0 is a tile without that record, which
// takes the hard 2x2 filter (ShadowVisibility).
//
// `size` is the emitter's size: for a perspective tile the world radius of
// the light (a disc at the view's origin), for an orthographic tile the
// tangent of the source's angular radius (the sun's disc). The blocker
// search averages the depth of the occluders between the point and the
// emitter; the penumbra is the emitter seen past that depth, filtered by a
// Poisson disc of bilinear compares, never narrower than 1.5 texels.
// `rotation` turns both discs (radians; 0 for a fixed pattern); `normal` is
// the receiver's offset (ShadowReceiverOffset).

const vec2 kShadowDisc[16] = vec2[16]( vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ), vec2( -0.09418410, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ), vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ), vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ), vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ), vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ), vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ), vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 ) );

// The receiver's offset for a light towards `toLight`: along the surface's
// geometric normal (a coarse mesh's facet, not its interpolated normal,
// which the shadow map does not see: the shadow terminator), longer as the
// light grazes the facet (the tangent of its angle, from 1 to 4).
vec3 ShadowReceiverOffset( vec3 geometricNormal, vec3 toLight )
{
	const float c = clamp( abs( dot( geometricNormal, toLight ) ), 0.05, 1.0 );
	const float slope = sqrt( 1.0 - c * c ) / c;
	const vec3 facing = dot( geometricNormal, toLight ) < 0.0 ? -geometricNormal : geometricNormal;
	return facing * clamp( 1.0 + slope, 1.0, 4.0 );
}

// The shadow terminator's fade: a light's shadow darkens a surface fully
// only where its interpolated normal faces the light by more than 0.2 in
// cosine. Below that a coarse mesh's own facets, which the shadow map sees
// and its interpolated normal smooths away, would cut a staircase across
// its terminator; the light there is at most a fifth of its facing value,
// so a shadow cast by something else loses little.
float ShadowTerminatorFade( vec3 smoothNormal, vec3 toLight, float visibility )
{
	return mix( 1.0, visibility, smoothstep( 0.0, 0.2, dot( smoothNormal, toLight ) ) );
}

// The distance from the view's origin plane of a clip depth on this tile.
float ShadowLinearDepth( ShadowTile tile, float depth )
{
	if ( tile.params.z > 0.0 )
	{
		const float n = tile.params.z;
		const float f = tile.params.w;
		return f * n / ( f - depth * ( f - n ) );
	}
	return depth * tile.params.w;
}

// Clip units per world unit across the view at a linear depth: the
// projection's x scale over depth (perspective), or its scale (orthographic).
float ShadowClipPerWorld( ShadowTile tile, float linearDepth )
{
	const float scale = length(
	    vec3( tile.viewProjection[0][0], tile.viewProjection[1][0], tile.viewProjection[2][0] ) );
	return tile.params.z > 0.0 ? scale / max( linearDepth, 1e-4 ) : scale;
}

// The includer sets this when soft shadows are off (a view setting): every
// shadow then takes the hard 2x2 filter.
bool gShadowHardOnly = false;

float ShadowVisibilitySoft( texture2D atlas, sampler pointSampler, sampler comparisonSampler, ShadowTile tile, vec3 world,
    vec3 normal, float size, float rotation )
{
	if ( tile.params.z == 0.0 || gShadowHardOnly )
		return ShadowVisibility( atlas, pointSampler, comparisonSampler, tile, world );
	// The receiver moved off its surface along `normal` (the caller's offset
	// direction, its length a multiplier: ShadowReceiverOffset) by one and a
	// half of the tile's texels at its depth per unit (normal offset), so the
	// compare needs only a small depth bias at any distance.
	// The offset point's clip position by linearity, VP (w + k n) = VP w + k VP n:
	// the matrix is read twice up front and is dead after, rather than live
	// across the offset's arithmetic (it set the surface program's register
	// peak). Equal to transforming the moved point up to rounding.
	const vec4 h0 = tile.viewProjection * vec4( world, 1.0 );
	const vec4 hNormal = tile.viewProjection * vec4( normal, 0.0 );
	const float clipPerWorld = length(
	    vec3( tile.viewProjection[0][0], tile.viewProjection[1][0], tile.viewProjection[2][0] ) );
	vec4 h = h0;
	if ( h0.w > 0.0 )
	{
		const float d0 = ShadowLinearDepth( tile, h0.z / h0.w );
		const float texelWorld = 2.0 / ( tile.params.y * abs( tile.transform.x ) ) /
		                         ( tile.params.z > 0.0 ? clipPerWorld / max( d0, 1e-4 ) : clipPerWorld );
		h = h0 + hNormal * ( 1.5 * texelWorld );
	}
	if ( !( h.w > 0.0 ) )
		return 1.0;
	const vec3 ndc = h.xyz / h.w;
	const vec2 uv = vec2( ndc.x * tile.transform.x + tile.transform.y,
	    ndc.y * tile.transform.z + tile.transform.w );
	if ( ndc.z > 1.0 || any( lessThan( uv, tile.bounds.xy ) ) ||
	     any( greaterThan( uv, tile.bounds.zw ) ) )
		return 1.0;
	const float depth = ndc.z - tile.params.x;
	const float atlasSize = tile.params.y;
	const bool perspective = tile.params.z > 0.0;
	const float receiver = ShadowLinearDepth( tile, ndc.z );
	const vec2 clipToUv = abs( vec2( tile.transform.x, tile.transform.z ) );
	const float texelClip = 1.0 / ( atlasSize * clipToUv.x );
	const float s = sin( rotation ), c = cos( rotation );
	const mat2 turn = mat2( c, s, -s, c );

	// The blocker search: where the rays from the point to the emitter cross
	// the tile. For a perspective tile they are widest at the near plane; an
	// orthographic tile's rays are parallel, spread by the source's angle
	// over the depth range towards the light.
	float searchClip;
	if ( perspective )
	{
		const float n = tile.params.z;
		searchClip = ( clipPerWorld / max( n, 1e-4 ) ) * size * max( receiver - n, 0.0 ) / receiver;
	}
	else
	{
		searchClip = ( perspective ? clipPerWorld / max( receiver, 1e-4 ) : clipPerWorld ) * size * receiver;
	}
	searchClip = clamp( searchClip, 2.0 * texelClip, 0.25 );
	float blockers = 0.0;
	float blockerDepth = 0.0;
	for ( int i = 0; i < 16; ++i )
	{
		const vec2 offset = turn * kShadowDisc[i] * searchClip;
		const vec2 tap = clamp(
		    uv + offset * vec2( tile.transform.x, tile.transform.z ), tile.bounds.xy, tile.bounds.zw );
		const ivec2 texel = clamp( ivec2( tap * atlasSize ), ivec2( 0 ), ivec2( atlasSize - 1.0 ) );
		const float stored = texelFetch( sampler2D( atlas, pointSampler ), texel, 0 ).r;
		if ( stored < depth )
		{
			blockers += 1.0;
			blockerDepth += ShadowLinearDepth( tile, stored );
		}
	}
	if ( blockers < 0.5 )
		return 1.0;
	blockerDepth /= blockers;
	// The penumbra: the emitter's size seen past the blockers, at the
	// receiver's scale.
	float penumbraClip;
	if ( perspective )
		penumbraClip = ( perspective ? clipPerWorld / max( receiver, 1e-4 ) : clipPerWorld ) * size *
		               max( receiver - blockerDepth, 0.0 ) / max( blockerDepth, 1e-3 );
	else
		penumbraClip = ( perspective ? clipPerWorld / max( receiver, 1e-4 ) : clipPerWorld ) * size *
		               max( receiver - blockerDepth, 0.0 );
	penumbraClip = clamp( penumbraClip, 1.5 * texelClip, 0.25 );
	float lit = 0.0;
	for ( int i = 0; i < 16; ++i )
	{
		const vec2 offset = turn * kShadowDisc[i] * penumbraClip;
		const vec2 tap = uv + offset * vec2( tile.transform.x, tile.transform.z );
#ifdef SEEDED_SHADOW_FILTER_SKIP_LAST
		if ( i == 15 )
			continue;
#endif
		lit += ShadowBilinear( atlas, pointSampler, comparisonSampler, tile, tap, depth );
	}
	return lit / 16.0;
}
