//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shadows (RFC 0016 K11, render.shadows.v1): one atlas
//			for the frame's shadowed lights, planned by render.pass.shadows'
//			PlanShadowAtlas and drawn by its ShadowDepthRenderer, and the tile
//			records the surface program reads (ShadowTileGpu, with the depth
//			mapping in params.z and w that its soft shadows need):
//			- a spot: one tile around its cone;
//			- a point light: the six faces of a cube;
//			- an area light: the five faces of a hemicube on its emitting
//			  side (six when two-sided);
//			- the sun: its cascades over the view;
//			- a projector: its frustum.
//			Cube faces are drawn wider than 90 degrees, so a soft shadow's
//			filter near a face's edge stays inside the face. Private to
//			render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_SHADOWS_H
#define RENDER_LAB_LAB_SHADOWS_H

#include "lab_media.h"

#include "render/device/device.h"
#include "render/math/matrix.h"
#include "render/pass/shadows/shadow_passes.h"
#include "render/resources/mesh_cache.h"
#include "render/shadow_tile.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

// The camera the sun's cascades cover.
struct LabShadowCamera
{
	math::float4x4 view;
	float verticalFovRadians = 1.0f;
	float aspect = 1.0f;
	float nearZ = 1.0f;
	float shadowDistance = 4096.0f;
};

struct LabShadows
{
	device::TextureId atlas;
	device::TextureDesc atlasDesc;
	std::vector<ShadowTileGpu> tiles;
	// Each light's first tile in `tiles`, or -1 (unshadowed); a point light's
	// six and an area light's five or six follow their first.
	std::vector<int> lightTiles;     // per LabLights::lights (spots: one, points: six)
	std::vector<int> lightTileCount; // 1 or 6
	std::vector<int> areaTiles;      // per LabLights::areas
	std::vector<int> projectorTiles; // per LabLights::projectors
	int sunFirst = -1;
	int sunCount = 0;
	std::uint32_t views = 0;
	std::uint32_t draws = 0;
};

// Plans and draws the atlas with the casters' meshes (positions at offset 0),
// submitted and waited for; the atlas is left in kSampled. Release it with
// ReleaseShadows once the frame is done.
std::optional<std::string> DrawShadows( device::IRenderDevice2 &device,
    pass::shadows::ShadowDepthRenderer &renderer, const LabLights &lights,
    std::span<const pass::shadows::ShadowCaster> casters, const LabShadowCamera &camera,
    LabShadows &out );

void ReleaseShadows( device::IRenderDevice2 &device, LabShadows &shadows,
    device::CompletionToken token );

} // namespace render::lab

#endif // RENDER_LAB_LAB_SHADOWS_H
