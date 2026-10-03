//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-library (RFC 0016): the artifact store,
//			permutation keys and pipeline recipes.
//
//=============================================================================//

#include "render/shaderlib/artifact_source.h"
#include "render/shaderlib/permutation.h"
#include "render/shaderlib/pipeline_recipe.h"

#include <algorithm>
#include <utility>

namespace render::shaderlib
{

namespace
{

foundation::Unexpected<ShaderLibraryError> Fail( ShaderLibraryStatus status, ArtifactKey key = {} )
{
	return foundation::MakeUnexpected( ShaderLibraryError{ status, std::move( key ) } );
}

} // namespace

foundation::Expected<void, ShaderLibraryError> ArtifactStore::Add( ShaderArtifact artifact )
{
	if ( artifact.key.source.empty() || artifact.key.compiler.empty() || artifact.code.empty() )
		return Fail( ShaderLibraryStatus::kInvalidArtifact, artifact.key );
	if ( Find( artifact.key ) )
		return Fail( ShaderLibraryStatus::kDuplicateArtifact, artifact.key );
	m_Artifacts.push_back( std::make_unique<ShaderArtifact>( std::move( artifact ) ) );
	++m_Revision;
	return {};
}

const ShaderArtifact *ArtifactStore::Find( const ArtifactKey &key ) const
{
	for ( const std::unique_ptr<ShaderArtifact> &artifact : m_Artifacts )
	{
		if ( artifact->key == key )
			return artifact.get();
	}
	return nullptr;
}

foundation::Expected<std::uint64_t, ShaderLibraryError> PermutationKey(
    std::span<const PermutationAxis> axes, std::span<const std::uint32_t> values )
{
	if ( axes.size() != values.size() )
		return Fail( ShaderLibraryStatus::kPermutationOutOfRange );
	std::uint64_t key = 0;
	for ( std::size_t i = axes.size(); i-- > 0; )
	{
		if ( axes[i].values == 0 || values[i] >= axes[i].values )
			return Fail( ShaderLibraryStatus::kPermutationOutOfRange );
		key = key * axes[i].values + values[i];
	}
	return key;
}

std::uint64_t PermutationCount( std::span<const PermutationAxis> axes )
{
	std::uint64_t count = 1;
	for ( const PermutationAxis &axis : axes )
		count *= axis.values;
	return count;
}

foundation::Expected<ResolvedPipeline, ShaderLibraryError> Resolve( const PipelineRecipe &recipe,
    const IShaderArtifactSource &source, device::ArtifactFormat format )
{
	ResolvedPipeline resolved;
	resolved.m_Recipe = recipe;
	for ( const std::string &path : recipe.sources )
	{
		ArtifactKey key{ path, recipe.compiler, format, recipe.permutation };
		const ShaderArtifact *artifact = source.Find( key );
		if ( !artifact )
			return Fail( ShaderLibraryStatus::kMissingArtifact, std::move( key ) );
		resolved.m_Stages.push_back( artifact->View() );
	}
	return resolved;
}

device::PipelineDesc ResolvedPipeline::Desc() const
{
	const PipelineRecipe &r = m_Recipe;
	device::PipelineDesc desc;
	desc.kind = r.kind;
	desc.stages = m_Stages;
	for ( const auto &stage : m_Stages )
		desc.drawConstantBytes = std::max( desc.drawConstantBytes, stage.drawConstantBytes );
	desc.layouts = r.layouts;
	desc.topology = r.topology;
	desc.raster = r.raster;
	desc.depthStencil = r.depthStencil;
	desc.colorFormats = r.colorFormats;
	desc.blends = r.blends;
	desc.depthFormat = r.depthFormat;
	desc.sampleCount = r.sampleCount;
	desc.debugName = r.debugName;
	return desc;
}

} // namespace render::shaderlib
