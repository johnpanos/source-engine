//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's participating media (RFC 0016 K11 step g): a map's
//			entity lump, the fog entities and lights it names as
//			render.pass.volumetric's inputs, the froxel layout copied from
//			render.pass.lights' subdivided ClusterGrid, and the projectors'
//			cookie array. Private to render.lab.
//
//			Entities (quality/fixtures/lighting/README.md, names decided by
//			the render-core owner, 2026-09-29):
//			- env_volumetric_fog_volume: origin, box_mins and box_maxs
//			  relative to it, density (extinction per unit), albedo,
//			  anisotropy (Henyey-Greenstein g), emission;
//			- env_volumetric_fog_controller: density everywhere,
//			  height_fog_density, height_fog_falloff (from the controller's
//			  origin z, 0 without one), anisotropy;
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

#ifndef RENDER_LAB_LAB_MEDIA_H
#define RENDER_LAB_LAB_MEDIA_H

#include "render/area_light.h"
#include "render/light_set.h"
#include "render/projected_light.h"

#include "lab_support.h"
#include "render/device/device.h"
#include "render/pass/lights/clusters.h"
#include "render/pass/lights/map_lights.h"
#include "render/pass/volumetric/volumetric.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

// The entity lump and a map's authored lights: render.pass.lights owns them
// (map_lights.h, shared with the product's world stage).
using Entity = pass::lights::Entity;
using pass::lights::ParseEntityLump;

// What the map's entities give the medium.
struct LabMedia
{
	bool present = false; // a fog volume or controller exists
	pass::volumetric::Medium medium;
	std::vector<pass::volumetric::MediumLight> lights;
	std::vector<pass::volumetric::MediumProjector> projectors;
	std::vector<std::string> cookieNames; // per projector, its layer's texture
	std::uint32_t unsupportedLights = 0;  // attenuations other than inverse square
};

LabMedia MediaFromEntities( const std::vector<Entity> &entities );

using LabSun = pass::lights::MapSun;
using LabLights = pass::lights::MapLights;
inline LabLights LightsFromEntities( const std::vector<Entity> &entities )
{
	return pass::lights::MapLightsFromEntities( entities );
}

// The subdivided grid as render.pass.volumetric reads it; the grid must
// outlive the layout (its slice depths are borrowed).
pass::volumetric::FroxelLayout FroxelLayoutOf( const pass::lights::ClusterGrid &grid );

// The projectors' cookies as one RGBA 2D array (at least two layers; layer i
// is projector i's cookie, the rest white), uploaded by RecordUpload.
class CookieArray
{
public:
	~CookieArray();
	// The reason when a cookie is missing, does not decode, or differs in size
	// or format from the first.
	std::optional<std::string> Create( device::IRenderDevice2 &device, const GameFiles &files,
	    const std::vector<std::string> &names );
	void RecordUpload( device::CommandEncoder &encoder );
	device::TextureId Texture() const { return m_Texture; }
	device::TextureDesc Desc() const;

private:
	device::IRenderDevice2 *m_Device = nullptr;
	device::TextureId m_Texture;
	device::BufferId m_Staging;
	std::vector<std::byte> m_Bytes;
	std::uint32_t m_Width = 0;
	std::uint32_t m_Height = 0;
	std::uint32_t m_Layers = 0;
	std::uint64_t m_LayerBytes = 0;
};

} // namespace render::lab

#endif // RENDER_LAB_LAB_MEDIA_H
