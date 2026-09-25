//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.light-set.v1 - the runtime light set's identities, style
//          scalars, baked state, epochs and value semantics over a scripted
//          frame sequence. The sensitivity builds substitute defective
//          builders the oracle must reject.
//
//===========================================================================//

#include "render/light_set.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <set>
#include <vector>

namespace
{
using namespace light_set;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( LIGHT_SET_SEEDED_SLOT_IDS ) || defined( LIGHT_SET_SEEDED_STYLE_IGNORED ) || \
    defined( LIGHT_SET_SEEDED_FROZEN_EPOCH )
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
// A seeded build passes when the oracle rejects its defect.
unsigned long g_rejected = 0;

void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		if ( kSeeded )
		{
			++g_rejected;
			std::fprintf( stderr, "seeded defect detected: %s\n", description );
			return;
		}
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

// The builder under test: the real one, or a seeded defective one.
class UnderTest
{
public:
	void BeginMap( uint64_t serial ) { m_builder.BeginMap( serial ); }
	Snapshot Build( const std::vector<WorldLightInput> &world, const std::vector<float> &styles,
	    const std::vector<DynamicLightInput> &dynamic )
	{
		Snapshot snapshot = m_builder.Build( world, styles, dynamic );
#ifdef LIGHT_SET_SEEDED_SLOT_IDS
		// Identity from the table slot: a reused slot reuses an ID.
		for ( RuntimeLight &light : snapshot.lights )
		{
			if ( light.kind != LightKind::World )
			{
				size_t index = &light - snapshot.lights.data() - world.size();
				light.id = Builder::kFirstDynamicId + dynamic[index].slot;
			}
		}
#endif
#ifdef LIGHT_SET_SEEDED_STYLE_IGNORED
		for ( size_t i = 0; i < world.size(); ++i )
		{
			RuntimeLight &light = snapshot.lights[i];
			light.styleScalar = 1.0f;
			light.matchesBaked = true;
			for ( int k = 0; k < 3; ++k )
				light.color[k] = world[i].color[k];
		}
#endif
#ifdef LIGHT_SET_SEEDED_FROZEN_EPOCH
		snapshot.epoch = 1;
#endif
		return snapshot;
	}

private:
	Builder m_builder;
};

WorldLightInput World( uint32_t index, int style, float red )
{
	WorldLightInput input;
	input.index = index;
	input.style = style;
	input.color[0] = red;
	input.color[1] = red * 0.5f;
	input.position[0] = float( index ) * 64.0f;
	return input;
}

DynamicLightInput Dynamic( uint32_t slot, int key, LightKind kind = LightKind::Dynamic )
{
	DynamicLightInput input;
	input.kind = kind;
	input.slot = slot;
	input.key = key;
	input.color[2] = 1.0f;
	input.radius = 128.0f;
	return input;
}

const RuntimeLight *Find( const Snapshot &snapshot, LightKind kind, size_t ordinal )
{
	for ( const RuntimeLight &light : snapshot.lights )
		if ( light.kind == kind && ordinal-- == 0 )
			return &light;
	return nullptr;
}

} // namespace

int main()
{
	UnderTest builder;
	builder.BeginMap( 7 );
	const std::vector<WorldLightInput> world = { World( 0, 0, 2.0f ), World( 1, 5, 1.0f ),
		World( 2, 5, 3.0f ) };
	std::vector<float> styles( 64, 1.0f );
	std::set<uint32_t> everIssued;
	uint64_t lastEpoch = 0;
	const auto issued = [&]( const Snapshot &snapshot, const char *what )
	{
		std::set<uint32_t> seen;
		bool unique = true;
		for ( const RuntimeLight &light : snapshot.lights )
		{
			unique &= light.id != 0 && seen.insert( light.id ).second;
			everIssued.insert( light.id );
		}
		Check( unique, what );
		Check( snapshot.epoch == lastEpoch + 1, "each built frame advances the epoch by one" );
		lastEpoch = snapshot.epoch;
	};

	// Frame 1: three world lights, two dlights, one elight.
	const Snapshot f1 = builder.Build(
	    world, styles, { Dynamic( 0, 11 ), Dynamic( 3, 12 ), Dynamic( 0, 11, LightKind::Entity ) } );
	issued( f1, "frame 1: IDs are non-zero and unique" );
	Check( f1.mapSerial == 7 && f1.lights.size() == 6, "frame 1 carries every light" );
	for ( size_t i = 0; i < 3; ++i )
		Check( f1.lights[i].kind == LightKind::World && f1.lights[i].id == 1 + i &&
		           f1.lights[i].baked && f1.lights[i].matchesBaked,
		    "world lights are baked, match their bake at scalar 1, and take index IDs" );
	Check( !Find( f1, LightKind::Dynamic, 0 )->baked &&
	           !Find( f1, LightKind::Dynamic, 0 )->matchesBaked &&
	           !Find( f1, LightKind::Entity, 0 )->baked,
	    "dynamic and entity lights are not baked" );
	Check( Find( f1, LightKind::Dynamic, 0 )->id >= Builder::kFirstDynamicId,
	    "dynamic IDs never collide with world IDs" );
	Check( Find( f1, LightKind::Dynamic, 0 )->id != Find( f1, LightKind::Entity, 0 )->id,
	    "a dlight and an elight in the same slot with the same key are different lights" );

	// Frame 2: the same lights; style 5 dims to 0.25.
	styles[5] = 0.25f;
	const Snapshot f2 = builder.Build(
	    world, styles, { Dynamic( 0, 11 ), Dynamic( 3, 12 ), Dynamic( 0, 11, LightKind::Entity ) } );
	issued( f2, "frame 2: IDs are unique" );
	for ( size_t i = 0; i < f1.lights.size(); ++i )
		Check( f2.lights[i].id == f1.lights[i].id, "a surviving light keeps its ID" );
	Check( f2.lights[1].styleScalar == 0.25f && f2.lights[1].color[0] == 0.25f &&
	           f2.lights[2].color[0] == 0.75f && f2.lights[0].color[0] == 2.0f,
	    "a style scalar scales its lights' color and no other" );
	Check( !f2.lights[1].matchesBaked && f2.lights[0].matchesBaked,
	    "a styled light away from its baked scalar no longer matches its bake" );
	Check( f1.lights[1].color[0] == 1.0f && f1.lights[1].matchesBaked,
	    "an earlier snapshot keeps its values" );

	// Frame 3: slot 3's light dies; slot 0 is reused by a new key.
	styles[5] = 1.0f;
	const uint32_t oldSlot0 = Find( f2, LightKind::Dynamic, 0 )->id;
	const uint32_t oldSlot3 = Find( f2, LightKind::Dynamic, 1 )->id;
	const Snapshot f3 = builder.Build( world, styles, { Dynamic( 0, 13 ) } );
	issued( f3, "frame 3: IDs are unique" );
	Check( f3.lights.size() == 4, "frame 3 carries the surviving lights" );
	Check( Find( f3, LightKind::Dynamic, 0 )->id != oldSlot0,
	    "a slot reused by another key is a new light" );
	Check( f3.lights[1].matchesBaked, "a style back at its baked scalar matches again" );

	// Frame 4: slot 3's old key returns after a dark frame; slot 3 reuse.
	const Snapshot f4 = builder.Build( world, styles, { Dynamic( 0, 13 ), Dynamic( 3, 12 ) } );
	issued( f4, "frame 4: IDs are unique" );
	Check( Find( f4, LightKind::Dynamic, 1 )->id != oldSlot3,
	    "a light that went dark and returned is a new light" );
	bool reused = false;
	for ( const RuntimeLight &light : f4.lights )
		reused |= light.kind != LightKind::World &&
		          ( light.id == oldSlot0 || light.id == oldSlot3 );
	Check( !reused, "no ID is reused within a map" );

	// A map change: world IDs rebind, the epoch restarts, dynamics are new.
	builder.BeginMap( 8 );
	lastEpoch = 0;
	const Snapshot g1 = builder.Build( { World( 0, 0, 1.0f ) }, {}, { Dynamic( 3, 12 ) } );
	issued( g1, "new map: IDs are unique" );
	Check( g1.mapSerial == 8 && g1.lights[0].id == 1 && g1.lights[0].styleScalar == 1.0f,
	    "a new map rebinds world IDs; a style the engine does not report is 1" );

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		// The seeded defect must be caught; the build reports the oracle's verdict.
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
