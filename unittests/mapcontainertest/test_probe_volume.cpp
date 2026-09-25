//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance for PRBV probe volumes (RFC 0011 G1, Q-CONTENT).
//
//  - The checked-in fixtures (tools/quality/probe_volume.py fixture) validate,
//    and the C++ reference sampler reproduces every sample the independent
//    Python sampler recorded (samples.txt), with and without visibility.
//  - Leak control: behind the fixture's wall the visibility test keeps the
//    lit probes out; without it light leaks.
//  - Inactive and relocated probes, the indirect layer and ambient cubes.
//  - Each malformation fails with its structured error; seeded mutation
//    fuzzing never crashes the validator.
//
//=============================================================================//

#include "mapcontainer/probe_volume.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
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

const char *kFixtures = "quality/fixtures/gi/prbv/";

std::vector<char> Load( const std::string &name )
{
	std::ifstream file( kFixtures + name, std::ios::binary );
	return std::vector<char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

void PutU32( std::vector<char> &bytes, size_t offset, uint32_t value )
{
	for ( int i = 0; i < 4; ++i )
		bytes[offset + i] = char( ( value >> ( 8 * i ) ) & 0xFF );
}

void PutF32( std::vector<char> &bytes, size_t offset, float value )
{
	uint32_t bits;
	std::memcpy( &bits, &value, sizeof( bits ) );
	PutU32( bytes, offset, bits );
}

void PutHalf( std::vector<char> &bytes, size_t offset, uint16_t half )
{
	bytes[offset] = char( half & 0xFF );
	bytes[offset + 1] = char( half >> 8 );
}

size_t TexelOffset( const ProbeVolumeLayout &layout, uint32_t x, uint32_t y, int channel )
{
	return size_t(
	    layout.atlasOffset + ( uint64_t( y ) * layout.atlasWidth + x ) * 8 + 2 * channel );
}

void SamplesMatchPython( const std::map<std::string, std::vector<char>> &files )
{
	std::ifstream text( std::string( kFixtures ) + "samples.txt" );
	std::string line;
	int compared = 0;
	while ( std::getline( text, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream in( line );
		std::string name, first;
		float position[3], normal[3];
		int layer = 0, visibility = 0;
		in >> name >> position[0] >> position[1] >> position[2] >> normal[0] >> normal[1] >>
		    normal[2] >> layer >> visibility >> first;
		const auto found = files.find( name );
		if ( found == files.end() )
		{
			Check( false, "sample names a known fixture: " + name );
			continue;
		}
		ProbeVolumeLayout layout{};
		ValidateProbeVolume( found->second.data(), found->second.size(), &layout );
		const ProbeVolumeView view( found->second.data(), layout );
		float value[3];
		const bool inside =
		    view.Sample( position, normal, ProbeVolumeLayer( layer ), visibility != 0, value );
		if ( first == "none" )
		{
			Check( !inside, "outside sample stays outside: " + line );
			continue;
		}
		float expected[3];
		expected[0] = std::strtof( first.c_str(), nullptr );
		in >> expected[1] >> expected[2];
		bool close = inside;
		for ( int c = 0; c < 3 && close; ++c )
			close =
			    std::fabs( value[c] - expected[c] ) <= 1.0e-4f + 1.0e-3f * std::fabs( expected[c] );
		if ( !close )
			std::printf(
			    "  C++ %.6f %.6f %.6f vs Python %s\n", value[0], value[1], value[2], line.c_str() );
		Check( close, "C++ sampler matches the Python sampler: " + line );
		++compared;
	}
	Check( compared >= 20, "the Python sample table was read" );
}

} // namespace

int main()
{
	std::map<std::string, std::vector<char>> files = { { "leak.prbv", Load( "leak.prbv" ) },
	    { "analytic.prbv", Load( "analytic.prbv" ) }, { "gpu.prbv", Load( "gpu.prbv" ) } };
	for ( const auto &entry : files )
	{
		ProbeVolumeLayout layout{};
		const ProbeVolumeError error =
		    ValidateProbeVolume( entry.second.data(), entry.second.size(), &layout );
		Check( !entry.second.empty() && error == ProbeVolumeError::Ok,
		    entry.first + " validates (" + ProbeVolumeErrorName( error ) + ")" );
	}
	ProbeVolumeLayout analytic{};
	ValidateProbeVolume( files["analytic.prbv"].data(), files["analytic.prbv"].size(), &analytic );
	Check( analytic.layerCount == 2 && analytic.gridCount == 1 &&
	           analytic.grids[0].probeCount == 27 && analytic.activeProbes == 26,
	    "analytic fixture: two layers, 27 probes, one inactive" );
	SamplesMatchPython( files );
	{
		// The GPU grid table mirrors the validated record (probe_volume.glsl).
		float table[kProbeGridTableFloats] = {};
		WriteProbeGridTable( analytic, table );
		const ProbeGridLayout &grid = analytic.grids[0];
		Check( table[0] == grid.origin[0] && table[2] == grid.origin[2] &&
		           table[3] == float( grid.tilesPerRow ) && table[6] == grid.spacing[2] &&
		           table[7] == grid.maxDistance && table[10] == float( grid.dims[2] ) &&
		           table[11] == 48.0f,
		    "grid table: origin, tiles per row, spacing, max distance, dims, smallest spacing" );
		Check( table[12] == float( grid.irradianceOrigin[0][0] ) &&
		           table[15] == float( grid.irradianceOrigin[1][1] ) &&
		           table[17] == float( grid.visibilityOrigin[1] ) &&
		           table[19] == float( grid.stateOrigin[1] ) && table[20] == 2.0f &&
		           table[21] == 1.0f && table[22] == 0.0f && table[23] == 0.0f,
		    "grid table: section origins, layer and grid counts" );
	}

	// Leak control on the fixture wall (x = 48): a point just behind it.
	{
		ProbeVolumeLayout layout{};
		const std::vector<char> &bytes = files["leak.prbv"];
		ValidateProbeVolume( bytes.data(), bytes.size(), &layout );
		const ProbeVolumeView view( bytes.data(), layout );
		const float dark[3] = { 56, 16, 16 };
		const float facing[3] = { 1, 0, 0 };
		float with[3], without[3];
		Check( view.Sample( dark, facing, ProbeVolumeLayer::Total, true, with ) &&
		           view.Sample( dark, facing, ProbeVolumeLayer::Total, false, without ),
		    "leak fixture samples" );
		Check( with[0] < 0.01f, "visibility keeps the lit probes out behind the wall" );
		Check( without[0] > with[0] + 0.03f, "without visibility, light leaks through the wall" );
		float cube[6][3];
		Check( view.AmbientCube( dark, ProbeVolumeLayer::Total, true, cube ),
		    "ambient cube evaluates at the dark point" );
		Check( !view.Sample( dark, facing, ProbeVolumeLayer::Indirect, true, with ),
		    "a missing layer is not sampled" );
		const float outside[3] = { -40, 0, 0 };
		Check( !view.Sample( outside, facing, ProbeVolumeLayer::Total, true, with ),
		    "a point outside the grid is not sampled" );
	}

	// Malformations, each with its structured error.
	const std::vector<char> &good = files["analytic.prbv"];
	const auto expect =
	    [&]( const char *name, const std::vector<char> &bytes, ProbeVolumeError wanted )
	{
		const ProbeVolumeError got = ValidateProbeVolume( bytes.data(), bytes.size() );
		Check( got == wanted, std::string( name ) + ": got " + ProbeVolumeErrorName( got ) +
		                          ", expected " + ProbeVolumeErrorName( wanted ) );
	};
	const auto mutated = [&]( size_t offset, uint32_t value )
	{
		std::vector<char> bytes = good;
		PutU32( bytes, offset, value );
		return bytes;
	};
	const size_t grid = kProbeVolumeHeaderBytes;
	expect( "truncated header", std::vector<char>( good.begin(), good.begin() + 40 ),
	    ProbeVolumeError::Truncated );
	expect( "truncated atlas", std::vector<char>( good.begin(), good.end() - 8 ),
	    ProbeVolumeError::InvalidAtlas );
	expect( "bad magic", mutated( 0, 0x12345678 ), ProbeVolumeError::BadMagic );
	expect( "version 2", mutated( 4, 2 ), ProbeVolumeError::UnsupportedVersion );
	expect( "flags", mutated( 36, 1 ), ProbeVolumeError::UnsupportedVersion );
	expect( "no grids", mutated( 12, 0 ), ProbeVolumeError::InvalidCounts );
	expect( "three layers", mutated( 16, 3 ), ProbeVolumeError::InvalidCounts );
	expect( "irradiance tile 6", mutated( 20, 6 ), ProbeVolumeError::InvalidCounts );
	expect( "atlas bytes", mutated( 48, 64 ), ProbeVolumeError::InvalidAtlas );
	expect( "misaligned atlas", mutated( 40, 1000 ), ProbeVolumeError::InvalidAtlas );
	expect( "dims 1", mutated( grid + 24, 1 ), ProbeVolumeError::InvalidGrid );
	expect( "huge dims", mutated( grid + 24, 1u << 30 ), ProbeVolumeError::InvalidGrid );
	{
		std::vector<char> bytes = good;
		PutF32( bytes, grid + 12, -64.0f );
		expect( "negative spacing", bytes, ProbeVolumeError::InvalidGrid );
		bytes = good;
		PutF32( bytes, grid + 0, std::nanf( "" ) );
		expect( "NaN origin", bytes, ProbeVolumeError::InvalidGrid );
		bytes = good;
		PutF32( bytes, grid + 40, 40.0f ); // > half the 48-unit spacing
		expect( "relocation bound past half a cell", bytes, ProbeVolumeError::InvalidGrid );
		bytes = good;
		bytes[grid + 90] = 1;
		expect( "reserved grid bytes", bytes, ProbeVolumeError::InvalidGrid );
	}
	expect( "tiles per row 0", mutated( grid + 36, 0 ), ProbeVolumeError::InvalidGrid );
	expect( "visibility section outside the atlas", mutated( grid + 68, 100000 ),
	    ProbeVolumeError::SectionOutsideAtlas );
	{
		ProbeVolumeLayout layout{};
		ValidateProbeVolume( good.data(), good.size(), &layout );
		const ProbeGridLayout &g = layout.grids[0];
		std::vector<char> bytes = good;
		PutHalf( bytes, TexelOffset( layout, g.stateOrigin[0], g.stateOrigin[1], 3 ), 0x3800 );
		expect( "active flag 0.5", bytes, ProbeVolumeError::InvalidState );
		bytes = good;
		PutHalf( bytes, TexelOffset( layout, g.stateOrigin[0] + 1, g.stateOrigin[1], 0 ),
		    0x5400 ); // 64 units > the 16-unit bound
		expect( "relocation past its bound", bytes, ProbeVolumeError::RelocationOutOfBounds );
		bytes = good;
		PutHalf( bytes,
		    TexelOffset( layout, g.irradianceOrigin[1][0] + 3, g.irradianceOrigin[1][1] + 3, 1 ),
		    0x7E00 ); // NaN in the indirect layer
		expect( "NaN irradiance", bytes, ProbeVolumeError::InvalidIrradiance );
		bytes = good;
		PutHalf( bytes,
		    TexelOffset( layout, g.visibilityOrigin[0] + 5, g.visibilityOrigin[1] + 5, 0 ),
		    0xBC00 ); // -1
		expect( "negative distance moment", bytes, ProbeVolumeError::InvalidVisibility );
	}

	// Seeded mutation fuzzing: any accepted mutation stays samplable.
	const char *seedText = std::getenv( "CONFORMANCE_SEED" );
	std::mt19937 random( seedText ? unsigned( std::strtoul( seedText, nullptr, 10 ) ) : 20260924u );
	int accepted = 0;
	for ( int trial = 0; trial < 3000; ++trial )
	{
		std::vector<char> bytes = trial % 2 ? good : files["leak.prbv"];
		const int flips = 1 + int( random() % 3 );
		for ( int f = 0; f < flips; ++f )
		{
			const size_t offset = trial % 3 ? random() % 160 : random() % bytes.size();
			bytes[offset] = char( random() );
		}
		ProbeVolumeLayout layout{};
		if ( ValidateProbeVolume( bytes.data(), bytes.size(), &layout ) == ProbeVolumeError::Ok )
		{
			++accepted;
			const ProbeVolumeView view( bytes.data(), layout );
			const float point[3] = { 0, 0, 40 };
			const float up[3] = { 0, 0, 1 };
			float value[3] = { 0, 0, 0 };
			view.Sample( point, up, ProbeVolumeLayer::Total, true, value );
			if ( !std::isfinite( value[0] ) )
			{
				Check( false, "a validated mutation samples a finite value" );
				break;
			}
		}
	}
	std::printf( "fuzz: %d of 3000 mutations accepted\n", accepted );
	Check( true, "fuzzing completed without a crash" );
	return testing::ReportConformance( g_checks, g_failures );
}
