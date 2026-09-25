//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The runtime light set (render.light-set.v1, RFC 0011): the lights
//          of one frame as an immutable value snapshot the engine builds on
//          the main thread before render work is scheduled, and consumers
//          (the native world path's direct light; later the indirect
//          producers and clustered lighting) borrow read-only for that frame.
//
//          Identity. A world light's ID is fixed by its map index for the
//          whole map. A dynamic light (dlight, entity light) is keyed by its
//          kind, table slot and key: it keeps its ID while it stays alive in
//          the same slot with the same key, and gets a new ID when its slot is
//          reused or it goes dark and comes back. No ID is reused within a
//          map, so a consumer can cache per-light work by ID.
//
//          Baked state. `baked` says the light's contribution is in the map's
//          bake (every world light is); `matchesBaked` says its current state
//          equals the baked one (a world light whose style scalar is its baked
//          value, 1). Dynamic lights are never baked.
//
//          Units: positions in Source units, colors linear and scaled by the
//          light's current scalar; no native or engine types.
//
//===========================================================================//

#ifndef RENDER_LIGHT_SET_H
#define RENDER_LIGHT_SET_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <map>
#include <span>
#include <tuple>
#include <vector>

namespace render::lightset
{
static const char *const kLightSetConsumerInterface = "RenderLightSetConsumer001";

enum class LightKind : uint8_t
{
	World,   // LUMP_WORLDLIGHTS (or World Stage lights)
	Dynamic, // dlight_t
	Entity,  // elight (model-only dynamic light)
};

enum class LightShape : uint8_t
{
	Point,
	Spot,
	Directional,
};

struct RuntimeLight
{
	uint32_t id = 0; // 0 is never a valid ID
	LightKind kind = LightKind::World;
	LightShape shape = LightShape::Point;
	bool baked = false;
	bool matchesBaked = false;
	float position[3] = {};
	float direction[3] = { 0, 0, -1 }; // spot and directional lights
	float color[3] = {};               // linear, times the current scalar
	float radius = 0.0f;               // 0: unbounded
	float innerCos = 1.0f;             // spot cone
	float outerCos = 1.0f;
	int style = 0;
	float styleScalar = 1.0f;
};

struct Snapshot
{
	uint64_t mapSerial = 0;
	uint64_t epoch = 0; // advances by one per built frame
	std::vector<RuntimeLight> lights;
};

// The per-frame source data the engine gathers.
struct WorldLightInput
{
	uint32_t index = 0; // map index
	LightShape shape = LightShape::Point;
	float position[3] = {};
	float direction[3] = { 0, 0, -1 };
	float color[3] = {};
	float radius = 0.0f;
	float innerCos = 1.0f;
	float outerCos = 1.0f;
	int style = 0;
};

struct DynamicLightInput
{
	LightKind kind = LightKind::Dynamic;
	uint32_t slot = 0;
	int key = 0;
	float position[3] = {};
	float direction[3] = { 0, 0, -1 };
	float color[3] = {};
	float radius = 0.0f;
	float innerCos = 1.0f;
	float outerCos = 1.0f;
	bool spot = false;
};

// A world light matches its bake when its style scalar is the baked value.
constexpr float kBakedStyleScalar = 1.0f;
constexpr float kStyleScalarTolerance = 1.0e-4f;

// Builds snapshots with stable identities; the engine owns one per map.
class Builder
{
public:
	// Starts a map: world IDs are rebound and the dynamic identities dropped.
	void BeginMap( uint64_t mapSerial )
	{
		m_mapSerial = mapSerial;
		m_epoch = 0;
		m_live.clear();
		m_nextDynamic = kFirstDynamicId;
	}

	// `styleScalars[s]` is style s's current scalar; a style outside it is 1.
	[[nodiscard]] Snapshot Build( std::span<const WorldLightInput> world,
	    std::span<const float> styleScalars, std::span<const DynamicLightInput> dynamic )
	{
		Snapshot snapshot;
		snapshot.mapSerial = m_mapSerial;
		snapshot.epoch = ++m_epoch;
		snapshot.lights.reserve( world.size() + dynamic.size() );
		for ( const WorldLightInput &input : world )
		{
			RuntimeLight light;
			light.id = 1 + input.index;
			light.kind = LightKind::World;
			light.shape = input.shape;
			light.baked = true;
			light.style = input.style;
			light.styleScalar =
			    input.style >= 0 && size_t( input.style ) < styleScalars.size()
			        ? styleScalars[size_t( input.style )]
			        : kBakedStyleScalar;
			light.matchesBaked =
			    std::fabs( light.styleScalar - kBakedStyleScalar ) <= kStyleScalarTolerance;
			for ( int k = 0; k < 3; ++k )
			{
				light.position[k] = input.position[k];
				light.direction[k] = input.direction[k];
				light.color[k] = input.color[k] * light.styleScalar;
			}
			light.radius = input.radius;
			light.innerCos = input.innerCos;
			light.outerCos = input.outerCos;
			snapshot.lights.push_back( light );
		}
		std::map<Key, uint32_t> live;
		for ( const DynamicLightInput &input : dynamic )
		{
			const Key key{ uint8_t( input.kind ), input.slot, input.key };
			if ( live.count( key ) )
				continue; // one light per slot and key
			const auto previous = m_live.find( key );
			const uint32_t id =
			    previous != m_live.end() ? previous->second : m_nextDynamic++;
			live[key] = id;
			RuntimeLight light;
			light.id = id;
			light.kind = input.kind;
			light.shape = input.spot ? LightShape::Spot : LightShape::Point;
			for ( int k = 0; k < 3; ++k )
			{
				light.position[k] = input.position[k];
				light.direction[k] = input.direction[k];
				light.color[k] = input.color[k];
			}
			light.radius = input.radius;
			light.innerCos = input.innerCos;
			light.outerCos = input.outerCos;
			snapshot.lights.push_back( light );
		}
		// A light absent this frame loses its identity: if its slot and key
		// return, it is a new light.
		m_live.swap( live );
		return snapshot;
	}

	// Dynamic IDs start above every world light's.
	static constexpr uint32_t kFirstDynamicId = 1u << 20;

private:
	using Key = std::tuple<uint8_t, uint32_t, int>;
	uint64_t m_mapSerial = 0;
	uint64_t m_epoch = 0;
	uint32_t m_nextDynamic = kFirstDynamicId;
	std::map<Key, uint32_t> m_live;
};

// A renderer that consumes the frame's light set (queried by
// kLightSetConsumerInterface). Publish is called on the main thread once per
// frame before its render work; the consumer copies what it keeps.
class ILightSetConsumer
{
public:
	virtual ~ILightSetConsumer() = default;
	virtual void PublishLightSet( const Snapshot &snapshot ) = 0;
};

} // namespace render::lightset

#endif // RENDER_LIGHT_SET_H
