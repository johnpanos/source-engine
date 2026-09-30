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
#include "render/pass/volumetric/volumetric.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace render::lab
{

using Entity = std::map<std::string, std::string>;

// The entity lump's blocks of quoted key/value pairs; nullopt when it does
// not parse.
std::optional<std::vector<Entity>> ParseEntityLump( const std::string &text );

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

// The sun: light_environment as vrad compiles it (README "Lights in the
// entity lump"): `_light` is its diffuse light on a surface facing it (the
// lightmap unit, E / pi), the direction from its `angles` with the `pitch`
// key, SunSpreadAngle the disc's angular diameter in degrees.
struct LabSun
{
	math::float3 toSun{ 0, 0, 1 }; // unit, towards the sun
	math::float3 color{ 0, 0, 0 };  // the lightmap unit
	float spreadDegrees = 0.0f;
	math::float3 ambient{ 0, 0, 0 }; // `_ambient`, the sky's radiance (the lightmap unit)
};

// The map's lights as the frame's light set (render.light-set.v1, v2): every
// light the bake saw is baked (its diffuse light is in the lightmap and the
// probe volume; the surface program adds its specular lobe); projectors are
// never baked (RFC 0011).
struct LabLights
{
	std::vector<light_set::RuntimeLight> lights;  // light, light_spot
	std::vector<area_light::AreaLight> areas;      // light_rect
	std::optional<LabSun> sun;                     // light_environment
	std::vector<projected_light::Light> projectors; // env_projectedtexture
	std::vector<std::string> cookieNames;           // per projector
	std::uint32_t unsupported = 0; // attenuations other than inverse square
};

LabLights LightsFromEntities( const std::vector<Entity> &entities );

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
