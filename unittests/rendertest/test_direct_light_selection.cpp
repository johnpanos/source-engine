//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.direct-light-selection.v1 - which unbaked lights a renderer
//          with a fixed direct-light budget takes from the frame's light set
//          (render/direct_light_selection.h). The sensitivity builds
//          substitute the original table-order choice, a ranking that ignores
//          the view, and unstable ties; the oracle must reject each.
//
//===========================================================================//

#include "render/direct_light_selection.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cstdio>
#include <vector>

namespace
{
using namespace light_set;

unsigned long g_checks = 0;
unsigned long g_failures = 0;
#if defined( DIRECT_LIGHT_SELECTION_SEEDED_TABLE_ORDER ) ||                                        \
    defined( DIRECT_LIGHT_SELECTION_SEEDED_NO_DISTANCE ) ||                                        \
    defined( DIRECT_LIGHT_SELECTION_SEEDED_UNSTABLE_TIES )
constexpr bool kSeeded = true;
#else
constexpr bool kSeeded = false;
#endif
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

// The selection under test: the real one, or a seeded defective one.
std::vector<size_t> Select( const Snapshot &snapshot, size_t maxLights )
{
#if defined( DIRECT_LIGHT_SELECTION_SEEDED_TABLE_ORDER )
	// The original consumer: the first candidates in table order.
	std::vector<size_t> selected;
	for ( size_t i = 0; i < snapshot.lights.size() && selected.size() < maxLights; ++i )
		if ( IsDirectCandidate( snapshot.lights[i] ) )
			selected.push_back( i );
	return selected;
#elif defined( DIRECT_LIGHT_SELECTION_SEEDED_NO_DISTANCE )
	Snapshot copy = snapshot;
	copy.hasView = false;
	return SelectDirectLights( copy, maxLights );
#elif defined( DIRECT_LIGHT_SELECTION_SEEDED_UNSTABLE_TIES )
	// Ties to the higher ID.
	std::vector<size_t> selected = SelectDirectLights( snapshot, snapshot.lights.size() );
	std::stable_sort( selected.begin(), selected.end(),
	    [&]( size_t a, size_t b )
	    {
		    const float ia = DirectImportance( snapshot.lights[a], snapshot );
		    const float ib = DirectImportance( snapshot.lights[b], snapshot );
		    if ( ia != ib )
			    return ia > ib;
		    return snapshot.lights[a].id > snapshot.lights[b].id;
	    } );
	if ( selected.size() > maxLights )
		selected.resize( maxLights );
	return selected;
#else
	return SelectDirectLights( snapshot, maxLights );
#endif
}

RuntimeLight Dynamic( uint32_t id, float x, float brightness, float radius = 200.0f )
{
	RuntimeLight light;
	light.id = id;
	light.kind = LightKind::Dynamic;
	light.shape = LightShape::Point;
	light.position[0] = x;
	light.color[0] = light.color[1] = light.color[2] = brightness;
	light.radius = radius;
	light.minLight = 1.0f / 256.0f;
	return light;
}

bool Contains( const std::vector<size_t> &selected, size_t index )
{
	return std::find( selected.begin(), selected.end(), index ) != selected.end();
}

} // namespace

int main()
{
	// Ten unbaked lights at the viewer, the brightest last in table order; a
	// budget of seven keeps the seven brightest.
	{
		Snapshot snapshot;
		snapshot.hasView = true;
		for ( uint32_t i = 0; i < 10; ++i )
			snapshot.lights.push_back( Dynamic( 1000 + i, 0.0f, 1.0f + float( i ) ) );
		const std::vector<size_t> selected = Select( snapshot, 7 );
		Check( selected.size() == 7, "a full budget takes seven lights" );
		bool strongest = true;
		for ( size_t i = 3; i < 10; ++i )
			strongest &= Contains( selected, i );
		Check( strongest, "the budget keeps the strongest lights, not the first in table order" );
		bool ranked = selected.size() == 7;
		for ( size_t i = 1; ranked && i < selected.size(); ++i )
			ranked &= selected[i - 1] > selected[i];
		Check( ranked, "the selection is ordered strongest first" );
	}

	// Baked, directional and dark lights are never taken.
	{
		Snapshot snapshot;
		RuntimeLight baked = Dynamic( 1, 0.0f, 50.0f );
		baked.kind = LightKind::World;
		baked.baked = true;
		RuntimeLight sun = Dynamic( 2, 0.0f, 50.0f );
		sun.shape = LightShape::Directional;
		const RuntimeLight dark = Dynamic( 3, 0.0f, 0.0f );
		snapshot.lights = { baked, sun, dark, Dynamic( 4, 0.0f, 1.0f ) };
		const std::vector<size_t> selected = Select( snapshot, 7 );
		Check( selected.size() == 1 && selected[0] == 3,
		    "baked, directional and dark lights are never taken" );
	}

	// Distance: of two equal lights, the one at the viewer ranks first; a much
	// brighter light far away still outranks a faint one at the viewer.
	{
		Snapshot snapshot;
		snapshot.hasView = true;
		snapshot.viewOrigin[0] = 0.0f;
		snapshot.lights = { Dynamic( 10, 5000.0f, 2.0f ), Dynamic( 11, 50.0f, 2.0f ) };
		std::vector<size_t> selected = Select( snapshot, 1 );
		Check( selected.size() == 1 && selected[0] == 1,
		    "of two equal lights, the one near the viewer is taken" );
		snapshot.lights = { Dynamic( 12, 0.0f, 0.001f ), Dynamic( 13, 600.0f, 50.0f ) };
		selected = Select( snapshot, 1 );
		Check( selected.size() == 1 && selected[0] == 1,
		    "a far light that is far brighter outranks a faint near one" );
	}

	// Ties go to the lower ID, whatever the table order.
	{
		Snapshot snapshot;
		snapshot.hasView = true;
		snapshot.lights = {
		    Dynamic( 30, 0.0f, 1.0f ), Dynamic( 20, 0.0f, 1.0f ), Dynamic( 25, 0.0f, 1.0f ) };
		const std::vector<size_t> selected = Select( snapshot, 2 );
		Check( selected.size() == 2 && selected[0] == 1 && selected[1] == 2,
		    "equal lights are taken lowest ID first" );
	}

	// A light's falloff sets its strength: an inverse-square light and a
	// legacy one of the same color and radius rank by their falloff.
	{
		Snapshot snapshot;
		snapshot.hasView = true;
		RuntimeLight bulb = Dynamic( 40, 0.0f, 1.0f, 300.0f );
		bulb.falloff = LightFalloff::InverseSquare;
		bulb.sourceRadius = kInverseSquareSourceRadius;
		const RuntimeLight legacy = Dynamic( 41, 0.0f, 1.0f, 300.0f );
		snapshot.lights = { legacy, bulb };
		const float expectBulb = InverseSquareFalloff( 150.0f * 150.0f, 300.0f, 2.0f );
		const float expectLegacy = Falloff( 150.0f * 150.0f, 300.0f, 1.0f / 256.0f );
		const std::vector<size_t> selected = Select( snapshot, 1 );
		Check( selected.size() == 1 && selected[0] == ( expectBulb > expectLegacy ? 1u : 0u ),
		    "falloff kinds rank by their own falloff" );
	}

	// A budget above the candidates takes them all; an empty set takes none.
	{
		Snapshot snapshot;
		snapshot.lights = { Dynamic( 50, 0.0f, 1.0f ), Dynamic( 51, 0.0f, 2.0f ) };
		std::vector<size_t> selected = Select( snapshot, 7 );
		Check( selected.size() == 2 && selected[0] == 1, "a large budget takes every candidate" );
		Check( Select( Snapshot(), 7 ).empty(), "no lights, no selection" );
	}

	if ( kSeeded )
	{
		std::fprintf( stderr, "%lu seeded rejection(s)\n", g_rejected );
		if ( g_rejected == 0 )
		{
			std::fprintf( stderr, "FAIL: the seeded defect was not detected\n" );
			return testing::ReportConformance( g_checks, g_checks );
		}
		return testing::ReportConformance( g_checks, 0 );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
