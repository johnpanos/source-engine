//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frame's shadow plan (RFC 0016 render.shadows.v1): one atlas
//			for the shadowed lights, planned by render.pass.shadows'
//			PlanShadowAtlas, and the tile records the surface program reads
//			(ShadowTileGpu, with the depth mapping in params.z and w that its
//			soft shadows need):
//			- a spot: one tile around its cone;
//			- a point light: the six faces of a cube;
//			- an area light: the five faces of a hemicube on its emitting
//			  side (six when two-sided);
//			- the sun: its cascades over the view;
//			- a projector: its frustum.
//			Cube faces are drawn wider than 90 degrees, so a soft shadow's
//			filter near a face's edge stays inside the face. A face group
//			gets all its tiles or none (the light stays unshadowed).
//			render_lab and the product's world stage both plan through it;
//			each draws the plan's views with ShadowDepthRenderer. CPU only.
//
//=============================================================================//

#ifndef RENDER_PASS_SHADOWS_SHADOW_PLAN_H
#define RENDER_PASS_SHADOWS_SHADOW_PLAN_H

#include "render/area_light.h"
#include "render/light_set.h"
#include "render/math/matrix.h"
#include "render/pass/shadows/atlas.h"
#include "render/projected_light.h"
#include "render/shadow_tile.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace render::pass::shadows
{

// The camera the sun's cascades cover.
struct ShadowCamera
{
	math::float4x4 view;
	float verticalFovRadians = 1.0f;
	float aspect = 1.0f;
	float nearZ = 1.0f;
	float shadowDistance = 4096.0f;
};

struct ShadowPlanInput
{
	std::span<const light_set::RuntimeLight> lights; // points and spots
	std::span<const area_light::AreaLight> areas;
	std::optional<math::float3> toSun; // a sun: towards it
	std::span<const projected_light::Light> projectors;
	ShadowCamera camera;
	std::uint32_t atlasSize = 8192;
	std::uint32_t guardTexels = 4;
};

struct ShadowPlanView
{
	math::float4x4 viewProjection;
	ShadowTile tile;
};

struct ShadowPlan
{
	std::uint32_t atlasSize = 0;
	std::uint32_t guardTexels = 0;
	std::vector<ShadowTileGpu> tiles;
	std::vector<ShadowPlanView> views; // parallel to tiles
	// Each light's first tile in `tiles`, or -1 (unshadowed); a point light's
	// six and an area light's five or six follow their first.
	std::vector<int> lightTiles;     // per input light
	std::vector<RuntimeShadowLayout> lightLayouts;
	std::vector<int> areaTiles;      // per input area light
	std::vector<int> projectorTiles; // per input projector
	int sunFirst = -1;
	int sunCount = 0;
};

// The reason when the sun's cascades or the plan are refused.
[[nodiscard]] std::optional<std::string> PlanShadows(
    const ShadowPlanInput &input, ShadowPlan &out );

} // namespace render::pass::shadows

#endif // RENDER_PASS_SHADOWS_SHADOW_PLAN_H
