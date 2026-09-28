//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shader permutation keys (RFC 0016). A permutation is one value
//			per declared axis; its key is the mixed-radix number of the values,
//			so every combination has exactly one key.
//
//=============================================================================//

#ifndef RENDER_SHADERLIB_PERMUTATION_H
#define RENDER_SHADERLIB_PERMUTATION_H

#include "foundation/expected.h"
#include "render/shaderlib/artifact_source.h"

#include <cstdint>
#include <span>
#include <string>

namespace render::shaderlib
{

struct PermutationAxis
{
	std::string name;
	std::uint32_t values = 2; // a boolean axis has two
};

foundation::Expected<std::uint64_t, ShaderLibraryError> PermutationKey(
    std::span<const PermutationAxis> axes, std::span<const std::uint32_t> values );
// The number of distinct permutations of axes.
std::uint64_t PermutationCount( std::span<const PermutationAxis> axes );

} // namespace render::shaderlib

#endif // RENDER_SHADERLIB_PERMUTATION_H
