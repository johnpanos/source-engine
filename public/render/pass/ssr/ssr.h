//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.ssr (RFC 0016 render.lighting.v1, "Screen-space
//			reflections"; contract render.ssr.v1): glossy reflections traced
//			in the current frame's lit colour over the image-based specular.
//			This header is the term's one definition; the RFC's model table
//			links here.
//
//			Inputs, per view (all of the view's extent):
//			- depth: the opaque pass's depth (D32, 0 at the near plane, 1 at
//			  the far plane, math::Perspective's convention);
//			- normalRoughness: the shading normal, octahedrally encoded
//			  (xy in [-1, 1]; OctEncode below), and the perceptual roughness
//			  the image-based specular used (z);
//			- specularWeight: what the surface multiplied its image light by
//			  (rgb: the split-sum directional albedo times specular
//			  occlusion);
//			- imageSpecular: the image-based specular the surface added
//			  (rgb);
//			- lit: the lit frame, image specular included (linear, RGBA16F).
//			The trace reads `lit` and a depth pyramid built from `depth`; it
//			keeps no history (RFC 0012 keeps temporal methods out).
//
//			Per pixel with depth < 1 and roughness r < roughnessCutoff:
//			1. P is the pixel centre's world position (fromClip), V the unit
//			   vector to the eye, N the decoded normal and R = 2 (N.V) N - V.
//			   R.N <= 0 has no hit.
//			2. The ray O + t R, from O = P + N w (w the world distance from
//			   the pixel centre to its right neighbour's at the same depth, so
//			   the ray starts clear of its own surface's texels), is
//			   projected to screen space (pixel x, y, and depth), clipped to
//			   the near plane and the screen; a projected line is straight in
//			   (x, y, depth), so it is walked texel by texel of the depth
//			   buffer, from the texel after the one holding O, for at most
//			   maxSteps texels.
//			3. Hit: the first texel whose depth d the ray reaches (the ray's
//			   depth over its span inside the texel, from where it enters to
//			   where it leaves, rises to d), provided the ray is then behind
//			   the surface by less than `thickness` (view-space distance,
//			   Source units). A texel the ray passes behind by more is not a
//			   hit, and the walk continues. The hit position is where the
//			   ray's depth meets d inside the texel (its entry when already
//			   behind).
//			4. Confidence c = edge * thicknessFade * roughnessFade, or 0 with
//			   no hit:
//			   - edge = smoothstep( 0, 1, min( u, 1 - u, v, 1 - v ) /
//			     edgeFade ) at the hit's screen position (u, v in [0, 1]);
//			   - thicknessFade = 1 - smoothstep( 0.5 thickness, thickness,
//			     behind ), `behind` the view-space distance the ray is behind
//			     the texel's surface at the hit (0 at a crossing);
//			   - roughnessFade = 1 - smoothstep( fadeStart, cutoff, r ),
//			     fadeStart = roughnessFadeStart * cutoff.
//			5. The reflected light: `lit`'s pyramid (mip k a 2 x 2 box of mip
//			   k - 1, sizes halved and at least 1) sampled trilinearly at the
//			   hit at mip clamp( log2( max( 1, 2 r^2 L ) ), 0, maxMip ), L the
//			   hit's distance from the pixel in pixels: the reflection lobe's
//			   footprint, the one spatial filter.
//			6. ssr = reflected * specularWeight, and the pixel becomes
//			   lit + c ( ssr - imageSpecular ), that is the rest of the light
//			   plus lerp( imageSpecular, ssr, c ).
//			Everything else is written unchanged, bitwise: background,
//			r >= roughnessCutoff, and c = 0.
//
//			The GPU trace walks a min-depth pyramid (hierarchical depth,
//			Uludag, GPU Pro 5, 2014) to skip cells the ray stays in front of;
//			its hit is the walk's (step 3). The suite's reference walks every
//			texel (render/lab/ssr_reference.h).
//
//=============================================================================//

#ifndef RENDER_PASS_SSR_SSR_H
#define RENDER_PASS_SSR_SSR_H

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>

namespace render::pass::ssr
{

struct SsrParams
{
	float roughnessCutoff = 0.4f;     // RFC 0016's provisional cutoff
	float roughnessFadeStart = 0.75f; // of the cutoff: the fade's start
	float thickness = 8.0f;           // Source units behind a surface that still hit
	float edgeFade = 0.1f;            // of the screen, from each edge
	std::uint32_t maxMip = 6;         // of the lit pyramid
	std::uint32_t maxSteps = 256;     // walk steps before a ray gives up (no hit)
};

// Why parameters are refused (nullopt: valid): the cutoff in (0, 1], the
// fade start in [0, 1), thickness and edge fade positive and finite (the
// edge fade at most 0.5), at least one step.
std::optional<std::string> ValidateParams( const SsrParams &params );

// The octahedral normal encoding the normalRoughness input carries (Cigolle
// et al., "A Survey of Efficient Representations for Independent Unit
// Vectors", JCGT 2014): a unit vector to [-1, 1]^2 and back.
struct Octahedral
{
	float x = 0.0f;
	float y = 0.0f;
};

inline Octahedral OctEncode( float nx, float ny, float nz )
{
	const float sum = std::fabs( nx ) + std::fabs( ny ) + std::fabs( nz );
	float x = nx / sum;
	float y = ny / sum;
	if ( nz < 0.0f )
	{
		const float fx = ( 1.0f - std::fabs( y ) ) * ( x >= 0.0f ? 1.0f : -1.0f );
		const float fy = ( 1.0f - std::fabs( x ) ) * ( y >= 0.0f ? 1.0f : -1.0f );
		x = fx;
		y = fy;
	}
	return { x, y };
}

inline void OctDecode( Octahedral e, float out[3] )
{
	float x = e.x;
	float y = e.y;
	const float z = 1.0f - std::fabs( x ) - std::fabs( y );
	if ( z < 0.0f )
	{
		const float fx = ( 1.0f - std::fabs( y ) ) * ( x >= 0.0f ? 1.0f : -1.0f );
		const float fy = ( 1.0f - std::fabs( x ) ) * ( y >= 0.0f ? 1.0f : -1.0f );
		x = fx;
		y = fy;
	}
	const float length = std::sqrt( x * x + y * y + z * z );
	out[0] = x / length;
	out[1] = y / length;
	out[2] = z / length;
}

} // namespace render::pass::ssr

#endif // RENDER_PASS_SSR_SSR_H
