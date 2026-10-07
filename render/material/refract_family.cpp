//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Refract_DX90 subset drawn by the shared surface program.
//
//=============================================================================//

#include "render/material/refract_family.h"

#include "family_program.h"

#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <string_view>

namespace render::material
{

RefractClaim ClaimRefract( const ParameterBlock &block, bool sceneColorAvailable,
    bool usesNativeProbe, bool nativeReflectionProbes )
{
	RefractClaim claim;
	if ( block.Family().desc.name != "refract" )
	{
		claim.reason = "the block is not Refract";
		return claim;
	}
	// Portal 2's $localrefract refracts the base texture in texture space; its
	// base coordinates take $basetexturetransform. The screen-space points
	// (scene color, or a base texture read at the warped screen position) have
	// no base coordinates for a transform to move.
	const bool local = detail::ReadFlag( block, "localrefract" );
	constexpr std::array<std::string_view, 20> kClaimed = { "model", "translucent", "basetexture",
	    "normalmap", "refractamount", "refracttint", "bluramount", "fadeoutonsilhouette", "envmap",
	    "envmaptint", "envmapcontrast", "envmapsaturation", "refracttinttexture", "nocull",
	    "bumptransform", "bumpframe", "vertexcolor", "vertexalpha", "vertexcolormodulate",
	    "nofog" };
	constexpr std::array<std::string_view, 17> kLocalClaimed = { "model", "translucent",
	    "basetexture", "normalmap", "refractamount", "refracttint", "bluramount", "envmap",
	    "envmaptint", "envmapcontrast", "envmapsaturation", "basetexturetransform", "localrefract",
	    "localrefractdepth", "nocull", "bumptransform", "bumpframe" };
	if ( const std::optional<std::string> unclaimed = detail::UnclaimedParameter(
	         block, local ? std::span<const std::string_view>( kLocalClaimed )
	                      : std::span<const std::string_view>( kClaimed ) ) )
	{
		claim.reason = ( local ? "the local Refract point does not draw "
		                       : "the Refract point does not draw " ) +
		               *unclaimed;
		return claim;
	}
	claim.local = local;
	claim.translucent = detail::ReadFlag( block, "translucent" );
	claim.baseTexture = detail::TextureBound( block, "basetexture" );
	if ( local && !claim.baseTexture )
	{
		claim.reason = "$localrefract needs $basetexture";
		return claim;
	}
	claim.sceneColor = !claim.baseTexture;
	if ( claim.sceneColor && !sceneColorAvailable )
	{
		claim.reason = "Refract needs the view's linear scene color";
		return claim;
	}
	if ( !detail::TextureBound( block, "normalmap" ) )
	{
		claim.reason = "Refract needs $normalmap";
		return claim;
	}
	claim.envmap = detail::TextureBound( block, "envmap" );
	// An env_cubemap reference is supplied by the map's RPRB. A named cube
	// remains the authored material image and is bound through the material.
	claim.nativeProbe = claim.envmap && usesNativeProbe;
	if ( claim.nativeProbe && !nativeReflectionProbes )
	{
		claim.reason = "$envmap needs the stage's native reflection probes";
		return claim;
	}
	const float amount = detail::ReadParameter( block, "refractamount" );
	const float blur = detail::ReadParameter( block, "bluramount" );
	const float contrast = detail::ReadParameter( block, "envmapcontrast" );
	// A negative amount warps the other way (refract_ps2x multiplies the
	// normal's offset by it): Portal 2's water beams use -.6.
	if ( !std::isfinite( amount ) || amount < -2.0f || amount > 2.0f || !std::isfinite( blur ) ||
	     blur < 0.0f || !std::isfinite( contrast ) || contrast < 0.0f || contrast > 1.0f )
	{
		claim.reason = "invalid Refract amount, blur or envmap contrast";
		return claim;
	}
	claim.constants.transmission[0] = amount;
	// Refract_DX90 reads $bluramount as an integer and clamps it to 0 or 1.
	claim.constants.transmission[1] = blur >= 1.0f ? 1.0f : 0.0f;
	claim.constants.transmission[2] = 1.0f; // Refract point, not PBR thin glass
	// The normal map's (and tint texture's) coordinates: $bumptransform.
	for ( int i = 0; i < 8; ++i )
		claim.constants.texture2Transform[i] = detail::ReadParameter( block, "bumptransform", i );
	if ( local )
	{
		// LOCALREFRACT warps no screen coordinate and has no blur; the amount's
		// slot carries $localrefractdepth (refract_ps2x c7.z).
		const float depth = detail::ReadParameter( block, "localrefractdepth" );
		if ( !std::isfinite( depth ) || depth < 0.0f || depth > 1.0f )
		{
			claim.reason = "invalid $localrefractdepth";
			return claim;
		}
		claim.constants.transmission[0] = depth;
		claim.constants.transmission[1] = 0.0f;
		claim.constants.transmission[2] = 2.0f; // the local Refract point
		for ( int i = 0; i < 8; ++i )
			claim.constants.baseTransform[i] =
			    detail::ReadParameter( block, "basetexturetransform", i );
	}
	claim.constants.transmission[3] =
	    detail::ReadFlag( block, "fadeoutonsilhouette" ) ? 1.0f : 0.0f;
	claim.constants.meshModes[0] = claim.baseTexture ? 1.0f : 0.0f;
	claim.constants.meshModes[1] = claim.envmap ? 1.0f : 0.0f;
	claim.tintTexture = detail::TextureBound( block, "refracttinttexture" );
	claim.constants.meshModes[2] = claim.tintTexture ? 1.0f : 0.0f;
	claim.constants.meshProbeColor[0] = contrast;
	// Vertex color (warp particles): flags.x tints the refraction by the
	// vertex color, state.w scales the warp and its tint by the vertex alpha;
	// state.y decodes the gamma vertex color per vertex.
	const bool modulate = detail::ReadFlag( block, "vertexcolormodulate" );
	const bool vertexColor = modulate || detail::ReadFlag( block, "vertexcolor" );
	const bool vertexAlpha = modulate || detail::ReadFlag( block, "vertexalpha" );
	// The scene snapshot is already fogged; $nofog only matters for a base
	// texture's own radiance, which the screen-space point fogs.
	if ( detail::ReadFlag( block, "nofog" ) && claim.baseTexture )
	{
		claim.reason = "$nofog on a Refract base texture is not drawn";
		return claim;
	}
	claim.constants.flags[0] = vertexColor ? 1.0f : 0.0f;
	claim.constants.state[1] = vertexColor ? 1.0f : 0.0f;
	claim.constants.state[3] = vertexAlpha ? 1.0f : 0.0f;
	for ( int c = 0; c < 3; ++c )
	{
		const float tint = detail::ReadParameter( block, "refracttint", c );
		const float envTint = detail::ReadParameter( block, "envmaptint", c );
		const float saturation = detail::ReadParameter( block, "envmapsaturation", c );
		if ( !std::isfinite( tint ) || !std::isfinite( envTint ) || !std::isfinite( saturation ) ||
		     tint < 0.0f || envTint < 0.0f || saturation < 0.0f )
		{
			claim.reason = "invalid Refract tint or envmap saturation";
			return claim;
		}
		claim.constants.tint[c] = detail::SourceGammaToLinear( tint );
		claim.constants.envTint[c] = detail::SourceGammaToLinear( envTint );
		claim.constants.envSaturation[c] = saturation;
	}
	claim.claimed = true;
	return claim;
}

} // namespace render::material
