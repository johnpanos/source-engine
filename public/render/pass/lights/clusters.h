//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.lights clustered light lists (RFC 0016 K7,
//			render.lights.v1). One froxel grid per view: screen tiles of a
//			fixed pixel size times depth slices spaced logarithmically between
//			the view's near and far distances. Assignment gives every froxel
//			the point and spot lights of the frame's light set
//			(render.light-set.v1, RFC 0011, the only runtime light list) that
//			may reach it, as a compact list: one offset and count per froxel
//			into one index list.
//
//			AssignLights is the serial CPU path and the oracle the compute
//			pass (cluster_assign.comp beside the sources) must match. Culling
//			is conservative: a light that reaches a froxel is never missing
//			from it; a light near a froxel's edge may be listed although it
//			does not reach it.
//
//			Capacity is explicit. Each profile has limits on lights per view,
//			lights per froxel and the index list. What does not fit is kept in
//			light order and the rest is counted in the result
//			(OverflowPolicy::kReport), or the build fails and leaves the
//			output unchanged (OverflowPolicy::kFail). Nothing is dropped
//			without being counted.
//
//			Conventions (render/device/conventions.h): clip depth 0 to 1, clip
//			Y up, pixel row 0 at the top. View space looks down -Z; a froxel's
//			depth is the view distance -z.
//
//=============================================================================//

#ifndef RENDER_PASS_LIGHTS_CLUSTERS_H
#define RENDER_PASS_LIGHTS_CLUSTERS_H

#include "foundation/expected.h"
#include "render/light_set.h"
#include "render/math/frustum.h"
#include "render/math/matrix.h"
#include "render/math/vector.h"

#include <cstddef>
#include <cstdint>
#include <array>
#include <span>
#include <vector>

namespace render::pass::lights
{

enum class OverflowPolicy : std::uint8_t
{
	kReport, // keep what fits, in light order, and count what was dropped
	kFail,   // fail the build and leave the output unchanged
};

// Per-profile grid shape and capacities.
struct ClusterLimits
{
	std::uint32_t tileSizePixels = 64;
	std::uint32_t depthSlices = 24;
	std::uint32_t maxFroxels = 1u << 16;
	// Point and spot lights one view takes, in light-set order.
	std::uint32_t maxLights = 1024;
	std::uint32_t maxLightsPerFroxel = 128;
	// The index list's capacity (the GPU buffer's size in indices).
	std::uint32_t maxLightIndices = 1u << 20;
	OverflowPolicy overflow = OverflowPolicy::kReport;
};

// Provisional per-profile limits; the K7 budget rows (render-v1.json) set
// the final numbers from desktop and Fold7 measurements.
ClusterLimits DesktopClusterLimits();
ClusterLimits MobileClusterLimits();

struct ClusterViewDesc
{
	math::float4x4 view;       // world to view
	math::float4x4 projection; // a perspective projection: clip w = -view z
	std::uint32_t widthPixels = 0;
	std::uint32_t heightPixels = 0;
	// The clustered depth range as view distances. Shading clamps depths
	// outside it to the first or last slice, so the renderer sets these to
	// the view's own near and far planes.
	float nearZ = 0.0f;
	float farZ = 0.0f;
};

enum class ClusterError : std::uint8_t
{
	kInvalidViewport,   // zero width or height
	kInvalidDepthRange, // near not above 0, far not above near, or not finite
	kNotPerspective,    // clip w is not -view z, or x and y do not depend on view x and y
	kInvalidLimits,     // a zero tile size, slice count or capacity
	kTooManyFroxels,    // the grid exceeds maxFroxels
	kOverflow,          // OverflowPolicy::kFail and something did not fit
};

// The grid of one view. Everything here is also what the compute pass reads.
struct ClusterGrid
{
	ClusterLimits limits;
	std::uint32_t widthPixels = 0;
	std::uint32_t heightPixels = 0;
	std::uint32_t tilesX = 0;
	std::uint32_t tilesY = 0;
	std::uint32_t slices = 0;
	float nearZ = 0.0f;
	float farZ = 0.0f;
	// slice = floor( log( depth ) * sliceScale + sliceBias ), clamped.
	float sliceScale = 0.0f;
	float sliceBias = 0.0f;
	math::float4x4 view;
	// View-space planes through the tile edges. columnPlanes[i] passes
	// through the left edge of column i (the last one through the screen's
	// right edge) and points toward increasing columns; rowPlanes[j] passes
	// through the top edge of row j and points down the screen.
	std::vector<math::Plane> columnPlanes; // tilesX + 1
	std::vector<math::Plane> rowPlanes;    // tilesY + 1
	// View distances of the slice boundaries: sliceDepths[0] is nearZ and
	// sliceDepths[slices] is farZ.
	std::vector<float> sliceDepths; // slices + 1
	// The view-space point at distance 1 (z = -1) under each tile corner,
	// row-major from the top-left: index = row * ( tilesX + 1 ) + column.
	std::vector<math::float3> cornerRays; // ( tilesX + 1 ) * ( tilesY + 1 )
	// A grid SubdivideClusterGrid made: how its froxels divide its parent's
	// (1 and 1 for a grid CreateClusterGrid made).
	std::uint32_t tileDivisor = 1;
	std::uint32_t sliceMultiplier = 1;

	std::uint32_t FroxelCount() const { return tilesX * tilesY * slices; }
	std::uint32_t FroxelIndex( std::uint32_t x, std::uint32_t y, std::uint32_t slice ) const
	{
		return ( slice * tilesY + y ) * tilesX + x;
	}
};

[[nodiscard]] foundation::Expected<ClusterGrid, ClusterError> CreateClusterGrid(
    const ClusterViewDesc &desc, const ClusterLimits &limits );

// A finer grid of the same view whose froxels subdivide `grid`'s (the
// volumetric fog's froxels, RFC 0016 participating media): tiles of
// tileSizePixels / tileDivisor pixels and slices * sliceMultiplier depth
// slices, still spaced logarithmically, so the fine froxel (x, y, s) lies in
// `grid`'s froxel ( x / tileDivisor, y / tileDivisor, s / sliceMultiplier )
// (ParentFroxelIndex) and can read that froxel's light list. The boundaries
// the two grids share are `grid`'s own values, bitwise: every
// sliceMultiplier-th slice depth, and the corner rays and planes of every
// tileDivisor-th column and row; the others divide them. The fine limits are
// `grid`'s with the tile size, slice count and froxel capacity scaled.
// kInvalidLimits when a factor is 0 or tileDivisor does not divide the tile
// size; kInvalidDepthRange for a grid without its slice depths.
[[nodiscard]] foundation::Expected<ClusterGrid, ClusterError> SubdivideClusterGrid(
    const ClusterGrid &grid, std::uint32_t tileDivisor, std::uint32_t sliceMultiplier );

// The index, in the grid a subdivided grid was made from, of the froxel
// holding the subdivided grid's froxel ( x, y, slice ).
std::uint32_t ParentFroxelIndex(
    const ClusterGrid &fine, std::uint32_t x, std::uint32_t y, std::uint32_t slice );

// The slice holding a view distance, clamped to the grid.
std::uint32_t SliceOfDepth( const ClusterGrid &grid, float viewDistance );
// The froxel shading reads at a pixel position (x right, y down, top-left
// origin) and view distance; positions and distances outside the grid clamp
// to its edge froxels.
std::uint32_t FroxelAt( const ClusterGrid &grid, float pixelX, float pixelY, float viewDistance );

struct FroxelRange
{
	std::uint32_t offset = 0;
	std::uint32_t count = 0;

	friend constexpr bool operator==( const FroxelRange &, const FroxelRange & ) = default;
};

struct ClusterLists
{
	std::vector<FroxelRange> froxels; // FroxelCount() entries
	// Indices into the light span AssignLights took, ascending per froxel.
	std::vector<std::uint32_t> lightIndices;
};

// Two words per froxel, one bit per area light in light-set order. Unlike
// point-light lists this cannot overflow at the supported 64-light limit.
// Rectangle support planes expanded by reach conservatively bound its window.
// More than 64 lights fails without changing out. Edge slices extend to infinity
// because the shader clamps depth; adjacent screen tiles cover pixel-center shifts.
using AreaFroxelMask = std::array<std::uint32_t, 2>;
[[nodiscard]] bool AssignAreaLights( const ClusterGrid &grid,
    std::span<const area_light::AreaLight> lights, std::vector<AreaFroxelMask> &out );

// Append masks to a CPU-built ClusterIndexHeader + indices buffer. Header.w
// stores the mask offset in uints plus one (zero means no spatial assignment).
// indices contains at least the 16-byte header and is a whole number of uints.
void AppendAreaMasks( std::span<const AreaFroxelMask> masks, std::vector<std::byte> &indices );

struct ClusterStats
{
	std::uint32_t lightsClustered = 0;    // point and spot lights taken into the grid
	std::uint32_t directionalLights = 0;  // not clustered: shading applies them everywhere
	std::uint32_t invalidLights = 0;      // non-finite or negative values; skipped, counted
	std::uint32_t lightsOverCapacity = 0; // point and spot lights beyond maxLights
	std::uint32_t froxelsOverflowed = 0;  // froxels whose list lost a light
	std::uint32_t assignmentsDropped = 0; // (froxel, light) pairs lost to capacity
	std::uint32_t assignments = 0;        // indices written

	bool Overflowed() const
	{
		return lightsOverCapacity != 0 || froxelsOverflowed != 0 || assignmentsDropped != 0;
	}
};

struct ClusterFailure
{
	ClusterError error = ClusterError::kOverflow;
	ClusterStats stats;
};

// The light record the compute pass reads (std430), in the grid's view
// space (transformed in double on the CPU): position and radius (a large
// finite value for unbounded), the cone's unit axis and its outer cosine
// (-1 for a point light: its cone is every direction).
struct ClusterLightGpu
{
	float positionRadius[4] = {};
	float directionOuterCos[4] = {};
};

// The records of the lights AssignLights clusters, in its order, with the
// light-set index of each (the index list refers to light-set indices).
struct ClusterLightTable
{
	std::vector<ClusterLightGpu> lights;
	std::vector<std::uint32_t> lightSetIndex;
};

ClusterLightTable PackClusterLights(
    const ClusterGrid &grid, std::span<const light_set::RuntimeLight> lights );

// The grid as the compute pass reads it (std430 vec4s): the column planes,
// the row planes (xyz normal, w distance), the slice depths (in x), then the
// corner rays (in xyz).
std::vector<math::float4> PackClusterGrid( const ClusterGrid &grid );

} // namespace render::pass::lights

#endif // RENDER_PASS_LIGHTS_CLUSTERS_H
