//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A map's authored lights (RFC 0016 K12, "Feed it the game's
//			lights"): the entity lump's lights as the frame's light set
//			(render.light-set.v1, v2) takes them, one definition for
//			render_lab and the product's world stage. Maps compiled without
//			vrad (the PBRT and USD pipelines, the lighting fixtures) carry no
//			worldlights lump; their lights are these.
//
//			Entities (quality/fixtures/lighting/README.md):
//			- light and light_spot as vrad compiles them (map_utils.cpp
//			  SetupLightNormalFromProps, lightmap.cpp): "_light" is
//			  GammaToLinear( rgb ) and a brightness, the inverse-square
//			  color is ( rgb / 255 )^2.2 x brightness / 255 (the diffuse
//			  light at 100 units), a spot's normal is ( cos yaw cos pitch,
//			  sin yaw cos pitch, sin pitch ) with its "pitch" key, and its
//			  cones are degrees ( at most 90 ); explicit constant/linear/
//			  quadratic attenuation uses vrad's intensity scaling at 100 units.
//			  Nonzero _fifty_percent_distance is refused until its solver is
//			  represented here;
//			- light_rect: an area light (render/area_light.h);
//			- light_environment: the sun;
//			- env_projectedtexture as render.projected-light.v1: angles by
//			  AngleVectors, lightfov both ways, nearz, farz, lightcolor as
//			  GammaToLinear( rgb ) x brightness / 255, texturename the cookie.
//			Every light the bake saw is baked (its diffuse light is in the
//			lightmap and the probe volume; the surface program adds its
//			specular lobe); projectors are never baked (RFC 0011).
//
//=============================================================================//

#ifndef RENDER_PASS_LIGHTS_MAP_LIGHTS_H
#define RENDER_PASS_LIGHTS_MAP_LIGHTS_H

#include "render/area_light.h"
#include "render/light_set.h"
#include "render/math/vector.h"
#include "render/projected_light.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace render::pass::lights
{

using Entity = std::map<std::string, std::string>;

// The entity lump's blocks of quoted key/value pairs; nullopt when it does
// not parse.
std::optional<std::vector<Entity>> ParseEntityLump( const std::string &text );

// An entity's values: a key's numbers ( "a b c" ) padded with `fill`, its
// presence and its text.
std::vector<float> Numbers(
    const Entity &entity, const std::string &key, std::size_t count, float fill = 0.0f );
bool Has( const Entity &entity, const std::string &key );
std::string Value( const Entity &entity, const std::string &key );
math::float3 Vector3( const std::vector<float> &v );
// GammaToLinear( rgb / 255 ) times a brightness / 255.
math::float3 GammaColor( const std::vector<float> &light );
// The engine's AngleVectors (pitch positive looks down).
void AngleVectors( const std::vector<float> &angles, math::float3 &forward, math::float3 &right,
    math::float3 &up );

// The sun: light_environment as vrad compiles it: `_light` is its diffuse
// light on a surface facing it (the lightmap unit, E / pi), the direction
// from its `angles` with the `pitch` key, SunSpreadAngle the disc's angular
// diameter in degrees.
struct MapSun
{
	math::float3 toSun{ 0, 0, 1 }; // unit, towards the sun
	math::float3 color{ 0, 0, 0 }; // the lightmap unit
	float spreadDegrees = 0.0f;
	math::float3 ambient{ 0, 0, 0 }; // `_ambient`, the sky's radiance (the lightmap unit)
};

struct MapLights
{
	std::vector<light_set::RuntimeLight> lights;    // light, light_spot
	std::vector<area_light::AreaLight> areas;       // light_rect
	std::optional<MapSun> sun;                      // light_environment
	std::vector<projected_light::Light> projectors; // env_projectedtexture
	std::vector<std::string> cookieNames;           // per projector
	std::uint32_t unsupported = 0;                  // unsupported authored falloff controls
};

MapLights MapLightsFromEntities( const std::vector<Entity> &entities );

// Add the always-on authored lights missing from a frame's compiled world
// lights. A relit BSP can retain one switchable world light while its baked
// world lights were removed; that one must not hide all of the map's lamps.
std::vector<light_set::RuntimeLight> MergeMapLights(
    const std::vector<light_set::RuntimeLight> &frame, const MapLights &map );

} // namespace render::pass::lights

#endif // RENDER_PASS_LIGHTS_MAP_LIGHTS_H
