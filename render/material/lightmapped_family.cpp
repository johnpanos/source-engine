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
constexpr std::array<std::string_view, 46> kClaimed = { "basetexture", "color", "alpha",
    "vertexcolor", "vertexalpha", "alphatest", "alphatestreference", "translucent", "model",
    "nofog", "nocull", "bumpmap", "ssbump", "nodiffusebumplighting", "envmap", "envmapmask",
    "basealphaenvmapmask", "normalmapalphaenvmapmask", "envmaptint", "envmapcontrast",
    "envmapsaturation", "fresnelreflection", "detail", "detailscale", "detailblendmode",
    "detailblendfactor", "detailtint", "selfillum", "selfillumtint", "ssbumpmathfix",
    "envmaplightscale", "envmaplightscaleminmax", "decal", "alpha2", "allowalphatocoverage",
    "frame", "bumptransform", "basetexture2", "basetexturetransform2", "bumpmap2",
    "blendmodulatetexture", "frame2", "bumpframe", "basetexturenoenvmap", "detail_ssbump",
    "additive" };

// The detail modes the port's combos draw: every TextureCombine mode but the
// self-illuminating ones (5, 6). 10 and 11 are the ssbump detail modes, which
// a detail texture's own flag selects.
//
// A bump map does not narrow this: the port combines the detail into the albedo
// before the bump perturbs the lighting and tests only the detail texture
// (lightmappedgeneric_ps2_3_x.h's "if( bDetailTexture ) albedo =
// TextureCombine( ... )"), and none of its SKIP lines exclude a detail texture
// with a bump map.
bool DetailModeDrawn( int mode )
{
	return mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 7 ||
	       mode == 8 || mode == 9 || mode == 10;
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
		// An ssbump detail texture overrides $detailblendmode.
		const int mode = ReadFlag( block, "detail_ssbump" ) ? ( bump ? 10 : 11 )
		                                                    : int( ReadParameter( block, "detailblendmode" ) );
		if ( !DetailModeDrawn( mode ) )
		{
			claim.reason = "the family does not draw $detailblendmode " + std::to_string( mode );
			return claim;
		}
		// Mode 10 (TCOMBINE_SSBUMP_BUMP) scales the bump's basis weights by the
		// detail, an ssbump's too (Portal 2's shaders and the native backend's
		// lightmapped.frag; the dimples of tile/white_wall_tile003a).
		claim.terms |= kSurfaceDetail;
		claim.detailMode = std::uint32_t( mode );
	}

	// WorldVertexTransition's second layer (lightmappedgeneric_ps2_3_x.h).
	const bool base2 = TextureBound( block, "basetexture2" );
	const bool bump2 = TextureBound( block, "bumpmap2" );
	const bool modulate = TextureBound( block, "blendmodulatetexture" );
	if ( !base2 && ( bump2 || modulate ) )
	{
		claim.reason = "$bumpmap2 and $blendmodulatetexture need $basetexture2";
		return claim;
	}
	if ( bump2 && !bump )
	{
		claim.reason = "$bumpmap2 needs $bumpmap"; // the port's SKIP: !$BUMPMAP && $BUMPMAP2
		return claim;
	}
	if ( bump2 && ReadFlag( block, "normalmapalphaenvmapmask" ) )
	{
		claim.reason = "$bumpmap2 with $normalmapalphaenvmapmask needs the blended normal alpha";
		return claim;
	}
	if ( bump2 && TextureBound( block, "envmapmask" ) )
	{
		claim.reason = "$bumpmap2 with $envmapmask"; // the port's SKIP line
		return claim;
	}
	if ( base2 && ( ReadFlag( block, "basetexturenoenvmap" ) && envmap ) )
	{
		claim.reason = "$basetexturenoenvmap needs the second layer's env map mask";
		return claim;
	}
	if ( ReadParameter( block, "frame2" ) != 0.0f || ReadParameter( block, "bumpframe" ) != 0.0f )
	{
		claim.reason = "the family draws frame 0 of $basetexture2 and $bumpmap";
		return claim;
	}
	// The port's vertex alpha is the layers' blend there, not a coverage.
	if ( base2 && ( ReadFlag( block, "vertexcolor" ) || ReadFlag( block, "vertexalpha" ) ) )
	{
		claim.reason = "$basetexture2 with $vertexcolor or $vertexalpha";
		return claim;
	}
	claim.blendTexture2 = base2;

	const bool alphaBlended = ReadFlag( block, "translucent" ) ||
	                          ReadFlag( block, "vertexalpha" ) ||
	                          ReadParameter( block, "alpha" ) < 1.0f;
	claim.blend = alphaBlended ? BlendMode::kAlpha : BlendMode::kOpaque;
	// $additive (SetAdditiveBlendingShadowState): SRC_ALPHA, ONE where the
	// output alpha is a coverage (the base alpha, $alpha or the vertex
	// alpha: 1 for a base without alpha, so the same as ONE, ONE there), and
	// ONE, ONE where the base alpha is a mask (self-illumination, the env map
	// mask) or alpha tested. It fogs to black (DefaultFog).
	if ( ReadFlag( block, "additive" ) )
	{
		const bool maskAlpha = ReadFlag( block, "selfillum" ) ||
		                       ReadFlag( block, "basealphaenvmapmask" ) ||
		                       ReadFlag( block, "alphatest" );
		claim.blend = maskAlpha && !ReadFlag( block, "vertexalpha" ) &&
		                      ReadParameter( block, "alpha" ) >= 1.0f
		                  ? BlendMode::kAdditive
		                  : BlendMode::kAlphaAdditive;
		claim.fogToBlack = true;
	}
	claim.alphaWrite = claim.blend == BlendMode::kOpaque && !ReadFlag( block, "alphatest" );
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
	// LightmappedGeneric's overlay modulation: nonpositive ALPHA2 means
	// one (lightmappedgeneric_dx9_helper.cpp's pixel constant 12).
	const float alpha2 = ReadParameter( block, "alpha2" );
	if ( !std::isfinite( alpha2 ) )
	{
		claim.reason = "$alpha2 must be finite";
		return claim;
	}
	constants.surfaceControls[3] = alpha2 > 0.0f ? alpha2 : 1.0f;
	constants.flags[0] = ReadFlag( block, "vertexcolor" ) ? 1.0f : 0.0f;
	constants.flags[1] = ReadFlag( block, "alphatest" ) ? 1.0f : 0.0f;
	constants.flags[2] = detail::AlphaTestReference( block );
	claim.alphaToCoverage =
	    constants.flags[1] != 0.0f && !alphaBlended && ReadFlag( block, "allowalphatocoverage" );

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
	// $basetexture2 is read at the base coordinates: WorldVertexTransition's
	// DX9 helper never binds $basetexturetransform2 (worldvertextransition.cpp
	// passes $basetexturetransform), so the key has no effect.
	// $bumpmap2's coordinates are $bumptransform's only without a detail
	// texture; with one the port reads it at the base coordinates.
	if ( !TextureBound( block, "detail" ) )
	{
		for ( std::size_t i = 0; i < 8; ++i )
			constants.bumpTransform[i] = ReadParameter( block, "bumptransform", i );
	}
	constants.blendControls[0] = modulate ? 1.0f : 0.0f;
	constants.blendControls[1] = bump2 ? 1.0f : 0.0f;
	claim.claimed = true;
	return claim;
}

foundation::Expected<ProgramRequest, LightmappedStatus> LightmappedFamily::Request(
    const LightmappedClaim &claim, std::string baseTexture, const SamplerDesc &sampler ) const
{
	SurfaceTextures textures;
	textures.base = std::move( baseTexture );
	return Request( claim, textures, SurfaceVertexLayout::kFlat, sampler );
}

} // namespace render::material
