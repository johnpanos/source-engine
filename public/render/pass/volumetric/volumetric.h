//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.volumetric (RFC 0016 "Lighting model", the
//			participating-media term; K11 "Volumetric fog"): single scattering
//			in a frustum-aligned froxel volume, applied to the opaque frame.
//
//			The froxel layout is render.pass.lights' ClusterGrid (the one
//			owner of the depth split): its tiles and depth slices, sliceScale
//			and sliceBias, the slice depths and the corner rays. Passes are
//			independent siblings (CAP011 rule 2), so this pass takes the grid
//			as a FroxelLayout the caller copies from it field by field; it
//			never computes a split. The caller creates the grid with the
//			limits the volume needs (lights::CreateClusterGrid).
//
//			Two stages, recorded by VolumetricRenderer::Record:
//
//			1. Inject (volumetric_inject.comp), one invocation per froxel:
//			   the medium at stratified points of the froxel (samplesXY^2 x
//			   samplesDepth, uniform in pixels and view distance) and the
//			   light scattered toward the eye there, averaged: rgb the
//			   in-scattered radiance per unit length, a the extinction.
//			   In-scattering at a point x with direction to the eye w:
//			       sum over lights of  pi E(x) T(light, x) sum_i sigma_s,i p_i(cos)
//			   with E the light's diffuse light at normal incidence in the
//			   lightmap unit (irradiance / pi, so pi E is irradiance), T the
//			   medium's transmittance from the light (exact for boxes, the
//			   height fog and the global density), p_i the
//			   Henyey-Greenstein phase of medium i and cos the cosine between
//			   the light's travel and w. Plus the media's emission.
//			2. Composite (volumetric_composite.frag), full screen over the
//			   frame: front to back along the pixel's own ray through the
//			   slices up to the scene depth, energy conserving (Hillaire 2015,
//			   after Wronski 2014), each slice's froxel values filtered
//			   bilinearly across columns: per slice of ray length d (the last
//			   one ends at the surface),
//			       L += T S (1 - exp(-sigma_t d)) / sigma_t,  T *= exp(-sigma_t d)
//			   written as ( L, T ) and blended by BlendMode::kTransmittance
//			   (render.device.v2 D21): dst * T + L, rgb only, alpha kept, T
//			   at the target's own precision. Within half a froxel of the
//			   screen's edge the froxel values are extrapolated from the two
//			   outermost columns, not held. Marching per
//			   pixel keeps the transmittance exact for a medium uniform across
//			   a column, the screen's edges included. An integrated volume
//			   (Hillaire's per-column prefix) is the translucent application's
//			   and an optimization (binding rule 7), not in this slice.
//
//			Neutral value: a medium of density zero (and no emission) gives
//			T = 1 and L = 0 exactly, so the frame is bitwise unchanged,
//			whatever the surfaces drew (the legacy range and height fog
//			included). No state carries between Record calls: the froxel
//			volumes are created per call and released behind its token, so
//			no history exists to survive a camera cut.
//
//			Units: positions and distances in Source units; light colors in
//			the lightmap unit (render.light-set.v1's inverse-square color is
//			the diffuse light at 100 units); extinction and scattering per
//			Source unit; the output is linear scene radiance in the unit the
//			surfaces draw.
//
//			Not in this slice (gaps, RFC/0016-progress.md): shadows (the
//			atlas's record and helper live in render.pass.shadows, a sibling
//			this pass may not read until the shared view-level types move
//			down a layer), the translucent application, the cluster lists
//			(every froxel loops over every light), the sun, rect lights, a
//			temporal
//			history (none: RFC 0012 keeps TAA out, and the default is none),
//			and a render.graph form of Record.
//
//=============================================================================//

#ifndef RENDER_PASS_VOLUMETRIC_VOLUMETRIC_H
#define RENDER_PASS_VOLUMETRIC_VOLUMETRIC_H

#include "foundation/expected.h"
#include "render/device/device.h"
#include "render/light_set.h"
#include "render/math/matrix.h"
#include "render/math/vector.h"
#include "render/projected_light.h"

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <vector>

namespace render::pass::volumetric
{

inline constexpr std::uint32_t kMaxFogVolumes = 64;
inline constexpr std::uint32_t kMaxMediumLights = 1024;
inline constexpr std::uint32_t kMaxMediumProjectors =
    std::uint32_t( projected_light::kMaxProjectedLights );

// An env_volumetric_fog_volume: a box of homogeneous medium.
struct FogVolume
{
	math::float3 mins; // world
	math::float3 maxs;
	float extinction = 0.0f; // sigma_t per unit (scattering plus absorption)
	float albedo = 0.0f;     // sigma_s / sigma_t
	float anisotropy = 0.0f; // Henyey-Greenstein g, -1 < g < 1
	math::float3 emission;   // radiance emitted per unit length
};

// The env_volumetric_fog_controller: a global density everywhere and a height
// fog, heightDensity * exp( -heightFalloff * ( z - baseHeight ) ).
struct HeightFog
{
	float density = 0.0f;       // sigma_t per unit everywhere
	float heightDensity = 0.0f; // sigma_t per unit at baseHeight
	float heightFalloff = 0.0f; // per unit; 0: the height fog is uniform
	float baseHeight = 0.0f;
	float albedo = 1.0f;
	float anisotropy = 0.0f;
};

struct Medium
{
	std::vector<FogVolume> volumes;
	HeightFog fog;
	// Every density and emission times this (the fixture's density-zero
	// state is 0).
	float densityScale = 1.0f;
};

enum class MediumLightKind : std::uint8_t
{
	kPoint,
	kSpot
};

// A point or spot light as the medium sees it, by render.light-set.v1's rules
// (light_set::InverseSquareFalloff or light_set::Falloff, times
// light_set::SpotFactor for a spot), as the surface program's clustered
// lights take them.
struct MediumLight
{
	MediumLightKind kind = MediumLightKind::kPoint;
	light_set::LightFalloff falloff = light_set::LightFalloff::InverseSquare;
	math::float3 position;
	math::float3 direction{ 0.0f, 0.0f, -1.0f }; // spot axis, unit
	math::float3 color;                          // lightmap unit
	float radius = 0.0f;                         // 0: unbounded
	float sourceRadius = light_set::kInverseSquareSourceRadius;
	float minLight = 0.0f; // the legacy falloff's threshold
	float innerCos = 1.0f;
	float outerCos = 1.0f;
	float spotExponent = 0.0f; // the cone ramp's (light set v3)
};

// A light set light as the medium sees it (points and spots).
MediumLight MediumLightFrom( const light_set::RuntimeLight &light );

struct MediumProjector
{
	projected_light::Light light;
	std::uint32_t cookieLayer = 0; // layer of VolumetricTargets::cookies
};

// Stratified samples per froxel: samplesXY^2 across it, samplesDepth along it.
struct VolumetricSampling
{
	std::uint32_t samplesXY = 2;
	std::uint32_t samplesDepth = 4;
};

// A render.pass.lights ClusterGrid as this pass reads it (the caller copies
// the fields; nothing here is derived).
struct FroxelLayout
{
	std::uint32_t tilesX = 0;
	std::uint32_t tilesY = 0;
	std::uint32_t slices = 0;
	std::uint32_t tileSizePixels = 0;
	std::uint32_t widthPixels = 0;
	std::uint32_t heightPixels = 0;
	float sliceScale = 0.0f; // slice = floor( log( depth ) * sliceScale + sliceBias )
	float sliceBias = 0.0f;
	float nearZ = 0.0f;
	float farZ = 0.0f;
	std::span<const float> sliceDepths; // slices + 1 view distances
	math::float4x4 view;                // world to view
	// The view-space points at distance 1 (z = -1) under the screen's
	// top-left and bottom-right corners (ClusterGrid::cornerRays' first and
	// last).
	math::float3 rayTopLeft;
	math::float3 rayBottomRight;
};

struct VolumetricView
{
	const FroxelLayout *froxels = nullptr;
	math::float4x4 projection; // the grid's projection (depth to distance)
	math::float3 eye;          // world
};

struct VolumetricFrame
{
	const Medium *medium = nullptr;
	std::span<const MediumLight> lights;
	std::span<const MediumProjector> projectors;
	VolumetricSampling sampling;
};

// The frame the pass reads and blends into. `color` is in kColorAttachment
// and stays there; `depth` is in `depthUsage` and returns to it. `cookies`
// is in kSampled; without it every cookie is white.
struct VolumetricTargets
{
	device::TextureId color;
	device::TextureId depth;
	device::ResourceUsage depthUsage = device::ResourceUsage::kDepthWrite;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	device::TextureId cookies; // RGBA 2D array, at least two layers
};

enum class VolumetricStatus : std::uint8_t
{
	kNoCompute = 1, // the device lacks Capability::kCompute
	kDevice,        // a layout, pipeline, resource or bind group was refused
	kInvalidGrid,   // no layout, or one that does not match the targets
	kInvalidFrame,  // too many lights, volumes or projectors; a cookie layer out of range
	kInvalidMedium, // non-finite or negative values, or |g| >= 1
};

struct VolumetricStats
{
	std::uint32_t froxels = 0;
	std::uint32_t lights = 0;
	std::uint32_t projectors = 0;
	std::uint32_t volumes = 0;
};

// Seeded stage programs (SPIR-V) the suites pass in place of the core ones.
struct VolumetricPrograms
{
	std::span<const std::uint32_t> inject;
	std::span<const std::uint32_t> composite;
};

class VolumetricRenderer
{
public:
	static foundation::Expected<std::unique_ptr<VolumetricRenderer>, VolumetricStatus> Create(
	    device::IRenderDevice2 &device, device::Format colorFormat,
	    const VolumetricPrograms &programs = {} );
	~VolumetricRenderer();
	VolumetricRenderer( const VolumetricRenderer & ) = delete;
	VolumetricRenderer &operator=( const VolumetricRenderer & ) = delete;

	// Records the three stages on `encoder`. The renderer's resources for
	// this call stay live until Collect with the submission's token.
	foundation::Expected<VolumetricStats, VolumetricStatus> Record( device::CommandEncoder &encoder,
	    const VolumetricView &view, const VolumetricFrame &frame,
	    const VolumetricTargets &targets );
	void Collect( device::CompletionToken token );

private:
	explicit VolumetricRenderer( device::IRenderDevice2 &device ) : m_Device( device ) {}

	device::IRenderDevice2 &m_Device;
	device::BindGroupLayoutId m_InjectLayout;
	device::BindGroupLayoutId m_CompositeLayout;
	device::PipelineId m_Inject;
	device::PipelineId m_Composite;
	device::SamplerId m_PointSampler;
	device::SamplerId m_LinearSampler;
	device::TextureId m_WhiteCookies;
	bool m_WhiteStaged = false;
	std::mutex m_PendingLock;
	std::vector<device::ResourceId> m_Pending;
	device::CompletionToken m_LastToken;
};

// The std140 parameters every stage reads (binding 0), 208 bytes.
struct VolumetricParamsGpu
{
	std::uint32_t dims[4] = {};   // froxels x, y, slices, samplesXY
	std::uint32_t counts[4] = {}; // lights, volumes, projectors, samplesDepth
	float screen[4] = {};         // width, height, tile size in pixels, density scale
	float slicing[4] = {};        // sliceScale, sliceBias, nearZ, farZ
	float rays[4] =
	    {}; // the view ray at z = -1 at the top-left corner (xy), its change across the screen (zw)
	float depth[4] = {}; // projection rows[2].z, rows[2].w: view distance = w / ( depth + z )
	float viewToWorld[4][4] = {}; // row-major
	float eye[4] = {};
	float fog0[4] = {}; // global sigma_t, height sigma_t, height falloff, base height
	float fog1[4] = {}; // global and height albedo, anisotropy, 0, 0
};
static_assert( sizeof( VolumetricParamsGpu ) == 208 );

// The pass's records (std430), as the stages read them.
struct FogVolumeGpu
{
	float minsExtinction[4] = {};
	float maxsAlbedo[4] = {};
	float emissionAnisotropy[4] = {};
};
struct MediumLightGpu
{
	float positionKind[4] = {}; // xyz; w 0 point, 1 spot
	float colorFalloff[4] = {}; // rgb; w 0 inverse square, 1 legacy
	float direction[4] = {};    // spot axis
	float cone[4] = {}; // innerCos, outerCos, radius, sourceRadius
	float misc[4] = {}; // minLight, spot exponent, 0, 0
};
struct MediumProjectorGpu
{
	float origin[4] = {}; // xyz; w cookie layer
	float forward[4] = {};
	float right[4] = {};
	float up[4] = {};
	float frustum[4] = {}; // tan half horizontal, tan half vertical, near, far
	float color[4] = {};   // rgb, 0
	float atten[4] = {};   // constant, linear, quadratic, 0
};
static_assert( sizeof( FogVolumeGpu ) == 48 && sizeof( MediumLightGpu ) == 80 &&
               sizeof( MediumProjectorGpu ) == 112 );

VolumetricParamsGpu PackVolumetricParams( const VolumetricView &view, const VolumetricFrame &frame,
    std::uint32_t lightCount, std::uint32_t volumeCount, std::uint32_t projectorCount );
FogVolumeGpu PackFogVolume( const FogVolume &volume );
MediumLightGpu PackMediumLight( const MediumLight &light );
MediumProjectorGpu PackMediumProjector( const MediumProjector &projector );

// Checks a medium's values (finite, non-negative, |g| < 1).
bool ValidMedium( const Medium &medium );

} // namespace render::pass::volumetric

#endif // RENDER_PASS_VOLUMETRIC_VOLUMETRIC_H
