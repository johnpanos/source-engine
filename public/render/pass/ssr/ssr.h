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
//			- iblRadiance: the image-based specular radiance the surface
//			  read (rgb, before its weight);
//			- specularWeight: the weight w it multiplied that radiance by
//			  (rgb: the split-sum directional albedo times specular
//			  occlusion);
//			- lit: the lit frame (linear, RGBA16F), which holds
//			  w * iblRadiance by construction.
//			The surface program writes the first three as its kSsrTargets
//			outputs (an opt-in pipeline guard; a family without it keeps one
//			target, bitwise): (1) the octahedral normal and roughness, (2)
//			iblRadiance and (3) w, the very factors of its own image-specular
//			expression, not a recomputation.
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
//			   no hit. The fades are continuous in the ray's own parameters
//			   (render-core owner's decision, 2026-09-29, after the first
//			   fallback-seam measurement):
//			   - the hit's footprint J: how far the hit moves on the screen
//			     per pixel of the reflecting surface, a ray differential.
//			     For each screen axis, the camera ray through the pixel centre
//			     one pixel along it meets the pixel's own plane (P, N) at P',
//			     reflects about N from P' + N w, and meets the hit's plane
//			     (the hit point, the normal read at the hit texel) at X';
//			     J_axis is the screen distance from the hit to X', and
//			     J = max( 1, J_x, J_y ); J = 1 where a plane is parallel to
//			     its ray or X' is behind the eye;
//			   - edge = smoothstep( 0, 1, min( x / ( edgeFade W J ),
//			     ( W - x ) / ( edgeFade W J ), y / ( edgeFade H J ),
//			     ( H - y ) / ( edgeFade H J ) ) ), ( x, y ) the hit's screen
//			     position in pixels and W x H the view: the fade spans
//			     edgeFade of the screen per pixel of the surface, however
//			     stretched its reflection;
//			   - thicknessFade = 1 - smoothstep( 0, thickness, behind ),
//			     `behind` the view-space distance the ray is behind the
//			     texel's surface at the hit (0 at a crossing);
//			   - roughnessFade = 1 - smoothstep( fadeStart, cutoff, r ),
//			     fadeStart = roughnessFadeStart * cutoff.
//			5. The reflected light: `lit`'s pyramid (mip k a 2 x 2 box of mip
//			   k - 1, sizes halved and at least 1) sampled trilinearly at the
//			   hit at mip clamp( log2( max( 1, 2 r^2 L ) ), 0, maxMip ), L the
//			   hit's distance from the pixel in pixels: the reflection lobe's
//			   footprint, the one spatial filter.
//			6. The pixel becomes lit + c w ( reflected - iblRadiance ): the
//			   image specular w iblRadiance replaced by w lerp( iblRadiance,
//			   reflected, c ).
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

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/math/matrix.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::pass::ssr
{

struct SsrParams
{
	float roughnessCutoff = 0.4f;     // RFC 0016's provisional cutoff
	float roughnessFadeStart = 0.75f; // of the cutoff: the fade's start
	float thickness = 8.0f;           // Source units behind a surface that still hit
	float edgeFade = 0.1f;            // of the screen, from each edge
	std::uint32_t maxMip = 6;         // of the lit pyramid
	// Texels walked (boundaries crossed from the origin's texel) before a
	// ray gives up with no hit; the default exceeds any screen's width plus
	// height up to 4096 pixels, so it binds only there.
	std::uint32_t maxSteps = 8192;
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

enum class SsrStatus : std::uint8_t
{
	kInvalidParams,  // ValidateParams refused them
	kInvalidTargets, // a missing texture, or a zero extent
	kDevice,         // the device refused a layout, pipeline, buffer or group
};

// The view the pass traces in.
struct SsrView
{
	math::float4x4 toClip; // world to clip (math::Perspective's depth range)
	float eye[3] = { 0.0f, 0.0f, 0.0f };
};

// The pass recorded straight into an encoder. Every input is in kSampled
// before and after it and has the output's extent; the output (RGBA16F,
// usable as kStorageWrite) is in `outputUsage` before and after, and is
// written whole.
struct SsrDirectTargets
{
	device::TextureId depth;           // D32
	device::TextureId normalRoughness; // RGBA16F or wider: oct xy, roughness
	device::TextureId iblRadiance;
	device::TextureId specularWeight;
	device::TextureId lit;
	device::TextureId output;
	device::ResourceUsage outputUsage = device::ResourceUsage::kStorageWrite;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

class ScreenSpaceReflections
{
public:
	static foundation::Expected<std::unique_ptr<ScreenSpaceReflections>, SsrStatus> Create(
	    device::IRenderDevice2 &device, const SsrParams &params );
	// Test endpoint: the pass with another trace program (a suite's
	// diagnostics or seeded variant of render/pass/ssr/ssr_trace.comp, SPIR-V,
	// with the same interface).
	static foundation::Expected<std::unique_ptr<ScreenSpaceReflections>, SsrStatus> CreateWithTrace(
	    device::IRenderDevice2 &device, const SsrParams &params,
	    std::span<const std::uint32_t> traceSpirv );
	~ScreenSpaceReflections();
	ScreenSpaceReflections( const ScreenSpaceReflections & ) = delete;
	ScreenSpaceReflections &operator=( const ScreenSpaceReflections & ) = delete;

	// Records the pyramids, the trace and the composite. The pass must
	// outlive the submission; Collect releases what it recorded.
	foundation::Expected<void, SsrStatus> Record(
	    device::CommandEncoder &encoder, const SsrDirectTargets &targets, const SsrView &view );
	void Collect( device::CompletionToken token );

	// The diagnostics buffer a trace built with SSR_DIAGNOSTICS writes (two
	// vec4 per pixel, row 0 first: hit x, y, confidence, mip; hit texel x, y,
	// 1 on a hit, behind), left in kStorageWrite; usable as kCopySource.
	device::BufferId Diagnostics() const { return m_Diagnostics; }
	std::uint32_t RecordFailures() const;

private:
	explicit ScreenSpaceReflections( device::IRenderDevice2 &device ) : m_Device( device ) {}
	foundation::Expected<void, SsrStatus> Resize( std::uint32_t width, std::uint32_t height );

	struct Level
	{
		std::uint32_t first = 0;
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};

	device::IRenderDevice2 &m_Device;
	SsrParams m_Params;
	device::BindGroupLayoutId m_Layout;
	device::SamplerId m_Sampler;
	device::PipelineId m_Pyramid;
	device::PipelineId m_Trace;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	std::vector<Level> m_DepthLevels;
	std::vector<Level> m_ColorLevels;
	device::BufferId m_DepthPyramid;
	device::BufferId m_ColorPyramid;
	device::BufferId m_Diagnostics;
	device::BufferId m_Constants;
	mutable std::mutex m_PendingLock;
	std::vector<device::ResourceId> m_Pending;
	std::uint32_t m_RecordFailures = 0;
	device::CompletionToken m_LastToken;
};

} // namespace render::pass::ssr

#endif // RENDER_PASS_SSR_SSR_H
