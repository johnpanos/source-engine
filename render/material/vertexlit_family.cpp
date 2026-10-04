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
constexpr std::array<std::string_view, 71> kMeshClaimed = { "basetexture", "color", "color2",
    "alpha", "translucent", "bumpmap", "phong", "phongexponent", "phongboost", "phongtint",
    "phongfresnelranges", "model", "ignore_alpha_modulation", "selfillum", "selfillummask",
    "selfillumtint", "rimlightexponent", "rimlightboost", "selfillumfresnelminmaxexp",
    "depthblendscale", "envmapsaturation", "envmapcontrast", "envmaptint", "envmapfresnel",
    "envmap", "invertphongmask", "halflambert", "basemapalphaphongmask", "phongalbedotint",
    "rimlight", "alphatest", "alphatestreference", "phongexponenttexture", "nocull",
    "basealphaenvmapmask", "normalmapalphaenvmapmask", "detail", "detailscale", "detailblendmode",
    "detailblendfactor", "detailtint", "blendtintbybasealpha", "blendtintcoloroverbase",
    "envmapfresnelminmaxexp", "envmaplightscale", "envmaplightscaleminmax", "additive",
    "lightwarptexture", "phongwarptexture", "envmapmask", "ssbump", "ssbumpmathfix", "forcephong",
    "selfillumfresnel", "treesway", "treeswayheight", "treeswaystartheight", "treeswayradius",
    "treeswaystartradius", "treeswayspeed", "treeswayspeedhighwindmultiplier", "treeswaystrength",
    "treeswayscrumblespeed", "treeswayscrumblestrength", "treeswayscrumblefrequency",
    "treeswayfalloffexp", "treeswayscrumblefalloffexp", "treeswayspeedlerpstart",
    "treeswayspeedlerpend", "treeswaystatic", "lowqualityflashlightshadows" };

std::optional<std::string> PackTreeSway( const ParameterBlock &block, SurfaceConstants &constants )
{
	const float mode = ReadParameter( block, "treesway" );
	if ( mode < 0 || mode > 2 )
		return "$treesway needs mode 0, 1 or 2";
	constants.treeWind[3] = mode;
	if ( mode == 0 )
		return std::nullopt;
	const char *keys[4][4] = {
	    { "treeswayheight", "treeswaystartheight", "treeswayradius", "treeswaystartradius" },
	    { "treeswayspeed", "treeswaystrength", "treeswayscrumblefrequency", "treeswayscrumblestrength" },
	    { "treeswayspeedhighwindmultiplier", "treeswayscrumblefalloffexp", "treeswayfalloffexp", "treeswayscrumblespeed" },
	    { "treeswayspeedlerpstart", "treeswayspeedlerpend", "treeswaystatic", "treesway" } };
	float *rows[] = { constants.treeGeometry, constants.treeMotion, constants.treeCurves, constants.treeWind };
	for ( int row = 0; row < 4; ++row )
		for ( int column = 0; column < 4; ++column )
		{
			rows[row][column] = ReadParameter( block, keys[row][column] );
			if ( !std::isfinite( rows[row][column] ) )
				return std::string( "$" ) + keys[row][column] + " must be finite";
		}
	const auto &geometry = constants.treeGeometry;
	if ( geometry[0] == 0 || geometry[2] <= 0 )
		return "$treeswayheight must be nonzero and $treeswayradius positive";
	if ( geometry[1] < 0 || geometry[1] >= 1 || geometry[3] < 0 || geometry[3] >= 1 )
		return "$treeswaystartheight and $treeswaystartradius must be in [0, 1)";
	for ( int column = 0; column < 4; ++column )
		if ( constants.treeMotion[column] < 0 )
			return std::string( "$" ) + keys[1][column] + " must be nonnegative";
	const auto &curves = constants.treeCurves;
	for ( int column = 0; column < 4; ++column )
	{
		const bool exponent = column == 1 || column == 2;
		if ( exponent ? curves[column] <= 0 : curves[column] < 0 )
			return std::string( "$" ) + keys[2][column] +
			       ( exponent ? " must be positive" : " must be nonnegative" );
	}
	if ( constants.treeWind[0] < 0 || constants.treeWind[1] <= constants.treeWind[0] )
		return "$treeswayspeedlerpend must exceed the nonnegative $treeswayspeedlerpstart";
	return std::nullopt;
}

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
	const bool phong = ReadFlag( block, "phong" ) || ReadFlag( block, "forcephong" );
	if ( auto invalid = PackTreeSway( block, claim.constants ) )
	{
		claim.reason = *invalid;
		return claim;
	}
	// $lowqualityflashlightshadows is a legacy shader cost hint, not a light
	// visibility term. The native point uses its profile's full shadow filter
	// for either value; it never reduces the authored light or its coverage.
	const float exponent = ReadParameter( block, "phongexponent" );
	const float boost = ReadParameter( block, "phongboost" );
	if ( !std::isfinite( exponent ) ||
	     ( exponent < 0.0f && !detail::TextureBound( block, "phongexponenttexture" ) ) )
	{
		claim.reason = "$phongexponent needs a nonnegative constant or an exponent map";
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
	// The min/max/exp parameter is inert without $selfillumfresnel, and
	// $selfillumfresnel without $selfillum (the shaders' SELFILLUMFRESNEL
	// combo needs SELFILLUM). The legacy material importer includes the
	// zero-valued shader default even on an ordinary self-illuminated door.
	const bool selfIllumFresnel = selfIllum && ReadFlag( block, "selfillumfresnel" );
	claim.normalMap = detail::TextureBound( block, "bumpmap" );
	claim.ssbump = claim.normalMap && ReadFlag( block, "ssbump" );
	if ( ReadFlag( block, "ssbump" ) && !claim.normalMap )
	{
		claim.reason = "$ssbump needs a bound bump texture";
		return claim;
	}
	const bool nativeProbe = detail::TextureBound( block, "envmap" );
	claim.constants.meshModes[1] = nativeProbe ? 1.0f : 0.0f;
	claim.constants.meshModes[2] = phong ? 1.0f : 0.0f;
	const float envFresnel = ReadParameter( block, "envmapfresnel" );
	if ( !std::isfinite( envFresnel ) || envFresnel < 0.0f || envFresnel > 1.0f )
	{
		claim.reason = "$envmapfresnel must be in [0, 1]";
		return claim;
	}
	claim.constants.meshProbeFresnel[0] = nativeProbe ? envFresnel : 1.0f;
	const float probeContrast = ReadParameter( block, "envmapcontrast" );
	const float probeSaturation = ReadParameter( block, "envmapsaturation" );
	if ( !std::isfinite( probeContrast ) || probeContrast < 0.0f || probeContrast > 1.0f ||
	     !std::isfinite( probeSaturation ) || probeSaturation < 0.0f )
	{
		claim.reason = "$envmapcontrast and $envmapsaturation need finite nonnegative values";
		return claim;
	}
	claim.constants.meshProbeColor[0] = probeContrast;
	claim.constants.meshProbeColor[1] = probeSaturation;
	const float lightScale = ReadParameter( block, "envmaplightscale" );
	const float lightMin = ReadParameter( block, "envmaplightscaleminmax", 0 );
	const float lightMax = ReadParameter( block, "envmaplightscaleminmax", 1 );
	if ( !std::isfinite( lightScale ) || lightScale < 0.0f || lightScale > 1.0f ||
	     !std::isfinite( lightMin ) || lightMin < 0.0f || !std::isfinite( lightMax ) ||
	     lightMax < 0.0f )
	{
		claim.reason = "$envmaplightscale needs a [0, 1] weight and nonnegative range";
		return claim;
	}
	claim.constants.envLightScale[0] = lightMin;
	claim.constants.envLightScale[1] = lightMin + lightMax;
	claim.constants.envLightScale[2] = lightScale;
	for ( int c = 0; c < 3; ++c )
	{
		const float control = ReadParameter( block, "envmapfresnelminmaxexp", c );
		if ( !std::isfinite( control ) || control < 0.0f )
		{
			claim.reason = "$envmapfresnelminmaxexp must be finite and nonnegative";
			return claim;
		}
		claim.constants.meshProbeFresnel[c + 1] = control;
	}
	claim.selfIllum = selfIllum;
	claim.selfIllumMask = selfIllum && detail::TextureBound( block, "selfillummask" );
	if ( selfIllumFresnel )
	{
		// The combinations VertexLitGeneric resolves by dropping one of the
		// authored settings (vertexlitgeneric_dx9_helper.cpp, the shaders'
		// SKIP lines) have no single intent to interpret.
		if ( claim.selfIllumMask )
		{
			claim.reason = "$selfillummask with $selfillumfresnel: the fresnel shaders mask by "
			               "base alpha and ignore the mask texture";
			return claim;
		}
		if ( detail::TextureBound( block, "detail" ) )
		{
			claim.reason = "$detail with $selfillumfresnel: VertexLitGeneric drops the fresnel "
			               "term under a detail texture";
			return claim;
		}
		if ( detail::TextureBound( block, "lightwarptexture" ) )
		{
			claim.reason = "$lightwarptexture with $selfillumfresnel: VertexLitGeneric drops the "
			               "light warp under the fresnel term";
			return claim;
		}
		if ( ReadFlag( block, "normalmapalphaenvmapmask" ) )
		{
			claim.reason = "$normalmapalphaenvmapmask with $selfillumfresnel: VertexLitGeneric "
			               "clears the normal-alpha mask under the fresnel term";
			return claim;
		}
		for ( int c = 0; c < 3; ++c )
		{
			const float control = ReadParameter( block, "selfillumfresnelminmaxexp", c );
			if ( !std::isfinite( control ) || control < 0.0f )
			{
				claim.reason = "$selfillumfresnelminmaxexp must be finite and nonnegative";
				return claim;
			}
			claim.constants.selfIllumFresnel[c] = control;
		}
		claim.constants.selfIllumFresnel[3] = 1.0f;
		claim.selfIllumFresnel = true;
	}
	claim.phongExponentTexture = phong && detail::TextureBound( block, "phongexponenttexture" );
	claim.envmapMask = detail::TextureBound( block, "envmapmask" );
	if ( claim.envmapMask && claim.normalMap )
	{
		claim.reason = "$envmapmask with $bumpmap disables the legacy envmap";
		return claim;
	}
	if ( claim.envmapMask && claim.phongExponentTexture )
	{
		claim.reason = "$envmapmask and $phongexponenttexture need distinct texture slots";
		return claim;
	}
	claim.constants.meshProbeColor[3] = claim.envmapMask ? 1.0f : 0.0f;
	claim.lightwarp = phong && detail::TextureBound( block, "lightwarptexture" );
	claim.phongWarp = phong && detail::TextureBound( block, "phongwarptexture" );
	if ( claim.lightwarp && claim.phongWarp )
	{
		claim.reason = "$lightwarptexture and $phongwarptexture need distinct texture slots";
		return claim;
	}
	claim.constants.meshModes[0] = claim.lightwarp ? 1.0f : 0.0f;
	claim.constants.meshProbeColor[2] = claim.phongWarp ? 1.0f : 0.0f;
	claim.detail = detail::TextureBound( block, "detail" );
	if ( claim.detail )
	{
		const float mode = ReadParameter( block, "detailblendmode" );
		if ( !std::isfinite( mode ) || std::floor( mode ) != mode ||
		     !( mode == 0.0f || mode == 1.0f || mode == 2.0f || mode == 3.0f || mode == 4.0f ||
		         mode == 7.0f || mode == 8.0f || mode == 9.0f ) )
		{
			claim.reason = "$detailblendmode is outside the shared surface combine modes";
			return claim;
		}
		claim.detailMode = std::uint32_t( mode );
		const float scale = ReadParameter( block, "detailscale" );
		const float blend = ReadParameter( block, "detailblendfactor" );
		if ( !std::isfinite( scale ) || scale <= 0.0f || !std::isfinite( blend ) || blend < 0.0f ||
		     blend > 1.0f )
		{
			claim.reason = "$detail needs a positive scale and a blend factor in [0, 1]";
			return claim;
		}
		claim.constants.detailScale[0] = claim.constants.detailScale[1] = scale;
		claim.constants.detailTint[3] = blend;
		for ( int c = 0; c < 3; ++c )
			claim.constants.detailTint[c] =
			    SourceGammaToLinear( ReadParameter( block, "detailtint", c ) );
	}
	if ( selfIllum )
	{
		for ( int c = 0; c < 3; ++c )
		{
			const float tint = ReadParameter( block, "selfillumtint", c );
			if ( !std::isfinite( tint ) || tint < 0.0f )
			{
				claim.reason = "$selfillumtint must be finite and nonnegative";
				return claim;
			}
			claim.constants.selfIllumTint[c] = SourceGammaToLinear( tint );
		}
	}
	claim.alphaTest = ReadFlag( block, "alphatest" );
	claim.halfLambert = ReadFlag( block, "halflambert" );
	claim.constants.flags[3] = claim.halfLambert ? 1.0f : 0.0f;
	claim.constants.envSaturation[3] = ReadFlag( block, "invertphongmask" ) ? 1.0f : 0.0f;
	claim.constants.meshControls[0] = ReadFlag( block, "basemapalphaphongmask" ) ? 1.0f : 0.0f;
	claim.constants.meshControls[1] = ReadFlag( block, "phongalbedotint" ) ? 1.0f : 0.0f;
	claim.constants.meshProbeMasks[0] = ReadFlag( block, "basealphaenvmapmask" ) ? 1.0f : 0.0f;
	claim.constants.meshProbeMasks[1] = ReadFlag( block, "normalmapalphaenvmapmask" ) ? 1.0f : 0.0f;
	claim.constants.meshProbeMasks[2] =
	    ReadFlag( block, "blendtintbybasealpha" ) && !selfIllum ? 1.0f : 0.0f;
	const float tintReplacement = ReadParameter( block, "blendtintcoloroverbase" );
	if ( !std::isfinite( tintReplacement ) || tintReplacement < 0.0f || tintReplacement > 1.0f )
	{
		claim.reason = "$blendtintcoloroverbase must be in [0, 1]";
		return claim;
	}
	claim.constants.meshProbeMasks[3] = tintReplacement;
	if ( ReadFlag( block, "rimlight" ) )
	{
		const float exponent = ReadParameter( block, "rimlightexponent" );
		const float rimBoost = ReadParameter( block, "rimlightboost" );
		if ( !std::isfinite( exponent ) || exponent < 0.0f || !std::isfinite( rimBoost ) ||
		     rimBoost < 0.0f )
		{
			claim.reason = "$rimlight exponent and boost must be finite and nonnegative";
			return claim;
		}
		claim.constants.meshControls[2] = rimBoost;
		claim.constants.meshControls[3] = exponent;
	}
	const float alpha = ReadParameter( block, "alpha" );
	if ( !std::isfinite( alpha ) || alpha < 0.0f || alpha > 1.0f )
	{
		claim.reason = "$alpha must be finite and in [0, 1]";
		return claim;
	}
	claim.blend = ReadFlag( block, "additive" )                      ? BlendMode::kAdditive
	              : ReadFlag( block, "translucent" ) || alpha < 1.0f ? BlendMode::kAlpha
	                                                                 : BlendMode::kOpaque;
	claim.constants.flags[1] = claim.alphaTest ? 1.0f : 0.0f;
	claim.constants.flags[2] = detail::AlphaTestReference( block );
	claim.constants.tint[3] = alpha;
	for ( int c = 0; c < 3; ++c )
	{
		const float tint = ReadParameter( block, "color", c ) * ReadParameter( block, "color2", c );
		if ( !std::isfinite( tint ) || tint < 0.0f )
		{
			claim.reason = "$color times $color2 must be finite and nonnegative";
			return claim;
		}
		claim.constants.tint[c] = SourceGammaToLinear( tint );
	}
	claim.constants.pbrFactors[0] = 0.0f; // dielectric
	claim.constants.pbrFactors[1] =
	    phong ? std::clamp( std::sqrt( 2.0f / ( exponent + 2.0f ) ), 0.02f, 1.0f ) : 0.55f;
	claim.constants.pbrFactors[2] = 1.0f;
	claim.constants.pbrFactors[3] = 0.0f; // the material has no MRAO texture
	if ( claim.phongExponentTexture )
		claim.constants.pbrFactors[3] = exponent;
	// Dielectric F0 is a neutral 0.04 in the PBR point. Phong boost and tint
	// become its bounded colored specular reflectance.
	for ( int c = 0; c < 3; ++c )
	{
		claim.constants.envTint[c] = std::clamp(
		    SourceGammaToLinear( ReadParameter( block, "phongtint", c ) ) * boost, 0.0f, 25.0f );
		if ( nativeProbe )
			claim.constants.envSaturation[c] =
			    SourceGammaToLinear( ReadParameter( block, "envmaptint", c ) );
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
