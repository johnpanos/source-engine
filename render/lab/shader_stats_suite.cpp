//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_lab's shader-stats suite (RFC 0016 Source 2 lighting
//			defaults, step 3/4): compiles one surface variant so the device's
//			pipeline statistics (SOURCE_VK_PIPELINE_STATS=1) report it, for the
//			register-peak work on the world PBR point. The variant comes from
//			the environment: SHADER_STATS_TERMS (kSurface* bits, decimal),
//			SHADER_STATS_VIEW (view features) and SHADER_STATS_MATERIAL
//			(material features), on the world layout. Compiler diagnostics,
//			not timings; the one check is that the variant compiled.
//
//=============================================================================//

#include "lab_canvas.h"
#include "lab_suite.h"
#include "lab_support.h"
#include "suites.h"

#include "render/material/surface_program.h"

#include <atomic>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

namespace render::lab
{

namespace
{

std::uint32_t FromEnvironment( const char *name, std::uint32_t fallback )
{
	const char *value = std::getenv( name );
	return value && *value ? std::uint32_t( std::strtoul( value, nullptr, 0 ) ) : fallback;
}

std::optional<std::string> RunChecks(
    bool validate, std::span<const std::uint32_t>, Results &results, std::uint64_t &messages )
{
	std::atomic<std::uint64_t> counter{ 0 };
	std::unique_ptr<device::IRenderDevice2> device;
	if ( std::optional<std::string> why = CreateLabDevice( validate, counter, device ) )
		return why;
	auto program = material::SurfaceProgram::Create( *device, kCanvasColor, kCanvasDepth, 1 );
	if ( !program )
		return std::string( "the surface program was refused" );
	material::SurfaceVariant variant;
	variant.layout = material::SurfaceVertexLayout::kWorld;
	variant.terms = FromEnvironment( "SHADER_STATS_TERMS", material::kSurfacePbr );
	variant.viewFeatures = FromEnvironment( "SHADER_STATS_VIEW", 0 );
	variant.materialFeatures =
	    FromEnvironment( "SHADER_STATS_MATERIAL", material::kSurfaceDynamicMaterialFeatures );
	const auto pipeline = program.Value()->Pipeline( variant );
	results.That( pipeline.HasValue(), "shader-stats.variant-compiled",
	    "terms " + std::to_string( variant.terms ) + ", view " +
	        std::to_string( variant.viewFeatures ) + ", material " +
	        std::to_string( variant.materialFeatures ) );
	program.Value().reset();
	(void)device->WaitIdle();
	messages = counter.load();
	return std::nullopt;
}

} // namespace

int RunShaderStatsSuite( int argc, char **argv )
{
	return RunSeededSuite( argc, argv, "shader-stats", {}, RunChecks );
}

} // namespace render::lab
