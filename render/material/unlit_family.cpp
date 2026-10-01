//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4); see
//			unlit_family.h.
//
//=============================================================================//

#include "render/material/unlit_family.h"

#include "family_program.h"

#include <array>
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

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 12> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "additive",
    "model", "nofog", "nocull" };

} // namespace

UnlitClaim ClaimUnlit( const ParameterBlock &block )
{
	UnlitClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "unlit" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	const bool translucent = ReadFlag( block, "translucent" );
	const bool additive = ReadFlag( block, "additive" );
	if ( translucent && additive )
	{
		claim.reason = "$translucent with $additive blends src-alpha/one, which the port lacks";
		return claim;
	}
	const bool alphaBlended = translucent || ReadFlag( block, "vertexalpha" );
	claim.blend =
	    additive ? BlendMode::kAdditive : ( alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque );
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	SurfaceConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
		constants.tint[c] = SourceGammaToLinear( ReadParameter( block, "color", c ) );
	constants.tint[3] = ReadParameter( block, "alpha" );
#if defined( RENDER_MATERIAL_UNLIT_SEEDED_IGNORE_VERTEX_COLOR )
	constants.flags[0] = 0.0f;
#else
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
#endif
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = detail::AlphaTestReference( block );
	constants.state[1] = 1.0f; // UnlitGeneric decodes vertex colors per vertex
	constants.state[3] = ReadFlag( block, "vertexalpha" ) ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimUnlitMesh( const ParameterBlock &block )
{
	// The ordinary unlit claim owns every shared parameter. The authored env
	// map is a model-only extension; the scene's RPRB supplies its radiance.
	ParameterBlock withoutProbe = block;
	const bool nativeProbe = detail::TextureBound( block, "envmap" );
	if ( nativeProbe )
		(void)withoutProbe.SetTexture( "envmap", {} );
	UnlitClaim claim = ClaimUnlit( withoutProbe );
	claim.nativeProbe = nativeProbe;
	return claim;
}

} // namespace render::material
