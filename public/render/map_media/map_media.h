//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A map's participating media (RFC 0016 "Lighting model", the
//			volumetric term; K11 step g, K12): the fog entities, lights and
//			projectors of its entity lump as render.pass.volumetric's inputs,
//			and the froxel layout copied from render.pass.lights' ClusterGrid.
//			The one owner shared by the product's world stage (CoreWorld)
//			and render_lab, so the game and the lab read the same medium.
//			Its own module (render.map-media), beside the composition, so
//			render_lab reads it without linking the product composition.
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

#ifndef RENDER_MAP_MEDIA_MAP_MEDIA_H
#define RENDER_MAP_MEDIA_MAP_MEDIA_H

#include "render/pass/lights/clusters.h"
#include "render/pass/lights/map_lights.h"
#include "render/pass/volumetric/volumetric.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace render::map_media
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
// 2 x 2 (32-pixel froxels, twice the light grid's slices) and 1 x 1 x 2
// samples: 0.73 / 2.2 / 10.2 ms at 1024x768 / 1080p / 4K against 56 / 183 /
// 688 ms for the earlier 8 x 4 and 2 x 2 x 4; the image moves by 0.75 mean
// and 14.6 p99 /255 at 1080p (foggy-hall nave).
inline constexpr std::uint32_t kFroxelTileDivisor = 2;
inline constexpr std::uint32_t kFroxelSliceMultiplier = 2;
inline constexpr pass::volumetric::VolumetricSampling kFroxelSampling{ 1, 2 };
// The froxels' depth range (their logarithmic slices), the same in every
// view whatever its projection's planes, so the game and the lab slice the
// medium alike; the composite reads distance through the view's projection.
inline constexpr float kFroxelNearZ = 1.0f;
inline constexpr float kFroxelFarZ = 65536.0f;

// The grid as render.pass.volumetric reads it; the grid must outlive the
// layout (its slice depths are borrowed).
pass::volumetric::FroxelLayout FroxelLayoutOf( const pass::lights::ClusterGrid &grid );

} // namespace render::map_media

#endif // RENDER_MAP_MEDIA_MAP_MEDIA_H
