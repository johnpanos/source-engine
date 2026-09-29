//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The core's build-time artifact store (RFC 0016 "Shader
//			artifacts", K10): every core program's SPIR-V and GLSL 4.50, as
//			the build generates them with the pinned toolchain
//			(tools/render/shader_artifacts.py writes spv/core_artifact_table.h
//			from the generated headers), with each artifact's reflected
//			bindings and draw-constant bytes. Keyed by the program's GLSL
//			source (repository-relative), CoreCompiler(), the target format
//			and permutation 0.
//
//			Consumers resolve their programs through it with
//			Resolve( recipe, CoreArtifacts(), device.Facts().artifactFormat ):
//			the format comes from the device, never from the consumer.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_CORE_ARTIFACTS_H
#define RENDER_SHADERLIB_CORE_ARTIFACTS_H

#include "render/shaderlib/artifact_source.h"
#include "render/shaderlib/pipeline_recipe.h"

#include <cstdint>
#include <initializer_list>
#include <span>
#include <string_view>

namespace render::shaderlib
{

// The store, built once on first use; it lives for the process.
const IShaderArtifactSource &CoreArtifacts();

// The pinned toolchain's identity, the compiler of every core artifact key.
std::string_view CoreCompiler();

// A recipe for the core program whose stages are `sources` (repository-
// relative GLSL paths, one per stage), compiled by CoreCompiler(); the caller
// sets its fixed-function state.
PipelineRecipe CoreRecipe( std::initializer_list<std::string_view> sources,
    device::PipelineKind kind = device::PipelineKind::kGraphics );

// A source that serves `base` with some artifacts' code replaced: the
// suites' seeded variants of a core program. It must not outlive `base`.
class ArtifactOverlay final : public IShaderArtifactSource
{
public:
	explicit ArtifactOverlay( const IShaderArtifactSource &base ) : m_Base( base ) {}

	// Replaces the code of base's artifact at key, keeping its reflection;
	// false when base has no such artifact.
	bool Replace( const ArtifactKey &key, std::span<const std::byte> code );
	// A suite's seeded SPIR-V for the core program stage `source`. Seeded
	// variants exist as SPIR-V only, so on a device of another format this
	// fails, and the pass never runs the unseeded program in their place.
	bool ReplaceSpirv( std::string_view source, std::span<const std::uint32_t> words,
	    device::ArtifactFormat deviceFormat );

	const ShaderArtifact *Find( const ArtifactKey &key ) const override;
	std::uint64_t Revision() const override { return m_Base.Revision() + m_Replaced.Revision(); }

private:
	const IShaderArtifactSource &m_Base;
	ArtifactStore m_Replaced;
};

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_CORE_ARTIFACTS_H
