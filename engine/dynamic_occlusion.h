//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The engine's moving occluders (render.dynamic-occlusion.v1,
//          RFC 0011): the client's boxes for the frame, versioned, and the
//          world surfaces their shadows can reach.
//
//          Versions. A box (entity, part) keeps its version while it stays
//          within a small tolerance of the pose it was published with (that
//          pose is kept, so a box that creeps does not drift); any larger
//          change, its arrival and its loss are new versions.
//
//          Lightmaps. A box can shadow world surfaces within its shadow reach
//          (kShadowReachScale times its radius, at most kMaxShadowReach). When
//          a box changes, every world surface within reach of its old or new
//          pose is dirtied and rebuilt once (when drawn); a box at rest costs
//          nothing. Every rebuild evaluates the boxes of its generation that
//          reach the surface (gl_lightmap.cpp R_ApplyDynamicOcclusion), so a
//          rebuild for any other reason keeps their shadows. Lightmap builds
//          on the material system's thread read the generation current when
//          they were queued.
//
//          Static visibility. A lightmap texel loses a light's blocked share
//          only where the light reached it in the bake: the world is traced
//          from the texel to the light once per ( texel, light ) that a box
//          blocks, and the answer cached for the map.
//
//===========================================================================//

#ifndef ENGINE_DYNAMIC_OCCLUSION_H
#define ENGINE_DYNAMIC_OCCLUSION_H

#include "render/dynamic_occlusion.h"

#include <cstdint>
#include <vector>

struct msurface2_t;
struct dworldlight_t;
class Vector;

constexpr float kShadowReachScale = 6.0f;
constexpr float kMaxShadowReach = 384.0f;

struct OccluderEntry
{
	dynamic_occlusion::Box box;
	uint32_t version = 0;
};

// Whether moving objects shadow the world and models (r_dynamic_occlusion).
bool DynamicOcclusion_Enabled();

int DynamicOcclusion_Generation();

// The boxes of a recent generation (an evicted one reads as the current).
const std::vector<OccluderEntry> &DynamicOcclusion_Get( int generation );

// A box's shadow reach.
float DynamicOcclusion_Reach( const dynamic_occlusion::Box &box );

// Lightmaps: whether a surface must be rebuilt for a box that changed.
bool DynamicOcclusion_IsDirty( msurface2_t *surfID );

// Lightmaps: a surface was rebuilt with the boxes of a generation.
void DynamicOcclusion_Rebuilt( msurface2_t *surfID );

// Lightmaps: whether the world lets world light `light` reach `point` (the
// bake's own visibility), cached per ( surface, luxel, light ).
bool DynamicOcclusion_StaticVisible(
    msurface2_t *surfID, int luxel, int light, const Vector &point, const Vector &normal );

// The same for any light: the world traced from `point` to `target` (a sky
// light: until it leaves the world), cached under `lightKey` (0..4095; world
// lights use their index, other lights keys from 2048 up).
bool DynamicOcclusion_StaticVisibleTo( msurface2_t *surfID, int luxel, int lightKey,
    const Vector &point, const Vector &normal, const Vector &target, bool sky );

// Dirties the world surfaces within `radius` of `center`: each is rebuilt once
// when drawn (DynamicOcclusion_IsDirty). For any light whose contribution changed.
void DynamicLightmaps_DirtySphere( const Vector &center, float radius );

// The visibility of a light of `lightRadius` at `lightPosition` past the boxes
// (render.dynamic-occlusion.v1), `self`'s boxes ignored (0: none).
float DynamicOcclusion_VisibilityFrom(
    const Vector &lightPosition, float lightRadius, const Vector &receiver, int self );

// The samples of world light `light` as seen from `receiver`.
dynamic_occlusion::Samples DynamicOcclusion_WorldLightSamples(
    const dworldlight_t &light, const Vector &receiver );

// Models: the visibility of a world or dynamic light (as a world light) at a
// model's lighting origin, the model's own boxes ignored (`self`: its entity
// handle).
float DynamicOcclusion_ModelVisibility(
    const dworldlight_t &light, const Vector &receiver, int self );

#endif // ENGINE_DYNAMIC_OCCLUSION_H
