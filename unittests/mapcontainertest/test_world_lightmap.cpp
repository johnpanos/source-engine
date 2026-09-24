//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for LMAP validation (RFC 0008 F4, RFC 0011 layers).
//
//  - The shared case table (world_lightmap_cases.h): v1 pages, v2 layered
//    pages with their fixed layer roles, version agreement, and each
//    malformation with its structured error.
//  - Layer lookup by role.
//  - Seeded mutation fuzzing: no input crashes the validator, and any
//    accepted layout stays inside the bytes it was given.
//
//=============================================================================//

#include "world_lightmap_cases.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

using namespace mapcontainer;

int main()
{
	unsigned long checks = 0;
	unsigned long failures = 0;
	const auto check = [&]( bool condition, const char *what )
	{
		++checks;
		if ( !condition )
		{
			++failures;
			std::printf( "FAIL %s\n", what );
		}
	};
	const lmap_cases::CaseResult table = lmap_cases::RunLmapCases( &ValidateWorldLightmap, true );
	check( table.cases >= 25, "the LMAP case table ran" );
	check( table.wrong == 0, "every LMAP case reports its structured result" );
	checks += unsigned( table.cases );

	WorldLightmapLayout layout{};
	const std::vector<char> three = lmap_cases::MakeLmap( 8, 4, 3 );
	check( ValidateWorldLightmap( three.data(), three.size(), 2, &layout ) ==
	           WorldLightmapError::Ok,
	    "three-layer page validates" );
	check( WorldLightmapLayerIndex( layout, WorldLightmapLayer::Total ) == 0 &&
	           WorldLightmapLayerIndex( layout, WorldLightmapLayer::Direct ) == 1 &&
	           WorldLightmapLayerIndex( layout, WorldLightmapLayer::Indirect ) == 2,
	    "three layers are total, direct, indirect" );
	check( layout.layerOffset[1] == layout.layerOffset[0] + layout.layerBytes &&
	           layout.layerBytes == 8u * 4u * 8u,
	    "layers are contiguous RGBA16F pages" );
	const std::vector<char> two = lmap_cases::MakeLmap( 8, 4, 2 );
	check( ValidateWorldLightmap( two.data(), two.size(), 2, &layout ) == WorldLightmapError::Ok &&
	           WorldLightmapLayerIndex( layout, WorldLightmapLayer::Direct ) == -1 &&
	           WorldLightmapLayerIndex( layout, WorldLightmapLayer::Indirect ) == 1,
	    "two layers are total and indirect, with no direct layer" );
	check( ValidateWorldLightmap( nullptr, 0, 0, &layout ) == WorldLightmapError::Truncated,
	    "no bytes are rejected" );
	check( std::string( WorldLightmapLayerName( WorldLightmapLayer::Indirect ) ) == "indirect" &&
	           std::string( WorldLightmapErrorName( WorldLightmapError::VersionMismatch ) ) ==
	               "version-mismatch",
	    "layer and error names" );

	// Seeded mutation fuzzing of the header and level index.
	const char *seedText = std::getenv( "CONFORMANCE_SEED" );
	std::mt19937 random( seedText ? unsigned( std::strtoul( seedText, nullptr, 10 ) ) : 20260924u );
	int accepted = 0;
	bool bounded = true;
	for ( int trial = 0; trial < 20000; ++trial )
	{
		std::vector<char> bytes = trial % 2 ? two : three;
		const int flips = 1 + int( random() % 4 );
		for ( int f = 0; f < flips; ++f )
			bytes[random() % 180] = char( random() );
		if ( random() % 8 == 0 )
			bytes.resize( random() % bytes.size() );
		WorldLightmapLayout fuzzed{};
		if ( ValidateWorldLightmap( bytes.data(), bytes.size(), 0, &fuzzed ) ==
		     WorldLightmapError::Ok )
		{
			++accepted;
			for ( uint32_t i = 0; i < fuzzed.layerCount; ++i )
				bounded = bounded && fuzzed.layerOffset[i] + fuzzed.layerBytes <= bytes.size();
			bounded = bounded && fuzzed.layerCount >= 1 && fuzzed.layerCount <= 3;
		}
	}
	std::printf( "fuzz: %d of 20000 mutations accepted (a flip may leave a valid page)\n",
	    accepted );
	check( bounded, "every accepted mutation's layers stay inside its bytes" );
	return testing::ReportConformance( checks, failures );
}
