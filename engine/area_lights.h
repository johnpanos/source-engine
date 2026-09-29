//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's area lights (render.area-light.v1, RFC 0011 light
//          set v2): the client's emitting surfaces (render/area_light.h),
//          each carried in a dlight slot flagged DLIGHT_AREA so the paths
//          that walk dlights find it. The slot's color is black, so a path
//          that does not know area lights adds nothing.
//
//          Owner of each light's identity and version. A light keeps its
//          version while it stays within a small tolerance of the state it
//          was published with (the published state is kept, so a light that
//          creeps does not drift); any larger change, or its loss, is a new
//          version.
//
//          Lightmaps. A world surface records which light versions its
//          lightmap holds. A light marks (and so rebuilds) only surfaces that
//          do not hold its current version, so an unchanged light costs no
//          rebuild; surfaces holding a version that is gone are dirtied and
//          rebuilt once. Every rebuild evaluates all the lights of its
//          generation that reach the surface, exactly (area_light::
//          FormFactor), so a rebuild for any other reason keeps them. Brush
//          entity surfaces move, so they are marked every frame a light
//          reaches them, as dlights are.
//
//          Threads. Lightmaps may be built on the material system's thread;
//          each publish is a new generation, kept in a small ring, and a
//          build evaluates the generation current when it was queued.
//
//          Known gaps: displacements, static props and the WMSH world draw no
//          area light yet; there is no occlusion (a light reaches through
//          walls within its reach).
//
//===========================================================================//

#ifndef ENGINE_AREA_LIGHTS_H
#define ENGINE_AREA_LIGHTS_H

#include "render/area_light.h"

#include <cstdint>

struct dlight_t;
struct dworldlight_t;
struct msurface2_t;
class Vector;
struct matrix3x4_t;

struct AreaLightSlot
{
	int slot = -1;
	int key = 0; // the client's key
	uint32_t version = 0;
	area_light::AreaLight light;
};

// The generation lightmap builds queued now should evaluate.
int AreaLights_Generation();

// The lights of a generation (a recent one; an evicted generation reads as the
// current one). Returns the count, at most area_light::kMaxAreaLights.
int AreaLights_Get( int generation, AreaLightSlot *out );

// The dlight slots that carry area lights now.
unsigned int AreaLights_SlotMask();

// The current light in a dlight slot, or null.
const AreaLightSlot *AreaLights_ForSlot( int slot );

// Whether a surface is the world's (not a brush entity's, which moves).
bool AreaLights_IsWorldSurface( msurface2_t *surfID );

// Lightmaps: whether the slot's light must mark this world surface (it does
// not hold the light's current version, or it is a brush entity surface).
bool AreaLights_ShouldMark( int slot, msurface2_t *surfID, bool worldSurface );

// Lightmaps: whether a surface holds a light version that is gone and must be
// rebuilt even though no light marked it.
bool AreaLights_IsDirty( msurface2_t *surfID );

// Lightmaps: a rebuilt surface now holds these light versions (world surfaces
// only; brush entity surfaces pass worldSurface false and are not recorded).
void AreaLights_Applied(
    msurface2_t *surfID, bool worldSurface, const AreaLightSlot *lights, int count );

// Models: a stand-in point light for the slot's area light at a receiver
// (area_light::RepresentativeAt), written into `out` as an inverse-square
// world light. Returns false when the light does not reach the receiver.
bool AreaLights_Representative( int slot, const Vector &receiver, dworldlight_t &out );

// Models and points: the exact light a point with a normal receives from the
// slot's area light, added to `color`.
void AreaLights_AddAtPoint( int slot, const Vector &point, const Vector &normal, Vector &color );

#endif // ENGINE_AREA_LIGHTS_H
