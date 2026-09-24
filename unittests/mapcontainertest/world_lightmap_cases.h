//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared LMAP (world lightmap) fixtures and the case table every
//          LMAP validator is judged by (RFC 0008 F4 / RFC 0011 layers).
//
//          MakeLmap writes a minimal uncompressed RGBA16F KTX2 page; RunLmap
//          Cases applies valid, malformed and version cases to a validator
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

// A KTX2 LMAP page with `layers` array layers (0: a version 1 single page).
// `texels`, when given, holds each layer's width * height * 4 halves.
inline std::vector<char> MakeLmap( uint32_t width, uint32_t height, uint32_t layers,
    const std::vector<std::vector<uint16_t>> &texels = {} )
{
	const uint32_t dfdOffset = 80 + 24;
	const uint32_t dfdLength = 4 + 24 + 16 * 4;
	const uint32_t dataOffset = ( dfdOffset + dfdLength + 7 ) & ~7u;
	const uint64_t layerBytes = uint64_t( width ) * height * 8;
	const uint32_t count = layers ? layers : 1;
	std::vector<char> bytes( size_t( dataOffset + layerBytes * count ), 0 );
	const unsigned char identifier[12] = {
	    0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	std::memcpy( bytes.data(), identifier, sizeof( identifier ) );
	const uint32_t header[] = { 97, 2, width, height, 0, layers, 1, 1, 0, dfdOffset, dfdLength,
	    0, 0 };
	for ( size_t i = 0; i < sizeof( header ) / sizeof( header[0] ); ++i )
		PutU32( bytes, 12 + 4 * i, header[i] );
	PutU64( bytes, 80, dataOffset );
	PutU64( bytes, 88, layerBytes * count );
	PutU64( bytes, 96, layerBytes * count );
	PutU32( bytes, dfdOffset, dfdLength );
	for ( uint32_t layer = 0; layer < count && layer < texels.size(); ++layer )
		std::memcpy( bytes.data() + dataOffset + layer * layerBytes, texels[layer].data(),
		    size_t( layerBytes ) );
	return bytes;
}

using Validator = mapcontainer::WorldLightmapError ( * )( const void *, size_t, uint32_t,
    mapcontainer::WorldLightmapLayout * );

struct CaseResult
{
	int cases = 0;
	int wrong = 0;
};

// Every case: bytes, the lump version the caller expects, and the one error
// a correct validator reports (Ok cases also check the layout it returns).
inline CaseResult RunLmapCases( Validator validate, bool verbose )
{
	using mapcontainer::WorldLightmapError;
	using mapcontainer::WorldLightmapLayer;
	using mapcontainer::WorldLightmapLayout;
	CaseResult result;
	const auto expect = [&]( const char *name, const std::vector<char> &bytes, uint32_t version,
	                        WorldLightmapError wanted, uint32_t layers = 0,
	                        const WorldLightmapLayer *roles = nullptr )
	{
		++result.cases;
		WorldLightmapLayout layout{};
		const WorldLightmapError got = validate( bytes.data(), bytes.size(), version, &layout );
		bool ok = got == wanted;
		if ( ok && wanted == WorldLightmapError::Ok )
		{
			ok = layout.layerCount == layers && layout.layerBytes == uint64_t( layout.width ) *
			                                                           layout.height * 8;
			for ( uint32_t i = 0; ok && i < layers; ++i )
				ok = layout.roles[i] == roles[i] &&
				     layout.layerOffset[i] + layout.layerBytes <= bytes.size();
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
	const WorldLightmapLayer three[] = { WorldLightmapLayer::Total, WorldLightmapLayer::Direct,
	    WorldLightmapLayer::Indirect };
	const std::vector<char> single = MakeLmap( 4, 2, 0 );
	const std::vector<char> layered = MakeLmap( 4, 2, 2 );
	expect( "v1 page", single, 1, WorldLightmapError::Ok, 1, total );
	expect( "v1 page, tool-derived version", single, 0, WorldLightmapError::Ok, 1, total );
	expect( "v2 total+indirect", layered, 2, WorldLightmapError::Ok, 2, two );
	expect( "v2 total+direct+indirect", MakeLmap( 4, 2, 3 ), 2, WorldLightmapError::Ok, 3, three );
	expect( "v1 bytes as a v2 lump", single, 2, WorldLightmapError::VersionMismatch );
	expect( "v2 bytes as a v1 lump", layered, 1, WorldLightmapError::VersionMismatch );
	expect( "unknown lump version", single, 3, WorldLightmapError::UnsupportedVersion );
	expect( "one array layer", MakeLmap( 4, 2, 1 ), 0, WorldLightmapError::InvalidLayerCount );
	expect( "four array layers", MakeLmap( 4, 2, 4 ), 0, WorldLightmapError::InvalidLayerCount );
	const auto mutate = [&]( size_t offset, uint32_t value, bool wide = false )
	{
		std::vector<char> bytes = layered;
		if ( wide )
			PutU64( bytes, offset, value );
		else
			PutU32( bytes, offset, value );
		return bytes;
	};
	std::vector<char> identifier = layered;
	identifier[1] = 'X';
	expect( "bad identifier", identifier, 2, WorldLightmapError::BadIdentifier );
	expect( "RGBA8 format", mutate( 12, 37 ), 2, WorldLightmapError::UnsupportedFormat );
	expect( "type size 1", mutate( 16, 1 ), 2, WorldLightmapError::UnsupportedFormat );
	expect( "zero width", mutate( 20, 0 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "3D depth", mutate( 28, 2 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "cube faces", mutate( 36, 6 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "no levels", mutate( 40, 0 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "two levels", mutate( 40, 2 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "supercompressed", mutate( 44, 1 ), 2, WorldLightmapError::UnsupportedTopology );
	expect( "descriptor length word", mutate( 80 + 24, 8 ), 2,
	    WorldLightmapError::InvalidDescriptor );
	expect( "descriptor past the end", mutate( 48, 1u << 30 ), 2,
	    WorldLightmapError::InvalidDescriptor );
	expect( "supercompression global data", mutate( 72, 16, true ), 2,
	    WorldLightmapError::InvalidDescriptor );
	expect( "misaligned level", mutate( 80, 180, true ), 2, WorldLightmapError::InvalidLevelIndex );
	expect( "short level", mutate( 88, 64, true ), 2, WorldLightmapError::InvalidLevelIndex );
	expect( "compressed level length", mutate( 96, 64, true ), 2,
	    WorldLightmapError::InvalidLevelIndex );
	std::vector<char> truncated( layered.begin(), layered.end() - 8 );
	expect( "truncated texels", truncated, 2, WorldLightmapError::InvalidLevelIndex );
	std::vector<char> header( layered.begin(), layered.begin() + 90 );
	expect( "truncated header", header, 2, WorldLightmapError::Truncated );
	return result;
}

} // namespace lmap_cases

#endif // UNITTESTS_MAPCONTAINERTEST_WORLD_LIGHTMAP_CASES_H
