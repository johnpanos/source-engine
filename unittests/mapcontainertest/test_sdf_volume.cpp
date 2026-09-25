//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for SDFV signed distance volumes (RFC 0011 G6,
//          Q-CONTENT): the C++ reader against the independent Python reader
//          (tools/quality/sdf_volume.py).
//
//  - The checked-in contract volume validates and its layout is what the
//    fixture record declares.
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
	Check( layout.voxelOffset == kSdfVolumeHeaderBytes &&
	           layout.lightOffset == kSdfVolumeHeaderBytes + 21ull * 13 * 13 * kSdfVoxelBytes &&
	           layout.lightOffset + kSdfLightBytes == sdfv.size(),
	    "the contract's sections tile its bytes" );
	SdfLight light;
	std::memcpy( &light, sdfv.data() + layout.lightOffset, sizeof( light ) );
	Check( light.kind == uint32_t( SdfLightKind::Dome ) && light.style == -1,
	    "the contract's light is a fixed dome" );
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
	Check( count >= 13,
	    "the malformation corpus has its 13 cases (read " + std::to_string( count ) + ")" );
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
		if ( layout.voxelOffset + voxels * kSdfVoxelBytes <= mutant.size() &&
		     layout.lightOffset + uint64_t( layout.lightCount ) * kSdfLightBytes == mutant.size() )
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
	RejectsTheCorpus( sdfv );
	SurvivesFuzzing( sdfv );
	return testing::ReportConformance( g_checks, g_failures );
}
