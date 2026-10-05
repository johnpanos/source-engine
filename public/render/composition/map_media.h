//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A map's participating media (RFC 0016 "Lighting model", the
//			volumetric term; K11 step g, K12): the fog entities, lights and
//			projectors of its entity lump as render.pass.volumetric's inputs,
//			and the froxel layout copied from render.pass.lights' ClusterGrid.
//			The one owner shared by the product's world stage (CoreWorld)
//			and render_lab, so the game and the lab read the same medium.
//
//			Entities (quality/fixtures/lighting/README.md, names decided by
//			the render-core owner, 2026-09-29):
//			- env_volumetric_fog_volume: origin, box_mins and box_maxs
//			  relative to it, density (extinction per unit), albedo,
//			  anisotropy (Henyey-Greenstein g), emission;
//			- env_volumetric_fog_controller: density everywhere,
//			  height_fog_density, height_fog_falloff (from the controller's
//			  origin z, 0 without one), anisotropy;
//			Its lights and projectors are render.pass.lights' MapLights
//			(map_lights.h), the surfaces' own set, through MediumLightsFrom:
//			- light and light_spot as vrad compiles them (map_utils.cpp
//			  SetupLightNormalFromProps, lightmap.cpp): "_light" is
//			  GammaToLinear( rgb ) and a brightness, the inverse-square
//			  color is ( rgb / 255 )^2.2 x brightness / 255 (the diffuse
//			  light at 100 units, render.light-set.v1), a spot's normal is
//			  ( cos yaw cos pitch, sin yaw cos pitch, sin pitch ) with its
//			  "pitch" key, and its cones are degrees ( at most 90 );
//			- env_projectedtexture as render.projected-light.v1: angles by
//			  AngleVectors, lightfov both ways, nearz, farz, lightcolor as
//			  GammaToLinear( rgb ) x brightness / 255, Portal's fixed
//			  attenuation ( 0, 100, 0 ), texturename the cookie.
//
//=============================================================================//

#ifndef RENDER_COMPOSITION_MAP_MEDIA_H
#define RENDER_COMPOSITION_MAP_MEDIA_H

#include "render/pass/lights/clusters.h"
#include "render/pass/lights/map_lights.h"
#include "render/pass/volumetric/volumetric.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace render::composition
{

// What the map's entities give the medium.
struct MapMedia
{
	bool present = false; // a fog volume or controller exists
	pass::volumetric::Medium medium;
	std::vector<pass::volumetric::MediumLight> lights;
	std::vector<pass::volumetric::MediumProjector> projectors;
	std::vector<std::string> cookieNames; // per projector, its layer's texture
	std::uint32_t unsupportedLights = 0;  // lights the medium does not take (MediumLightsFrom)
};

MapMedia MediaFromEntities( const std::vector<pass::lights::Entity> &entities,
    pass::lights::EntityConvention convention = pass::lights::EntityConvention::kPortal );

// The lights the medium takes from a light set: points and spots with the
// inverse-square or legacy falloff (render.light-set.v1), at most
// kMaxMediumLights; each other light counts in *unsupported.
std::vector<pass::volumetric::MediumLight> MediumLightsFrom(
    std::span<const light_set::RuntimeLight> lights, std::uint32_t *unsupported );

// The medium's froxel grid and samples, shared by the product's world stage
// and render_lab: the view's light grid (64-pixel tiles) subdivided
// kFroxelTileDivisor across and kFroxelSliceMultiplier in depth, and the
// inject stage's stratified samples per froxel. Chosen against the frame
// budget (user decision 2026-10-05: coarser froxels and fewer samples
// approved; RFC/0016-progress.md).
inline constexpr std::uint32_t kFroxelTileDivisor = 8;
inline constexpr std::uint32_t kFroxelSliceMultiplier = 4;
inline constexpr pass::volumetric::VolumetricSampling kFroxelSampling{ 2, 4 };

// The grid as render.pass.volumetric reads it; the grid must outlive the
// layout (its slice depths are borrowed).
pass::volumetric::FroxelLayout FroxelLayoutOf( const pass::lights::ClusterGrid &grid );

} // namespace render::composition

#endif // RENDER_COMPOSITION_MAP_MEDIA_H
