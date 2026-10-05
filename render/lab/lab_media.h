//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's names for a map's lights, participating media
//			and projector cookies (RFC 0016 K11 step g). Each has one owner
//			shared with the product's world stage: render.pass.lights'
//			map_lights.h (the entity lump and the authored lights) and
//			render.composition's map_media.h (the medium, its froxels) and
//			projector_cookies.h (the cookie array). Private to render.lab.
//
//=============================================================================//

#ifndef RENDER_LAB_LAB_MEDIA_H
#define RENDER_LAB_LAB_MEDIA_H

#include "render/area_light.h"
#include "render/light_set.h"
#include "render/projected_light.h"

#include "lab_support.h"
#include "render/device/device.h"
#include "render/composition/map_media.h"
#include "render/composition/projector_cookies.h"
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

// What the map's entities give the medium: render.composition owns the
// parse (map_media.h, shared with the product's world stage).
using LabMedia = composition::MapMedia;
using composition::MediaFromEntities;

using LabSun = pass::lights::MapSun;
using LabLights = pass::lights::MapLights;
inline LabLights LightsFromEntities( const std::vector<Entity> &entities,
    pass::lights::EntityConvention convention = pass::lights::EntityConvention::kPortal )
{
	return pass::lights::MapLightsFromEntities( entities, convention );
}

using composition::FroxelLayoutOf;

// The projectors' cookies as one RGBA 2D array: render.composition owns
// them (projector_cookies.h, shared with the product's world stage).
using composition::CookieArray;

} // namespace render::lab

#endif // RENDER_LAB_LAB_MEDIA_H
