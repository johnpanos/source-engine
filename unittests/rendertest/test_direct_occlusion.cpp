//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.direct-occlusion (RFC 0011): baked direct light blocked by
//          moving geometry (render/direct_occlusion.h), on a synthetic floor
//          (100 x 100 units, a 32 x 32 lightmap whose total is direct 1 plus
//          indirect 0.1):
//
//  - No occluder: the total is the bake's bytes exactly.
//  - A box between the floor's centre and a rectangle light above it takes
//    the centre's direct light (total 0.1 left) and leaves a corner whose
//    path passes beside it exactly as baked.
//  - A distant light straight down: the box shadows its footprint only.
//  - A box nowhere near any path changes nothing; a box below the floor
//    (behind its lit side) changes nothing.
//  - A floor wound downward is lit from above all the same.
//
//===========================================================================//

#include "render/direct_occlusion.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace indirect_light;

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &what )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::printf( "FAIL %s\n", what.c_str() );
	}
}

constexpr uint32_t kSize = 32;

std::vector<unsigned char> Layer( float value )
{
	std::vector<unsigned char> layer( size_t( kSize ) * kSize * 8 );
	const uint16_t half = FloatToHalf( value ), one = FloatToHalf( 1.0f );
	for ( size_t t = 0; t < size_t( kSize ) * kSize; ++t )
	{
		for ( int c = 0; c < 3; ++c )
			std::memcpy( &layer[t * 8 + 2 * c], &half, 2 );
		std::memcpy( &layer[t * 8 + 6], &one, 2 );
	}
	return layer;
}

float Red( const std::vector<unsigned char> &layer, uint32_t x, uint32_t y )
{
	uint16_t half;
	std::memcpy( &half, &layer[( size_t( y ) * kSize + x ) * 8], 2 );
	return mapcontainer::HalfToFloat( half );
}

mapcontainer::SdfLight RectAbove()
{
	mapcontainer::SdfLight light = {};
	light.kind = uint32_t( mapcontainer::SdfLightKind::Rect );
	light.style = -1;
	light.rgb[0] = light.rgb[1] = light.rgb[2] = 10.0f;
	light.a[0] = 50.0f, light.a[1] = 50.0f, light.a[2] = 100.0f;
	light.b[0] = 10.0f; // b x c = -z: it emits down
	light.c[1] = -10.0f;
	return light;
}

mapcontainer::SdfLight SunDown()
{
	mapcontainer::SdfLight light = {};
	light.kind = uint32_t( mapcontainer::SdfLightKind::Distant );
	light.style = -1;
	light.rgb[0] = light.rgb[1] = light.rgb[2] = 3.0f;
	light.a[2] = -1.0f; // travels down
	light.b[0] = 0.01f;
	return light;
}

// A small light above the floor's centre: a sphere, or a spot aiming down
// with vrad's cone (full within 18 degrees, none beyond 26).
mapcontainer::SdfLight SmallAbove( bool spot )
{
	mapcontainer::SdfLight light = {};
	light.kind =
	    uint32_t( spot ? mapcontainer::SdfLightKind::Spot : mapcontainer::SdfLightKind::Sphere );
	light.style = -1;
	light.rgb[0] = light.rgb[1] = light.rgb[2] = 1000.0f;
	light.a[0] = 50.0f, light.a[1] = 50.0f, light.a[2] = 100.0f;
	if ( spot )
	{
		light.b[2] = -1.0f;
		light.c[0] = 2.0f;
		light.c[1] = 0.95f;
		light.c[2] = 0.90f;
		light.reserved[0] = 1.0f;
	}
	else
		light.b[0] = 2.0f;
	return light;
}

Proxy Box( float x0, float y0, float z0, float x1, float y1, float z1 )
{
	Proxy box;
	box.lo[0] = x0, box.lo[1] = y0, box.lo[2] = z0;
	box.hi[0] = x1, box.hi[1] = y1, box.hi[2] = z1;
	return box;
}

} // namespace

int main()
{
	// The floor: two triangles, front face up, UVs spanning the page.
	const std::vector<float> positions = { 0, 0, 0, 100, 0, 0, 100, 100, 0, 0, 100, 0 };
	const std::vector<float> uvs = { 0, 0, 1, 0, 1, 1, 0, 1 };
	const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
	const std::vector<unsigned char> total = Layer( 1.1f ), direct = Layer( 1.0f );
	std::vector<unsigned char> out;

	for ( const bool sun : { false, true } )
	{
		const std::string name = sun ? "distant light" : "rectangle light";
		const mapcontainer::SdfLight light = sun ? SunDown() : RectAbove();
		DirectOcclusion occlusion;
		Check( occlusion.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(),
		           direct.data(), std::span<const mapcontainer::SdfLight>( &light, 1 ) ),
		    name + ": builds" );
		Check( occlusion.CoveredTexels() == kSize * kSize, name + ": every texel is on the floor" );

		Check( occlusion.Compose( {}, nullptr, &out ) == 0 && out == total,
		    name + ": no occluder leaves the bake's bytes" );

		const Proxy between = Box( 40, 40, 40, 60, 60, 60 );
		const size_t blocked =
		    occlusion.Compose( std::span<const Proxy>( &between, 1 ), nullptr, &out );
		const float centre = Red( out, kSize / 2, kSize / 2 );
		const float corner = Red( out, 0, 0 );
		std::printf( "%s: %zu texels blocked, centre %.4f, corner %.4f\n", name.c_str(), blocked,
		    centre, corner );
		Check( std::fabs( centre - 0.1f ) < 0.01f,
		    name + ": the box takes the centre's direct light (0.1 indirect left)" );
		Check( corner == Red( total, 0, 0 ), name + ": a corner beside the box keeps its light" );
		if ( sun )
		{
			// Straight down: the footprint x, y in 40..60 (texels 13..18) only.
			bool footprint = true;
			for ( uint32_t y = 0; y < kSize; ++y )
				for ( uint32_t x = 0; x < kSize; ++x )
				{
					const float cx = ( float( x ) + 0.5f ) * 100.0f / kSize;
					const float cy = ( float( y ) + 0.5f ) * 100.0f / kSize;
					const bool under = cx > 40.0f && cx < 60.0f && cy > 40.0f && cy < 60.0f;
					footprint = footprint && ( under ? Red( out, x, y ) < 0.11f
					                                 : Red( out, x, y ) == Red( total, x, y ) );
				}
			Check( footprint, name + ": only the box's footprint loses its light" );
		}

		const Proxy away = Box( 300, 300, 0, 320, 320, 20 );
		Check( occlusion.Compose( std::span<const Proxy>( &away, 1 ), nullptr, &out ) == 0 &&
		           out == total,
		    name + ": a box beside every path changes nothing" );
		const Proxy below = Box( 40, 40, -60, 60, 60, -40 );
		Check( occlusion.Compose( std::span<const Proxy>( &below, 1 ), nullptr, &out ) == 0 &&
		           out == total,
		    name + ": a box behind the lit side changes nothing" );
	}

	// Sphere and spot lights: a box over the centre takes its direct light
	// from either; a box on the path to a corner darkens it under the sphere,
	// but not under the spot, whose cone never reaches the corner (35
	// degrees off its axis).
	for ( const bool spot : { false, true } )
	{
		const std::string name = spot ? "spot light" : "sphere light";
		const mapcontainer::SdfLight light = SmallAbove( spot );
		DirectOcclusion occlusion;
		Check( occlusion.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(),
		           direct.data(), std::span<const mapcontainer::SdfLight>( &light, 1 ) ),
		    name + ": builds" );
		const Proxy between = Box( 40, 40, 40, 60, 60, 60 );
		occlusion.Compose( std::span<const Proxy>( &between, 1 ), nullptr, &out );
		Check( std::fabs( Red( out, kSize / 2, kSize / 2 ) - 0.1f ) < 0.01f,
		    name + ": the box over the centre takes its direct light" );
		const Proxy cornerPath = Box( 8, 8, 18, 22, 22, 32 );
		occlusion.Compose( std::span<const Proxy>( &cornerPath, 1 ), nullptr, &out );
		const float corner = Red( out, 0, 0 );
		std::printf( "%s: corner %.4f behind a box on its path (baked %.4f)\n", name.c_str(),
		    corner, Red( total, 0, 0 ) );
		Check( spot ? corner == Red( total, 0, 0 ) : std::fabs( corner - 0.1f ) < 0.01f,
		    spot ? name + ": outside the cone, the corner keeps its light"
		         : name + ": the corner loses the light the box blocks" );
	}

	// A floor wound the other way (its face normal down) is lit from above
	// all the same: its texels face their light.
	{
		const std::vector<uint32_t> reversed = { 0, 2, 1, 0, 3, 2 };
		const mapcontainer::SdfLight light = RectAbove();
		DirectOcclusion occlusion;
		const Proxy between = Box( 40, 40, 40, 60, 60, 60 );
		Check( occlusion.BuildLayers( positions, uvs, reversed, kSize, kSize, total.data(),
		           direct.data(), std::span<const mapcontainer::SdfLight>( &light, 1 ) ) &&
		           occlusion.Compose( std::span<const Proxy>( &between, 1 ), nullptr, &out ) > 0 &&
		           std::fabs( Red( out, kSize / 2, kSize / 2 ) - 0.1f ) < 0.01f,
		    "a floor wound downward still loses the light the box blocks" );
	}

	// Recompose (a change of occluders) equals Compose byte for byte: a room
	// (a floor chart and a wall chart on a 128 x 128 page) under a rectangle,
	// a sphere, a spot and a distant light, through 60 seeded steps of boxes
	// moving, appearing and leaving; every texel that changed is in a tile
	// Recompose reports. Negative control: told that nothing moved when a box
	// did, it keeps stale texels, which the comparison catches.
	{
		constexpr uint32_t kRoom = 128;
		const std::vector<float> roomPositions = { 0, 0, 0, 100, 0, 0, 100, 100, 0, 0, 100, 0, 0,
		    100, 0, 100, 100, 0, 100, 100, 100, 0, 100, 100 };
		const std::vector<float> roomUvs = {
		    0, 0, 0.49f, 0, 0.49f, 1, 0, 1, 0.51f, 0, 1, 0, 1, 1, 0.51f, 1 };
		const std::vector<uint32_t> roomIndices = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 };
		std::vector<unsigned char> roomTotal( size_t( kRoom ) * kRoom * 8 ),
		    roomDirect( size_t( kRoom ) * kRoom * 8 );
		for ( size_t t = 0; t < size_t( kRoom ) * kRoom; ++t )
			for ( int c = 0; c < 4; ++c )
			{
				const float d = c == 3 ? 1.0f : 0.5f + 0.5f * float( ( t * 7 + c ) % 13 ) / 13.0f;
				const uint16_t hd = FloatToHalf( d ), ht = FloatToHalf( c == 3 ? 1.0f : d + 0.1f );
				std::memcpy( &roomDirect[t * 8 + 2 * c], &hd, 2 );
				std::memcpy( &roomTotal[t * 8 + 2 * c], &ht, 2 );
			}
		mapcontainer::SdfLight lights[4] = {
		    RectAbove(), SmallAbove( false ), SmallAbove( true ), SunDown() };
		lights[1].a[0] = 20.0f, lights[1].a[1] = 70.0f, lights[1].a[2] = 60.0f;
		lights[3].a[1] = 0.6f, lights[3].a[2] = -0.8f; // slanting onto the wall
		DirectOcclusion occlusion;
		Check( occlusion.BuildLayers( roomPositions, roomUvs, roomIndices, kRoom, kRoom,
		           roomTotal.data(), roomDirect.data(), lights ),
		    "room: builds" );
		uint32_t seed = 12345;
		const auto next = [&]()
		{
			return ( seed = seed * 1664525u + 1013904223u ) >> 8;
		};
		const auto uniform = [&]( float lo, float hi )
		{
			return lo + ( hi - lo ) * float( next() % 10000 ) / 10000.0f;
		};
		std::vector<Proxy> previous, proxies;
		std::vector<unsigned char> incremental, whole, before;
		std::vector<DirectOcclusion::Rect> dirty;
		bool equal = true, covered = true, skipped = false;
		size_t recomposed = 0, steps = 0;
		for ( int step = 0; step < 60; ++step )
		{
			previous = proxies;
			const uint32_t action = next() % 4;
			if ( action == 0 || proxies.empty() )
			{
				const float x = uniform( 0, 90 ), y = uniform( 0, 90 ), z = uniform( 5, 80 );
				proxies.push_back( Box(
				    x, y, z, x + uniform( 3, 20 ), y + uniform( 3, 20 ), z + uniform( 3, 20 ) ) );
			}
			else if ( action == 1 && proxies.size() > 1 )
				proxies.erase( proxies.begin() + ptrdiff_t( next() % proxies.size() ) );
			else
			{
				// A box moves a little, as a door or a lift does between frames.
				Proxy &box = proxies[next() % proxies.size()];
				const float dx = uniform( -4, 4 ), dz = uniform( -4, 4 );
				box.lo[0] += dx, box.hi[0] += dx, box.lo[2] += dz, box.hi[2] += dz;
			}
			before = incremental;
			recomposed += occlusion.Recompose( previous, proxies, nullptr, &incremental, &dirty );
			occlusion.Compose( proxies, nullptr, &whole );
			equal = equal && incremental == whole;
			++steps;
			if ( before.size() == incremental.size() )
				for ( uint32_t y = 0; y < kRoom && covered; ++y )
					for ( uint32_t x = 0; x < kRoom && covered; ++x )
					{
						const size_t at = ( size_t( y ) * kRoom + x ) * 8;
						if ( std::memcmp( &before[at], &incremental[at], 8 ) == 0 )
							continue;
						bool in = false;
						for ( const DirectOcclusion::Rect &r : dirty )
							in = in || ( x >= r.x && x < r.x + r.width && y >= r.y &&
							               y < r.y + r.height );
						covered = in;
					}
			// The negative control on a copy: "nothing moved".
			std::vector<unsigned char> stale = before;
			if ( stale.size() == incremental.size() && previous != proxies )
			{
				std::vector<DirectOcclusion::Rect> none;
				occlusion.Recompose( proxies, proxies, nullptr, &stale, &none );
				skipped = skipped || stale != whole;
			}
		}
		std::printf( "room: %zu steps, %zu texels recomposed (of %zu a step whole)\n", steps,
		    recomposed, occlusion.CoveredTexels() );
		Check( equal, "room: Recompose equals Compose byte for byte at every step" );
		Check( covered, "room: every changed texel is in a reported tile" );
		Check( recomposed < steps * occlusion.CoveredTexels(),
		    "room: Recompose visits fewer texels than Compose" );
		Check( skipped, "room: negative control: a Recompose told nothing moved is caught" );
	}

	// Without a light there is nothing to occlude: not built.
	{
		DirectOcclusion occlusion;
		Check( !occlusion.BuildLayers(
		           positions, uvs, indices, kSize, kSize, total.data(), direct.data(), {} ),
		    "no lights: not built" );
	}

	// Only some triangles (the surfaces whose baked direct light is still
	// drawn): their texels alone are covered and recomposed.
	{
		const mapcontainer::SdfLight light = SunDown();
		const std::span<const mapcontainer::SdfLight> lights( &light, 1 );
		const std::vector<DirectOcclusion::IndexRange> first = { { 0, 3 } };
		DirectOcclusion part;
		Check( part.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(), direct.data(),
		           lights, &first ),
		    "index ranges: one triangle builds" );
		const size_t covered = part.CoveredTexels();
		Check( covered > 0 && covered < kSize * kSize,
		    "index ranges: only the kept triangle's texels are covered" );
		// A box over the whole floor darkens the kept triangle (x > y: the
		// triangle 0, 1, 2) and leaves the other's texels as baked.
		const Proxy roof = Box( -10, -10, 40, 110, 110, 60 );
		part.Compose( std::span<const Proxy>( &roof, 1 ), nullptr, &out );
		Check( std::fabs( Red( out, kSize - 2, 1 ) - 0.1f ) < 0.01f &&
		           Red( out, 1, kSize - 2 ) == Red( total, 1, kSize - 2 ),
		    "index ranges: the kept triangle loses its direct light, the other keeps its bytes" );
		const std::vector<DirectOcclusion::IndexRange> none;
		Check( !part.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(),
		           direct.data(), lights, &none ) &&
		           !part.Ready(),
		    "index ranges: an empty list builds nothing" );
		const std::vector<DirectOcclusion::IndexRange> past = { { 3, 6 } };
		Check( !part.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(),
		           direct.data(), lights, &past ),
		    "index ranges: a range past the indices is refused" );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
