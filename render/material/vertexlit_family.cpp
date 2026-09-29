//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `vertexlit` material family's program (RFC 0016 K4); see
//			vertexlit_family.h.
//
//=============================================================================//

#include "render/material/vertexlit_family.h"

#include "family_program.h"

#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadFlag;
using detail::ReadParameter;
using detail::SourceGammaToLinear;

// The parameters the family draws, and the ones the caller owns or the port
// ignores ($vertexcolor, and $vertexalpha's vertex data).
constexpr std::array<std::string_view, 12> kClaimed = { "basetexture", "color", "alpha",
    "alphatest", "alphatestreference", "translucent", "halflambert", "vertexcolor", "vertexalpha",
    "model", "nofog", "nocull" };

} // namespace

VertexLitClaim ClaimVertexLit( const ParameterBlock &block )
{
	VertexLitClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "vertexlit" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const float alpha = ReadParameter( block, "alpha" );
	const bool alphaTest = ReadFlag( block, "alphatest" );
	// EvaluateBlendRequirements: constant alpha modulation or vertex alpha.
	const bool alphaBlended =
	    ReadFlag( block, "translucent" ) || ReadFlag( block, "vertexalpha" ) || alpha < 1.0f;
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !alphaTest;
	SurfaceConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
		constants.tint[c] = SourceGammaToLinear( ReadParameter( block, "color", c ) );
	constants.tint[3] = alpha;
#if defined( RENDER_MATERIAL_VERTEXLIT_SEEDED_IGNORE_HALF_LAMBERT )
	claim.halfLambert = false;
#else
	claim.halfLambert = ReadFlag( block, "halflambert" );
#endif
	constants.flags[1] = alphaTest ? 1.0f : 0.0f;
	// The port's alpha test holds its reference as a byte; 0.7 when unset.
	constants.flags[2] = detail::AlphaTestReference( block );
	claim.claimed = true;
	return claim;
}

} // namespace render::material
