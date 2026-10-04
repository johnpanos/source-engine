//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `unlit` material family's program (RFC 0016 K4); see
//			unlit_family.h.
//
//=============================================================================//

#include "render/material/unlit_family.h"

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

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 23> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "additive",
    "model", "nofog", "nocull", "texture2", "frame2", "texture2transform", "ignorez",
    "hdrcolorscale", "hdrbasetexture", "basetexturetransform", "decal", "frame", "depthblend",
    "depthblendscale" };

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
	// Gameplay fades (including area-portal covers) modulate $alpha even
	// when the authored material has no $translucent flag.
	const bool alphaBlended =
	    translucent || ReadFlag( block, "vertexalpha" ) || ReadParameter( block, "alpha" ) < 1.0f;
	claim.blend =
	    additive ? BlendMode::kAdditive : ( alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque );
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	claim.ignoreDepth = ReadFlag( block, "ignorez" );
	claim.depthBlend = ReadFlag( block, "depthblend" );
	const float depthScale = ReadParameter( block, "depthblendscale" );
	if ( claim.depthBlend && ( !std::isfinite( depthScale ) || depthScale <= 0.0f ) )
	{
		claim.reason = "$depthblend needs a finite positive $depthblendscale";
		return claim;
	}
	SurfaceConstants &constants = claim.constants;
	constants.surfaceControls[0] = ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
	constants.surfaceControls[1] = ReadParameter( block, "hdrcolorscale" );
	constants.surfaceControls[2] = claim.depthBlend ? 1.0f : 0.0f;
	constants.surfaceControls[3] = depthScale;
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
	for ( std::size_t i = 0; i < 8; ++i )
		constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	claim.twoTexture = detail::TextureBound( block, "texture2" );
	constants.meshModes[0] = claim.twoTexture ? 1.0f : 0.0f;
	if ( claim.twoTexture )
	{
		for ( std::size_t i = 0; i < 8; ++i )
			constants.texture2Transform[i] = ReadParameter( block, "texture2transform", i );
	}
	claim.claimed = true;
	return claim;
}

UnlitClaim ClaimDecalModulate( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "decal-modulate" )
	{
		claim.reason = "the block is of family " + block.Family().desc.name;
		return claim;
	}
	// The shader ignores modulation and vertex colors. Projection owns decalscale.
	constexpr std::string_view keys[] = { "basetexture", "frame", "decal", "decalscale",
	    "vertexcolor", "vertexalpha", "model", "nocull", "nofog" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the decal point does not draw " + *unread;
		return claim;
	}
	if ( !detail::TextureBound( block, "basetexture" ) )
	{
		claim.reason = "DecalModulate needs its multiplicative texture";
		return claim;
	}
	claim.blend = device::BlendMode::kModulate2x;
	claim.alphaWrite = false;
	claim.decalModulate = true;
	claim.constants.surfaceControls[0] = detail::ReadFlag( block, "nofog" ) ? 1.0f : 0.0f;
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
	if ( claim.claimed && claim.depthBlend )
	{
		claim.claimed = false;
		claim.reason = "$depthblend needs the unlit particle point, not the emissive model point";
	}
	claim.nativeProbe = nativeProbe;
	return claim;
}

} // namespace render::material
