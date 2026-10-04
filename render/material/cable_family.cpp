//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: Cable / SplineRope's expanded ribbon surface.
//=============================================================================//
#include "render/material/cable_family.h"
#include "family_program.h"

#include <array>
#include <string_view>

namespace render::material
{
UnlitClaim ClaimCable( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "cable" )
	{
		claim.reason = "the block is of family " + block.Family().desc.name;
		return claim;
	}
	// Cable_DX9 declares MINLIGHT/MAXLIGHT but neither shader reads them.
	// Illumination is already in the captured vertex colors.
	constexpr std::array<std::string_view, 9> keys = { "basetexture", "bumpmap", "translucent",
	    "alphatest", "model", "nofog", "nocull", "minlight", "maxlight" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the cable point does not draw " + *unread;
		return claim;
	}
	if ( !detail::TextureBound( block, "basetexture" ) ||
	     !detail::TextureBound( block, "bumpmap" ) )
	{
		claim.reason = "Cable needs its base texture and normal texture";
		return claim;
	}
	const bool translucent = detail::ReadFlag( block, "translucent" );
	const bool alphaTest = detail::ReadFlag( block, "alphatest" );
	claim.blend = translucent ? device::BlendMode::kAlpha : device::BlendMode::kOpaque;
	claim.alphaWrite = !translucent && !alphaTest;
	claim.cable = true;
	claim.constants.flags[1] = alphaTest ? 1.0f : 0.0f;
	claim.constants.flags[2] = 0.5f; // Cable's default fixed-function alpha test.
	claim.constants.surfaceControls[0] = detail::ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}
}
