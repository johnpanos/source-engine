//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-library (RFC 0016): the artifact store rejects
//			duplicates and empty artifacts, permutation keys are a bijection,
//			and a recipe resolves to the device's format or fails naming the
//			missing artifact (never another format). A resolved recipe creates
//			a pipeline on the null device.
//
//=============================================================================//

#include "../device/test_shaders.h"
#include "render/device/null/provider.h"
#include "render/shaderlib/permutation.h"
#include "render/shaderlib/pipeline_recipe.h"
#include "testing/checks.h"

#include <set>

namespace
{

using namespace render;
using namespace render::shaderlib;

ShaderArtifact Artifact( const char *source, device::ShaderStage stage,
    device::ArtifactFormat format, const std::uint32_t *words, std::size_t count )
{
	ShaderArtifact artifact;
	artifact.key = { source, "glslc-2026.1", format, 0 };
	artifact.stage = stage;
	const auto *bytes = reinterpret_cast<const std::byte *>( words );
	artifact.code.assign( bytes, bytes + count * sizeof( std::uint32_t ) );
	return artifact;
}

} // namespace

int main()
{
	testing::Checks checks;
	ArtifactStore store;
	checks.That( store
	                 .Add( Artifact( "fullscreen.vert", device::ShaderStage::kVertex,
	                     device::ArtifactFormat::kSpirv, rendertest::shaders::kFullScreenVertex,
	                     std::size( rendertest::shaders::kFullScreenVertex ) ) )
	                 .HasValue(),
	    "S1.an-artifact-is-added" );
	const std::uint64_t revision = store.Revision();
	auto duplicate = store.Add( Artifact( "fullscreen.vert", device::ShaderStage::kVertex,
	    device::ArtifactFormat::kSpirv, rendertest::shaders::kFullScreenVertex, 4 ) );
	checks.That( !duplicate && duplicate.Error().status == ShaderLibraryStatus::kDuplicateArtifact,
	    "S1.a-duplicate-key-is-rejected" );
	checks.Equal( store.Revision(), revision, "S1.a-rejected-add-keeps-the-revision" );
	ShaderArtifact empty;
	empty.key = { "empty.frag", "glslc-2026.1", device::ArtifactFormat::kSpirv, 0 };
	checks.That( !store.Add( empty ), "S1.an-empty-artifact-is-rejected" );

	const PermutationAxis axes[] = { { "FOG", 2 }, { "LIGHTS", 3 }, { "ALPHA", 2 } };
	std::set<std::uint64_t> keys;
	bool inRange = true;
	for ( std::uint32_t a = 0; a < 2; ++a )
	{
		for ( std::uint32_t b = 0; b < 3; ++b )
		{
			for ( std::uint32_t c = 0; c < 2; ++c )
			{
				const std::uint32_t values[] = { a, b, c };
				auto key = PermutationKey( axes, values );
				inRange &= key.HasValue() && key.Value() < PermutationCount( axes );
				if ( key )
					keys.insert( key.Value() );
			}
		}
	}
	checks.That( inRange && keys.size() == PermutationCount( axes ),
	    "S2.every-combination-has-its-own-key" );
	const std::uint32_t outside[] = { 0, 3, 0 };
	checks.That( !PermutationKey( axes, outside ), "S2.an-out-of-range-value-fails" );

	PipelineRecipe recipe;
	recipe.sources = { "fullscreen.vert" };
	recipe.compiler = "glslc-2026.1";
	recipe.colorFormats = { device::Format::kRGBA8Unorm };
	auto spirv = Resolve( recipe, store, device::ArtifactFormat::kSpirv );
	checks.That( spirv.HasValue(), "S3.a-recipe-resolves-for-its-format" );
	auto gl = Resolve( recipe, store, device::ArtifactFormat::kGlsl450 );
	checks.That( !gl && gl.Error().status == ShaderLibraryStatus::kMissingArtifact &&
	                 gl.Error().key.source == "fullscreen.vert" &&
	                 gl.Error().key.format == device::ArtifactFormat::kGlsl450,
	    "S3.a-missing-format-fails-naming-the-artifact" );

	auto device = device::null::Create( {} ).Value();
	if ( spirv )
	{
		ResolvedPipeline moved = std::move( spirv ).Value();
		checks.That( device->CreatePipeline( moved.Desc() ).HasValue(),
		    "S3.a-resolved-recipe-creates-a-pipeline-after-a-move" );
	}
	// Reflected draw constants must survive recipe resolution and a move.
	// The null provider checks the range against every stage independently.
	auto constants = Artifact( "constants.vert", device::ShaderStage::kVertex,
	    device::ArtifactFormat::kSpirv, rendertest::shaders::kFullScreenVertex,
	    std::size( rendertest::shaders::kFullScreenVertex ) );
	constants.drawConstantBytes = 32;
	checks.That( store.Add( std::move( constants ) ).HasValue(), "S3.constant-artifact" );
	recipe.sources = { "constants.vert" };
	auto constantRecipe = Resolve( recipe, store, device::ArtifactFormat::kSpirv );
	checks.That( constantRecipe.HasValue(), "S3.constant-recipe" );
	if ( constantRecipe )
	{
		ResolvedPipeline moved = std::move( constantRecipe ).Value();
		auto desc = moved.Desc();
		checks.Equal( desc.drawConstantBytes, 32u, "S3.reflected-constant-range" );
		checks.That( device->CreatePipeline( desc ).HasValue(), "S3.constant-pipeline" );
		desc.drawConstantBytes = 0;
		checks.That( !device->CreatePipeline( desc ), "S3.missing-constant-range-rejected" );
	}
	return checks.Report();
}
