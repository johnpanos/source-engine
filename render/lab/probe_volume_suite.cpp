//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite probe-volume (RFC 0016 K11 "Model assembly", the
//			indirect diffuse term of dynamic surfaces): the probe volume
//			(render/shaders/common/probe_volume.glsl) evaluated on the GPU by a
//			check kernel (probe_volume_check.comp) over the checked-in PRBV
//			fixtures (quality/fixtures/gi/prbv: analytic, contract, gpu and
//			leak, written by tools/quality/probe_volume.py), judged against its
//			oracle, mapcontainer::ProbeVolumeView::Sample, which the GLSL ports
//			line for line:
//			- the fixture's own sample table (samples.txt) and 384 seeded
//			  random cases per fixture (inside and outside the grids, both
//			  layers and one the volume lacks, with and without visibility):
//			  whether the point is covered, and the irradiance within
//			  2e-3 + 1e-3 relative (the native reflection-probe oracle's
//			  bound; the atlas is half float and the GPU filters with
//			  sub-texel precision);
//			- ProbeIrradiancePair's first result is ProbeIrradiance (within
//			  1e-5 relative: the two call sites may contract differently),
//			  and its second atlas (the fixture with its irradiance changed,
//			  states and moments kept) is the oracle over that atlas;
//			- a case whose oracle moves by more than half the bound, or
//			  changes coverage, within a thousandth of the volume's smallest
//			  spacing of its point is ill-conditioned and skipped, at most 5
//			  percent of a fixture's cases (the rule the area-light suite
//			  uses; first fixed as 0.05 units, which is 8 percent of the gpu
//			  fixture's spacing, and scaled the day it was written);
//			- outside every grid the result is zero;
//			- the leak fixture's visibility changes its samples (so the
//			  Chebyshev test is exercised).
//			Tolerances were fixed before the first run.
//
//			Seeded (--sensitivity): no normal bias, inactive probes used,
//			the visibility test ignored, no weight crush: each a check kernel
//			built from probe_volume.glsl.
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "mapcontainer/probe_volume.h"
#include "render/device/device.h"
#include "spv/probe_volume_check_spv.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace render::lab
{

namespace
{

using namespace render::device;
using mapcontainer::ProbeVolumeLayer;
using mapcontainer::ProbeVolumeLayout;
using mapcontainer::ProbeVolumeView;

constexpr const char *kFixtures = "quality/fixtures/gi/prbv/";
constexpr const char *kNames[] = { "analytic.prbv", "contract.prbv", "gpu.prbv", "leak.prbv" };
constexpr float kAbsolute = 2e-3f;
constexpr float kRelative = 1e-3f;
// Of the volume's smallest spacing (the fixtures span clip-space and Source
// units).
constexpr float kConditioning = 1e-3f;
constexpr float kIllConditionedShare = 0.05f;
constexpr int kRandomCases = 384;

struct GpuCase
{
	float position[4]; // w: the layer
	float normal[4];   // w: 1 with visibility
};

struct Case
{
	float position[3];
	float normal[3];
	int layer = 0;
	bool visibility = true;
};

std::vector<unsigned char> Load( const std::string &name )
{
	std::ifstream file( std::string( kFixtures ) + name, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

// The fixture with each irradiance texel changed (v * 0.5 + 0.1 * (c + 1)):
// the same states and moments, so the same weights, and other light.
std::vector<unsigned char> SecondAtlas(
    const std::vector<unsigned char> &bytes, const ProbeVolumeLayout &layout )
{
	std::vector<unsigned char> second = bytes;
	unsigned char *atlas = second.data() + layout.atlasOffset;
	for ( std::uint32_t g = 0; g < layout.gridCount; ++g )
	{
		const mapcontainer::ProbeGridLayout &grid = layout.grids[g];
		const std::uint32_t rows = ( grid.probeCount + grid.tilesPerRow - 1 ) / grid.tilesPerRow;
		for ( std::uint32_t l = 0; l < layout.layerCount; ++l )
		{
			for ( std::uint32_t y = 0; y < rows * mapcontainer::kProbeIrradianceTile; ++y )
			{
				for ( std::uint32_t x = 0;
				    x < grid.tilesPerRow * mapcontainer::kProbeIrradianceTile; ++x )
				{
					unsigned char *texel =
					    atlas +
					    ( std::uint64_t( grid.irradianceOrigin[l][1] + y ) * layout.atlasWidth +
					        grid.irradianceOrigin[l][0] + x ) *
					        8;
					for ( int c = 0; c < 3; ++c )
					{
						std::uint16_t half;
						std::memcpy( &half, texel + 2 * c, 2 );
						half = FloatToHalf( HalfToFloat( half ) * 0.5f + 0.1f * float( c + 1 ) );
						std::memcpy( texel + 2 * c, &half, 2 );
					}
				}
			}
		}
	}
	return second;
}

std::vector<Case> Cases(
    const std::string &name, const ProbeVolumeLayout &layout, std::mt19937 &random )
{
	std::vector<Case> cases;
	// The fixture's own table.
	std::ifstream text( std::string( kFixtures ) + "samples.txt" );
	std::string line;
	while ( std::getline( text, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream in( line );
		std::string fixture;
		Case c;
		int visibility = 0;
		in >> fixture >> c.position[0] >> c.position[1] >> c.position[2] >> c.normal[0] >>
		    c.normal[1] >> c.normal[2] >> c.layer >> visibility;
		c.visibility = visibility != 0;
		if ( fixture == name )
			cases.push_back( c );
	}
	// Random cases over the grids' bounds, grown by a tenth.
	float low[3] = { 1e30f, 1e30f, 1e30f }, high[3] = { -1e30f, -1e30f, -1e30f };
	for ( std::uint32_t g = 0; g < layout.gridCount; ++g )
	{
		for ( int i = 0; i < 3; ++i )
		{
			const float a = layout.grids[g].origin[i];
			const float b = a + float( layout.grids[g].dims[i] - 1 ) * layout.grids[g].spacing[i];
			low[i] = std::min( low[i], a );
			high[i] = std::max( high[i], b );
		}
	}
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	std::normal_distribution<float> gauss( 0.0f, 1.0f );
	for ( int k = 0; k < kRandomCases; ++k )
	{
		Case c;
		for ( int i = 0; i < 3; ++i )
		{
			const float margin = 0.1f * ( high[i] - low[i] );
			c.position[i] = low[i] - margin + unit( random ) * ( high[i] - low[i] + 2 * margin );
		}
		float length = 0.0f;
		do
		{
			for ( float &v : c.normal )
				v = gauss( random );
			length = std::sqrt(
			    c.normal[0] * c.normal[0] + c.normal[1] * c.normal[1] + c.normal[2] * c.normal[2] );
		} while ( length < 1e-3f );
		for ( float &v : c.normal )
			v /= length;
		// One case in ten asks for a layer the volume lacks.
		c.layer = unit( random ) < 0.1f ? int( layout.layerCount )
		                                : int( unit( random ) * float( layout.layerCount ) ) %
		                                      int( layout.layerCount );
		c.visibility = unit( random ) < 0.75f;
		cases.push_back( c );
	}
	return cases;
}

bool Near( const float *expected, const float *actual )
{
	for ( int c = 0; c < 3; ++c )
	{
		if ( !( std::fabs( expected[c] - actual[c] ) <=
		         kAbsolute + kRelative * std::fabs( expected[c] ) ) )
			return false;
	}
	return true;
}

// Whether the oracle is stable within `reach` of the case's point.
bool Conditioned( const ProbeVolumeView &view, const Case &c, float reach )
{
	float centre[3];
	const bool covered =
	    view.Sample( c.position, c.normal, ProbeVolumeLayer( c.layer ), c.visibility, centre );
	for ( int axis = 0; axis < 3; ++axis )
	{
		for ( float step : { -reach, reach } )
		{
			float moved[3] = { c.position[0], c.position[1], c.position[2] };
			moved[axis] += step;
			float value[3];
			const bool inside =
			    view.Sample( moved, c.normal, ProbeVolumeLayer( c.layer ), c.visibility, value );
			if ( inside != covered )
				return false;
			if ( !covered )
				continue;
			for ( int k = 0; k < 3; ++k )
			{
				if ( std::fabs( value[k] - centre[k] ) >
				     0.5f * ( kAbsolute + kRelative * std::fabs( centre[k] ) ) )
					return false;
			}
		}
	}
	return true;
}

std::string Describe( std::size_t i, const Case &c, const float *expected, bool expectedCovered,
    const float *actual, bool actualCovered )
{
	char text[256];
	std::snprintf( text, sizeof( text ),
	    "case %zu at (%.4g %.4g %.4g) n (%.3f %.3f %.3f) layer %d visibility %d: expected %s "
	    "(%.5g %.5g %.5g), got %s (%.5g %.5g %.5g)",
	    i, c.position[0], c.position[1], c.position[2], c.normal[0], c.normal[1], c.normal[2],
	    c.layer, int( c.visibility ), expectedCovered ? "covered" : "outside", expected[0],
	    expected[1], expected[2], actualCovered ? "covered" : "outside", actual[0], actual[1],
	    actual[2] );
	return text;
}

std::optional<std::string> FixtureChecks( IRenderDevice2 &device, CheckKernel &kernel,
    const std::string &name, std::mt19937 &random, Results &results )
{
	const std::string prefix = "probe." + name.substr( 0, name.find( '.' ) ) + ".";
	const std::vector<unsigned char> bytes = Load( name );
	ProbeVolumeLayout layout{};
	const bool valid = !bytes.empty() &&
	                   mapcontainer::ValidateProbeVolume( bytes.data(), bytes.size(), &layout ) ==
	                       mapcontainer::ProbeVolumeError::Ok;
	results.That( valid, prefix + "validates", std::string( kFixtures ) + name );
	if ( !valid )
		return std::nullopt;
	const std::vector<unsigned char> secondBytes = SecondAtlas( bytes, layout );
	ProbeVolumeLayout secondLayout{};
	results.That( mapcontainer::ValidateProbeVolume( secondBytes.data(), secondBytes.size(),
	                  &secondLayout ) == mapcontainer::ProbeVolumeError::Ok,
	    prefix + "second-atlas-validates" );
	const ProbeVolumeView view( bytes.data(), layout );
	const ProbeVolumeView secondView( secondBytes.data(), layout );

	resources::TextureCache cache( device );
	ProbeVolumeLayout staged{}, stagedSecond{};
	if ( std::optional<std::string> why =
	         StageProbeVolume( cache, "first", std::as_bytes( std::span( bytes ) ), staged ) )
		return why;
	if ( std::optional<std::string> why = StageProbeVolume(
	         cache, "second", std::as_bytes( std::span( secondBytes ) ), stagedSecond ) )
		return why;
	const resources::TextureEntry *atlas = cache.Find( "first-atlas" );
	const resources::TextureEntry *second = cache.Find( "second-atlas" );
	const resources::TextureEntry *grids = cache.Find( "first-grids" );
	if ( !atlas || !second || !grids )
		return std::string( "a fixture texture was not staged" );

	const std::vector<Case> cases = Cases( name, layout, random );
	float smallest = 1e30f;
	for ( std::uint32_t g = 0; g < layout.gridCount; ++g )
		for ( float spacing : layout.grids[g].spacing )
			smallest = std::min( smallest, spacing );
	const float reach = kConditioning * smallest;
	std::vector<GpuCase> gpu( cases.size() );
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		// The oracle normalizes the normal; the GLSL takes a unit one.
		const float length = std::sqrt(
		    c.normal[0] * c.normal[0] + c.normal[1] * c.normal[1] + c.normal[2] * c.normal[2] );
		gpu[i] = { { c.position[0], c.position[1], c.position[2], float( c.layer ) },
		    { c.normal[0] / length, c.normal[1] / length, c.normal[2] / length,
		        c.visibility ? 1.0f : 0.0f } };
	}
	const TextureId textures[] = { atlas->texture, second->texture, grids->texture };
	std::vector<std::byte> out;
	if ( std::optional<std::string> why = kernel.Run( cache, textures, std::uint32_t( gpu.size() ),
	         std::as_bytes( std::span( gpu ) ), gpu.size() * 3 * 16, out ) )
		return why;
	std::vector<float> values( out.size() / 4 );
	std::memcpy( values.data(), out.data(), out.size() );

	std::size_t skipped = 0, compared = 0, coverageFailures = 0, valueFailures = 0,
	            bitwiseFailures = 0, secondFailures = 0, absentFailures = 0, visibilityMatters = 0;
	std::size_t outsideFailures = 0;
	std::string outsideFirst;
	std::string coverageFirst, valueFirst, bitwiseFirst, secondFirst, absentFirst;
	for ( std::size_t i = 0; i < cases.size(); ++i )
	{
		const Case &c = cases[i];
		const float *single = &values[i * 12 + 0];
		const float *first = &values[i * 12 + 4];
		const float *secondValue = &values[i * 12 + 8];
		const bool gpuCovered = single[3] > 0.5f;
		if ( !gpuCovered && ( single[0] != 0.0f || single[1] != 0.0f || single[2] != 0.0f ) &&
		     outsideFailures++ == 0 )
			outsideFirst = Describe( i, c, single, false, single, false );
		bool same = single[3] == first[3];
		for ( int k = 0; k < 3; ++k )
			same =
			    same && std::fabs( single[k] - first[k] ) <= 1e-5f * std::fabs( single[k] ) + 1e-7f;
		if ( !same && bitwiseFailures++ == 0 )
			bitwiseFirst = Describe( i, c, single, gpuCovered, first, first[3] > 0.5f );
		float expected[3] = { 0, 0, 0 };
		const bool covered = view.Sample(
		    c.position, c.normal, ProbeVolumeLayer( c.layer ), c.visibility, expected );
		if ( c.layer >= int( layout.layerCount ) )
		{
			if ( gpuCovered && absentFailures++ == 0 )
				absentFirst = Describe( i, c, expected, false, single, gpuCovered );
			continue;
		}
		if ( !Conditioned( view, c, reach ) )
		{
			++skipped;
			continue;
		}
		++compared;
		if ( covered )
		{
			float other[3];
			if ( view.Sample(
			         c.position, c.normal, ProbeVolumeLayer( c.layer ), !c.visibility, other ) &&
			     !Near( expected, other ) )
				++visibilityMatters;
		}
		if ( gpuCovered != covered )
		{
			if ( coverageFailures++ == 0 )
				coverageFirst = Describe( i, c, expected, covered, single, gpuCovered );
			continue;
		}
		if ( !covered )
			continue;
		if ( !Near( expected, single ) && valueFailures++ == 0 )
			valueFirst = Describe( i, c, expected, covered, single, gpuCovered );
		float expectedSecond[3];
		(void)secondView.Sample(
		    c.position, c.normal, ProbeVolumeLayer( c.layer ), c.visibility, expectedSecond );
		if ( !Near( expectedSecond, secondValue ) && secondFailures++ == 0 )
			secondFirst = Describe( i, c, expectedSecond, covered, secondValue, gpuCovered );
	}
	auto count = []( std::size_t failures, const std::string &first )
	{
		return failures == 0 ? std::string()
		                     : std::to_string( failures ) + " cases differ; first " + first;
	};
	results.That(
	    coverageFailures == 0, prefix + "covered", count( coverageFailures, coverageFirst ) );
	results.That( valueFailures == 0, prefix + "irradiance", count( valueFailures, valueFirst ) );
	results.That( bitwiseFailures == 0, prefix + "pair-first-is-single",
	    count( bitwiseFailures, bitwiseFirst ) );
	results.That(
	    secondFailures == 0, prefix + "pair-second", count( secondFailures, secondFirst ) );
	results.That(
	    outsideFailures == 0, prefix + "outside-is-zero", count( outsideFailures, outsideFirst ) );
	results.That( absentFailures == 0, prefix + "absent-layer-uncovered",
	    count( absentFailures, absentFirst ) );
	results.That( float( skipped ) <= kIllConditionedShare * float( skipped + compared ),
	    prefix + "conditioned",
	    std::to_string( skipped ) + " of " + std::to_string( skipped + compared ) +
	        " cases ill-conditioned" );
	if ( name == "leak.prbv" )
		results.That( visibilityMatters > 0, prefix + "visibility-exercised",
		    std::to_string( visibilityMatters ) + " cases where visibility changes the light" );
	return std::nullopt;
}

std::optional<std::string> RunOnce( bool validate, std::span<const std::uint32_t> module,
    Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	{
		CheckKernel kernel( *device );
		if ( std::optional<std::string> why = kernel.Create(
		         module.empty() ? std::span<const std::uint32_t>( spirv::kProbeVolumeCheck )
		                        : module,
		         3, 2, "render_lab.probe-volume-check" ) )
			return why;
		std::mt19937 random( 20260929u );
		for ( const char *name : kNames )
		{
			if ( std::optional<std::string> why =
			         FixtureChecks( *device, kernel, name, random, results ) )
				return why;
		}
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kProbeSeeded[] = { { "no-normal-bias", spirv::kProbeVolumeNoNormalBias, "probe." },
    { "state-ignored", spirv::kProbeVolumeStateIgnored, "probe." },
    { "visibility-ignored", spirv::kProbeVolumeVisibilityIgnored, "probe." },
    { "no-crush", spirv::kProbeVolumeNoCrush, "probe." } };

} // namespace

int RunProbeVolumeSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "probe-volume", kProbeSeeded, RunOnce );
}

} // namespace render::lab
