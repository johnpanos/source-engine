//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared LMAP (world lightmap) fixtures and the case table every
//          LMAP validator is judged by (RFC 0008 F4 / RFC 0011 layers).
//
//          MakeLmap writes an LMAP v3 lump of constant blocks; RunLmapCases
//          applies valid, malformed and version cases to a validator
//          with ValidateWorldLightmap's signature and returns the number of
//          cases it got wrong. The real validator must score zero; the
//          sensitivity suite shows deliberately permissive ones do not.
//
//===========================================================================//

#ifndef UNITTESTS_MAPCONTAINERTEST_WORLD_LIGHTMAP_CASES_H
#define UNITTESTS_MAPCONTAINERTEST_WORLD_LIGHTMAP_CASES_H

#include "mapcontainer/world_lightmap.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace lmap_cases
{

inline void PutU32( std::vector<char> &bytes, size_t offset, uint32_t value )
{
	for ( int i = 0; i < 4; ++i )
		bytes[offset + i] = char( ( value >> ( 8 * i ) ) & 0xFF );
}

inline void PutU64( std::vector<char> &bytes, size_t offset, uint64_t value )
{
	PutU32( bytes, offset, uint32_t( value ) );
	PutU32( bytes, offset + 4, uint32_t( value >> 32 ) );
}

// One 4x4 BC6H block of irradiance (0.5, 0.25, 1) and one BC7 block of
// gradient bytes (255, 128, 0, 204), from the pinned ktx encoder.
static const unsigned char kIrradianceBlock[16] = { 0xaf, 0xf3, 0xad, 0xbf, 0x07, 0x07, 0x2c,
    0xf0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const unsigned char kGradientBlock[16] = { 0x20, 0xff, 0x7f, 0xe8, 0x0f, 0x00, 0x30, 0x33,
    0xaf, 0xaa, 0xaa, 0xaa, 0, 0, 0, 0 };

// An LMAP v3 lump: `layers` layers of width x height pages, every block the
// constant blocks above.
inline std::vector<char> MakeLmap( uint32_t width, uint32_t height, uint32_t layers,
    bool sun = false )
{
	const uint64_t blocks = uint64_t( ( width + 3 ) / 4 ) * ( ( height + 3 ) / 4 );
	const uint64_t pageBytes = blocks * 16;
	std::vector<char> bytes( size_t( 64 + layers * 2 * pageBytes ), 0 );
	const uint32_t header[] = { mapcontainer::kWorldLightmapMagic, 3, width, height, layers,
	    sun ? 1u : 0u, 143, 145 };
	for ( size_t i = 0; i < sizeof( header ) / sizeof( header[0] ); ++i )
		PutU32( bytes, 4 * i, header[i] );
	PutU64( bytes, 32, pageBytes );
	PutU64( bytes, 40, pageBytes );
	PutU64( bytes, 48, 64 );
	for ( uint32_t layer = 0; layer < layers; ++layer )
		for ( uint64_t b = 0; b < blocks; ++b )
		{
			const size_t base = size_t( 64 + layer * 2 * pageBytes + 16 * b );
			std::memcpy( bytes.data() + base, kIrradianceBlock, 16 );
			std::memcpy( bytes.data() + base + pageBytes, kGradientBlock, 16 );
		}
	return bytes;
}

using Validator = mapcontainer::WorldLightmapError ( * )(
    const void *, size_t, uint32_t, mapcontainer::WorldLightmapBlocks * );

struct CaseResult
{
	int cases = 0;
	int wrong = 0;
};

// Every case: bytes, the lump version the caller expects, and the one error
// a correct validator reports (Ok cases also check the blocks it returns).
inline CaseResult RunLmapCases( Validator validate, bool verbose )
{
	using mapcontainer::WorldLightmapBlocks;
	using mapcontainer::WorldLightmapError;
	using mapcontainer::WorldLightmapLayer;
	CaseResult result;
	const auto expect = [&]( const char *name, const std::vector<char> &bytes, uint32_t version,
	                        WorldLightmapError wanted, uint32_t layers = 0,
	                        const WorldLightmapLayer *roles = nullptr )
	{
		++result.cases;
		WorldLightmapBlocks blocks{};
		const WorldLightmapError got = validate( bytes.data(), bytes.size(), version, &blocks );
		bool ok = got == wanted;
		if ( ok && wanted == WorldLightmapError::Ok )
		{
			ok = blocks.layerCount == layers;
			for ( uint32_t i = 0; ok && i < layers; ++i )
				ok = blocks.roles[i] == roles[i] &&
				     blocks.gradientOffset[i] == blocks.irradianceOffset[i] +
				                                     blocks.irradianceBytes &&
				     blocks.gradientOffset[i] + blocks.gradientBytes <= bytes.size();
		}
		if ( !ok )
		{
			++result.wrong;
			if ( verbose )
				std::printf( "FAIL lmap case %s: got %s, expected %s\n", name,
				    mapcontainer::WorldLightmapErrorName( got ),
				    mapcontainer::WorldLightmapErrorName( wanted ) );
		}
	};
	const WorldLightmapLayer total[] = { WorldLightmapLayer::Total };
	const WorldLightmapLayer two[] = { WorldLightmapLayer::Total, WorldLightmapLayer::Indirect };
	const WorldLightmapLayer three[] = {
	    WorldLightmapLayer::Total, WorldLightmapLayer::Direct, WorldLightmapLayer::Indirect };
	const std::vector<char> layered = MakeLmap( 8, 6, 2 );
	expect( "one layer", MakeLmap( 8, 6, 1 ), 3, WorldLightmapError::Ok, 1, total );
	expect( "total+indirect", layered, 3, WorldLightmapError::Ok, 2, two );
	expect( "tool-derived version", layered, 0, WorldLightmapError::Ok, 2, two );
	expect( "total+direct+indirect", MakeLmap( 5, 3, 3, true ), 3, WorldLightmapError::Ok, 3,
	    three );
	expect( "v2 lump version", layered, 2, WorldLightmapError::UnsupportedVersion );
	expect( "zero layers", MakeLmap( 8, 6, 0 ), 3, WorldLightmapError::InvalidLayerCount );
	expect( "four layers", MakeLmap( 8, 6, 4 ), 3, WorldLightmapError::InvalidLayerCount );
	const auto mutate = [&]( size_t offset, uint64_t value, bool wide = false )
	{
		std::vector<char> bytes = layered;
		if ( wide )
			PutU64( bytes, offset, value );
		else
			PutU32( bytes, offset, uint32_t( value ) );
		return bytes;
	};
	expect( "bad magic", mutate( 0, 0x32504D4C ), 3, WorldLightmapError::BadIdentifier );
	expect( "header version 2", mutate( 4, 2 ), 3, WorldLightmapError::UnsupportedVersion );
	expect( "unknown flag", mutate( 20, 2 ), 3, WorldLightmapError::UnsupportedVersion );
	expect( "reserved word", mutate( 56, 1, true ), 3, WorldLightmapError::UnsupportedVersion );
	expect( "RGBA16F irradiance", mutate( 24, 97 ), 3, WorldLightmapError::UnsupportedFormat );
	expect( "BC6H gradient", mutate( 28, 143 ), 3, WorldLightmapError::UnsupportedFormat );
	expect( "zero width", mutate( 8, 0 ), 3, WorldLightmapError::InvalidDescriptor );
	expect( "huge height", mutate( 12, 1u << 20 ), 3, WorldLightmapError::InvalidDescriptor );
	expect( "irradiance size", mutate( 32, 16, true ), 3, WorldLightmapError::InvalidDescriptor );
	expect( "gradient size", mutate( 40, 16, true ), 3, WorldLightmapError::InvalidDescriptor );
	expect( "data offset", mutate( 48, 80, true ), 3, WorldLightmapError::InvalidDescriptor );
	expect( "width without its blocks", mutate( 8, 12 ), 3, WorldLightmapError::InvalidDescriptor );
	std::vector<char> truncated( layered.begin(), layered.end() - 16 );
	expect( "truncated blocks", truncated, 3, WorldLightmapError::InvalidLevelIndex );
	std::vector<char> trailing = layered;
	trailing.resize( trailing.size() + 16 );
	expect( "trailing bytes", trailing, 3, WorldLightmapError::InvalidLevelIndex );
	std::vector<char> header( layered.begin(), layered.begin() + 40 );
	expect( "truncated header", header, 3, WorldLightmapError::Truncated );
	return result;
}

} // namespace lmap_cases

#endif // UNITTESTS_MAPCONTAINERTEST_WORLD_LIGHTMAP_CASES_H
