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
	light.b[0] = 10.0f;  // b x c = -z: it emits down
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

	// Without a light there is nothing to occlude: not built.
	{
		DirectOcclusion occlusion;
		Check( !occlusion.BuildLayers( positions, uvs, indices, kSize, kSize, total.data(),
		           direct.data(), {} ),
		    "no lights: not built" );
	}
	return testing::ReportConformance( g_checks, g_failures );
}
