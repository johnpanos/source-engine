//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-library: the core's build-time artifact store; see
//			public/render/shaderlib/core_artifacts.h.
//
//=============================================================================//

#include "render/shaderlib/core_artifacts.h"
#include "spv/core_artifact_table.h"

namespace render::shaderlib
{

std::string_view CoreCompiler()
{
	return generated::kCompiler;
}

const IShaderArtifactSource &CoreArtifacts()
{
	static const ArtifactStore store = []
	{
		ArtifactStore built;
		for ( const generated::CoreArtifact &entry : generated::kCoreArtifacts )
		{
			ShaderArtifact artifact;
			artifact.key = { entry.source, std::string( generated::kCompiler ), entry.format, 0 };
			artifact.stage = entry.stage;
			artifact.code.assign( entry.code.begin(), entry.code.end() );
			artifact.bindings.assign( entry.bindings.begin(), entry.bindings.end() );
			artifact.drawConstantBytes = entry.drawConstantBytes;
			// The build writes each (source, format) once; a duplicate would be a
			// generator defect, which the store's own rule reports as absent.
			(void)built.Add( std::move( artifact ) );
		}
		return built;
	}();
	return store;
}

PipelineRecipe CoreRecipe(
    std::initializer_list<std::string_view> sources, device::PipelineKind kind )
{
	PipelineRecipe recipe;
	recipe.kind = kind;
	for ( std::string_view source : sources )
		recipe.sources.emplace_back( source );
	recipe.compiler = std::string( generated::kCompiler );
	return recipe;
}

bool ArtifactOverlay::ReplaceSpirv( std::string_view source, std::span<const std::uint32_t> words,
    device::ArtifactFormat deviceFormat )
{
	if ( deviceFormat != device::ArtifactFormat::kSpirv )
		return false;
	return Replace( { std::string( source ), std::string( generated::kCompiler ),
	                    device::ArtifactFormat::kSpirv, 0 },
	    std::as_bytes( words ) );
}

bool ArtifactOverlay::Replace( const ArtifactKey &key, std::span<const std::byte> code )
{
	const ShaderArtifact *base = m_Base.Find( key );
	if ( !base || code.empty() )
		return false;
	ShaderArtifact replaced = *base;
	replaced.code.assign( code.begin(), code.end() );
	return m_Replaced.Add( std::move( replaced ) ).HasValue();
}

const ShaderArtifact *ArtifactOverlay::Find( const ArtifactKey &key ) const
{
	if ( const ShaderArtifact *replaced = m_Replaced.Find( key ) )
		return replaced;
	return m_Base.Find( key );
}

} // namespace render::shaderlib
