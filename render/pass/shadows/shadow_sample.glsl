// render.pass.shadows receiver helper (RFC 0016 K7, render.shadows.v1): the
// visibility of a world point from one shadow view's atlas tile.
//
// ShadowTile is ShadowTileGpu (public/render/pass/shadows/shadow_passes.h):
// the view's world to clip matrix, the tile transform applied after the
// perspective divide (u = x * scaleU + biasU, v = y * scaleV + biasV), the
// tile viewport's rectangle in atlas texture coordinates, and the receiver
// depth bias and the atlas size in texels.
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
