//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for RTRN radiosity transfers (RFC 0011 G4, Q-CONTENT):
//          the C++ reader against the independent Python reader
//          (tools/quality/radiosity_transfer.py).
//
//  - The checked-in contract transfer validates, pairs with its PRBV, and
//    the view reads every section as the fixture declares it.
//  - Every malformation in the shared corpus (malformations.txt, the Python
//    reader's rejections and their codes) is rejected with the same code.
//  - A transfer is rejected against a volume it was not baked for.
//  - Seeded mutation fuzzing never crashes the validator, and a mutant it
//    accepts reads only in-range links.
//
//=============================================================================//

#include "mapcontainer/radiosity_transfer.h"
#include "testing/conformance_result.h"

#include <cmath>
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
RadiosityTransferError Validate( const std::vector<unsigned char> &bytes,
    RadiosityTransferLayout *layout, const std::vector<unsigned char> *prbv = nullptr,
    const ProbeVolumeLayout *prbvLayout = nullptr )
{
	std::vector<uint64_t> aligned( ( bytes.size() + 7 ) / 8 + 1 );
	std::memcpy( aligned.data(), bytes.data(), bytes.size() );
	return ValidateRadiosityTransfer(
	    aligned.data(), bytes.size(), layout, prbv ? prbv->data() : nullptr, prbvLayout );
}

void ReadsTheContract( const std::vector<unsigned char> &rtrn,
    const std::vector<unsigned char> &prbv, const ProbeVolumeLayout &prbvLayout )
{
	RadiosityTransferLayout layout = {};
	const RadiosityTransferError error = Validate( rtrn, &layout, &prbv, &prbvLayout );
	Check( error == RadiosityTransferError::Ok,
	    std::string( "the contract transfer validates with its volume: " ) +
	        RadiosityTransferErrorName( error ) );
	if ( error != RadiosityTransferError::Ok )
		return;
	Check( layout.sourceCount == 2 && layout.patchCount == 24 && layout.probeCount == 16,
	    "the contract's counts: 2 sources, 24 patches, 16 probes" );
	Check( layout.transferLinks == 24 * 24 && layout.injectionLinks == 24 + 4,
	    "the contract's link counts" );
	std::vector<uint64_t> aligned( ( rtrn.size() + 7 ) / 8 );
	std::memcpy( aligned.data(), rtrn.data(), rtrn.size() );
	const RadiosityTransferView view( aligned.data(), layout );
	Check( std::strcmp( view.Source( 0 ).name, "Emitters" ) == 0 && view.Source( 0 ).style == 32 &&
	           view.Source( 0 ).kind == uint32_t( RadiositySourceKind::Emissive ) &&
	           std::strcmp( view.Source( 1 ).name, "Lamp" ) == 0 && view.Source( 1 ).style == 33,
	    "the sources read back with their names, kinds and styles" );
	double rowSum = 0.0;
	for ( auto *l = view.TransferBegin( 5 ); l != view.TransferEnd( 5 ); ++l )
		rowSum += l->factor;
	Check( std::fabs( rowSum - 1.0 ) < 1e-5, "a closed enclosure's transfer row sums to one" );
	Check( std::fabs( view.Patch( 7 ).albedo[1] - 0.6f ) < 1e-6f && view.Patch( 7 ).area == 256.0f,
	    "patch records read back" );
	Check( view.InjectionEnd( 1 ) - view.InjectionBegin( 1 ) == 4 &&
	           std::fabs( view.InjectionBegin( 1 )->light[0] - 0.6f ) < 1e-6f,
	    "the lamp's injection reaches four patches" );
	int litProbes = 0;
	for ( uint32_t i = 0; i < layout.probeCount; ++i )
		litProbes += view.GatherEnd( i ) != view.GatherBegin( i );
	Check( litProbes == 8, "eight probes (the lit side) gather patches" );
	Check( std::fabs( view.ProbeDirect( 0, 0 )[0] - 0.2f ) < 1e-6f &&
	           std::fabs( view.ProbeDirect( 1, 0 )[35 * 3 + 2] - 0.1f ) < 1e-6f,
	    "probe direct light reads back per source and texel" );

	// Paired only with its own volume: a changed probe state is a mismatch.
	std::vector<unsigned char> moved = prbv;
	const ProbeGridLayout &grid = prbvLayout.grids[0];
	const size_t state = size_t(
	    prbvLayout.atlasOffset + ( uint64_t( grid.stateOrigin[1] ) * prbvLayout.atlasWidth +
	                                 grid.stateOrigin[0] ) *
	                                 8 );
	moved[state] ^= 0x01; // relocate probe 0 by a half-float ulp
	ProbeVolumeLayout movedLayout = {};
	Check( ValidateProbeVolume( moved.data(), moved.size(), &movedLayout ) == ProbeVolumeError::Ok,
	    "the relocated volume is itself valid" );
	Check( Validate( rtrn, nullptr, &moved, &movedLayout ) == RadiosityTransferError::TopologyMismatch,
	    "a transfer is rejected against a volume it was not baked for" );
}

void RejectsTheCorpus( const std::vector<unsigned char> &rtrn )
{
	std::ifstream corpus( "quality/fixtures/gi/rtrn/malformations.txt" );
	std::string line;
	int count = 0;
	while ( std::getline( corpus, line ) )
	{
		std::istringstream fields( line );
		std::string name, error;
		size_t length = 0;
		fields >> name >> error >> length;
		std::vector<unsigned char> variant = rtrn;
		variant.resize( length, 0 );
		std::string edit;
		while ( fields >> edit )
		{
			const size_t colon = edit.find( ':' );
			size_t offset = std::strtoull( edit.substr( 0, colon ).c_str(), nullptr, 10 );
			const std::string hex = edit.substr( colon + 1 );
			for ( size_t k = 0; k + 1 < hex.size(); k += 2 )
				variant[offset++] =
				    (unsigned char)std::strtoul( hex.substr( k, 2 ).c_str(), nullptr, 16 );
		}
		const RadiosityTransferError got = Validate( variant, nullptr );
		Check( error == RadiosityTransferErrorName( got ),
		    "malformation " + name + ": Python rejects it as " + error + ", C++ as " +
		        RadiosityTransferErrorName( got ) );
		++count;
	}
	Check( count >= 25, "the shared malformation corpus was read" );
}

void FuzzNeverCrashes( const std::vector<unsigned char> &rtrn )
{
	std::mt19937 random( 20260925u );
	int accepted = 0, rejected = 0;
	bool inRange = true;
	for ( int iteration = 0; iteration < 20000; ++iteration )
	{
		std::vector<unsigned char> mutant = rtrn;
		const int flips = 1 + int( random() % 4 );
		for ( int k = 0; k < flips; ++k )
			mutant[random() % mutant.size()] = (unsigned char)( random() & 0xFF );
		if ( random() % 10 == 0 )
			mutant.resize( random() % mutant.size() );
		RadiosityTransferLayout layout = {};
		std::vector<uint64_t> aligned( ( mutant.size() + 7 ) / 8 + 1 );
		std::memcpy( aligned.data(), mutant.data(), mutant.size() );
		if ( ValidateRadiosityTransfer( aligned.data(), mutant.size(), &layout ) !=
		     RadiosityTransferError::Ok )
		{
			++rejected;
			continue;
		}
		++accepted;
		const RadiosityTransferView view( aligned.data(), layout );
		for ( uint32_t p = 0; p < layout.patchCount; ++p )
			for ( auto *l = view.TransferBegin( p ); l != view.TransferEnd( p ); ++l )
				inRange = inRange && l->patch < layout.patchCount;
		for ( uint32_t i = 0; i < layout.probeCount; ++i )
			for ( auto *l = view.GatherBegin( i ); l != view.GatherEnd( i ); ++l )
				inRange = inRange && l->patch < layout.patchCount;
	}
	std::printf( "fuzz: %d accepted, %d rejected\n", accepted, rejected );
	Check( rejected > 1000 && accepted > 100, "mutation fuzzing exercised both outcomes" );
	Check( inRange, "every accepted mutant's links are in range" );
}

} // namespace

int main()
{
	const std::vector<unsigned char> rtrn = Load( "quality/fixtures/gi/rtrn/contract.rtrn" );
	const std::vector<unsigned char> prbv = Load( "quality/fixtures/gi/prbv/contract.prbv" );
	ProbeVolumeLayout prbvLayout = {};
	Check( !rtrn.empty() && ValidateProbeVolume( prbv.data(), prbv.size(), &prbvLayout ) ==
	                            ProbeVolumeError::Ok,
	    "the contract transfer and volume fixtures load" );
	if ( rtrn.empty() )
		return testing::ReportConformance( g_checks, g_failures );
	ReadsTheContract( rtrn, prbv, prbvLayout );
	RejectsTheCorpus( rtrn );
	FuzzNeverCrashes( rtrn );
	return testing::ReportConformance( g_checks, g_failures );
}
