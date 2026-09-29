//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-artifacts.gl (RFC 0016 K10, "Artifacts per target"
//			on the OpenGL adapter): every core program's GLSL 4.50 artifacts,
//			as the build generates them (spv/<stem>_glsl.h), compile and link
//			as pipelines on render.device.gl, with the adapter's reading of
//			the artifact form (combined-sampler names, the draw-constant
//			block's slot, the specialization line). A truncated artifact is
//			the negative control: it must fail kInvalidDescription.
//
//=============================================================================//

#include "render/device/gl/provider.h"
#include "spv/cluster_assign_glsl.h"
#include "spv/debug_glsl.h"
#include "spv/families_glsl.h"
#include "spv/lines_glsl.h"
#include "spv/output_glsl.h"
#include "spv/shadow_glsl.h"
#include "spv/skin_glsl.h"
#include "testing/checks.h"

#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace
{

using namespace render::device;

std::span<const std::byte> Code( std::string_view glsl )
{
	return std::as_bytes( std::span<const char>( glsl ) );
}

struct Program
{
	const char *name;
	std::string_view vertex;   // empty for a compute program
	std::string_view fragment; // may be empty
	std::string_view compute;
};

// Each vertex stage with the fragment stage it is drawn with.
const Program kPrograms[] = {
    { "surface-flat", render::material::glsl::kSurfaceFlatVertex,
        render::material::glsl::kSurfaceFragment, {} },
    { "surface-world", render::material::glsl::kSurfaceWorldVertex,
        render::material::glsl::kSurfaceFragment, {} },
    { "surface-model", render::material::glsl::kSurfaceModelVertex,
        render::material::glsl::kSurfaceFragment, {} },
    { "lines", render::pass::lines::glsl::kLinesVertex, render::pass::lines::glsl::kLinesFragment,
        {} },
    { "debug-hatch", render::pass::debug::glsl::kFullscreenVertex,
        render::pass::debug::glsl::kHatchFragment, {} },
    { "debug-tint", render::pass::debug::glsl::kFullscreenVertex,
        render::pass::debug::glsl::kTintFragment, {} },
    { "output", render::pass::output::glsl::kOutputVertex,
        render::pass::output::glsl::kOutputFragment, {} },
    { "shadow-depth", render::pass::shadows::glsl::kShadowDepthVertex, {}, {} },
    { "shadow-receiver", render::pass::shadows::glsl::kShadowReceiverVertex,
        render::pass::shadows::glsl::kShadowReceiverFragment, {} },
    { "skin", {}, {}, render::pass::skinning::glsl::kSkinCompute },
    { "cluster-assign", {}, {}, render::pass::lights::glsl::kClusterAssignCompute },
};

DeviceResult<PipelineId> Create( IRenderDevice2 &device, const Program &program )
{
	ShaderArtifactView stages[2];
	std::size_t count = 0;
	PipelineDesc desc;
	if ( !program.compute.empty() )
	{
		desc.kind = PipelineKind::kCompute;
		stages[count++] = {
		    ShaderStage::kCompute, ArtifactFormat::kGlsl450, Code( program.compute ), "main", {} };
	}
	else
	{
		stages[count++] = {
		    ShaderStage::kVertex, ArtifactFormat::kGlsl450, Code( program.vertex ), "main", {} };
		if ( !program.fragment.empty() )
			stages[count++] = { ShaderStage::kFragment, ArtifactFormat::kGlsl450,
			    Code( program.fragment ), "main", {} };
	}
	static const Format colors[] = { Format::kRGBA16Float };
	if ( desc.kind == PipelineKind::kGraphics )
	{
		// A target the program draws into: a color one, or depth alone.
		if ( program.fragment.empty() )
			desc.depthFormat = Format::kD32Float;
		else
			desc.colorFormats = colors;
	}
	desc.stages = std::span<const ShaderArtifactView>( stages, count );
	desc.debugName = program.name;
	return device.CreatePipeline( desc );
}

} // namespace

int main()
{
	testing::Checks checks;
	auto created = gl::Create( {} );
	if ( !checks.That( created.HasValue(), "gl.programs an OpenGL 4.5 core device is created" ) )
		return checks.Report();
	IRenderDevice2 &device = *created.Value();
	for ( const Program &program : kPrograms )
	{
		auto pipeline = Create( device, program );
		if ( !checks.That( pipeline.HasValue(),
		         std::string( "gl.programs " ) + program.name + " compiles and links" ) )
			std::printf( "  %s: %s\n", program.name, DescribeStatus( pipeline.Error().status ) );
	}
	// Negative control: an artifact cut short does not compile.
	const std::string_view whole = render::material::glsl::kSurfaceFragment;
	const Program truncated = { "truncated", render::material::glsl::kSurfaceFlatVertex,
	    whole.substr( 0, whole.size() / 2 ), {} };
	auto broken = Create( device, truncated );
	checks.That( !broken && broken.Error().status == DeviceStatus::kInvalidDescription,
	    "gl.programs a truncated artifact fails kInvalidDescription" );
	(void)device.WaitIdle();
	return checks.Report();
}
