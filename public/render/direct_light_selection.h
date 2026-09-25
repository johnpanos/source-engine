//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Which unbaked lights a renderer with a fixed direct-light budget
//          takes from the frame's light set (render.direct-light-selection.v1,
//          RFC 0011). The native world path takes at most a few point and spot
//          lights; when more exist, it keeps the strongest at the viewer, not
//          the first in table order.
//
//          A light's importance is its luminance, times its falloff at half
//          its reach (its strength), times how much of the view its reach
//          covers: reach^2 / ( reach^2 + d^2 ), d the distance from the view
//          to the light (1 without a view). Reach is the light's radius, or
//          four reference distances for an unbounded inverse-square light.
//          Ties go to the lower ID, so the choice is deterministic.
//
//===========================================================================//

#ifndef RENDER_DIRECT_LIGHT_SELECTION_H
#define RENDER_DIRECT_LIGHT_SELECTION_H

#include "render/light_set.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace light_set
{

// Whether a light is a candidate for the direct budget: not in the bake, and
// not directional (the budget holds point and spot lights).
[[nodiscard]] inline bool IsDirectCandidate( const RuntimeLight &light )
{
	return !light.baked && light.shape != LightShape::Directional;
}

[[nodiscard]] inline float DirectReach( const RuntimeLight &light )
{
	if ( light.radius > 0.0f )
		return light.radius;
	return 4.0f * kInverseSquareReferenceDistance;
}

[[nodiscard]] inline float DirectImportance( const RuntimeLight &light, const Snapshot &snapshot )
{
	const float luminance =
	    0.2126f * light.color[0] + 0.7152f * light.color[1] + 0.0722f * light.color[2];
	if ( !( luminance > 0.0f ) )
		return 0.0f;
	const float reach = DirectReach( light );
	const float half = 0.25f * reach * reach;
	const float strength = light.falloff == LightFalloff::InverseSquare
	                           ? InverseSquareFalloff( half, light.radius, light.sourceRadius )
	                           : Falloff( half, light.radius, light.minLight );
	float coverage = 1.0f;
	if ( snapshot.hasView )
	{
		float d2 = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			const float delta = light.position[k] - snapshot.viewOrigin[k];
			d2 += delta * delta;
		}
		coverage = reach * reach / ( reach * reach + d2 );
	}
	return luminance * strength * coverage;
}

// The indices (into snapshot.lights) of at most maxLights candidates, the most
// important first; lights of no importance are never taken.
[[nodiscard]] inline std::vector<size_t> SelectDirectLights(
    const Snapshot &snapshot, size_t maxLights )
{
	struct Ranked
	{
		float importance;
		uint32_t id;
		size_t index;
	};
	std::vector<Ranked> ranked;
	for ( size_t i = 0; i < snapshot.lights.size(); ++i )
	{
		const RuntimeLight &light = snapshot.lights[i];
		if ( !IsDirectCandidate( light ) )
			continue;
		const float importance = DirectImportance( light, snapshot );
		if ( importance > 0.0f )
			ranked.push_back( { importance, light.id, i } );
	}
	std::sort( ranked.begin(), ranked.end(),
	    []( const Ranked &a, const Ranked &b )
	    {
		    if ( a.importance != b.importance )
			    return a.importance > b.importance;
		    return a.id < b.id;
	    } );
	std::vector<size_t> selected;
	for ( size_t i = 0; i < ranked.size() && i < maxLights; ++i )
		selected.push_back( ranked[i].index );
	return selected;
}

} // namespace light_set

#endif // RENDER_DIRECT_LIGHT_SELECTION_H
