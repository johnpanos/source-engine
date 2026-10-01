//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `vertexlit` material family's program (RFC 0016 K4); see
//			vertexlit_family.h.
//
//=============================================================================//

#include "render/material/vertexlit_family.h"

#include "family_program.h"

#include <algorithm>
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

// One modernized dielectric point. Other variables wait for their surface
// terms instead of being dropped during import.
constexpr std::array<std::string_view, 21> kMeshClaimed = { "basetexture", "color", "bumpmap",
    "phong", "phongexponent", "phongboost", "phongtint", "phongfresnelranges", "model",
    "ignore_alpha_modulation", "selfillum", "rimlightexponent", "rimlightboost",
    "selfillumfresnelminmaxexp", "depthblendscale", "envmapsaturation", "envmapcontrast",
    "envmaptint", "envmapfresnel", "alphatest", "alphatestreference" };

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

VertexLitMeshClaim ClaimVertexLitMesh( const ParameterBlock &block )
{
	VertexLitMeshClaim claim;
	if ( block.Family().desc.name != "vertexlit" )
	{
		claim.reason = "the block is not VertexLitGeneric";
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kMeshClaimed ) )
	{
		claim.reason = "the modern mesh point does not draw " + *unclaimed;
		return claim;
	}
	const bool phong = ReadFlag( block, "phong" );
	const float exponent = ReadParameter( block, "phongexponent" );
	const float boost = ReadParameter( block, "phongboost" );
	if ( !std::isfinite( exponent ) || exponent < 0.0f )
	{
		claim.reason = "$phongexponent must be finite and nonnegative";
		return claim;
	}
	if ( !std::isfinite( boost ) || boost < 0.0f )
	{
		claim.reason = "$phongboost must be finite and nonnegative";
		return claim;
	}
	// $phongexponent is inert in VertexLitGeneric when $phong is off. The
	// imported shader can still carry a nondefault value in that case.
	const bool selfIllum = ReadFlag( block, "selfillum" );
	// The min/max/exp parameter is inert without $selfillumfresnel. The
	// legacy material importer includes its zero-valued shader default even
	// on an ordinary self-illuminated door. A nondefault enable switch stays
	// outside the mapped mesh subset and is refused by the resolver.
	claim.normalMap = detail::TextureBound( block, "bumpmap" );
	claim.selfIllum = selfIllum;
	claim.alphaTest = ReadFlag( block, "alphatest" );
	claim.constants.flags[1] = claim.alphaTest ? 1.0f : 0.0f;
	claim.constants.flags[2] = detail::AlphaTestReference( block );
	for ( int c = 0; c < 3; ++c )
		claim.constants.tint[c] = SourceGammaToLinear( ReadParameter( block, "color", c ) );
	claim.constants.pbrFactors[0] = 0.0f; // dielectric
	claim.constants.pbrFactors[1] =
	    phong ? std::clamp( std::sqrt( 2.0f / ( exponent + 2.0f ) ), 0.02f, 1.0f ) : 0.55f;
	claim.constants.pbrFactors[2] = 1.0f;
	claim.constants.pbrFactors[3] = 0.0f; // the material has no MRAO texture
	// Dielectric F0 is a neutral 0.04 in the PBR point. Phong boost and tint
	// become its bounded colored specular reflectance.
	for ( int c = 0; c < 3; ++c )
	{
		claim.constants.envTint[c] = std::clamp(
		    SourceGammaToLinear( ReadParameter( block, "phongtint", c ) ) * boost, 0.0f, 25.0f );
		const float range = ReadParameter( block, "phongfresnelranges", c );
		if ( !std::isfinite( range ) || range < 0.0f )
		{
			claim.reason = "$phongfresnelranges must be finite and nonnegative";
			return claim;
		}
		claim.constants.envContrast[c] = range;
	}
	claim.constants.envContrast[3] = 1.0f;
	claim.claimed = true;
	return claim;
}

} // namespace render::material
