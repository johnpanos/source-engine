//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `lightmapped` material family's program (RFC 0016 K4); see
//			lightmapped_family.h.
//
//=============================================================================//

#include "render/material/lightmapped_family.h"

#include "family_program.h"

#include <array>
#include <string>
#include <vector>
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
using detail::TextureBound;

// The parameters the family draws, and the ones the caller owns.
constexpr std::array<std::string_view, 32> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "model",
    "nofog", "nocull", "bumpmap", "ssbump", "nodiffusebumplighting", "envmap", "envmapmask",
    "basealphaenvmapmask", "normalmapalphaenvmapmask", "envmaptint", "envmapcontrast",
    "envmapsaturation", "fresnelreflection", "detail", "detailscale", "detailblendmode",
    "detailblendfactor", "detailtint", "selfillum", "selfillumtint", "ssbumpmathfix",
    "envmaplightscale", "envmaplightscaleminmax" };

// The detail modes the port's combos draw: every TextureCombine mode but the
// self-illuminating ones (5, 6) without a bump map, and 0 and 1 with one
// (lightmappedgeneric_ps2_3_x.h's SKIP lines). 10 and 11 are the ssbump
// detail modes, which a detail texture's own flag selects.
bool DetailModeDrawn( int mode, bool bump )
{
	if ( bump )
		return mode == 0 || mode == 1;
	return mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 7 ||
	       mode == 8 || mode == 9;
}

} // namespace

LightmappedClaim ClaimLightmapped( const ParameterBlock &block )
{
	LightmappedClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "lightmapped" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	if ( std::optional<std::string> unclaimed = detail::UnclaimedParameter( block, kClaimed ) )
	{
		claim.reason = "the family does not draw " + *unclaimed;
		return claim;
	}
	// The terms, as lightmappedgeneric_dx9_helper.cpp sets the port's combos.
	const bool bump = TextureBound( block, "bumpmap" );
	const bool envmap = TextureBound( block, "envmap" );
	const bool selfIllum = ReadFlag( block, "selfillum" );
	if ( bump )
	{
		claim.terms |= ReadFlag( block, "ssbump" ) ? kSurfaceSsbump : kSurfaceBump;
		if ( !ReadFlag( block, "nodiffusebumplighting" ) )
			claim.terms |= kSurfaceDiffuseBump;
	}
	if ( envmap )
		claim.terms |= kSurfaceEnvmap;
	if ( TextureBound( block, "envmapmask" ) )
		claim.terms |= kSurfaceEnvmapMask;
	if ( ReadFlag( block, "basealphaenvmapmask" ) )
		claim.terms |= kSurfaceBaseAlphaEnvmapMask;
	if ( ReadFlag( block, "normalmapalphaenvmapmask" ) )
		claim.terms |= kSurfaceNormalMapAlphaEnvmapMask;
	if ( selfIllum )
		claim.terms |= kSurfaceSelfIllum;
	if ( TextureBound( block, "detail" ) )
	{
		const int mode = int( ReadParameter( block, "detailblendmode" ) );
		if ( !DetailModeDrawn( mode, bump ) )
		{
			claim.reason = "the family does not draw $detailblendmode " + std::to_string( mode ) +
			               ( bump ? " with a bump map" : "" );
			return claim;
		}
		claim.terms |= kSurfaceDetail;
		claim.detailMode = std::uint32_t( mode );
	}

	const bool alphaBlended = ReadFlag( block, "translucent" ) || ReadFlag( block, "vertexalpha" );
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	claim.alphaWrite = !alphaBlended && !ReadFlag( block, "alphatest" );
	SurfaceConstants &constants = claim.constants;
	for ( int c = 0; c < 3; ++c )
	{
#if defined( RENDER_MATERIAL_LIGHTMAPPED_SEEDED_GAMMA_COLOR )
		constants.tint[c] = detail::SourceGammaToLinear( ReadParameter( block, "color", c ) );
#else
		constants.tint[c] = ReadParameter( block, "color", c );
#endif
	}
	constants.tint[3] = ReadParameter( block, "alpha" );
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = detail::AlphaTestReference( block );

	// The env map's knobs as the port reads them. Its pixel fast path
	// (contrast 0 or 1, saturation 1, no fresnel, no self-illumination tint)
	// holds unless the material uses contrast with saturation, fresnel, or a
	// self-illumination tint; on it $envmapcontrast is 1 exactly when set to
	// 1 (FASTPATHENVMAPCONTRAST), and anything else is 0.
	float tint[3];
	float selfIllumTint[3];
	for ( int c = 0; c < 3; ++c )
	{
		tint[c] = ReadParameter( block, "envmaptint", c );
		selfIllumTint[c] = ReadParameter( block, "selfillumtint", c );
	}
	const float contrast = ReadParameter( block, "envmapcontrast" );
	const float saturation = ReadParameter( block, "envmapsaturation" );
	const float fresnel = ReadParameter( block, "fresnelreflection" );
	const bool usingContrast = envmap && contrast != 0.0f && contrast != 1.0f && saturation != 1.0f;
	const bool usingFresnel = envmap && fresnel != 1.0f;
	const bool usingSelfIllumTint =
	    selfIllum &&
	    ( selfIllumTint[0] != 1.0f || selfIllumTint[1] != 1.0f || selfIllumTint[2] != 1.0f );
	const bool fastPath = !usingContrast && !usingFresnel && !usingSelfIllumTint;
	for ( int c = 0; c < 3; ++c )
	{
		constants.envTint[c] = tint[c];
		constants.envContrast[c] = fastPath ? ( contrast == 1.0f ? 1.0f : 0.0f ) : contrast;
		constants.envSaturation[c] = fastPath ? 1.0f : saturation;
		constants.selfIllumTint[c] = fastPath ? 1.0f : selfIllumTint[c];
		constants.detailTint[c] = ReadParameter( block, "detailtint", c );
	}
	constants.envTint[3] = fastPath ? 1.0f : fresnel;
	constants.envContrast[3] = fastPath ? 0.0f : 1.0f - fresnel;
	constants.detailTint[3] = ReadParameter( block, "detailblendfactor" );
	constants.detailScale[0] = constants.detailScale[1] = ReadParameter( block, "detailscale" );
	constants.state[2] = ReadFlag( block, "ssbumpmathfix" ) ? 0.57735025882720947f : 1.0f;
	const float lightScaleMin = ReadParameter( block, "envmaplightscaleminmax", 0 );
	constants.envLightScale[0] = lightScaleMin;
	constants.envLightScale[1] =
	    ReadParameter( block, "envmaplightscaleminmax", 1 ) + lightScaleMin;
	constants.envLightScale[2] = ReadParameter( block, "envmaplightscale" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<LightmappedFamily>, LightmappedStatus>
LightmappedFamily::Create( IRenderDevice2 &device, Format colorFormat, Format depthFormat,
    std::uint32_t sampleCount, std::span<const std::uint32_t> fragmentModule )
{
	auto program =
	    SurfaceProgram::Create( device, colorFormat, depthFormat, sampleCount, fragmentModule );
	if ( !program )
		return foundation::MakeUnexpected( program.Error() );
	return std::unique_ptr<LightmappedFamily>(
	    new LightmappedFamily( std::move( program.Value() ) ) );
}

foundation::Expected<ProgramRequest, LightmappedStatus> LightmappedFamily::Request(
    const LightmappedClaim &claim, std::string baseTexture, const SamplerDesc &sampler )
{
	SurfaceTextures textures;
	textures.base = std::move( baseTexture );
	return Request( claim, textures, SurfaceVertexLayout::kFlat, sampler );
}

} // namespace render::material
