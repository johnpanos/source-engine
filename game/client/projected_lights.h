//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: env_projectedtexture as a light of the light model (render.
//          projected-light.v1, RFC 0011): each frame, every lit projected
//          texture submits its light, and the lights go to the engine, which
//          lights the world and models with them (shadowed by the world and by
//          moving objects). While this is active the projected textures draw no
//          legacy flashlight pass.
//
//=============================================================================//
#ifndef PROJECTED_LIGHTS_H
#define PROJECTED_LIGHTS_H
#ifdef _WIN32
#pragma once
#endif

#include "render/projected_light.h"

// Whether projected textures are lights of the light model (an engine that
// takes them, r_projected_lights 1).
bool ProjectedLights_Active();

// This frame's light of the projected texture with entity handle `nKey`.
void ProjectedLights_Submit( int nKey, const projected_light::Light &light );

#endif // PROJECTED_LIGHTS_H
