//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.pbr-brdf.glsl (RFC 0007): the GLSL BRDF library
//          (materialsystem/shaderapivulkan/shaders/pbr_brdf.glsl), which
//          every native PBR stage includes, agrees with the C++ reference
//          (public/render/pbr_brdf.h) on the same inputs. A compute program
//          (pbr_brdf_check.comp) evaluates each GLSL function per case on a
//          real device, headless; the suite compares GGX, Smith visibility,
//          Schlick Fresnel, the single-scatter lobe, the multiple-scattering
//          energy compensation and directional albedo, and the clear coat.
//
//  - Seeded controls: the same comparator rejects a C++ model without
//    energy compensation, with alpha = roughness, and with the clear coat's
//    Fresnel left out, so agreement is not vacuous.
//  - With the validation layer present, no message is reported.
//
//===========================================================================//

#include "../../materialsystem/shaderapivulkan/material_spv.h"
#include "../../materialsystem/shaderapivulkan/vulkan_compute.h"
#include "headless_vulkan.h"
#include "render/pbr_brdf.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>

namespace
{
using namespace render_vulkan;
using namespace headless_vulkan;
using namespace render::pbr;

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

// pbr_brdf_check.comp's std430 structs.
struct Case
{
	float angles[4];   // N.V, N.L, N.H, V.H
	float material[4]; // roughness, F0 rgb
	float extra[4];    // split-sum A, B, clear-coat weight, clear-coat roughness
};
struct Result
{
	float terms[4]; // D, V, scalar Schlick F of F0.r
	float specular[4];
	float compensation[4];
	float albedo[4];
	float coat[4]; // specular, Fresnel
};
static_assert( sizeof( Case ) == 48 && sizeof( Result ) == 80, "pbr_brdf_check.comp layout" );

// The C++ reference for one case, in the program's result layout. Each
// seeded defect replaces one term with a plausible wrong one.
enum Defect
{
	kNone,
	kNoCompensation,     // single-scatter albedo and no compensation
	kAlphaIsRoughness,   // GGX and Smith with alpha = roughness
	kCoatWithoutFresnel, // clear coat lobe missing its Fresnel weight
};

Result Reference( const Case &c, Defect defect )
{
	const float roughness = c.material[0];
	const float lobeRoughness = defect == kAlphaIsRoughness ? std::sqrt( roughness ) : roughness;
	const Color f0 = { c.material[1], c.material[2], c.material[3] };
	const SplitSumCoefficients split = { c.extra[0], c.extra[1] };
	Result r = {};
	r.terms[0] = GgxDistribution( c.angles[2], lobeRoughness );
	r.terms[1] = SmithVisibility( c.angles[0], c.angles[1], lobeRoughness );
	r.terms[2] = FresnelSchlick( f0.red, c.angles[3] );
	const Color specular =
	    EvaluateSpecular( f0, c.angles[0], c.angles[1], c.angles[2], c.angles[3], lobeRoughness );
	r.specular[0] = specular.red;
	r.specular[1] = specular.green;
	r.specular[2] = specular.blue;
	const float channels[3] = { f0.red, f0.green, f0.blue };
	for ( int i = 0; i < 3; ++i )
	{
		r.compensation[i] =
		    defect == kNoCompensation ? 1.0f : SpecularEnergyCompensation( channels[i], split );
		r.albedo[i] = defect == kNoCompensation ? std::fmin( 1.0f, channels[i] * split.a + split.b )
		                                        : SpecularDirectionalAlbedo( channels[i], split );
	}
	const ClearCoat coat = EvaluateClearCoat( c.extra[2], c.angles[2], c.angles[3], c.extra[3] );
	r.coat[0] = coat.specular;
	r.coat[1] = coat.fresnel;
	if ( defect == kCoatWithoutFresnel && coat.fresnel > 0.0f )
		r.coat[0] = coat.specular / coat.fresnel * c.extra[2];
	return r;
}

bool Close( float gpu, float cpu )
{
	// Float evaluation order and the device's sqrt/division differ from the
	// host's; the lobe spans many decades, so the bound is relative.
	return std::isfinite( gpu ) && std::fabs( gpu - cpu ) <= 1e-6f + 3e-4f * std::fabs( cpu );
}

// Cases whose every compared value agrees; `first` reports the first miss.
size_t Agreeing(
    const std::vector<Case> &cases, const Result *gpu, Defect defect, std::string *first )
{
	size_t agreeing = 0;
	for ( size_t index = 0; index < cases.size(); ++index )
	{
		const Result cpu = Reference( cases[index], defect );
		const float *g = gpu[index].terms;
		const float *c = cpu.terms;
		bool all = true;
		for ( size_t k = 0; k < sizeof( Result ) / sizeof( float ); ++k )
		{
			if ( Close( g[k], c[k] ) )
				continue;
			all = false;
			if ( first && first->empty() )
			{
				char text[256];
				std::snprintf( text, sizeof( text ),
				    "case %zu (N.V %.3f N.L %.3f N.H %.3f V.H %.3f r %.3f F0 %.3f): value %zu "
				    "GPU %.9g CPU %.9g",
				    index, cases[index].angles[0], cases[index].angles[1], cases[index].angles[2],
				    cases[index].angles[3], cases[index].material[0], cases[index].material[1], k,
				    g[k], c[k] );
				*first = text;
			}
		}
		agreeing += all ? 1 : 0;
	}
	return agreeing;
}

std::vector<Case> Cases()
{
	std::vector<Case> cases;
	const float f0s[][3] = { { 0.04f, 0.04f, 0.04f }, { 1.0f, 1.0f, 1.0f }, { 0.95f, 0.64f, 0.54f },
	    { 0.5f, 0.2f, 0.9f } };
	for ( float roughness : { 0.02f, 0.1f, 0.35f, 0.6f, 0.85f, 1.0f } )
		for ( float normalDotView : { 0.0f, 0.05f, 0.3f, 0.7f, 1.0f } )
			for ( float normalDotLight : { 0.0f, 0.2f, 0.9f } )
				for ( float normalDotHalf : { 0.4f, 0.95f, 1.0f } )
					for ( const float *f0 : f0s )
					{
						Case c = {};
						c.angles[0] = normalDotView;
						c.angles[1] = normalDotLight;
						c.angles[2] = normalDotHalf;
						c.angles[3] = 0.25f + 0.7f * normalDotHalf * normalDotView;
						c.material[0] = roughness;
						for ( int i = 0; i < 3; ++i )
							c.material[1 + i] = f0[i];
						const SplitSumCoefficients split =
						    SampleSplitSum( normalDotView, roughness );
						c.extra[0] = split.a;
						c.extra[1] = split.b;
						c.extra[2] = normalDotLight; // coat weights 0, 0.2, 0.9
						c.extra[3] = 1.05f - roughness;
						cases.push_back( c );
					}
	return cases;
}

void Compare( Device &d, ComputeResources &compute )
{
	std::string error;
	const std::vector<Case> cases = Cases();
	const uint32_t count = static_cast<uint32_t>( cases.size() );
	const uint32_t input = compute.CreateBuffer( cases.size() * sizeof( Case ), &error );
	const uint32_t output = compute.CreateBuffer( cases.size() * sizeof( Result ), &error, true );
	const uint32_t program = compute.CreateProgram( g_pbrBrdfCheckSpv, sizeof( g_pbrBrdfCheckSpv ),
	    { ComputeBinding::StorageBuffer, ComputeBinding::StorageBuffer }, 4, &error );
	Check( input && output && program, "buffers and the pbr_brdf.glsl program: " + error );
	if ( !input || !output || !program )
		return;
	std::memcpy( compute.Map( input ), cases.data(), cases.size() * sizeof( Case ) );
	std::memset( compute.Map( output ), 0xff, cases.size() * sizeof( Result ) ); // NaN
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	bool recorded = false;
	VkFence fence = Submit( d, &cmd,
	    [&]( VkCommandBuffer c )
	    {
		    recorded = compute.RecordDispatch(
		        c, 1, program, { input, output }, &count, 4, ( count + 63 ) / 64, 1, 1 );
		    VkMemoryBarrier host = {};
		    host.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		    host.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		    host.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
		    vkCmdPipelineBarrier( c, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		        VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &host, 0, nullptr, 0, nullptr );
	    } );
	Finish( d, fence, cmd );
	compute.Collect( 1 );
	Check( recorded, "the dispatch records on the graphics queue" );
	const Result *gpu = static_cast<const Result *>( compute.Map( output ) );

	std::string first;
	const size_t agreeing = Agreeing( cases, gpu, kNone, &first );
	std::printf( "%zu of %zu cases agree\n", agreeing, cases.size() );
	Check( agreeing == cases.size(),
	    "pbr_brdf.glsl matches pbr_brdf.h on every case" + ( first.empty() ? "" : ": " + first ) );

	// The comparator is sensitive: each seeded C++ defect disagrees somewhere.
	const struct
	{
		Defect defect;
		const char *name;
	} defects[] = { { kNoCompensation, "a model without energy compensation" },
	    { kAlphaIsRoughness, "a model with alpha = roughness" },
	    { kCoatWithoutFresnel, "a clear coat without its Fresnel weight" } };
	for ( const auto &seeded : defects )
	{
		const size_t seededAgreeing = Agreeing( cases, gpu, seeded.defect, nullptr );
		std::printf(
		    "seeded %s: %zu of %zu cases agree\n", seeded.name, seededAgreeing, cases.size() );
		Check(
		    seededAgreeing < cases.size(), std::string( "the comparator rejects " ) + seeded.name );
	}

	// The compensated rough white metal keeps its energy: its albedo is one
	// while the single-scatter albedo A + B is well below it.
	const SplitSumCoefficients rough = SampleSplitSum( 0.7f, 1.0f );
	Check( rough.a + rough.b < 0.8f &&
	           std::fabs( SpecularDirectionalAlbedo( 1.0f, rough ) - 1.0f ) < 1e-5f,
	    "a rough white metal's compensated directional albedo is one" );
	for ( uint32_t resource : { input, output, program } )
		compute.Retire( resource, 1 );
	compute.Collect( 1 );
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
		Compare( d, compute );
	vkDeviceWaitIdle( d.device );
	compute.Shutdown();
	DestroyDevice( d );
	std::printf( "validation layer %s, %lu message(s)\n", d.validation ? "on" : "absent",
	    g_validationMessages );
	if ( d.validation )
		Check( g_validationMessages == 0, "no validation messages" );
	return testing::ReportConformance( g_checks, g_failures );
}
