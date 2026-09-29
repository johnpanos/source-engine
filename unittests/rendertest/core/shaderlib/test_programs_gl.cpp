//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-artifacts.gl (RFC 0016 K10, "Artifacts per target"
//			on the OpenGL adapter, through the core artifact store):
//
//			G1 the store (render/shaderlib/core_artifacts.h) holds every core
//			   program stage in SPIR-V and in GLSL 4.50, and Resolve takes the
//			   one of the format asked for;
//			G2 every core program, resolved for render.device.gl's format,
//			   compiles and links as a pipeline, the adapter reading the
//			   artifacts' form (combined-sampler names, the draw-constant slot,
//			   the specialization line); a truncated artifact must fail
//			   kInvalidDescription;
//			G3 the passes that resolve through the store create their programs
//			   on GL (output, skinning, cluster assignment), and a suite's seeded
//			   SPIR-V variant is refused on GL rather than run unseeded.
//
//=============================================================================//

#include "render/device/gl/provider.h"
#include "render/pass/lights/cluster_pass.h"
#include "render/pass/output/output.h"
#include "render/pass/skinning/skinning.h"
#include "render/shaderlib/core_artifacts.h"
#include "spv/skin_defects_spv.h"
#include "testing/checks.h"

#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using namespace render;
using namespace render::device;

struct Program
{
	const char *name;
	const char *first;
	const char *second = nullptr; // the fragment stage, if any
	PipelineKind kind = PipelineKind::kGraphics;
	bool depthOnly = false;

	shaderlib::PipelineRecipe Recipe() const
	{
		return second ? shaderlib::CoreRecipe( { first, second }, kind )
		              : shaderlib::CoreRecipe( { first }, kind );
	}
};

// Each vertex stage with the fragment stage it is drawn with (K11 a2's
// program set: unlit and vertexlit are points of the surface program).
const Program kPrograms[] = {
    { "surface-flat", "render/material/families/surface_flat.vert",
        "render/material/families/surface.frag" },
    { "surface-world", "render/material/families/surface_world.vert",
        "render/material/families/surface.frag" },
    { "surface-model", "render/material/families/surface_model.vert",
        "render/material/families/surface.frag" },
    { "lines", "render/pass/lines/lines.vert", "render/pass/lines/lines.frag" },
    { "debug-hatch", "render/pass/debug/fullscreen.vert", "render/pass/debug/hatch.frag" },
    { "debug-tint", "render/pass/debug/fullscreen.vert", "render/pass/debug/tint.frag" },
    { "output", "render/pass/output/output.vert", "render/pass/output/output.frag" },
    { "shadow-depth", "render/pass/shadows/shadow_depth.vert", nullptr, PipelineKind::kGraphics,
        true },
    { "shadow-receiver", "render/pass/shadows/shadow_receiver.vert",
        "render/pass/shadows/shadow_receiver.frag" },
    { "skin", "render/pass/skinning/skin.comp", nullptr, PipelineKind::kCompute },
    { "cluster-assign", "render/pass/lights/cluster_assign.comp", nullptr, PipelineKind::kCompute },
};

// A pipeline of the program's stages only: no layouts (the reflected
// bindings are checked against them by D4, which the port suite covers), a
// target it can draw into.
DeviceResult<PipelineId> Create(
    IRenderDevice2 &device, const Program &program, const shaderlib::IShaderArtifactSource &source )
{
	shaderlib::PipelineRecipe recipe = program.Recipe();
	if ( program.kind == PipelineKind::kGraphics )
	{
		if ( program.depthOnly )
			recipe.depthFormat = Format::kD32Float;
		else
			recipe.colorFormats = { Format::kRGBA16Float };
	}
	recipe.debugName = program.name;
	auto resolved = shaderlib::Resolve( recipe, source, device.Facts().artifactFormat );
	if ( !resolved )
		return foundation::MakeUnexpected(
		    DeviceError{ DeviceStatus::kUnsupported, DeviceOperation::kCreatePipeline, 0 } );
	PipelineDesc desc = resolved.Value().Desc();
	// Every stage's reflected block fits the pipeline's (D16).
	desc.drawConstantBytes = kMaxDrawConstantBytes;
	std::vector<ShaderArtifactView> stages( desc.stages.begin(), desc.stages.end() );
	for ( ShaderArtifactView &stage : stages )
		stage.bindings = {};
	desc.stages = stages;
	return device.CreatePipeline( desc );
}

bool SameBindings( const std::vector<ReflectedBinding> &a, const std::vector<ReflectedBinding> &b )
{
	if ( a.size() != b.size() )
		return false;
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( a[i].group != b[i].group || a[i].binding != b[i].binding || a[i].kind != b[i].kind )
			return false;
	}
	return true;
}

void StoreClauses( testing::Checks &checks )
{
	const shaderlib::IShaderArtifactSource &store = shaderlib::CoreArtifacts();
	std::size_t both = 0, stages = 0;
	for ( const Program &program : kPrograms )
	{
		for ( const char *source : { program.first, program.second } )
		{
			if ( !source )
				continue;
			++stages;
			const shaderlib::ArtifactKey spirv{ std::string( source ),
			    std::string( shaderlib::CoreCompiler() ), ArtifactFormat::kSpirv, 0 };
			shaderlib::ArtifactKey glsl = spirv;
			glsl.format = ArtifactFormat::kGlsl450;
			const shaderlib::ShaderArtifact *a = store.Find( spirv );
			const shaderlib::ShaderArtifact *b = store.Find( glsl );
			both += a && b && a->code.size() % 4 == 0 && b->code.size() > 8 &&
			        std::string_view( reinterpret_cast<const char *>( b->code.data() ), 8 ) ==
			            "#version" &&
			        SameBindings( a->bindings, b->bindings ) &&
			        a->drawConstantBytes == b->drawConstantBytes;
		}
	}
	checks.That( both == stages,
	    "gl.store every core stage is held in SPIR-V and GLSL 4.50 with one reflection" );
	shaderlib::PipelineRecipe recipe = shaderlib::CoreRecipe(
	    { "render/pass/output/output.vert", "render/pass/output/output.frag" } );
	auto glsl = shaderlib::Resolve( recipe, store, ArtifactFormat::kGlsl450 );
	auto spirv = shaderlib::Resolve( recipe, store, ArtifactFormat::kSpirv );
	checks.That( glsl && spirv &&
	                 glsl.Value().Desc().stages[1].format == ArtifactFormat::kGlsl450 &&
	                 spirv.Value().Desc().stages[1].format == ArtifactFormat::kSpirv,
	    "gl.store Resolve takes the artifacts of the format asked for" );
}

} // namespace

int main()
{
	testing::Checks checks;
	StoreClauses( checks );
	auto created = gl::Create( {} );
	if ( !checks.That( created.HasValue(), "gl.programs an OpenGL 4.5 core device is created" ) )
		return checks.Report();
	IRenderDevice2 &device = *created.Value();
	for ( const Program &program : kPrograms )
	{
		auto pipeline = Create( device, program, shaderlib::CoreArtifacts() );
		if ( !checks.That( pipeline.HasValue(),
		         std::string( "gl.programs " ) + program.name + " compiles and links" ) )
			std::printf( "  %s: %s\n", program.name, DescribeStatus( pipeline.Error().status ) );
	}
	// Negative control: an artifact cut short does not compile.
	shaderlib::ArtifactOverlay truncated( shaderlib::CoreArtifacts() );
	const shaderlib::ArtifactKey key{ "render/material/families/surface.frag",
	    std::string( shaderlib::CoreCompiler() ), ArtifactFormat::kGlsl450, 0 };
	const shaderlib::ShaderArtifact *whole = shaderlib::CoreArtifacts().Find( key );
	const bool cut =
	    whole && truncated.Replace( key,
	                 std::span<const std::byte>( whole->code ).first( whole->code.size() / 2 ) );
	auto broken = Create( device, kPrograms[0], truncated );
	checks.That( cut && !broken && broken.Error().status == DeviceStatus::kInvalidDescription,
	    "gl.programs a truncated artifact fails kInvalidDescription" );

	// G3: the passes resolve for the device's format.
	checks.That( pass::output::OutputRenderer::Create( device, Format::kRGBA8Unorm ).HasValue(),
	    "gl.passes the output pass creates its program" );
	checks.That( pass::skinning::SkinningKernel::Create( device ).HasValue(),
	    "gl.passes the skinning kernel creates its program" );
	checks.That( pass::lights::ClusterKernel::Create( device ).HasValue(),
	    "gl.passes the cluster assignment kernel creates its program" );
	auto seeded = pass::skinning::SkinningKernel::Create(
	    device, rendertest::skinning::spirv::kSkinBoneIndexError );
	checks.That( !seeded && seeded.Error() == pass::skinning::SkinningStatus::kDevice,
	    "gl.passes a seeded SPIR-V kernel is refused on GL, not run unseeded" );
	(void)device.WaitIdle();
	return checks.Report();
}
