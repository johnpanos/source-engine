//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.reflection-probes.glsl (R50-PARALLAX): the GLSL reflection
//          probe blend (materialsystem/shaderapivulkan/shaders/
//          reflection_probes.glsl), which every native PBR stage includes,
//          agrees with the C++ reference (mapcontainer::ReflectionProbesView)
//          on the same inputs. A compute program (reflection_probes_check.comp)
//          evaluates it per case on a real device, headless, over the RPRB
//          fixture's GPU texture in each mode: blended and parallax-corrected,
//          the nearest capture alone, direction-only, and the weight view.
//
//  - The fixture's own shading samples (quality/fixtures/reflection/rprb/
//    samples.txt, from the Python oracle) plus seeded random cases: points
//    inside and outside influence volumes, both sides of a wall, rough and
//    mirror lobes.
//  - Seeded controls: the same comparator rejects the reference evaluated in
//    another mode (a shader that ignored parallax, or snapped to the nearest
//    capture, or blended where it should snap), so agreement is not vacuous.
//  - A texture without the marker (an LMAP page) carries no probes.
//  - With the validation layer present, no message is reported.
//  - R50-RELIGHT: over the relight fixture (RPRB v2), the shader relit by
//    the fixture's analytic change (reflection_probes_check.comp) matches the
//    reference relit by the same change in each selection mode; the relit
//    shader is rejected against the unrelit reference (the control), and
//    with the texture's relight switch off it matches the unrelit reference.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/material_spv.h"
#include "../../materialsystem/shaderapivulkan/vulkan_compute.h"
#include "headless_vulkan.h"
#include "mapcontainer/probe_volume.h"
#include "mapcontainer/reflection_probes.h"
#include "testing/conformance_result.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using namespace render_vulkan;
using namespace headless_vulkan;
using namespace mapcontainer;

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool condition, const std::string &description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description.c_str() );
	}
}

const char *kFixtures = "quality/fixtures/reflection/rprb/";

// reflection_probes_check.comp's std430 structs.
struct Case
{
	float positionRoughness[4];
	float normal[4];
	float reflected[4];
};
struct Result
{
	float radiance[4];
};
static_assert(
    sizeof( Case ) == 48 && sizeof( Result ) == 16, "reflection_probes_check.comp layout" );

std::vector<char> Load( const std::string &name )
{
	std::ifstream file( kFixtures + name, std::ios::binary );
	return std::vector<char>(
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
	// Seeded cases: anywhere in and around the fixture's 6 x 4 x 3 m room
	// (Source units), including outside every influence and behind walls.
	const float scale = 39.37007874015748f;
	std::mt19937 rng( 20260925 );
	std::uniform_real_distribution<float> unit( 0.0f, 1.0f );
	for ( int i = 0; i < 400; ++i )
	{
		Case c = {};
		c.positionRoughness[0] = ( -0.5f + 7.0f * unit( rng ) ) * scale;
		c.positionRoughness[1] = ( -0.5f + 5.0f * unit( rng ) ) * scale;
		c.positionRoughness[2] = ( 3.2f * unit( rng ) ) * scale;
		const float roughness[] = { 0.0f, 0.05f, 0.3f, 0.7f, 1.0f };
		c.positionRoughness[3] = roughness[i % 5];
		float n[3], r[3];
		float nl = 0.0f, rl = 0.0f;
		for ( int k = 0; k < 3; ++k )
		{
			n[k] = unit( rng ) * 2.0f - 1.0f;
			r[k] = unit( rng ) * 2.0f - 1.0f;
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

bool Close( const float gpu[3], const float cpu[3] )
{
	// Device trigonometry and division differ from the host's; a lookup that
	// lands on the other side of a stripe edge by a rounding moves one
	// bilinear tap, so the bound is absolute plus a small relative part.
	for ( int c = 0; c < 3; ++c )
		if ( !std::isfinite( gpu[c] ) ||
		     std::fabs( gpu[c] - cpu[c] ) > 2e-3f + 1e-3f * std::fabs( cpu[c] ) )
			return false;
	return true;
}

// Runs the program over `cases` on `texture` (halves); fills `out`.
bool Evaluate( Device &d, ComputeResources &compute, const std::vector<uint16_t> &texture,
    uint32_t width, uint32_t height, const std::vector<Case> &cases, std::vector<Result> *out )
{
	std::string error;
	const uint32_t texels = compute.CreateBuffer( texture.size() * 2, &error );
	const uint32_t input = compute.CreateBuffer( cases.size() * sizeof( Case ), &error );
	const uint32_t output = compute.CreateBuffer( cases.size() * sizeof( Result ), &error, true );
	const uint32_t program =
	    compute.CreateProgram( g_reflectionProbesCheckSpv, sizeof( g_reflectionProbesCheckSpv ),
	        { ComputeBinding::StorageBuffer, ComputeBinding::StorageBuffer,
	            ComputeBinding::StorageBuffer },
	        12, &error );
	Check( texels && input && output && program,
	    "buffers and the reflection_probes.glsl program: " + error );
	if ( !texels || !input || !output || !program )
		return false;
	std::memcpy( compute.Map( texels ), texture.data(), texture.size() * 2 );
	std::memcpy( compute.Map( input ), cases.data(), cases.size() * sizeof( Case ) );
	std::memset( compute.Map( output ), 0xff, cases.size() * sizeof( Result ) ); // NaN
	const uint32_t push[3] = { uint32_t( cases.size() ), width, height };
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	bool recorded = false;
	VkFence fence = Submit( d, &cmd,
	    [&]( VkCommandBuffer c )
	    {
		    recorded = compute.RecordDispatch( c, 1, program, { texels, input, output }, push, 12,
		        ( uint32_t( cases.size() ) + 63 ) / 64, 1, 1 );
		    VkMemoryBarrier host = {};
		    host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		    host.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		    host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		    vkCmdPipelineBarrier( c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		        VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &host, 0, nullptr, 0, nullptr );
	    } );
	Finish( d, fence, cmd );
	compute.Collect( 1 );
	Check( recorded, "the dispatch records" );
	const Result *results = static_cast<const Result *>( compute.Map( output ) );
	out->assign( results, results + cases.size() );
	for ( uint32_t resource : { texels, input, output, program } )
		compute.Retire( resource, 1 );
	compute.Collect( 1 );
	return recorded;
}

size_t Agreeing( const ReflectionProbesView &view, ReflectionProbeMode reference,
    const std::vector<Case> &cases, const std::vector<Result> &gpu, std::string *first,
    const ReflectionProbeDiffuseChange *change = nullptr )
{
	size_t agreeing = 0;
	for ( size_t i = 0; i < cases.size(); ++i )
	{
		float cpu[3];
		view.Radiance( cases[i].positionRoughness, cases[i].normal, cases[i].reflected,
		    cases[i].positionRoughness[3], reference, cpu, change );
		if ( Close( gpu[i].radiance, cpu ) && gpu[i].radiance[3] == 1.0f )
		{
			++agreeing;
			continue;
		}
		if ( first && first->empty() )
		{
			char text[256];
			std::snprintf( text, sizeof( text ),
			    "case %zu at (%.1f %.1f %.1f) r %.2f: GPU (%.5f %.5f %.5f) CPU (%.5f %.5f %.5f)", i,
			    cases[i].positionRoughness[0], cases[i].positionRoughness[1],
			    cases[i].positionRoughness[2], cases[i].positionRoughness[3], gpu[i].radiance[0],
			    gpu[i].radiance[1], gpu[i].radiance[2], cpu[0], cpu[1], cpu[2] );
			*first = text;
		}
	}
	return agreeing;
}

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
		return "blend weights";
	default:
		return "off";
	}
}

void Compare( Device &d, ComputeResources &compute )
{
	const std::vector<char> bytes = Load( "valid.rprb" );
	ReflectionProbesLayout layout = {};
	Check( ValidateReflectionProbes( bytes.data(), bytes.size(), &layout ) ==
	           ReflectionProbesError::Ok,
	    "the RPRB fixture validates" );
	if ( g_failures )
		return;
	const ReflectionProbesView view( bytes.data(), layout );
	const std::vector<Case> cases = Cases();
	const uint32_t width = layout.atlasWidth;
	const uint32_t height = ReflectionProbeTextureRows( layout );
	const ReflectionProbeMode modes[] = { ReflectionProbeMode::Blend, ReflectionProbeMode::Nearest,
	    ReflectionProbeMode::DirectionOnly, ReflectionProbeMode::BlendWeights };
	const int kModes = 4;
	std::vector<Result> gpu[kModes];
	for ( int m = 0; m < kModes; ++m )
	{
		std::vector<uint16_t> texture( size_t( width ) * height * 4 );
		WriteReflectionProbeTexture( bytes.data(), layout, modes[m], texture.data() );
		if ( !Evaluate( d, compute, texture, width, height, cases, &gpu[m] ) )
			return;
		std::string first;
		const size_t agreeing = Agreeing( view, modes[m], cases, gpu[m], &first );
		std::printf( "%s: %zu of %zu cases agree\n", ModeName( modes[m] ), agreeing, cases.size() );
		Check( agreeing == cases.size(), std::string( "reflection_probes.glsl matches the C++ "
		                                              "reference, " ) +
		                                     ModeName( modes[m] ) +
		                                     ( first.empty() ? "" : ": " + first ) );
	}
	// Sensitivity: each mode's GPU results disagree with the reference in
	// every other mode (a shader that dropped parallax, snapped, or blended).
	for ( int m = 0; m < kModes; ++m )
		for ( int other = 0; other < kModes; ++other )
		{
			if ( other == m )
				continue;
			const size_t agreeing = Agreeing( view, modes[other], cases, gpu[m], nullptr );
			std::printf( "seeded: %s GPU against the %s reference: %zu of %zu agree\n",
			    ModeName( modes[m] ), ModeName( modes[other] ), agreeing, cases.size() );
			Check( agreeing < cases.size() * 9 / 10,
			    std::string( "the comparator rejects a " ) + ModeName( modes[other] ) +
			        " reference for the " + ModeName( modes[m] ) + " shader" );
		}
	// A texture without the marker (an LMAP page's first texel) carries none.
	std::vector<uint16_t> lightmap( size_t( width ) * height * 4, FloatToHalf( 0.5f ) );
	std::vector<Result> none;
	if ( Evaluate( d, compute, lightmap, width, height, cases, &none ) )
	{
		bool silent = true;
		for ( const Result &result : none )
			silent = silent && result.radiance[3] == 0.0f && result.radiance[0] == 0.0f;
		Check( silent, "a texture without the marker carries no probes" );
	}
}

// reflection_probe_set.fixture_change, as reflection_probes_check.comp has it.
void FixtureChange( void *, const float p[3], const float n[3], float out[3] )
{
	out[0] = 0.002f * p[0] + 0.5f * std::max( n[2], 0.0f );
	out[1] = 0.3f - 0.003f * p[1];
	out[2] = 0.25f * n[0] - 0.1f;
}

void CompareRelight( Device &d, ComputeResources &compute )
{
	const std::vector<char> bytes = Load( "valid-relight.rprb" );
	ReflectionProbesLayout layout = {};
	Check( ValidateReflectionProbes( bytes.data(), bytes.size(), &layout ) ==
	               ReflectionProbesError::Ok &&
	           layout.relight,
	    "the relight fixture validates with relight bands" );
	if ( g_failures )
		return;
	const ReflectionProbesView view( bytes.data(), layout );
	const ReflectionProbeDiffuseChange change = { FixtureChange, nullptr };
	const std::vector<Case> cases = Cases();
	const uint32_t width = layout.atlasWidth;
	const uint32_t height = ReflectionProbeTextureRows( layout );
	const ReflectionProbeMode modes[] = { ReflectionProbeMode::Blend, ReflectionProbeMode::Nearest,
	    ReflectionProbeMode::DirectionOnly };
	for ( ReflectionProbeMode mode : modes )
	{
		std::vector<uint16_t> texture( size_t( width ) * height * 4 );
		std::vector<Result> relit, baked;
		WriteReflectionProbeTexture( bytes.data(), layout, mode, texture.data(), true );
		if ( !Evaluate( d, compute, texture, width, height, cases, &relit ) )
			return;
		WriteReflectionProbeTexture( bytes.data(), layout, mode, texture.data(), false );
		if ( !Evaluate( d, compute, texture, width, height, cases, &baked ) )
			return;
		std::string first;
		const size_t agreeing = Agreeing( view, mode, cases, relit, &first, &change );
		const size_t control = Agreeing( view, mode, cases, relit, nullptr );
		std::string offFirst;
		const size_t off = Agreeing( view, mode, cases, baked, &offFirst );
		std::printf( "relit %s: %zu of %zu agree; against the unrelit reference %zu; switched off "
		             "%zu\n",
		    ModeName( mode ), agreeing, cases.size(), control, off );
		Check( agreeing == cases.size(),
		    std::string( "relit reflection_probes.glsl matches the relit reference, " ) +
		        ModeName( mode ) + ( first.empty() ? "" : ": " + first ) );
		Check( control < cases.size() * 9 / 10,
		    std::string( "the comparator rejects the unrelit reference for the relit shader, " ) +
		        ModeName( mode ) );
		Check( off == cases.size(),
		    std::string( "the relight switch off matches the unrelit reference, " ) +
		        ModeName( mode ) + ( offFirst.empty() ? "" : ": " + offFirst ) );
	}
}

} // namespace

int main()
{
	Device d;
	if ( !CreateDevice( &d ) )
	{
		std::fprintf( stderr, "SKIP: no Vulkan device\n" );
		return 77;
	}
	std::printf( "device %s\n", d.name.c_str() );
	ComputeResources compute;
	std::string error;
	const bool ready = compute.Init( d.physical, d.device, d.chain.Enabled(), &error );
	Check( ready, "compute init: " + error );
	if ( ready )
	{
		Compare( d, compute );
		CompareRelight( d, compute );
	}
	vkDeviceWaitIdle( d.device );
	compute.Shutdown();
	DestroyDevice( d );
	std::printf( "validation layer %s, %lu message(s)\n", d.validation ? "on" : "absent",
	    g_validationMessages );
	if ( d.validation )
		Check( g_validationMessages == 0, "no validation messages" );
	return testing::ReportConformance( g_checks, g_failures );
}
