//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Self-illuminated world geometry as area lights (RFC 0011 light set
//          v2): brush faces and overlays whose material is $selfillum (the
//          chamber icons, exit signs), fitted to rectangles by the same policy
//          and emission rule as models (render/emissive_area_lights.h,
//          materialsystem/selfillum_emission.h), at tint 1. Built once per
//          map. The client ranks them with every other emitter
//          (IAreaLights::GetWorldEmitters) at the material's live tint.
//
//          Faces vrad already lit as texture lights (the map's emit_surface
//          world lights) are left out: their light is in the bake.
//
//===========================================================================//

#ifndef ENGINE_WORLD_EMITTERS_H
#define ENGINE_WORLD_EMITTERS_H

#include "render/area_light.h"

#include <vector>

class IMaterial;

struct WorldEmitter
{
	area_light::AreaLight light; // radiance at tint 1; reach unset
	IMaterial *material;         // referenced for the map
};

// The map's world emitters (built on first use after a map loads).
const std::vector<WorldEmitter> &WorldEmitters_Get();

#endif // ENGINE_WORLD_EMITTERS_H
