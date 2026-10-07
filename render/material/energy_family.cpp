//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: SolidEnergy's claim and constants (energy_family.h).
//=============================================================================//
#include "render/material/energy_family.h"
#include "family_program.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace render::material
{
namespace
{

using detail::ReadFlag;
using detail::ReadParameter;
using detail::TextureBound;

// kDefaultFalloffRanges (solidenergy_dx9_helper.h): an opacity term turned on
// by its $needs* flag without authored ranges.
constexpr float kDefaultRanges[4] = { 1.0f, 0.9f, 0.0f, 0.7f };

// The ranges when authored (any component away from the schema's zero, which
// stands for IS_PARAM_DEFINED false), into row.
bool Ranges( const ParameterBlock &block, std::string_view name, float ( &row )[4] )
{
	bool defined = false;
	for ( std::size_t c = 0; c < 4; ++c )
	{
		row[c] = ReadParameter( block, name, c );
		defined = defined || row[c] != 0.0f;
	}
	if ( !defined )
		std::copy_n( kDefaultRanges, 4, row );
	return defined;
}

bool Identity( const ParameterBlock &block, std::string_view name )
{
	for ( std::size_t i = 0; i < 8; ++i )
		if ( ReadParameter( block, name, i ) != ( ( i == 0 || i == 5 ) ? 1.0f : 0.0f ) )
			return false;
	return true;
}

// SetVertexShaderTextureScaledTransform: the detail's transform when
// authored, else the base's, its scale applied to the first two columns and
// the translation (BaseVSShader.cpp).
void ScaledRows( const ParameterBlock &block, std::string_view transform, float scale,
    float ( &row0 )[4], float ( &row1 )[4] )
{
	const std::string_view source = Identity( block, transform ) ? "basetexturetransform" : transform;
	float m[8];
	for ( std::size_t i = 0; i < 8; ++i )
		m[i] = ReadParameter( block, source, i );
	row0[0] = m[0] * scale;
	row0[1] = m[1] * scale;
	row0[2] = m[2];
	row0[3] = m[3] * scale;
	row1[0] = m[4] * scale;
	row1[1] = m[5] * scale;
	row1[2] = m[6];
	row1[3] = m[7] * scale;
}

} // namespace

UnlitClaim ClaimEnergy( const ParameterBlock &block )
{
	UnlitClaim claim;
	if ( block.Family().desc.name != "energy" )
	{
		claim.reason = "the block is of family " + block.Family().desc.name;
		return claim;
	}
	// SolidEnergy reads no $color or $alpha (BaseShader's, which its pixel
	// shader never multiplies), and declares no $detailscale (its own is
	// $detail1scale; a proxy may use the name as a TextureTransform input).
	constexpr std::array<std::string_view, 55> keys = { "detailscale", "basetexture", "frame", "color", "alpha",
	    "translucent", "additive", "vertexcolor", "vertexalpha", "nocull", "nofog", "model",
	    "basetexturetransform", "detail1", "detail1scale", "detail1frame", "detail1blendmode",
	    "detail1texturetransform", "detail2", "detail2scale", "detail2frame", "detail2blendmode",
	    "detail2texturetransform", "tangenttopacityranges", "tangentsopacityranges",
	    "fresnelopacityranges", "needstangentt", "needstangents", "needsnormals", "flowmap",
	    "flowmapframe", "flow_noise_texture", "flowbounds", "flow_worlduvscale",
	    "flow_normaluvscale", "flow_timeintervalinseconds", "flow_uvscrolldistance",
	    "flow_noise_scale", "flow_lerpexp", "powerup", "flow_color_intensity", "flow_color",
	    "flow_vortex_color", "flow_vortex_size", "flow_vortex1", "flow_vortex_pos1",
	    "flow_vortex2", "flow_vortex_pos2", "flow_cheap", "modelformat", "outputintensity",
	    "detail1blendfactor", "detail2blendfactor", "depthblend", "depthblendscale" };
	if ( const auto unread = detail::UnclaimedParameter( block, keys ) )
	{
		claim.reason = "the energy point does not draw " + *unread;
		return claim;
	}
	if ( !TextureBound( block, "basetexture" ) )
	{
		claim.reason = "SolidEnergy needs its base texture";
		return claim;
	}
	// DrawSolidEnergy's static combos.
	const bool detail1 = TextureBound( block, "detail1" );
	const bool detail2 = detail1 && TextureBound( block, "detail2" );
	const bool flowMap = !detail1 && TextureBound( block, "flowmap" );
	if ( flowMap &&
	     ( !TextureBound( block, "flow_noise_texture" ) || !TextureBound( block, "flowbounds" ) ) )
	{
		claim.reason = "a SolidEnergy flow field needs $flow_noise_texture and $flowbounds";
		return claim;
	}
	const bool cheap = flowMap && ReadFlag( block, "flow_cheap" );
	const bool translucent = ReadFlag( block, "translucent" );
	const bool additive = ReadFlag( block, "additive" );
	const bool vertexColor = ReadFlag( block, "vertexcolor" ) || ReadFlag( block, "vertexalpha" );
	const bool model = ReadFlag( block, "model" ) || ReadFlag( block, "modelformat" );
	SurfaceConstants &constants = claim.constants;
	auto &energy = constants.energy;
	bool tangentT = Ranges( block, "tangenttopacityranges", energy[0] ) ||
	                ReadFlag( block, "needstangentt" );
	bool tangentS = Ranges( block, "tangentsopacityranges", energy[1] ) ||
	                ReadFlag( block, "needstangents" );
	const bool fresnelSet = Ranges( block, "fresnelopacityranges", energy[2] ) ||
	                        ReadFlag( block, "needsnormals" );
	if ( tangentS && tangentT )
		tangentS = false; // both on: T wins
	const bool fresnel = !tangentS && !tangentT && fresnelSet;
	const int detail1Mode =
	    detail1 ? std::clamp( int( ReadParameter( block, "detail1blendmode" ) ), 0, 1 ) : 0;
	const int detail2Mode =
	    detail2 ? std::clamp( int( ReadParameter( block, "detail2blendmode" ) ), 0, 1 ) : 0;

	// The dynamic combos, from the values the draw's proxies left.
	const float powerUp = ReadParameter( block, "powerup" );
	const float intensity = ReadParameter( block, "flow_color_intensity" );
	const bool active = intensity > 0.0f && !( flowMap && powerUp <= 0.0f );
	const bool powerUpCombo = active && flowMap && powerUp > 0.0f && powerUp < 1.0f;
	const bool vortex1 = active && flowMap && ReadFlag( block, "flow_vortex1" );
	const bool vortex2 = active && flowMap && ReadFlag( block, "flow_vortex2" );

	for ( std::size_t i = 0; i < 8; ++i )
		constants.baseTransform[i] = ReadParameter( block, "basetexturetransform", i );
	energy[3][0] = ReadParameter( block, "flow_worlduvscale" );
	energy[3][1] = ReadParameter( block, "flow_normaluvscale" );
	energy[3][2] = ReadParameter( block, "flow_noise_scale" );
	energy[3][3] = ReadParameter( block, "outputintensity" );
	energy[4][0] = ReadParameter( block, "flow_timeintervalinseconds" );
	energy[4][1] = ReadParameter( block, "flow_uvscrolldistance" );
	energy[4][2] = ReadParameter( block, "flow_lerpexp" );
	energy[4][3] = powerUp;
	for ( std::size_t c = 0; c < 3; ++c )
	{
		energy[5][c] = ReadParameter( block, "flow_color", c );
		energy[6][c] = ReadParameter( block, "flow_vortex_color", c );
		energy[7][c] = ReadParameter( block, "flow_vortex_pos1", c );
		energy[8][c] = ReadParameter( block, "flow_vortex_pos2", c );
	}
	energy[5][3] = intensity;
	energy[6][3] = ReadParameter( block, "flow_vortex_size" );
	energy[7][3] = vortex1 ? 1.0f : 0.0f;
	energy[8][3] = vortex2 ? 1.0f : 0.0f;
	if ( flowMap && !( energy[4][0] > 0.0f ) )
	{
		claim.reason = "a SolidEnergy flow field needs a positive $flow_timeintervalinseconds";
		return claim;
	}
	for ( const auto &row : energy )
		for ( float value : row )
			if ( !std::isfinite( value ) )
			{
				claim.reason = "SolidEnergy's parameters must be finite";
				return claim;
			}
	ScaledRows( block, "detail1texturetransform", ReadParameter( block, "detail1scale" ),
	    energy[10], energy[11] );
	ScaledRows( block, "detail2texturetransform", ReadParameter( block, "detail2scale" ),
	    energy[12], energy[13] );
	std::uint32_t flags = ( additive ? kEnergyAdditive : 0u ) | ( detail1 ? kEnergyDetail1 : 0u ) |
	                      ( detail2 ? kEnergyDetail2 : 0u ) | ( tangentT ? kEnergyTangentT : 0u ) |
	                      ( tangentS ? kEnergyTangentS : 0u ) | ( fresnel ? kEnergyFresnel : 0u ) |
	                      ( vertexColor ? kEnergyVertexColor : 0u ) |
	                      ( flowMap ? kEnergyFlowMap : 0u ) | ( model ? kEnergyModelFormat : 0u ) |
	                      ( cheap ? kEnergyFlowCheap : 0u ) | ( powerUpCombo ? kEnergyPowerUp : 0u ) |
	                      ( vortex1 ? kEnergyVortex1 : 0u ) | ( vortex2 ? kEnergyVortex2 : 0u );
	energy[9][0] = float( flags );
	energy[9][1] = float( detail1Mode );
	energy[9][2] = float( detail2Mode );
	energy[9][3] = active ? 1.0f : 0.0f;

	// SolidEnergy's shadow state: src-alpha blending, additive with $additive;
	// opaque without $translucent.
	claim.blend = !translucent ? device::BlendMode::kOpaque
	              : additive   ? device::BlendMode::kAlphaAdditive
	                           : device::BlendMode::kAlpha;
	claim.alphaWrite = !translucent;
	claim.energy = true;
	claim.claimed = true;
	return claim;
}

} // namespace render::material
