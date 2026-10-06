//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for LMAP validation (RFC 0008 F4, RFC 0011 layers).
//
//  - The shared case table (world_lightmap_cases.h): LMAP v3 lumps with
//    their fixed layer roles, the lump version, and each malformation with
//    its structured error.
//  - The CPU decode of known BC6H/BC7 blocks into the RGBA16F form.
//  - Layer lookup by role.
//  - Seeded mutation fuzzing: no input crashes the validator, and any
//    accepted layout stays inside the bytes it was given.
//
//=============================================================================//

#include "world_lightmap_cases.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
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
	check( table.cases >= 21, "the LMAP case table ran" );
	check( table.wrong == 0, "every LMAP case reports its structured result" );
	checks += unsigned( table.cases );

	WorldLightmapBlocks blocks{};
	const std::vector<char> three = lmap_cases::MakeLmap( 6, 5, 3, true );
	check( ValidateWorldLightmap( three.data(), three.size(), 3, &blocks ) == WorldLightmapError::Ok,
	    "three-layer lump validates" );
	check( WorldLightmapLayerIndex( blocks, WorldLightmapLayer::Total ) == 0 &&
	           WorldLightmapLayerIndex( blocks, WorldLightmapLayer::Direct ) == 1 &&
	           WorldLightmapLayerIndex( blocks, WorldLightmapLayer::Indirect ) == 2 && blocks.sun,
	    "three layers are total, direct, indirect, with the sun" );
	check( blocks.irradianceBytes == 2u * 2u * 16u &&
	           blocks.irradianceOffset[1] == blocks.gradientOffset[0] + blocks.gradientBytes,
	    "partial blocks are padded and layers are contiguous" );

	// The decoded form: irradiance left, beta right, the sun in layer 0's alpha.
	std::vector<std::byte> decoded;
	WorldLightmapLayout layout{};
	check( DecodeWorldLightmap( three.data(), blocks, &decoded, &layout ) &&
	           layout.width == 12 && layout.height == 5 && layout.layerCount == 3 &&
	           layout.layerBytes == 12u * 5u * 8u && decoded.size() == 3 * layout.layerBytes &&
	           WorldLightmapLayerIndex( layout, WorldLightmapLayer::Indirect ) == 2,
	    "decoded layout is the 2:1 RGBA16F page per layer" );
	const auto texel = [&]( uint32_t layer, uint32_t x, uint32_t y, int c )
	{
		uint16_t half;
		std::memcpy( &half,
		    decoded.data() + layout.layerOffset[layer] + ( size_t( y ) * layout.width + x ) * 8 +
		        2 * c,
		    2 );
		return half;
	};
	// Halves: 0.5 = 0x3800, 0.25 = 0x3400, 1 = 0x3c00; beta 255 -> 1, 128 -> ~0.0039,
	// 0 -> -1 (0xbc00); sun 204/255 = 0.8 (0x3a66).
	check( texel( 0, 5, 4, 0 ) == 0x3800 && texel( 0, 5, 4, 1 ) == 0x3400 &&
	           texel( 0, 5, 4, 2 ) == 0x3c00 && texel( 0, 5, 4, 3 ) == 0x3a66,
	    "irradiance and the sun decode on layer 0" );
	check( texel( 1, 0, 0, 3 ) == 0x3c00, "other layers carry alpha 1" );
	check( texel( 2, 6, 0, 0 ) == 0x3c00 && texel( 2, 11, 4, 2 ) == 0xbc00 &&
	           texel( 2, 6, 0, 1 ) > 0 && texel( 2, 6, 0, 1 ) < 0x2000 &&
	           texel( 2, 6, 0, 3 ) == 0x3c00,
	    "beta decodes on the right half" );

	const std::vector<char> two = lmap_cases::MakeLmap( 8, 4, 2 );
	check( ValidateWorldLightmap( two.data(), two.size(), 3, &blocks ) == WorldLightmapError::Ok &&
	           WorldLightmapLayerIndex( blocks, WorldLightmapLayer::Direct ) == -1 &&
	           WorldLightmapLayerIndex( blocks, WorldLightmapLayer::Indirect ) == 1 && !blocks.sun,
	    "two layers are total and indirect, with no direct layer" );
	check( ValidateWorldLightmap( nullptr, 0, 0, &blocks ) == WorldLightmapError::Truncated,
	    "no bytes are rejected" );
	check( std::string( WorldLightmapLayerName( WorldLightmapLayer::Indirect ) ) == "indirect" &&
	           std::string( WorldLightmapErrorName( WorldLightmapError::UnsupportedVersion ) ) ==
	               "unsupported-version",
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
			bytes[random() % 96] = char( random() );
		if ( random() % 8 == 0 )
			bytes.resize( random() % bytes.size() );
		WorldLightmapBlocks fuzzed{};
		if ( ValidateWorldLightmap( bytes.data(), bytes.size(), 0, &fuzzed ) ==
		     WorldLightmapError::Ok )
		{
			++accepted;
			for ( uint32_t i = 0; i < fuzzed.layerCount; ++i )
				bounded = bounded && fuzzed.gradientOffset[i] + fuzzed.gradientBytes <= bytes.size();
			bounded = bounded && fuzzed.layerCount >= 1 && fuzzed.layerCount <= 3;
		}
	}
	std::printf(
	    "fuzz: %d of 20000 mutations accepted (a flip may leave a valid page)\n", accepted );
	check( bounded, "every accepted mutation's layers stay inside its bytes" );
	return testing::ReportConformance( checks, failures );
}
