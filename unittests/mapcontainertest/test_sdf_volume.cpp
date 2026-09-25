//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for SDFV signed distance volumes (RFC 0011 G6,
//          Q-CONTENT): the C++ reader against the independent Python reader
//          (tools/quality/sdf_volume.py).
//
//  - The checked-in contract volume validates and its layout is what the
//    fixture record declares; so do the every-light-kind volume with its
//    light cells (version 2) and the previous contract (version 1, read
//    as one cell holding every light).
//  - Every malformation in the shared corpus (malformations.txt, the Python
//    reader's rejections and their codes) is rejected with the same code.
//  - Seeded mutation fuzzing never crashes the validator, and a mutant it
//    accepts has a layout inside its bytes.
//
//=============================================================================//

#include "mapcontainer/sdf_volume.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace mapcontainer;

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

std::vector<unsigned char> Load( const std::string &path )
{
	std::ifstream file( path, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

// Validates a copy in 16-byte aligned storage (map lumps are aligned).
SdfVolumeError Validate( const std::vector<unsigned char> &bytes, SdfVolumeLayout *layout )
{
	std::vector<uint64_t> aligned( ( bytes.size() + 7 ) / 8 + 1 );
	std::memcpy( aligned.data(), bytes.data(), bytes.size() );
	return ValidateSdfVolume( aligned.data(), bytes.size(), layout );
}

void ReadsTheContract( const std::vector<unsigned char> &sdfv )
{
	SdfVolumeLayout layout = {};
	const SdfVolumeError error = Validate( sdfv, &layout );
	Check( error == SdfVolumeError::Ok,
	    std::string( "the contract volume validates: " ) + SdfVolumeErrorName( error ) );
	if ( error != SdfVolumeError::Ok )
		return;
	// quality/fixtures/gi/sdfv/fixtures.json's contract record.
	Check( layout.dims[0] == 21 && layout.dims[1] == 13 && layout.dims[2] == 13,
	    "the contract's dims: 21 x 13 x 13" );
	Check( layout.voxel == 8.0f && layout.maxDistance == 64.0f,
	    "the contract's voxel (8) and clamp (64)" );
	Check( layout.origin[0] == -32.0f && layout.origin[1] == -32.0f && layout.origin[2] == -32.0f,
	    "the contract's origin" );
	Check( layout.lightCount == 1, "the contract's one light (a dome)" );
	Check( layout.version == 2 && layout.cellDims[0] == 1 && layout.cellDims[1] == 1 &&
	           layout.cellDims[2] == 1 && layout.cellEntries == 1,
	    "the contract is version 2 with one cell listing its light" );
	// Header, voxels, the light, two cell offsets, one entry and its padding.
	Check( layout.voxelOffset == kSdfVolumeHeaderBytes &&
	           layout.lightOffset == kSdfVolumeHeaderBytes + 21ull * 13 * 13 * kSdfVoxelBytes &&
	           layout.cellOffset == layout.lightOffset + kSdfLightBytes &&
	           layout.cellEntryOffset == layout.cellOffset + 8 &&
	           layout.cellEntryOffset + 4 == sdfv.size(),
	    "the contract's sections tile its bytes" );
	SdfLight light;
	std::memcpy( &light, sdfv.data() + layout.lightOffset, sizeof( light ) );
	Check( light.kind == uint32_t( SdfLightKind::Dome ) && light.style == -1,
	    "the contract's light is a fixed dome" );
}

uint32_t CellEntry( const std::vector<unsigned char> &bytes, const SdfVolumeLayout &layout,
    uint32_t cell, uint32_t k )
{
	uint32_t first;
	std::memcpy( &first, bytes.data() + layout.cellOffset + cell * 4, 4 );
	uint16_t light;
	std::memcpy( &light, bytes.data() + layout.cellEntryOffset + ( first + k ) * 2, 2 );
	return light;
}

void ReadsEveryLightKind( const std::vector<unsigned char> &sdfv )
{
	SdfVolumeLayout layout = {};
	const SdfVolumeError error = Validate( sdfv, &layout );
	Check( error == SdfVolumeError::Ok,
	    std::string( "the light-kinds volume validates: " ) + SdfVolumeErrorName( error ) );
	if ( error != SdfVolumeError::Ok )
		return;
	// quality/fixtures/gi/sdfv/fixtures.json's "lights" record.
	Check( layout.lightCount == 5, "five lights, one of each kind" );
	bool kinds = true;
	for ( uint32_t i = 0; i < 5 && layout.lightCount == 5; ++i )
	{
		SdfLight light;
		std::memcpy(
		    &light, sdfv.data() + layout.lightOffset + i * kSdfLightBytes, sizeof( light ) );
		kinds = kinds && light.kind == i;
		if ( light.kind == uint32_t( SdfLightKind::Spot ) )
			Check( light.reserved[0] == 2.0f && light.c[1] > light.c[2] && light.style == 33,
			    "the spot's exponent, cone and style" );
		if ( light.kind == uint32_t( SdfLightKind::Sphere ) )
			Check( light.b[0] == 2.0f && light.a[2] == 16.0f, "the sphere's radius and centre" );
	}
	Check( kinds, "light i has kind i (rect, distant, dome, sphere, spot)" );
	Check( layout.cellDims[0] == 2 && layout.cellDims[1] == 2 && layout.cellDims[2] == 1 &&
	           layout.cellSize == 40.0f && layout.cellEntries == 15 &&
	           layout.cellOrigin[0] == -8.0f,
	    "the 2 x 2 x 1 cells of 40 units, 15 entries" );
	Check( CellEntry( sdfv, layout, 1, 2 ) == 2 && CellEntry( sdfv, layout, 3, 3 ) == 4,
	    "cell 1 lists the dome third; cell 3 lists the spot last" );
}

void ReadsVersionOne( const std::vector<unsigned char> &sdfv )
{
	SdfVolumeLayout layout = {};
	const SdfVolumeError error = Validate( sdfv, &layout );
	Check( error == SdfVolumeError::Ok,
	    std::string( "the version-1 contract validates: " ) + SdfVolumeErrorName( error ) );
	Check( layout.version == 1 && layout.voxelOffset == kSdfVolumeV1HeaderBytes &&
	           layout.cellDims[0] == 0 && layout.cellEntries == 0 && layout.lightCount == 1 &&
	           layout.lightOffset + kSdfLightBytes == sdfv.size(),
	    "a version-1 volume: 64-byte header, no cells" );
}

void RejectsTheCorpus( const std::vector<unsigned char> &sdfv )
{
	std::ifstream corpus( "quality/fixtures/gi/sdfv/malformations.txt" );
	std::string line;
	int count = 0;
	while ( std::getline( corpus, line ) )
	{
		std::istringstream fields( line );
		std::string name, error;
		size_t length = 0;
		fields >> name >> error >> length;
		std::vector<unsigned char> variant = sdfv;
		variant.resize( length, 0 );
		std::string edit;
		while ( fields >> edit )
		{
			const size_t colon = edit.find( ':' );
			size_t offset = std::strtoull( edit.substr( 0, colon ).c_str(), nullptr, 10 );
			const std::string hex = edit.substr( colon + 1 );
			for ( size_t k = 0; k + 1 < hex.size() && offset < variant.size(); k += 2 )
				variant[offset++] =
				    (unsigned char)std::strtoul( hex.substr( k, 2 ).c_str(), nullptr, 16 );
		}
		const SdfVolumeError got = Validate( variant, nullptr );
		Check( error == SdfVolumeErrorName( got ),
		    "malformation " + name + ": expected " + error + ", got " + SdfVolumeErrorName( got ) );
		++count;
	}
	Check( count >= 20,
	    "the malformation corpus has its 20 cases (read " + std::to_string( count ) + ")" );
}

void SurvivesFuzzing( const std::vector<unsigned char> &sdfv )
{
	std::mt19937 rng( 20260925u );
	int accepted = 0, inRange = 0;
	constexpr int kMutants = 3000;
	for ( int i = 0; i < kMutants; ++i )
	{
		std::vector<unsigned char> mutant = sdfv;
		const int edits = 1 + int( rng() % 4 );
		for ( int e = 0; e < edits; ++e )
		{
			// Mostly the header and the lights, where the structure lives.
			const size_t region = rng() % 3;
			const size_t offset = region == 0   ? rng() % kSdfVolumeHeaderBytes
			                      : region == 1 ? mutant.size() - 1 - rng() % kSdfLightBytes
			                                    : rng() % mutant.size();
			mutant[offset] = (unsigned char)rng();
		}
		if ( rng() % 8 == 0 )
			mutant.resize( rng() % ( mutant.size() + 1 ) );
		SdfVolumeLayout layout = {};
		if ( Validate( mutant, &layout ) != SdfVolumeError::Ok )
			continue;
		++accepted;
		const uint64_t voxels = uint64_t( layout.dims[0] ) * layout.dims[1] * layout.dims[2];
		const uint64_t lightsEnd =
		    layout.lightOffset + uint64_t( layout.lightCount ) * kSdfLightBytes;
		const uint64_t end = layout.version == 1
		                         ? lightsEnd
		                         : layout.cellEntryOffset + uint64_t( layout.cellEntries ) * 2 +
		                               ( layout.cellEntries % 2 ) * 2;
		if ( layout.voxelOffset + voxels * kSdfVoxelBytes <= mutant.size() && lightsEnd <= end &&
		     end == mutant.size() )
			++inRange;
	}
	Check( accepted == inRange, "every accepted mutant's sections lie inside its bytes (" +
	                                std::to_string( accepted ) + " accepted of " +
	                                std::to_string( kMutants ) + ")" );
}

} // namespace

int main()
{
	const std::vector<unsigned char> sdfv = Load( "quality/fixtures/gi/sdfv/contract.sdfv" );
	Check( !sdfv.empty(), "the contract volume loads (run from the repository root)" );
	if ( sdfv.empty() )
		return testing::ReportConformance( g_checks, g_failures );
	ReadsTheContract( sdfv );
	ReadsEveryLightKind( Load( "quality/fixtures/gi/sdfv/lights.sdfv" ) );
	ReadsVersionOne( Load( "quality/fixtures/gi/sdfv/contract-v1.sdfv" ) );
	RejectsTheCorpus( sdfv );
	SurvivesFuzzing( sdfv );
	return testing::ReportConformance( g_checks, g_failures );
}
