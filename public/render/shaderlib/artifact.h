//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.shader-artifacts.v1 artifacts (RFC 0016 "Shader
//			artifacts"). One GLSL source becomes one artifact per target format
//			and permutation, keyed by source, compiler identity, format and
//			permutation, with the bindings its reflection reports.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_ARTIFACT_H
#define RENDER_SHADERLIB_ARTIFACT_H

#include "render/device/pipeline.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace render::shaderlib
{

struct ArtifactKey
{
	std::string source;   // source path relative to the repository root
	std::string compiler; // pinned compiler identity, e.g. "glslc-2026.1"
	device::ArtifactFormat format = device::ArtifactFormat::kSpirv;
	std::uint64_t permutation = 0;

	friend bool operator==( const ArtifactKey &, const ArtifactKey & ) = default;
};

struct ShaderArtifact
{
	ArtifactKey key;
	device::ShaderStage stage = device::ShaderStage::kVertex;
	std::string entryPoint = "main";
	std::vector<std::byte> code;
	std::vector<device::ReflectedBinding> bindings;

	// A view for PipelineDesc; valid while this artifact lives.
	device::ShaderArtifactView View() const
	{
		return { stage, key.format, code, entryPoint, bindings };
	}
};

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_ARTIFACT_H
