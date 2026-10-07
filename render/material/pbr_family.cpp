//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `pbr` material family's program (RFC 0016 K4); see
//			pbr_family.h.
//
//=============================================================================//

#include "render/material/pbr_family.h"

#include "family_program.h"
#include "render/pbr_material_schema.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadParameter;

// The parameters the family draws, and $fallbackmaterial, which only other
// profiles read. $envmap is env_cubemap, the view's reflection probes, which
// the point always reads (the resolver refuses a named cube). The P2:CE keys (vmt_mapping.cpp's kPbrCompatKeys) follow;
// the parallax settings and $blendtintbymraoalpha are claimed because they
// are inert while $parallax is off and no $color2 is set (both refused).
constexpr std::array<std::string_view, 34> kClaimed = { "envmap", "basetexture", "mraotexture", "bumpmap",
    "emissiontexture", "emissionscale", "emissiononesided", "emissioncameraonly", "emissioncone",
    "emissionconeinner", "emissionconeouter", "emissionconeexponent", "alphatest",
    "alphatestreference", "fallbackmaterial", "transmission", "ior", "thickness", "translucent",
    "mraoscale", "envmaptint", "envmaplightscale", "basetexturetransform", "parallax",
    "parallaxdepth", "parallaxcenter", "parallaxdither", "parallaxscale",
    "blendtintbymraoalpha", "basetexturetransform2", "mraoscale2", "envmaptint2", "nocull",
    "srgbtint" };

bool TextureBound( const ParameterBlock &block, std::string_view name )
{
	const FamilySchema &family = block.Family();
	const std::optional<std::size_t> index = family.IndexOf( name );
	if ( !index || family.layout[*index].type != ParameterType::kTexture )
		return false;
	return block.Textures()[family.layout[*index].offset].IsValid();
}

} // namespace

PbrClaim ClaimPbr( const ParameterBlock &block, bool sceneColorAvailable )
{
	PbrClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "pbr" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	if ( !TextureBound( block, "basetexture" ) || !TextureBound( block, "mraotexture" ) )
	{
		claim.reason = "PBRMetalRough needs $basetexture and $mraotexture";
		return claim;
	}
	if ( detail::ReadFlag( block, "parallax" ) )
	{
		claim.reason = "$parallax (parallax occlusion mapping) is not drawn by the core";
		return claim;
	}
	// P2:CE's MRAO scale and probe tint: finite and nonnegative per channel.
	for ( int c = 0; c < 3; ++c )
	{
		const float scale = ReadParameter( block, "mraoscale", c );
		const float tint = ReadParameter( block, "envmaptint", c );
		if ( !std::isfinite( scale ) || scale < 0.0f || !std::isfinite( tint ) || tint < 0.0f )
		{
			claim.reason = "$mraoscale and $envmaptint need finite nonnegative values";
			return claim;
		}
		claim.constants.mraoScale[c] = scale;
		claim.constants.envSaturation[c] = detail::SourceGammaToLinear( tint );
		const float baseTint = ReadParameter( block, "srgbtint", c );
		if ( !std::isfinite( baseTint ) || baseTint < 0.0f )
		{
			claim.reason = "$srgbtint needs finite nonnegative values";
			return claim;
		}
		claim.constants.tint[c] = detail::SourceGammaToLinear( baseTint );
	}
	const float lightScale = ReadParameter( block, "envmaplightscale" );
	if ( !std::isfinite( lightScale ) || lightScale < 0.0f || lightScale > 1.0f )
	{
		claim.reason = "$envmaplightscale needs a [0, 1] weight";
		return claim;
	}
	// Portal 2's light-scaled reflection with its default range [0 1], as the
	// lightmapped and vertexlit points read it.
	claim.constants.envLightScale[0] = 0.0f;
	claim.constants.envLightScale[1] = 1.0f;
	claim.constants.envLightScale[2] = lightScale;
	for ( std::size_t i = 0; i < 8; ++i )
		claim.constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	claim.normalMap = TextureBound( block, "bumpmap" );
	claim.emission = TextureBound( block, "emissiontexture" );
	const bool oneSidedEmission = detail::ReadFlag( block, "emissiononesided" );
	const bool cameraOnlyEmission = detail::ReadFlag( block, "emissioncameraonly" );
	if ( oneSidedEmission && !claim.emission )
	{
		claim.reason = "$emissiononesided needs $emissiontexture";
		return claim;
	}
	if ( cameraOnlyEmission && !claim.emission )
	{
		claim.reason = "$emissioncameraonly needs $emissiontexture";
		return claim;
	}
	const bool emissionCone = detail::ReadFlag( block, "emissioncone" );
	const float coneInner = ReadParameter( block, "emissionconeinner" );
	const float coneOuter = ReadParameter( block, "emissionconeouter" );
	const float coneExponent = ReadParameter( block, "emissionconeexponent" );
	if ( ( emissionCone && ( !oneSidedEmission || !claim.emission ) ) ||
	     !std::isfinite( coneInner ) || !std::isfinite( coneOuter ) ||
	     !std::isfinite( coneExponent ) || coneOuter < -1.0f || coneInner > 1.0f ||
	     coneInner < coneOuter || coneExponent < 0.0f ||
	     ( !emissionCone && ( coneInner != 1.0f || coneOuter != 1.0f || coneExponent != 1.0f ) ) )
	{
		claim.reason = "invalid $emissioncone and its inner, outer or exponent";
		return claim;
	}
	claim.alphaTest = detail::ReadFlag( block, "alphatest" );
	claim.translucent = detail::ReadFlag( block, "translucent" );
	const float transmission = ReadParameter( block, "transmission" );
	const float ior = ReadParameter( block, "ior" );
	const float thickness = ReadParameter( block, "thickness" );
	if ( !pbr::IsValidTransmission( transmission, ior, thickness ) ||
	     !std::isfinite( transmission ) || !std::isfinite( ior ) || !std::isfinite( thickness ) )
	{
		claim.reason = "invalid $transmission, $ior or $thickness";
		return claim;
	}
	if ( transmission > 0.0f )
	{
		if ( !sceneColorAvailable )
		{
			claim.reason = "$transmission needs the view's linear scene color";
			return claim;
		}
		if ( thickness > 0.0f )
		{
			claim.reason = "$thickness needs a depth-aware refraction path";
			return claim;
		}
		if ( claim.alphaTest )
		{
			claim.reason = "$transmission with $alphatest needs a separate coverage decision";
			return claim;
		}
		claim.transmission = true;
		claim.constants.transmission[0] = transmission;
		claim.constants.transmission[1] = ior;
	}
#if defined( RENDER_MATERIAL_PBR_SEEDED_IGNORE_NORMAL_MAP )
	claim.normalMap = false;
#endif
	claim.constants.emission[0] = ReadParameter( block, "emissionscale" );
	claim.constants.emission[1] = oneSidedEmission ? 1.0f : 0.0f;
	claim.constants.emission[2] = cameraOnlyEmission ? 1.0f : 0.0f;
	claim.constants.emissionCone[0] = coneInner;
	claim.constants.emissionCone[1] = coneOuter;
	claim.constants.emissionCone[2] = coneExponent;
	claim.constants.emissionCone[3] = emissionCone ? 1.0f : 0.0f;
	claim.constants.flags[1] = claim.alphaTest ? 1.0f : 0.0f;
	claim.constants.flags[2] = detail::AlphaTestReference( block );
	claim.claimed = true;
	return claim;
}

foundation::Expected<ProgramRequest, PbrStatus> PbrFamily::Request(
    const PbrClaim &claim, SurfaceTextures textures, const SamplerDesc &sampler ) const
{
	if ( !claim.normalMap )
		textures.bump.clear();
	if ( !claim.emission )
		textures.emission.clear();
	return Program().Request( claim.Variant(), claim.constants, textures, sampler );
}

} // namespace render::material
