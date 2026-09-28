//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What every material family's program shares (RFC 0016 K4): the
//			claim rule (a family draws a material only when every parameter
//			outside its claimed subset holds its schema default), reading
//			parameters back from a block, and Source's gamma conversion for
//			material colors. Private to render.material.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_FAMILY_PROGRAM_H
#define RENDER_MATERIAL_FAMILY_PROGRAM_H

#include "render/material/parameter_block.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace render::material::detail
{

// The first parameter outside `claimed` that the block sets away from its
// schema default (a texture counts when bound), as "$name"; nullopt when the
// family may draw the material.
std::optional<std::string> UnclaimedParameter(
    const ParameterBlock &block, std::span<const std::string_view> claimed );

// A parameter's value as the block stores it (component of a vector; an int
// or bool as a float); 0 when the schema has no such parameter.
float ReadParameter(
    const ParameterBlock &block, std::string_view name, std::size_t component = 0 );
bool ReadFlag( const ParameterBlock &block, std::string_view name );

// Source's GammaToLinear for a material color component (mathlib
// color_conversion.cpp): values above one pass unchanged, values from 0.95
// are one, the rest go through the 256-entry pow(2.2) table, indexed by
// RoundFloatToInt (half to even).
float SourceGammaToLinear( float gamma );

} // namespace render::material::detail

#endif // RENDER_MATERIAL_FAMILY_PROGRAM_H
