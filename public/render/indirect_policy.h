//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The indirect-light policy (render.indirect-policy.v1, RFC 0011):
//          the one owner of what a map's lightmap holds and what an indirect
//          producer publishes, so that no light is counted twice.
//
//          Policy           World reads                       Dynamic objects read
//          Baked            the lightmap's total layer        the baked volume
//          BakedPlusDelta   total + the producer's delta      baked volume + delta
//          RuntimeIndirect  direct layer + producer indirect  the runtime volume
//
//          RuntimeIndirect needs the bake's separated direct layer (LMAP v2
//          with direct and indirect layers). The policy is part of the world
//          shader's variant key: a variant that adds producer indirect must
//          read the direct layer, and one that reads the total layer must add
//          none. Light styles stay in the lightmap under every policy.
//
//===========================================================================//

#ifndef RENDER_INDIRECT_POLICY_H
#define RENDER_INDIRECT_POLICY_H

#include <cstdint>

namespace indirect_policy
{
enum class Policy : uint8_t
{
	Baked = 0,
	BakedPlusDelta = 1,
	RuntimeIndirect = 2,
};

enum class WorldLightmapLayer : uint8_t
{
	Total,
	Direct,
};

// What a policy composes.
struct Selection
{
	WorldLightmapLayer worldLayer = WorldLightmapLayer::Total;
	bool worldAddsProducerIndirect = false; // the producer's full indirect light
	bool worldAddsProducerDelta = false;    // the producer's signed change
	bool modelsReadRuntimeVolume = false;   // else the baked volume (+ delta)
};

[[nodiscard]] constexpr Selection Select( Policy policy )
{
	switch ( policy )
	{
	case Policy::BakedPlusDelta:
		return { WorldLightmapLayer::Total, false, true, false };
	case Policy::RuntimeIndirect:
		return { WorldLightmapLayer::Direct, true, false, true };
	case Policy::Baked:
	default:
		return { WorldLightmapLayer::Total, false, false, false };
	}
}

// Whether a map's lightmap supports a policy.
[[nodiscard]] constexpr bool Available(
    Policy policy, bool lightmapHasDirect, bool lightmapHasIndirect )
{
	return policy != Policy::RuntimeIndirect || ( lightmapHasDirect && lightmapHasIndirect );
}

// The world's diffuse light under a policy, from the map's layers and the
// producer's output: the composition every renderer implements and the
// furnace oracle checks (Baked total = direct + indirect).
[[nodiscard]] constexpr float WorldDiffuseLight(
    Policy policy, float total, float direct, float producerIndirect, float producerDelta )
{
	const Selection selection = Select( policy );
	float light = selection.worldLayer == WorldLightmapLayer::Total ? total : direct;
	if ( selection.worldAddsProducerIndirect )
		light += producerIndirect;
	if ( selection.worldAddsProducerDelta )
		light += producerDelta;
	return light;
}

} // namespace indirect_policy

#endif // RENDER_INDIRECT_POLICY_H
