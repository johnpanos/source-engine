//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pass.shadows shadow atlas (RFC 0016 K7, render.shadows.v1).
//			One depth atlas per frame holds every shadowed view (spot lights,
//			flashlights, the sun's cascades). Each profile has an atlas size
//			and a caster budget.
//
//			Planning is deterministic: requests are ranked by priority, then
//			by the tile size they want, then by key. Every request within the
//			budget keeps a tile while the atlas holds them all at the
//			profile's minimum size: short of room, the lowest-ranked tile
//			halves first, and a request loses its tile (lowest rank first)
//			only when every tile is at the minimum; reduced tiles then grow
//			back in rank order while room remains. The tiles are
//			power-of-two squares taken from a quadtree, largest first (a
//			buddy allocator, so tiles never overlap and stay inside the
//			atlas). Nothing fails silently: every
//			request comes back with a status, and requests beyond the budget
//			or with no room left are reported as such.
//
//			Conventions (render/device/conventions.h): texture row 0 at the
//			top, clip Y up, clip depth 0 to 1.
//
//=============================================================================//

#ifndef RENDER_PASS_SHADOWS_ATLAS_H
#define RENDER_PASS_SHADOWS_ATLAS_H

#include "foundation/expected.h"
#include "render/math/matrix.h"
#include "render/math/vector.h"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace render::pass::shadows
{

struct ShadowAtlasLimits
{
	std::uint32_t atlasSize = 4096; // square, a power of two
	std::uint32_t minTileSize = 128;
	std::uint32_t maxTileSize = 2048;
	std::uint32_t casterBudget = 32; // shadowed views per frame
	// Texels kept free inside each tile's edge so filtering never reads a
	// neighbor; the view renders into the rest.
	std::uint32_t guardTexels = 2;
};

// Provisional per-profile limits; the K7 atlas budget rows (render-v1.json)
// set the final numbers from desktop and Fold7 measurements.
ShadowAtlasLimits DesktopShadowAtlasLimits();
ShadowAtlasLimits MobileShadowAtlasLimits();

enum class ShadowAtlasError : std::uint8_t
{
	kInvalidLimits, // sizes not powers of two, min above max, max above the atlas, no budget,
	                // or a guard band that leaves the smallest tile nothing
};

struct ShadowRequest
{
	std::uint64_t key = 0;       // the caller's identity (a light id), unique per plan
	float priority = 0.0f;       // higher is served first
	float screenCoverage = 0.0f; // the light's projected extent over the view height, 0 to 1
};

// The tile size a coverage asks for: the power of two at or above coverage
// times the maximum tile size, clamped to the limits.
std::uint32_t DesiredTileSize( const ShadowAtlasLimits &limits, float screenCoverage );

enum class ShadowTileStatus : std::uint8_t
{
	kAllocated,      // the desired size
	kReduced,        // a smaller size (atlas space ran short)
	kOverBudget,     // ranked beyond the caster budget
	kAtlasFull,      // no room even at the minimum size
	kInvalidRequest, // non-finite priority or coverage, or a repeated key
};

struct ShadowTile
{
	std::uint32_t x = 0; // texels from the atlas's left edge
	std::uint32_t y = 0; // texels from the atlas's top row
	std::uint32_t size = 0;

	friend constexpr bool operator==( const ShadowTile &, const ShadowTile & ) = default;
};

struct ShadowAllocation
{
	std::uint64_t key = 0;
	ShadowTileStatus status = ShadowTileStatus::kInvalidRequest;
	std::uint32_t desiredSize = 0;
	ShadowTile tile; // valid for kAllocated and kReduced

	bool HasTile() const
	{
		return status == ShadowTileStatus::kAllocated || status == ShadowTileStatus::kReduced;
	}
};

struct ShadowAtlasPlan
{
	ShadowAtlasLimits limits;
	std::vector<ShadowAllocation> allocations; // in request order
	std::uint32_t allocated = 0;
	std::uint32_t reduced = 0;
	std::uint32_t overBudget = 0;
	std::uint32_t atlasFull = 0;
	std::uint32_t invalid = 0;
};

[[nodiscard]] foundation::Expected<ShadowAtlasPlan, ShadowAtlasError> PlanShadowAtlas(
    const ShadowAtlasLimits &limits, std::span<const ShadowRequest> requests );

// Where a view renders inside its tile: the tile less its guard band, in
// atlas texels.
struct ShadowViewport
{
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	std::uint32_t size = 0;
};

ShadowViewport TileViewport( const ShadowTile &tile, std::uint32_t guardTexels );

// Maps a shadow view's normalized device coordinates onto its tile's
// viewport in atlas texture space (u right and v down across the whole
// atlas, 0 to 1): u = x * scaleU + biasU, v = y * scaleV + biasV, so x -1..1
// and y 1..-1 cover the viewport. It applies after the perspective divide:
// folding the tile's offset into a world matrix would multiply it into the
// view's large translation and lose precision far from the atlas's origin.
struct ShadowTileTransform
{
	float scaleU = 0.0f;
	float biasU = 0.0f;
	float scaleV = 0.0f;
	float biasV = 0.0f;
};

ShadowTileTransform TileTransform(
    const ShadowTile &tile, std::uint32_t atlasSize, std::uint32_t guardTexels );

// A shadow view bound to its tile: the view's world to clip matrix, the
// tile transform, and the viewport's rectangle in texture coordinates.
struct ShadowTileProjection
{
	math::float4x4 viewProjection;
	ShadowTileTransform transform;
	float u0 = 0.0f;
	float v0 = 0.0f;
	float u1 = 0.0f;
	float v1 = 0.0f;
};

ShadowTileProjection MakeTileProjection( const math::float4x4 &viewProjection,
    const ShadowTile &tile, std::uint32_t atlasSize, std::uint32_t guardTexels );

// ( u, v, depth ) of a world point in its tile, or empty when the point is
// behind the view (w not above 0), outside depth 0 to 1, or outside the
// tile's viewport.
std::optional<math::float3> ProjectToShadowTile(
    const ShadowTileProjection &projection, const math::float3 &world );

} // namespace render::pass::shadows

#endif // RENDER_PASS_SHADOWS_ATLAS_H
