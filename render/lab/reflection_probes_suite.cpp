//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab suite reflection-probes (RFC 0016 K11 "Model assembly",
//			the image-based specular term): the map's RPRB reflection probes
//			(render/shaders/common/reflection_probes.glsl), blended,
//			parallax-corrected, relit and with distance-based roughness,
//			evaluated on the GPU by a check kernel
//			(reflection_probes_check.comp) over the checked-in RPRB fixtures
//			(quality/fixtures/reflection/rprb: valid.rprb and
//			valid-relight.rprb, written by tools/quality/reflection_probe_set.py)
//			staged by render_lab's StageReflectionProbes, and judged against
//			their oracle, mapcontainer::ReflectionProbesView::Radiance:
//			- the fixture's shading samples and 400 seeded random rays (in
//			  and outside every influence, behind walls, mirror to rough) in
//			  each selection mode (blended, the nearest capture, direction
//			  only, the weight view): every case within 2e-3 + 1e-3 relative
//			  (the native oracle's bound: device trigonometry moves a lookup
//			  by a rounding);
//			- each mode's GPU results are rejected by every other mode's
//			  reference (under 90 percent agree), so agreement is not vacuous;
//			- a texture without the marker (an LMAP page) carries no probes;
//			- relit (RPRB v2) by the fixture's analytic light and moving
//			  occluder: the relit reference in each selection mode; the relit
//			  results are rejected by the unrelit reference, and, blended, by
//			  the reference without the occluder; with the texture's relight
//			  switch off the unrelit reference.
//			The split-sum weighting of this light is the pbr point's
//			(pbr_brdf.glsl's directional albedo), judged by its own suites.
//
//			Seeded (--sensitivity): distance-based roughness dropped, the
//			facing term dropped, and removed light relit additively: each a
//			check kernel built from reflection_probes.glsl. The fixtures
//			carry two probes, so the blend's third share is always zero and
//			its subtraction is not exercised (a seeded kernel without it
//			passed; a fixture with three overlapping influences would).
//
//=============================================================================//

#include "lab_compute.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "mapcontainer/reflection_probes.h"
#include "render/device/device.h"
#include "spv/reflection_probes_check_spv.h"

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
using mapcontainer::ReflectionProbeMode;
using mapcontainer::ReflectionProbeOccluder;
using mapcontainer::ReflectionProbeRelight;
using mapcontainer::ReflectionProbesLayout;
using mapcontainer::ReflectionProbesView;

constexpr const char *kFixtures = "quality/fixtures/reflection/rprb/";
constexpr float kAbsolute = 2e-3f;
constexpr float kRelative = 1e-3f;
constexpr int kRandomCases = 400;

struct Case
{
	float positionRoughness[4];
	float normal[4];
	float reflected[4];
};
static_assert( sizeof( Case ) == 48, "reflection_probes_check.comp's Case" );

std::vector<unsigned char> Load( const std::string &name )
{
	std::ifstream file( std::string( kFixtures ) + name, std::ios::binary );
	return std::vector<unsigned char>(
	    std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} );
}

std::vector<Case> Cases()
{
	std::vector<Case> cases;
	std::ifstream samples( std::string( kFixtures ) + "samples.txt" );
	std::string line;
	while ( std::getline( samples, line ) )
	{
		if ( line.empty() || line[0] == '#' )
			continue;
		std::istringstream fields( line );
		float mode;
		Case c = {};
		fields >> mode >> c.positionRoughness[0] >> c.positionRoughness[1] >>
		    c.positionRoughness[2] >> c.normal[0] >> c.normal[1] >> c.normal[2] >> c.reflected[0] >>
		    c.reflected[1] >> c.reflected[2] >> c.positionRoughness[3];
		if ( mode == 1.0f ) // each mode's cases are the same set of rays
			cases.push_back( c );
	}
	// Anywhere in and around the fixture's 6 x 4 x 3 m room (Source units),
	// outside every influence and behind walls included.
	const float scale = 39.37007874015748f;
	std::mt19937 random( 20260929u );
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	const float roughness[] = { 0.0f, 0.05f, 0.3f, 0.7f, 1.0f };
	for ( int i = 0; i < kRandomCases; ++i )
	{
		Case c = {};
		c.positionRoughness[0] = ( -0.5f + 7.0f * unit( random ) ) * scale;
		c.positionRoughness[1] = ( -0.5f + 5.0f * unit( random ) ) * scale;
		c.positionRoughness[2] = ( 3.2f * unit( random ) ) * scale;
		c.positionRoughness[3] = roughness[i % 5];
		float n[3], r[3];
		float nl = 0.0f, rl = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			n[k] = unit( random ) * 2.0f - 1.0f;
			r[k] = unit( random ) * 2.0f - 1.0f;
			nl += n[k] * n[k];
			rl += r[k] * r[k];
		}
		for ( int k = 0; k < 3; ++k )
		{
			c.normal[k] = n[k] / std::sqrt( nl );
			c.reflected[k] = r[k] / std::sqrt( rl );
		}
		cases.push_back( c );
	}
	return cases;
}

// reflection_probe_set.fixture_light and FIXTURE_OCCLUDER, as the check
// kernel has them.
void FixtureLight( void *, const float p[3], const float n[3], float now[3], float baked[3] )
{
	baked[0] = 0.3f + 0.001f * p[0];
	baked[1] = 0.25f + 0.1f * std::fabs( n[2] );
	baked[2] = 0.2f + 0.0005f * p[2];
	now[0] = baked[0] + 0.002f * p[0] + 0.5f * std::max( n[2], 0.0f ) - 0.45f;
	now[1] = baked[1] + 0.3f - 0.003f * p[1];
	now[2] = baked[2] + 0.25f * n[0] - 0.1f;
}

constexpr float kUnitsPerMeter = 39.37007874015748f;
const ReflectionProbeOccluder kFixtureOccluder = {
    { 2.8f * kUnitsPerMeter, 1.2f * kUnitsPerMeter, 0.0f },
    { 3.2f * kUnitsPerMeter, 2.8f * kUnitsPerMeter, 2.2f * kUnitsPerMeter }, 0.4f };

const char *ModeName( ReflectionProbeMode mode )
{
	switch ( mode )
	{
	case ReflectionProbeMode::Blend:
		return "blended";
	case ReflectionProbeMode::Nearest:
		return "nearest";
	case ReflectionProbeMode::DirectionOnly:
		return "direction-only";
	case ReflectionProbeMode::BlendWeights:
		return "blend-weights";
	default:
		return "off";
	}
}

struct Lab
{
	IRenderDevice2 &device;
	CheckKernel &kernel;
	std::vector<Case> cases;
	int staged = 0;
};

// The GPU radiance (and carried flag) of every case over a probe texture.
std::optional<std::string> Evaluate( Lab &lab, const std::vector<unsigned char> &bytes,
    ReflectionProbeMode mode, bool relight, std::vector<float> &out )
{
	resources::TextureCache cache( lab.device );
	ReflectionProbesLayout layout{};
	const std::string name = "probes-" + std::to_string( lab.staged++ );
	if ( std::optional<std::string> why = StageReflectionProbes(
	         cache, name, std::as_bytes( std::span( bytes ) ), mode, relight, layout ) )
		return why;
	const TextureId textures[] = { cache.Find( name )->texture };
	std::vector<std::byte> results;
	if ( std::optional<std::string> why =
	         lab.kernel.Run( cache, textures, std::uint32_t( lab.cases.size() ),
	             std::as_bytes( std::span( lab.cases ) ), lab.cases.size() * 16, results ) )
		return why;
	out.resize( lab.cases.size() * 4 );
	std::memcpy( out.data(), results.data(), results.size() );
	return std::nullopt;
}

// How many cases the reference in `mode` agrees with; the first that does
// not, described.
std::size_t Agreeing( const Lab &lab, const ReflectionProbesView &view, ReflectionProbeMode mode,
    const std::vector<float> &gpu, std::string *first, const ReflectionProbeRelight *relight )
{
	std::size_t agreeing = 0;
	for ( std::size_t i = 0; i < lab.cases.size(); ++i )
	{
		const Case &c = lab.cases[i];
		float cpu[3];
		view.Radiance( c.positionRoughness, c.normal, c.reflected, c.positionRoughness[3], mode,
		    cpu, relight );
		const float *g = &gpu[i * 4];
		bool close = g[3] == 1.0f;
		for ( int k = 0; k < 3; ++k )
			close = close && std::isfinite( g[k] ) &&
			        std::fabs( g[k] - cpu[k] ) <= kAbsolute + kRelative * std::fabs( cpu[k] );
		if ( close )
		{
			++agreeing;
			continue;
		}
		if ( first && first->empty() )
		{
			char text[256];
			std::snprintf( text, sizeof( text ),
			    "case %zu at (%.1f %.1f %.1f) r %.2f: GPU (%.5f %.5f %.5f) CPU (%.5f %.5f %.5f)", i,
			    c.positionRoughness[0], c.positionRoughness[1], c.positionRoughness[2],
			    c.positionRoughness[3], g[0], g[1], g[2], cpu[0], cpu[1], cpu[2] );
			*first = text;
		}
	}
	return agreeing;
}

std::optional<std::string> BlendChecks( Lab &lab, Results &results )
{
	const std::vector<unsigned char> bytes = Load( "valid.rprb" );
	ReflectionProbesLayout layout{};
	const bool valid =
	    !bytes.empty() && mapcontainer::ValidateReflectionProbes( bytes.data(), bytes.size(),
	                          &layout ) == mapcontainer::ReflectionProbesError::Ok;
	results.That( valid, "rprb.valid.validates" );
	if ( !valid )
		return std::nullopt;
	const ReflectionProbesView view( bytes.data(), layout );
	const ReflectionProbeMode modes[] = { ReflectionProbeMode::Blend, ReflectionProbeMode::Nearest,
	    ReflectionProbeMode::DirectionOnly, ReflectionProbeMode::BlendWeights };
	std::vector<float> gpu[std::size( modes )];
	for ( std::size_t m = 0; m < std::size( modes ); ++m )
	{
		if ( std::optional<std::string> why = Evaluate( lab, bytes, modes[m], true, gpu[m] ) )
			return why;
		std::string first;
		const std::size_t agreeing = Agreeing( lab, view, modes[m], gpu[m], &first, nullptr );
		results.That( agreeing == lab.cases.size(),
		    std::string( "rprb.valid." ) + ModeName( modes[m] ) + ".oracle",
		    std::to_string( agreeing ) + " of " + std::to_string( lab.cases.size() ) + " agree" +
		        ( first.empty() ? "" : "; first " + first ) );
	}
	for ( std::size_t m = 0; m < std::size( modes ); ++m )
	{
		for ( std::size_t other = 0; other < std::size( modes ); ++other )
		{
			if ( other == m )
				continue;
			const std::size_t agreeing =
			    Agreeing( lab, view, modes[other], gpu[m], nullptr, nullptr );
			results.That( agreeing < lab.cases.size() * 9 / 10,
			    std::string( "rprb.valid." ) + ModeName( modes[m] ) + ".rejects-" +
			        ModeName( modes[other] ),
			    std::to_string( agreeing ) + " of " + std::to_string( lab.cases.size() ) +
			        " agree" );
		}
	}
	// A texture without the marker (an LMAP page's first texel) carries none.
	{
		resources::TextureCache cache( lab.device );
		TextureDesc desc;
		desc.format = Format::kRGBA16Float;
		desc.width = layout.atlasWidth;
		desc.height = mapcontainer::ReflectionProbeTextureRows( layout );
		desc.usages = { ResourceUsage::kCopyDestination, ResourceUsage::kSampled };
		const std::vector<std::uint16_t> page(
		    std::size_t( desc.width ) * desc.height * 4, FloatToHalf( 0.5f ) );
		auto staged = cache.Stage( "page", desc, std::as_bytes( std::span( page ) ) );
		if ( !staged )
			return std::string( "the page was refused" );
		const TextureId textures[] = { staged.Value().texture };
		std::vector<std::byte> out;
		if ( std::optional<std::string> why =
		         lab.kernel.Run( cache, textures, std::uint32_t( lab.cases.size() ),
		             std::as_bytes( std::span( lab.cases ) ), lab.cases.size() * 16, out ) )
			return why;
		std::vector<float> values( out.size() / 4 );
		std::memcpy( values.data(), out.data(), out.size() );
		bool silent = true;
		for ( std::size_t i = 0; i < lab.cases.size(); ++i )
			silent = silent && values[i * 4 + 3] == 0.0f && values[i * 4] == 0.0f &&
			         values[i * 4 + 1] == 0.0f && values[i * 4 + 2] == 0.0f;
		results.That( silent, "rprb.no-marker-carries-none" );
	}
	return std::nullopt;
}

std::optional<std::string> RelightChecks( Lab &lab, Results &results )
{
	const std::vector<unsigned char> bytes = Load( "valid-relight.rprb" );
	ReflectionProbesLayout layout{};
	const bool valid = !bytes.empty() &&
	                   mapcontainer::ValidateReflectionProbes( bytes.data(), bytes.size(),
	                       &layout ) == mapcontainer::ReflectionProbesError::Ok &&
	                   layout.relight;
	results.That( valid, "rprb.relight.validates-with-bands" );
	if ( !valid )
		return std::nullopt;
	const ReflectionProbesView view( bytes.data(), layout );
	const ReflectionProbeRelight relight = { FixtureLight, nullptr, &kFixtureOccluder, 1 };
	const ReflectionProbeRelight open = { FixtureLight, nullptr, nullptr, 0 };
	for ( ReflectionProbeMode mode : { ReflectionProbeMode::Blend, ReflectionProbeMode::Nearest,
	          ReflectionProbeMode::DirectionOnly } )
	{
		std::vector<float> relit, unrelit;
		if ( std::optional<std::string> why = Evaluate( lab, bytes, mode, true, relit ) )
			return why;
		if ( std::optional<std::string> why = Evaluate( lab, bytes, mode, false, unrelit ) )
			return why;
		const std::string prefix = std::string( "rprb.relight." ) + ModeName( mode ) + ".";
		std::string first, offFirst;
		const std::size_t agreeing = Agreeing( lab, view, mode, relit, &first, &relight );
		const std::size_t control = Agreeing( lab, view, mode, relit, nullptr, nullptr );
		const std::size_t unoccluded = Agreeing( lab, view, mode, relit, nullptr, &open );
		const std::size_t off = Agreeing( lab, view, mode, unrelit, &offFirst, nullptr );
		const std::string of = " of " + std::to_string( lab.cases.size() ) + " agree";
		results.That( agreeing == lab.cases.size(), prefix + "oracle",
		    std::to_string( agreeing ) + of + ( first.empty() ? "" : "; first " + first ) );
		results.That( control < lab.cases.size() * 9 / 10, prefix + "rejects-unrelit",
		    std::to_string( control ) + of );
		if ( mode == ReflectionProbeMode::Blend )
			results.That( unoccluded < lab.cases.size(), prefix + "rejects-unoccluded",
			    std::to_string( unoccluded ) + of );
		results.That( off == lab.cases.size(), prefix + "switch-off-is-unrelit",
		    std::to_string( off ) + of + ( offFirst.empty() ? "" : "; first " + offFirst ) );
	}
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
		         module.empty() ? std::span<const std::uint32_t>( spirv::kReflectionProbesCheck )
		                        : module,
		         1, 1, "render_lab.reflection-probes-check" ) )
			return why;
		Lab lab{ *device, kernel, Cases() };
		if ( std::optional<std::string> why = BlendChecks( lab, results ) )
			return why;
		if ( std::optional<std::string> why = RelightChecks( lab, results ) )
			return why;
		(void)device->WaitIdle();
	}
	device.reset();
	messages = counter.load();
	return std::nullopt;
}

const Seeded kProbesSeeded[] = {
    { "no-distance-roughness", spirv::kReflectionProbesNoDistanceRoughness, "rprb." },
    { "no-facing", spirv::kReflectionProbesNoFacing, "rprb." },
    { "relight-added-only", spirv::kReflectionProbesRelightAddedOnly, "rprb.relight." } };

} // namespace

int RunReflectionProbesSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "reflection-probes", kProbesSeeded, RunOnce );
}

} // namespace render::lab
