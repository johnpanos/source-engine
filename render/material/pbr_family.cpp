//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The `pbr` material family's program (RFC 0016 K4); see
//			pbr_family.h.
//
//=============================================================================//

#include "render/material/pbr_family.h"

#include "family_program.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

namespace render::material
{

namespace
{

using namespace render::device;
using detail::ReadParameter;

// The parameters the family draws, and $fallbackmaterial, which only other
// profiles read.
constexpr std::array<std::string_view, 6> kClaimed = { "basetexture", "mraotexture", "bumpmap",
    "emissiontexture", "emissionscale", "fallbackmaterial" };

bool TextureBound( const ParameterBlock &block, std::string_view name )
{
	const FamilySchema &family = block.Family();
	const std::optional<std::size_t> index = family.IndexOf( name );
	if ( !index || family.layout[*index].type != ParameterType::kTexture )
		return false;
	return block.Textures()[family.layout[*index].offset].IsValid();
}

} // namespace

PbrClaim ClaimPbr( const ParameterBlock &block )
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
	claim.normalMap = TextureBound( block, "bumpmap" );
	claim.emission = TextureBound( block, "emissiontexture" );
#if defined( RENDER_MATERIAL_PBR_SEEDED_IGNORE_NORMAL_MAP )
	claim.normalMap = false;
#endif
	claim.constants.emission[0] = ReadParameter( block, "emissionscale" );
	claim.claimed = true;
	return claim;
}

foundation::Expected<std::unique_ptr<PbrFamily>, PbrStatus> PbrFamily::Create(
    IRenderDevice2 &device, Format colorFormat, Format depthFormat )
{
	auto program = SurfaceProgram::Create( device, colorFormat, depthFormat );
	if ( !program )
		return foundation::MakeUnexpected( program.Error() );
	return std::unique_ptr<PbrFamily>( new PbrFamily( std::move( program.Value() ) ) );
}

foundation::Expected<ProgramRequest, PbrStatus> PbrFamily::Request(
    const PbrClaim &claim, SurfaceTextures textures, const SamplerDesc &sampler )
{
	if ( !claim.normalMap )
		textures.bump.clear();
	if ( !claim.emission )
		textures.emission.clear();
	return m_Program->Request( claim.Variant(), claim.constants, textures, sampler );
}

} // namespace render::material
