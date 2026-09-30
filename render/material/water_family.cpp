//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `water` material family (RFC 0016); see water_family.h.
//
//=============================================================================//

#include "render/material/water_family.h"

#include "family_program.h"

#include <array>
#include <cmath>
#include <optional>
#include <string_view>

namespace render::material
{

namespace
{

using detail::ReadFlag;
using detail::ReadParameter;
using detail::SourceGammaToLinear;
using detail::TextureBound;

// The parameters the point draws, and the ones the caller or the client
// owns ($forceexpensive: the core has no cheap path, so water is always
// expensive; $flow_timescale: not retail's, ignored as retail does;
// $flowmapscrollrate: set by retail's shader, read by no instruction of it;
// $fogstart/$fogend: the engine's fog volume; $flashlighttint: the frame's
// projected lights; $nofog: the view's fog, the caller's).
constexpr std::array<std::string_view, 34> kClaimed = { "basetexture", "frame", "normalmap",
    "bumpframe", "flowmap", "flowmapframe", "flowmapscrollrate", "flow_noise_texture",
    "flow_worlduvscale", "flow_normaluvscale", "flow_timeintervalinseconds",
    "flow_uvscrolldistance", "flow_bumpstrength", "flow_noise_scale", "flow_timescale",
    "color_flow_uvscale", "color_flow_timeintervalinseconds", "color_flow_uvscrolldistance",
    "color_flow_lerpexp", "color_flow_displacebynormalstrength", "reflecttexture", "reflectamount",
    "reflecttint", "envmap", "envmapframe", "forceenvmap", "fogcolor", "fogstart", "fogend",
    "lightmapwaterfog", "abovewater", "forcefresnel", "waterblendfactor", "flashlighttint" };
constexpr std::array<std::string_view, 36> kClaimedWithControls = []
{
	std::array<std::string_view, 36> all{};
	for ( std::size_t i = 0; i < kClaimed.size(); ++i )
		all[i] = kClaimed[i];
	all[34] = "forceexpensive";
	all[35] = "nofog";
	return all;
}();

// The sRGB curve (mathlib SrgbGammaToLinear), which the shader's fog color
// constant takes so it matches the fog the material system applies.
[[maybe_unused]] float SrgbGammaToLinear( float gamma )
{
	return gamma <= 0.04045f ? gamma / 12.92f : std::pow( ( gamma + 0.055f ) / 1.055f, 2.4f );
}

} // namespace

WaterClaim ClaimWater( const ParameterBlock &block )
{
	WaterClaim claim;
	const FamilySchema &family = block.Family();
	if ( family.desc.name != "water" )
	{
		claim.reason = "the block is of family " + family.desc.name;
		return claim;
	}
	// Named before the generic rule, so the reason says what the point lacks.
	if ( TextureBound( block, "refracttexture" ) )
	{
		claim.reason = "water with $refracttexture: the point draws no refraction yet";
		return claim;
	}
	if ( !ReadFlag( block, "abovewater" ) )
	{
		claim.reason = "water seen from below ($abovewater 0): the point draws the surface from "
		               "above only";
		return claim;
	}
	if ( ReadFlag( block, "forcecheap" ) )
	{
		claim.reason = "$forcecheap water: the point draws water_ps2x's expensive path only";
		return claim;
	}
	if ( ReadParameter( block, "flow_debug" ) != 0.0f )
	{
		claim.reason = "$flow_debug views are not drawn";
		return claim;
	}
	if ( std::optional<std::string> unclaimed =
	         detail::UnclaimedParameter( block, kClaimedWithControls ) )
	{
		claim.reason = "the water point does not draw " + *unclaimed;
		return claim;
	}
	const bool reflect = TextureBound( block, "reflecttexture" );
	const bool envmap = TextureBound( block, "envmap" ) && ReadFlag( block, "forceenvmap" );
	if ( !reflect && !envmap )
	{
		claim.reason = "water without $reflecttexture or a forced $envmap is water_ps2x's cheap "
		               "path, which the point does not draw";
		return claim;
	}
	if ( !TextureBound( block, "normalmap" ) )
	{
		claim.reason = "water without $normalmap";
		return claim;
	}
	claim.flow = TextureBound( block, "flowmap" );
	const bool base = TextureBound( block, "basetexture" );
	if ( base && !claim.flow )
	{
		claim.reason = "water with $basetexture and no $flowmap (bumped-lightmap water) is not "
		               "drawn by the point";
		return claim;
	}
	claim.sludge = base && claim.flow;
	claim.reflectTarget = reflect;

	SurfaceConstants &c = claim.constants;
	c.waterFlow[0] = 1.0f / ReadParameter( block, "flow_worlduvscale" );
	c.waterFlow[1] = 1.0f / ReadParameter( block, "flow_normaluvscale" );
	c.waterFlow[2] = ReadParameter( block, "flow_bumpstrength" );
	c.waterFlow[3] = ReadParameter( block, "color_flow_displacebynormalstrength" );
	c.waterFlowTime[0] = ReadParameter( block, "flow_timeintervalinseconds" );
	c.waterFlowTime[1] = ReadParameter( block, "flow_uvscrolldistance" );
	c.waterFlowTime[2] = ReadParameter( block, "flow_noise_scale" );
	c.waterFlowTime[3] = claim.flow ? 1.0f : 0.0f;
	c.waterColorFlow[0] = 1.0f / ReadParameter( block, "color_flow_uvscale" );
	c.waterColorFlow[1] = ReadParameter( block, "color_flow_timeintervalinseconds" );
	c.waterColorFlow[2] = ReadParameter( block, "color_flow_uvscrolldistance" );
	c.waterColorFlow[3] = ReadParameter( block, "color_flow_lerpexp" );
	// SetPixelShaderConstantGammaToLinear: Source's table.
	for ( int k = 0; k < 3; ++k )
		c.waterReflect[k] = SourceGammaToLinear( ReadParameter( block, "reflecttint", k ) );
	c.waterReflect[3] = ReadParameter( block, "waterblendfactor" );
	for ( int k = 0; k < 3; ++k )
	{
#if defined( RENDER_MATERIAL_WATER_SEEDED_GAMMA_FOG_COLOR )
		c.waterFog[k] = ReadParameter( block, "fogcolor", k );
#else
		c.waterFog[k] = SrgbGammaToLinear( ReadParameter( block, "fogcolor", k ) );
#endif
	}
	c.waterFog[3] = ReadParameter( block, "reflectamount" );
	c.waterMode[0] = reflect ? 1.0f : 0.0f;
	c.waterMode[1] = claim.sludge ? 1.0f : 0.0f;
	c.waterMode[2] = ReadFlag( block, "lightmapwaterfog" ) ? 1.0f : 0.0f;
	c.waterMode[3] = ReadParameter( block, "forcefresnel" );
	claim.blend = c.waterReflect[3] < 1.0f ? device::BlendMode::kAlpha : device::BlendMode::kOpaque;
	claim.claimed = true;
	return claim;
}

} // namespace render::material
