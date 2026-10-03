//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Pipeline recipes (RFC 0016). A recipe names shader sources, a
//			permutation and fixed-function state without naming a target
//			format; Resolve() finds the artifacts for the device's format and
//			yields a device::PipelineDesc. A missing artifact fails by key, it
//			never falls back to another format.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_PIPELINE_RECIPE_H
#define RENDER_SHADERLIB_PIPELINE_RECIPE_H

#include "foundation/expected.h"
#include "render/device/pipeline.h"
#include "render/shaderlib/artifact_source.h"

#include <string>
#include <vector>

namespace render::shaderlib
{

struct PipelineRecipe
{
	device::PipelineKind kind = device::PipelineKind::kGraphics;
	std::vector<std::string> sources; // one per stage
	std::string compiler;
	std::uint64_t permutation = 0;
	std::vector<device::BindGroupLayoutId> layouts;
	device::PrimitiveTopology topology = device::PrimitiveTopology::kTriangleList;
	device::RasterState raster;
	device::DepthStencilState depthStencil;
	std::vector<device::Format> colorFormats;
	std::vector<device::BlendMode> blends;
	device::Format depthFormat = device::Format::kUnknown;
	std::uint32_t sampleCount = 1;
	std::string debugName;
};

// Owns the arrays a PipelineDesc views. Desc() is built on each call, so a
// moved ResolvedPipeline never hands out views of its old storage. The draw
// constant range is the maximum reflected requirement of its resolved stages.
class ResolvedPipeline
{
public:
	device::PipelineDesc Desc() const;

private:
	friend foundation::Expected<ResolvedPipeline, ShaderLibraryError> Resolve(
	    const PipelineRecipe &, const IShaderArtifactSource &, device::ArtifactFormat );

	PipelineRecipe m_Recipe;
	std::vector<device::ShaderArtifactView> m_Stages;
};

foundation::Expected<ResolvedPipeline, ShaderLibraryError> Resolve( const PipelineRecipe &recipe,
    const IShaderArtifactSource &source, device::ArtifactFormat format );

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_PIPELINE_RECIPE_H
