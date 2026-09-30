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
// The device port has no comparison samplers, so the compare is done here:
// a 2x2 bilinear percentage-closer filter over the four texels around the
// point, each read with a point sampler at its center and clamped into the
// tile's viewport (the guard band keeps neighbours out). A texel lights the
// point when the point's depth, less the bias, is at most the stored depth
// (clip depth 0 near, 1 far). A point behind the view, beyond its far plane
// or outside the tile has no shadow information and is lit.
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

float ShadowTap( texture2D atlas, sampler pointSampler, vec2 texel, float atlasSize, float depth )
{
	float stored = textureLod( sampler2D( atlas, pointSampler ), ( texel + 0.5 ) / atlasSize, 0.0 ).r;
#ifdef SEEDED_DEPTH_REVERSED
	return depth >= stored ? 1.0 : 0.0;
#else
	return depth <= stored ? 1.0 : 0.0;
#endif
}

float ShadowVisibility( texture2D atlas, sampler pointSampler, ShadowTile tile, vec3 world )
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
	float depth = ndc.z - tile.params.x;
	float atlasSize = tile.params.y;
	vec2 position = uv * atlasSize - 0.5;
	vec2 base = floor( position );
	vec2 f = position - base;
	vec2 lo = tile.bounds.xy * atlasSize;
	vec2 hi = tile.bounds.zw * atlasSize - 1.0;
	float s00 = ShadowTap( atlas, pointSampler, clamp( base, lo, hi ), atlasSize, depth );
	float s10 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 1.0, 0.0 ), lo, hi ), atlasSize, depth );
	float s01 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 0.0, 1.0 ), lo, hi ), atlasSize, depth );
	float s11 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 1.0, 1.0 ), lo, hi ), atlasSize, depth );
	return mix( mix( s00, s10, f.x ), mix( s01, s11, f.x ), f.y );
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
// `rotation` turns both discs (radians; 0 for a fixed pattern).

const vec2 kShadowDisc[16] = vec2[16]( vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ), vec2( -0.09418410, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ), vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ), vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ), vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ), vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ), vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ), vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 ) );

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

// A 2x2 bilinear compare at a tile position (texels, before the -0.5).
float ShadowBilinear( texture2D atlas, sampler pointSampler, ShadowTile tile, vec2 uv, float depth )
{
	const float atlasSize = tile.params.y;
	const vec2 position = uv * atlasSize - 0.5;
	const vec2 base = floor( position );
	const vec2 f = position - base;
	const vec2 lo = tile.bounds.xy * atlasSize;
	const vec2 hi = tile.bounds.zw * atlasSize - 1.0;
	const float s00 = ShadowTap( atlas, pointSampler, clamp( base, lo, hi ), atlasSize, depth );
	const float s10 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 1.0, 0.0 ), lo, hi ), atlasSize, depth );
	const float s01 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 0.0, 1.0 ), lo, hi ), atlasSize, depth );
	const float s11 =
	    ShadowTap( atlas, pointSampler, clamp( base + vec2( 1.0, 1.0 ), lo, hi ), atlasSize, depth );
	return mix( mix( s00, s10, f.x ), mix( s01, s11, f.x ), f.y );
}

float ShadowVisibilitySoft( texture2D atlas, sampler pointSampler, ShadowTile tile, vec3 world,
    float size, float rotation )
{
	if ( tile.params.z == 0.0 )
		return ShadowVisibility( atlas, pointSampler, tile, world );
	const vec4 h = tile.viewProjection * vec4( world, 1.0 );
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
		searchClip = ShadowClipPerWorld( tile, n ) * size * max( receiver - n, 0.0 ) / receiver;
	}
	else
	{
		searchClip = ShadowClipPerWorld( tile, receiver ) * size * receiver;
	}
	searchClip = clamp( searchClip, 2.0 * texelClip, 0.25 );
	float blockers = 0.0;
	float blockerDepth = 0.0;
	for ( int i = 0; i < 16; ++i )
	{
		const vec2 offset = turn * kShadowDisc[i] * searchClip;
		const vec2 tap = clamp(
		    uv + offset * vec2( tile.transform.x, tile.transform.z ), tile.bounds.xy, tile.bounds.zw );
		const float stored =
		    textureLod( sampler2D( atlas, pointSampler ), tap, 0.0 ).r;
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
		penumbraClip = ShadowClipPerWorld( tile, receiver ) * size *
		               max( receiver - blockerDepth, 0.0 ) / max( blockerDepth, 1e-3 );
	else
		penumbraClip = ShadowClipPerWorld( tile, receiver ) * size *
		               max( receiver - blockerDepth, 0.0 );
	penumbraClip = clamp( penumbraClip, 1.5 * texelClip, 0.25 );
	float lit = 0.0;
	for ( int i = 0; i < 16; ++i )
	{
		const vec2 offset = turn * kShadowDisc[i] * penumbraClip;
		const vec2 tap = uv + offset * vec2( tile.transform.x, tile.transform.z );
		lit += ShadowBilinear( atlas, pointSampler, tile, tap, depth );
	}
	return lit / 16.0;
}
