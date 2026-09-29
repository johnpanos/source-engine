//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's projected lights (render.projected-light.v1, RFC 0011
//          light set v2): the client's env_projectedtextures, lit like every
//          other light.
//
//          Versions. A light keeps its version while it stays within a small
//          tolerance of the state it was published with (that state is kept);
//          a larger change, its arrival and its loss are new versions, and
//          dirty the world surfaces within its frustum's sphere, which are
//          rebuilt once when drawn. A projector at rest costs nothing.
//
//          World lightmaps take each light per texel (gl_lightmap.cpp
//          R_AddProjectedLights): its cookie, attenuation and Lambert term
//          (projected_light::IrradianceAt), shadowed by the world (a cached
//          trace) and by moving objects (render.dynamic-occlusion.v1).
//          Models take a stand-in point light at their lighting origin,
//          irradiance-matched, shadowed the same way. The light set publishes
//          them in Snapshot::projected.
//
//          Lightmap builds on the material system's thread read the
//          generation current when they were queued.
//
//===========================================================================//

#ifndef ENGINE_PROJECTED_LIGHTS_H
#define ENGINE_PROJECTED_LIGHTS_H

#include "render/projected_light.h"

#include <cstdint>
#include <vector>

struct dworldlight_t;
class Vector;

struct ProjectedLightEntry
{
	int key = 0; // the client's (the entity handle)
	uint32_t version = 0;
	projected_light::Light light;
};

// Whether env_projectedtextures are lights of the light model
// (r_projected_lights); the client then draws no flashlight pass for them.
bool ProjectedLights_Enabled();

int ProjectedLights_Generation();

// The lights of a recent generation (an evicted one reads as the current).
const std::vector<ProjectedLightEntry> &ProjectedLights_Get( int generation );

// A light's cookie at ( u, v ) (linear RGB; white without one).
void ProjectedLights_Cookie( const projected_light::Light &light, float u, float v, float rgb[3] );

// Models: the stand-in point light of a projected light at a receiver, as an
// inverse-square world light (the world's shadow applied; moving objects' are
// applied with every model light); false when it does not reach the receiver.
bool ProjectedLights_Representative(
    const ProjectedLightEntry &entry, const Vector &receiver, dworldlight_t &out );

#endif // ENGINE_PROJECTED_LIGHTS_H
